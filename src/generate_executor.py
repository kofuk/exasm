import sys
from inst_reader import *

if __name__ == '__main__':
    if len(sys.argv) < 2:
        print('output file required.')
        sys.exit(1)

    insts = read_insts(sys.argv[1:-1])

    with open(sys.argv[-1], 'w') as out:
        for inst in insts:
            out.write('case InstType::%s: {\n'%(inst['name'].upper()))
            code = inst['action'] \
                .replace('uword', 'static_cast<std::uint16_t>') \
                .replace('sword', 'sign_extend') \
                .replace('ubyte', 'static_cast<std::uint8_t>') \
                .replace('sbyte', 'static_cast<std::int8_t>') \
                .replace('rd', 'inst.rd') \
                .replace('rs', 'inst.rs') \
                .replace('setreg', 'set_register') \
                .replace('setmem', 'set_memory') \
                .replace('getmem', 'get_memory') \
                .replace('imm', 'inst.imm') \
                .replace('addr', 'reg[inst.rs]')

            if 'word_align' in inst and inst['word_align']:
                out.write('    if (reg[inst.rs] % 2 != 0) {\n')
                out.write('        throw ExecutionError(\n')
                out.write('            "Addess is not well aligned");\n')
                out.write('    }\n')

            if inst['type'] == 'branch':
                out.write(f'    if ({code}) ''{\n')
                out.write('        transaction.emplace_back([&] {\n')
                out.write('            branched_pc = exec_addr + 2 + sign_extend(inst.imm);\n')
                out.write('            delay_slot_rem = 1;\n')
                out.write('            is_delay_slot = true;\n')
                out.write('        });\n')
                out.write('    }\n')
            else:
                out.write('    transaction.emplace_back([&] {%s;});\n'%(code))
            out.write('    break;\n')
            out.write('}\n')
