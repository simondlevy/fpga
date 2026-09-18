#!/usr/bin/env python3

# Copyright (c) 2026 Simon D. Levy
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

import argparse
from math import ceil

import neuro

from fpga._processor import DispatchOpcode, SYSTEM_BUFFER
from fpga.network import spike_value_factor, charge_width, spike_value_factor
from fpga._math import unsigned_width, width_bits_to_bytes, width_nearest_byte
import fpga.network as Net

def main():

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


    #print('to_fpga.spk: ', self._countbits(self._to_fpga.spk_fmt_str))

    opc_width = unsigned_width(len(DispatchOpcode) - 1)

    input_index_width = unsigned_width(net.num_inputs() - 1)

    max_bytes_per_run = (width_bits_to_bytes(opc_width +
                         unsigned_width(net.num_outputs() - 1)) *
                         (net.num_outputs() + 1))

    max_runs_ahead = SYSTEM_BUFFER // max_bytes_per_run
    opcode_shift = 8 - opc_width

    index_shift = opcode_shift - input_index_width

    spk_width = input_index_width + charge_width(net)
    operand_width = width_nearest_byte(opc_width + spk_width) - opc_width

    cmd_bytes = int(ceil((opc_width + operand_width) / 8))

    h_code = '// AUTO-GENERATED: DO NOT EDIT\n\n'
    h_code += '#pragma once\n\n'
    h_code += '#include <processor.hpp>\n\n'
    h_code += ('static const int kOutputNeurons = %d;\n' %
                net.num_outputs())
    h_code += ('static const int kChargeWidth = %d;\n' % charge_width(net))
    h_code += ('static const int kSpikeValueFactor = %d;\n' %
                spike_value_factor(net))
    h_code += 'static const int kOpcodeWidth = %d;\n' % opc_width
    h_code += 'static const int kInputIndexWidth = %d;\n' % input_index_width
    h_code += ('static const int kOutputIndexWidth = %d;\n' %
              unsigned_width(net.num_outputs() - 1))
    h_code += 'static const int kOpcodeShift = %d;\n' % opcode_shift
    h_code += 'static const int kIndexShift = %d;\n' % index_shift
    h_code += ('static const int kValueShift = %d;\n' %
              (index_shift - charge_width(net)))
    h_code += 'static const int kMaxRunsAhead = %d;\n' % max_runs_ahead
    h_code += ('static const int kMaxRun = %d;\n' %
              (min((1 << (width_nearest_byte(opc_width +
                          (input_index_width + charge_width(net))) -
                          opc_width)) - 1,
                   max_runs_ahead)))
    h_code += ('static const bool kDebug = %s;\n\n' %
                ('true' if args.debug else 'false'))
     
    with open('network_config.h', 'w') as outfile:
        outfile.write(h_code)

    cpp_code = '// AUTO-GENERATED: DO NOT EDIT\n\n'

    cpp_code += 'void WriteClr()\n'
    cpp_code += '{\n'
    cpp_code += ('    const uint8_t bytes[%d] = {%s 0x%02X};\n' %
                 (cmd_bytes,
                 ('x00, ') * (cmd_bytes-1),
                  DispatchOpcode.CLR << (8-opc_width)))
    cpp_code += '}\n\n'

    cpp_code += 'void WriteRun(const int to_run)\n'
    cpp_code += '{\n'
    cpp_code += ('    const uint8_t bytes[%d] = {%s 0x%02X};\n' %
                 (cmd_bytes,
                 ('x00, ') * (cmd_bytes-1),
                  DispatchOpcode.RUN << (8-opc_width)))
    cpp_code += '}\n\n'

    with open('proc.cpp', 'w') as outfile:
        outfile.write(cpp_code)


main()
