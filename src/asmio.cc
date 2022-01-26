#include "asmio.h"

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

        std::ostream &write_reg_arith_and_mem_op_bin(std::ostream &out,
                                                     InstType ty) {
            switch (ty) {
            case InstType::MOV:
                return out << "00001";
            case InstType::NOT:
                return out << "00010";
            case InstType::XOR:
                return out << "00011";
            case InstType::ADD:
                return out << "00100";
            case InstType::SUB:
                return out << "00101";
            case InstType::SL8:
                return out << "00110";
            case InstType::SR8:
                return out << "00111";
            case InstType::SL:
                return out << "01000";
            case InstType::SR:
                return out << "01001";
            case InstType::AND:
                return out << "01010";
            case InstType::OR:
                return out << "01011";
            case InstType::SW:
                return out << "10000";
            case InstType::LW:
                return out << "10001";
            case InstType::SBU:
                return out << "10010";
            case InstType::LBU:
                return out << "10011";
            default:
                asm_error("Not a reg arith type or mem type instruction");
            }
        }

        std::ostream &write_imm_and_branch_op_bin(std::ostream &out,
                                                  InstType ty) {
            switch (ty) {
            case InstType::ADDI:
                return out << "00100";
            case InstType::ANDI:
                return out << "01010";
            case InstType::ORI:
                return out << "01011";
            case InstType::LLI:
                return out << "00001";
            case InstType::LUI:
                return out << "00110";
            case InstType::BEQZ:
                return out << "10000";
            case InstType::BNEZ:
                return out << "10001";
            case InstType::BMI:
                return out << "10010";
            case InstType::BPL:
                return out << "10011";
            default:
                asm_error("Not a immediate type or branch instruction");
            }
        }

        std::ostream &write_reg_num_bin(std::ostream &out,
                                        std::uint8_t reg_num) {
            for (int i = 2; i >= 0; --i) {
                out << ((reg_num >> i) & 0x1);
            }
            return out;
        }

        std::ostream &write_imm_bin(std::ostream &out, std::uint8_t imm) {
            for (int i = 7; i >= 0; --i) {
                out << ((imm >> i) & 0x1);
            }
            return out;
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

        if (inst == "nop") {
            return InstType::NOP;
        } else if (inst == "mov") {
            return InstType::MOV;
        } else if (inst == "not") {
            return InstType::NOT;
        } else if (inst == "xor") {
            return InstType::XOR;
        } else if (inst == "add") {
            return InstType::ADD;
        } else if (inst == "sub") {
            return InstType::SUB;
        } else if (inst == "sl8") {
            return InstType::SL8;
        } else if (inst == "sr8") {
            return InstType::SR8;
        } else if (inst == "sl") {
            return InstType::SL;
        } else if (inst == "sr") {
            return InstType::SR;
        } else if (inst == "and") {
            return InstType::AND;
        } else if (inst == "or") {
            return InstType::OR;
        } else if (inst == "addi") {
            return InstType::ADDI;
        } else if (inst == "andi") {
            return InstType::ANDI;
        } else if (inst == "ori") {
            return InstType::ORI;
        } else if (inst == "lli") {
            return InstType::LLI;
        } else if (inst == "lui") {
            return InstType::LUI;
        } else if (inst == "sw") {
            return InstType::SW;
        } else if (inst == "lw") {
            return InstType::LW;
        } else if (inst == "sbu") {
            return InstType::SBU;
        } else if (inst == "lbu") {
            return InstType::LBU;
        } else if (inst == "beqz") {
            return InstType::BEQZ;
        } else if (inst == "bnez") {
            return InstType::BNEZ;
        } else if (inst == "bmi") {
            return InstType::BMI;
        } else if (inst == "bpl") {
            return InstType::BPL;
        } else if (inst == "j") {
            return InstType::J;
        }

        throw ParseError(format_error("Unknown instruction"));
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
        for (;;) {
            char c;
            strm.get(c);
            if (strm.fail()) {
                return;
            }
            if (!(c == ' ' || c == '\t')) {
                strm.unget();
                return;
            }
        }
    }

    void AsmReader::skip_space_and_newline() {
        for (;;) {
            char c;
            strm.get(c);
            if (strm.fail()) {
                return;
            }
            if (c == '\n') {
                ++linum;
            }
            if (!(c == ' ' || c == '\t' || c == '\r' || c == '\n')) {
                strm.unget();
                return;
            }
        }
    }

    bool AsmReader::finished() {
        char c;
        strm.get(c);
        if (strm.fail()) {
            return true;
        }
        strm.unget();
        return false;
    }

    std::uint8_t AsmReader::read_reg() {
        char c;
        strm.get(c);
        if (strm.fail()) {
            throw ParseError(format_error("Register expected, but got EOF"));
        }
        if (c != 'r') {
            throw ParseError(format_error("Illegal register syntax"));
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

    void AsmReader::must_read(char c) {
        char d;
        strm.get(d);
        if (strm.fail()) {
            throw ParseError(format_error("Unexpected EOF"));
        }
        if (d != c) {
            throw ParseError(format_error("Unexpected char"));
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

    std::ostream &operator<<(std::ostream &out, InstType ty) {
        switch (ty) {
        case InstType::NOP:
            return out << "nop";
        case InstType::MOV:
            return out << "mov";
        case InstType::NOT:
            return out << "not";
        case InstType::XOR:
            return out << "xor";
        case InstType::ADD:
            return out << "add";
        case InstType::SUB:
            return out << "sub";
        case InstType::SL8:
            return out << "sl8";
        case InstType::SR8:
            return out << "sr8";
        case InstType::SL:
            return out << "sl";
        case InstType::SR:
            return out << "sr";
        case InstType::AND:
            return out << "and";
        case InstType::OR:
            return out << "or";
        case InstType::SW:
            return out << "sw";
        case InstType::LW:
            return out << "lw";
        case InstType::SBU:
            return out << "sbu";
        case InstType::LBU:
            return out << "lbu";
        case InstType::ADDI:
            return out << "addi";
        case InstType::ANDI:
            return out << "andi";
        case InstType::ORI:
            return out << "ori";
        case InstType::LLI:
            return out << "lli";
        case InstType::LUI:
            return out << "lui";
        case InstType::BEQZ:
            return out << "beqz";
        case InstType::BNEZ:
            return out << "bnez";
        case InstType::BMI:
            return out << "bmi";
        case InstType::BPL:
            return out << "bpl";
        case InstType::J:
            return out << "j";
        }
        return out << "<unknown>";
    }

    std::ostream &operator<<(std::ostream &out, Inst &inst) {
        out << inst.inst;
        if (inst.inst != InstType::NOP) {
            out << ' ';
            return inst.print_asm_operand(out);
        }
        return out;
    }

    std::ostream &Inst::print_asm_operand(std::ostream &out) const {
        if (inst == InstType::NOP) {
            return out;
        } else if (reg_arith) {
            return out << "r" << +rd << ", r" << +rs;
        } else if (imm_inst) {
            bool use_sign = inst == InstType::ADDI || inst == InstType::BEQZ ||
                            inst == InstType::BNEZ || inst == InstType::BMI ||
                            inst == InstType::BPL;
            return write_hex(out << "r" << +rd << ", ", imm, use_sign);
        } else if (mem_inst) {
            return out << "r" << +rd << ", (r" << +rs << ")";
        } else if (jump_inst) {
            return write_hex(out << ' ', imm, true);
        }
        return out << "<unknown>";
    }

    std::ostream &Inst::print_bin(std::ostream &out) const {
        if (inst == InstType::NOP) {
            out << "00000000 00000000";
        } else if (reg_arith) {
            out << "00000";
            write_reg_num_bin(out, rd) << ' ';
            write_reg_num_bin(out, rs);
            return write_reg_arith_and_mem_op_bin(out, inst);
        } else if (imm_inst) {
            write_imm_and_branch_op_bin(out, inst);
            write_reg_num_bin(out, rd) << ' ';
            write_imm_bin(out, imm);
        } else if (mem_inst) {
            out << "00000";
            write_reg_num_bin(out, rd) << ' ';
            write_reg_num_bin(out, rs);
            return write_reg_arith_and_mem_op_bin(out, inst);
        } else if (jump_inst) {
            out << "11000000 ";
            return write_imm_bin(out, imm);
        }
        return out;
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

    Inst AsmReader::read_next() {
        InstType ty = read_inst_type();
        switch (ty) {
        case InstType::NOP:
            next_line();
            return Inst();
        case InstType::MOV:
        case InstType::NOT:
        case InstType::XOR:
        case InstType::ADD:
        case InstType::SUB:
        case InstType::SL8:
        case InstType::SR8:
        case InstType::SL:
        case InstType::SR:
        case InstType::AND:
        case InstType::OR: {
            skip_space();
            std::uint8_t rd = read_reg();
            skip_space();
            must_read(',');
            skip_space();
            std::uint8_t rs = read_reg();
            next_line();
            return Inst(ty, rd, rs, true);
        }
        case InstType::SW:
        case InstType::LW:
        case InstType::SBU:
        case InstType::LBU: {
            skip_space();
            std::uint8_t rd = read_reg();
            skip_space();
            must_read(',');
            skip_space();
            must_read('(');
            skip_space();
            std::uint8_t rs = read_reg();
            skip_space();
            must_read(')');
            next_line();
            return Inst(ty, rd, rs, false);
        }
        case InstType::ADDI:
        case InstType::ANDI:
        case InstType::ORI:
        case InstType::LLI:
        case InstType::LUI:
        case InstType::BEQZ:
        case InstType::BNEZ:
        case InstType::BMI:
        case InstType::BPL: {
            skip_space();
            std::uint8_t rd = read_reg();
            skip_space();
            must_read(',');
            skip_space();
            bool sign_allowed = ty == InstType::ADDI || ty == InstType::BEQZ ||
                                ty == InstType::BNEZ || ty == InstType::BMI ||
                                ty == InstType::BPL;
            std::uint8_t imm = read_immediate(sign_allowed);
            next_line();
            return Inst(ty, rd, imm);
        }
        case InstType::J: {
            skip_space();
            std::uint8_t imm = read_immediate(true);
            next_line();
            return Inst(ty, imm);
        }
        }

        throw ParseError(format_error("Unknown instruction"));
    }

    std::vector<Inst> AsmReader::read_all() {
        std::vector<Inst> result;
        while (!finished()) {
            skip_space_and_newline();
            Inst m = read_next();
            result.emplace_back(m);
        }
        return result;
    }
} // namespace exasm
