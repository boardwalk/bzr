import io
import struct
import time
import binascii

__all__ = ['LoginClient', 'WorldClient']

# packet header
#  u32 sequence
#  u32 flags
#  u32 crc
#  u16 unk1
#  u16 unk2
#  u16 totalSize
#  u16 session

# fragment header
#  u32 sequence1
#  u32 sequence2
#  u16 fragcount
#  u16 fraglen
#  u16 fragindex
#  u16 unk

def expect(stm, expected, name):
    actual = stm.read(len(expected))
    if actual != expected:
        raise RuntimeError('unexpected value for {}, saw {}, expected {}'.format(name, binascii.hexlify(actual), binascii.hexlify(expected)))

def expectword(stm, expected, name):
    expect(stm, struct.pack('H', expected), name)

def expectdword(stm, expected, name):
    expect(stm, struct.pack('I', expected), name)

def expectany(stm, nbytes, name):
    data = stm.read(nbytes)
    if len(data) != nbytes:
        raise RuntimeError('saw {} bytes, expected {}'.format(len(data), nbytes))
    print('{} is {}'.format(name, binascii.hexlify(data)))
    return data

def expectend(stm):
    if stm.tell() != len(stm.getvalue()):
        raise RuntimeError('only {} of {} bytes consumed'.format(stm.tell(), len(stm.getvalue())))

def checksum(data, includesize):
    result = 0

    if includesize:
        result = len(data) << 16

    dwordend = len(data) >> 2 << 2

    for i in range(0, dwordend, 4):
        dw, = struct.unpack('I', data[i : i + 4])
        result = (result + dw) & 0xFFFFFFFF

    for i in range(dwordend, len(data)):
        by = data[i]
        shift = len(data) - i - 1
        result = (result + by << (shift * 8)) & 0xFFFFFFFF

    return result

def checksumpacket(data):
    datacopy = data[0:8] + b'\xDD\x70\xDD\xBA' + data[12:]
    headersum = checksum(datacopy[0:20], True)
    payloadsum = checksum(datacopy[20:], False)
    return struct.pack('I', (headersum + payloadsum) & 0xFFFFFFFF)

def checksumpacket2(data, doheader=True, dopayload=True, headersize=True, bodysize=False):
    datacopy = data[0:8] + b'\xDD\x70\xDD\xBA' + data[12:]
    headersum = checksum(datacopy[0:20], headersize) if doheader else 0
    payloadsum = checksum(datacopy[20:], bodysize) if dopayload else 0
    return struct.pack('I', (headersum + payloadsum) & 0xFFFFFFFF)

def trychecksumpacket2(*args, **kwargs):
    result = checksumpacket2(*args, **kwargs)
    print('calc crc is {}'.format(binascii.hexlify(result)))

def trychecksumpacket3(data):
    trychecksumpacket2(data, True, True, True, True)
    trychecksumpacket2(data, True, True, True, False)
    trychecksumpacket2(data, True, True, False, True)
    trychecksumpacket2(data, True, True, False, False)

    trychecksumpacket2(data, True, False, False, False)
    trychecksumpacket2(data, True, False, True, False)

    trychecksumpacket2(data, False, True, False, True)
    trychecksumpacket2(data, False, True, False, False)

class WorldClient(object):
    def __init__(self):
        self.state = 'clienthello'

    def on_server_packet(self, data):
        if self.state == 'clienthello':
            raise RuntimeError('expected clienthello, got server packet')
        elif self.state == 'serverhello':
            self.handle_serverhello(data)
            self.state = 'connected'
        elif self.state == 'connected':
            self.handle_server_connected(data)
        else:
            raise RuntimeError('unhandled state {}'.format(self.state))

    def on_client_packet(self, data):
        if self.state == 'clienthello':
            self.handle_clienthello(data)
            self.state = 'serverhello'
        elif self.state == 'connected':
            self.handle_client_connected(data)
        else:
            raise RuntimeError('unhandled state {}'.format(self.state))

    def handle_clienthello(self, data):
        pass

    def handle_serverhello(self, data):
        pass

    def handle_server_connected(self, data):
        pass

    def handle_client_connected(self, data):
        pass

class LoginClient(object):
    def __init__(self):
        self.state = 'clienthello'
        self.clientsequence = 0
        self.serversequence = 0

    def on_server_packet(self, data):
        if self.state == 'clienthello':
            raise RuntimeError('expected clienthello, got server packet')
        elif self.state == 'serverhello':
            self.handle_serverhello(data)
            self.state = 'clienthello2'
        elif self.state == 'clienthello2':
            raise RuntimeError('expected clienthello2, got server packet')
        elif self.state == 'halfconnected':
            self.state = 'connected'
            self.clientsequence = 1
            self.serversequence = 1
            return self.on_server_packet(data)
        elif self.state == 'connected':
            self.handle_server_connected(data)
        else:
            raise RuntimeError('unhandled state {}'.format(self.state))

        print('processed server packet, now in state {}'.format(self.state))

    def on_client_packet(self, data):
        if self.state == 'clienthello':
            self.handle_clienthello(data)
            self.state = 'serverhello'
        elif self.state == 'serverhello':
            raise RuntimeError('expected serverhello, got client packet')
        elif self.state == 'clienthello2' or self.state == 'halfconnected':
            self.handle_clienthello2(data)
            self.state = 'halfconnected'
        elif self.state == 'connected':
            self.handle_client_connected(data)
        else:
            raise RuntimeError('unhandled state {}'.format(self.state))

        print('processed client packet, now in state {}'.format(self.state))

    def handle_clienthello(self, data):
        stm = io.BytesIO(data)

        #print('calc crc is {}'.format(binascii.hexlify(checksumpacket(data))))
        #trychecksumpacket3(data)

        # packet header
        expect(stm, b'\x00\x00\x00\x00', 'sequence')
        expect(stm, b'\x00\x00\x01\x00', 'flags')
        expectany(stm, 4, 'crc')
        expect(stm, b'\x00\x00', 'unk1')
        expect(stm, b'\x00\x00', 'unk2')
        expect(stm, b'\x32\x01', 'totalsize') # size beyond header
        expect(stm, b'\x00\x00', 'session') # always 0 in client hello

        # packet contents
        expect(stm, b'\x04\x001802', '1802')
        expect(stm, b'\x00\x00\x26\x01', 'unk4')
        expect(stm, b'\x00\x00\x02\x00', 'unk5')
        expect(stm, b'\x00\x40\x00\x00\x00\x00', 'unk6')

        unixtime, = struct.unpack('I', stm.read(4))
        testunixtime = int(time.time())
        print('unixtime is {}, offset is {}'.format(unixtime, unixtime - testunixtime))

        expect(stm, b'\x19\x00', 'identifier len')
        expectany(stm, 0x19, 'identifier')

        expect(stm, b'\x00\x00\x00\x00', 'unk7')
        expect(stm, b'\x00\xf6\x00\x00', 'unk8')
        expect(stm, b'\x00\x80', 'unk9')

        expect(stm, b'\xf4', 'ticket len')
        expectany(stm, 0xf4, 'ticket') # could check if ticket is base64

        expectend(stm)

    def handle_serverhello(self, data):
        stm = io.BytesIO(data)

        #print('calc crc is {}'.format(binascii.hexlify(checksumpacket(data))))
        #trychecksumpacket3(data)

        # packet header
        expect(stm, b'\x00\x00\x00\x00', 'sequence')
        expect(stm, b'\x00\x00\x04\x00', 'flags')
        expectany(stm, 4, 'crc')
        expect(stm, b'\x0b\x00', 'unk1')
        expect(stm, b'\x00\x00', 'unk2')
        expect(stm, b'\x20\x00', 'totalsize')
        # appears to be a 'session identifier'
        # if you log in several times, you get session++ each time
        # i'm assuming this resets at some point
        # it's probably used to discriminate between AC sessions run on the same host
        self.session = expectany(stm, 2, 'session')

        # packet contents
        expectany(stm, 9, 'unk3')

        # f8b3530b, f9b3530b, fab3530b
        # looks like this increments with every world login?
        # going to the character screen does *not* do it
        expectany(stm, 4, 'unk4')
        expectany(stm, 4, 'unk5')
        expect(stm, b'\x00\x00\x00', 'unk6')
        expectany(stm, 8, 'unk7')
        # 00000000, 07000000, 58890324
        expectany(stm, 4, 'unk8')

        expectend(stm)

    def handle_clienthello2(self, data):
        stm = io.BytesIO(data)

        # packet header
        expect(stm, b'\x00\x00\x00\x00', 'sequence')
        expect(stm, b'\x00\x00\x08\x00', 'flags')
        expectany(stm, 4, 'crc')
        expectany(stm, 2, 'unk1')
        expect(stm, b'\x00\x00', 'unk2')
        expect(stm, b'\x08\x00', 'totalsize')
        expect(stm, self.session, 'session')

        # packet contents
        expectany(stm, 8, 'unk3')

        expectend(stm)

# FIRST CONNECTED SERVER PACKET
# sequence 02 00 00 00
# flags 06 00 00 01
# crc 94 e7 4b 88
# unk1 0b 00
# unk2 02 00
# totalsize d8 00
# session 02 00
#
# extra header bytes? 62 0a 26 49 ca a5 c1 41
#
# sequence1 1f 56 04 00
# sequence2 20 56 0b 03
# fragcount 01 00
# fraglen 78 00
# fragindex 00 00
# unk 09 00
# message type 58 f6 00 00
# unknown 00 00 00 00
# character count 03 00 00 00
# character 0
#  objectid 79 4d 0f 50
#  name 06 00 41 65 72 69 69 6e
#  deletion status 00 00 00 00
# character 1
#  object 7a 4d 0f 50
#  name 05 00 4d 61 64 64 73
#  padding 00
#  deletion status 00 00 00 00
# character 2
#  object fb 4c 0f 50
#  name 06 00 4d 61 64 6c 79 6e
#  deletion status 00 00 00 00
# 00 00 00 00
# 0b 00 00 00
# account 19 00 6d 64 39 6e 71 39 6d 33 6e 6a 78 6a 6c 70 64 63 77 79 34 61 6e 71 64 6e 71
# padding 00
# 01 00 00 00
# 01 00 00 00
#
# sequence1 20 56 04 00
# sequence2 21 56 0b 03
# fragcount 01 00
# fraglen 2c 00 INCLUDES HEADER
# fragindex 00 00
# unk 09 00
# message type e1 f7 00 00
# playercount 85 00 00 00
# unk ff ff ff ff
# server name 0b 00 4d 6f 72 6e 69 6e 67 74 68 61 77
# padding 00 00 00
#
# sequence1 21 56 04 00
# sequence2 22 56 0b 03
# fragcount 01 00
# fraglen 2c 00
# fragindex 00 00
# unk 05 00
# message type e5 f7 00 00
# 01 00 00 00
# 01 00 00 00
# 01 00 00 00
# 02 00 00 00
# 00 00 00 00
# 01 00 00 00

# FIRST CONNECTED CLIENT PACKET
# 02 00 00 00
# 06 40 00 00
# 86 7b 39 d2
# 1c 00
# 01 00
# 64 00
# 01 00
# 02 00 00 00

# sequence1 01 00 00 00
# sequence2 01 00 00 23
# fragcount 01 00
# fraglen 60 00
# fragindex 00 00
# unk 05 00
# message type e6 f7 00 00
# 01 00 00 00 03 00 00 00 00 00 00 00 01 00 00 00 86 06 00 00 7a f9 ff ff 01 00 00 00 01 00 00 00 03
# 00 00 00 54 02 00 00 ac fd ff ff 01 00 00 00 01 00 00 00 02 00 00 00 48 02 00 00 b8 fd ff ff 01 00
# 00 00 00 00 00 00 00 00 00 00
# len74

# 07 00 00 00 sequence
# 06 00 00 08 flags
# 4a ac 70 91 53 00 0c 00 3a 00 06 00 2e 00 00 00 0c 00 04 00 00 00 04 00  ........J.p.S...:...............
# 00 23 01 00 34 00 00 00 04 00 57 f6 00 00 79 4d 0f 50 19 00 6d 64 39 6e 71 39 6d 33 6e 6a 78 6a  .#..4.....W...yM.P..md9nq9m3njxj
# 6c 70 64 63 77 79 34 61 6e 71 64 6e 71 00                                                        lpdcwy4anqdnq.


    def handle_server_connected(self, data):
        stm = io.BytesIO(data)

        self.serversequence += 1

        # packet header
        #expectdword(stm, self.serversequence, 'sequence')
        expectany(stm, 4, 'sequence')
        flags, = struct.unpack('I', expectany(stm, 4, 'flags'))
        expectany(stm, 4, 'crc')
        expectany(stm, 2, 'unk1')
        expectany(stm, 2, 'unk2')
        expectword(stm, len(data) - 20, 'payloadsize')
        expect(stm, self.session, 'session')

        print("FLAGS = {:08x}".format(flags))

        # exact match, for now...
        if flags == 0x01000006:
            # standard fragments packet?
            expectany(stm, 8, 'extra')
        elif flags == 0x00000006:
            # 03 00 00 00 06 00 00 00 27 43 b0 56 0b 00 03 00 14 00 01 00|0d 00 00 00 0e 00 0b 03 01 00 14 00  ........'C.V....................
            # 00 00 05 00 ea f7 00 00                                                                          ........
            pass
        elif flags == 0x08004002:
            pass
        elif flags == 0x0c004002:
            pass
        else:
            print('unknown flags')

    def handle_client_connected(self, data):
        stm = io.BytesIO(data)

        self.clientsequence += 1

        # packet header
        #expectdword(stm, self.clientsequence, 'sequence')
        expectany(stm, 4, 'sequence')
        expectany(stm, 4, 'flags')
        expectany(stm, 4, 'crc')
        expectany(stm, 2, 'unk1')
        expectany(stm, 2, 'unk2')
        expectword(stm, len(data) - 20, 'payloadsize')
        expect(stm, self.session, 'session')

