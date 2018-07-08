#include <cstdio>
#include <cstdlib>
#include "gpu.hpp"

GPU::GPU()
{

}

void GPU::reset()
{
    stat.ready_cmd = true;
    stat.ready_DMA = true;
}

uint32_t GPU::read_response()
{
    return 0;
}

uint32_t GPU::read_stat()
{
    uint32_t reg = 0;
    reg |= stat.ready_cmd << 26;
    reg |= stat.ready_DMA << 28;
    printf("[GPU] Read STAT: $%08X\n", reg);
    return reg;
}

void GPU::write_GP0(uint32_t value)
{
    printf("[GPU] Write GP0: $%08X\n", value);
    if (stat.ready_cmd)
    {
        uint8_t cmd = value >> 24;
        switch (cmd)
        {
            default:
                printf("[GPU] Unrecognized GP0 command $%02X!\n", cmd);
                exit(1);
        }
    }
}

void GPU::write_GP1(uint32_t value)
{
    printf("[GPU] Write GP1: $%08X\n", value);
    if (stat.ready_cmd)
    {
        uint8_t cmd = value >> 24;
        switch (cmd)
        {
            default:
                printf("[GPU] Unrecognized GP1 command $%02X!\n", cmd);
                exit(1);
        }
    }
}
