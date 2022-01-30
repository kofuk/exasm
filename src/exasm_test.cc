#include <cstdlib>
#include <fstream>
#include <iostream>
#include <istream>
#include <sstream>

#include "asmio.h"

int main(int argc, char **argv) {
    if (argc < 3) {
        std::cerr << "usage: exasm_test SOURCE EXPECTS\n";
        return 1;
    }

    std::ifstream in(argv[1]);
    if (!in) {
        std::cerr << "Can't open source file.\n";
        return 1;
    }
    exasm::AsmReader reader(in);

    std::stringstream out;
    std::uint16_t addr = 0;
    try {
        for (exasm::Inst &i : reader.read_all()) {
            exasm::write_addr(out, addr) << ' ';
            i.print_bin(out);
            out << " // ";
            i.print_asm(out);
            out << '\n';
            addr += 2;
        }
    } catch (exasm::ParseError &e) {
        std::cerr << e.what() << '\n';
        return 1;
    }

    std::ifstream expects(argv[2]);
    if (!expects) {
        std::cerr << "Can't open expects file.\n";
        return 1;
    }

    for (;;) {
        std::string e;
        std::string a;
        bool e_read = static_cast<bool>(std::getline(expects, e));
        bool a_read = static_cast<bool>(std::getline(out, a));
        if (!e_read != !a_read) {
            std::cerr << "Line count not match.\n";
            return 1;
        }
        if (!e_read) {
            return 0;
        }

        if (e != a) {
            std::cerr << "Assertion failed:\n";
            std::cerr << "    expects:\n";
            std::cerr << "        " << e << '\n';
            std::cerr << "    actual:\n";
            std::cerr << "        " << a << '\n';
            return 1;
        }
    }
}
