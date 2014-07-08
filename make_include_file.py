#!/usr/bin/env python
import argparse
import os
import sys
import sys

def escape_c_string(s):
    s = s.replace("\n", "\\n")
    s = s.replace("\"", "\\\"")
    return s

parser = argparse.ArgumentParser()
parser.add_argument('infile')
parser.add_argument('outfile')
args = parser.parse_args()
root, ext = os.path.splitext(args.infile)

with open(args.outfile, 'w') as outf:
    outf.write("static const char {}[] =".format(os.path.basename(root)))
    empty = True
    with open(args.infile) as inf: 
        for ln in inf:
            outf.write("\n    \"{}\"".format(escape_c_string(ln)))
            empty = False
    if empty:
        outf.write(" \"\"")
    outf.write(";\n")
