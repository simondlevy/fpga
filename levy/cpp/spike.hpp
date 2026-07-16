/*
 * Copyright (c) 2026 Simon D. Levy
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

namespace neuro {

    class Spike {

        public:

            int id;
            float time;
            float value;

            Spike() = default;

            Spike& operator=(const Spike& other) = default;

            Spike(int id, float time, float value)
                :id (id), time(time), value(value) { }

            auto operator<(const Spike& other) const {
                return time < other.time;
            }

    }; // class Spike

} // namespace neuro

