#include <cstdio>
#include <cstdlib>
#include "interpreter.hpp"

void Interpreter::interpret(CPU &cpu, uint32_t instruction)
{
    if (!instruction)
        return;
    int op = instruction >> 26;
    switch (op)
    {
        case 0x00:
            special(cpu, instruction);
            break;
        case 0x01:
            regimm(cpu, instruction);
            break;
        case 0x02:
            j(cpu, instruction);
            break;
        case 0x03:
            jal(cpu, instruction);
            break;
        case 0x04:
            beq(cpu, instruction);
            break;
        case 0x05:
            bne(cpu, instruction);
            break;
        case 0x06:
            blez(cpu, instruction);
            break;
        case 0x07:
            bgtz(cpu, instruction);
            break;
        case 0x08:
            addi(cpu, instruction);
            break;
        case 0x09:
            addiu(cpu, instruction);
            break;
        case 0x0A:
            slti(cpu, instruction);
            break;
        case 0x0B:
            sltiu(cpu, instruction);
            break;
        case 0x0C:
            andi(cpu, instruction);
            break;
        case 0x0D:
            ori(cpu, instruction);
            break;
        case 0x0E:
            xori(cpu, instruction);
            break;
        case 0x0F:
            lui(cpu, instruction);
            break;
        case 0x10:
        case 0x11:
        case 0x12:
        case 0x13:
            cop(cpu, instruction);
            break;
        case 0x20:
            lb(cpu, instruction);
            break;
        case 0x21:
            lh(cpu, instruction);
            break;
        case 0x22:
            lwl(cpu, instruction);
            break;
        case 0x23:
            lw(cpu, instruction);
            break;
        case 0x24:
            lbu(cpu, instruction);
            break;
        case 0x25:
            lhu(cpu, instruction);
            break;
        case 0x26:
            lwr(cpu, instruction);
            break;
        case 0x28:
            sb(cpu, instruction);
            break;
        case 0x29:
            sh(cpu, instruction);
            break;
        case 0x2A:
            swl(cpu, instruction);
            break;
        case 0x2B:
            sw(cpu, instruction);
            break;
        case 0x2E:
            swr(cpu, instruction);
            break;
        default:
            unknown_op("regular", op, instruction);
    }
}

void Interpreter::j(CPU &cpu, uint32_t instruction)
{
    uint32_t addr = (instruction & 0x3FFFFFF) << 2;
    uint32_t PC = cpu.get_PC();
    addr += (PC + 4) & 0xF0000000;
    cpu.jp(addr);
}

void Interpreter::jal(CPU &cpu, uint32_t instruction)
{
    uint32_t addr = (instruction & 0x3FFFFFF) << 2;
    uint32_t PC = cpu.get_PC();
    addr += (PC + 4) & 0xF0000000;
    cpu.jp(addr);
    cpu.set_gpr(31, PC + 8);
}

void Interpreter::beq(CPU &cpu, uint32_t instruction)
{
    int offset = (int16_t)(instruction & 0xFFFF);
    offset <<= 2;
    uint32_t reg1 = cpu.get_gpr((instruction >> 21) & 0x1F);
    uint32_t reg2 = cpu.get_gpr((instruction >> 16) & 0x1F);
    cpu.branch(reg1 == reg2, offset);
}

void Interpreter::bne(CPU &cpu, uint32_t instruction)
{
    int offset = (int16_t)(instruction & 0xFFFF);
    offset <<= 2;
    uint32_t reg1 = cpu.get_gpr((instruction >> 21) & 0x1F);
    uint32_t reg2 = cpu.get_gpr((instruction >> 16) & 0x1F);
    cpu.branch(reg1 != reg2, offset);
}

void Interpreter::blez(CPU &cpu, uint32_t instruction)
{
    int offset = (int16_t)(instruction & 0xFFFF);
    offset <<= 2;
    int32_t reg = (int32_t)cpu.get_gpr((instruction >> 21) & 0x1F);
    cpu.branch(reg <= 0, offset);
}

void Interpreter::bgtz(CPU &cpu, uint32_t instruction)
{
    int offset = (int16_t)(instruction & 0xFFFF);
    offset <<= 2;
    int32_t reg = (int32_t)cpu.get_gpr((instruction >> 21) & 0x1F);
    cpu.branch(reg > 0, offset);
}

void Interpreter::addi(CPU &cpu, uint32_t instruction)
{
    int16_t imm = (int16_t)(instruction & 0xFFFF);
    uint32_t dest = (instruction >> 16) & 0x1F;
    uint32_t source = (instruction >> 21) & 0x1F;
    uint32_t result = cpu.get_gpr(source);
    result += imm;
    cpu.set_gpr(dest, result);
}

void Interpreter::addiu(CPU &cpu, uint32_t instruction)
{
    int16_t imm = (int16_t)(instruction & 0xFFFF);
    uint32_t dest = (instruction >> 16) & 0x1F;
    uint32_t source = (instruction >> 21) & 0x1F;
    uint32_t result = cpu.get_gpr(source);
    result += imm;
    cpu.set_gpr(dest, result);
}

void Interpreter::slti(CPU &cpu, uint32_t instruction)
{
    int32_t imm = (int32_t)(int16_t)(instruction & 0xFFFF);
    uint32_t dest = (instruction >> 16) & 0x1F;
    int32_t source = (instruction >> 21) & 0x1F;
    source = (int32_t)cpu.get_gpr(source);
    cpu.set_gpr(dest, source < imm);
}

void Interpreter::sltiu(CPU &cpu, uint32_t instruction)
{
    uint32_t imm = (uint32_t)(int32_t)(int16_t)(instruction & 0xFFFF);
    uint32_t dest = (instruction >> 16) & 0x1F;
    uint32_t source = (instruction >> 21) & 0x1F;
    source = cpu.get_gpr(source);
    cpu.set_gpr(dest, source < imm);
}

void Interpreter::andi(CPU &cpu, uint32_t instruction)
{
    uint32_t imm = instruction & 0xFFFF;
    uint32_t dest = (instruction >> 16) & 0x1F;
    uint32_t source = (instruction >> 21) & 0x1F;
    cpu.set_gpr(dest, cpu.get_gpr(source) & imm);
}

void Interpreter::ori(CPU &cpu, uint32_t instruction)
{
    uint32_t imm = instruction & 0xFFFF;
    uint32_t dest = (instruction >> 16) & 0x1F;
    uint32_t source = (instruction >> 21) & 0x1F;
    cpu.set_gpr(dest, cpu.get_gpr(source) | imm);
}

void Interpreter::xori(CPU &cpu, uint32_t instruction)
{
    uint32_t imm = instruction & 0xFFFF;
    uint32_t dest = (instruction >> 16) & 0x1F;
    uint32_t source = (instruction >> 21) & 0x1F;
    cpu.set_gpr(dest, cpu.get_gpr(source) ^ imm);
}

void Interpreter::lui(CPU &cpu, uint32_t instruction)
{
    uint32_t imm = (instruction & 0xFFFF) << 16;
    uint32_t dest = (instruction >> 16) & 0x1F;
    cpu.set_gpr(dest, imm);
}

void Interpreter::lb(CPU &cpu, uint32_t instruction)
{
    int16_t offset = (int16_t)(instruction & 0xFFFF);
    uint32_t dest = (instruction >> 16) & 0x1F;
    uint32_t base = (instruction >> 21) & 0x1F;
    uint32_t addr = cpu.get_gpr(base);
    addr += offset;
    cpu.set_gpr(dest, (int32_t)(int8_t)cpu.read8(addr));
}

void Interpreter::lh(CPU &cpu, uint32_t instruction)
{
    int16_t offset = (int16_t)(instruction & 0xFFFF);
    uint32_t dest = (instruction >> 16) & 0x1F;
    uint32_t base = (instruction >> 21) & 0x1F;
    uint32_t addr = cpu.get_gpr(base);
    addr += offset;
    cpu.set_gpr(dest, (int32_t)(int16_t)cpu.read16(addr));
}

void Interpreter::lwl(CPU &cpu, uint32_t instruction)
{
    static const uint32_t LWL_MASK[4] = { 0xffffff, 0x0000ffff, 0x000000ff, 0x00000000 };
    static const uint8_t LWL_SHIFT[4] = { 24, 16, 8, 0 };
    int16_t offset = (int16_t)(instruction & 0xFFFF);
    uint32_t dest = (instruction >> 16) & 0x1F;
    uint32_t base = (instruction >> 21) & 0x1F;
    uint32_t addr = cpu.get_gpr(base) + offset;
    int shift = addr & 0x3;

    uint32_t mem = cpu.read32(addr & ~0x3);
    cpu.set_gpr(dest, (cpu.get_gpr(dest) & LWL_MASK[shift]) | (mem << LWL_SHIFT[shift]));
}

void Interpreter::lwr(CPU &cpu, uint32_t instruction)
{
    static const uint32_t LWR_MASK[4] = { 0x000000, 0xff000000, 0xffff0000, 0xffffff00 };
    static const uint8_t LWR_SHIFT[4] = { 0, 8, 16, 24 };
    int16_t offset = (int16_t)(instruction & 0xFFFF);
    uint32_t dest = (instruction >> 16) & 0x1F;
    uint32_t base = (instruction >> 21) & 0x1F;
    uint32_t addr = cpu.get_gpr(base) + offset;
    int shift = addr & 0x3;

    uint32_t mem = cpu.read32(addr & ~0x3);
    mem = (cpu.get_gpr(dest) & LWR_MASK[shift]) | (mem >> LWR_SHIFT[shift]);
    cpu.set_gpr(dest, mem);
}

void Interpreter::lw(CPU &cpu, uint32_t instruction)
{
    int16_t offset = (int16_t)(instruction & 0xFFFF);
    uint32_t dest = (instruction >> 16) & 0x1F;
    uint32_t base = (instruction >> 21) & 0x1F;
    uint32_t addr = cpu.get_gpr(base);
    addr += offset;
    cpu.set_gpr(dest, cpu.read32(addr));
}

void Interpreter::lbu(CPU &cpu, uint32_t instruction)
{
    int16_t offset = (int16_t)(instruction & 0xFFFF);
    uint32_t dest = (instruction >> 16) & 0x1F;
    uint32_t base = (instruction >> 21) & 0x1F;
    uint32_t addr = cpu.get_gpr(base);
    addr += offset;
    cpu.set_gpr(dest, cpu.read8(addr));
}

void Interpreter::lhu(CPU &cpu, uint32_t instruction)
{
    int16_t offset = (int16_t)(instruction & 0xFFFF);
    uint32_t dest = (instruction >> 16) & 0x1F;
    uint32_t base = (instruction >> 21) & 0x1F;
    uint32_t addr = cpu.get_gpr(base);
    addr += offset;
    cpu.set_gpr(dest, cpu.read16(addr));
}

void Interpreter::sb(CPU &cpu, uint32_t instruction)
{
    int16_t offset = (int16_t)(instruction & 0xFFFF);
    uint32_t source = (instruction >> 16) & 0x1F;
    uint32_t base = (instruction >> 21) & 0x1F;
    uint32_t addr = cpu.get_gpr(base);
    addr += offset;
    cpu.write8(addr, cpu.get_gpr(source) & 0xFF);
}

void Interpreter::sh(CPU &cpu, uint32_t instruction)
{
    int16_t offset = (int16_t)(instruction & 0xFFFF);
    uint32_t source = (instruction >> 16) & 0x1F;
    uint32_t base = (instruction >> 21) & 0x1F;
    uint32_t addr = cpu.get_gpr(base);
    addr += offset;
    cpu.write16(addr, cpu.get_gpr(source) & 0xFFFF);
}

void Interpreter::swl(CPU &cpu, uint32_t instruction)
{
    static const uint32_t SWL_MASK[4] = { 0xffffff00, 0xffff0000, 0xff000000, 0x00000000 };
    static const uint8_t SWL_SHIFT[4] = { 24, 16, 8, 0 };
    int16_t offset = (int16_t)(instruction & 0xFFFF);
    uint32_t source = (instruction >> 16) & 0x1F;
    uint32_t base = (instruction >> 21) & 0x1F;
    uint32_t addr = cpu.get_gpr(base) + offset;
    int shift = addr & 3;
    uint32_t mem = cpu.read32(addr & ~3);

    cpu.write32(addr & ~0x3, (cpu.get_gpr(source) >> SWL_SHIFT[shift]) | (mem & SWL_MASK[shift]));
}

void Interpreter::sw(CPU &cpu, uint32_t instruction)
{
    int16_t offset = (int16_t)(instruction & 0xFFFF);
    uint32_t source = (instruction >> 16) & 0x1F;
    uint32_t base = (instruction >> 21) & 0x1F;
    uint32_t addr = cpu.get_gpr(base);
    addr += offset;
    cpu.write32(addr, cpu.get_gpr(source));
}

void Interpreter::swr(CPU &cpu, uint32_t instruction)
{
    static const uint32_t SWR_MASK[4] = { 0x00000000, 0x000000ff, 0x0000ffff, 0x00ffffff };
    static const uint8_t SWR_SHIFT[4] = { 0, 8, 16, 24 };
    int16_t offset = (int16_t)(instruction & 0xFFFF);
    uint32_t source = (instruction >> 16) & 0x1F;
    uint32_t base = (instruction >> 21) & 0x1F;
    uint32_t addr = cpu.get_gpr(base) + offset;
    int shift = addr & 3;
    uint32_t mem = cpu.read32(addr & ~3);

    cpu.write32(addr & ~0x3, (cpu.get_gpr(source) << SWR_SHIFT[shift]) | (mem & SWR_MASK[shift]));
}

void Interpreter::special(CPU &cpu, uint32_t instruction)
{
    int op = instruction & 0x3F;
    switch (op)
    {
        case 0x00:
            sll(cpu, instruction);
            break;
        case 0x02:
            srl(cpu, instruction);
            break;
        case 0x03:
            sra(cpu, instruction);
            break;
        case 0x04:
            sllv(cpu, instruction);
            break;
        case 0x06:
            srlv(cpu, instruction);
            break;
        case 0x07:
            srav(cpu, instruction);
            break;
        case 0x08:
            jr(cpu, instruction);
            break;
        case 0x09:
            jalr(cpu, instruction);
            break;
        case 0x0C:
            syscall(cpu, instruction);
            break;
        case 0x10:
            mfhi(cpu, instruction);
            break;
        case 0x11:
            mthi(cpu, instruction);
            break;
        case 0x12:
            mflo(cpu, instruction);
            break;
        case 0x13:
            mtlo(cpu, instruction);
            break;
        case 0x18:
            mult(cpu, instruction);
            break;
        case 0x19:
            multu(cpu, instruction);
            break;
        case 0x1A:
            div(cpu, instruction);
            break;
        case 0x1B:
            divu(cpu, instruction);
            break;
        case 0x20:
            add(cpu, instruction);
            break;
        case 0x21:
            addu(cpu, instruction);
            break;
        case 0x22:
            sub(cpu, instruction);
            break;
        case 0x23:
            subu(cpu, instruction);
            break;
        case 0x24:
            and_cpu(cpu, instruction);
            break;
        case 0x25:
            or_cpu(cpu, instruction);
            break;
        case 0x26:
            xor_cpu(cpu, instruction);
            break;
        case 0x27:
            nor(cpu, instruction);
            break;
        case 0x2A:
            slt(cpu, instruction);
            break;
        case 0x2B:
            sltu(cpu, instruction);
            break;
        default:
            unknown_op("special", op, instruction);
    }
}

void Interpreter::sll(CPU &cpu, uint32_t instruction)
{
    uint32_t source = (instruction >> 16) & 0x1F;
    uint32_t dest = (instruction >> 11) & 0x1F;
    uint32_t shift = (instruction >> 6) & 0x1F;
    source = cpu.get_gpr(source);
    source <<= shift;
    cpu.set_gpr(dest, source);
}

void Interpreter::srl(CPU &cpu, uint32_t instruction)
{
    uint32_t source = (instruction >> 16) & 0x1F;
    uint32_t dest = (instruction >> 11) & 0x1F;
    uint32_t shift = (instruction >> 6) & 0x1F;
    source = cpu.get_gpr(source);
    source >>= shift;
    cpu.set_gpr(dest, source);
}

void Interpreter::sra(CPU &cpu, uint32_t instruction)
{
    int32_t source = (instruction >> 16) & 0x1F;
    uint32_t dest = (instruction >> 11) & 0x1F;
    uint32_t shift = (instruction >> 6) & 0x1F;
    source = (int32_t)cpu.get_gpr(source);
    source >>= shift;
    cpu.set_gpr(dest, (uint32_t)source);
}

void Interpreter::sllv(CPU &cpu, uint32_t instruction)
{
    uint32_t source = (instruction >> 16) & 0x1F;
    uint32_t dest = (instruction >> 11) & 0x1F;
    uint32_t shift = (instruction >> 21) & 0x1F;
    source = cpu.get_gpr(source);
    source <<= cpu.get_gpr(shift) & 0x1F;
    cpu.set_gpr(dest, source);
}

void Interpreter::srlv(CPU &cpu, uint32_t instruction)
{
    uint32_t source = (instruction >> 16) & 0x1F;
    uint32_t dest = (instruction >> 11) & 0x1F;
    uint32_t shift = (instruction >> 21) & 0x1F;
    source = cpu.get_gpr(source);
    source >>= cpu.get_gpr(shift) & 0x1F;
    cpu.set_gpr(dest, source);
}

void Interpreter::srav(CPU &cpu, uint32_t instruction)
{
    int32_t source = (instruction >> 16) & 0x1F;
    uint32_t dest = (instruction >> 11) & 0x1F;
    uint32_t shift = (instruction >> 21) & 0x1F;
    source = (int32_t)cpu.get_gpr(source);
    source >>= cpu.get_gpr(shift) & 0x1F;
    cpu.set_gpr(dest, (uint32_t)source);
}

void Interpreter::jr(CPU &cpu, uint32_t instruction)
{
    uint32_t address = (instruction >> 21) & 0x1F;
    cpu.jp(cpu.get_gpr(address));
}

void Interpreter::jalr(CPU &cpu, uint32_t instruction)
{
    uint32_t new_addr = (instruction >> 21) & 0x1F;
    uint32_t return_reg = (instruction >> 11) & 0x1F;
    uint32_t return_addr = cpu.get_PC() + 8;
    cpu.jp(cpu.get_gpr(new_addr));
    cpu.set_gpr(return_reg, return_addr);
}

void Interpreter::syscall(CPU &cpu, uint32_t instruction)
{
    cpu.syscall_exception();
}

void Interpreter::mfhi(CPU &cpu, uint32_t instruction)
{
    uint32_t dest = (instruction >> 11) & 0x1F;
    cpu.set_gpr(dest, cpu.get_HI());
}

void Interpreter::mthi(CPU &cpu, uint32_t instruction)
{
    uint32_t source = (instruction >> 21) & 0x1F;
    cpu.set_HI(cpu.get_gpr(source));
}

void Interpreter::mflo(CPU &cpu, uint32_t instruction)
{
    uint32_t dest = (instruction >> 11) & 0x1F;
    cpu.set_gpr(dest, cpu.get_LO());
}

void Interpreter::mtlo(CPU &cpu, uint32_t instruction)
{
    uint32_t source = (instruction >> 21) & 0x1F;
    cpu.set_LO(cpu.get_gpr(source));
}

void Interpreter::mult(CPU &cpu, uint32_t instruction)
{
    int64_t op1 = (instruction >> 21) & 0x1F;
    int64_t op2 = (instruction >> 16) & 0x1F;
    op1 = (int32_t)cpu.get_gpr(op1);
    op2 = (int32_t)cpu.get_gpr(op2);
    int64_t temp = op1 * op2;
    cpu.set_LO(temp & 0xFFFFFFFF);
    cpu.set_HI(temp >> 32);
}

void Interpreter::multu(CPU &cpu, uint32_t instruction)
{
    uint64_t op1 = (instruction >> 21) & 0x1F;
    uint64_t op2 = (instruction >> 16) & 0x1F;
    op1 = cpu.get_gpr(op1);
    op2 = cpu.get_gpr(op2);
    uint64_t temp = op1 * op2;
    cpu.set_LO(temp & 0xFFFFFFFF);
    cpu.set_HI(temp >> 32);
}

void Interpreter::div(CPU &cpu, uint32_t instruction)
{
    int32_t op1 = (instruction >> 21) & 0x1F;
    int32_t op2 = (instruction >> 16) & 0x1F;
    op1 = (int32_t)cpu.get_gpr(op1);
    op2 = (int32_t)cpu.get_gpr(op2);
    if (!op2)
    {
        cpu.set_HI(op1);
        if (op1 < 0)
            cpu.set_LO(1);
        else
            cpu.set_LO(0xFFFFFFFF);
    }
    else if (op1 == 0x80000000 && op2 == 0xFFFFFFFF)
    {
        cpu.set_LO(0x80000000);
        cpu.set_HI(0);
    }
    else
    {
        cpu.set_LO(op1 / op2);
        cpu.set_HI(op1 % op2);
    }
}

void Interpreter::divu(CPU &cpu, uint32_t instruction)
{
    uint32_t op1 = (instruction >> 21) & 0x1F;
    uint32_t op2 = (instruction >> 16) & 0x1F;
    op1 = cpu.get_gpr(op1);
    op2 = cpu.get_gpr(op2);
    if (!op2)
    {
        cpu.set_LO(0xFFFFFFFF);
        cpu.set_HI(op1);
    }
    else
    {
        cpu.set_LO(op1 / op2);
        cpu.set_HI(op1 % op2);
    }
}

void Interpreter::add(CPU &cpu, uint32_t instruction)
{
    uint32_t op1 = (instruction >> 21) & 0x1F;
    uint32_t op2 = (instruction >> 16) & 0x1F;
    uint32_t dest = (instruction >> 11) & 0x1F;
    op1 = cpu.get_gpr(op1);
    op2 = cpu.get_gpr(op2);
    cpu.set_gpr(dest, op1 + op2);
}

void Interpreter::addu(CPU &cpu, uint32_t instruction)
{
    uint32_t op1 = (instruction >> 21) & 0x1F;
    uint32_t op2 = (instruction >> 16) & 0x1F;
    uint32_t dest = (instruction >> 11) & 0x1F;
    op1 = cpu.get_gpr(op1);
    op2 = cpu.get_gpr(op2);
    cpu.set_gpr(dest, op1 + op2);
}

void Interpreter::sub(CPU &cpu, uint32_t instruction)
{
    uint32_t op1 = (instruction >> 21) & 0x1F;
    uint32_t op2 = (instruction >> 16) & 0x1F;
    uint32_t dest = (instruction >> 11) & 0x1F;
    op1 = cpu.get_gpr(op1);
    op2 = cpu.get_gpr(op2);
    cpu.set_gpr(dest, op1 - op2);
}

void Interpreter::subu(CPU &cpu, uint32_t instruction)
{
    uint32_t op1 = (instruction >> 21) & 0x1F;
    uint32_t op2 = (instruction >> 16) & 0x1F;
    uint32_t dest = (instruction >> 11) & 0x1F;
    op1 = cpu.get_gpr(op1);
    op2 = cpu.get_gpr(op2);
    cpu.set_gpr(dest, op1 - op2);
}

void Interpreter::and_cpu(CPU &cpu, uint32_t instruction)
{
    uint32_t op1 = (instruction >> 21) & 0x1F;
    uint32_t op2 = (instruction >> 16) & 0x1F;
    uint32_t dest = (instruction >> 11) & 0x1F;
    op1 = cpu.get_gpr(op1);
    op2 = cpu.get_gpr(op2);
    cpu.set_gpr(dest, op1 & op2);
}

void Interpreter::or_cpu(CPU &cpu, uint32_t instruction)
{
    uint32_t op1 = (instruction >> 21) & 0x1F;
    uint32_t op2 = (instruction >> 16) & 0x1F;
    uint32_t dest = (instruction >> 11) & 0x1F;
    op1 = cpu.get_gpr(op1);
    op2 = cpu.get_gpr(op2);
    cpu.set_gpr(dest, op1 | op2);
}

void Interpreter::xor_cpu(CPU &cpu, uint32_t instruction)
{
    uint32_t op1 = (instruction >> 21) & 0x1F;
    uint32_t op2 = (instruction >> 16) & 0x1F;
    uint32_t dest = (instruction >> 11) & 0x1F;
    op1 = cpu.get_gpr(op1);
    op2 = cpu.get_gpr(op2);
    cpu.set_gpr(dest, op1 ^ op2);
}

void Interpreter::nor(CPU &cpu, uint32_t instruction)
{
    uint32_t op1 = (instruction >> 21) & 0x1F;
    uint32_t op2 = (instruction >> 16) & 0x1F;
    uint32_t dest = (instruction >> 11) & 0x1F;
    op1 = cpu.get_gpr(op1);
    op2 = cpu.get_gpr(op2);
    cpu.set_gpr(dest, ~(op1 | op2));
}

void Interpreter::slt(CPU &cpu, uint32_t instruction)
{
    int32_t op1 = (instruction >> 21) & 0x1F;
    int32_t op2 = (instruction >> 16) & 0x1F;
    uint32_t dest = (instruction >> 11) & 0x1F;
    op1 = (int32_t)cpu.get_gpr(op1);
    op2 = (int32_t)cpu.get_gpr(op2);
    cpu.set_gpr(dest, op1 < op2);
}

void Interpreter::sltu(CPU &cpu, uint32_t instruction)
{
    uint32_t op1 = (instruction >> 21) & 0x1F;
    uint32_t op2 = (instruction >> 16) & 0x1F;
    uint32_t dest = (instruction >> 11) & 0x1F;
    op1 = cpu.get_gpr(op1);
    op2 = cpu.get_gpr(op2);
    cpu.set_gpr(dest, op1 < op2);
}

void Interpreter::regimm(CPU &cpu, uint32_t instruction)
{
    int op = (instruction >> 16) & 0x1F;
    switch (op)
    {
        case 0x00:
            bltz(cpu, instruction);
            break;
        case 0x01:
            bgez(cpu, instruction);
            break;
        case 0x10:
            bltzal(cpu, instruction);
            break;
        case 0x11:
            bgezal(cpu, instruction);
            break;
        default:
            unknown_op("regimm", op, instruction);
    }
}

void Interpreter::bltz(CPU &cpu, uint32_t instruction)
{
    int offset = (int16_t)(instruction & 0xFFFF);
    offset <<= 2;
    int32_t reg = (int32_t)cpu.get_gpr((instruction >> 21) & 0x1F);
    cpu.branch(reg < 0, offset);
}

void Interpreter::bgez(CPU &cpu, uint32_t instruction)
{
    int offset = (int16_t)(instruction & 0xFFFF);
    offset <<= 2;
    int32_t reg = (int32_t)cpu.get_gpr((instruction >> 21) & 0x1F);
    cpu.branch(reg >= 0, offset);
}

void Interpreter::bltzal(CPU &cpu, uint32_t instruction)
{
    int32_t offset = (int16_t)(instruction & 0xFFFF);
    offset <<= 2;
    int32_t reg = (instruction >> 21) & 0x1F;
    reg = (int32_t)cpu.get_gpr(reg);
    cpu.set_gpr(31, cpu.get_PC() + 8);
    cpu.branch(reg < 0, offset);
}

void Interpreter::bgezal(CPU &cpu, uint32_t instruction)
{
    int32_t offset = (int16_t)(instruction & 0xFFFF);
    offset <<= 2;
    int32_t reg = (instruction >> 21) & 0x1F;
    reg = (int32_t)cpu.get_gpr(reg);
    cpu.set_gpr(31, cpu.get_PC() + 8);
    cpu.branch(reg >= 0, offset);
}

void Interpreter::cop(CPU &cpu, uint32_t instruction)
{
    int op = (instruction >> 21) & 0x1F;
    uint8_t cop_id = ((instruction >> 26) & 0x3);
    op |= cop_id << 8;
    switch (op)
    {
        case 0x000:
            mfc(cpu, instruction);
            break;
        case 0x004:
            mtc(cpu, instruction);
            break;
        case 0x010:
            cpu.rfe();
            break;
        default:
            unknown_op("cop", op, instruction);
    }
}

void Interpreter::mfc(CPU &cpu, uint32_t instruction)
{
    uint8_t cop_id = (instruction >> 26) & 0x3;
    uint8_t reg = (instruction >> 16) & 0x1F;
    uint8_t cop_reg = (instruction >> 11) & 0x1F;
    cpu.mfc(cop_id, cop_reg, reg);
}

void Interpreter::mtc(CPU &cpu, uint32_t instruction)
{
    uint8_t cop_id = (instruction >> 26) & 0x3;
    uint8_t reg = (instruction >> 16) & 0x1F;
    uint8_t cop_reg = (instruction >> 11) & 0x1F;
    cpu.mtc(cop_id, cop_reg, reg);
}

void Interpreter::unknown_op(const char *type, uint16_t op, uint32_t instruction)
{
    printf("\n[Interpreter] Unrecognized %s op $%02X\n", type, op);
    printf("[IOP Interpreter] Instruction: $%08X\n", instruction);
    exit(1);
}
