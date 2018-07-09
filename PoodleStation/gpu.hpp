#ifndef GPU_HPP
#define GPU_HPP
#include <cstdint>

struct GPUSTAT
{
    bool ready_cmd;
    bool ready_DMA;
};

struct DrawMode
{
    uint8_t texbase_x, texbase_y;
    uint8_t semi_trans;
    uint8_t tex_colors;
    bool tex_rect_x_flip;
    bool tex_rect_y_flip;
};

struct TexWindow
{
    uint16_t mask_x, mask_y;
    uint16_t offset_x, offset_y;
};

struct ClipArea
{
    uint16_t x1, x2, y1, y2;
};

struct DrawOffset
{
    int16_t x, y;
};

struct DisplayStart
{
    uint16_t x; //in halfwords
    uint16_t y; //scanlines
};

struct Vertex
{
    int16_t x, y;
    uint16_t s, t;
    uint32_t color;

    Vertex();
    Vertex(uint32_t v, uint32_t color);

    void set_texcoords(uint32_t param);
};

struct RenderContext
{
    uint16_t palette;
    uint16_t texpage;
    bool opaque;
    bool textured;
    bool texture_blending;
};

class GPU
{
    private:
        uint8_t* VRAM;
        uint32_t* framebuffer;
        DrawMode draw_mode;
        ClipArea clip_area;
        DrawOffset draw_offset;
        TexWindow tex_window;
        RenderContext context;
        bool force_mask_draw;
        bool check_mask;
        bool is_odd_frame;

        DisplayStart display_start;

        GPUSTAT stat;
        uint32_t params[20];
        uint8_t cmd;
        uint32_t option;
        int cmd_params;
        int params_needed;

        bool IRQ;

        uint32_t transfer_dir;

        uint16_t transfer_x, transfer_y;
        uint32_t transfer_w, transfer_h;
        uint32_t transfer_x_bound;
        uint32_t transfer_size;
        bool read_transfer;
        bool write_transfer;

        bool display_enabled;

        void transfer_to_VRAM();

        int32_t orient2D(Vertex& v1, Vertex& v2, Vertex& v3);
        void draw_quad(bool textured, bool shaded);
        void draw_tri(Vertex vertices[]);
        void draw_rect(Vertex& corner, int width, int height);
        void draw_pixel(uint16_t x, uint16_t y, uint32_t color);

        uint16_t tex_lookup(uint32_t texpage_x, uint32_t texpage_y, uint8_t s, uint8_t t, uint8_t color_depth);
    public:
        GPU();
        ~GPU();

        uint32_t* get_framebuffer();
        void reset();
        void new_frame();

        void render_frame();

        uint32_t read_response();
        uint32_t read_stat();
        void write_GP0(uint32_t value);
        void write_GP1(uint32_t value);
};

#endif // GPU_HPP
