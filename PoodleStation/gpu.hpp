#ifndef GPU_HPP
#define GPU_HPP
#include <cstdint>

class GPU
{
    public:
        GPU();

        uint32_t read_stat();
        void write_GP0(uint32_t value);
};

#endif // GPU_HPP
