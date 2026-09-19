#include <stdint.h>
#include <stdio.h>

enum { kRun, kSpk, kSnc, kClr };

static const bool kUseDronepong = false;
static const bool kUseNew = false;

static const int kOpcodeWidth = 2;
static const int kIndexWidth = 1;
static const int kChargeWidth = kUseDronepong ? 7 : 2;
static const int kOperandWidth = kUseDronepong ? 14 : 6;


static inline size_t bitpack_size(unsigned nbits)
{
    return ((size_t)nbits + 7) / 8;
}

static constexpr inline size_t kBytesPerMesssage()
{
    return ((kOpcodeWidth + kIndexWidth + kChargeWidth) + 7) / 8;
}

static uint8_t bytes[kBytesPerMesssage()];

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


static void NewDumpBytes(const uint8_t * bytes, const size_t count, const char * target)
{
    printf("target = %s; actual = ", target);
    for (int k=0; k<count; ++k) {
        printf("x%02X ", bytes[count-k-1]);
    }
    printf("\n");
}

static void NewDumpCmd(const int opcode, const int operand, const char * target)
{
    const auto nbytes = bitpack_size(kOpcodeWidth + kOperandWidth);

    uint8_t buf[8] = {};
    size_t pos = 0;
    bitpack_put(buf, &pos, opcode, kOpcodeWidth);
    bitpack_put(buf, &pos, operand, kOperandWidth);

    NewDumpBytes(buf, nbytes, target);
}

static void NewDumpSpk(const int index, const int charge, const char * target)
{
    const auto nbytes = bitpack_size(kOpcodeWidth + kIndexWidth + kChargeWidth);

    const auto charge_twoscomp =
        charge < 0 ? (1 << kChargeWidth) + charge  : charge;

    uint8_t buf[8] = {};
    size_t pos = 0;
    bitpack_put(buf, &pos, kSpk, kOpcodeWidth);
    bitpack_put(buf, &pos, index, kIndexWidth);
    bitpack_put(buf, &pos, charge_twoscomp, kChargeWidth);

    NewDumpBytes(buf, nbytes, target);
}

static void OldDumpBytes(uint64_t bits, const uint8_t nbytes, const char * target)
{
    printf("target = %s; actual = ", target);
    for (int k=0; k<nbytes; ++k) {
        printf("x%02X ", (int)(bits & 0xFF));
        bits >>= 8;
    }

    printf("\n");
}

static void OldDumpCmd(const int opcode, const int operand, const char * target)
{
    const auto nbytes = bitpack_size(kOpcodeWidth + kOperandWidth);

    OldDumpBytes((opcode << kOperandWidth) | operand, nbytes, target);
}


static void OldDumpSpk(const int index, const int charge, const char * target)
{
    const auto nbytes = bitpack_size(kOpcodeWidth + kIndexWidth + kChargeWidth);

    const auto charge_twoscomp =
        charge < 0 ? (1 << kChargeWidth) + charge  : charge;

    OldDumpBytes(
            (kSpk << kOperandWidth) +
            (index << (kOperandWidth-kIndexWidth)) +
            (charge_twoscomp << (kOperandWidth-kChargeWidth-1)), 
            nbytes,
            target);
}


int main()
{
    if (kUseDronepong) {
        if (kUseNew) {
            NewDumpCmd(kClr, 0, "x00 xC0");
            NewDumpCmd(kSnc, 0, "x00 x80");
            NewDumpCmd(kRun, 1, "x01 x00");
            NewDumpCmd(kRun, 23, "x17 x00");
            NewDumpSpk(0, 63, "xC0 x4F");
            NewDumpSpk(1, 63, "xC0 x6F");
        }
        else {
            OldDumpCmd(kClr, 0, "x00 xC0");
            OldDumpCmd(kSnc, 0, "x00 x80");
            OldDumpCmd(kRun, 1, "x01 x00");
            OldDumpCmd(kRun, 23, "x17 x00");
            OldDumpSpk(0, 63, "xC0 x4F");
            OldDumpSpk(1, 63, "xC0 x6F");
        }
    }
    else {
        if (kUseNew) {
            NewDumpCmd(kClr, 0, "xC0");
            NewDumpCmd(kSnc, 0, "x80");
            NewDumpCmd(kRun, 3, "x03");
            NewDumpSpk(0, 1, "x48");
            NewDumpSpk(1, 1, "x68");
        }
        else {
            OldDumpCmd(kClr, 0, "xC0");
            OldDumpCmd(kSnc, 0, "x80");
            OldDumpCmd(kRun, 3, "x03");
            OldDumpSpk(0, 1, "x48");
            OldDumpSpk(1, 1, "x68");
        }
    }

    return 0;
}

