/*
 * Copyright (c) 2026 Simon D. Levy
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <Arduino.h>

#include "network_config.h"
#include "processor.hpp"

static const uint32_t kBaudRate = 4'000'000; // Based on FPGA
static const uint32_t kDelayUsec = 10;       // Based on trial-and-error 

static const uint8_t kRxPin = 4;
static const uint8_t kTxPin = 14;

void neuro::Processor::UartBegin()
{
    Serial1.begin(kBaudRate, SERIAL_8N1, kRxPin, kTxPin);
}

void neuro::Processor::UartWrite(const uint8_t * bytes, const size_t count)
{
    Serial1.write(bytes, count);

    delayMicroseconds(kDelayUsec);
}

auto neuro::Processor::UartAvailable() -> int
{
    return Serial1.available();
}

auto neuro::Processor::UartRead() -> uint8_t
{
    return Serial1.read();
}
