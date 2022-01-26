#include <algorithm>
#include <cstring>
#include <sstream>
#include <string>

#include <emscripten/emscripten.h>

#include "asmio.h"

extern "C" {
EMSCRIPTEN_KEEPALIVE char *asm_byte_array(const char *arr, std::size_t len) {
    std::istringstream istrm(std::string(arr, arr + len));

    exasm::AsmReader reader(istrm);

    std::ostringstream ostrm;
    std::uint16_t addr = 0;
    try {
        for (exasm::Inst &i : reader.read_all()) {
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
}
