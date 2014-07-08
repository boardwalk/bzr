#!/usr/bin/env python
import argparse
import ninja_syntax
import os
import re
import subprocess

os.chdir(os.path.dirname(os.path.realpath(__file__)))

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
        if not os.path.exists(path):
            return
        with open(path) as f:
            for ln in f:
                m = re.match(r'\s*#include "(.+)"', ln)
                if m:
                    depend_path = os.path.join('src', m.group(1))
                    result.append(depend_path)
                    collect(depend_path, result)

    result = []
    collect(path, result)
    return uniq(sorted(result))

def execute(*args):
    return subprocess.check_output(args).decode('utf-8').strip()

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--release', action='store_true', default=False)
    args = parser.parse_args()

    with open('build.ninja', 'w') as buildfile:
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

        cxxflags += ' ' + execute('sdl2-config', '--cflags')
        ldflags += ' -framework OpenGL ' + execute('sdl2-config', '--libs')

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
        n.rule('header', './make_include_file.py $in $out')

        link_inputs = []

        for entry in os.listdir('shaders'):
            name, ext = os.path.splitext(entry)
            if ext != '.glsl':
                continue
            in_file = os.path.join('shaders', entry)
            out_file = os.path.join('src', name + '.h')
            n.build(out_file, 'header', in_file)

        for entry in os.listdir('src'):
            name, ext = os.path.splitext(entry)
            if ext != '.cpp':
                continue
            in_file = os.path.join('src', entry)
            out_file = os.path.join('obj', name + '.o')
            n.build(out_file, 'compile', in_file, includes(in_file))
            link_inputs.append(out_file)

        n.build(os.path.join('out', 'bzr'), 'link', link_inputs)
        n.default(os.path.join('out', 'bzr'))

if __name__ == '__main__':
    main()
