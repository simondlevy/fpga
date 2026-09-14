#!/usr/bin/env python3

# Copyright (c) 2026 Simon D. Levy
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

import argparse

import neuro
import fpga

PORT = '/dev/ttyUSB1'
IO_TYPE = 'DIDO'
NET = '/home/levys/Desktop/framework/cpp-apps/applications/dronepong/networks/dronepong_risp_train.txt'

parser = argparse.ArgumentParser(
        formatter_class=argparse.ArgumentDefaultsHelpFormatter)

parser.add_argument('-t', '--target', default='cmoda7_35t', help='target board')

args = parser.parse_args()

net = neuro.Network()

try:
    net.read_from_file(NET)
except:
    print('Unable to open ' + NET)
    exit(1)

proc = fpga.Processor(args.target, PORT, IO_TYPE)
