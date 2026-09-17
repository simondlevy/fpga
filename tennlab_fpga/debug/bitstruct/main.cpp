#include <stdint.h>
#include <stdio.h>

#include <vector>

enum { RUN, SPK, SNC, CLR };

static std::vector<uint8_t> int2bin(const int val)
{
    std::vector<uint8_t> bits;

    auto v = val;

    while (v > 0) {
        bits.push_back(v & 0x1);
        v >>= 1;
    }

    return bits; 
}

static std::vector<uint8_t> pack_u(const int val, const int size)
{
    std::vector<uint8_t> bits = int2bin(val);

   
    return bits;
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
    for (auto bit : int2bin(3)) {
        printf("%d", bit);
    }
    printf("\n");

    pack_spk_command(2, 1, 7, 0, 63);
    pack_spk_command(2, 1, 7, 1, 63);
    pack_run_command(2, 6, 3);

    return 0;
}
