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
    uint32_t texbase_x, texbase_y;
    uint8_t semi_trans;
    uint8_t tex_colors;
    bool tex_rect_x_flip;
    bool tex_rect_y_flip;
};

class GPU
{
    private:
        GPUSTAT stat;
    public:
        GPU();

        void reset();

        uint32_t read_response();
        uint32_t read_stat();
        void write_GP0(uint32_t value);
        void write_GP1(uint32_t value);
};

#endif // GPU_HPP
