#include <cstdint>
#include <cstdio>
#include <fstream>
#include <vector>

#include "asmio.h"
#include "emulator.h"

namespace {
    void pretty_print_mem(const std::array<std::uint8_t, 0x10000> &mem, int start = 0,
                          int end = 0x10000) {
        start &= ~0x7;
        end &= ~0x7;
        for (int i = start; i < end; ++i) {
            if (i % 8 == 0) {
                if (i != 0) {
                    std::cout << '\n';
                }
                std::printf("mem[%04x-%04x]:", i, i + 7);
            }
            std::printf(" %02x", mem[i]);
        }
        std::puts("");
    }

    void pretty_print_reg(const std::array<std::uint16_t, 8> &reg) {
        for (int i = 0; i < 8; ++i) {
            std::printf(" r%d=%04x", i, reg[i]);
        }
        std::puts("");
    }
} // namespace

int main(int argc, char **argv) {
    if (argc < 3) {
        std::cout << "usage: exemu memfile prog\n";
        return 1;
    }

    exasm::Emulator emu;

    std::ifstream memin(argv[1]);
    if (!memin) {
        std::cerr << "Can't open memfile\n";
        return 1;
    }
    emu.load_memfile(memin);
    std::cout << "memfile loaded.\n";

    std::ifstream progin(argv[2]);
    if (!progin) {
        std::cerr << "Can't open prog\n";
        return 1;
    }
    exasm::AsmReader reader(progin);
    std::vector<exasm::Inst> prog;
    try {
        exasm::RawAsm raw_asm = reader.read_all();
        prog = raw_asm.get_executable();
    } catch (const exasm::ParseError &e) {
        std::cout << e.what() << '\n';
        return 1;
    } catch (const exasm::LinkError &e) {
        std::cout << e.what() << '\n';
        return 1;
    }
    std::cout << "program loaded.\n";

    emu.set_program(std::move(prog));

    pretty_print_reg(emu.get_register());
    pretty_print_mem(emu.get_memory(), 0x34, 0x40);

    for (;;) {
        std::string tmp;
        if (!std::getline(std::cin, tmp)) {
            break;
        }

        try {
            std::uint16_t addr = emu.clock();
            std::printf("executed %04x\n", addr);
        } catch (exasm::ExecutionError &e) {
            std::cout << e.what() << '\n';
            return 1;
        } catch (exasm::Breakpoint &bp) {
            std::cout << "Breakpoint hit: addr=" << bp.get_addr() << '\n';
            continue;
        }
        pretty_print_reg(emu.get_register());
        pretty_print_mem(emu.get_memory(), 0x34, 0x40);
    }
}
