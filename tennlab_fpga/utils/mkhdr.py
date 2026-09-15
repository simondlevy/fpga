#!/usr/bin/env python3

# Copyright (c) 2026 Simon D. Levy
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

import argparse

import neuro

from fpga.network import charge_width, spike_value_factor
from fpga._processor import SYSTEM_BUFFER, DispatchOpcode
from fpga._math import unsigned_width, width_bits_to_bytes, width_nearest_byte

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

opcode_width = unsigned_width(len(DispatchOpcode) - 1)

input_index_width = unsigned_width(net.num_inputs() - 1)

max_bytes_per_run = (width_bits_to_bytes(opcode_width +
                     unsigned_width(net.num_outputs() - 1)) *
                     (net.num_outputs() + 1))

max_runs_ahead = SYSTEM_BUFFER // max_bytes_per_run

opcode_shift = 8 - opcode_width

index_shift = opcode_shift - input_index_width

outfile.write('// AUTO-GENERATED: DO NOT EDIT\n\n')
outfile.write('#pragma once\n\n')
outfile.write('#include <processor.hpp>\n\n')
outfile.write('static const int kNumOutputs = %d;\n' % net.num_outputs())
outfile.write('static const int kChargeWidth = %d;\n' % charge_width(net))
outfile.write('static const int kSpikeValueFactor = %d;\n' %
              spike_value_factor(net))
outfile.write('static const int kOpcodeWidth = %d;\n' % opcode_width)
outfile.write('static const int kInputIndexWidth = %d;\n' % input_index_width)
outfile.write('static const int kOutputIndexWidth = %d;\n' %
              unsigned_width(net.num_outputs() - 1))
outfile.write('static const int kOpcodeShift = %d;\n' % opcode_shift)
outfile.write('static const int kIndexShift = %d;\n' % index_shift)
outfile.write('static const int kValueShift = %d;\n' %
              (index_shift - charge_width(net)))
outfile.write('static const int kMaxRunsAhead = %d;\n' % max_runs_ahead)
outfile.write('static const int kMaxRun = %d;\n' %
              (min((1 << (width_nearest_byte(opcode_width +
                          (input_index_width + charge_width(net))) -
                          opcode_width)) - 1,
                   max_runs_ahead)))
outfile.write('static const bool kDebug = %s;\n\n' %
              ('true' if args.debug else 'false'))
