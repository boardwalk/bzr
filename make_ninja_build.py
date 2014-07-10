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
                    depend_path = m.group(1)
                    # this is super ghetto too, deal with it
                    if sys.platform == 'win32':
                        depend_path = depend_path.replace('/', '\\')
                    depend_path = os.path.join('include', depend_path)
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

        if sys.platform == 'win32':
            clflags = ' /nologo /Iinclude /FIbasic.h /EHsc'
            linkflags = ' /nologo /subsystem:WINDOWS'

            sdl_dir = os.path.expanduser(r'~\Documents\SDL2-2.0.3')
            clflags += r' /MD /I{}\include'.format(sdl_dir)
            linkflags += r' /libpath:{}\lib\x64 SDL2.lib SDL2main.lib'.format(sdl_dir)

            glew_dir = os.path.expanduser(r'~\Documents\glew-1.10.0')
            clflags += r' /I{}\include'.format(glew_dir)
            linkflags += r' /libpath:{}\lib\Release\x64 OpenGL32.lib glew32.lib'.format(glew_dir)

            if args.release:
                clflags += ' /O2 /GL'
                linkflags += ' /LTCG'
            else:
                clflags += r' /Zi /Fdout\bzr.pdb /FS'
                linkflags += ' /DEBUG'

            if args.headless:
                clflags += ' /DHEADLESS'
            
            n.variable('clflags', clflags)
            n.variable('linkflags', linkflags)
            n.variable('appext', '.exe')

            n.rule('compile', 'cl $clflags /c $in /Fo$out')
            n.rule('header', 'python make_include_file.py $in $out')
            n.rule('link', 'link $linkflags $in /out:$out')
            n.rule('copy', 'cmd /c copy $in $out')

            n.build(r'out\SDL2.dll', 'copy', r'{}\lib\x64\SDL2.dll'.format(sdl_dir))
            n.build(r'out\glew32.dll', 'copy', r'{}\bin\Release\x64\glew32.dll'.format(glew_dir))
            
            n.default(r'out\SDL2.dll out\glew32.dll')
        else:
            cxxflags = ''
            cppflags = ''
            ldflags = ''
            linkcmd = 'clang++ $cppflags $ldflags $in -o $out'

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
            else:
                ldflags += ' -lGL'

            if args.release:
                cppflags += ' -O2 -flto'
                linkcmd += ' && strip -s $out'
            else:
                cppflags += ' -g'

            if args.headless:
                cxxflags += ' -DHEADLESS'

            n.variable('cppflags', cppflags)
            n.variable('cxxflags', cxxflags)
            n.variable('ldflags', ldflags)
            n.variable('appext', '')

            n.rule('compile', 'clang++ $cppflags $cxxflags -c $in -o $out')
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

        n.build(os.path.join('out', 'bzr$appext'), 'link', link_inputs)
        n.default(os.path.join('out', 'bzr$appext'))

if __name__ == '__main__':
    main()
