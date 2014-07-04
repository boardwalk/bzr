#!/usr/bin/env python
import argparse
import struct
import sys

# $ hexdump -n 1024 -C data/client_portal.dat
# 00000000  00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00  |................|
# *
# 00000100  00 50 4c 00 00 00 00 01  00 01 00 ff ff 00 2c ce  |.PL...........,.|
# 00000110  0f 0c 00 00 00 b8 54 29  52 01 00 00 00 00 70 cf  |......T)R.....p.|
# 00000120  35 00 00 00 00 2b 00 00  00 00 24 cf 35 00 00 00  |5....+....$.5...|
# 00000130  00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00  |................|
# 00000140  42 54 00 00/00 04 00 00 /00 00 d0 35/01 00 00 00  |BT.........5....|
# 00000150  00 00 00 00/00 74 cf 35//00 24 cf 35/d8 06 00 00  |.....t.5.$.5....|
# 00000160  00 d4 32 02 00 00 00 00  00 00 00 00 00 cd cd cd  |..2.............|
# 00000170  00 00 00 25 6e 00 00 00  00 00 00 00 d2 d7 a7 34  |...%n..........4|
# 00000180  2f 72 46 4c 8a b4 ef 51  4f 85 6f fd 01 1a 00 00  |/rFL...QO.o.....|
# 00000190  00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00  |................|
# *
# 00000400

# 0x130 originally
SECTOR_SIZE_OFFSET = 0x144
# 0x134 originally (+4)
MAX_SIZE_OFFSET = 0x148
# 0x13C originally (+8)
CUR_SIZE_OFFSET = 0x154
# 0x148 originally (+12)
ROOT_DIR_OFFSET = 0x160
# 0x3F without eliding next sector offset
NUM_FILE_LOC = 0x3E # in dwords

#
# struct DirectoryFile
#   u32 subdirs[NUM_FILE_LOC]
#   u32 numfiles
#   FileEntry files[numfiles]
# struct FileEntry
#   u32 unk1 (flags? typically 0x00030000, 0x00020000)
#   u32 id (ascending order)
#   u32 offset
#   u32 length (bytes)
#   u32 unk2
#   u32 unk3 (usually one, other times a small number < 0xFF)
#
# the directories form a sorted tree structure, where
# dir[i] contains files with id > file[i] and id < file[i + 1]
#

def read_dword(fp, off):
    fp.seek(off)
    val, = struct.unpack('I', fp.read(4))
    return val

def read_file(fp, off, sector_size):
    data = b''
    while off:
        off = read_dword(fp, off)
        data += fp.read(sector_size - 4)
    return data

def main():
    parser = argparse.ArgumentParser(description='Inspect a portal.dat or cell.dat')
    parser.add_argument('datfile', type=argparse.FileType('rb'))
    args = parser.parse_args()

    sector_size = read_dword(args.datfile, SECTOR_SIZE_OFFSET)
    max_size = read_dword(args.datfile, MAX_SIZE_OFFSET)
    cur_size = read_dword(args.datfile, CUR_SIZE_OFFSET)
    root_dir = read_dword(args.datfile, ROOT_DIR_OFFSET)

    sys.stderr.write('sector size: {:x}h\n'.format(sector_size))
    sys.stderr.write('max size: {:x}h\n'.format(max_size))
    sys.stderr.write('cur_size: {:x}h\n'.format(cur_size))
    sys.stderr.write('root_dir: {:x}h\n'.format(root_dir))

    root_dir_data = read_file(args.datfile, root_dir, sector_size)

    unk, = struct.unpack('I', root_dir_data[0 : 4])
    sys.stderr.write('unk: {}\n'.format(unk))

    num_file, = struct.unpack('I', root_dir_data[NUM_FILE_LOC * 4 : (NUM_FILE_LOC + 1) * 4])
    sys.stderr.write('num_file: {}\n'.format(num_file))

    assert num_file <= NUM_FILE_LOC

    print("DIRECTORIES")
    nfields = 1
    for i in range(num_file):
        dir_off = i * nfields
        fields = struct.unpack('I' * nfields, root_dir_data[dir_off * 4 : (dir_off + nfields) * 4])
        print(('{:08x} ' * nfields).format(*fields))

    print("FILES")
    nfields = 6
    for i in range(num_file):
        file_off = NUM_FILE_LOC + 1 + i * nfields
        fields = struct.unpack('I' * nfields, root_dir_data[file_off * 4 : (file_off + nfields) * 4])
        print(('{:08x} ' * nfields).format(*fields))

if __name__ == '__main__':
    main()

