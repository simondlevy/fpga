#!/usr/bin/env python3

# Copyright (c) 2026 Simon D. Levy
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

import argparse
import sys

import neuro

from fpga.network import charge_width, spike_value_factor

parser = argparse.ArgumentParser(
        formatter_class=argparse.ArgumentDefaultsHelpFormatter)

parser.add_argument('input_file')

parser.add_argument('-o', '--output_file', help='path for output file')

parser.add_argument('-d', '--debug', help='turn on debugging',
                    action='store_true')

args = parser.parse_args()

net = neuro.Network()

try:
    net.read_from_file(args.input_file)
except Exception:
    print('Unable to read from ' + args.input_file)
    exit(1)

outfile = (sys.stdout if args.output_file is None
           else open(args.output_file, 'w'))

outfile.write('// AUTO-GENERATED: DO NOT EDIT\n\n')
outfile.write('#pragma once\n\n')
outfile.write('#include <new_processor.hpp>\n\n')
outfile.write('static auto proc_ = neuro::Processor(')
outfile.write('%d, ' % net.num_inputs())
outfile.write('%d, ' % net.num_outputs())
outfile.write('%d, ' % charge_width(net))
outfile.write('%d, %s);\n' % (spike_value_factor(net), 'true'
                              if args.debug else 'false'))
