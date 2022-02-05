#ifndef EMULATOR_HH
#define EMULATOR_HH

#include <array>
#include <exception>
#include <istream>
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

    class Watchpoint : public std::exception {
        std::uint16_t addr_;
        bool is_read_;

    public:
        Watchpoint(std::uint16_t addr, bool is_read) : addr_(addr), is_read_(is_read) {}

        std::uint16_t get_addr() const { return addr_; }
        bool is_read() const { return is_read_; }
        bool is_write() const { return !is_read_; }
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
                std::uint16_t old_delay_slot_rem;
                std::uint16_t old_branched_pc;
                bool old_is_delay_slot;
            } pc;
            struct {
                std::uint16_t addr;
                std::uint8_t old_val;
            } mem;
            struct {
                std::uint8_t regnum;
                std::uint16_t old_val;
            } reg;
        } event;

        static ExecHistory of_change_pc(std::uint16_t old_pc, int old_delay_slot_rem,
                                        bool old_is_delay_slot, std::uint16_t old_branched_pc) {
            ExecHistory eh;
            eh.type = ExecHistoryType::CHANGE_PC;
            eh.event.pc.old_pc = old_pc;
            eh.event.pc.old_delay_slot_rem = old_delay_slot_rem;
            eh.event.pc.old_branched_pc = old_branched_pc;
            eh.event.pc.old_is_delay_slot = old_is_delay_slot;
            return eh;
        }

        static ExecHistory of_change_mem(std::uint16_t addr, std::uint8_t old_val) {
            ExecHistory eh;
            eh.type = ExecHistoryType::CHANGE_MEM;
            eh.event.mem.addr = addr;
            eh.event.mem.old_val = old_val;
            return eh;
        }

        static ExecHistory of_change_reg(std::uint8_t regnum, std::uint16_t old_val) {
            ExecHistory eh;
            eh.type = ExecHistoryType::CHANGE_REG;
            eh.event.reg.regnum = regnum;
            eh.event.reg.old_val = old_val;
            return eh;
        }
    };

    class WatchpointInfo {
        std::uint16_t addr_;
        int mode_;

    public:
        WatchpointInfo(std::uint16_t addr, bool read, bool write) : addr_(addr) {
            set_mode(read, write);
        }

        std::uint16_t addr() const { return addr_; }

        bool watches_read() const { return ((mode_ >> 1) & 0x1) == 1; }

        bool watches_write() const { return ((mode_ >> 0) & 0x1) == 1; }

        void set_mode(bool read, bool write) {
            mode_ = (static_cast<int>(read) << 1) | static_cast<int>(write);
        }
    };

    class Emulator {
        std::array<std::uint8_t, 0x10000> mem;
        std::vector<Inst> prog;
        std::array<std::uint16_t, 8> reg;

        std::vector<std::uint16_t> breakpoints;
        std::vector<WatchpointInfo> watchpoints;
        bool enable_trap = true;

        std::uint16_t pc = 0;
        bool is_delay_slot = false;
        int delay_slot_rem = 0;
        std::uint16_t branched_pc;

        bool enable_exec_history = false;
        std::vector<ExecHistory> exec_history;

        int clock_count = 0;

        void set_pc(std::uint16_t pc) { this->pc = pc; }

        void record_exec_history(ExecHistory hist) { exec_history.push_back(std::move(hist)); }

        bool should_trap(std::uint16_t addr) const;

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

        const std::array<std::uint8_t, 0x10000> &get_memory() const { return mem; }

        std::uint8_t get_memory_b(std::uint16_t addr) const;
        std::uint16_t get_memory_w(std::uint16_t addr) const;

        void set_memory_b(std::uint16_t addr, std::uint8_t val);
        void set_memory_w(std::uint16_t addr, std::uint16_t val);

        const std::array<std::uint16_t, 8> &get_register() const { return reg; }

        void set_register(std::uint8_t regnum, std::uint16_t val) {
            if (enable_exec_history) {
                record_exec_history(ExecHistory::of_change_reg(regnum, reg[regnum]));
            }

            reg[regnum] = val;
        }

        void set_program(std::vector<Inst> &&prog) { this->prog = std::move(prog); }

        void set_program(std::vector<Inst> &prog) { this->prog = prog; }
        const std::vector<Inst> &get_program() const { return this->prog; }

        void load_memfile(std::istream &strm);
        void set_breakpoint(std::uint16_t addr);
        void remove_breakpoint(std::uint16_t addr);
        void set_watchpoint(std::uint16_t addr, bool read, bool write);
        void remove_watchpoint(std::uint16_t addr);
        void set_enable_trap(bool enable) { enable_trap = enable; }

        std::uint16_t clock();

        std::uint16_t reverse_next_clock();

        int get_estimated_clock_count() const;
    };
} // namespace exasm

#endif
