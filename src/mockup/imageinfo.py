#!/usr/bin/env python
import struct
import sys

data = sys.stdin.buffer.raw.readall()
values = struct.unpack('IIIIII', data[0 : 24])

values = list(values)
values.append(len(data) - 24)

print('imageid={:x}h type={:x}h width={} height={} flags={:x}h datasize={} actualsize={}'.format(*values))

#fname = 'out/{:08x}_{:x}_{}_{}_{:x}_{}.dat'.format(*values)
#
#with open(fname, 'wb') as f:
#    f.write(data[24:])

