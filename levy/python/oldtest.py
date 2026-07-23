# Copyright (c) 2026 Simon D. Levy
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at https://mozilla.org/MPL/2.0/.

import numpy as np

import neuro
import fpga

input_file = '/home/levys/Desktop/framework/cpp-apps/networks/dronepong_risp_train.txt'

net = neuro.Network()

net.read_from_file(input_file)

proc = fpga.Processor("basys3", "/dev/ttyUSB1", "DIDO")
proc.load_network(net)

data = np.loadtxt('../spikes.txt', delimiter=' ')

runtime = int(max(data[:,0])) + 1

for t in range(runtime):

    proc.clear_activity()

    for spike in data:
        if int(spike[0]) == t:
            proc.apply_spike(neuro.Spike(int(spike[1]), spike[2], spike[3]))

    proc.run(50)

    print("\n", proc.output_count(0), proc.output_count(1))
