#ifndef EMULATOR_HH
#define EMULATOR_HH

#include <array>
#include <istream>
#include <list>
#include <random>
#include <stdexcept>
#include <vector>

#include "asmio.h"

namespace exasm {
    class ExecutionError : public std::runtime_error {
    public:
        ExecutionError(std::string message) : std::runtime_error(message) {}
    };

    class Breakpoint {
        std::uint16_t addr;

    public:
        Breakpoint(std::uint16_t addr) : addr(addr) {}

        std::uint16_t get_addr() const { return addr; };
    };

    enum class ExecHistoryType {
        CHANGE_PC,
        CHANGE_MEM,
        CHANGE_REG,
    };

    class ExecHistory {
    public:
        ExecHistoryType type;
        union {
            struct {
                std::uint16_t old_pc;
                std::uint16_t old_delayed_addr;
                bool old_is_branch_delayed;
            } change_pc;
            struct {
                std::uint16_t addr;
                std::uint8_t old_val;
            } change_mem;
            struct {
                std::uint8_t regnum;
                std::uint16_t old_val;
            } change_reg;
        } event_info;

        static ExecHistory of_change_pc(std::uint16_t old_pc,
                                        std::uint16_t old_delayed_addr,
                                        bool old_is_branch_delayed) {
            ExecHistory eh;
            eh.type = ExecHistoryType::CHANGE_PC;
            eh.event_info.change_pc.old_pc = old_pc;
            eh.event_info.change_pc.old_delayed_addr = old_delayed_addr;
            eh.event_info.change_pc.old_is_branch_delayed =
                old_is_branch_delayed;
            return eh;
        }

        static ExecHistory of_change_mem(std::uint16_t addr, std::uint8_t old_val) {
            ExecHistory eh;
            eh.type = ExecHistoryType::CHANGE_MEM;
            eh.event_info.change_mem.addr = addr;
            eh.event_info.change_mem.old_val = old_val;
            return eh;
        }

        static ExecHistory of_change_reg(std::uint8_t regnum, std::uint16_t old_val) {
            ExecHistory eh;
            eh.type = ExecHistoryType::CHANGE_REG;
            eh.event_info.change_reg.regnum = regnum;
            eh.event_info.change_reg.old_val = old_val;
            return eh;
        }
    };

    class Emulator {
        std::array<std::uint8_t, 0x10000> mem;
        std::vector<Inst> prog;
        std::array<std::uint16_t, 8> reg;

        std::vector<std::uint16_t> breakpoints;
        bool breaked = false;

        std::uint16_t pc = 0;
        bool is_branch_delayed = false;
        std::uint16_t delayed_addr;

        bool enable_exec_history = false;
        std::list<ExecHistory> exec_history;

        void set_pc(std::uint16_t pc) { this->pc = pc; }

        void record_exec_history(ExecHistory &&hist) {
            exec_history.push_back(std::move(hist));
        }

    public:
        Emulator() {
            std::random_device seed_gen;
            std::default_random_engine engine(seed_gen());

            std::uniform_int_distribution<> dist(0, 255);
            for (std::size_t i = 0; i < 0x10000; ++i) {
                mem[i] = dist(engine);
            }

            reg.fill(0);
        }

        void set_enable_exec_history(bool enable) {
            enable_exec_history = enable;
            if (enable_exec_history && !enable) {
                exec_history.clear();
            }
        }

        const std::array<std::uint8_t, 0x10000> &get_memory() const {
            return mem;
        }

        void set_memory(std::uint16_t addr, std::uint8_t val) {
            if (enable_exec_history) {
                record_exec_history(ExecHistory::of_change_mem(addr, mem[addr]));
            }

            mem[addr] = val;
        }

        const std::array<std::uint16_t, 8> &get_register() const { return reg; }

        void set_register(std::uint8_t regnum, std::uint16_t val) {
            if (enable_exec_history) {
                record_exec_history(ExecHistory::of_change_reg(regnum, reg[regnum]));
            }

            reg[regnum] = val;
        }

        void set_program(std::vector<Inst> &&prog) {
            this->prog = std::move(prog);
        }

        void set_program(std::vector<Inst> &prog) { this->prog = prog; }
        const std::vector<Inst> &get_program() const { return this->prog; }

        void load_memfile(std::istream &strm);
        void set_breakpoint(std::uint16_t addr);
        void remove_breakpoint(std::uint16_t addr);

        std::uint16_t clock();

        std::uint16_t reverse_next_clock();
    };
} // namespace exasm

#endif
