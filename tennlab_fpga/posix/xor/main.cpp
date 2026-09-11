/*
 * Copyright (c) 2026 Simon D. Levy
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <stdio.h>

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include <processor.hpp>

#include "xor.hpp"

static void run(const uint8_t a, const uint8_t b)
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

int main()
{
    proc_.Connect();

    proc_.ClearActivity();

    run(0, 0);
    run(0, 1);
    run(1, 0);
    run(1, 1);

    return 0;
}
