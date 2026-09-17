#!/usr/bin/python3

RUN, SPK, SNC, CLR = 0, 1, 2, 3

def pack_u(val, size):
    return bin(val)[2:].zfill(size)


def pack_s(val, size):
    if val < 0:
        # Calculate Two's Complement for negative numbers
        val = (1 << size) + val
    return bin(val)[2:].zfill(size)


def pack_spk_command(opcode_width, index_width, charge_width, index, charge):

    bit_string = (
            pack_u(SPK, opcode_width) +
            pack_u(index, index_width) +
            pack_s(charge, charge_width))

    # Pad the final sequence with zeros if it doesn't align to an 8-bit byte
    remainder = len(bit_string) % 8
    if remainder != 0:
        bit_string += '0' * (8 - remainder)

    # Group the bits into chunks of 8 and convert them into actual bytes
    packed_bytes = bytearray()
    for i in range(0, len(bit_string), 8):
        byte_chunk = bit_string[i:i+8]
        packed_bytes.append(int(byte_chunk, 2))

    return bytes(packed_bytes)


result = pack_spk_command(2, 1, 7, 0, 63)
print([('x%02X' % c) for c in result])

result = pack_spk_command(2, 1, 7, 1, 63)
print([('x%02X' % c) for c in result])
