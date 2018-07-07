#ifndef DISASM_HPP
#define DISASM_HPP

#include <cstdint>
#include <string>

namespace Disasm
{
    std::string disasm_instr(uint32_t instruction, uint32_t instr_addr);

    std::string disasm_special(uint32_t instruction);
    std::string disasm_special_shift(const std::string opcode, uint32_t instruction);
    std::string disasm_sll(uint32_t instruction);
    std::string disasm_srl(uint32_t instruction);
    std::string disasm_sra(uint32_t instruction);
    std::string disasm_variableshift(const std::string opcode, uint32_t instruction);
    std::string disasm_sllv(uint32_t instruction);
    std::string disasm_srlv(uint32_t instruction);
    std::string disasm_srav(uint32_t instruction);
    std::string disasm_jr(uint32_t instruction);
    std::string disasm_jalr(uint32_t instruction);
    std::string disasm_conditional_move(const std::string opcode, uint32_t instruction);
    std::string disasm_movz(uint32_t instruction);
    std::string disasm_movn(uint32_t instruction);
    std::string disasm_syscall_ee(uint32_t instruction);
    std::string disasm_movereg(const std::string opcode, uint32_t instruction);
    std::string disasm_mfhi(uint32_t instruction);
    std::string disasm_moveto(const std::string opcode, uint32_t instruction);
    std::string disasm_mthi(uint32_t instruction);
    std::string disasm_mflo(uint32_t instruction);
    std::string disasm_mtlo(uint32_t instruction);
    std::string disasm_dsllv(uint32_t instruction);
    std::string disasm_dsrav(uint32_t instruction);
    std::string disasm_dsrlv(uint32_t instruction);
    std::string disasm_mult(uint32_t instruction);
    std::string disasm_multu(uint32_t instruction);
    std::string disasm_division(const std::string opcode, uint32_t instruction);
    std::string disasm_div(uint32_t instruction);
    std::string disasm_divu(uint32_t instruction);
    std::string disasm_special_simplemath(const std::string opcode, uint32_t instruction);
    std::string disasm_add(uint32_t instruction);
    std::string disasm_addu(uint32_t instruction);
    std::string disasm_sub(uint32_t instruction);
    std::string disasm_subu(uint32_t instruction);
    std::string disasm_and_ee(uint32_t instruction);
    std::string disasm_or_ee(uint32_t instruction);
    std::string disasm_xor_ee(uint32_t instruction);
    std::string disasm_nor(uint32_t instruction);
    std::string disasm_mfsa(uint32_t instruction);
    std::string disasm_mtsa(uint32_t instruction);
    std::string disasm_slt(uint32_t instruction);
    std::string disasm_sltu(uint32_t instruction);

    std::string disasm_regimm(uint32_t instruction, uint32_t instr_addr);

    std::string disasm_jump(const std::string opcode, uint32_t instruction, uint32_t instr_addr);
    std::string disasm_j(uint32_t instruction, uint32_t instr_addr);
    std::string disasm_jal(uint32_t instruction, uint32_t instr_addr);
    std::string disasm_branch_equality(std::string opcode, uint32_t instruction, uint32_t instr_addr);
    std::string disasm_beq(uint32_t instruction, uint32_t instr_addr);
    std::string disasm_bne(uint32_t instruction, uint32_t instr_addr);
    std::string disasm_branch_inequality(const std::string opcode, uint32_t instruction, uint32_t instr_addr);
    std::string disasm_blez(uint32_t instruction, uint32_t instr_addr);
    std::string disasm_bgtz(uint32_t instruction, uint32_t instr_addr);
    std::string disasm_math(const std::string opcode, uint32_t instruction);
    std::string disasm_addi(uint32_t instruction);
    std::string disasm_addiu(uint32_t instruction);
    std::string disasm_slti(uint32_t instruction);
    std::string disasm_sltiu(uint32_t instruction);
    std::string disasm_andi(uint32_t instruction);
    std::string disasm_ori(uint32_t instruction);
    std::string disasm_move(uint32_t instruction);
    std::string disasm_xori(uint32_t instruction);
    std::string disasm_lui(uint32_t instruction);
    std::string disasm_beql(uint32_t instruction, uint32_t instr_addr);
    std::string disasm_bnel(uint32_t instruction, uint32_t instr_addr);
    std::string disasm_daddiu(uint32_t instruction);
    std::string disasm_loadstore(const std::string opcode, uint32_t instruction);
    std::string disasm_lb(uint32_t instruction);
    std::string disasm_lh(uint32_t instruction);
    std::string disasm_lw(uint32_t instruction);
    std::string disasm_lwl(uint32_t instruction);
    std::string disasm_lbu(uint32_t instruction);
    std::string disasm_lhu(uint32_t instruction);
    std::string disasm_lwr(uint32_t instruction);
    std::string disasm_lwu(uint32_t instruction);
    std::string disasm_sb(uint32_t instruction);
    std::string disasm_sh(uint32_t instruction);
    std::string disasm_swl(uint32_t instruction);
    std::string disasm_sw(uint32_t instruction);
    std::string disasm_sdl(uint32_t instruction);
    std::string disasm_sdr(uint32_t instruction);
    std::string disasm_swr(uint32_t instruction);

    std::string disasm_cop(uint32_t instruction, uint32_t instr_addr);
    std::string disasm_cop_move(std::string opcode, uint32_t instruction);
    std::string disasm_cop_mfc(uint32_t instruction);
    std::string disasm_cop_mtc(uint32_t instruction);
    std::string disasm_cop_cfc(uint32_t instruction);
    std::string disasm_cop_ctc(uint32_t instruction);

    std::string unknown_op(const std::string optype, uint32_t op, int width);

};

#endif // DISASM_HPP
