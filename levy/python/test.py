#!/usr/bin/env python3

# Copyright (c) 2026 Simon D. Levy
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

from fpga_connection import FpgaConnection, Spike
import neuro

PORT_NAME = "/dev/ttyUSB1"
BAUD_RATE = 4000000
NUM_INPUTS = 2
NUM_OUTPUTS = 2
CLOCK_FREQ = 100000000.0
CHARGE_WIDTH = 5
SPIKE_VALUE_FACTOR = 10

conn = FpgaConnection(
        PORT_NAME, BAUD_RATE,
        CLOCK_FREQ,
        NUM_INPUTS,
        NUM_OUTPUTS,
        CHARGE_WIDTH,
        SPIKE_VALUE_FACTOR)

conn.clear_activity()

for i in range(28):
    conn.apply_spike(Spike(0, i, 1.0))

for i in range(26):
    conn.apply_spike(Spike(1, i, 1.0))

conn.run(5)

# conn.run(50)
# print("\n", conn.output_count(0), conn.output_count(1))
