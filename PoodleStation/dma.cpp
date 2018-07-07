#include <cstdio>
#include "dma.hpp"

DMA::DMA()
{

}

void DMA::reset()
{
    PCR = 0x07654321;
    ICR.MASK = 0;
    ICR.STAT = 0;
}

uint32_t DMA::read_PCR()
{
    printf("[DMA] Read PCR: $%08X\n", PCR);
    return 0;
}

uint32_t DMA::read_ICR()
{
    uint32_t reg = 0;
    reg |= ICR.force_IRQ << 15;
    reg |= ICR.MASK << 16;
    reg |= ICR.master_IRQ_enable << 23;
    reg |= ICR.STAT << 24;

    reg |= (ICR.force_IRQ || (ICR.master_IRQ_enable && (ICR.MASK & ICR.STAT))) << 31;
    printf("[DMA] Read ICR: $%08X\n", reg);
    return reg;
}

void DMA::write_PCR(uint32_t value)
{
    printf("[DMA] Write PCR: $%08X\n", value);
    PCR = value;
}

void DMA::write_ICR(uint32_t value)
{
    printf("[DMA] Write ICR: $%08X\n", value);
    ICR.force_IRQ = value & (1 << 15);
    ICR.MASK = (value >> 16) & 0x7F;
    ICR.master_IRQ_enable = value & (1 << 23);
    ICR.STAT &= ~((value >> 24) & 0x7F);
}
