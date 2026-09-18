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

    ByteArray reversed_copy(packed_bytes.rbegin(), packed_bytes.rend());

    return reversed_copy;
}


// FPGA stuff ----------------------------------------------------------------

enum { RUN, SPK, SNC, CLR };

static const int kOpcodeWidth = 2;
static const int kOperandWidth = 14;

static BitArray PackUnsigned(const uint8_t opcode)
{
    return Zpad(Int2Bin(opcode), kOperandWidth - kOpcodeWidth);
}

/*
static BitArray PackSigned(const int val, const int size)
{
    // Calculate Two's Complement for negative numbers
    return PackUnsigned(val < 0 ? (1 << size) + val : val, size);
}*/


static ByteArray PackClr()
{
    return BitsToBytes(PackUnsigned(CLR));
}

static ByteArray PackSnc()
{
    return BitsToBytes(PackUnsigned(SNC));
}


static ByteArray PackRun(const int to_run)
{
    ByteArray bytes;
    return bytes;
}

static ByteArray PackSpk(const int opcode_width, const int index_width,
        const int charge_width, const int index, const int charge)
{
    ByteArray bytes;
    return bytes;
}

int main()
{
    /*
    CLR: write: x00 xC0 
    SNC: write: x00 x80 
    RUN 1: write: x01 x00 
    RUN 23: write: x17 x00 
    SPK 0 63: write: xC0 x4F 
    SPK 1 63: write: xC0 x6F 
    */

    DumpBytes(PackClr());
    DumpBytes(PackSnc());
    DumpBytes(PackRun(1));
    DumpBytes(PackRun(23));

    return 0;
}

