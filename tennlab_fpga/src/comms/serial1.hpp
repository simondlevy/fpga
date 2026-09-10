/*
 * Copyright (c) 2026 Simon D. Levy
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <Arduino.h>

#include <processor.hpp>

void neuro::Processor::Connect()
{
    Serial1.begin(Processor::kBaudRate);
}

void neuro::Processor::Write(const uint8_t byte)
{
    Serial1.write(byte);

    delayMicroseconds(Processor::kDelayUsec);
}

auto neuro::Processor::Available() -> size_t
{
    return Serial1.available();
}

auto neuro::Processor::Read() -> uint8_t
{
    return Serial1.read();
}
