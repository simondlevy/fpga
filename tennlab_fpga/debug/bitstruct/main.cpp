#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

enum { kRun, kSpk, kSnc, kClr };

static const bool kUseDronepong = true;

static const int kOpcodeWidth = 2;
static const int kIndexWidth = 1;
static const int kChargeWidth = kUseDronepong ? 7 : 2;
static const int kOperandWidth = kUseDronepong ? 14 : 6;


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

static inline size_t bitpack_uu(uint8_t *out, size_t out_size,
                                 unsigned wa, unsigned wb,
                                 uint64_t a, uint64_t b)
{
    size_t nbytes = bitpack_size(wa + wb);
    if (out_size < nbytes) {
        return 0;
    }

    size_t pos = 0;

    memset(out, 0, nbytes);
    bitpack_put(out, &pos, a, wa);
    bitpack_put(out, &pos, b, wb);

    return nbytes;
}


static void DumpBytes(const uint8_t * bytes, const size_t count, const char * target)
{
    printf("target = %s; actual = ", target);
    for (int k=0; k<count; ++k) {
        printf("x%02X ", bytes[count-k-1]);
    }
    printf("\n");
}

static void DumpCmd(const int opcode, const int operand, const char * target)
{
    uint8_t buf[8];

    size_t n = bitpack_uu(buf, sizeof buf,
            kOpcodeWidth, kOperandWidth,
            opcode, operand);

    DumpBytes(buf, n, target);
}


static void DumpSpk(const int index, const int charge, const char * target)
{
    const auto charge_twoscomp =
        charge < 0 ? (1 << kChargeWidth) + charge  : charge;

    uint8_t buf[8];

    size_t n = bitpack_uuu(buf, sizeof buf,
            kOpcodeWidth, kIndexWidth, kChargeWidth,
            kSpk, index, charge_twoscomp);

    DumpBytes(buf, n, target);
}

int main()
{
    if (kUseDronepong) {
        DumpCmd(kClr, 0, "x00 xC0");
        DumpCmd(kSnc, 0, "x00 x80");
        DumpCmd(kRun, 1, "x01 x00");
        DumpCmd(kRun, 23, "x17 x00");
        DumpSpk(0, 63, "xC0 x4F");
        DumpSpk(1, 63, "xC0 x6F");
    }
    else {
        DumpCmd(kClr, 0, "xC0");
        DumpCmd(kSnc, 0, "x80");
        DumpCmd(kRun, 3, "x03");
        DumpSpk(0, 1, "x48");
        DumpSpk(1, 1, "x68");
    }

    return 0;
}

