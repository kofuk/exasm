import sys
import re
from inst_reader import *

code_regex = re.compile(r'`([^`]*)`')

if __name__ == '__main__':
    if len(sys.argv) < 2:
        print('output file required.')
        sys.exit(1)

    insts = read_insts(sys.argv[1:-1])

    with open(sys.argv[-1], 'w') as out:
        insts = sorted(insts, key=lambda x: x['name'])

        out.write(r'''\begin{longtable}[h]{llllp{8cm}} \hline
  Name & Opcode & Mnemonic & Meaning & Description \\ \hline''')
        out.write('\n')

        for inst in insts:
            out.write(r'  \verb|')
            out.write(inst['name'])
            out.write(r'| & \verb|')
            if inst['type'] == 'reg_arith':
                out.write('00000ddd sss0')
                out.write(inst['opcode'])
            elif inst['type'] == 'mem':
                out.write('00000ddd sss1')
                out.write(inst['opcode'])
            elif inst['type'] == 'imm':
                out.write('0')
                out.write(inst['opcode'])
                out.write('ddd xxxxxxxx')
            elif inst['type'] == 'branch':
                out.write('1')
                out.write(inst['opcode'])
                out.write('ddd xxxxxxxx')
            out.write(r'| & \verb|')
            out.write(inst['name'])
            for i, arg in enumerate(inst['args']):
                if i != 0:
                    out.write(',')
                out.write(' ')
                if arg == 'rd':
                    out.write('rd')
                elif arg == 'rs':
                    out.write('rs')
                elif arg == 'imm':
                    out.write('imm')
                elif arg == 'addr':
                    out.write('(rs)')
                elif arg == 'baddr':
                    out.write('imm')
            out.write(r'| & ')
            out.write(inst['meaning'])
            out.write(' & ')
            out.write(code_regex.sub(r'\\verb|\1|', inst['doc']))
            out.write(r' \\')
            out.write('\n')

        out.write('\\hline\n')
        out.write(r'\end{longtable}')
        out.write('\n')
