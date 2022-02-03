#include <cstdlib>
#include <iostream>
#include <istream>

#include "asmio.h"

int main() {
    exasm::AsmReader reader(std::cin);

    std::uint16_t addr = 0;
    try {
        exasm::RawAsm raw_asm = reader.read_all();
        for (exasm::Inst &i : raw_asm.get_executable()) {
            exasm::write_addr(std::cout, addr) << ' ';
            i.print_bin(std::cout);
            std::cout << " // ";
            i.print_asm(std::cout);
            std::cout << '\n';
            addr += 2;
        }
    } catch (const exasm::ParseError &e) {
        std::cerr << e.what() << '\n';
    } catch (const exasm::LinkError &e) {
        std::cerr << e.what() << '\n';
    }
}
