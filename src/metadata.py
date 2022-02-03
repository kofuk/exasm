import inspect

def write_line_directive(out, frame):
    fi = inspect.getframeinfo(frame)
    out.write('#line {} "{}"\n'.format(fi.lineno + 1, fi.filename))
