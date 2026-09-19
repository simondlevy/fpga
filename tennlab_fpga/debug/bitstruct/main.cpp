#include <stdint.h>
#include <stdio.h>


enum { RUN, SPK, SNC, CLR };

// u2u14
// u2u1s7


static const int kOpcodeWidth = 2;
static const int kOperandWidth = 14;
static const int kIndexWidth = 1;
static const int kChargeWidth = 7;

static const constexpr int Ceil(const int a, const int b)
{
    return (a + b - 1) / b;
}

const constexpr int kByteCount = Ceil(kOpcodeWidth + kOperandWidth, 8);

static void DumpBits(uint64_t bits)
{
    const auto nbits = kByteCount * 8;
    uint8_t c[nbits] = {};
    for (int k=0; k<nbits; ++k) {
        c[k] = bits & 0x1;
        bits >>= 1;
    }
    printf("b");
    for (int k=nbits; k>0; --k) {
        printf("%d", c[k-1]);
    }
    printf("\n");
}

static void DumpBytes(uint64_t bits)
{
    for (int k=0; k<kByteCount; ++k) {
        printf("x%02X ", (int)(bits & 0xFF));
        bits >>= 8;
    }
    
    printf("\n");
}

static void DumpCmd(const int opcode, const int operand)
{
    const uint64_t bits = (opcode << kOperandWidth) | operand;

    DumpBytes(bits);
}

static void DumpSpk(const int index, const int charge)
{
    const auto charge_twoscomp =
        charge < 0 ? (1 << kChargeWidth) + charge  : charge;

    const uint64_t bits = (SPK << 14) + (index << 13) + (charge_twoscomp << 6);

    DumpBytes(bits);
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

    //DumpCmd(CLR, 0);
    //DumpCmd(SNC, 0);

    //DumpCmd(RUN, 1);
    //DumpCmd(RUN, 23);

    DumpSpk(0, 63);
    DumpSpk(1, 63);

    return 0;
}

