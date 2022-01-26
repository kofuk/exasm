#include <cstdlib>
#include <iostream>
#include <istream>

#include "asmio.h"

int main() {
    std::uint16_t addr = 0;
    for (exasm::Inst &i : exasm::read_all(std::cin)) {
        exasm::write_addr(std::cout, addr) << ' ';
        i.print_bin(std::cout) << " // ";
        std::cout << i << '\n';
        addr += 2;
    }
}
