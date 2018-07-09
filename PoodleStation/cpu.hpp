#ifndef CPU_HPP
#define CPU_HPP

#include <cstdint>
#include <cstdlib>
#include <cstdio>
#include "cop0.hpp"
#include "gte.hpp"

class Emulator;

class CPU
{
    private:
        Emulator* e;
        Cop0 cop0;
        GTE gte; //cop2
        uint32_t gpr[32];
        uint32_t PC;
        uint32_t LO, HI;

        uint32_t new_PC;
        int load_delay;
        bool can_disassemble;
        bool will_branch;
        bool inc_PC;

        uint32_t translate_addr(uint32_t addr);
    public:
        CPU(Emulator* e);
        static const char* REG(int id);

        void reset();
        void run();
        void print_state();
        void set_disassembly(bool dis);

        void jp(uint32_t addr);
        void branch(bool condition, int32_t offset);

        void handle_exception(uint32_t addr, uint8_t cause);
        void syscall_exception();
        void interrupt_check(bool i_pass);
        void interrupt();
        void rfe();

        void mfc(int cop_id, int cop_reg, int reg);
        void mtc(int cop_id, int cop_reg, int reg);
        void ctc(int cop_id, int cop_reg, int reg);

        uint32_t get_PC();
        uint32_t get_gpr(int index);
        uint32_t get_LO();
        uint32_t get_HI();
        void set_PC(uint32_t value);
        void set_gpr(int index, uint32_t value);
        void set_LO(uint32_t value);
        void set_HI(uint32_t value);

        uint8_t read8(uint32_t addr);
        uint16_t read16(uint32_t addr);
        uint32_t read32(uint32_t addr);
        void write8(uint32_t addr, uint8_t value);
        void write16(uint32_t addr, uint16_t value);
        void write32(uint32_t addr, uint32_t value);
};

inline uint32_t CPU::get_PC()
{
    return PC;
}

inline uint32_t CPU::get_gpr(int index)
{
    return gpr[index];
}

inline uint32_t CPU::get_LO()
{
    return LO;
}

inline uint32_t CPU::get_HI()
{
    return HI;
}

inline void CPU::set_PC(uint32_t value)
{
    PC = value;
}

inline void CPU::set_gpr(int index, uint32_t value)
{
    if (index)
        gpr[index] = value;
}

inline void CPU::set_LO(uint32_t value)
{
    LO = value;
}

inline void CPU::set_HI(uint32_t value)
{
    HI = value;
}

#endif // CPU_HPP
