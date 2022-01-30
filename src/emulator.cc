#include <algorithm>
#include <cstdint>
#include <stdexcept>
#include <string>

#include "asmio.h"
#include "emulator.h"
#include "insts.h"

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
        bool is_delay_slot_save = is_delay_slot;
        int delay_slot_rem_save = delay_slot_rem;

        if (is_delay_slot) {
            if (delay_slot_rem == 0) {
                is_delay_slot = false;
                pc = branched_pc;
            } else {
                --delay_slot_rem;
            }
        }
        std::uint16_t exec_addr = pc;

        if (exec_addr / 2 >= prog.size()) {
            throw ExecutionError("Program finished");
        }

        if (should_trap(exec_addr)) {
            is_delay_slot = is_delay_slot_save;
            delay_slot_rem = delay_slot_rem_save;
            throw Breakpoint(exec_addr);
        }

        if (enable_exec_history) {
            record_exec_history(ExecHistory::of_change_pc(
                pc, delay_slot_rem_save, is_delay_slot_save));
        }

        Inst inst = prog[exec_addr / 2];
        pc += 2;

        switch (inst.inst) {
#include "executor.inc"
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
                is_delay_slot = eh.event_info.change_pc.old_is_delay_slot;
                delay_slot_rem = eh.event_info.change_pc.old_delay_slot_rem;
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
                return (*itr).event_info.change_pc.old_pc;
            }
        }
        throw std::runtime_error("Couldn't find last executed address");
    }

    int Emulator::get_estimated_clock_count() const { return clock_count + 3; }
} // namespace exasm
