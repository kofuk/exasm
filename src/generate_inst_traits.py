from inspect import currentframe
import sys
from inst_reader import *
from metadata import *

def write_func_is_inst_branch(out, insts):
    write_line_directive(out, currentframe())
    out.write('bool is_inst_branch(InstType ty) {\n')
    out.write('    return\n')

    for inst in insts:
        if inst['type'] == 'branch':
            write_line_directive(out, currentframe())
            out.write('        ty == InstType::{} ||\n'.format(inst['name'].upper()))

    write_line_directive(out, currentframe())
    out.write('        false;\n')
    out.write('}\n')

if __name__ == '__main__':
    if len(sys.argv) < 2:
        print('output file required.')
        sys.exit(1)

    insts = read_insts(sys.argv[1:-1])


    with open(sys.argv[-1], 'w') as out:
        write_line_directive(out, currentframe())
        out.write('namespace {\n')

        write_func_is_inst_branch(out, insts)

        write_line_directive(out, currentframe())
        out.write('} // namespace\n')
