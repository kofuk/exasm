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
            if inst['type'] == 'reg_arith':
                if inst['args'] == []:
                    out.write('    out << "00000000 00000000";\n')
                elif inst['args'] == ['rd', 'rs']:
                    out.write('    out << "00000";\n')
                    out.write('    write_reg_num_bin(out, rd);\n')
                    out.write('    out << \' \';\n')
                    out.write('    write_reg_num_bin(out, rs);\n')
                    out.write('    out << "0%s";\n'%(inst['opcode']))
                else:
                    print('Unsupported reg_arith type args: ', inst['args'])
                    sys.exit(1)
            elif inst['type'] == 'imm':
                if inst['args'] != ['rd', 'imm']:
                    print('Unsupported imm type args: ', inst['args'])
                    sys.exit(1)
                out.write('    out << "0%s";\n'%(inst['opcode']))
                out.write('    write_reg_num_bin(out, rd);\n')
                out.write('    out << \' \';\n')
                out.write('    write_imm_bin(out, imm);\n')
            elif inst['type'] == 'branch':
                if inst['args'] == ['rd', 'imm']:
                    out.write('    out << "1%s";\n'%(inst['opcode']))
                    out.write('    write_reg_num_bin(out, rd);\n')
                    out.write('    out << \' \';\n')
                    out.write('    write_imm_bin(out, imm);\n')
                elif inst['args'] == ['imm']:
                    out.write('    out << "1%s000 ";\n'%(inst['opcode']))
                    out.write('    write_imm_bin(out, imm);\n')
                else:
                    print('Unsupported imm type args: ', inst['args'])
                    sys.exit(1)
            elif inst['type'] == 'mem':
                if inst['args'] == ['rd', 'addr']:
                    out.write('    out << "00000";\n')
                    out.write('    write_reg_num_bin(out, rd);\n')
                    out.write('    out << \' \';\n')
                    out.write('    write_reg_num_bin(out, rs);\n')
                    out.write('    out << "1%s";\n'%(inst['opcode']))
                else:
                    print('Unsupported mem type args: ', inst['args'])
                    sys.exit(1)
            else:
                print('Unsupported reg_arith type args: ', inst['args'])
                sys.exit(1)
            out.write('    break;\n')
            out.write('}\n')
