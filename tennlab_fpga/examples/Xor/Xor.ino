/*
 * Copyright (c) 2026 Simon D. Levy
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <stdint.h>

#include <processor.hpp>


static const uint32_t kBaudRate = 4'000'000; // Based on FPGA
static const uint32_t kDelayUsec = 10;       // Based on trial-and-error 

static auto proc_ = neuro::Processor(2, 1, 2, 1, false);

namespace neuro {

    class ArduinoUart : public Uart {

        void Write(const uint8_t byte) 
        {
            Serial1.write(byte);
        }

        auto Available() -> size_t 
        {
            return Serial1.available();
        }

        auto Read() -> uint8_t 
        {
            return Serial1.read();
        }
    };
}

void neuro::Processor::Write(const uint8_t byte)
{
    Serial1.write(byte);

    delayMicroseconds(kDelayUsec);
}

auto neuro::Processor::Available() -> size_t
{
    return Serial1.available();
}

auto neuro::Processor::Read() -> uint8_t
{
    return Serial1.read();
}

static void Run(const uint8_t a, const uint8_t b)
{
    // proc_ is declared in auto-generated xor.hpp
    proc_.ClearActivity();

    if (a) {
        proc_.ApplySpike(0, 0, 1);
    }

    if (b) {
        proc_.ApplySpike(1, 0, 1);
    }

    proc_.Run(3);

    Serial.print("input=");
    Serial.print(a);
    Serial.print(",");
    Serial.print(b);
    Serial.print(" output=");
    Serial.println(proc_.GetOutputCount(0));
}

void setup()
{
    Serial.begin(115200);

    proc_.Connect();

    proc_.ClearActivity();
}

void loop() 
{
    Run(0, 0);
    Run(0, 1);
    Run(1, 0);
    Run(1, 1);

    Serial.println();

    delay(1000);
}
