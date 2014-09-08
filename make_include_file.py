#!/usr/bin/env python
import argparse
import os
import re

def escape_c_string(s):
    s = s.replace("\n", "\\n")
    s = s.replace("\"", "\\\"")
    return s

def process_file(path, outf):
    with open(path) as inf:
        for ln in inf:
            m = re.match('^#include "([^"]+)"$', ln)
            if m:
                include_path = os.path.join('source', m.group(1))
                process_file(include_path, outf)
            else:
                outf.write("\n    \"{}\"".format(escape_c_string(ln)))

parser = argparse.ArgumentParser()
parser.add_argument('infile')
parser.add_argument('outfile')
args = parser.parse_args()
root, ext = os.path.splitext(args.infile)

with open(args.outfile, 'w') as outf:
    outf.write('static const char {}[] ='.format(os.path.basename(root)))
    process_file(args.infile, outf)
    outf.write(";\n")
