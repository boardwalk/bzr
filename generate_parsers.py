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

PRIMITIVES = ['uint8_t', 'uint16_t', 'uint32_t', 'float', 'double']

HEADER_TEMPLATE = """\
#ifndef BZR_PARSERS_{ucname}_H
#define BZR_PARSERS_{ucname}_H

class BinReader;

{decl}

#endif
"""

SOURCE_TEMPLATE = """\
#include "parsers/{name}.h"
#include "BinReader.h"

{impl}
"""

class Struct(object):
    def __init__(self, name, parent, schema):
        self.name = name
        self.parent = parent
        self.children = []
        self.fields = []
        self.source = []

        for ent in schema:
            if ent['what'] == 'field':
                self.fields.append('{type} {name};'.format(**ent))
                if ent['type'] in PRIMITIVES:
                    self.source.append('{name} = reader.read<{type}>();'.format(**ent))
                else:
                    self.source.append('{name} = {type}(reader);'.format(**ent))
            elif ent['what'] == 'vector':
                self.fields.append('vector<{name}Element> {name};'.format(**ent))
                self.source.append('')
                self.source.append('{name}.reserve({length});'.format(**ent))
                self.source.append('for(size_t i = 0; i < {length}; i++)'.format(**ent))
                self.source.append('{')
                self.source.append('    {name}.emplace_back(reader);'.format(**ent))
                self.source.append('}')
                self.source.append('')

                self.children.append(Struct(ent['name'] + 'Element', self, ent['fields']))
            else:
                self.source.append('// TODO {what}'.format(**ent))

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
    {name}(BinReader& reader);
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

{fullname}::{name}(BinReader& reader)
{{
    {source}
}}"""

        s = s.format(
            children = ''.join([c.impl for c in self.children]),
            fullname = self.fullname,
            name = self.name,
            source = '\n    '.join(self.source))

        return s

def write_type(name, schema, outdir):
    structs = []
    fields = []
    source = []


    st = Struct(name, None, schema)

    #for ent in typ:
        #if ent['what'] == 'field':
            #fields.append('{type} {name};'.format(**ent))
            #if ent['type'] in PRIMITIVES:
                #source.append('{name} = reader.read<{type}>();'.format(**ent))
            #else:
                #source.append('{name} = {type}(reader);'.format(**ent))
        #elif ent['what'] == 'vector':
            #build_temp_type(ent['name'] + 'Element', ent['fields'], structs)
            #fields.append('vector<{type}> {name};'.format(type=ent['name'] + 'Element', name=ent['name']))
        #else:
            #source.append('// TODO {}'.format(ent['what']))

    headerstr = HEADER_TEMPLATE.format(ucname=name.upper(), decl=st.decl)
    sourcestr = SOURCE_TEMPLATE.format(name=name, impl=st.impl)

    with open(os.path.join(outdir, name + '.h'), 'w') as f:
        f.write(headerstr)
    with open(os.path.join(outdir, name + '.cpp'), 'w') as f:
        f.write(sourcestr)

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('infile')
    parser.add_argument('outdir')
    args = parser.parse_args()

    with open(args.infile) as f:
        schema = json.load(f)

    #for name, typ in schema['types']:
    #    write_type(name, typ)
    write_type('Position0', schema['types']['Position0'], args.outdir)
    write_type('CharacterOptionData', schema['types']['CharacterOptionData'], args.outdir)

if __name__ == '__main__':
    main()
