#include <stdint.h>
#include <stdio.h>

#include <algorithm>
#include <iostream>
#include <vector>

// Bit-twiddling utilities ---------------------------------------------------

typedef uint8_t Bit;

typedef std::vector<Bit> BitArray;

typedef uint8_t Byte;

typedef std::vector<Byte> ByteArray;

static BitArray Int2Bin(const int val)
{
    BitArray reversed_bits;

    auto v = val;

    while (v > 0) {
        reversed_bits.push_back(v & 0x1);
        v >>= 1;
    }

    BitArray bits(reversed_bits.size());

    std::reverse_copy(reversed_bits.begin(), reversed_bits.end(), bits.begin());

    return bits;
}

static BitArray Append(const BitArray a, const BitArray b)
{
    auto c = a;

    c.insert(c.end(), b.begin(), b.end());

    return c;
}

static BitArray Zpad(const BitArray inp, const size_t n)
{
    BitArray zeros;

    while (zeros.size() < n) {
        zeros.push_back(0);
    }

    return Append(inp, zeros);
}

static Byte BitsToByte(const BitArray bits)
{
    const Byte byte = 
        (bits[0] << 7) + 
        (bits[1] << 6) + 
        (bits[2] << 5) + 
        (bits[3] << 4) + 
        (bits[4] << 3) + 
        (bits[5] << 2) + 
        (bits[6] << 1) +
        bits[7];

    return byte;
}

static void DumpBits(const BitArray bits)
{
    for (auto bit : bits) {
        printf("%d", bit);
    }
    printf("\n");
}

static void DumpByte(const Byte byte)
{
    printf("x%02X ", byte);
}

static void DumpBytes(const ByteArray bytes)
{
    for (auto byte : bytes) {
        DumpByte(byte);
    }
    printf("\n");
}

// FPGA stuff ----------------------------------------------------------------

enum { RUN, SPK, SNC, CLR };

static BitArray PackU(const int val, const int size)
{
    return Zpad(Int2Bin(val), size - 2);
}

static BitArray pack_s(const int val, const int size)
{
    // Calculate Two's Complement for negative numbers
    const auto twoscomp = val < 0 ? (1 << size) + val : val;

    BitArray bits;
    return bits;
}

static ByteArray FinishPacking(const BitArray bit_string)
{
    // Pad the final sequence with zeros if it doesn't align to an 8-bit byte
    const auto remainder = bit_string.size() % 8;
    const auto full_bit_string = remainder == 0 ? bit_string :
        Zpad(bit_string, 8 - remainder);

    // Group the bits into chunks of 8 and convert them into actual bytes
    auto packed_bytes = ByteArray();
    for (int i=0; i<bit_string.size(); i += 8) {

        const auto byte_chunk = BitArray (
                bit_string.begin() + i,
                bit_string.begin() + i + 8);
        
        //packed_bytes.Append(int(byte_chunk, 2));
    }

    return packed_bytes;
}

static ByteArray PackSpkCommand(const int opcode_width, const int index_width,
        const int charge_width, const int index, const int charge)
{
    ByteArray bytes;
    return bytes;
}

static ByteArray PackRunCommand(const int opcode_width, const int operand_width,
        const int to_run)
{
    ByteArray bytes;
    return bytes;
}

int main()
{
    auto bits = Zpad(PackU(3, 6), 2);
    DumpBits(bits);
    DumpByte(BitsToByte(bits));
    printf("\n");

    /*
       PackSpkCommand(2, 1, 7, 0, 63);
       PackSpkCommand(2, 1, 7, 1, 63);
       PackRunCommand(2, 6, 3);*/

    return 0;
}
