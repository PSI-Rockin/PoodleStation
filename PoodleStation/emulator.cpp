#include <cstring>
#include "emulator.hpp"

#define CYCLES_PER_FRAME 550000
#define CYCLES_PER_VBLANK CYCLES_PER_FRAME * 0.90

Emulator::Emulator() : cdrom(this), cpu(this), dma(this, &gpu)
{
    BIOS = nullptr;
    RAM = nullptr;
}

Emulator::~Emulator()
{
    if (RAM)
        delete[] RAM;
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
    if (!RAM)
        RAM = new uint8_t[1024 * 1024 * 2];
    cdrom.reset();
    cpu.reset();
    dma.reset(RAM);
    gpu.reset();
    timers.reset();
    frames = 0;

    I_STAT = 0;
    I_MASK = 0;
}

void Emulator::run()
{
    bool VBLANK_set = false;
    for (int cycles = 0; cycles < CYCLES_PER_FRAME; cycles++)
    {
        if (!dma.run())
            cpu.run();
        cdrom.run();
        timers.count(1);
        if (cycles >= CYCLES_PER_VBLANK && !VBLANK_set)
        {
            printf("VBLANK: %d frames\n", frames);
            //cpu.set_disassembly(frames == 176);
            VBLANK_set = true;
            request_IRQ(0);
            gpu.render_frame();
        }
    }
    gpu.new_frame();
    frames++;
}

void Emulator::request_IRQ(int id)
{
    printf("[Emulator] Requesting IRQ %d...\n", id);
    I_STAT |= 1 << id;
    cpu.interrupt_check(I_STAT & I_MASK);
}

void Emulator::get_resolution(int &w, int &h)
{
    w = 640;
    h = 480;
}

uint32_t* Emulator::get_framebuffer()
{
    return gpu.get_framebuffer();
}

uint8_t Emulator::read8(uint32_t addr)
{
    if (addr < 0x00200000)
        return RAM[addr & 0x1FFFFF];
    if (addr >= 0x1FC00000)
        return BIOS[addr & 0x7FFFF];
    switch (addr)
    {
        case 0x1F000084:
            return 0;
        case 0x1F801040:
            return 0;
        case 0x1F801800:
            return cdrom.read_reg1();
        case 0x1F801801:
            return cdrom.read_reg2();
        case 0x1F801803:
            return cdrom.read_reg4();
    }
    printf("[CPU] Unrecognized read8 from $%08X!\n", addr);
    exit(1);
}

uint16_t Emulator::read16(uint32_t addr)
{
    if (addr < 0x00200000)
        return *(uint16_t*)&RAM[addr & 0x1FFFFF];
    if (addr >= 0x1FC00000)
        return *(uint16_t*)&BIOS[addr & 0x7FFFF];
    if (addr >= 0x1F801100 && addr < 0x1F801130)
        return timers.read16(addr);
    if (addr >= 0x1F801C00 && addr < 0x1F801F00)
    {
        printf("[SPU] Read16 $%08X\n", addr);
        return 0;
    }
    switch (addr)
    {
        case 0x1F801044:
            printf("[PAD] JOY_STAT\n");
            //cpu.set_disassembly(true);
            return 0x7;
        case 0x1F80104A:
            printf("[PAD] JOY_CTRL\n");
            return 0;
        case 0x1F801070:
            return I_STAT;
        case 0x1F801074:
            return I_MASK;
    }
    printf("[CPU] Unrecognized read16 from $%08X!\n", addr);
    exit(1);
}

uint32_t Emulator::read32(uint32_t addr)
{
    if (addr < 0x00200000)
        return *(uint32_t*)&RAM[addr & 0x1FFFFF];
    if (addr >= 0x1FC00000)
        return *(uint32_t*)&BIOS[addr & 0x7FFFF];
    if (addr >= 0x1F801100 && addr < 0x1F801130)
        return timers.read16(addr);
    //printf("[CPU] Read32: $%08X\n", addr);
    switch (addr)
    {
        case 0x1F801070:
            return I_STAT;
        case 0x1F801074:
            return I_MASK;
        case 0x1F8010A8:
            return dma.read_control(2);
        case 0x1F8010E8:
            return dma.read_control(6);
        case 0x1F8010F0:
            return dma.read_PCR();
        case 0x1F8010F4:
            return dma.read_ICR();
        case 0x1F801810:
            return gpu.read_response();
        case 0x1F801814:
            return gpu.read_stat();
    }
    printf("[CPU] Unrecognized read32 from $%08X!\n", addr);
    exit(1);
}

void Emulator::write8(uint32_t addr, uint8_t value)
{
    if (addr < 0x00200000)
    {
        RAM[addr & 0x1FFFFF] = value;
        return;
    }
    switch (addr)
    {
        case 0x1F801040:
            printf("[JOY] Write FIFO: $%02X\n", value);
            return;
        case 0x1F801800:
            cdrom.write_reg1(value);
            return;
        case 0x1F801801:
            cdrom.write_reg2(value);
            return;
        case 0x1F801802:
            cdrom.write_reg3(value);
            return;
        case 0x1F801803:
            cdrom.write_reg4(value);
            return;
        case 0x1F802041:
            printf("[Emulator] POST: $%02X\n", value);
            return;
    }
    printf("[CPU] Unrecognized write8 to $%08X of $%02X!\n", addr, value);
    exit(1);
}

void Emulator::write16(uint32_t addr, uint16_t value)
{
    if (addr < 0x00200000)
    {
        *(uint16_t*)&RAM[addr & 0x1FFFFF] = value;
        return;
    }
    if (addr >= 0x1F801040 && addr < 0x1F801050)
    {
        printf("[JOY] Write16 $%08X: $%04X\n", addr, value);
        return;
    }
    if (addr >= 0x1F801100 && addr < 0x1F801130)
    {
        timers.write16(addr, value);
        return;
    }
    if (addr >= 0x1F801C00 && addr < 0x1F801F00)
    {
        printf("[SPU] Write16 $%08X: $%04X\n", addr, value);
        return;
    }
    switch (addr)
    {
        case 0x1F801070:
            printf("[Emulator] Write I_STAT: $%04X\n", value);
            I_STAT &= value;
            cpu.interrupt_check(I_STAT & I_MASK);
            return;
        case 0x1F801074:
            printf("[Emulator] Write I_MASK: $%04X\n", value);
            I_MASK = value;
            cpu.interrupt_check(I_STAT & I_MASK);
            return;
    }
    printf("[CPU] Unrecognized write16 to $%08X of $%04X!\n", addr, value);
    exit(1);
}

void Emulator::write32(uint32_t addr, uint32_t value)
{
    if (addr >= 0x00138CFC && addr < 0x00138D00)
        printf("Write32 special $%08X: $%08X\n", addr, value);
    if (addr < 0x00200000)
    {
        *(uint32_t*)&RAM[addr & 0x1FFFFF] = value;
        return;
    }
    if (addr >= 0x1F801000 && addr <= 0x1F801020)
        return;
    if (addr >= 0x1F801100 && addr < 0x1F801130)
    {
        timers.write16(addr, value);
        return;
    }
    switch (addr)
    {
        case 0x1F801060:
            return;
        case 0x1F801070:
            printf("[Emulator] Write I_STAT: $%08X\n", value);
            I_STAT &= value;
            cpu.interrupt_check(I_STAT & I_MASK);
            return;
        case 0x1F801074:
            printf("[Emulator] Write I_MASK: $%08X\n", value);
            I_MASK = value;
            cpu.interrupt_check(I_STAT & I_MASK);
            return;
        case 0x1F8010A0:
            dma.write_addr(2, value);
            return;
        case 0x1F8010A4:
            dma.write_block(2, value);
            return;
        case 0x1F8010A8:
            dma.write_control(2, value);
            return;
        case 0x1F8010E0:
            dma.write_addr(6, value);
            return;
        case 0x1F8010E4:
            dma.write_block(6, value);
            return;
        case 0x1F8010E8:
            dma.write_control(6, value);
            return;
        case 0x1F8010F0:
            dma.write_PCR(value);
            return;
        case 0x1F8010F4:
            dma.write_ICR(value);
            return;
        case 0x1F801810:
            gpu.write_GP0(value);
            return;
        case 0x1F801814:
            gpu.write_GP1(value);
            return;
        case 0xFFFE0130:
            return;
    }
    printf("[CPU] Unrecognized write32 to $%08X of $%08X!\n", addr, value);
    exit(1);
}
