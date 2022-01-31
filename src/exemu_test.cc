#include <algorithm>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <cstring>

#include "asmio.h"
#include "emulator.h"

int main(int argc, char **argv) {
    if (argc < 4) {
        std::cerr << "usage: exemu_test SOURCE OPERATION EXPECTS\n";
        return 1;
    }

    std::ifstream in(argv[1]);
    if (!in) {
        std::cerr << "Can't open source file.\n";
        return 1;
    }
    exasm::AsmReader reader(in);
    exasm::RawAsm prog = reader.read_all();

    exasm::Emulator emu;
    emu.set_program(prog.get_executable());

    std::ifstream op(argv[2]);
    if (!op) {
        std::cerr << "Can't open operation file.\n";
        return 1;
    }

    for (;;) {
        std::string current_op;
        for (;;) {
            std::getline(op, current_op);
            if (!op) {
                goto finish;
            }
            if (current_op.size() != 0) {
                break;
            }
        }

        try {
            if (current_op == "n") {
                emu.clock();
                emu.set_enable_trap(true);
            } else if (current_op == "rn") {
                emu.reverse_next_clock();
            } else if (current_op.substr(0, 2) == "b ") {
                if (current_op.size() < 3) {
                    std::cerr << "Address expected for break operation.\n";
                    return 1;
                }
                int addr = std::stoi(current_op.substr(2), nullptr, 0);
                emu.set_breakpoint(addr);
            } else if (current_op == "finish") {
                try {
                    clock();
                } catch(const exasm::ExecutionError &e) {
                    if (std::strcmp(e.what(), "Program finished") == 0) {
                        break;
                    }
                    std::cerr << e.what() << '\n';
                    return 1;
                }
            } else {
                std::cerr << "Invalid operation: " << current_op << '\n';
                return 1;
            }
        } catch (const exasm::ExecutionError &e) {
            std::cerr << e.what() << '\n';
            return 1;
        } catch (const exasm::Breakpoint &) {
            emu.set_enable_trap(false);
        }
    }
finish:

    std::ifstream expects(argv[3]);
    if (!expects) {
        std::cerr << "Can't open expects file.\n";
        return 1;
    }
    for (int i = 0;; ++i) {
        std::string e;
        if (!std::getline(expects, e)) {
            break;
        }
        unsigned int ev = std::stoi(e, nullptr, 0);
        unsigned int av = emu.get_memory()[0x100 + i];
        if (ev != av) {
            std::cerr << "Assertion failed:\n";
            std::cerr << "    expects:\n";
            std::cerr << "        " << ev << " (" << e << ")\n";
            std::cerr << "    actual:\n";
            std::cerr << "        " << av << '\n';
            return 1;
        }
    }
}
