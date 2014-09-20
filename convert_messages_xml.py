#!/usr/bin/env python
import argparse
import json
import os
import xml.etree.ElementTree

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

def dump_binf(name, schema, outdir):
    path = os.path.join(outdir, name + '.binf')
    print('Writing {}'.format(path))
    with open(path, 'w') as outf:
        json.dump(schema, outf, indent=2)

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('infile')
    parser.add_argument('outdir')
    args = parser.parse_args()

    tree = xml.etree.ElementTree.parse(args.infile)

    for elem in tree.findall('./datatypes/type'):
        if elem.get('primitive'):
            continue
        dump_binf(get(elem, 'name'), parse_struct(elem), args.outdir)

    for elem in tree.findall('./messages/message'):
        dump_binf('Message' + get(elem, 'type'), parse_struct(elem), args.outdir)

if __name__ == '__main__':
    main()
