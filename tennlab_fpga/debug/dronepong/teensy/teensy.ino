/*
 * Copyright (c) 2026 Simon D. Levy
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "network_config.h"
#include <processor.hpp>

#include "spikes.h"

static neuro::Processor proc_;

static int timestep_prev;

void setup()
{
    proc_.Connect();

    timestep_prev = -1;
}

void loop()
{
    for (auto entry : entries) {

        /*printf("(%d, %d, %d, %d)\n",
                entry.timestep, entry.id, (int)entry.time, (int)entry.value);*/

            if (timestep_prev != entry.timestep) {
                if (timestep_prev != -1) {
                    proc_.Run(kSimTime);
                    printf("[%d, %d]\n",
                            proc_.GetOutputCount(0),  proc_.GetOutputCount(1));
                }
                proc_.ClearActivity();
                timestep_prev = entry.timestep;
            }

        proc_.ApplySpike(entry.id, entry.time, entry.value);
    }
}
