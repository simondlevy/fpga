# Copyright (c) 2024-2025 Keegan Dent, 2026 Simon D. Levy
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

from importlib import resources
from json import load

import conn
import fpga
import neuro

TARGET_NAME = "basys3"
PORT_NAME = "/dev/ttyUSB1"

target_config = None

with open(resources.files(fpga.config).joinpath("targets.json")) as f:
    target_config = load(f)[TARGET_NAME]

baudrate = target_config["parameters"]["uart"]["baud_rates"][-1]

clock_freq = target_config["parameters"]["clk_freq"]

net = neuro.Network()
net.read_from_file("../networks/simple.txt")

charge_width = fpga.network.charge_width(net)

num_inputs = net.num_inputs()

num_outputs = net.num_outputs()

spike_value_factor = fpga.network.spike_value_factor(net)

conn = conn.Connection(
        PORT_NAME,
        baudrate,
        clock_freq,
        num_inputs,
        num_outputs,
        charge_width,
        spike_value_factor,
        "DIDO")

for _ in range(3):

    conn.clear_activity()

    conn.apply_spikes([neuro.Spike(0, i, 1.0) for i in range(3)])

    conn.run(6)

    print(conn.output_last_fire(0))
