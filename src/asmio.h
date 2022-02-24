#ifndef ASMIO_H
#define ASMIO_H

#include <iostream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

namespace exasm {
    enum class InstType;

    enum class PseudoInst {
        PLACEHOLDER,
        LI,
        WORD,
    };

    class Inst {
    public:
        std::variant<InstType, PseudoInst> inst;
        std::uint8_t rd;
        std::uint8_t rs;
        std::variant<std::uint8_t, std::string> imm;
        std::uint16_t pseudo_param;

        static Inst new_with_type(InstType inst) {
            Inst result;
            result.inst = inst;
            result.rd = 0;
            result.rs = 0;
            return result;
        }

        static Inst new_with_reg_reg(InstType inst, std::uint8_t rd, std::uint8_t rs) {
            Inst result;
            result.inst = inst;
            result.rd = rd;
            result.rs = rs;
            return result;
        }

        static Inst new_with_reg_imm(InstType inst, std::uint8_t rd, std::uint8_t imm) {
            Inst result;
            result.inst = inst;
            result.rd = rd;
            result.rs = 0;
            result.imm = imm;
            return result;
        }

        static Inst new_with_reg_label(InstType inst, std::uint8_t rd, std::string label_name) {
            Inst result;
            result.inst = inst;
            result.rd = rd;
            result.rs = 0;
            result.imm = label_name;
            return result;
        }

        static Inst new_with_label(InstType inst, std::string label_name) {
            Inst result;
            result.inst = inst;
            result.rd = 0;
            result.rs = 0;
            result.imm = label_name;
            return result;
        }

        static Inst new_with_p_reg_imm(PseudoInst inst, std::uint8_t rd, std::string label_name,
                                       std::uint16_t pseudo_param) {
            Inst result;
            result.inst = inst;
            result.rd = rd;
            result.rs = 0;
            result.imm = label_name;
            result.pseudo_param = pseudo_param;
            return result;
        }

        static Inst new_with_data(std::uint16_t data) {
            Inst result;
            result.inst = PseudoInst::WORD;
            result.pseudo_param = data;
            return result;
        }

        static Inst decode(std::uint16_t inst);
        std::uint16_t encode() const;

        void print_asm(std::ostream &out) const;

        void print_bin(std::ostream &out) const;
    };

    class ParseError : public std::runtime_error {
    public:
        ParseError(std::string msg) : std::runtime_error(msg) {}
    };

    class LinkError : public std::runtime_error {
    public:
        LinkError(std::string msg) : std::runtime_error(msg) {}
    };

    class RawAsm {
        std::vector<Inst> insts;
        bool linked = false;
        std::uint16_t current_addr = 0;
        std::unordered_map<std::string, std::uint16_t> label_addr_mapping;
        int next_auto_label = 0;

        std::uint16_t get_destination(const std::string &label_name);
        void add_label(const std::string &label_name, std::uint16_t addr);
        void pre_handle_pseudo_instructions();
        void post_handle_pseudo_instructions();
        void handle_long_jump();
        void insert_inst_at_addr(Inst inst, std::uint16_t addr);

    public:
        void append(Inst &&inst, const std::string &label_name);
        std::vector<Inst> get_executable();
        std::string add_auto_label(std::int8_t addr_diff);
        std::string add_auto_label_at_addr(std::uint16_t addr);
    };

    class AsmReader {
        long linum = 1;
        std::istream &strm;

        std::string format_error(std::string message = "");

        std::variant<InstType, PseudoInst> read_inst_type();
        void next_line();
        void skip_space();
        void must_read_newline(const std::string &context);
        bool skip_newline();
        bool goto_next_instruction();
        std::uint8_t read_reg(std::string king);
        bool maybe_read(char c);
        void must_read(char c, std::string context);
        template <typename T> T read_immediate(bool allow_sign);
        std::string maybe_read_label();

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
