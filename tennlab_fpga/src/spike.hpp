/*
 * Copyright (c) 2026 Simon D. Levy
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

namespace neuro {

    class LevySpike {

        friend class Processor;
        friend class SpikeHeap;

        private:

            int id;
            float time;
            float value;

            LevySpike() = default;

            LevySpike& operator=(const LevySpike& other) = default;

            LevySpike(int id, float time, float value)
                :id (id), time(time), value(value) { }

            auto operator<(const LevySpike& other) const {
                return time < other.time;
            }

    }; // class LevySpike

} // namespace neuro

