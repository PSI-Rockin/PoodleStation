#ifndef TIMERS_HPP
#define TIMERS_HPP
#include <cstdint>

struct Timer
{
    uint32_t count;
    uint16_t target;

    //mode variables
    bool sync;
    uint8_t sync_mode;
    bool reset_on_target;
    bool IRQ_on_target;
    bool IRQ_on_overflow;
    bool IRQ_repeat;
    bool IRQ_toggle;
    uint8_t clock_source;
    bool IRQ;
    bool reached_target;
    bool reached_overflow;
};

class Timers
{
    private:
        Timer timers[3];

        void target_overflow_check(int index, int cycles);
    public:
        Timers();

        void reset();

        void count(int cycles);
        void write16(uint32_t addr, uint16_t value);
};

#endif // TIMERS_HPP
