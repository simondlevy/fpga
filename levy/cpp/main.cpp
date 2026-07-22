/*
 * Copyright (c) 2026 Simon D. Levy
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "fpga_connection.hpp"
#include "spike.hpp"

static constexpr int kNumInputs = 2;
static constexpr int kNumOutputs = 2;
static constexpr float kClockFreq = 100000000;
static constexpr int kChargeWidth = 5;
static constexpr int kSpikeValueFactor = 10;

int main()
{
    auto conn = neuro::FpgaConnection(
            kNumInputs,
            kNumOutputs,
            kChargeWidth,
            kClockFreq,
            kSpikeValueFactor);

    for (uint8_t k=0; k<2; ++k) {

        conn.ClearActivity();

        for (int i=0; i<28; ++i) {
            conn.ApplySpike(neuro::FpgaSpike(0, i, 1.0));
        }

        for (int i=0; i<26; ++i) {
            conn.ApplySpike(neuro::FpgaSpike(1, i, 1.0));
        }

        conn.Run(50);

        printf("%d %d\n\n", conn.GetOutputCount(0), conn.GetOutputCount(1));
    }

    return 0;
}
