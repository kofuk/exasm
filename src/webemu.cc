#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <exception>
#include <sstream>

#include "asmio.h"
#include "emulator.h"

namespace {
    struct EmulatorWrapper {
        std::vector<exasm::Inst> prog;
        exasm::Emulator *emu;
        std::uint16_t next_pc;
        bool breakpoint_hit = false;
        std::uint16_t break_addr;

        EmulatorWrapper(exasm::Emulator *emu) : emu(emu) {}
        ~EmulatorWrapper() { delete emu; }
    };
} // namespace

extern "C" {
__attribute__((used)) std::uint16_t get_register_value(EmulatorWrapper *ew, int number) {
    if (number < 0 || 7 < number) {
        return 0;
    }
    return ew->emu->get_register()[number];
}

__attribute__((used)) const std::uint8_t *get_memory(EmulatorWrapper *ew) {
    return ew->emu->get_memory().data();
}

__attribute__((used)) std::uint16_t next_clock(EmulatorWrapper *ew) {
    try {
        std::uint16_t addr;
        if (ew->breakpoint_hit) {
            ew->breakpoint_hit = false;
            ew->emu->set_enable_trap(false);
            addr = ew->emu->clock();
            ew->emu->set_enable_trap(true);
        } else {
            addr = ew->emu->clock();
        }
        ew->next_pc = addr;
        return addr;
    } catch (exasm::ExecutionError &e) {
        std::cerr << e.what() << '\n';
    } catch (exasm::Breakpoint &bp) {
        std::cerr << "Breakpoint hit\n";
        ew->breakpoint_hit = true;
        ew->break_addr = bp.get_addr();
    }
    return 0;
}

__attribute__((used)) void set_register_value(EmulatorWrapper *ew, int number, std::uint16_t val) {
    if (number < 0 || 7 < number) {
        return;
    }
    ew->emu->set_register(number, val);
}

__attribute__((used)) void set_mem_value(EmulatorWrapper *ew, std::uint8_t *new_val,
                                         std::uint16_t n, std::uint16_t base_addr) {
    for (int i = 0; i < n; ++i) {
        if (ew->emu->get_memory()[base_addr + i] != new_val[i]) {
            ew->emu->set_memory(base_addr + i, new_val[i]);
        }
    }
}

__attribute__((used)) char *dump_partial_program(EmulatorWrapper *ew, int offset, int window_size) {
    if (window_size < 0) {
        window_size = 10;
    }

    std::ostringstream ostrm;
    try {
        int begin_addr = static_cast<int>(ew->next_pc) + offset * 2 - window_size;
        if (begin_addr < 0) {
            begin_addr = 0;
        }
        int end_addr = static_cast<int>(ew->next_pc) + offset * 2 + window_size;
        if (end_addr >= 0x10000) {
            end_addr = 0x10000;
        }
        while (end_addr - begin_addr < window_size * 2) {
            if (end_addr < 0x10000) {
                end_addr += 2;
            } else if (begin_addr > 0) {
                begin_addr -= 2;
            }
        }
        const std::array<std::uint8_t, 0x10000> &mem = ew->emu->get_memory();
        for (int i = begin_addr; i < end_addr; i += 2) {
            std::uint16_t bin = (mem[i] << 8) | mem[i + 1];
            exasm::Inst inst = exasm::Inst::decode(bin);
            exasm::write_addr(ostrm, i) << ' ';
            inst.print_bin(ostrm);
            ostrm << " // ";
            inst.print_asm(ostrm);
            ostrm << '\n';
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

__attribute__((used)) char *dump_program(EmulatorWrapper *ew) {
    std::ostringstream ostrm;
    try {
        std::uint16_t addr = 0;
        for (const exasm::Inst &inst : ew->prog) {
            exasm::write_addr(ostrm, addr) << ' ';
            inst.print_bin(ostrm);
            ostrm << " // ";
            inst.print_asm(ostrm);
            ostrm << '\n';
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

__attribute__((used)) void set_breakpoint(EmulatorWrapper *ew, std::uint16_t addr) {
    ew->emu->set_breakpoint(addr);
}

__attribute__((used)) void remove_breakpoint(EmulatorWrapper *ew, std::uint16_t addr) {
    ew->emu->remove_breakpoint(addr);
}

    __attribute__((used)) int get_hit_breakpoint(EmulatorWrapper *ew) {
    if (ew->breakpoint_hit) {
        return static_cast<std::uint32_t>(ew->break_addr);
    }
    return -1;
}

__attribute__((used)) int reverse_next_clock(EmulatorWrapper *ew) {
    try {
        std::uint16_t addr = ew->emu->reverse_next_clock();
        ew->next_pc = addr;
        return static_cast<std::uint32_t>(addr);
    } catch (std::exception &) {
        return -1;
    }
}

__attribute__((used)) EmulatorWrapper *init_emulator(char *memfile, std::size_t memfile_len,
                                                     char *prog, std::size_t prog_len) {
    std::istringstream prog_strm(std::string(prog, prog + prog_len));
    exasm::AsmReader reader(prog_strm);
    exasm::RawAsm raw_asm;
    bool has_error = false;
    while (!reader.finished()) {
        try {
            reader.read_next(raw_asm);
        } catch (const exasm::ParseError &e) {
            std::cerr << e.what() << '\n';
            has_error = true;
            reader.try_recover();
        } catch (const exasm::LinkError &e) {
            std::cerr << e.what() << '\n';
            has_error = true;
            return nullptr;
        }
    }
    if (has_error) {
        return nullptr;
    }

    std::vector<exasm::Inst> insts;
    try {
        insts = raw_asm.get_executable();
    } catch (const exasm::LinkError &e) {
        std::cerr << e.what() << '\n';
        return nullptr;
    }

    auto *emu = new exasm::Emulator;
    emu->set_enable_exec_history(true);

    std::istringstream mem_strm(std::string(memfile, memfile + memfile_len));
    emu->load_memfile(mem_strm);
    emu->set_program(insts);

    auto *ew = new EmulatorWrapper(emu);
    ew->prog = insts;
    ew->next_pc = 0;

    return ew;
}

__attribute__((used)) void destroy_emulator(EmulatorWrapper *ew) {
    delete ew;
}

__attribute__((used)) char *serialize_mem(EmulatorWrapper *ew) {
    std::ostringstream strm;
    strm << "{int i;static unsigned char t[]={";
    for (std::uint8_t val : ew->emu->get_memory()) {
        strm << +val << ',';
    }
    strm << "};for(i=0;i<0x10000;++i)mem[i]=t[i];}\n";

    char *result = new char[strm.str().size() + 1];
    const std::string &&str = strm.str();
    std::copy(str.begin(), str.end(), result);
    result[str.size()] = '\0';
    return result;
}

__attribute__((used)) int get_estimated_clock(EmulatorWrapper *ew) {
    return ew->emu->get_estimated_clock_count();
}
}
