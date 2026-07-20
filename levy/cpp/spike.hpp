/*
 * Copyright (c) 2026 Simon D. Levy
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

namespace neuro {

    class FpgaSpike {

        public:

            int id;
            float time;
            float value;

            FpgaSpike() = default;

            FpgaSpike& operator=(const FpgaSpike& other) = default;

            FpgaSpike(int id, float time, float value)
                :id (id), time(time), value(value) { }

            auto operator<(const FpgaSpike& other) const {
                return time < other.time;
            }

    }; // class FpgaSpike

} // namespace neuro

