#!/usr/bin/env python
import argparse
import os
import re

def process_file(path):
    with open(path) as inf:
        content = inf.read()

    # remove c-style comments
    content = re.sub('/\*.+\*/', '', content, 0, re.DOTALL)

    # remove cpp-style comments
    content = re.sub('//.+', '', content)

    # replace includes
    def include_file(m):
        return process_file(os.path.join('source', m.group(1)))

    content = re.sub('^#include "([^"]+)"$', include_file, content, 0, re.MULTILINE)

    # replace newlines with spaces
    def replace_newline(m):
        return m.group(1) + ('\n' if m.group(1).startswith('#') else ' ')

    content = re.sub('(.*)\n', replace_newline, content)

    # compress extra spaces
    content = re.sub(' +', ' ', content)

    # remove leading and trailing spaces
    content = content.strip()

    return content

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
    varname = os.path.basename(root)
    content = escape_c_string(process_file(args.infile))
    outf.write('static const char {}[] = "{}";\n'.format(varname, content))
