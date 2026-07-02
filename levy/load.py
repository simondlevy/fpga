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

'''
parser.add_argument('-a', '--address', help='address (IP or MAC)',
                    required=True)
parser.add_argument('-p', '--port', help='port', type=int, required=True)
parser.add_argument('-i', '--ids', help='neuron ids')
parser.add_argument('-v', '--video', help='video file to save',
                    default=None)
parser.add_argument('-n', '--display-numbers', help='display numbers',
                    action='store_true')
parser.add_argument('-l', '--logarithmic', help='use logarithm of counts',
                    action='store_true')
parser.add_argument('-s', '--timespan', type=int, default=1000,
                    help='Time span in milliseconds')
'''

args = parser.parse_args()

net = neuro.Network()

try:
    net.read_from_file(args.input_file)
except:
    print('Unable to read from ' + args.input_file)
    exit(1)

proc = fpga.Processor("basys3", "/dev/ttyUSB1", "DIDO")

proc.load_network(net)
