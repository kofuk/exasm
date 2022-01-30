import sys
from inst_reader import *

if __name__ == '__main__':
    if len(sys.argv) < 2:
        print('output file required.')
        sys.exit(1)

    insts = read_insts(sys.argv[1:-1])

    with open(sys.argv[-1], 'w') as out:
        out.write('if (inst == "{}") return InstType::{};\n'
                  .format(insts[0]['name'], insts[0]['name'].upper()))
        for inst in insts[1:]:
            out.write('else if (inst == "{}") return InstType::{};\n'
                      .format(inst['name'], inst['name'].upper()))
