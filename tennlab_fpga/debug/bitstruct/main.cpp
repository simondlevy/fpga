#include <stdint.h>
#include <stdio.h>

#include <algorithm>
#include <vector>

enum { RUN, SPK, SNC, CLR };

typedef std::vector<uint8_t> Bits;

static Bits int2bin(const int val, const int size)
{
    Bits reversed_bits;

    auto v = val;

    while (v > 0) {
        reversed_bits.push_back(v & 0x1);
        v >>= 1;
    }

    while ((int)reversed_bits.size() < size) {
        reversed_bits.push_back(0);
    }

    Bits bits(reversed_bits.size());

    std::reverse_copy(reversed_bits.begin(), reversed_bits.end(), bits.begin());

    return bits;
}


static Bits append(const Bits a, const Bits b)
{
    auto c = a;

    c.insert(c.end(), b.begin(), b.end());

    return c;
}

static Bits zfill(const Bits inp, const size_t n)
{
    Bits zeros;

    while (zeros.size() < n) {
        zeros.push_back(0);
    }

    return append(inp, zeros);
}

static void pack_spk_command(const int opcode_width, const int index_width,
        const int charge_width, const int index, const int charge)
{
}

static void pack_run_command(const int opcode_width, const int operand_width,
        const int to_run)
{
}

int main()
{
    for (auto bit : zfill(int2bin(3, 2), 6)) {
        printf("%d", bit);
    }
    printf("\n");

    pack_spk_command(2, 1, 7, 0, 63);
    pack_spk_command(2, 1, 7, 1, 63);
    pack_run_command(2, 6, 3);

    return 0;
}
