#!/usr/bin/env python3

# Copyright (c) 2026 Simon D. Levy
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

import argparse
from periphery import serial

parser = argparse.ArgumentParser(
        formatter_class=argparse.ArgumentDefaultsHelpFormatter)

parser.add_argument('-p', '--port', help='port', default='/dev/ttyUSB1')

parser.add_argument('-b', '--baud', help='baud rate', type=int,
                    default=4_000_000)

args = parser.parse_args()

with serial.Serial(args.port, args.baud) as ser:

    ser.write(b'\xC0')

