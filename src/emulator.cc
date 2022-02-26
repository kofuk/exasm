#include <algorithm>
#include <cassert>
#include <cstdint>
#include <functional>
#include <ostream>
#include <stdexcept>
#include <string>
#include <variant>
#include <vector>

#include "asmio.h"
#include "emulator.h"
#include "insts.h"

namespace exasm {
    namespace {
        std::int16_t sign_extend(std::uint8_t num) {
            return static_cast<std::int16_t>(static_cast<std::int8_t>(num));
        }

        [[maybe_unused]] std::ostream &operator<<(std::ostream &out, const ExecHistory &eh) {
            switch (eh.type) {
            case ExecHistoryType::CHANGE_PC:
                out << "ExecHistoryType::CHANGE_PC{old_pc: " << eh.event.pc.old_pc
                    << ", old_branched_pc: " << eh.event.pc.old_branched_pc
                    << ", old_delay_slot_rem: " << eh.event.pc.old_delay_slot_rem
                    << ", old_is_delay_slot: " << eh.event.pc.old_is_delay_slot << "}";
                break;
            case ExecHistoryType::CHANGE_REG:
                out << "ExecHistoryType::CHANGE_REG{regnum: " << +eh.event.reg.regnum
                    << ", old_val: " << eh.event.reg.old_val << "}";
                break;
            case ExecHistoryType::CHANGE_MEM:
                out << "ExecHistoryType::CHANGE_MEM{addr: " << eh.event.mem.addr
                    << ", old_val: " << +eh.event.mem.old_val << "}";
                break;
            case ExecHistoryType::CHANGE_STATE:
                out << "ExecHistoryType::CHANGE_STATE{addr: " << eh.event.state.num
                    << ", old_val: " << +eh.event.state.old_val << "}";
                break;
            }
            return out;
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
        std::vector<std::function<void()>> transaction;

        if (is_delay_slot) {
            if (delay_slot_rem == 0) {
                transaction.emplace_back([&] { is_delay_slot = false; });
                pc = branched_pc;
            } else {
                transaction.emplace_back([&] { --delay_slot_rem; });
            }
        }
        std::uint16_t exec_addr = pc;

        if (should_trap(exec_addr)) {
            throw Breakpoint(exec_addr);
        }

        std::uint16_t bin = (mem[exec_addr] << 8) | mem[exec_addr + 1];
        Inst inst = Inst::decode(bin);
        transaction.emplace_back([&] {
            if (enable_exec_history) {
                record_exec_history(
                    ExecHistory::of_change_pc(pc, delay_slot_rem, is_delay_slot, branched_pc));
            }
            pc += 2;
        });

        if (!std::holds_alternative<InstType>(inst.inst)) {
            throw ExecutionError("Illegal instruction (you are about to execute raw word).");
        }

        switch (std::get<InstType>(inst.inst)) {
#include "executor.inc"
        default:
            throw ExecutionError("Unsupported instruction");
        }

        // commit the transaction
        for (const std::function<void()> &f : transaction) {
            f();
        }

        ++clock_count;

        if (is_delay_slot && delay_slot_rem == 0) {
            return branched_pc;
        }
        return pc;
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
                pc = eh.event.pc.old_pc;
                is_delay_slot = eh.event.pc.old_is_delay_slot;
                branched_pc = eh.event.pc.old_branched_pc;
                delay_slot_rem = eh.event.pc.old_delay_slot_rem + 1;
                goto end;
            case ExecHistoryType::CHANGE_MEM:
                mem[eh.event.mem.addr] = eh.event.mem.old_val;
                break;
            case ExecHistoryType::CHANGE_REG:
                reg[eh.event.reg.regnum] = eh.event.reg.old_val;
                break;
            case ExecHistoryType::CHANGE_STATE:
                priv_state[eh.event.state.num] = eh.event.state.old_val;
                break;
            }
        }
    end:

        --clock_count;

        return pc;
    }

    int Emulator::get_estimated_clock_count() const { return clock_count + 3; }
} // namespace exasm
