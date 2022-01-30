#include <algorithm>
#include <cstdint>
#include <stdexcept>
#include <string>

#include "asmio.h"
#include "emulator.h"

namespace exasm {
    namespace {
        std::int16_t sign_extend(std::uint8_t num) {
            return static_cast<std::int16_t>(static_cast<std::int8_t>(num));
        }
    } // namespace

    void Emulator::load_memfile(std::istream &strm) {
        std::string line;

    next:
        while (std::getline(strm, line)) {
            if (line.size() == 0) continue;

            std::size_t off = 0;
            if (line[off++] != '@') {
                continue;
            }

            std::uint16_t addr = 0;
            for (;;) {
                if (line.size() <= off) goto next;
                char c = line[off++];
                if ('0' <= c && c <= '9') {
                    addr <<= 4;
                    addr |= c - '0';
                } else if ('a' <= c && c <= 'f') {
                    addr <<= 4;
                    addr |= c - 'W';
                } else if ('A' <= c && c <= 'F') {
                    addr <<= 4;
                    addr |= c - '7';
                } else {
                    break;
                }
            }

            for (;;) {
                if (line.size() <= off) goto next;
                if (line[off] != ' ') {
                    break;
                }
                ++off;
            }

            std::uint8_t b = 0;
            for (int i = 0; i < 8; ++i) {
                if (line.size() <= off) goto next;
                char c = line[off++];
                if (c != '0' && c != '1') {
                    goto next;
                }
                b <<= 1;
                b |= c - '0';
            }
            mem[addr] = b;

            for (;;) {
                if (line.size() <= off) goto next;
                if (line[off] != ' ') {
                    break;
                }
                ++off;
            }

            b = 0;
            for (int i = 0; i < 8; ++i) {
                if (line.size() <= off) goto next;
                char c = line[off++];
                if (c != '0' && c != '1') {
                    goto next;
                }
                b <<= 1;
                b |= c - '0';
            }
            mem[addr + 1] = b;
        }
    }

    bool Emulator::should_trap(std::uint16_t addr) const {
        auto bp = std::find(breakpoints.begin(), breakpoints.end(), addr);
        return enable_trap && bp != breakpoints.end();
    }

    std::uint16_t Emulator::clock() {
        std::uint16_t delayed_addr_save = delayed_addr;
        bool is_branch_delayed_save = is_branch_delayed;

        std::uint16_t exec_addr = pc;
        if (is_branch_delayed) {
            exec_addr = delayed_addr;
        }

        if (exec_addr / 2 >= prog.size()) {
            throw ExecutionError("Program finished");
        }

        Inst inst = prog[exec_addr / 2];
        if (!is_branch_delayed && inst.needs_delay_slot()) {
            delayed_addr = pc;
            is_branch_delayed = true;
            exec_addr = pc + 2;
            if (exec_addr / 2 >= prog.size()) {
                throw ExecutionError("Program finished");
            }
            inst = prog[exec_addr / 2];
        } else if (is_branch_delayed) {
            is_branch_delayed = false;
        }

        if (should_trap(exec_addr)) {
            delayed_addr = delayed_addr_save;
            is_branch_delayed = is_branch_delayed_save;
            throw Breakpoint(exec_addr);
        }

        if (enable_exec_history) {
            record_exec_history(ExecHistory::of_change_pc(
                pc, delayed_addr_save, is_branch_delayed_save));
        }

        pc += 2;

        switch (inst.inst) {
        case InstType::NOP:
            break;
#ifdef EXTEND_T
        case InstType::SR4:
            set_register(inst.rd, reg[inst.rs] >> 4);
            break;
#endif
        case InstType::MOV:
            set_register(inst.rd, reg[inst.rs]);
            break;
        case InstType::NOT:
            set_register(inst.rd, ~reg[inst.rs]);
            break;
        case InstType::XOR:
            set_register(inst.rd, reg[inst.rd] ^ reg[inst.rs]);
            break;
        case InstType::ADD:
            set_register(inst.rd, reg[inst.rd] + reg[inst.rs]);
            break;
        case InstType::SUB:
            set_register(inst.rd, reg[inst.rd] - reg[inst.rs]);
            break;
        case InstType::SL8:
            set_register(inst.rd, reg[inst.rs] << 8);
            break;
        case InstType::SR8:
            set_register(inst.rd, reg[inst.rs] >> 8);
            break;
        case InstType::SL:
            set_register(inst.rd, reg[inst.rs] << 1);
            break;
        case InstType::SR:
            set_register(inst.rd, reg[inst.rs] >> 1);
            break;
        case InstType::AND:
            set_register(inst.rd, reg[inst.rd] & reg[inst.rs]);
            break;
        case InstType::OR:
            set_register(inst.rd, reg[inst.rd] | reg[inst.rs]);
            break;
        case InstType::ADDI:
            set_register(inst.rd, reg[inst.rd] + sign_extend(inst.imm));
            break;
        case InstType::ANDI:
            set_register(inst.rd, reg[inst.rd] & inst.imm);
            break;
        case InstType::ORI:
            set_register(inst.rd, reg[inst.rd] | inst.imm);
            break;
        case InstType::LLI:
            set_register(inst.rd, inst.imm);
            break;
        case InstType::LUI:
            set_register(inst.rd, inst.imm << 8);
            break;
        case InstType::SW:
            if (reg[inst.rs] % 2 != 0) {
                throw ExecutionError(
                    "Destination for `sw' instruction is not well aligned");
            }
            {
                std::uint16_t addr = reg[inst.rs];
                set_memory(addr, static_cast<std::uint8_t>(reg[inst.rd] >> 8));
                set_memory(addr + 1,
                           static_cast<std::uint8_t>(reg[inst.rd] & 0xFF));
            }
            break;
        case InstType::LW:
            if (reg[inst.rs] % 2 != 0) {
                throw ExecutionError(
                    "Destination for `lw' instruction is not well aligned");
            }
            {
                std::uint16_t addr = reg[inst.rs];
                std::uint16_t val = static_cast<std::uint16_t>(mem[addr]) << 8;
                val |= static_cast<std::uint16_t>(mem[addr + 1]);
                set_register(inst.rd, val);
            }
            break;
        case InstType::SBU:
            set_memory(reg[inst.rs], static_cast<std::uint8_t>(reg[inst.rd]));
            break;
        case InstType::LBU:
            set_register(inst.rd, static_cast<std::uint8_t>(mem[reg[inst.rs]]));
            break;
        case InstType::BEQZ:
            if (reg[inst.rd] == 0) {
                pc = exec_addr + 2 + sign_extend(inst.imm);
            }
            break;
        case InstType::BNEZ:
            if (reg[inst.rd] != 0) {
                pc = exec_addr + 2 + sign_extend(inst.imm);
            }
            break;
        case InstType::BMI:
            if (reg[inst.rd] >> 15 != 0) {
                pc = exec_addr + 2 + sign_extend(inst.imm);
            }
            break;
        case InstType::BPL:
            if (reg[inst.rd] >> 15 == 0) {
                pc = exec_addr + 2 + sign_extend(inst.imm);
            }
            break;
        case InstType::J:
            pc = exec_addr + 2 + sign_extend(inst.imm);
            break;
        default:
            throw ExecutionError("Unsupported instruction");
        }

        ++clock_count;

        return exec_addr;
    }

    void Emulator::set_breakpoint(std::uint16_t addr) {
        auto pos = std::find(breakpoints.begin(), breakpoints.end(), addr);
        if (pos == breakpoints.end()) {
            breakpoints.push_back(addr);
        }
    }
    void Emulator::remove_breakpoint(std::uint16_t addr) {
        auto pos = std::find(breakpoints.begin(), breakpoints.end(), addr);
        breakpoints.erase(pos);
    }

    std::uint16_t Emulator::reverse_next_clock() {
        if (!enable_exec_history) {
            throw std::logic_error("Emulator::reverse_next_clock is available "
                                   "only if exec_history is enabled");
        }
        for (;;) {
            if (exec_history.empty()) {
                throw std::logic_error("Broken exec_history");
            }
            ExecHistory eh = exec_history.back();
            exec_history.pop_back();
            switch (eh.type) {
            case ExecHistoryType::CHANGE_PC:
                pc = eh.event_info.change_pc.old_pc;
                delayed_addr = eh.event_info.change_pc.old_delayed_addr;
                is_branch_delayed =
                    eh.event_info.change_pc.old_is_branch_delayed;
                goto end;
            case ExecHistoryType::CHANGE_MEM:
                mem[eh.event_info.change_mem.addr] =
                    eh.event_info.change_mem.old_val;
                break;
            case ExecHistoryType::CHANGE_REG:
                reg[eh.event_info.change_reg.regnum] =
                    eh.event_info.change_reg.old_val;
                break;
            }
        }
    end:

        --clock_count;

        // find older pc
        for (auto itr = exec_history.rbegin(), e = exec_history.rend();
             itr != e; ++itr) {
            if ((*itr).type == ExecHistoryType::CHANGE_PC) {
                if (is_branch_delayed) {
                    return (*itr).event_info.change_pc.old_pc + 2;
                } else if ((*itr).event_info.change_pc.old_is_branch_delayed) {
                    return (*itr).event_info.change_pc.old_pc - 2;
                }
                return (*itr).event_info.change_pc.old_pc;
            }
        }
        throw std::runtime_error("Couldn't find last executed address");
    }

    int Emulator::get_estimated_clock_count() const { return clock_count + 3; }
} // namespace exasm
