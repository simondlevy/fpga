#!/usr/bin/env python3

# Copyright (c) 2026 Simon D. Levy
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

from fpga_connection import FpgaConnection, Spike
import neuro
import numpy as np

PORT_NAME = "/dev/ttyUSB1"
BAUD_RATE = 4000000
NUM_INPUTS = 2
NUM_OUTPUTS = 2
CLOCK_FREQ = 100000000.0
CHARGE_WIDTH = 5
SPIKE_VALUE_FACTOR = 10

fpga = FpgaConnection(
        PORT_NAME,
        BAUD_RATE,
        CLOCK_FREQ,
        NUM_INPUTS,
        NUM_OUTPUTS,
        CHARGE_WIDTH,
        SPIKE_VALUE_FACTOR)

data = np.loadtxt('../spikes.txt', delimiter=' ')

runtime = int(max(data[:,0])) + 1

for timestep in range(runtime):

    fpga.clear_activity()

    for spike in data:
        if int(spike[0]) == timestep:
            fpga.apply_spike(Spike(int(spike[1]), spike[2], spike[3]))

    fpga.run(50)

    print('%03d: %02d %02d' %
            (timestep, fpga.output_count(0), fpga.output_count(1)))
