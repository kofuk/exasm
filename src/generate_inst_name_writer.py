import sys
from inst_reader import *

if __name__ == '__main__':
    if len(sys.argv) < 2:
        print('output file required.')
        sys.exit(1)

    insts = read_insts(sys.argv[1:-1])

    with open(sys.argv[-1], 'w') as out:
        for inst in insts:
            out.write('case InstType::{}: return out << "{}";\n'
                      .format(inst['name'].upper(), inst['name']))
