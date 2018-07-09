#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include "gpu.hpp"

//#define printf(fmt, ...)(0)

using namespace std;

Vertex::Vertex() {}

Vertex::Vertex(uint32_t v, uint32_t color) : color(color)
{
    x = (int16_t)((v & 0x7FF) << 5) >> 5;
    y = (int16_t)(((v >> 16) & 0x7FF) << 5) >> 5;
}

void Vertex::set_texcoords(uint32_t param)
{
    s = param & 0xFF;
    t = (param >> 8) & 0xFF;
}

GPU::GPU()
{
    VRAM = nullptr;
    framebuffer = nullptr;
}

GPU::~GPU()
{
    if (VRAM)
        delete[] VRAM;
    if (framebuffer)
        delete[] framebuffer;
}

void GPU::reset()
{
    if (!VRAM)
        VRAM = new uint8_t[1024 * 1024];
    if (!framebuffer)
        framebuffer = new uint32_t[640 * 480];
    is_odd_frame = false;
    stat.ready_cmd = true;
    stat.ready_DMA = true;

    IRQ = false;

    display_start.x = 0;
    display_start.y = 0;

    draw_mode.texbase_x = 0;
    draw_mode.texbase_y = 0;
    draw_mode.semi_trans = 0;
    draw_mode.tex_colors = 0;

    read_transfer = false;
    write_transfer = false;
    cmd_params = 0;
    params_needed = 0;
}

void GPU::new_frame()
{
    is_odd_frame = !is_odd_frame;
}

void GPU::render_frame()
{
    printf("Display start: (%d, %d)\n", display_start.x, display_start.y);
    for (int y = 0; y < 480; y++)
    {
        for (int x = 0; x < 640; x++)
        {
            int pix_x = x + display_start.x;
            int pix_y = y + display_start.y;
            uint32_t final_color = 0xFF000000;
            if (display_enabled)
            {
                uint16_t color = *(uint16_t*)&VRAM[(pix_x + (pix_y * 1024)) * 2];
                final_color |= (color & 0x1F) << 3;
                final_color |= ((color >> 5) & 0x1F) << 11;
                final_color |= ((color >> 10) & 0x1F) << 19;
            }

            framebuffer[x + (y * 640)] = final_color;
        }
    }
}

uint32_t* GPU::get_framebuffer()
{
    return framebuffer;
}

uint32_t GPU::read_response()
{
    uint32_t value = 0;
    if (read_transfer)
    {
        //printf("Transfer: %d\n", transfer_x);
        value = *(uint32_t*)&VRAM[(transfer_x + (transfer_y * 1024)) * 2];
        printf("Read transfer: $%08X\n", value);
        transfer_x += 2;
        if (transfer_x >= transfer_x_bound)
        {
            transfer_y++;
            if (transfer_y >= transfer_h)
            {
                read_transfer = false;
                printf("[GPU] VRAM->CPU transfer ended!\n");
            }
            else
                transfer_x -= transfer_w;
        }
    }
    return value;
}

uint32_t GPU::read_stat()
{
    uint32_t reg = 0;
    reg |= draw_mode.texbase_x;
    reg |= draw_mode.texbase_y << 4;
    reg |= draw_mode.semi_trans << 5;
    reg |= draw_mode.tex_colors << 7;
    reg |= force_mask_draw << 11;
    reg |= check_mask << 12;
    reg |= IRQ << 24;
    switch (transfer_dir)
    {
        case 1:
            reg |= 1 << 25;
            break;
        case 2:
            reg |= stat.ready_DMA << 25;
            break;
        case 3:
            reg |= read_transfer << 25;
            break;
    }
    reg |= stat.ready_cmd << 26;
    reg |= read_transfer << 27;
    reg |= stat.ready_DMA << 28;
    reg |= is_odd_frame << 31;
    printf("[GPU] Read STAT: $%08X\n", reg);
    return reg;
}

void GPU::draw_quad(bool textured, bool shaded)
{
    Vertex tri1[3], tri2[3];

    int cmd = textured | (shaded << 1);
    switch (cmd)
    {
        case 0:
            tri1[0] = Vertex(params[0], option);
            tri1[1] = Vertex(params[1], option);
            tri1[2] = Vertex(params[2], option);

            tri2[0] = Vertex(params[1], option);
            tri2[1] = Vertex(params[2], option);
            tri2[2] = Vertex(params[3], option);
            break;
        case 1:
            tri1[0] = Vertex(params[0], option);
            tri1[0].set_texcoords(params[1]);
            tri1[1] = Vertex(params[2], option);
            tri1[1].set_texcoords(params[3]);
            tri1[2] = Vertex(params[4], option);
            tri1[2].set_texcoords(params[5]);

            tri2[0] = Vertex(params[2], option);
            tri2[0].set_texcoords(params[3]);
            tri2[1] = Vertex(params[4], option);
            tri2[1].set_texcoords(params[5]);
            tri2[2] = Vertex(params[6], option);
            tri2[2].set_texcoords(params[7]);
            break;
        case 2:
            tri1[0] = Vertex(params[0], option);
            tri1[1] = Vertex(params[2], params[1]);
            tri1[2] = Vertex(params[4], params[3]);

            tri2[0] = Vertex(params[2], params[1]);
            tri2[1] = Vertex(params[4], params[3]);
            tri2[2] = Vertex(params[6], params[5]);
            break;
    }

    draw_tri(tri1);
    draw_tri(tri2);
}

int32_t GPU::orient2D(Vertex &v1, Vertex &v2, Vertex &v3)
{
    return (v2.x - v1.x) * (v3.y - v1.y) - (v3.x - v1.x) * (v2.y - v1.y);
}

void GPU::draw_tri(Vertex vertices[])
{
    uint32_t texpage_x = context.texpage & 0xF;
    uint32_t texpage_y = ((context.texpage >> 4) & 0x1) * 256;
    int color_depth = (context.texpage >> 7) & 0x3;
    Vertex v1 = vertices[0], v2 = vertices[1], v3 = vertices[2];

    v1.x += draw_offset.x;
    v2.x += draw_offset.x;
    v3.x += draw_offset.x;

    v1.y += draw_offset.y;
    v2.y += draw_offset.y;
    v3.y += draw_offset.y;

    printf("[GPU] Draw triangle: (%d, %d) (%d, %d) (%d, %d)\n", v1.x, v1.y, v2.x, v2.y, v3.x, v3.y);

    if (orient2D(v1, v2, v3) < 0)
        swap(v2, v3);

    int32_t min_x = min({v1.x, v2.x, v3.x});
    int32_t min_y = min({v1.y, v2.y, v3.y});
    int32_t max_x = max({v1.x, v2.x, v3.x});
    int32_t max_y = max({v1.y, v2.y, v3.y});

    int32_t A12 = v1.y - v2.y;
    int32_t B12 = v2.x - v1.x;
    int32_t A23 = v2.y - v3.y;
    int32_t B23 = v3.x - v2.x;
    int32_t A31 = v3.y - v1.y;
    int32_t B31 = v1.x - v3.x;

    Vertex min_corner;
    min_corner.x = min_x;
    min_corner.y = min_y;
    int32_t w1_row = orient2D(v2, v3, min_corner);
    int32_t w2_row = orient2D(v3, v1, min_corner);
    int32_t w3_row = orient2D(v1, v2, min_corner);

    int r1 = v1.color & 0xFF;
    int r2 = v2.color & 0xFF;
    int r3 = v3.color & 0xFF;

    int g1 = (v1.color >> 8) & 0xFF;
    int g2 = (v2.color >> 8) & 0xFF;
    int g3 = (v3.color >> 8) & 0xFF;

    int b1 = (v1.color >> 16) & 0xFF;
    int b2 = (v2.color >> 16) & 0xFF;
    int b3 = (v3.color >> 16) & 0xFF;

    int32_t divider = orient2D(v1, v2, v3);

    bool transparent = false;

    for (int32_t y = min_y; y <= max_y; y++)
    {
        int32_t w1 = w1_row;
        int32_t w2 = w2_row;
        int32_t w3 = w3_row;

        for (int32_t x = min_x; x <= max_x; x++)
        {
            if ((w1 | w2 | w3) >= 0)
            {
                int r = ((float) r1 * w1 + (float) r2 * w2 + (float) r3 * w3) / divider;
                int g = ((float) g1 * w1 + (float) g2 * w2 + (float) g3 * w3) / divider;
                int b = ((float) b1 * w1 + (float) b2 * w2 + (float) b3 * w3) / divider;
                if (context.textured)
                {
                    int s = ((float) v1.s * w1 + (float) v2.s * w2 + (float) v3.s * w3) / divider;
                    int t = ((float) v1.t * w1 + (float) v2.t * w2 + (float) v3.t * w3) / divider;

                    uint16_t tex_color = tex_lookup(texpage_x, texpage_y, s, t, color_depth);

                    r = (tex_color & 0x1F) << 3;
                    g = ((tex_color >> 5) & 0x1F) << 3;
                    b = ((tex_color >> 10) & 0x1F) << 3;

                    transparent = !tex_color;
                }

                if (!transparent)
                {
                    uint32_t color = r | (g << 8) | (b << 16);
                    draw_pixel(x, y, color);
                }
            }
            w1 += A23;
            w2 += A31;
            w3 += A12;
        }
        //Vertical step
        w1_row += B23;
        w2_row += B31;
        w3_row += B12;
    }
}

void GPU::draw_rect(Vertex& corner, int width, int height)
{
    printf("Draw rect: (%d, %d)\n", corner.x, corner.y);
    bool transparent = false;
    for (int y = corner.y; y < corner.y + height; y++)
    {
        for (int x = corner.x; x < corner.x + width; x++)
        {
            int r = option & 0xFF;
            int g = (option >> 8) & 0xFF;
            int b = (option >> 16) & 0xFF;
            if (context.textured)
            {
                int s = (x - corner.x) & 0xFF;
                int t = (y - corner.y) & 0xFF;
                uint16_t tex_color = tex_lookup(draw_mode.texbase_x, draw_mode.texbase_y * 256, s, t, draw_mode.tex_colors);

                r = (tex_color & 0x1F) << 3;
                g = ((tex_color >> 5) & 0x1F) << 3;
                b = ((tex_color >> 10) & 0x1F) << 3;

                transparent = !tex_color;
            }
            if (!transparent)
            {
                uint32_t color = r | (g << 8) | (b << 16);
                draw_pixel(x, y, color);
            }
        }
    }
}

void GPU::draw_pixel(uint16_t x, uint16_t y, uint32_t color)
{
    if (x < clip_area.x1 || x > clip_area.x2 || y < clip_area.y1 || y > clip_area.y2)
        return;

    uint16_t final_color;
    int r = (color & 0xFF) >> 3;
    int g = ((color >> 8) & 0xFF) >> 3;
    int b = ((color >> 16) & 0xFF) >> 3;
    final_color = r;
    final_color |= g << 5;
    final_color |= b << 10;
    *(uint16_t*)&VRAM[(x + (y * 1024)) * 2] = final_color;
}

uint16_t GPU::tex_lookup(uint32_t texpage_x, uint32_t texpage_y, uint8_t s, uint8_t t, uint8_t color_depth)
{
    uint32_t base = ((texpage_x * 64) + (texpage_y * 1024)) * 2;
    base += (t * 2048);
    //printf("Base: $%08X Texpage: $%08X\n", base, context.texpage);

    int pal_index;
    uint16_t color;
    uint32_t palette_addr;
    switch (color_depth)
    {
        case 0:
            //printf("(%d, %d)\n", s, t);
            palette_addr = context.palette * 16 * 2;
            pal_index = *(uint8_t*)&VRAM[base + (s / 2)];
            if (s & 0x1)
                pal_index >>= 4;
            else
                pal_index &= 0xF;
            color = *(uint16_t*)&VRAM[palette_addr + (pal_index * 2)];
            break;
        case 1:
            palette_addr = context.palette * 256 * 2;
            pal_index = *(uint8_t*)&VRAM[base + s];
            color = *(uint16_t*)&VRAM[palette_addr + (pal_index * 2)];
            break;
        case 2:
        case 3:
            color = *(uint16_t*)&VRAM[base + (s * 2)];
            color = 0x8000;
            break;
        default:
            printf("[GPU] Unrecognized texture color %d\n", color_depth);
            exit(1);
    }
    return color;
}

void GPU::write_GP0(uint32_t value)
{
    printf("[GPU] Write GP0: $%08X\n", value);
    if (write_transfer)
    {
        printf("Transfer: $%08X (%d, %d) ($%08X)\n", value, transfer_x, transfer_y, (transfer_x + (transfer_y * 1024)) * 2);
        *(uint32_t*)&VRAM[(transfer_x + (transfer_y * 1024)) * 2] = value;
        transfer_x += 2;
        if (transfer_x >= transfer_x_bound)
        {
            transfer_y++;
            if (transfer_y >= transfer_h)
            {
                write_transfer = false;
                printf("[GPU] CPU->VRAM transfer ended!\n");
            }
            else
                transfer_x -= transfer_w;
        }
        return;
    }
    if (stat.ready_cmd)
    {
        cmd = value >> 24;
        option = value & 0xFFFFFF;
        switch (cmd)
        {
            case 0x00:
                //NOP
                break;
            case 0x01:
                printf("[GPU] Clear cache\n");
                break;
            case 0x02:
                printf("[GPU] Fill VRAM\n");
                stat.ready_cmd = false;
                params_needed = 2;
                break;
            case 0x27:
                printf("[GPU] Textured three-point polygon: $%08X\n", option);
                stat.ready_cmd = false;
                params_needed = 6;
                break;
            case 0x28:
                printf("[GPU] Monochrome four-point polygon: $%08X\n", option);
                stat.ready_cmd = false;
                params_needed = 4;
                break;
            case 0x2C:
                printf("[GPU] Textured four-point polygon, opaque, blended: $%08X\n", option);
                stat.ready_cmd = false;
                params_needed = 8;
                break;
            case 0x2D:
                printf("[GPU] Textured four-point polygon, opaque, raw: $%08X\n", option);
                stat.ready_cmd = false;
                params_needed = 8;
                break;
            case 0x30:
                printf("[GPU] Shaded three-point polygon: $%08X\n", option);
                stat.ready_cmd = false;
                params_needed = 5;
                break;
            case 0x38:
                printf("[GPU] Shaded four-point polygon: $%08X\n", option);
                stat.ready_cmd = false;
                params_needed = 7;
                break;
            case 0x65:
                printf("[GPU] Textured rectangle, variable size, opaque, raw: $%08X\n", option);
                stat.ready_cmd = false;
                params_needed = 3;
                break;
            case 0x78:
                printf("[GPU] Monochrome rectangle, opaque: $%08X\n", option);
                stat.ready_cmd = false;
                params_needed = 1;
                break;
            case 0xA0:
                printf("[GPU] CPU->VRAM transfer\n");
                stat.ready_cmd = false;
                params_needed = 2;
                break;
            case 0xC0:
                printf("[GPU] VRAM->CPU transfer\n");
                stat.ready_cmd = false;
                params_needed = 2;
                break;
            case 0xE1:
                printf("[GPU] Draw mode: $%08X\n", option);
                draw_mode.texbase_x = value & 0xF;
                draw_mode.texbase_y = value & (1 << 4);
                draw_mode.semi_trans = (value >> 5) & 0x3;
                draw_mode.tex_colors = (value >> 7) & 0x3;
                draw_mode.tex_rect_x_flip = value & (1 << 12);
                draw_mode.tex_rect_y_flip = value & (1 << 13);
                break;
            case 0xE2:
                printf("[GPU] Tex window: $%08X\n", option);
                tex_window.mask_x = value & 0x1F;
                tex_window.mask_y = (value >> 5) & 0x1F;
                tex_window.offset_x = (value >> 10) & 0x1F;
                tex_window.offset_y = (value >> 15) & 0x1F;
                break;
            case 0xE3:
                printf("[GPU] Top-left clip: $%08X\n", option);
                clip_area.x1 = value & 0x3FF;
                clip_area.y1 = (value >> 10) & 0x1FF;
                break;
            case 0xE4:
                printf("[GPU] Bottom-right clip: $%08X\n", option);
                clip_area.x2 = value & 0x3FF;
                clip_area.y2 = (value >> 10) & 0x1FF;
                printf("(%d, %d) (%d, %d)\n", clip_area.x1, clip_area.y1, clip_area.x2, clip_area.y2);
                break;
            case 0xE5:
                printf("[GPU] Draw offset: $%08X\n", option);
                draw_offset.x = ((int16_t)((value & 0x7FF) << 4)) >> 4;
                draw_offset.y = ((int16_t)(((value >> 11) & 0x7FF) << 4)) >> 4;
                break;
            case 0xE6:
                printf("[GPU] Mask Bit: $%08X\n", option);
                force_mask_draw = value & 0x1;
                check_mask = value & (1 << 1);
                break;
            default:
                printf("[GPU] Unrecognized GP0 command $%02X! ($%08X)\n", cmd, value);
                exit(1);
        }
    }
    else
    {
        params[cmd_params] = value;
        cmd_params++;

        if (cmd_params >= params_needed)
        {
            stat.ready_cmd = true;
            cmd_params = 0;
            switch (cmd)
            {
                case 0x02:
                {
                    int fill_x = (params[0] & 0xFFFF);
                    int fill_y = params[0] >> 16;

                    int fill_w = (params[1] & 0xFFFF);
                    int fill_x_bound = fill_w + fill_x;
                    int fill_h = (params[1] >> 16) + fill_y;

                    printf("(%d, %d) (%d, %d)\n", fill_x, fill_y, fill_w, fill_h);

                    //The color in option is 24-bit but converted to 15-bit during fill
                    uint16_t color = (option & 0xFF) >> 3;
                    color |= ((option >> 8) & 0xFF) >> 3;
                    color |= ((option >> 16) & 0xFF) >> 3;

                    while (fill_y < fill_h)
                    {
                        printf("Fill: (%d, %d)\n", fill_x, fill_y);
                        *(uint16_t*)&VRAM[(fill_x + (fill_y * 1024)) * 2] = color;
                        fill_x++;
                        if (fill_x >= fill_x_bound)
                        {
                            fill_x -= fill_w;
                            fill_y++;
                        }
                    }
                }
                    break;
                case 0x28:
                    context.textured = false;
                    context.texture_blending = false;
                    context.opaque = true;
                    draw_quad(false, false);
                    break;
                case 0x2C:
                    context.textured = true;
                    context.texture_blending = true;
                    context.opaque = true;
                    context.palette = params[1] >> 16;
                    context.texpage = params[3] >> 16;
                    printf("Palette: $%04X Texpage: $%04X\n", context.palette, context.texpage);
                    draw_quad(true, false);
                    break;
                case 0x2D:
                    context.textured = true;
                    context.texture_blending = false;
                    context.opaque = true;
                    context.palette = params[1] >> 16;
                    context.texpage = params[3] >> 16;
                    printf("Palette: $%04X Texpage: $%04X\n", context.palette, context.texpage);
                    draw_quad(true, false);
                    break;
                case 0x30:
                {
                    context.textured = false;
                    context.texture_blending = false;
                    context.opaque = true;
                    Vertex tri[3];
                    tri[0] = Vertex(params[0], option);
                    tri[1] = Vertex(params[2], params[1]);
                    tri[2] = Vertex(params[4], params[3]);
                    draw_tri(tri);
                }
                    break;
                case 0x38:
                    context.textured = false;
                    context.texture_blending = false;
                    context.opaque = true;
                    draw_quad(false, true);
                    break;
                case 0x65:
                {
                    context.textured = true;
                    context.texture_blending = false;
                    context.opaque = true;
                    context.palette = params[1] >> 16;
                    Vertex corner;
                    corner = Vertex(params[0], option);
                    corner.set_texcoords(params[1]);

                    int w = params[2] & 0xFFFF;
                    int h = params[2] >> 16;

                    draw_rect(corner, w, h);
                }
                    break;
                case 0x78:
                {
                    context.textured = false;
                    context.opaque = true;
                    Vertex corner;
                    corner = Vertex(params[0], option);
                    draw_rect(corner, 16, 16);
                }
                    break;
                case 0xA0:
                    transfer_x = params[0] & 0xFFFF;
                    transfer_y = params[0] >> 16;
                    transfer_w = params[1] & 0xFFFF;
                    transfer_x_bound = transfer_x + transfer_w;
                    transfer_h = (params[1] >> 16) + transfer_y;
                    printf("(%d, %d) (%d, %d)\n", transfer_x, transfer_y, transfer_w, transfer_h);
                    write_transfer = true;
                    break;
                case 0xC0:
                    transfer_x = params[0] & 0xFFFF;
                    transfer_y = params[0] >> 16;
                    transfer_w = params[1] & 0xFFFF;
                    transfer_x_bound = transfer_x + transfer_w;
                    transfer_h = (params[1] >> 16) + transfer_y;
                    printf("(%d, %d) (%d, %d)\n", transfer_x, transfer_y, transfer_w, transfer_h);
                    read_transfer = true;
                    break;
                default:
                    exit(1);
            }
        }
    }
}

void GPU::write_GP1(uint32_t value)
{
    //printf("[GPU] Write GP1: $%08X\n", value);
    cmd = value >> 24;
    option = value & 0xFFFFFF;
    switch (cmd)
    {
        case 0x00:
            printf("[GPU] Reset\n");
            reset();
            break;
        case 0x01:
            printf("[GPU] Reset command buffer\n");
            cmd_params = 0;
            stat.ready_cmd = true;
            break;
        case 0x02:
            printf("[GPU] IRQ acknowledge\n");
            IRQ = false;
            break;
        case 0x03:
            printf("[GPU] Display enable\n");
            display_enabled = !(value & 0x1);
            break;
        case 0x04:
            printf("[GPU] DMA dir: $%08X\n", option);
            transfer_dir = option & 0x3;
            break;
        case 0x05:
            printf("[GPU] Display start: $%08X\n", option);
            display_start.x = value & 0x3FF;
            display_start.y = (value >> 10) & 0x1FF;
            break;
        case 0x06:
            printf("[GPU] Horizontal range: $%08X\n", option);
            break;
        case 0x07:
            printf("[GPU] Vertical range: $%08X\n", option);
            break;
        case 0x08:
            printf("[GPU] Display mode: $%08X\n", option);
            break;
        default:
            printf("[GPU] Unrecognized GP1 command $%02X! ($%08X)\n", cmd, value);
            exit(1);
     }
}
