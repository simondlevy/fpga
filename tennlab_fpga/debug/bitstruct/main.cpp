#include <stdint.h>
#include <stdio.h>

enum { RUN, SPK, SNC, CLR };

static const int kOpcodeWidth = 2;
static const int kIndexWidth = 1;

//#define DRONEPONG

#ifdef DRONEPONG
static const int kChargeWidth = 7;
static const int kOperandWidth = 14;
#else
static const int kChargeWidth = 2;
static const int kOperandWidth = 6;
#endif


static const constexpr int Ceil(const int a, const int b)
{
    return (a + b - 1) / b;
}

const constexpr int kByteCount = Ceil(kOpcodeWidth + kOperandWidth, 8);

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
    DumpBytes((opcode << kOperandWidth) | operand);
}

static void DumpSpk(const int index, const int charge)
{
    const auto charge_twoscomp =
        charge < 0 ? (1 << kChargeWidth) + charge  : charge;

    DumpBytes(
        (SPK << kOperandWidth) +
        (index << (kOperandWidth-kIndexWidth)) +
        (charge_twoscomp << (kChargeWidth-kIndexWidth)));
}

int main()
{
    // XOR:
    // cmd: u2u6
    // spk: u2u1s2
    // CLR: write: xC0 
    // SNC: write: x80 
    // RUN 3: write: x03 
    // SPK 0 1: write: x48 
    // SPK 1 1: write: x68 

    // Dronepong:
    // cmd: u2u14
    // spk: u2u1s7
    // CLR: write: x00 xC0 
    // SNC: write: x00 x80 
    // RUN 1: write: x01 x00 
    // RUN 23: write: x17 x00 
    // SPK 0 63: write: xC0 x4F 
    // SPK 1 63: write: xC0 x6F 

    DumpCmd(CLR, 0);
    DumpCmd(SNC, 0);

#ifdef DRONEPONG
    DumpCmd(RUN, 1);
    DumpCmd(RUN, 23);

    DumpSpk(0, 63);
    DumpSpk(1, 63);
#else
    DumpCmd(RUN, 3);
    DumpSpk(0, 1);
    DumpSpk(1, 1);
#endif

    return 0;
}

