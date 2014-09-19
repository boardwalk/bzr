#!/usr/bin/env python
import argparse
import json
import xml.etree.ElementTree

parser = argparse.ArgumentParser()
parser.add_argument('infile')
parser.add_argument('outfile')
args = parser.parse_args()

tree = xml.etree.ElementTree.parse(args.infile)

def checkattribs(elem, values):
    for attrib in elem.keys():
        if attrib not in values:
            raise RuntimeError('unknown attribute: ' + attrib)

def get(elem, key):
    value = elem.get(key)
    if value is None:
        raise RuntimeError('missing attribute')
    return value

def parse_field(elem):
    checkattribs(elem, ('name', 'type'))
    return {
        'what': 'field',
        'name': get(elem, 'name'),
        'type': get(elem, 'type')
    }

def parse_maskmap(elem):
    def parse_mask(elem):
        assert elem.tag == 'mask'
        checkattribs(elem, ('value'))
        return {
            'value': get(elem, 'value'),
            'fields': parse_struct(elem)
        }

    checkattribs(elem, ('name', 'xor'))
    return {
        'what': 'maskmap',
        'name': get(elem, 'name'),
        'xor': elem.get('xor'), # TODO elide if null
        'alt': [parse_mask(child) for child in elem]
    }

def parse_vector(elem):
    checkattribs(elem, ('name', 'length', 'skip', 'mask'))
    return {
        'what': 'vector',
        'name': get(elem, 'name'),
        'length': get(elem, 'length'),
        'skip': elem.get('skip'), # TODO elide if null
        'mask': elem.get('mask'), # TODO elide if null
        'fields': parse_struct(elem)
    }

def parse_switch(elem):
    def parse_case(elem):
        assert elem.tag == 'case'
        checkattribs(elem, ('value'))
        return {
            'value': get(elem, 'value'),
            'fields': parse_struct(elem)
        }

    checkattribs(elem, ('name'))
    return {
        'what': 'switch',
        'name': get(elem, 'name'),
        'alt': [parse_case(child) for child in elem]
    }

def parse_align(elem):
    checkattribs(elem, ('type'))
    assert elem.get('type') == 'DWORD'
    return {
        'what': 'align'
    }

def parse_elem(elem):
    if elem.tag == 'field':
        return parse_field(elem)
    elif elem.tag == 'maskmap':
        return parse_maskmap(elem)
    elif elem.tag == 'vector':
        return parse_vector(elem)
    elif elem.tag == 'switch':
        return parse_switch(elem)
    elif elem.tag == 'align':
        return parse_align(elem)

    raise RuntimeError('unknown element: ' + elem.tag)

def parse_struct(elem):
    return [parse_elem(child) for child in elem]

types = {}
for elem in tree.findall('./datatypes/type'):
    if elem.get('primitive'):
        continue
    types[elem.get('name')] = parse_struct(elem)

messages = {}
for elem in tree.findall('./messages/message'):
    messages[elem.get('type')] = parse_struct(elem)

with open(args.outfile, 'w') as outf:
    json.dump({'types': types, 'messages': messages}, outf, indent=2)
