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

parser.add_argument('-t', '--target', help='target board',
                    default='cmoda7_35t_pmod')

parser.add_argument('-p', '--port', help='target port',
                    default='/dev/ttyUSB1')

parser.add_argument('-i', '--io_type', help='IO type',
                    default='DIDO')

args = parser.parse_args()

net = neuro.Network()
net.read_from_file(args.input_file)

proc = fpga.Processor(args.target, args.port, args.io_type)

proc.load_network_nonvolatile(net)
