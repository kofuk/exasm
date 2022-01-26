#include "asmio.h"
#include "emulator.h"
#include <cstddef>
#include <cstdint>
#include <sstream>

namespace {
    bool breakpoint_hit = false;
    std::uint16_t break_addr;
}

extern "C" {
__attribute__((used)) exasm::Emulator *init_emulator(char *memfile,
                                                     std::size_t memfile_len,
                                                     char *prog,
                                                     std::size_t prog_len) {
    std::istringstream prog_strm(std::string(prog, prog + prog_len));
    exasm::AsmReader reader(prog_strm);
    std::vector<exasm::Inst> insts;
    try {
        insts = reader.read_all();
    } catch (exasm::ParseError &e) {
        std::cerr << e.what() << '\n';
        return nullptr;
    }

    auto *emu = new exasm::Emulator;

    std::istringstream mem_strm(std::string(memfile, memfile + memfile_len));
    emu->load_memfile(mem_strm);
    emu->set_program(insts);

    return emu;
}

__attribute__((used)) std::uint16_t get_register_value(exasm::Emulator *emu,
                                                       int number) {
    if (number < 0 || 7 < number) {
        return 0;
    }
    return emu->get_register()[number];
}

__attribute__((used)) std::uint8_t *get_memory(exasm::Emulator *emu) {
    return emu->get_memory().data();
}

__attribute__((used)) std::uint16_t next_clock(exasm::Emulator *emu) {
    try {
        return emu->clock();
    } catch (exasm::ExecutionError &e) {
        std::cerr << e.what() << '\n';
    } catch (exasm::Breakpoint &bp) {
        std::cerr << "Breakpoint hit\n";
        breakpoint_hit = true;
        break_addr = bp.get_addr();
    }
    return 0;
}

__attribute__((used)) void set_register_value(exasm::Emulator *emu, int number,
                                              std::uint16_t val) {
    if (number < 0 || 7 < number) {
        return;
    }
    emu->get_register()[number] = val;
}

__attribute__((used)) void set_mem_value(exasm::Emulator *emu,
                                         std::uint8_t *new_val, std::uint16_t n,
                                         std::uint16_t base_addr) {
    for (int i = 0; i < n; ++i) {
        emu->get_memory()[base_addr + i] = new_val[i];
    }
}

__attribute__((used)) char *dump_program(exasm::Emulator *emu) {
    std::ostringstream ostrm;
    std::uint16_t addr = 0;
    try {
        for (const exasm::Inst &i : emu->get_program()) {
            exasm::write_addr(ostrm, addr) << ' ';
            i.print_bin(ostrm) << " // ";
            ostrm << i << '\n';
            addr += 2;
        }
    } catch (exasm::ParseError &e) {
        std::cerr << e.what() << '\n';
    }
    std::string out = ostrm.str().c_str();
    char *cstr = new char[out.size() + 1];
    std::copy(out.begin(), out.end(), cstr);
    cstr[out.size()] = '\0';
    return cstr;
}

__attribute__((used)) void set_breakpoint(exasm::Emulator *emu,
                                          std::uint16_t addr) {
    emu->set_breakpoint(addr);
}

__attribute__((used)) void remove_breakpoint(exasm::Emulator *emu,
                                             std::uint16_t addr) {
    emu->remove_breakpoint(addr);
}

__attribute__((used)) int get_hit_breakpoint() {
    if (breakpoint_hit) {
        breakpoint_hit = false;
        return static_cast<std::uint32_t>(break_addr);
    }
    return -1;
}
}
