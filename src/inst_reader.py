import sys
import json

def read_insts(files):
    insts = []
    for filename in files:
        with open(filename, 'r') as f:
            for inst in json.load(f):
                if inst['type'] == 'remove':
                    insts = [*filter(lambda x: x['name'] != inst['name'], insts)]
                else:
                    for existing_inst in insts:
                        if existing_inst['name'] == inst['name']:
                            print('Duplicate instruction. If you are sure to overwrite it, prepend', file=sys.stderr)
                            print('    {', file=sys.stderr)
                            print('        "name": "{}",'.format(inst['name']), file=sys.stderr)
                            print('        "type": "remove"', file=sys.stderr)
                            print('    },', file=sys.stderr)
                            print('to your ISA definition', file=sys.stderr)
                            sys.exit(1)
                    insts.append(inst)
    return insts

