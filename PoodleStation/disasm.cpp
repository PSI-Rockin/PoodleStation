#include <iomanip>
#include <sstream>
#include "cpu.hpp"
#include "disasm.hpp"

using namespace std;

#define RS ((instruction >> 21) & 0x1F)
#define RT ((instruction >> 16) & 0x1F)
#define RD ((instruction >> 11) & 0x1F)
#define SA ((instruction >> 6 ) & 0x1F)
#define IMM ((int16_t)(instruction & 0xFFFF))

string Disasm::disasm_instr(uint32_t instruction, uint32_t instr_addr)
{
    if (!instruction)
        return "nop";
    switch (instruction >> 26)
    {
        case 0x00:
            return disasm_special(instruction);
        case 0x01:
            return disasm_regimm(instruction, instr_addr);
        case 0x02:
            return disasm_j(instruction, instr_addr);
        case 0x03:
            return disasm_jal(instruction, instr_addr);
        case 0x04:
            return disasm_beq(instruction, instr_addr);
        case 0x05:
            return disasm_bne(instruction, instr_addr);
        case 0x06:
            return disasm_blez(instruction, instr_addr);
        case 0x07:
            return disasm_bgtz(instruction, instr_addr);
        case 0x08:
            return disasm_addi(instruction);
        case 0x09:
            return disasm_addiu(instruction);
        case 0x0A:
            return disasm_slti(instruction);
        case 0x0B:
            return disasm_sltiu(instruction);
        case 0x0C:
            return disasm_andi(instruction);
        case 0x0D:
            return disasm_ori(instruction);
        case 0x0E:
            return disasm_xori(instruction);
        case 0x0F:
            return disasm_lui(instruction);
        case 0x10:
        case 0x11:
        case 0x12:
        case 0x13:
            return disasm_cop(instruction, instr_addr);
        case 0x14:
            return disasm_beql(instruction, instr_addr);
        case 0x15:
            return disasm_bnel(instruction, instr_addr);
        case 0x16:
            return disasm_branch_inequality("blezl", instruction, instr_addr);
        case 0x17:
            return disasm_branch_inequality("bgtzl", instruction, instr_addr);
        case 0x19:
            return disasm_daddiu(instruction);
        case 0x20:
            return disasm_lb(instruction);
        case 0x21:
            return disasm_lh(instruction);
        case 0x22:
            return disasm_lwl(instruction);
        case 0x23:
            return disasm_lw(instruction);
        case 0x24:
            return disasm_lbu(instruction);
        case 0x25:
            return disasm_lhu(instruction);
        case 0x26:
            return disasm_lwr(instruction);
        case 0x27:
            return disasm_lwu(instruction);
        case 0x28:
            return disasm_sb(instruction);
        case 0x29:
            return disasm_sh(instruction);
        case 0x2A:
            return disasm_swl(instruction);
        case 0x2B:
            return disasm_sw(instruction);
        case 0x2C:
            return disasm_sdl(instruction);
        case 0x2D:
            return disasm_sdr(instruction);
        case 0x2E:
            return disasm_swr(instruction);
        case 0x2F:
            return "cache";
        case 0x30:
            return disasm_loadstore("lwc0", instruction);
        case 0x33:
            return "pref";
        default:
            return unknown_op("normal", instruction >> 26, 2);
    }
}

string Disasm::disasm_j(uint32_t instruction, uint32_t instr_addr)
{
    return disasm_jump("j", instruction, instr_addr);
}

string Disasm::disasm_jal(uint32_t instruction, uint32_t instr_addr)
{
    return disasm_jump("jal", instruction, instr_addr);
}

string Disasm::disasm_jump(const string opcode, uint32_t instruction, uint32_t instr_addr)
{
    stringstream output;
    uint32_t addr = (instruction & 0x3FFFFFF) << 2;
    addr += (instr_addr + 4) & 0xF0000000;
    output << "$" << setfill('0') << setw(8) << hex << addr;

    return opcode + " " + output.str();
}

string Disasm::disasm_bne(uint32_t instruction, uint32_t instr_addr)
{
    return disasm_branch_equality("bne", instruction, instr_addr);
}

string Disasm::disasm_branch_equality(string opcode, uint32_t instruction, uint32_t instr_addr)
{
    stringstream output;
    int offset = IMM;
    offset <<=2;
    uint64_t rs = RS;
    uint64_t rt = RT;

    output << CPU::REG(rs) << ", ";
    if (!rt)
        opcode += "z";
    else
        output << CPU::REG(rt) << ", ";
    output << "$" << setfill('0') << setw(8) << hex << (instr_addr + offset + 4);

    return opcode + " " + output.str();
}

string Disasm::disasm_addiu(uint32_t instruction)
{
    return disasm_math("addiu", instruction);
}

string Disasm::disasm_regimm(uint32_t instruction, uint32_t instr_addr)
{
    stringstream output;
    string opcode = "";
    switch (RT)
    {
        case 0x00:
            opcode += "bltz";
            break;
        case 0x01:
            opcode += "bgez";
            break;
        case 0x02:
            opcode += "bltzl";
            break;
        case 0x03:
            opcode += "bgezl";
            break;
        case 0x10:
            opcode += "bltzal";
            break;
        case 0x11:
            opcode += "bgezal";
            break;
        case 0x12:
            opcode += "bltzall";
            break;
        case 0x13:
            opcode += "bgezall";
            break;
        default:
            return unknown_op("regimm", RT, 2);
    }
    int32_t offset = IMM;
    offset <<= 2;
    output << CPU::REG(RS) << ", "
           << "$" << setfill('0') << setw(8) << hex << (instr_addr + offset + 4);

    return opcode + " " + output.str();

}

string Disasm::disasm_special(uint32_t instruction)
{
    switch (instruction & 0x3F)
    {
        case 0x00:
            return disasm_sll(instruction);
        case 0x02:
            return disasm_srl(instruction);
        case 0x03:
            return disasm_sra(instruction);
        case 0x04:
            return disasm_sllv(instruction);
        case 0x06:
            return disasm_srlv(instruction);
        case 0x07:
            return disasm_srav(instruction);
        case 0x08:
            return disasm_jr(instruction);
        case 0x09:
            return disasm_jalr(instruction);
        case 0x0A:
            return disasm_movz(instruction);
        case 0x0B:
            return disasm_movn(instruction);
        case 0x0C:
            return disasm_syscall_ee(instruction);
        case 0x0F:
            return "sync";
        case 0x10:
            return disasm_mfhi(instruction);
        case 0x11:
            return disasm_mthi(instruction);
        case 0x12:
            return disasm_mflo(instruction);
        case 0x13:
            return disasm_mtlo(instruction);
        case 0x14:
            return disasm_dsllv(instruction);
        case 0x16:
            return disasm_dsrlv(instruction);
        case 0x17:
            return disasm_dsrav(instruction);
        case 0x18:
            return disasm_mult(instruction);
        case 0x19:
            return disasm_multu(instruction);
        case 0x1A:
            return disasm_div(instruction);
        case 0x1B:
            return disasm_divu(instruction);
        case 0x20:
            return disasm_add(instruction);
        case 0x21:
            return disasm_addu(instruction);
        case 0x22:
            return disasm_sub(instruction);
        case 0x23:
            return disasm_subu(instruction);
        case 0x24:
            return disasm_and_ee(instruction);
        case 0x25:
            return disasm_or_ee(instruction);
        case 0x26:
            return disasm_xor_ee(instruction);
        case 0x27:
            return disasm_nor(instruction);
        case 0x28:
            return disasm_mfsa(instruction);
        case 0x29:
            return disasm_mtsa(instruction);
        case 0x2A:
            return disasm_slt(instruction);
        case 0x2B:
            return disasm_sltu(instruction);
        default:
            return unknown_op("special", instruction & 0x3F, 2);
    }
}

string Disasm::disasm_sll(uint32_t instruction)
{
    return disasm_special_shift("sll", instruction);
}

string Disasm::disasm_srl(uint32_t instruction)
{
    return disasm_special_shift("srl", instruction);
}

string Disasm::disasm_sra(uint32_t instruction)
{
    return disasm_special_shift("sra", instruction);
}

string Disasm::disasm_variableshift(const string opcode, uint32_t instruction)
{
    stringstream output;
    output << CPU::REG(RD) << ", "
           << CPU::REG(RT) << ", "
           << CPU::REG(RS);

    return opcode + " " + output.str();
}

string Disasm::disasm_sllv(uint32_t instruction)
{
    return disasm_variableshift("sllv", instruction);
}

string Disasm::disasm_srlv(uint32_t instruction)
{
    return disasm_variableshift("srlv", instruction);
}

string Disasm::disasm_srav(uint32_t instruction)
{
    return disasm_variableshift("srav", instruction);
}

string Disasm::disasm_jr(uint32_t instruction)
{
    stringstream output;
    string opcode = "jr";
    output << CPU::REG(RS);

    return opcode + " " + output.str();
}

string Disasm::disasm_jalr(uint32_t instruction)
{
    stringstream output;
    string opcode = "jalr";
    if (RD != 31)
        output << CPU::REG(RD) << ", ";
    output << CPU::REG(RS);

    return opcode + " " + output.str();
}

string Disasm::disasm_conditional_move(const string opcode, uint32_t instruction)
{
    stringstream output;
    output << CPU::REG(RD) << ", "
           << CPU::REG(RS) << ", "
           << CPU::REG(RT);

    return opcode + " " + output.str();
}

string Disasm::disasm_movz(uint32_t instruction)
{
    return disasm_conditional_move("movz", instruction);
}

string Disasm::disasm_movn(uint32_t instruction)
{
    return disasm_conditional_move("movn", instruction);
}

string Disasm::disasm_syscall_ee(uint32_t instruction)
{
    stringstream output;
    string opcode = "syscall";
    uint32_t code = (instruction >> 6) & 0xFFFFF;
    output << "$" << setfill('0') << setw(8) << hex << code;

    return opcode + " " + output.str();
}

string Disasm::disasm_movereg(const string opcode, uint32_t instruction)
{
    stringstream output;
    output << CPU::REG(RD);
    return opcode + " " + output.str();
}

string Disasm::disasm_mfhi(uint32_t instruction)
{
    return disasm_movereg("mfhi", instruction);
}

string Disasm::disasm_moveto(const string opcode, uint32_t instruction)
{
    stringstream output;
    output << CPU::REG(RS);
    return opcode + " " + output.str();
}

string Disasm::disasm_mthi(uint32_t instruction)
{
    return disasm_moveto("mthi", instruction);
}

string Disasm::disasm_mflo(uint32_t instruction)
{
    return disasm_movereg("mflo", instruction);
}

string Disasm::disasm_mtlo(uint32_t instruction)
{
    return disasm_moveto("mtlo", instruction);
}

string Disasm::disasm_dsllv(uint32_t instruction)
{
    return disasm_variableshift("dsllv", instruction);
}

string Disasm::disasm_dsrlv(uint32_t instruction)
{
    return disasm_variableshift("dsrlv", instruction);
}

string Disasm::disasm_dsrav(uint32_t instruction)
{
    return disasm_variableshift("dsrav", instruction);
}

string Disasm::disasm_mult(uint32_t instruction)
{
    return disasm_special_simplemath("mult", instruction);
}

string Disasm::disasm_multu(uint32_t instruction)
{
    return disasm_special_simplemath("multu", instruction);
}

string Disasm::disasm_division(const string opcode, uint32_t instruction)
{
    stringstream output;
    output << CPU::REG(RS) << ", "
           << CPU::REG(RT);

    return opcode + " " + output.str();
}

string Disasm::disasm_div(uint32_t instruction)
{
    return disasm_division("div", instruction);
}

string Disasm::disasm_divu(uint32_t instruction)
{
    return disasm_division("divu", instruction);
}

string Disasm::disasm_special_simplemath(const string opcode, uint32_t instruction)
{
    stringstream output;
    output << CPU::REG(RD) << ", "
           << CPU::REG(RS) << ", "
           << CPU::REG(RT);

    return opcode + " " + output.str();
}

string Disasm::disasm_add(uint32_t instruction)
{
    return disasm_special_simplemath("add", instruction);
}

string Disasm::disasm_addu(uint32_t instruction)
{
    return disasm_special_simplemath("addu", instruction);
}

string Disasm::disasm_sub(uint32_t instruction)
{
    return disasm_special_simplemath("add", instruction);
}

string Disasm::disasm_subu(uint32_t instruction)
{
    return disasm_special_simplemath("subu", instruction);
}

string Disasm::disasm_and_ee(uint32_t instruction)
{
    return disasm_special_simplemath("and", instruction);
}

string Disasm::disasm_or_ee(uint32_t instruction)
{
    return disasm_special_simplemath("or", instruction);
}

string Disasm::disasm_xor_ee(uint32_t instruction)
{
    return disasm_special_simplemath("xor", instruction);
}

string Disasm::disasm_nor(uint32_t instruction)
{
    return disasm_special_simplemath("nor", instruction);
}

string Disasm::disasm_mfsa(uint32_t instruction)
{
    return disasm_movereg("mfsa", instruction);
}

string Disasm::disasm_mtsa(uint32_t instruction)
{
    return disasm_moveto("mtsa", instruction);
}

string Disasm::disasm_slt(uint32_t instruction)
{
    return disasm_special_simplemath("slt", instruction);
}

string Disasm::disasm_sltu(uint32_t instruction)
{
    return disasm_special_simplemath("sltu", instruction);
}

string Disasm::disasm_special_shift(const string opcode, uint32_t instruction)
{
    stringstream output;
    output << CPU::REG(RD) << ", "
           << CPU::REG(RT) << ", "
           << SA;
    return opcode + " " + output.str();
}

string Disasm::disasm_beq(uint32_t instruction, uint32_t instr_addr)
{
    return disasm_branch_equality("beq", instruction, instr_addr);
}

string Disasm::disasm_branch_inequality(const std::string opcode, uint32_t instruction, uint32_t instr_addr)
{
    stringstream output;
    int32_t offset = IMM;
    offset <<= 2;

    output << CPU::REG(RS) << ", "
           << "$" << setfill('0') << setw(8) << hex << (instr_addr + offset + 4);

    return opcode + " " + output.str();

}

string Disasm::disasm_blez(uint32_t instruction, uint32_t instr_addr)
{
    return disasm_branch_inequality("blez", instruction, instr_addr);
}

string Disasm::disasm_bgtz(uint32_t instruction, uint32_t instr_addr)
{
    return disasm_branch_inequality("bgtz", instruction, instr_addr);
}

string Disasm::disasm_math(const string opcode, uint32_t instruction)
{
    stringstream output;
    output << CPU::REG(RT) << ", " << CPU::REG(RS) << ", "
           << "$" << setfill('0') << setw(4) << hex << IMM;
    return opcode + " " + output.str();
}

string Disasm::disasm_addi(uint32_t instruction)
{
    return disasm_math("addi", instruction);
}

string Disasm::disasm_slti(uint32_t instruction)
{
    return disasm_math("slti", instruction);
}

string Disasm::disasm_sltiu(uint32_t instruction)
{
    return disasm_math("sltiu", instruction);
}

string Disasm::disasm_andi(uint32_t instruction)
{
    return disasm_math("andi", instruction);
}

string Disasm::disasm_ori(uint32_t instruction)
{
    if (IMM == 0)
        return disasm_move(instruction);
    return disasm_math("ori", instruction);
}

string Disasm::disasm_move(uint32_t instruction)
{
    stringstream output;
    string opcode = "move";
    output << CPU::REG(RD) << ", "
           << CPU::REG(RS);

    return opcode + " " + output.str();
}

string Disasm::disasm_xori(uint32_t instruction)
{
    return disasm_math("xori", instruction);
}

string Disasm::disasm_lui(uint32_t instruction)
{
    stringstream output;
    string opcode = "lui";
    output << CPU::REG(RT) << ", "
           << "$" << setfill('0') << setw(4) << hex << IMM;

    return opcode + " " + output.str();
}

string Disasm::disasm_beql(uint32_t instruction, uint32_t instr_addr)
{
    return disasm_branch_equality("beql", instruction, instr_addr);
}

string Disasm::disasm_bnel(uint32_t instruction, uint32_t instr_addr)
{
    return disasm_branch_equality("bnel", instruction, instr_addr);
}

string Disasm::disasm_daddiu(uint32_t instruction)
{
    return disasm_math("daddiu", instruction);
}

string Disasm::disasm_loadstore(const std::string opcode, uint32_t instruction)
{
    stringstream output;
    output << CPU::REG(RT) << ", "
           << IMM
           << "{" << CPU::REG(RS) << "}";
    return opcode + " " + output.str();
}

string Disasm::disasm_lb(uint32_t instruction)
{
    return disasm_loadstore("lb", instruction);
}

string Disasm::disasm_lh(uint32_t instruction)
{
    return disasm_loadstore("lh", instruction);
}

string Disasm::disasm_lwl(uint32_t instruction)
{
    return disasm_loadstore("lwl", instruction);
}

string Disasm::disasm_lw(uint32_t instruction)
{
    return disasm_loadstore("lw", instruction);
}

string Disasm::disasm_lbu(uint32_t instruction)
{
    return disasm_loadstore("lbu", instruction);
}

string Disasm::disasm_lhu(uint32_t instruction)
{
    return disasm_loadstore("lhu", instruction);
}

string Disasm::disasm_lwr(uint32_t instruction)
{
    return disasm_loadstore("lwr", instruction);
}

string Disasm::disasm_lwu(uint32_t instruction)
{
    return disasm_loadstore("lwu", instruction);
}

string Disasm::disasm_sb(uint32_t instruction)
{
    return disasm_loadstore("sb", instruction);
}

string Disasm::disasm_sh(uint32_t instruction)
{
    return disasm_loadstore("sh", instruction);
}

string Disasm::disasm_swl(uint32_t instruction)
{
    return disasm_loadstore("swl", instruction);
}

string Disasm::disasm_sw(uint32_t instruction)
{
    return disasm_loadstore("sw", instruction);
}

string Disasm::disasm_sdl(uint32_t instruction)
{
    return disasm_loadstore("sdl", instruction);
}

string Disasm::disasm_sdr(uint32_t instruction)
{
    return disasm_loadstore("sdr", instruction);
}

string Disasm::disasm_swr(uint32_t instruction)
{
    return disasm_loadstore("swr", instruction);
}

string Disasm::disasm_cop(uint32_t instruction, uint32_t instr_addr)
{
    uint16_t op = RS;
    uint8_t cop_id = ((instruction >> 26) & 0x3);
    switch (op | (cop_id * 0x100))
    {
        case 0x000:
        case 0x100:
            return disasm_cop_mfc(instruction);
        case 0x004:
        case 0x104:
            return disasm_cop_mtc(instruction);
        case 0x010:
        {
            uint8_t op2 = instruction & 0x3F;
            switch (op2)
            {
                case 0x2:
                    return "tlbwi";
                case 0x18:
                    return "eret";
                case 0x38:
                    return "ei";
                case 0x39:
                    return "di";
                default:
                    return unknown_op("cop0x010", op2, 2);

            }
        }
        case 0x106:
        case 0x206:
            return disasm_cop_ctc(instruction);
        case 0x102:
        case 0x202:
            return disasm_cop_cfc(instruction);
        default:
            return unknown_op("cop", op, 2);
    }
}

string Disasm::disasm_cop_move(string opcode, uint32_t instruction)
{
    stringstream output;

    int cop_id = (instruction >> 26) & 0x3;

    output << opcode << cop_id << " " << CPU::REG(RT) << ", "
           << RD;

    return output.str();
}

string Disasm::disasm_cop_mfc(uint32_t instruction)
{
    return disasm_cop_move("mfc", instruction);
}

string Disasm::disasm_cop_mtc(uint32_t instruction)
{
    return disasm_cop_move("mtc", instruction);
}

string Disasm::disasm_cop_cfc(uint32_t instruction)
{
    return disasm_cop_move("cfc", instruction);
}

string Disasm::disasm_cop_ctc(uint32_t instruction)
{
    return disasm_cop_move("ctc", instruction);
}

string Disasm::unknown_op(const string optype, uint32_t op, int width)
{
    stringstream output;
    output << "Unrecognized " << optype << " op "
           << "$" << setfill('0') << setw(width) << hex << op;
    return output.str();
}
