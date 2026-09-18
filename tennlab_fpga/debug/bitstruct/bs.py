#!/usr/bin/python3

RUN, SPK, SNC, CLR = 0, 1, 2, 3


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


def pack_clr_command(opcode_width, operand_width):

    return bits_to_bytes(
            pack_u(CLR, opcode_width) +
            pack_u(0, operand_width))

def pack_snc_command(opcode_width, operand_width):

    return bits_to_bytes(
            pack_u(SNC, opcode_width) +
            pack_u(0, operand_width))

def pack_run_command(opcode_width, operand_width, to_run):

    return bits_to_bytes(
            pack_u(RUN, opcode_width) +
            pack_u(to_run, operand_width))


def pack_spk_command(opcode_width, index_width, charge_width, index, charge):

    return bits_to_bytes(
            pack_u(SPK, opcode_width) +
            pack_u(index, index_width) +
            pack_s(charge, charge_width))

print('CLR: ', end='')
result = pack_clr_command(2, 14)
print([('x%02X' % c) for c in result])

print('SNC: ', end='')
result = pack_snc_command(2, 14)
print([('x%02X' % c) for c in result])

print('RUN 23: ', end='')
result = pack_run_command(2, 14, 23)
print([('x%02X' % c) for c in result])


#result = pack_spk_command(2, 1, 7, 0, 63)
#print([('x%02X' % c) for c in result])
#result = pack_spk_command(2, 1, 7, 1, 63)
#print([('x%02X' % c) for c in result])


