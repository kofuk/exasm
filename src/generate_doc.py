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

        out.write('''<!doctype html>
<html>
<head>
  <meta charset="utf-8" />
  <title>Instruction Set Manual - WebEmu</title>
  <meta name="viewport" content="width=device-width" />
  <style>
    table {
      border-collapse: collapse;
    }

    td, th {
      border: 1px solid black;
      padding: 5px 3px;
    }

    code {
      padding-inline: 5px;
      background-color: #ddd;
      border-radius: 3px;
    }
    .arg {
      font-style: italic;
    }
  </style>
</head>
<body>
  <h1>Instruction Set Manual</h1>
  <table>
    <thead>
      <tr>
        <th>Name</th><th>Instruction</th><th>Meaning</th><th>Description</th>
      </tr>
    </thead>
    <tbody>
''')

        for inst in insts:
            out.write('      <tr>\n')
            out.write('        <td><code>')
            out.write(inst['name'])
            out.write('</code></td>\n')
            out.write('        <td><code>')
            out.write(inst['name'])
            for i, arg in enumerate(inst['args']):
                if i != 0:
                    out.write(',')
                out.write(' ')
                if arg == 'rd':
                    out.write('<span class="arg">rd</span>')
                elif arg == 'rs':
                    out.write('<span class="arg">rs</span>')
                elif arg == 'imm':
                    out.write('<span class="arg">imm</span>')
                elif arg == 'addr':
                    out.write('(<span class="arg">rs</span>)')
                elif arg == 'baddr':
                    out.write('<span class="arg">imm</span>')
            out.write('</code></td>\n')
            out.write('        <td>')
            out.write(inst['meaning'])
            out.write('</td>\n')
            out.write('        <td>')
            out.write(code_regex.sub(r'<code>\1</code>', inst['doc']))
            out.write('</td>\n')
            out.write('      </tr>\n')

        out.write('''    </tbody>
  </table>
</body>
</html>
''')
