/*
 * Copyright (c) 2026 Simon D. Levy
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <Arduino.h>

#include <uart.hpp>

namespace neuro {

    class ArduinoUart : public Uart {


        static const uint32_t kBaudRate = 4'000'000; // Based on FPGA
        static const uint32_t kDelayUsec = 10;       // Based on trial-and-error 

        void Begin()
        {
            Serial1.begin(kBaudRate);
        }

        void Write(const uint8_t byte)
        {
            Serial1.write(byte);

            delayMicroseconds(kDelayUsec);
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
