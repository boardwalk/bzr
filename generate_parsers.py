#!/usr/bin/env python

#
# Bael'Zharon's Respite
# Copyright (C) 2014 Daniel Skorupski
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License along
# with this program; if not, write to the Free Software Foundation, Inc.,
# 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
#

import argparse
import json
import os
import re

with open('c++keywords.txt') as f:
    KEYWORDS = set([k.strip() for k in f.readlines()])

PRIMITIVES = {
    'BYTE': ('uint8_t', 'readByte'),
    'WORD': ('uint16_t', 'readShort'),
    'DWORD': ('uint32_t', 'readInt'),
    'QWORD': ('uint64_t', 'readLong'),
    'float': ('float', 'readFloat'),
    'double': ('double', 'readDouble'),
    'PackedWORD': ('uint16_t', 'readPackedShort'),
    'PackedDWORD': ('uint32_t', 'readPackedInt'),
    'String': ('string', 'readString'),
    'WString': ('string', 'readWideString')
}

HEADER_TEMPLATE = """\
#ifndef BZR_PARSERS_{ucname}_H
#define BZR_PARSERS_{ucname}_H

{includes}

class BinReader;

{decl}

#endif
"""

SOURCE_TEMPLATE = """\
#include "parsers/{name}.h"
#include "BinReader.h"

{impl}
"""

def fix_var_name(ent):
    try:
        name = ent['name']
    except KeyError:
        return
    name = re.sub('[^a-zA-Z0-9_]', '_', name)
    if name in KEYWORDS:
        name = name + '_'
    ent['name'] = name

class Struct(object):
    def __init__(self, name, parent, schema):
        self.name = name
        self.parent = parent
        self.children = []
        self.fields = []
        self.source = []
        self.indent = 0
        self.generate_all(schema)

    @property
    def fullname(self):
        if self.parent is None:
            return self.name

        return self.parent.fullname + '::' + self.name

    @property
    def decl(self):
        s = """\
struct {name}
{{
{children}
    read(BinReader& reader);
    {fields}
}};"""

        s = s.format(
            name = self.name,
            children = re.sub('^', '    ', '\n'.join([c.decl for c in self.children]), 0, re.MULTILINE),
            fields = '\n    '.join(self.fields))

        return s

    @property
    def impl(self):
        s = """\
{children}

{fullname}::read(BinReader& reader)
{{
    {source}
}}"""

        s = s.format(
            children = ''.join([c.impl for c in self.children]),
            fullname = self.fullname,
            source = '\n    '.join(self.source))

        return s

    def generate_all(self, schema):
        for ent in schema:
            self.generate(ent)

    def generate(self, ent):
        fix_var_name(ent)
        if ent['what'] == 'field':
            self.generate_field(ent)
        elif ent['what'] == 'vector':
            self.generate_vector(ent)
        elif ent['what'] == 'maskmap':
            self.generate_maskmap(ent)
        elif ent['what'] == 'switch':
            self.generate_switch(ent)
        elif ent['what'] == 'align':
            self.append_source('reader.align();')
        else:
            raise RuntimeError('unknown entity type')

    def generate_field(self, ent):
        pair = PRIMITIVES.get(ent['type'])
        if pair:
            self.fields.append('{type} {name};'.format(type=pair[0], name=ent['name']))
            self.append_source('{name} = reader.{method}();'.format(name=ent['name'], method=pair[1]))
        else:
            includes.add(ent['type'])
            self.fields.append('{type} {name};'.format(**ent))
            self.append_source('{name}.read(reader);'.format(**ent))

    def generate_vector(self, ent):
        typename = ent['name'][0].upper() + ent['name'][1:] + 'Element'
        self.children.append(Struct(typename, self, ent['fields']))
        self.fields.append('vector<{typename}> {name};'.format(typename=typename, name=ent['name']))
        self.append_source('{name}.resize({length});'.format(**ent))
        self.append_source('for(size_t i = 0; i < {length}; i++)'.format(**ent))
        self.append_source('{{')
        self.append_source('    {name}[i].read(reader);'.format(**ent))
        self.append_source('}}')

    def generate_maskmap(self, ent):
        for case in ent['alt']:
            self.append_source('if({name} & {value})'.format(name=ent['name'], value=case['value']))
            self.append_source('{{')
            self.indent += 1
            self.generate_all(case['fields'])
            self.indent -= 1
            self.append_source('}}')

    def generate_switch(self, ent):
        firstCase = True
        for case in ent['alt']:
            if firstCase:
                keyword = 'if'
                firstCase = False
            else:
                keyword = 'else if'
            self.append_source('{keyword}({name} == {value})'.format(keyword=keyword, name=ent['name'], value=case['value']))
            self.append_source('{{')
            self.indent += 1
            self.generate_all(case['fields'])
            self.indent -= 1
            self.append_source('}}')

    def append_source(self, fmt, *args, **kwargs):
        self.source.append('    ' * self.indent + fmt.format(*args, **kwargs))

def write_type(name, schema, outdir):
    structs = []
    fields = []
    source = []

    st = Struct(name, None, schema)

    includes_str = '\n'.join(['#include "parsers/{}.h"'.format(inc) for inc in includes])

    headerstr = HEADER_TEMPLATE.format(ucname=name.upper(), decl=st.decl, includes=includes_str)
    sourcestr = SOURCE_TEMPLATE.format(name=name, impl=st.impl)

    with open(os.path.join(outdir, name + '.h'), 'w') as f:
        f.write(headerstr)
    with open(os.path.join(outdir, name + '.cpp'), 'w') as f:
        f.write(sourcestr)

def main():
    global includes

    parser = argparse.ArgumentParser()
    parser.add_argument('infile')
    parser.add_argument('outdir')
    args = parser.parse_args()

    with open(args.infile) as f:
        schema = json.load(f)

    for name, typ in schema['types'].iteritems():
        includes = set()
        write_type(name, typ, args.outdir)

    for name, msg in schema['messages'].iteritems():
        includes = set()
        write_type('Message' + name, msg, args.outdir)

if __name__ == '__main__':
    main()
