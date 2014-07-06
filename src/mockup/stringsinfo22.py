#!/usr/bin/env python
import struct
import sys

data = sys.stdin.buffer.raw.readall()

fileid, next_fileid, unk, count = struct.unpack('=IIBB', data[0 : 10])

print('fileid {:08x}h, next_fileid {:08x}h, unk {:02x}h, count {:02x}h'.format(fileid, next_fileid, unk, count))

off = 10

if count & 0x80:
    count = (count & 0x7F) << 8 | data[off]
    off += 1

for i in range(count):
    sid, slen = struct.unpack('=IB', data[off : off + 5])
    off += 5
    val, = struct.unpack('{}s'.format(slen), data[off: off + slen])
    off += slen
    print('{:08x}h {}'.format(sid, val))

if off != len(data):
    print("OFFSET LENGTH MISMATCH {} {}".format(off, len(data)))
