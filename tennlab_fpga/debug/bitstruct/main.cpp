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

static BitArray AppendBits(const BitArray a, const BitArray b)
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

    return AppendBits(inp, zeros);
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

static ByteArray BitsToBytes(const BitArray bits)
{
    // Pad the final sequence with zeros if it doesn't align to an 8-bit byte
    const auto remainder = bits.size() % 8;
    const auto full_bits = remainder == 0 ? bits :
        Zpad(bits, 8 - remainder);

    // Group the bits into chunks of 8 and convert them into actual bytes
    auto packed_bytes = ByteArray();
    for (int i=0; i<full_bits.size(); i += 8) {

        const auto byte_chunk = BitArray (
                full_bits.begin() + i,
                full_bits.begin() + i + 8);

        packed_bytes.push_back(BitsToByte(byte_chunk));
    }

    return packed_bytes;
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
    return PackU(val < 0 ? (1 << size) + val : val, size);
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
    DumpBytes(BitsToBytes(bits));

    /*
       PackSpkCommand(2, 1, 7, 0, 63);
       PackSpkCommand(2, 1, 7, 1, 63);
       PackRunCommand(2, 6, 3);*/

    return 0;
}
