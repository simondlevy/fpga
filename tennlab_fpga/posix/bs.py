#!/usr/bin/python3

import re
from enum import Enum, IntEnum, auto

class DispatchOpcode(IntEnum):
    RUN = 0
    SPK = auto()
    SNC = auto()
    CLR = auto()


def custom_bitstruct_pack(format_str, opcode_width, index_width, charge_width,
                          opcode, index, charge):

    bit_string = ''

    data_types = 'u', 'u', 's'

    data_values = opcode, index, charge

    data_sizes = opcode_width, index_width, charge_width

    for k in range(3):

        data_type = data_types[k]
        size = data_sizes[k]
        val = data_values[k]

        if data_type == 'u':  # Unsigned Integer
            # Convert to binary and pad with leading zeros to match the specified bit size
            bits = bin(val)[2:].zfill(size)
            
        elif data_type == 's':  # Signed Integer (Two's Complement)
            min_val = -(1 << (size - 1))
            max_val = (1 << (size - 1)) - 1
            if val < 0:
                # Calculate Two's Complement for negative numbers
                val = (1 << size) + val
            bits = bin(val)[2:].zfill(size)
        
        # 3. Concatenate the bits together
        bit_string += bits

    # 4. Pad the final sequence with zeros if it doesn't align to an 8-bit byte
    remainder = len(bit_string) % 8
    if remainder != 0:
        bit_string += '0' * (8 - remainder)
        
    # 5. Group the bits into chunks of 8 and convert them into actual bytes
    packed_bytes = bytearray()
    for i in range(0, len(bit_string), 8):
        byte_chunk = bit_string[i:i+8]
        packed_bytes.append(int(byte_chunk, 2))
        
    return bytes(packed_bytes)

# --- Verification & Example Usage ---
# Packing 4 variables into a tight 3-byte layout (24 bits total)
format_pattern = 'u2u1s7'

values = (2, 1, 7, DispatchOpcode.SPK, 0, 63)
result = custom_bitstruct_pack(format_pattern, *values)
print([('x%02X' % c) for c in result])

values = (2, 1, 7, DispatchOpcode.SPK, 1, 63)
result = custom_bitstruct_pack(format_pattern, *values)
print([('x%02X' % c) for c in result])
