#!/usr/bin/env python3

# Copyright (c) 2026 Simon D. Levy
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

import argparse
import sys

import neuro

from fpga.network import charge_width, spike_value_factor, sim_time
from fpga._processor import DispatchOpcode, unsigned_width

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

outfile = open('network_config.h', 'w')

outfile.write('// AUTO-GENERATED: DO NOT EDIT\n\n')
outfile.write('#pragma once\n\n')
outfile.write('#include <processor.hpp>\n\n')
outfile.write('static const int kNumInputs = %d;\n' % net.num_inputs())
outfile.write('static const int kNumOutputs = %d;\n' % net.num_outputs())
outfile.write('static const int kChargeWidth = %d;\n' % charge_width(net))
outfile.write('static const int kSpikeValueFactor = %d;\n' %
              spike_value_factor(net))
outfile.write('static const int kOpcodeWidth = %d;\n' %
              unsigned_width(len(DispatchOpcode) - 1))
outfile.write('static const int kOutputIndexWidth = %d;\n' %
              unsigned_width(net.num_outputs() - 1))



outfile.write('static const int kSimTime = %d;\n' % sim_time(net))
outfile.write('static const bool kDebug = %s;\n\n' %
              ('true' if args.debug else 'false'))
