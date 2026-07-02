#!/usr/bin/env python3
# Copyright (c) 2024-2025 Keegan Dent, 2026 Simon D. Levy
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

import argparse

import neuro
import fpga

parser = argparse.ArgumentParser(
        formatter_class=argparse.ArgumentDefaultsHelpFormatter)

parser.add_argument('input_file')

parser.add_argument('-t', '--target', help='target board', default='basys3')

parser.add_argument('-p', '--port', help='port', type=str,
                    default='/dev/ttyUSB1')

args = parser.parse_args()

net = neuro.Network()

try:
    net.read_from_file(args.input_file)
except Exception:
    print('Unable to read from ' + args.input_file)
    exit(1)

proc = fpga.Processor(args.target, args.port, "DIDO")

proc.load_network(net)
