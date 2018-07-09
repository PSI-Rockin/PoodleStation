#ifndef INTERPRETER_HPP
#define INTERPRETER_HPP

#include "cpu.hpp"

namespace Interpreter
{
    void interpret(CPU& cpu, uint32_t instruction);

    void j(CPU& cpu, uint32_t instruction);
    void jal(CPU& cpu, uint32_t instruction);
    void beq(CPU& cpu, uint32_t instruction);
    void bne(CPU& cpu, uint32_t instruction);
    void blez(CPU& cpu, uint32_t instruction);
    void bgtz(CPU& cpu, uint32_t instruction);
    void addi(CPU& cpu, uint32_t instruction);
    void addiu(CPU& cpu, uint32_t instruction);
    void slti(CPU& cpu, uint32_t instruction);
    void sltiu(CPU& cpu, uint32_t instruction);
    void andi(CPU& cpu, uint32_t instruction);
    void ori(CPU& cpu, uint32_t instruction);
    void xori(CPU& cpu, uint32_t instruction);
    void lui(CPU& cpu, uint32_t instruction);
    void lb(CPU& cpu, uint32_t instruction);
    void lh(CPU& cpu, uint32_t instruction);
    void lwl(CPU& cpu, uint32_t instruction);
    void lw(CPU& cpu, uint32_t instruction);
    void lbu(CPU& cpu, uint32_t instruction);
    void lhu(CPU& cpu, uint32_t instruction);
    void lwr(CPU& cpu, uint32_t instruction);
    void sb(CPU& cpu, uint32_t instruction);
    void sh(CPU& cpu, uint32_t instruction);
    void swl(CPU& cpu, uint32_t instruction);
    void sw(CPU& cpu, uint32_t instruction);
    void swr(CPU& cpu, uint32_t instruction);

    void special(CPU& cpu, uint32_t instruction);
    void sll(CPU& cpu, uint32_t instruction);
    void srl(CPU& cpu, uint32_t instruction);
    void sra(CPU& cpu, uint32_t instruction);
    void sllv(CPU& cpu, uint32_t instruction);
    void srlv(CPU& cpu, uint32_t instruction);
    void srav(CPU& cpu, uint32_t instruction);
    void jr(CPU& cpu, uint32_t instruction);
    void jalr(CPU& cpu, uint32_t instruction);
    void syscall(CPU& cpu, uint32_t instruction);
    void mfhi(CPU& cpu, uint32_t instruction);
    void mthi(CPU& cpu, uint32_t instruction);
    void mflo(CPU& cpu, uint32_t instruction);
    void mtlo(CPU& cpu, uint32_t instruction);
    void mult(CPU& cpu, uint32_t instruction);
    void multu(CPU& cpu, uint32_t instruction);
    void div(CPU& cpu, uint32_t instruction);
    void divu(CPU& cpu, uint32_t instruction);
    void add(CPU& cpu, uint32_t instruction);
    void addu(CPU& cpu, uint32_t instruction);
    void sub(CPU& cpu, uint32_t instruction);
    void subu(CPU& cpu, uint32_t instruction);
    void and_cpu(CPU& cpu, uint32_t instruction);
    void or_cpu(CPU& cpu, uint32_t instruction);
    void xor_cpu(CPU& cpu, uint32_t instruction);
    void nor(CPU& cpu, uint32_t instruction);
    void slt(CPU& cpu, uint32_t instruction);
    void sltu(CPU& cpu, uint32_t instruction);

    void regimm(CPU& cpu, uint32_t instruction);
    void bltz(CPU& cpu, uint32_t instruction);
    void bgez(CPU& cpu, uint32_t instruction);
    void bltzal(CPU& cpu, uint32_t instruction);
    void bgezal(CPU& cpu, uint32_t instruction);

    void cop(CPU& cpu, uint32_t instruction);
    void mfc(CPU& cpu, uint32_t instruction);
    void mtc(CPU& cpu, uint32_t instruction);
    void rfe(CPU& cpu, uint32_t instruction);
    void ctc(CPU& cpu, uint32_t instruction);

    void unknown_op(const char* type, uint16_t op, uint32_t instruction);
};

#endif // INTERPRETER_HPP
