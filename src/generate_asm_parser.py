from inspect import currentframe
import sys
from inst_reader import *
from metadata import *

if __name__ == '__main__':
    if len(sys.argv) < 2:
        print('output file required.')
        sys.exit(1)

    insts = read_insts(sys.argv[1:-1])

    with open(sys.argv[-1], 'w') as out:
        for inst in insts:
            write_line_directive(out, currentframe())
            out.write('case InstType::%s: {\n'%(inst['name'].upper()))
            out.write('    skip_space();\n')

            for i in range(len(inst['args'])):
                if i != 0:
                    write_line_directive(out, currentframe())
                    out.write('    must_read(\',\', "after operand");\n')
                    out.write('    skip_space();\n')

                if inst['args'][i] == 'rd':
                    write_line_directive(out, currentframe())
                    out.write('    std::uint8_t rd = read_reg("destination");\n')
                elif inst['args'][i] == 'rs':
                    write_line_directive(out, currentframe())
                    out.write('    std::uint8_t rs = read_reg("source");\n')
                elif inst['args'][i] == 'imm':
                    signed_imm = 'true' if inst['signed_imm'] else 'false'
                    write_line_directive(out, currentframe())
                    out.write(f'    std::uint8_t imm = read_immediate({signed_imm});\n')
                elif inst['args'][i] == 'addr':
                    write_line_directive(out, currentframe())
                    out.write('    must_read(\'(\', "before address register");\n')
                    out.write('    skip_space();\n')
                    out.write('    std::uint8_t rs = read_reg("source");\n')
                    out.write('    skip_space();\n')
                    out.write('    must_read(\')\', "before address register");\n')
                elif inst['args'][i] == 'baddr':
                    write_line_directive(out, currentframe())
                    out.write('    std::string imm_label = maybe_read_label();\n')
                    out.write('    if (imm_label.empty()) {\n')
                    out.write('        std::uint8_t baddr = read_immediate(true);\n')
                    out.write('        imm_label = to.add_auto_label(static_cast<std::int8_t>(baddr));\n')
                    out.write('    }\n')
                    out.write('    \n')
                    pass
                write_line_directive(out, currentframe())
                out.write('    skip_space();\n')

            write_line_directive(out, currentframe())
            out.write('    must_read_newline("after %s");\n'%(
                inst['args'][-1] if len(inst['args']) > 0 else 'instruction'))

            if inst['args'] == []:
                write_line_directive(out, currentframe())
                out.write('    to.append(Inst::new_with_type(ty), label);\n')
            elif inst['args'] == ['rd', 'rs']:
                write_line_directive(out, currentframe())
                out.write(f'    to.append(Inst::new_with_reg_reg(ty, rd, rs), label);\n')
            elif inst['args'] == ['rd', 'addr']:
                write_line_directive(out, currentframe())
                out.write('    to.append(Inst::new_with_reg_reg(ty, rd, rs), label);\n')
            elif inst['args'] == ['rd', 'imm']:
                write_line_directive(out, currentframe())
                out.write('    to.append(Inst::new_with_reg_imm(ty, rd, imm), label);\n')
            elif inst['args'] == ['rd', 'baddr']:
                write_line_directive(out, currentframe())
                out.write('    to.append(Inst::new_with_reg_label(ty, rd, imm_label), label);\n')
            elif inst['args'] == ['baddr']:
                write_line_directive(out, currentframe())
                out.write('    to.append(Inst::new_with_label(ty, imm_label), label);\n')
            else:
                print('Unsupported arg operand: ', inst['args'])
                sys.exit(1)
            write_line_directive(out, currentframe())
            out.write('    return;\n')
            out.write('}\n')

