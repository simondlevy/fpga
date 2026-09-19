#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

enum { kRun, kSpk, kSnc, kClr };

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



/* Append `width` bits of `value` (MSB first) at bit offset *pos.
 * `buf` must have been zeroed by the caller. */
static inline void bitpack_put(uint8_t *buf, size_t *pos,
                               uint64_t value, unsigned width)
{
    if (width == 0) {
        return;
    }

    /* keep only the low `width` bits of the value */
    if (width < 64) {
        value &= ((uint64_t)1 << width) - 1;
    }

    size_t p = *pos;
    *pos = p + width;

    while (width > 0) {
        unsigned off   = (unsigned)(p & 7);          /* bit in byte, from MSB */
        unsigned avail = 8 - off;                    /* room left in this byte */
        unsigned n     = (width < avail) ? width : avail;
        unsigned shift = avail - n;                  /* align chunk in the byte */
        uint8_t  chunk = (uint8_t)((value >> (width - n)) & ((1u << n) - 1));

        buf[p >> 3] |= (uint8_t)(chunk << shift);

        p     += n;
        width -= n;
    }
}

static inline size_t bitpack_size(unsigned nbits)
{
    return ((size_t)nbits + 7) / 8;
}

/* pack "u<wa>u<wb>u<wc>": writes ceil((wa+wb+wc)/8) bytes into `out`
 * and returns that count, or 0 if `out_size` is too small. */
static inline size_t bitpack_uuu(uint8_t *out, size_t out_size,
                                 unsigned wa, unsigned wb, unsigned wc,
                                 uint64_t a, uint64_t b, uint64_t c)
{
    size_t nbytes = bitpack_size(wa + wb + wc);
    if (out_size < nbytes) {
        return 0;
    }

    size_t pos = 0;

    memset(out, 0, nbytes);
    bitpack_put(out, &pos, a, wa);
    bitpack_put(out, &pos, b, wb);
    bitpack_put(out, &pos, c, wc);

    return nbytes;
}

static void DumpSpk(const int index, const int charge)
{
    const auto charge_twoscomp =
        charge < 0 ? (1 << kChargeWidth) + charge  : charge;

    uint8_t buf[8];

    size_t n = bitpack_uuu(buf, sizeof buf,
            kOpcodeWidth, kIndexWidth, kChargeWidth,
            kSpk, index, charge_twoscomp);

    for (int k=0; k<n; ++k) {
        printf("x%02X ", buf[n-k-1]);
    }
    printf("\n");
}

int main()
{
    // XOR:
    // cmd: u2u6
    // spk: u2u1s2
    // kClr: write: xC0 
    // kSnc: write: x80 
    // kRun 3: write: x03 
    // kSpk 0 1: write: x48 
    // kSpk 1 1: write: x68 

    // Dronepong:
    // cmd: u2u14
    // spk: u2u1s7
    // kClr: write: x00 xC0 
    // kSnc: write: x00 x80 
    // kRun 1: write: x01 x00 
    // kRun 23: write: x17 x00 
    // kSpk 0 63: write: xC0 x4F 
    // kSpk 1 63: write: xC0 x6F 

    DumpCmd(kClr, 0);
    DumpCmd(kSnc, 0);

#ifdef DRONEPONG
    DumpCmd(kRun, 1);
    DumpCmd(kRun, 23);

    DumpSpk(0, 63);
    DumpSpk(1, 63);
#else
    DumpCmd(kRun, 3);
    DumpSpk(0, 1);
    DumpSpk(1, 1);
#endif

    return 0;
}

