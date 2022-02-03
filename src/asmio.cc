#include <cctype>
#include <optional>
#include <stdexcept>
#include <string>
#include <variant>

#include "asmio.h"
#include "insts.h"

namespace exasm {
    namespace {
        [[noreturn]] void asm_error(const char *message = nullptr) {
            std::cerr << "Asm error";
            if (message != nullptr) {
                std::cerr << ": " << message;
            }
            std::cerr << '\n';
            std::exit(1);
        }

        std::ostream &write_hex(std::ostream &out, std::uint8_t num,
                                bool use_sign) {
            if (use_sign && (num >> 7) != 0) {
                out << '-';
                num = ~num + 1;
            }
            out << "0x";
            for (int i = 1; i >= 0; --i) {
                int hd = (num >> (4 * i)) & 0xF;
                if (hd < 10) {
                    out << static_cast<char>(hd + '0');
                } else {
                    out << static_cast<char>(hd + '7');
                }
            }
            return out;
        }

        void write_reg_num_bin(std::ostream &out, std::uint8_t reg_num) {
            for (int i = 2; i >= 0; --i) {
                out << ((reg_num >> i) & 0x1);
            }
        }

        void write_imm_bin(std::ostream &out, std::uint8_t imm) {
            for (int i = 7; i >= 0; --i) {
                out << ((imm >> i) & 0x1);
            }
        }
    } // namespace

    std::string AsmReader::format_error(std::string message) {
        std::string result = ""
                             "Parse error at line " +
                             std::to_string(linum);
        if (message.size() != 0) {
            result += ": " + message;
        }
        return result;
    }

    InstType AsmReader::read_inst_type() {
        std::string inst;
        for (;;) {
            char c;
            strm.get(c);
            if (strm.fail()) {
                break;
            }
            if (('a' <= c && c <= 'z') || ('0' <= c && c <= '9')) {
                inst.push_back(c);
            } else {
                strm.unget();
                break;
            }
        }

#include "inst_name_to_enum.inc"

        throw ParseError(format_error("Unknown instruction: " + inst));
    }

    void AsmReader::next_line() {
        for (;;) {
            char c;
            strm.get(c);
            if (strm.fail()) {
                break;
            }
            if (c == '\n') {
                ++linum;
                return;
            }
        }
    }

    void AsmReader::skip_space() {
        bool comment = false;
        for (;;) {
            char c;
            strm.get(c);
            if (strm.fail()) {
                return;
            }
            if (c == '#') {
                comment = true;
            }
            if (comment) {
                if (c == '\r' || c == '\n') {
                    strm.unget();
                    return;
                }
            } else {
                if (!(c == ' ' || c == '\t')) {
                    strm.unget();
                    return;
                }
            }
        }
    }

    void AsmReader::must_read_newline(const std::string &context) {
        char c;
        strm.get(c);
        if (strm.fail()) {
            return;
        }
        if (c == '\r') {
            strm.get(c);
            if (strm.fail()) {
                return;
            }
            if (c != '\n') {
                ++linum;
                strm.unget();
                return;
            }
        } else if (c != '\n') {
            strm.unget();
            throw ParseError(format_error("New line expected " + context));
        }
        ++linum;
    }

    bool AsmReader::skip_newline() {
        char c;
        strm.get(c);
        if (strm.fail()) {
            return true;
        }
        if (c == '\r') {
            strm.get(c);
            if (strm.fail()) {
                return true;
            }
            if (c != '\n') {
                ++linum;
                strm.unget();
                return true;
            }
        } else if (c != '\n') {
            strm.unget();
            return false;
        }
        ++linum;
        return true;
    }

    bool AsmReader::goto_next_instruction() {
        for (;;) {
            skip_space();
            char c;
            strm.get(c);
            if (strm.fail()) {
                return false;
            }
            strm.unget();
            if (!skip_newline()) {
                return true;
            }
        }
    }

    bool AsmReader::finished() { return !goto_next_instruction(); }

    std::uint8_t AsmReader::read_reg(std::string kind) {
        char c;
        strm.get(c);
        if (strm.fail()) {
            throw ParseError(format_error("Register expected, but got EOF"));
        }
        if (c != 'r') {
            if (!kind.empty()) {
                kind[0] = std::toupper(kind[0]);
            }
            throw ParseError(format_error(kind + " register name expected"));
        }
        strm.get(c);
        if (strm.fail()) {
            throw ParseError(
                format_error("Register number expected, but got EOF"));
        }
        if ('0' <= c && c <= '9') {
            std::uint8_t reg_num = c - '0';
            if (reg_num >= 8) {
                throw ParseError(format_error("Illegal register number"));
            }
            return reg_num;
        }
        throw ParseError(format_error("Illegal register"));
    }

    void AsmReader::must_read(char c, std::string context) {
        char d;
        strm.get(d);
        if (strm.fail()) {
            throw ParseError(format_error("Unexpected EOF"));
        }
        if (d != c) {
            std::string err_msg = "Expects '";
            err_msg.push_back(c);
            err_msg.push_back('\'');
            err_msg.push_back(' ');
            err_msg += context;
            throw ParseError(format_error(err_msg));
        }
    }

    std::uint8_t AsmReader::read_immediate(bool allow_sign) {
        char c;
        strm.get(c);
        if (strm.fail()) {
            throw ParseError(format_error("Unexpected EOF"));
        }
        bool minus = false;
        if (allow_sign) {
            if (c == '-') {
                minus = true;
                strm.get(c);
                if (strm.fail()) {
                    throw ParseError(format_error("Unexpected EOF"));
                }
            }
        }

        std::uint8_t result = 0;
        int base = 10;
        if (c == '0') {
            base = 8;
            char c;
            strm.get(c);
            if (strm.fail()) {
                return 0;
            }
            if (c == 'x') {
                base = 16;
            } else {
                strm.unget();
            }
        } else if ('1' <= c && c <= '9') {
            result = c - '0';
        } else {
            throw ParseError(format_error("Illegal immediate"));
        }
        for (;;) {
            char c;
            strm.get(c);
            if (strm.fail()) {
                break;
            }
            int curnum;
            if ('0' <= c && c <= '9') {
                curnum = c - '0';
            } else if ('a' <= c && c <= 'f') {
                curnum = c - 'W';
            } else if ('A' <= c && c <= 'F') {
                curnum = c - '7';
            } else {
                strm.unget();
                break;
            }
            result *= base;
            result += curnum;
        }

        if (minus) {
            result = ~result + 1;
        }

        return result;
    }

    std::string AsmReader::maybe_read_label() {
        char c;
        strm.get(c);
        if (strm.fail()) {
            return "";
        }
        if (c != '@') {
            strm.unget();
            return "";
        }
        strm.get(c);
        if (strm.fail() || c == ' ' || c == '\t' || c == '\r' || c == '\n') {
            throw ParseError(format_error("Label name expected"));
        }
        if (c != '_' && (c < 'a' || 'z' < c) && (c < 'A' || 'Z' < c)) {
            strm.unget();
            throw ParseError(
                format_error("Label name should start with a-z, A-Z or _"));
        }
        std::string label_name;
        label_name.push_back(c);

        for (;;) {
            strm.get(c);
            if (strm.fail()) {
                break;
            }
            if (c != '_' && (c < 'a' || 'z' < c) && (c < 'A' || 'Z' < c) &&
                (c < '0' || '9' < c)) {
                strm.unget();
                break;
            }
            label_name.push_back(c);
        }

        return label_name;
    }

    void Inst::print_asm(std::ostream &out) const {
        switch (inst) {
#include "asm_writer.inc"
        default:
            out << "<unknown>";
        }
    }

    void Inst::print_bin(std::ostream &out) const {
        switch (inst) {
#include "inst_mem_writer.inc"
        default:
            asm_error("Illegal instruction");
        }
    }

    std::ostream &write_addr(std::ostream &out, std::uint16_t num) {
        out << "@";
        for (int i = 1; i >= 0; --i) {
            int hd = (num >> (4 * i)) & 0xF;
            if (hd < 10) {
                out << static_cast<char>(hd + '0');
            } else {
                out << static_cast<char>(hd + 'W');
            }
        }
        return out;
    }

    void RawAsm::append(Inst &&inst, const std::string &label_name) {
        if (linked) {
            throw std::logic_error("Appending to linked RawAsm is not allowed");
        }

        if (!label_name.empty()) {
            add_label(label_name, current_addr);
        }
        insts.push_back(inst);
        current_addr += 2;
    }

    std::vector<Inst> RawAsm::get_executable() {
        if (!linked) {
            linked = true;

            std::uint16_t inst_pc = 0;
            for (Inst &inst : insts) {
                inst_pc += 2;
                if (std::holds_alternative<std::string>(inst.imm)) {
                    std::uint16_t dest =
                        get_destination(std::get<std::string>(inst.imm));
                    std::uint8_t addr_diff = static_cast<std::uint8_t>(
                        static_cast<std::int16_t>(dest) -
                        static_cast<std::int16_t>(inst_pc));
                    inst.imm = addr_diff;
                }
            }

            label_addr_mapping.clear();
        }
        return insts;
    }

    std::string RawAsm::add_auto_label(std::int8_t diff_from_pc) {
        std::uint16_t addr = current_addr + 2 + diff_from_pc;

        for (auto const &[label, label_addr] : label_addr_mapping) {
            if (addr == label_addr) {
                return label;
            }
        }
        std::string label("!");
        label += std::to_string(next_auto_label);
        ++next_auto_label;
        add_label(label, addr);
        return label;
    }

    std::uint16_t RawAsm::get_destination(const std::string &label_name) {
        auto pos = label_addr_mapping.find(label_name);
        if (pos == label_addr_mapping.end()) {
            throw LinkError("Undefined label: " + label_name);
        }
        return pos->second;
    }

    void RawAsm::add_label(const std::string &label_name, std::uint16_t addr) {
        if (label_addr_mapping.find(label_name) != label_addr_mapping.end()) {
            throw LinkError("Duplicate label: " + label_name);
        }
        label_addr_mapping[label_name] = addr;
    }

    void AsmReader::read_next(RawAsm &to) {
        if (!goto_next_instruction()) {
            throw ParseError(
                "AsmReader::read_next called after last instruction finished.");
        }

        std::string label = maybe_read_label();
        skip_space();

        InstType ty = read_inst_type();
        switch (ty) {
#include "asm_parser.inc"
        }

        throw ParseError(format_error("Unknown instruction"));
    }

    void AsmReader::try_recover() { next_line(); }

    RawAsm AsmReader::read_all() {
        RawAsm result;
        while (!finished()) {
            read_next(result);
        }
        return result;
    }
} // namespace exasm
