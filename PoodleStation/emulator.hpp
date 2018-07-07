#ifndef EMULATOR_HPP
#define EMULATOR_HPP
#include <cstdint>
#include "cpu.hpp"

class Emulator
{
    private:
        uint8_t* BIOS;

        CPU cpu;
    public:
        Emulator();
        ~Emulator();

        void load_BIOS(uint8_t* BIOS);
        void reset();
        void run();

        void get_resolution(int& w, int& h);
        uint32_t* get_framebuffer();

        uint8_t read8(uint32_t addr);
        uint16_t read16(uint32_t addr);
        uint32_t read32(uint32_t addr);

        void write8(uint32_t addr, uint8_t value);
        void write16(uint32_t addr, uint16_t value);
        void write32(uint32_t addr, uint32_t value);
};

#endif // EMULATOR_HPP
