#!/usr/bin/env python3

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
net.read_from_file("../networks/xor.txt")

proc = fpga.Processor(args.target, "/dev/ttyUSB1", "DIDO")

proc.load_network(net)

proc.clear_activity()
print('input = 0,0; output = ', proc.output_counts()[0])

proc.clear_activity()
proc.apply_spike(neuro.Spike(0, 0, 1))
proc.run(3)
print('input = 1,0; output = ', proc.output_counts()[0])

proc.clear_activity()
proc.apply_spike(neuro.Spike(1, 0, 1))
proc.run(3)
print('input = 0,1; output = ', proc.output_counts()[0])

proc.clear_activity()
proc.apply_spike(neuro.Spike(0, 0, 1))
proc.apply_spike(neuro.Spike(1, 0, 1))
proc.run(3)
print('input = 1,1; output = ', proc.output_counts()[0])

