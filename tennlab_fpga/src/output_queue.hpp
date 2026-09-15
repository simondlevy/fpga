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

            static constexpr int kMaxOutputNeurons = 16;
            static constexpr int kMaxSpikesPerNeuron = 256;

        public:

            float output_times_[kMaxOutputNeurons][kMaxSpikesPerNeuron];
            int output_counts_[kMaxOutputNeurons];

            OutputQueue() = default;

            OutputQueue& operator=(const OutputQueue& other) = default;

            void Append(const int index, const float time)
            {
                output_times_[index][output_counts_[index]] = time;
                output_counts_[index]++;
            }

    }; // class OutputQueue

} // namespace neuro
