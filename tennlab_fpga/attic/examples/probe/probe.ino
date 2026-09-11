/*
 * Copyright (c) 2026 Simon D. Levy
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */


void setup()
{
    pinMode(LED_BUILTIN, OUTPUT);

    Serial1.begin(4'000'000);
}

void loop() 
{
    static uint32_t msec_prev_;
    const auto msec_curr = millis();
    if (msec_curr - msec_prev_ > 1000) {

        static bool led_on_;
        digitalWrite(LED_BUILTIN, led_on_);
        led_on_ = !led_on_;

        Serial1.write(0xC0);

        while (Serial1.available()) {
            printf("%06lu: 0x%02X\n", millis(), Serial1.read());
        }

        msec_prev_ = msec_curr;
    }
}
