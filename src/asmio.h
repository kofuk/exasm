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

        static Inst new_with_type(InstType inst) {
            Inst result;
            result.inst = inst;
            return result;
        }

        static Inst new_with_reg_reg(InstType inst, std::uint8_t rd,
                                     std::uint8_t rs) {
            Inst result;
            result.inst = inst;
            result.rd = rd;
            result.rs = rs;
            return result;
        }

        static Inst new_with_reg_imm(InstType inst, std::uint8_t rd,
                                     std::uint8_t imm) {
            Inst result;
            result.inst = inst;
            result.rd = rd;
            result.imm = imm;
            return result;
        }

        static Inst new_with_imm(InstType inst, std::uint8_t imm) {
            Inst result;
            result.inst = inst;
            result.imm = imm;
            return result;
        }

        void print_asm(std::ostream &out) const;

        void print_bin(std::ostream &out) const;
    };

    class ParseError : public std::runtime_error {
    public:
        ParseError(std::string msg) : std::runtime_error(msg) {}
    };

    class RawAsm {
        std::vector<Inst> insts;
        bool linked = false;

    public:
        void append(Inst &&inst);
        std::vector<Inst> get_executable();
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

        void read_next(RawAsm &to);
        void try_recover();
        bool finished();
        RawAsm read_all();
    };

    std::ostream &write_addr(std::ostream &out, std::uint16_t num);
} // namespace exasm

#endif
