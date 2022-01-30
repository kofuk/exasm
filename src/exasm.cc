#include <cstdlib>
#include <iostream>
#include <istream>

#include "asmio.h"

int main() {
    exasm::AsmReader reader(std::cin);

    std::uint16_t addr = 0;
    try {
        for (exasm::Inst &i : reader.read_all()) {
            exasm::write_addr(std::cout, addr) << ' ';
            i.print_bin(std::cout);
            std::cout << " // ";
            i.print_asm(std::cout);
            std::cout << '\n';
            addr += 2;
        }
    } catch (exasm::ParseError &e) {
        std::cerr << e.what() << '\n';
    }
}
