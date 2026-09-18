#!/usr/bin/python3

def pack_u(val, size):
    return bin(val)[2:].zfill(size)


def pack_s(val, size):

    # Calculate Two's Complement for negative numbers
    if val < 0:
        val = (1 << size) + val

    return pack_u(val, size)


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

##############################################################################

RUN, SPK, SNC, CLR = 0, 1, 2, 3

OPCODE_WIDTH = 2
OPERAND_WIDTH = 14

def pack_clr():

    return bits_to_bytes(
            pack_u(CLR, OPCODE_WIDTH) +
            pack_u(0, OPERAND_WIDTH))

def pack_snc():

    return bits_to_bytes(
            pack_u(SNC, OPCODE_WIDTH) +
            pack_u(0, OPERAND_WIDTH))

def pack_run(opcode_width, operand_width, to_run):

    return bits_to_bytes(
            pack_u(RUN, opcode_width) +
            pack_u(to_run, operand_width))


def pack_spk(opcode_width, index_width, charge_width, index, charge):

    return bits_to_bytes(
            pack_u(SPK, opcode_width) +
            pack_u(index, index_width) +
            pack_s(charge, charge_width))

def test_run(to_run):
    print('RUN: %d' % to_run, [('x%02X' % c)for c in pack_run(2, 14, to_run)])

def test_spk(idx, val):
    print('SPK: %d %d' % (idx, val), [('x%02X' % c) for c in pack_spk(2, 1, 7, idx, val)])

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

print('CLR: ', [('x%02X' % c) for c in pack_clr()])

print('SNC: ', [('x%02X' % c) for c in pack_snc()])

test_run(1)
test_run(23)

test_spk(0, 63)
test_spk(1, 63)
