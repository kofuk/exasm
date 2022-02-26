import sys
import json

def read_insts(files):
    insts = []
    for filename in files:
        with open(filename, 'r') as f:
            for inst in json.load(f):
                if inst['type'] == 'remove':
                    insts = [*filter(lambda x: x['name'] != inst['name'], insts)]
                elif inst['type'] == 'patch':
                    for i, _ in filter(lambda x: x[1]['name'] == inst['name'], enumerate(insts)):
                        for k, v in inst.items():
                            if k != 'type':
                                insts[i][k] = v
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

