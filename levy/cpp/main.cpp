/*
 * Copyright (c) 2026 Simon D. Levy
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <string>

#include "fpga_connection.hpp"
#include "serial.h"
#include "spike.hpp"

static const std::string kPortName = "/dev/ttyUSB1";
static constexpr int kBaudRate = 4000000;
static constexpr int kNumInputs = 2;
static constexpr int kNumOutputs = 2;
static constexpr float kClockFreq = 100000000;
static constexpr int kChargeWidth = 5;
static constexpr int kSpikeValueFactor = 10;
static constexpr float kReadTimeoutSec = 10.0;

int main()
{
    auto serial = Serial(kPortName, kBaudRate);

    serial.Begin(kReadTimeoutSec);

    auto conn = neuro::FpgaConnection(
            serial,
            kNumInputs,
            kNumOutputs,
            kChargeWidth,
            kClockFreq,
            kSpikeValueFactor);

    conn.ClearActivity();

    for (int i=0; i<28; ++i) {
        conn.ApplySpike(neuro::Spike(0, i, 1.0));
    }

    for (int i=0; i<26; ++i) {
        conn.ApplySpike(neuro::Spike(1, i, 1.0));
    }

    conn.Run(5);

    //conn.Run(50);
    // printf("%d %d\n", conn.GetOutputCount(0), conn.GetOutputCount(1));

    return 0;
}
