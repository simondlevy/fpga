#!/usr/bin/env python3

# Copyright (c) 2024-2025 Keegan Dent, 2026 Simon D. Levy
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

import argparse
from pathlib import Path

import neuro
import fpga


parser = argparse.ArgumentParser(
        formatter_class=argparse.ArgumentDefaultsHelpFormatter)

parser.add_argument('input_file')

parser.add_argument('-t', '--target',
                    choices=('cmoda7_35t', 'cmoda7_35t_pmod'),
                    help='target board')

parser.add_argument('-p', '--port', default='/dev/ttyUSB1', help='target port')

parser.add_argument('-s', '--spi_flash', action='store_true', help='flash SPI')

parser.add_argument('-i', '--io_type', default='DIDO', help='IO type')

args = parser.parse_args()

if not Path(args.input_file).is_file():
    print('File %s not found' % args.input_file)
    exit(1)

net = neuro.Network()
net.read_from_file(args.input_file)

proc = fpga.Processor(args.target, args.port, args.io_type)

proc.load_network(net, args.spi_flash)
