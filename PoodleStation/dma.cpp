#include <cstdio>
#include <cstdlib>
#include "dma.hpp"
#include "emulator.hpp"
#include "gpu.hpp"

static const char* NAMES[] =
{
    "MDECin",
    "MDECout",
    "GPU",
    "CDROM",
    "SPU",
    "PIO",
    "OTC"
};

DMA::DMA(Emulator* e, GPU* gpu) : e(e), gpu(gpu)
{

}

void DMA::reset(uint8_t* RAM)
{
    this->RAM = RAM;
    PCR = 0x07654321;
    ICR.MASK = 0;
    ICR.STAT = 0;

    for (int i = 0; i < 7; i++)
    {
        channels[i].addr = 0;
        channels[i].block = 0;
        channels[i].active = false;
        channels[i].busy = false;
    }
}

bool DMA::run()
{
    for (int i = 0; i < 7; i++)
    {
        if (PCR & (1 << ((i << 2) + 3)))
        {
            if (channels[i].active)
            {
                switch (i)
                {
                    case 2:
                        process_GPU();
                        break;
                    case 6:
                        process_OTC();
                        break;
                }
                return true;
            }
        }
    }
    return false;
}

void DMA::process_GPU()
{
    DMA_Channel* GPU_chan = &channels[2];
    switch (GPU_chan->sync_mode)
    {
        case 1: //DMA request mode
            gpu->write_GP0(*(uint32_t*)&RAM[GPU_chan->addr]);
            GPU_chan->word_count--;
            GPU_chan->addr += 4;
            if (!GPU_chan->word_count)
                end_transfer(2);
            break;
        case 2: //Linked list mode
            if (!GPU_chan->word_count)
            {
                if (GPU_chan->next_addr == 0xFFFFFF)
                    end_transfer(2);
                else
                {
                    GPU_chan->addr = GPU_chan->next_addr;

                    uint32_t pointer = *(uint32_t*)&RAM[GPU_chan->addr];
                    //printf("[GPU DMA] Pointer: $%08X\n", pointer);
                    GPU_chan->next_addr = pointer & 0xFFFFFF;
                    GPU_chan->word_count = pointer >> 24;
                }
            }
            else
            {
                GPU_chan->word_count--;
                GPU_chan->addr += 4;
                uint32_t data = *(uint32_t*)&RAM[GPU_chan->addr];
                gpu->write_GP0(data);
            }
            break;
    }
}

void DMA::process_OTC()
{
    //In OTC, only three bits are writable - 24, 28, 30. Everything else is static
    DMA_Channel* OTC = &channels[6];
    OTC->word_count--;
    if (!OTC->word_count)
    {
        *(uint32_t*)&RAM[OTC->addr] = 0xFFFFFF;
        end_transfer(6);
    }
    else
    {
        *(uint32_t*)&RAM[OTC->addr] = OTC->addr - 4;
        //printf("[OTC] Write $%08X\n", OTC->addr - 4);
        OTC->addr -= 4;
    }
}

void DMA::end_transfer(int index)
{
    printf("[DMA] %s transfer ended\n", NAMES[index]);
    channels[index].active = false;
    channels[index].busy = false;

    bool old_IRQ = ICR.master_IRQ_enable && (ICR.STAT & ICR.MASK);
    ICR.STAT |= 1 << index;
    bool new_IRQ = ICR.master_IRQ_enable && (ICR.STAT & ICR.MASK);
    if (!old_IRQ && new_IRQ)
        e->request_IRQ(3);
}

uint32_t DMA::read_control(int index)
{
    uint32_t reg = 0;
    reg |= channels[index].transfer_dir;
    reg |= channels[index].step_back << 1;
    reg |= channels[index].chop << 8;
    reg |= channels[index].sync_mode << 9;
    reg |= channels[index].chop_dma_size << 16;
    reg |= channels[index].chop_cpu_size << 20;
    reg |= channels[index].active << 24;
    printf("[DMA] Read %s control: $%08X\n", NAMES[index], reg);
    return reg;
}

void DMA::write_addr(int index, uint32_t value)
{
    printf("[DMA] Write %s addr: $%08X\n", NAMES[index], value);
    channels[index].addr = value & 0xFFFFFF;
}

void DMA::write_block(int index, uint32_t value)
{
    printf("[DMA] Write %s block: $%08X\n", NAMES[index], value);
    channels[index].block = value;
}

void DMA::write_control(int index, uint32_t value)
{
    printf("[DMA] Write %s control: $%08X\n", NAMES[index], value);
    channels[index].transfer_dir = value & 0x1;
    channels[index].step_back = value & (1 << 1);
    channels[index].chop = value & (1 << 8);
    channels[index].sync_mode = (value >> 9) & 0x3;
    channels[index].chop_dma_size = (value >> 16) & 0x7;
    channels[index].chop_cpu_size = (value >> 20) & 0x7;

    if (!channels[index].active && (value & (1 << 24)))
    {
        uint32_t block = channels[index].block;
        switch (channels[index].sync_mode)
        {
            case 0:
                block &= 0xFFFF;
                if (!block)
                    channels[index].word_count = 0x10000;
                else
                    channels[index].word_count = block;
                break;
            case 1:
                channels[index].word_count = (block & 0xFFFF) * (block >> 16);
                break;
            case 2:
                channels[index].word_count = 0;
                channels[index].next_addr = channels[index].addr;
                break;
        }
    }

    channels[index].active = value & (1 << 24);
    channels[index].busy = value & (1 << 28);
}

uint32_t DMA::read_PCR()
{
    printf("[DMA] Read PCR: $%08X\n", PCR);
    return PCR;
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
