#!/usr/bin/env python
import struct
import sys

data = sys.stdin.buffer.raw.readall()

fileid, = struct.unpack('I', data[0 : 4])

if (fileid >> 16) & 0xFF == 0:
    sys.stderr.write("on fileid {:08x}\n".format(fileid))

topo = struct.unpack('81H', data[8 : 8 + 81*2])

for t in topo:
    print('{:04x}'.format(t))

