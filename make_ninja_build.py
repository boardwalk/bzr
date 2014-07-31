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
                    base_depend_path = m.group(1)
                    # our include directives use forward slashes, use backslash on windows
                    if sys.platform == 'win32':
                        base_depend_path = base_depend_path.replace('/', '\\')
                    depend_path = os.path.join('include', base_depend_path)
                    # if it doesn't exist in include, try source instead
                    if not os.path.exists(depend_path):
                        depend_path = os.path.join('source', base_depend_path);
                    # if it doesn't exist in either of those, try build instead
                    if not os.path.exists(depend_path):
                        depend_path = os.path.join('build', base_depend_path)
                    result.append(depend_path)
                    collect(depend_path, result)

    result = [] 
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
    parser.add_argument('--oculusvr', action='store_true', default=False)
    args = parser.parse_args()

    with open('build.ninja', 'w') as buildfile:
        n = ninja_syntax.Writer(buildfile)

        if sys.platform == 'win32':
            cppflags = '/nologo /EHsc /Iinclude /Ibuild /D_CRT_SECURE_NO_WARNINGS'
            cxxflags = '/FIbasic.h /W4 /WX'
            cflags = ''
            linkflags = '/nologo'

            sdl_dir = os.path.expanduser(r'~\Documents\SDL2-2.0.3')
            cppflags += r' /MD /I{}\include'.format(sdl_dir)
            linkflags += r' /libpath:{}\lib\x64 SDL2.lib SDL2main.lib'.format(sdl_dir)

            glew_dir = os.path.expanduser(r'~\Documents\glew-1.10.0')
            cppflags += r' /I{}\include'.format(glew_dir)
            linkflags += r' /libpath:{}\lib\Release\x64 OpenGL32.lib glew32.lib'.format(glew_dir)

            jansson_dir = os.path.expanduser(r'~\Documents\jansson-2.6\VisualStudioSolution')
            cppflags += r' /I{}\include'.format(jansson_dir)
            linkflags += r' /libpath:{}\x64\Release jansson.lib advapi32.lib'.format(jansson_dir)

            if args.oculusvr:
                ovr_dir = os.path.expanduser(r'~\Documents\OculusSDK\LibOVR')
                cppflags += r' /I{}\Src /DOVR_OS_WIN32 /DOCULUSVR'.format(ovr_dir)
                # there are VS2010 and 2013 directories too
                # i'm only testing this support with VS 2013 right now
                # attn! libovr requires atls.lib, which is not provided by VS Express
                # the version provided in the WDK does *not* work, you need the full version of VS
                linkflags += r' /libpath:{}\Lib\x64\VS2013 libovr64.lib winmm.lib ws2_32.lib gdi32.lib'.format(ovr_dir)

            if args.release:
                cppflags += ' /O2 /GL'
                linkflags += ' /LTCG'
            else:
                cppflags += r' /Zi /Fdoutput\bzr.pdb'
                # cl complains about write access to the PDB with parallel compilation on VS2013 without /FS
                if os.environ['VisualStudioVersion'] == '12.0':
                   cppflags += ' /FS'
                linkflags += ' /DEBUG'

            if args.release and not args.headless:
               linkflags += ' /subsystem:WINDOWS'
            else:
               linkflags += ' /subsystem:CONSOLE'

            if args.headless:
                cppflags += ' /DHEADLESS'
            
            n.variable('cppflags', cppflags)
            n.variable('cxxflags', cxxflags)
            n.variable('cflags', cflags)
            n.variable('linkflags', linkflags)
            n.variable('appext', '.exe')

            n.rule('c', 'cl $cppflags $cflags /c $in /Fo$out')
            n.rule('cxx', 'cl $cppflags $cxxflags /c $in /Fo$out')
            n.rule('link', 'link $linkflags $in /out:$out')
            n.rule('header', 'python make_include_file.py $in $out')
            n.rule('copy', 'cmd /c copy $in $out')

            n.build(r'output\SDL2.dll', 'copy', r'{}\lib\x64\SDL2.dll'.format(sdl_dir))
            n.build(r'output\glew32.dll', 'copy', r'{}\bin\Release\x64\glew32.dll'.format(glew_dir))
            
            n.default(r'output\SDL2.dll output\glew32.dll')
        else:
            # applied to 'c' rule only
            cflags = ''
            # applied to 'cxx' rule only
            cxxflags = ''
            # applied to everything, including link
            cppflags = ''
            ldflags = ''
            linkextra = ''

            cxxflags += ' -std=c++11'
            cxxflags += ' -include ' + os.path.join('include', 'basic.h')

            cppflags += ' -Iinclude -Ibuild'
            cppflags += ' -Wall -Wextra -Wformat=2 -Wno-format-nonliteral -Wshadow'
            cppflags += ' -Wpointer-arith -Wcast-qual -Wno-missing-braces'
            cppflags += ' -Werror -pedantic-errors'
            cppflags += ' -Wstrict-aliasing=1'
            cppflags += ' -Wno-unused-parameter' # annoying error when stubbing things out

            cppflags += ' ' + execute('sdl2-config', '--cflags')
            ldflags += ' ' + execute('sdl2-config', '--libs')

            if sys.platform == 'darwin':
                ldflags += ' -framework OpenGL'
            else:
                ldflags += ' -lGL'

            if args.release:
                cppflags += ' -O2 -flto'
                if sys.platform == 'darwin':
                    linkextra = ' && strip $out'
                else:
                    linkextra = ' && strip -s $out'
            else:
                cppflags += ' -g'

            if args.headless:
                cxxflags += ' -DHEADLESS'

            n.variable('cppflags', cppflags)
            n.variable('cflags', cflags)
            n.variable('cxxflags', cxxflags)
            n.variable('ldflags', ldflags)
            n.variable('appext', '')

            n.rule('c', 'gcc $cppflags $cflags $flags -c $in -o $out')
            n.rule('cxx', 'g++ $cppflags $cxxflags -c $in -o $out')
            n.rule('link', 'g++ $cppflags $ldflags $in -o $out' + linkextra)
            n.rule('header', './make_include_file.py $in $out')

        link_inputs = []

        for dirpath, dirnames, filenames in os.walk('source'):
            for filename in filenames:
                name, ext = os.path.splitext(filename)
                in_file = os.path.join(dirpath, filename)
                implicit = includes(in_file)
                
                if ext == '.glsl':
                    buildrule = 'header'
                    newext = '.h'
                elif ext == '.cpp':
                    buildrule = 'cxx'
                    newext = '.o'
                    # all c++ is dependent on basic.h
                    implicit.append(os.path.join('include', 'basic.h'))
                elif ext == '.c':
                    buildrule = 'c'
                    newext = '.o'
                else:
                    continue

                out_file = os.path.join(change_top_dir(dirpath, 'build'), name + newext)
                n.build(out_file, buildrule, in_file, implicit)

                if ext == '.cpp' or ext == '.c':
                    link_inputs.append(out_file)

        n.build(os.path.join('output', 'bzr$appext'), 'link', link_inputs)
        n.default(os.path.join('output', 'bzr$appext'))

if __name__ == '__main__':
    main()
