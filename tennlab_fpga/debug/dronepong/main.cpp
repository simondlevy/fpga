/*
 * Copyright (c) 2026 Simon D. Levy
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <stdio.h>
#include <string>

#include <processor.hpp>

#include "spikes.h"

static neuro::Processor proc_;

extern void clear_encoded_spikes();
extern void encode();
extern unsigned int num_encoded_spikes;
extern void apply_spike(unsigned int input_ind, unsigned int time, double value);
extern void run(double duration);
extern unsigned int output_count(unsigned int output_ind);

int main()
{
    proc_.Connect();

    int time_prev = -1;

    for (auto entry : entries) {

        if (entry.time == 1) {
            break;
        }

        if (time_prev != entry.time) {
            if (time_prev != -1) {
                proc_.Run(kSimTime);
                //print(proc.output_counts());
            }
            proc_.ClearActivity();
            time_prev = entry.time;
        }

        proc_.ApplySpike(entry.id, entry.time, entry.value);
    }

    return 0;
}
