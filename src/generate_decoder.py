from inspect import currentframe
import sys
from inst_reader import *
from metadata import *

def write_reg_type(out, insts):
    out.write('    InstType type;\n')
    out.write('    switch (inst & 0xF) {\n')
    for inst in insts:
        out.write('    case 0b{}:\n'.format(inst['opcode']))
        out.write('        type = InstType::{};\n'.format(inst['name'].upper()))
        out.write('        break;\n')
    out.write('    default:\n')
    out.write('        return Inst::new_with_data(inst);\n')
    out.write('    }\n')
    out.write('    return new_with_reg_reg(type, static_cast<std::uint8_t>((inst >> 8) & 0x7), static_cast<std::uint8_t>((inst >> 5) & 0x7));\n')

def write_imm_type(out, insts):
    out.write('    InstType type;\n')
    out.write('    switch ((inst >> 11) & 0xF) {\n')
    for inst in insts:
        out.write('    case 0b{}:\n'.format(inst['opcode']))
        out.write('        type = InstType::{};\n'.format(inst['name'].upper()))
        out.write('        break;\n')
    out.write('    default:\n')
    out.write('        return Inst::new_with_data(inst);\n')
    out.write('    }\n')
    out.write('    return new_with_reg_imm(type, static_cast<std::uint8_t>((inst >> 8) & 0x7), static_cast<std::uint8_t>(inst  & 0xFF));\n')

if __name__ == '__main__':
    if len(sys.argv) < 2:
        print('output file required.')
        sys.exit(1)

    insts = read_insts(sys.argv[1:-1])

    with open(sys.argv[-1], 'w') as out:
        out.write('if ((inst >> 11) == 0 && ((inst >> 4) & 1) == 0) {\n')
        write_reg_type(out, filter(lambda x: x['type'] == 'reg_arith', insts));
        out.write('} else if ((inst >> 11) == 0 && ((inst >> 4) & 1) == 1) {\n')
        write_reg_type(out, filter(lambda x: x['type'] == 'mem', insts));
        out.write('} else if (((inst >> 15) & 1) == 0) {\n')
        write_imm_type(out, filter(lambda x: x['type'] == 'imm', insts));
        out.write('} else if (((inst >> 15) & 1) == 1) {\n')
        write_imm_type(out, filter(lambda x: x['type'] == 'branch', insts));
        out.write('}\n')
        out.write('return new_with_data(inst);\n')

