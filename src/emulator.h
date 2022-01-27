#ifndef EMULATOR_HH
#define EMULATOR_HH

#include <array>
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

    class Emulator {
        std::array<std::uint8_t, 0x10000> mem;
        std::vector<Inst> prog;
        std::array<std::uint16_t, 8> reg;

        std::vector<std::uint16_t> breakpoints;
        bool breaked = false;

        std::uint16_t pc = 0;
        bool is_branch_delayed = false;
        std::uint16_t delayed_addr;

        void set_pc(std::uint16_t pc) { this->pc = pc; }

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

        const std::array<std::uint8_t, 0x10000> &get_memory() const {
            return mem;
        }

        void set_memory(std::uint16_t addr, std::uint8_t val) {
            mem[addr] = val;
        }

        const std::array<std::uint16_t, 8> &get_register() const { return reg; }

        void set_register(std::uint8_t regnum, std::uint16_t val) {
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
    };
} // namespace exasm

#endif
