/*
 * Copyright (c) 2026 Simon D. Levy
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <processor.hpp>

#include "decl.hpp"

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

    printf("input = %d,%d; output = %d\n", a, b, proc_.GetOutputCount(0));
}

static void BlinkLed()
{
    static bool led_on_;
    digitalWrite(LED_BUILTIN, led_on_);
    led_on_ = !led_on_;
}


void setup()
{
    pinMode(LED_BUILTIN, OUTPUT);

    delay(5000);

    proc_.Connect();

    proc_.ClearActivity();

    Run(0, 0);
    Run(0, 1);
    Run(1, 0);
    Run(1, 1);
}

void loop() 
{
    static uint32_t msec_prev_;
    const auto msec_curr = millis();
    if (msec_curr - msec_prev_ > 1000) {
        msec_prev_ = msec_curr;
        BlinkLed();
    }
}
