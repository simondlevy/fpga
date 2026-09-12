/*
 * Copyright (c) 2026 Simon D. Levy
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <stdio.h>

namespace neuro {

    class Uart {

        friend class Processor;

        private:

            virtual void Begin() = 0;
            virtual void Write(const uint8_t byte) = 0;
            virtual auto Available() -> size_t = 0;
            virtual auto Read() -> uint8_t = 0;

    };
}
