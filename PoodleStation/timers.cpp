#include <cstdio>
#include "timers.hpp"

Timers::Timers()
{

}

void Timers::reset()
{
    for (int i = 0; i < 3; i++)
    {
        timers[i].count = 0;
        timers[i].target = 0xFFFF;
        write16((i << 4) | 0x4, 0);
    }
}

void Timers::count(int cycles)
{
    if (!timers[0].sync)
    {
        timers[0].count += cycles;
        target_overflow_check(0, cycles);
    }
    if (!timers[1].sync)
    {
        timers[1].count += cycles;
        target_overflow_check(1, cycles);
    }
    if (!timers[2].sync || (timers[2].sync_mode != 0 && timers[2].sync_mode != 3))
    {
        timers[2].count += cycles;
        target_overflow_check(2, cycles);
    }
}

void Timers::target_overflow_check(int index, int cycles)
{
    int count = timers[index].count;
    int target = timers[index].target;
    if ((count - cycles) < target && count >= target)
    {
        timers[index].reached_target = true;
        if (timers[index].reset_on_target)
            timers[index].count -= target;
    }
    if (count >= 0xFFFF)
    {
        timers[index].count -= 0xFFFF;
        timers[index].reached_overflow = true;
    }
}

uint16_t Timers::read16(uint32_t addr)
{
    printf("[Timer] Read16: $%08X\n", addr);
    int index = (addr >> 4) & 0x3;
    int reg = (addr >> 2) & 0x3;
    switch (reg)
    {
        case 0:
            return timers[index].count;
        case 1:
        {
            uint16_t reg = 0;
            reg |= timers[index].sync;
            reg |= timers[index].sync_mode << 1;
            reg |= timers[index].reset_on_target << 3;
            reg |= timers[index].IRQ_on_target << 4;
            reg |= timers[index].IRQ_on_overflow << 5;
            reg |= timers[index].IRQ_repeat << 6;
            reg |= timers[index].IRQ_toggle << 7;
            reg |= timers[index].clock_source << 8;
            reg |= timers[index].IRQ << 10;
            reg |= timers[index].reached_target << 11;
            reg |= timers[index].reached_overflow << 12;
            return reg;
        }
        case 2:
            return timers[index].target;
        default:
            return 0;
    }
}

void Timers::write16(uint32_t addr, uint16_t value)
{
    printf("[Timer] Write16 $%08X: $%04X\n", addr, value);
    int index = (addr >> 4) & 0x3;
    int reg = (addr >> 2) & 0x3;
    switch (reg)
    {
        case 0:
            timers[index].count = 0;
            break;
        case 1:
            timers[index].sync = value & 0x1;
            timers[index].sync_mode = (value >> 1) & 0x3;
            timers[index].reset_on_target = value & (1 << 3);
            timers[index].IRQ_on_target = value & (1 << 4);
            timers[index].IRQ_on_overflow = value & (1 << 5);
            timers[index].IRQ_repeat = value & (1 << 6);
            timers[index].IRQ_toggle = value & (1 << 7);
            timers[index].clock_source = (value >> 8) & 0x3;
            timers[index].IRQ = true;
            break;
        case 2:
            timers[index].target = value;
            break;
    }
}
