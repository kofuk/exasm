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
            out.write('case InstType::%s:\n'%(inst['name'].upper()))
            out.write('    out << "%s"'%(inst['name']))
            for i in range(len(inst['args'])):
                if i != 0:
                    out.write(' ","')
                if inst['args'][i] == 'rd' or inst['args'][i] == 'rs':
                    out.write(' " r" << +%s'%(inst['args'][i]))
                    if i != len(inst['args']) - 1:
                        out.write(' << ')
                elif inst['args'][i] == 'addr':
                    out.write(' " (r" << +rs << ")"')
                elif inst['args'][i] == 'imm' or inst['args'][i] == 'baddr':
                    signed_imm = 'true' if inst['signed_imm'] else 'false'
                    out.write('" ";\n    write_hex(out, std::get<std::uint8_t>(imm), %s);\n'%(signed_imm))
                else:
                    print('Unknown arg type: ', inst['args'][i])
                    sys.exit(1)
            out.write(';\n');
            write_line_directive(out, currentframe())
            out.write('    break;\n');
