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

void Timers::write16(uint32_t addr, uint16_t value)
{
    printf("[Timer] Write16 $%08X: $%04X\n", addr, value);
    int index = (addr >> 4) & 0x3;
    int reg = addr >> 2;
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
