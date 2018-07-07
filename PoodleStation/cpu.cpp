#include <cstdio>
#include <cstdlib>
#include "cpu.hpp"
#include "disasm.hpp"
#include "emulator.hpp"
#include "interpreter.hpp"

CPU::CPU(Emulator* e) : e(e)
{

}

const char* CPU::REG(int id)
{
    static const char* names[] =
    {
        "zero", "at", "v0", "v1",
        "a0", "a1", "a2", "a3",
        "t0", "t1", "t2", "t3",
        "t4", "t5", "t6", "t7",
        "s0", "s1", "s2", "s3",
        "s4", "s5", "s6", "s7",
        "t8", "t9", "k0", "k1",
        "gp", "sp", "fp", "ra"
    };
    return names[id];
}

void CPU::reset()
{
    cop0.reset();
    PC = 0xBFC00000;
    gpr[0] = 0;
    load_delay = 0;
    will_branch = false;
    inc_PC = true;
    can_disassemble = false;
}

uint32_t CPU::translate_addr(uint32_t addr)
{
    //KSEG0
    if (addr >= 0x80000000 && addr < 0xA0000000)
        return addr - 0x80000000;
    //KSEG1
    if (addr >= 0xA0000000 && addr < 0xC0000000)
        return addr - 0xA0000000;

    //KUSEG, KSEG2
    return addr;
}

void CPU::run()
{
    //bool old_int = cop0.status.IEc && (cop0.status.Im & cop0.cause.int_pending);
    uint32_t instr = read32(PC);
    if (can_disassemble)
    {
        printf("[CPU] [$%08X] $%08X - %s\n", PC, instr, Disasm::disasm_instr(instr, PC).c_str());
        //print_state();
    }
    Interpreter::interpret(*this, instr);

    if (inc_PC)
        PC += 4;
    else
        inc_PC = true;

    if (will_branch)
    {
        if (!load_delay)
        {
            will_branch = false;
            PC = new_PC;
            if (PC & 0x3)
            {
                printf("[CPU] Invalid PC address $%08X!\n", PC);
                exit(1);
            }
            if (PC == 0xA0 || PC == 0xB0 || PC == 0xC0)
            {
                uint8_t function = get_gpr(9);
                if (!(PC == 0xB0 && function == 0x3D))
                    printf("[CPU] Jump to function table $%02X (function: $%02X)\n", PC, function);

                else
                    printf("%c", get_gpr(4));
            }
        }
        else
            load_delay--;
    }

    if (cop0.status.IEc && (cop0.status.Im & cop0.cause.int_pending))
        interrupt();
}

void CPU::print_state()
{
    for (int i = 1; i < 32; i++)
    {
        printf("%s:$%08X", REG(i), get_gpr(i));
        if (i % 4 == 3)
            printf("\n");
        else
            printf("\t");
    }
}

void CPU::set_disassembly(bool dis)
{
    can_disassemble = dis;
}

void CPU::jp(uint32_t addr)
{
    if (!will_branch)
    {
        new_PC = addr;
        will_branch = true;
        load_delay = 1;
    }
}

void CPU::branch(bool condition, int32_t offset)
{
    if (condition)
        jp(PC + offset + 4);
}

void CPU::handle_exception(uint32_t addr, uint8_t cause)
{
    inc_PC = false;
    cop0.cause.code = cause;
    if (will_branch)
    {
        cop0.EPC = PC - 4;
        cop0.cause.bd = true;
    }
    else
    {
        cop0.EPC = PC;
        cop0.cause.bd = false;
    }
    cop0.status.IEo = cop0.status.IEp;
    cop0.status.IEp = cop0.status.IEc;
    cop0.status.IEc = false;
    PC = addr;
    load_delay = 0;
    will_branch = false;
}

void CPU::syscall_exception()
{
    uint8_t op = read8(PC - 4);
    printf("[CPU] SYSCALL: $%02X\n", op);
    handle_exception(0x80000080, 0x08);
    //can_disassemble = true;
}

void CPU::interrupt_check(bool i_pass)
{
    if (i_pass)
        cop0.cause.int_pending |= 0x4;
    else
        cop0.cause.int_pending &= ~0x4;

    /*if (!edge && i_pass true)
    {
        if (cop0.status.IEc && (cop0.cause.int_pending & cop0.status.Im))
            interrupt();
    }*/
}

void CPU::interrupt()
{
    printf("[CPU] Processing interrupt!\n");
    handle_exception(0x80000080, 0x00);
    //can_disassemble = true;
}

void CPU::mfc(int cop_id, int cop_reg, int reg)
{
    switch (cop_id)
    {
        case 0:
            set_gpr(reg, cop0.mfc(cop_reg));
            break;
        default:
            printf("\n[CPU] MFC: Unknown COP%d", cop_id);
            exit(1);
    }
}

void CPU::mtc(int cop_id, int cop_reg, int reg)
{
    uint32_t bark = get_gpr(reg);
    switch (cop_id)
    {
        case 0:
            cop0.mtc(cop_reg, bark);
            break;
        default:
            printf("\n[CPU] MTC: Unknown COP%d", cop_id);
            exit(1);
    }
}

void CPU::rfe()
{
    cop0.status.KUc = cop0.status.KUp;
    cop0.status.KUp = cop0.status.KUo;

    cop0.status.IEc = cop0.status.IEp;
    cop0.status.IEp = cop0.status.IEo;
    printf("[CPU] RFE!\n");
    //can_disassemble = false;
}

uint8_t CPU::read8(uint32_t addr)
{
    return e->read8(translate_addr(addr));
}

uint16_t CPU::read16(uint32_t addr)
{
    if (addr & 0x1)
    {
        printf("[CPU] Invalid read16 from $%08X!\n", addr);
        exit(1);
    }
    return e->read16(translate_addr(addr));
}

uint32_t CPU::read32(uint32_t addr)
{
    if (addr & 0x3)
    {
        printf("[CPU] Invalid read32 from $%08X!\n", addr);
        exit(1);
    }
    return e->read32(translate_addr(addr));
}

void CPU::write8(uint32_t addr, uint8_t value)
{
    if (cop0.status.IsC)
        return;
    e->write8(translate_addr(addr), value);
}

void CPU::write16(uint32_t addr, uint16_t value)
{
    if (cop0.status.IsC)
        return;
    if (addr & 0x1)
    {
        printf("[CPU] Invalid write16 to $%08X!\n", addr);
        exit(1);
    }
    e->write16(translate_addr(addr), value);
}

void CPU::write32(uint32_t addr, uint32_t value)
{
    if (cop0.status.IsC)
        return;
    if (addr & 0x3)
    {
        printf("[CPU] Invalid write32 to $%08X!\n", addr);
        exit(1);
    }
    e->write32(translate_addr(addr), value);
}
