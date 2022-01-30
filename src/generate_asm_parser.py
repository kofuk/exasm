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
            out.write('    skip_space();\n')

            for i in range(len(inst['args'])):
                if i != 0:
                    out.write('    must_read(\',\', "after operand");\n')
                    out.write('    skip_space();\n')

                if inst['args'][i] == 'rd':
                    out.write('    std::uint8_t rd = read_reg("destination");\n')
                elif inst['args'][i] == 'rs':
                    out.write('    std::uint8_t rs = read_reg("source");\n')
                elif inst['args'][i] == 'imm':
                    signed_imm = 'true' if inst['signed_imm'] else 'false'
                    out.write(f'    std::uint8_t imm = read_immediate({signed_imm});\n')
                elif inst['args'][i] == 'addr':
                    out.write('    must_read(\'(\', "before address register");\n')
                    out.write('    skip_space();\n')
                    out.write('    std::uint8_t rs = read_reg("source");\n')
                    out.write('    skip_space();\n')
                    out.write('    must_read(\')\', "before address register");\n')
                out.write('    skip_space();\n')

            out.write('    must_read_newline("after %s");\n'%(
                inst['args'][-1] if len(inst['args']) > 0 else 'instruction'))

            if inst['args'] == []:
                out.write('    return Inst(ty);\n')
            elif inst['args'] == ['rd', 'rs']:
                out.write(f'    return Inst(ty, rd, rs, true);\n')
            elif inst['args'] == ['rd', 'addr']:
                out.write('    return Inst(ty, rd, rs, false);\n')
            elif inst['args'] == ['rd', 'imm']:
                out.write('    return Inst(ty, rd, imm);\n')
            elif inst['args'] == ['imm']:
                out.write('    return Inst(ty, imm);\n')
            else:
                print('Unsupported arg operand: ', inst['args'])
                sys.exit(1)
            out.write('}\n')
