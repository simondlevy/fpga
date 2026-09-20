#!/usr/bin/env python3

# Copyright (c) 2026 Simon D. Levy
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

import argparse

import neuro

from fpga._processor import DispatchOpcode, SYSTEM_BUFFER
from fpga.network import spike_value_factor, charge_width, sim_time
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

opcode_width = unsigned_width(len(DispatchOpcode) - 1)

input_index_width = unsigned_width(net.num_inputs() - 1)

output_index_width = unsigned_width(net.num_outputs() - 1)

max_bytes_per_run = (width_bits_to_bytes(opcode_width +
                     unsigned_width(net.num_outputs() - 1)) *
                     (net.num_outputs() + 1))

max_runs_ahead = SYSTEM_BUFFER // max_bytes_per_run

spk_width = input_index_width + charge_width(net)

operand_width = width_nearest_byte(opcode_width + spk_width) - opcode_width

if opcode_width + output_index_width > 8:

    print('Compiler cannot accommodate FPGA outputs of more than one byte')

else:

    code = '// AUTO-GENERATED: DO NOT EDIT\n\n'
    code += '#pragma once\n\n'
    code += '#include <processor.hpp>\n\n'
    code += 'static const int kOpcodeWidth = %d;\n' % opcode_width
    code += ('static const int kInputNeuronIndexWidth = %d;\n' %
             unsigned_width(net.num_inputs() - 1))
    code += ('static const int kChargeWidth = %d;\n' % charge_width(net))
    code += 'static const int kOperandWidth = %d;\n' % operand_width
    code += ('static const int kOutputNeurons = %d;\n' %
             net.num_outputs())
    code += ('static const int kSpikeValueFactor = %d;\n' %
             spike_value_factor(net))
    code += ('static const int kOutputNeuronIndexWidth = %d;\n' %
             unsigned_width(net.num_outputs() - 1))
    code += 'static const int kMaxRunsAhead = %d;\n' % max_runs_ahead
    code += ('static const int kMaxRun = %d;\n' %
             (min((1 << (width_nearest_byte(opcode_width +
                         (input_index_width + charge_width(net))) -
                         opcode_width)) - 1, max_runs_ahead)))
    code += 'static const int kSimTime = %d;\n\n' % sim_time(net)
    code += ('static const bool kDebug = %s;\n\n' %
             ('true' if args.debug else 'false'))

    with open('network_config.h', 'w') as outfile:
        outfile.write(code)
