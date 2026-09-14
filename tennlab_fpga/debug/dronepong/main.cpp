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

#include "dronepong_fpga.hpp"
#include "spikes.h"

static const std::string kPortName = "/dev/ttyUSB1";

extern void clear_encoded_spikes();
extern void encode();
extern unsigned int num_encoded_spikes;
extern void apply_spike(unsigned int input_ind, unsigned int time, double value);
extern void run(double duration);
extern unsigned int output_count(unsigned int output_ind);

int main()
{
    proc_.Connect();

#if 0
    int timestep_prev = -1;

    for (auto entry : entries) {

        if (entry.timestep == 1) {
            break;
        }

        if (timestep_prev != entry.timestep) {
            if (timestep_prev != -1) {
                proc_.Run(kSimTime);
                //print(proc.output_counts());
            }
            proc_.ClearActivity();
            timestep_prev = entry.timestep;
        }

        proc_.ApplySpike(entry.id, entry.time, entry.value);
    }


    const int runtime = 10; // entries[kEntries-1].step;

    for (int timestep=0; timestep<runtime; ++timestep) {

        clear_encoded_spikes();
        encode();

        proc_.ClearActivity();

        for (auto entry:entries) {
            if (entry.step == timestep) {
                proc_.ApplySpike(entry.id, entry.time, entry.value);
                apply_spike(entry.id, entry.time, entry.value);
            }
        }

        run(kSimTime);

        proc_.Run(SIM_TIME);

        printf("%03d | %02d %02d | %02d %02d\n",
                timestep, output_count(0), output_count(1),
                proc_.GetOutputCount(0), proc_.GetOutputCount(1));
    }
#endif

    return 0;
}
