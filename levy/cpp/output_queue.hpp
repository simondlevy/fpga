/*
 * Copyright (c) 2026 Simon D. Levy
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

namespace neuro {

    class OutputQueue {

        private:

            static constexpr size_t MAX_NEURONS = 16;
            static constexpr size_t MAX_SPIKES_PER_NEURON = 256;

        public:

            float times[MAX_NEURONS][MAX_SPIKES_PER_NEURON];
            size_t counts[MAX_NEURONS];

            OutputQueue() = default;

            OutputQueue& operator=(const OutputQueue& other) = default;

            void Append(const int index, const float time)
            {
                times[index][counts[index]] = time;
                counts[index]++;
            }

    }; // class OutputQueue


} // namespace neuro
