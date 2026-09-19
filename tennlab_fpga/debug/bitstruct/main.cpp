#include <stdint.h>
#include <stdio.h>


enum { RUN, SPK, SNC, CLR };

static const int kOpcodeWidth = 2;
static const int kOperandWidth = 14;
static const int kIndexWidth = 1;
static const int kChargeWidth = 7;

static const constexpr int ceil(const int a, const int b)
{
    return (a + b - 1) / b;
}

const constexpr int nbytes = ceil(kOpcodeWidth + kOperandWidth, 8);

static void Dump(const int opcode, const int operand)
{
    uint64_t bits = opcode;

    bits <<= kOperandWidth;

    bits |= operand;

    for (int k=0; k<nbytes; ++k) {
        printf("x%02X ", (int)(bits & 0xFF));
        bits >>= 8;
    }
    printf("\n");
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

    Dump(CLR, 0);
    Dump(SNC, 0);

    Dump(RUN, 1);
    Dump(RUN, 23);

    return 0;
}

