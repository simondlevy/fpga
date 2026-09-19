#include <stdint.h>
#include <stdio.h>
#include <math.h>


enum { RUN, SPK, SNC, CLR };

static const int kOpcodeWidth = 2;
static const int kOperandWidth = 14;
static const int kIndexWidth = 1;
static const int kChargeWidth = 7;

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

    const uint64_t nbytes = (uint8_t)ceil((kOpcodeWidth + kOperandWidth) / 8);

    uint32_t bits = CLR;

    bits <<= kOperandWidth;

    for (uint8_t k=0; k<nbytes; ++k) {
        printf("x%02X ", (uint8_t)(bits & 0xFF));
        bits >>= 8;
    }
    printf("\n");

    return 0;
}

