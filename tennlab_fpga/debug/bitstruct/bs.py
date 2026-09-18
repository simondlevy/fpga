#!/usr/bin/python3

RUN, SPK, SNC, CLR = 0, 1, 2, 3


def pack_u(val, size):
    return bin(val)[2:].zfill(size)


def pack_s(val, size):

    # Calculate Two's Complement for negative numbers
    if val < 0:
        val = (1 << size) + val

    return pack_u(val, size)


def finish_packing(bit_string):

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


def pack_spk_command(opcode_width, index_width, charge_width, index, charge):

    return finish_packing(
            pack_u(SPK, opcode_width) +
            pack_u(index, index_width) +
            pack_s(charge, charge_width))


def pack_run_command(opcode_width, operand_width, to_run):

    return finish_packing(
            pack_u(RUN, opcode_width) +
            pack_u(to_run, operand_width))


result = pack_spk_command(2, 1, 7, 0, 63)
print([('x%02X' % c) for c in result])

result = pack_spk_command(2, 1, 7, 1, 63)
print([('x%02X' % c) for c in result])

result = pack_run_command(2, 6, 3)
print([('x%02X' % c) for c in result])
