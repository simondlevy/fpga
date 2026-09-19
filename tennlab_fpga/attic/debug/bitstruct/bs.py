#!/usr/bin/python3

import bitstruct as bs

def bits_to_bytes(bit_string):

    # Pad the final sequence with zeros if it doesn't align to an 8-bit byte
    remainder = len(bit_string) % 8
    if remainder != 0:
        bit_string += '0' * (8 - remainder)

    # Group the bits into chunks of 8 and convert them into actual bytes
    packed_bytes = []
    for i in range(0, len(bit_string), 8):
        byte_chunk = bit_string[i:i+8]
        packed_bytes = [int(byte_chunk, 2)] + packed_bytes

    return bytes(packed_bytes)


def bytes_to_list(bytes):
    return [('x%02X' % c) for c in bytes]

##############################################################################

RUN, SPK, SNC, CLR = 0, 1, 2, 3

OPCODE_WIDTH = 2
OPERAND_WIDTH = 14
INDEX_WIDTH = 1
CHARGE_WIDTH = 7

def pack_unsigned(val, size):
    return bin(val)[OPCODE_WIDTH:].zfill(size)

def pack_signed(val, size):

    # Calculate Two's Complement for negative numbers
    if val < 0:
        val = (1 << size) + val

    return pack_unsigned(val, size)

def pack_clr():

    return bits_to_bytes(
            pack_unsigned(CLR, OPCODE_WIDTH) +
            pack_unsigned(0, OPERAND_WIDTH))

def pack_snc():

    return bits_to_bytes(
            pack_unsigned(SNC, OPCODE_WIDTH) +
            pack_unsigned(0, OPERAND_WIDTH))

def pack_run(to_run):

    return bits_to_bytes(
            pack_unsigned(RUN, OPCODE_WIDTH) +
            pack_unsigned(to_run, OPERAND_WIDTH))


def pack_spk(index, charge):

    return bits_to_bytes(
            pack_unsigned(SPK, OPCODE_WIDTH) +
            pack_unsigned(index, INDEX_WIDTH) +
            pack_signed(charge, CHARGE_WIDTH))

def dump(label, result):
    print(label, ': ', bytes_to_list(result))

def test_run(to_run):
    dump('RUN: %d' % to_run, pack_run(to_run))

def test_spk(idx, val):
    dump('SPK: %d %d' % (idx, val), pack_spk(idx, val))
    packed = bs.pack('u2u1s7', SPK, idx, val)
    print(['x%02X' % byte for byte in packed])
    print("".join(f"{byte:08b}" for byte in packed))

'''
cmd_fmt = u2u14
spk_fmt = u2u1s7

CLR:      write: x00 xC0

SNC:      write: x00 x80

RUN 1:    write: x01 x00
RUN 23:   write: x17 x00

SPK 0 63: write: xC0 x4F
SPK 1 63: write: xC0 x6F

'''

#dump('CLR', pack_clr())
#dump('SNC', pack_snc())

#test_run(1)
#test_run(23)

test_spk(0, 63)
test_spk(1, 63)
