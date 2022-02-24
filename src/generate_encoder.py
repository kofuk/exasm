from inspect import currentframe
import sys
from inst_reader import *
from metadata import *

def write_reg_arith(out, insts):
    for inst in insts:
        out.write('    case InstType::{}:\n'.format(inst['name'].upper()))
        out.write('        return ((rd & 0x7) << 8) | ((rs & 0x7) << 5) | 0b{};\n'.format(inst['opcode']))

def write_mem(out, insts):
    for inst in insts:
        out.write('    case InstType::{}:\n'.format(inst['name'].upper()))
        out.write('        return ((rd & 0x7) << 8) | ((rs & 0x7) << 5) | 0b1{};\n'.format(inst['opcode']))

def write_imm(out, insts):
    for inst in insts:
        out.write('    case InstType::{}:\n'.format(inst['name'].upper()))
        out.write('        return (0b{} << 11) | ((rd & 0x7) << 8) | (std::get<std::uint8_t>(imm) & 0xFF);\n'.format(inst['opcode']))

def write_branch(out, insts):
    for inst in insts:
        out.write('    case InstType::{}:\n'.format(inst['name'].upper()))
        out.write('        return (0b1{} << 11) | ((rd & 0x7) << 8) | (std::get<std::uint8_t>(imm) & 0xFF);\n'.format(inst['opcode']))

if __name__ == '__main__':
    if len(sys.argv) < 2:
        print('output file required.')
        sys.exit(1)

    insts = read_insts(sys.argv[1:-1])

    with open(sys.argv[-1], 'w') as out:
        out.write('switch (std::get<InstType>(inst)) {\n')
        write_reg_arith(out, filter(lambda x: x['type'] == 'reg_arith', insts))
        write_mem(out, filter(lambda x: x['type'] == 'mem', insts))
        write_imm(out, filter(lambda x: x['type'] == 'imm', insts))
        write_branch(out, filter(lambda x: x['type'] == 'branch', insts))
        out.write('}\n')
