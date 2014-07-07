#!/usr/bin/env python
import argparse
import ninja_syntax
import os
import re
import subprocess

TOP_DIR = os.path.dirname(os.path.realpath(__file__))

def uniq(inp):
    out = []
    first = True
    prev = None
    for val in inp:
        if first or val != prev:
            out.append(val)
        first = False
        prev = val

    return out

# super fantastically ghetto dependency discovery
def includes(path):
    def collect(path, result):
        with open(os.path.join(TOP_DIR, path)) as f:
            for ln in f:
                m = re.match(r'\s*#include "(.+)"', ln)
                if m:
                    depend_path = os.path.join('src', m.group(1))
                    result.append(depend_path)
                    collect(depend_path, result)

    result = []
    collect(path, result)
    return uniq(sorted(result))

def pkgconfig(*args):
    popen_args = ['pkg-config']
    popen_args.extend(args)
    return subprocess.check_output(popen_args).decode('utf-8').strip()

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--release', action='store_true', default=False)
    args = parser.parse_args()

    with open(os.path.join(TOP_DIR, 'build.ninja'), 'w') as buildfile:
        n = ninja_syntax.Writer(buildfile)
        n.variable('cxx', 'g++')

        cxxflags = ''
        cppflags = ''
        ldflags = ''
        linkcmd = '$cxx $cppflags $ldflags $in -o $out'

        cxxflags += ' -Wall -Wextra -Wformat=2 -Wno-format-nonliteral -Wshadow'
        cxxflags += ' -Wpointer-arith -Wcast-qual -Wno-missing-braces'
        cxxflags += ' -Werror -pedantic-errors'
        cxxflags += ' -Wstrict-aliasing=1'
        cxxflags += ' -Wno-unused-parameter' # annoying error when stubbing things out
        cxxflags += ' -std=c++11'

        cxxflags += ' ' + pkgconfig('--libs', '--static', 'sfml-all')
        ldflags += ' ' + pkgconfig('--libs', '--static', 'sfml-all')

        if args.release:
            cppflags += ' -O2'
            linkcmd += ' && strip -s $out'
        else:
            cppflags += ' -g'

        n.variable('cppflags', cppflags)
        n.variable('cxxflags', cxxflags)
        n.variable('ldflags', ldflags)

        n.rule('compile', '$cxx $cppflags $cxxflags -c $in -o $out')
        n.rule('link', linkcmd)

        link_inputs = []

        for entry in os.listdir(os.path.join(TOP_DIR, 'src')):
            name, ext = os.path.splitext(entry)
            if ext != '.cpp':
                continue
            in_file = os.path.join('src', entry)
            out_file = os.path.join('obj', name + '.o')
            n.build(out_file, 'compile', in_file, includes(in_file))
            link_inputs.append(out_file)

        n.build(os.path.join('out', 'bzr'), 'link', link_inputs)

if __name__ == '__main__':
    main()

