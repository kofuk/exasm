#ifndef ASMIO_H
#define ASMIO_H

#include <iostream>
#include <stdexcept>
#include <vector>

namespace exasm {
    enum class InstType;

    class Inst {
    public:
        InstType inst;
        std::uint8_t rd;
        std::uint8_t rs;
        std::uint8_t imm;
        bool reg_arith = false;
        bool mem_inst = false;
        bool imm_inst = false;
        bool jump_inst = false;

        Inst(InstType inst) : inst(inst) {}

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

        void print_asm(std::ostream &out) const;

        void print_bin(std::ostream &out) const;
    };

    class ParseError : public std::runtime_error {
    public:
        ParseError(std::string msg) : std::runtime_error(msg) {}
    };

    class AsmReader {
        long linum = 1;
        std::istream &strm;

        std::string format_error(std::string message = "");

        InstType read_inst_type();
        void next_line();
        void skip_space();
        void must_read_newline(const std::string &context);
        bool skip_newline();
        bool goto_next_instruction();
        std::uint8_t read_reg(std::string king);
        void must_read(char c, std::string context);
        std::uint8_t read_immediate(bool allow_sign);

    public:
        AsmReader(std::istream &strm) : strm(strm) {}

        Inst read_next();
        void try_recover();
        bool finished();
        std::vector<Inst> read_all();
    };

    std::ostream &write_addr(std::ostream &out, std::uint16_t num);
} // namespace exasm

#endif
