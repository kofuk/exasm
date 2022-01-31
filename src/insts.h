#ifndef INSTS_H
#define INSTS_H

namespace exasm {
    enum class InstType {
#include "inst_type_enum.inc"
    };
} // namespace exasm

#endif
