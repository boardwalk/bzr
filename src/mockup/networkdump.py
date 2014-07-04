#!/usr/bin/env python
import sys
import io
import ctypes
import struct
import ipaddress
from client import *

class FileHeader(ctypes.Structure):
    _fields_ = [
            ('magic', ctypes.c_uint32),
            ('vermajor', ctypes.c_uint16),
            ('verminor', ctypes.c_uint16),
            ('tzoffset', ctypes.c_uint32),
            ('tsaccuracy', ctypes.c_uint32),
            ('snapshotlen', ctypes.c_uint32),
            ('linktype', ctypes.c_uint32)]

    def check(self):
        assert self.magic == 0xa1b2c3d4 # byte swapping not supported
        assert self.vermajor == 2
        assert self.verminor == 4
        assert self.tzoffset == 0 # always 0
        assert self.tsaccuracy == 0 # always 0
        assert self.linktype == 113 # linux sll

class PacketHeader(ctypes.Structure):
    _fields_ = [
            ('time', ctypes.c_uint32),
            ('subtime', ctypes.c_uint32),
            ('packetlen', ctypes.c_uint32),
            ('untruncatedlen', ctypes.c_uint32)]

    def check(self):
        assert self.packetlen == self.untruncatedlen

class SllHeader(ctypes.Structure):
    _fields_ = [
            ('packettype', ctypes.c_uint16),
            ('arphrd', ctypes.c_uint16),
            ('addrlen', ctypes.c_uint16),
            ('addr', ctypes.c_ubyte * 8),
            ('ethertype', ctypes.c_uint16)]

    def check(self):
        assert self.packettype <= 4
        assert self.addrlen <= 8

class IpHeader(ctypes.Structure):
    _fields_ = [
            ('version_ihl', ctypes.c_ubyte),
            ('dscp_ecn', ctypes.c_ubyte),
            ('totallen', ctypes.c_uint16),
            ('iden', ctypes.c_uint16),
            ('flags_fragoff', ctypes.c_uint16),
            ('ttl', ctypes.c_uint8),
            ('proto', ctypes.c_uint8),
            ('checksum', ctypes.c_uint16),
            ('srcip', ctypes.c_ubyte * 4),
            ('dstip', ctypes.c_ubyte * 4)]

    def check(self):
        assert (self.version_ihl >> 4) == 4 # version
        assert (self.version_ihl & 0xF) == 5 # ihl
        # could check checksum

class UdpHeader(ctypes.Structure):
    _fields_ = [
            ('srcport', ctypes.c_uint16),
            ('dstport', ctypes.c_uint16),
            ('len', ctypes.c_uint16),
            ('checksum', ctypes.c_uint16)]

    def check(self):
        # could check checksum
        pass

def readexact(fp, sz):
    result = b''
    while len(result) < sz:
        data = fp.read(sz - len(result))
        if len(data) == 0:
            raise IOError('unexpected end of file')
        result += data
    return result

def readstruct(fp, ty):
    result = ty()
    data = readexact(fp, ctypes.sizeof(result))
    ctypes.memmove(ctypes.addressof(result), data, len(data))
    return result

def hexdump(data):
    hexpart = ''
    asciipart = ''

    for i in range(len(data)):
        if i != 0 and i % 32 == 0:
            print('{} {}'.format(hexpart, asciipart))
            hexpart = ''
            asciipart = ''

        hexpart = '{}{:02x} '.format(hexpart, data[i])
        asciipart = '{}{}'.format(asciipart, chr(data[i]) if isprint(data[i]) else '.')

    if hexpart:
        for i in range(len(data), len(data) + 32):
            if i % 32 == 0:
                print('{} {}'.format(hexpart, asciipart))
                break

            hexpart = '{}   '.format(hexpart)
            asciipart = '{} '.format(asciipart)

def swapshort(x):
    s = struct.pack('<H', x)
    x, = struct.unpack('>H', s)
    return x

def isprint(c):
    return c > 0x1F and c < 0x7F

def handle_packet(data, clients):
    stm = io.BytesIO(data)

    sllheader = readstruct(stm, SllHeader)
    sllheader.packettype = swapshort(sllheader.packettype)
    assert sllheader.ethertype == 8 # ipv4

    # since we're forwarding these packets, tcpdump picks them up twice
    # only consider the incoming ones
    if sllheader.packettype == 0: # LINUX_SLL_HOST
        return

    ipheader = readstruct(stm, IpHeader)
    ipheader.check()
    assert ipheader.proto == 17 # udp

    udpheader = readstruct(stm, UdpHeader)
    udpheader.srcport = swapshort(udpheader.srcport)
    udpheader.dstport = swapshort(udpheader.dstport)
    udpheader.len = swapshort(udpheader.len)
    assert udpheader.srcport in range(9000, 9014) or udpheader.dstport in range(9000, 9014) # 9000-9013

    payload = readexact(stm, udpheader.len - ctypes.sizeof(udpheader))

    from_server = udpheader.srcport in range(9000, 9014)

    if from_server:
        serverip, serverport = ipheader.srcip, udpheader.srcport
        clientip, clientport = ipheader.dstip, udpheader.dstport
    else:
        serverip, serverport = ipheader.dstip, udpheader.dstport
        clientip, clientport = ipheader.srcip, udpheader.srcport

    # normalize server port
    # we use two ports on the server side, the even port for handshaking and the odd port +1 for normal taffic
    serverport = serverport - serverport % 2

    # we're ignoring clientip because the way we
    # route things makes packets in different directions come from different client ips
    clientname = '{}:{} :{}'.format(ipaddress.IPv4Address(bytes(serverip)), serverport, clientport)

    client = clients.get(clientname)

    if client is None:
        # login servers are setup on ports 9000-9007 and world servers 9008-9013
        if serverport <= 9007:
            client = LoginClient()
        else:
            client = WorldClient()

        clients[clientname] = client

    print('#### {} {} {} ####'.format('server' if from_server else 'client', clientname, udpheader.len))

    hexdump(payload)

    if from_server:
        client.on_server_packet(payload)
    else:
        client.on_client_packet(payload)

    print()

def main():
    clients = {}

    fheader = readstruct(sys.stdin.buffer.raw, FileHeader)
    fheader.check()

    while True:
        try:
            pheader = readstruct(sys.stdin.buffer.raw, PacketHeader)
            pheader.check()
        except IOError:
            break

        data = readexact(sys.stdin.buffer.raw, pheader.packetlen)
        handle_packet(data, clients)

if __name__ == '__main__':
    main()

