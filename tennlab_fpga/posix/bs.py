#!/usr/bin/python3

import re
from enum import Enum, IntEnum, auto

class DispatchOpcode(IntEnum):
    RUN = 0
    SPK = auto()
    SNC = auto()
    CLR = auto()


def custom_bitstruct_pack(format_str, opcode, index, charge):

    # 1. Parse the format string using Regular Expressions
    # Finds pairs like ('u', '1'), ('u', '3'), etc.
    fields = re.findall(r'([usfbtrapP])(\d+)', format_str)

    print(fields)
    
    bit_string = ""
    
    # 2. Process each value according to its type and bit size
    for (data_type, size_str), val in zip(fields, (opcode, index, charge)):

        print(data_type, size_str)

        size = int(size_str)
        
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

values = (DispatchOpcode.SPK, 0, 63)
result = custom_bitstruct_pack(format_pattern, *values)
print([('x%02X' % c) for c in result])

values = (DispatchOpcode.SPK, 1, 63)
result = custom_bitstruct_pack(format_pattern, *values)
print([('x%02X' % c) for c in result])
