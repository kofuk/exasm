#ifndef EMULATOR_HH
#define EMULATOR_HH

#include <istream>
#include <random>
#include <array>
#include <stdexcept>
#include <vector>

#include "asmio.h"

namespace exasm {
    class ExecutionError: public std::runtime_error {
    public:
        ExecutionError(std::string message): std::runtime_error(message) {}
    };

    class Emulator {
        std::array<std::uint8_t, 0x10000> mem;
        std::vector<Inst> prog;
        std::array<std::uint16_t, 8> reg;

        std::uint16_t pc = 0;
        int delay_slot = -1;
        std::uint16_t branch_addr = 0;
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

        std::array<std::uint8_t, 0x10000> &get_memory() {
            return mem;
        }

        std::array<std::uint16_t, 8> &get_register() {
            return reg;
        }

        void set_program(std::vector<Inst> &&prog) {
            this->prog = std::move(prog);
        }

        void set_program(std::vector<Inst> &prog) {
            this->prog = prog;
        }

        void load_memfile(std::istream &strm);

        std::uint16_t clock();
    };
}

#endif
