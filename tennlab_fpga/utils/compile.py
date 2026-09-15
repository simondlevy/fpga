#!/usr/bin/env python3

# Copyright (c) 2026 Simon D. Levy
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

import argparse

import neuro

from fpga._processor import Processor

parser = argparse.ArgumentParser(
        formatter_class=argparse.ArgumentDefaultsHelpFormatter)

parser.add_argument('input_file')

parser.add_argument('-d', '--debug', help='turn on debugging',
                    action='store_true')

args = parser.parse_args()

net = neuro.Network()

try:
    net.read_from_file(args.input_file)
except Exception:
    print('Unable to read from ' + args.input_file)
    exit(1)

proc = Processor('cpp')

hdr_code, cpp_code = proc.compile_to_cpp(net, args.debug)

with open('network_config.h', 'w') as outfile:
    outfile.write(hdr_code)

with open('proc.cpp', 'w') as outfile:
    outfile.write(cpp_code)
