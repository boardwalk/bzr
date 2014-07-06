#!/usr/bin/env python
import struct
import sys

data = sys.stdin.buffer.raw.readall()

fileid, = struct.unpack('I', data[0 : 4])
print('fileid {:08x}h'.format(fileid))

def read_varint(data, off):
    val = data[off]
    off += 1

    if val & 0x80:
        val = (val & 0x7F) << 8 | data[off]
        off += 1

    return val, off

def do_int_table(data, off):
    assert data[off] in (1, 2, 3)
    off += 1

    count, off = read_varint(data, off)

    for i in range(count):
        key, value = struct.unpack('II', data[off : off + 8])
        off += 8

        print('{:08x}h {:08x}h'.format(key, value))

    return off

def do_str_table(data, off):
    assert data[off] in (1, 2, 3)
    off += 1

    count, off = read_varint(data, off)

    for i in range(count):
        key, = struct.unpack('I', data[off : off + 4])
        off += 4

        strlen = data[off]
        off += 1

        strval = data[off : off + strlen]
        off += strlen

        print('{:08x}h {}'.format(key, strval))

    return off

off = 4

while off < len(data):
    off = do_int_table(data, off)
    assert off is not None
    off = do_str_table(data, off)

