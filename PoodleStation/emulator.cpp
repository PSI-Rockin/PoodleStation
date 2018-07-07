#include <cstring>
#include "emulator.hpp"

Emulator::Emulator() : cpu(this)
{
    BIOS = nullptr;
}

Emulator::~Emulator()
{
    if (BIOS)
        delete[] BIOS;
}

void Emulator::load_BIOS(uint8_t *BIOS)
{
    if (!this->BIOS)
        this->BIOS = new uint8_t[1024 * 512];
    memcpy(this->BIOS, BIOS, 1024 * 512);
}

void Emulator::reset()
{
    cpu.reset();
    cpu.set_disassembly(true);
}

void Emulator::run()
{
    for (int i = 0; i < 10000; i++)
        cpu.run();
}

void Emulator::get_resolution(int &w, int &h)
{
    w = 640;
    h = 480;
}

uint32_t* Emulator::get_framebuffer()
{
    return nullptr;
}

uint8_t Emulator::read8(uint32_t addr)
{
    printf("[CPU] Unrecognized read8 from $%08X!\n", addr);
    exit(1);
}

uint16_t Emulator::read16(uint32_t addr)
{
    printf("[CPU] Unrecognized read16 from $%08X!\n", addr);
    exit(1);
}

uint32_t Emulator::read32(uint32_t addr)
{
    if (addr >= 0x1FC00000)
        return *(uint32_t*)&BIOS[addr & 0x7FFFF];
    printf("[CPU] Unrecognized read32 from $%08X!\n", addr);
    exit(1);
}

void Emulator::write8(uint32_t addr, uint8_t value)
{
    printf("[CPU] Unrecognized write8 to $%08X of $%02X!\n", addr, value);
    exit(1);
}

void Emulator::write16(uint32_t addr, uint16_t value)
{
    printf("[CPU] Unrecognized write16 to $%08X of $%04X!\n", addr, value);
    exit(1);
}

void Emulator::write32(uint32_t addr, uint32_t value)
{
    printf("[CPU] Unrecognized write32 to $%08X of $%08X!\n", addr, value);
    exit(1);
}
