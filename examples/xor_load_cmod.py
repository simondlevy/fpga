#!/usr/bin/env python3

# Copyright (c) 2026 Simon D. Levy
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at https://mozilla.org/MPL/2.0/.

import neuro
import fpga

net = neuro.Network()
net.read_from_file("../networks/xor.txt")

proc = fpga.Processor("cmod", "/dev/ttyUSB1", "DIDO")

proc.load_network(net)
