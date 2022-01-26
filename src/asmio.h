#ifndef ASMIO_H
#define ASMIO_H

#include <iostream>
#include <vector>

namespace exasm {
    enum class InstType {
        NOP,
        MOV,
        NOT,
        XOR,
        ADD,
        SUB,
        SL8,
        SR8,
        SL,
        SR,
        AND,
        OR,
        ADDI,
        ANDI,
        ORI,
        LLI,
        LUI,
        SW,
        LW,
        SBU,
        LBU,
        BEQZ,
        BNEZ,
        BMI,
        BPL,
        J,
    };

    class Inst {
    public:
        InstType inst = InstType::NOP;
        std::uint8_t rd;
        std::uint8_t rs;
        std::uint8_t imm;
        bool reg_arith = false;
        bool mem_inst = false;
        bool imm_inst = false;
        bool jump_inst = false;

        Inst() {}

        Inst(InstType inst, std::uint8_t rd, std::uint8_t rs, bool reg_arith)
            : inst(std::move(inst)), rd(rd), rs(rs) {
            if (reg_arith) {
                this->reg_arith = true;
            } else {
                this->mem_inst = true;
            }
        }

        Inst(InstType inst, std::uint8_t rd, std::uint8_t imm)
            : inst(inst), rd(rd), imm(imm), imm_inst(true) {}

        Inst(InstType inst, std::uint8_t imm)
            : inst(inst), imm(imm), jump_inst(true) {}

        std::ostream &print_asm_operand(std::ostream &out) const;

        std::ostream &print_bin(std::ostream &out) const;
    };

    std::ostream &operator<<(std::ostream &out, InstType ty);
    std::ostream &operator<<(std::ostream &out, Inst &inst);

    std::ostream &write_addr(std::ostream &out, std::uint16_t num);
    Inst read_next(std::istream &strm);
    std::vector<Inst> read_all(std::istream &strm);
} // namespace exasm

#endif
