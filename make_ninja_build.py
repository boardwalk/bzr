#!/usr/bin/env python
import argparse
import ninja_syntax
import os
import re
import subprocess
import sys

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
                    depend_path = os.path.join('include', m.group(1))
                    result.append(depend_path)
                    collect(depend_path, result)

    result = [os.path.join('include', 'basic.h')] # everything is dependent on basic.h
    collect(path, result)
    return uniq(sorted(result))

def execute(*args):
    return subprocess.check_output(args).decode('utf-8').strip()

def splitall(path):
    result = []
    while path:
        head, tail = os.path.split(path)
        result.insert(0, tail)
        path = head
    return result

def change_top_dir(path, new_top_dir):
    parts = splitall(path)
    return os.path.join(new_top_dir, *parts[1:])

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--release', action='store_true', default=False)
    parser.add_argument('--headless', action='store_true', default=False)
    args = parser.parse_args()

    with open('build.ninja', 'w') as buildfile:
        n = ninja_syntax.Writer(buildfile)
        n.variable('cxx', 'g++')

        cxxflags = ''
        cppflags = ''
        ldflags = ''
        linkcmd = '$cxx $cppflags $ldflags $in -o $out'

        cxxflags += ' -std=c++11'
        cxxflags += ' -Iinclude'
        cxxflags += ' -include ' + os.path.join('include', 'basic.h')

        cxxflags += ' -Wall -Wextra -Wformat=2 -Wno-format-nonliteral -Wshadow'
        cxxflags += ' -Wpointer-arith -Wcast-qual -Wno-missing-braces'
        cxxflags += ' -Werror -pedantic-errors'
        cxxflags += ' -Wstrict-aliasing=1'
        cxxflags += ' -Wno-unused-parameter' # annoying error when stubbing things out

        cxxflags += ' ' + execute('sdl2-config', '--cflags')
        ldflags += ' ' + execute('sdl2-config', '--libs')

        if sys.platform == 'darwin':
            ldflags += ' -framework OpenGL'
        elif sys.platform.startswith('linux'):
            ldflags += ' -lGL'

        if args.release:
            cppflags += ' -O2'
            linkcmd += ' && strip -s $out'
        else:
            cppflags += ' -g'

        if args.headless:
            cxxflags += ' -DHEADLESS'

        n.variable('cppflags', cppflags)
        n.variable('cxxflags', cxxflags)
        n.variable('ldflags', ldflags)

        n.rule('compile', '$cxx $cppflags $cxxflags -c $in -o $out')
        n.rule('link', linkcmd)
        n.rule('header', './make_include_file.py $in $out')

        link_inputs = []

        for dirpath, dirnames, filenames in os.walk('src'):
            for filename in filenames:
                name, ext = os.path.splitext(filename)
                in_file = os.path.join(dirpath, filename)
                if ext == '.glsl':
                    out_file = os.path.join(change_top_dir(dirpath, 'include'), filename + '.h')
                    n.build(out_file, 'header', in_file)
                elif ext == '.cpp':
                    out_file = os.path.join(change_top_dir(dirpath, 'obj'), name + '.o')
                    n.build(out_file, 'compile', in_file, includes(in_file))
                    link_inputs.append(out_file)

        n.build(os.path.join('out', 'bzr'), 'link', link_inputs)
        n.default(os.path.join('out', 'bzr'))

if __name__ == '__main__':
    main()
