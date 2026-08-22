# Copyright (c) 2024 Keegan Dent
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at https://mozilla.org/MPL/2.0/.

import neuro
import fpga

TARGET = "basys3"
#TARGET = "nexys"
#TARGET = "cmod"

net = neuro.Network()
net.read_from_file("networks/simple.txt")

proc = fpga.Processor(TARGET, "/dev/ttyUSB1", "DIDO")

proc.load_network(net)
proc.apply_spikes([neuro.Spike(0, i, 1.0) for i in range(3)])
proc.run(6)
print(proc.output_last_fire(0))
