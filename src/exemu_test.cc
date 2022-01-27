#include <algorithm>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "asmio.h"
#include "emulator.h"

int main(int argc, char **argv) {
    if (argc < 3) {
        std::cerr << "usage: exemu_test SOURCE EXPECTS\n";
        return 1;
    }

    std::ifstream in(argv[1]);
    if (!in) {
        std::cerr << "Can't open source file.\n";
        return 1;
    }
    exasm::AsmReader reader(in);
    std::vector<exasm::Inst> prog = reader.read_all();

    exasm::Emulator emu;
    emu.set_program(std::move(prog));

    bool finished = false;
    for (int i = 0; i < 1000000; ++i) {
        try {
            emu.clock();
        } catch (const exasm::ExecutionError &e) {
            if (std::string(e.what()) == "Program finished") {
                finished = true;
                break;
            }
            std::cerr << e.what() << '\n';
            return 1;
        }
    }
    if (!finished) {
        std::cerr << "Program not finished\n";
        return 1;
    }

    std::ifstream expects(argv[2]);
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
        unsigned int av = emu.get_memory()[0x30 + i];
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
