import json

def read_insts(files):
    insts = []
    for filename in files:
        with open(filename, 'r') as f:
            insts.extend(json.load(f))
    return insts

