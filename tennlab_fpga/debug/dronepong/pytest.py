#!/usr/bin/env python3

# Copyright (c) 2026 Simon D. Levy
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

import argparse

import neuro
import fpga

from spikes import entries

PORT = '/dev/ttyUSB1'
IO_TYPE = 'DIDO'

NET = ('/home/levys/Desktop/framework/cpp-apps/applications/dronepong/' +
       'networks/dronepong_risp_train.txt')

SIM_TIME = 50

parser = argparse.ArgumentParser(
        formatter_class=argparse.ArgumentDefaultsHelpFormatter)

parser.add_argument('-t', '--target', default='cmoda7_35t',
                    help='target board')

args = parser.parse_args()

net = neuro.Network()

try:
    net.read_from_file(NET)
except Exception:
    print('Unable to open ' + NET)
    exit(1)

proc = fpga.Processor(args.target, PORT, IO_TYPE, debug=True)

proc.attach_network(net)

'''
timestep_prev = -1

for entry in entries:

    timestep = entry[0]

    if timestep == 10:
        break

    if timestep_prev != timestep:
        if timestep_prev != -1:
            proc.run(SIM_TIME)
            print(proc.output_counts())
        proc.clear_activity()
        timestep_prev = timestep

    proc.apply_spike(neuro.Spike(entry[1], entry[2], entry[3]))
'''
