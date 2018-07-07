#include <cstdio>
#include "gpu.hpp"

GPU::GPU()
{

}

uint32_t GPU::read_stat()
{
    return 0;
}

void GPU::write_GP0(uint32_t value)
{
    printf("[GPU] Write GP0: $%08X\n", value);
}
