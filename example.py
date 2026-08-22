# Copyright (c) 2024 Keegan Dent, 2026 Simon D. Levy
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at https://mozilla.org/MPL/2.0/.

import argparse
from argparse import ArgumentDefaultsHelpFormatter

import neuro
import fpga

argparser = argparse.ArgumentParser(
        formatter_class=ArgumentDefaultsHelpFormatter)
argparser.add_argument("-t", "--target", type=str, required=False,
                       default="basys3", help="Target board")
args = argparser.parse_args()

net = neuro.Network()
net.read_from_file("networks/simple.txt")

proc = fpga.Processor(args.target, "/dev/ttyUSB1", "DIDO")

proc.load_network(net)
proc.apply_spikes([neuro.Spike(0, i, 1.0) for i in range(3)])
proc.run(6)
print(proc.output_last_fire(0))
