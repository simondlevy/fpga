<!--
 Copyright (c) 2026 Simon D. Levy

 This Source Code Form is subject to the terms of the Mozilla Public
 License, v. 2.0. If a copy of the MPL was not distributed with this
 file, You can obtain one at https://mozilla.org/MPL/2.0/.
-->

# Neuromorphic FPGA

This repository contains a standalone fork of the UTK TeNNLab FPGA
[repository](https://github.com/TENNLab-UTK/fpga), q.v. for background,
installation, and other details.  I made the following additions:

1. Support for the [Nexys A7](https://digilent.com/shop/nexys-a7-amd-artix-7-fpga-trainer-board-recommended-for-ece-curriculum/)
and [Cmod A7-35t](https://digilent.com/shop/nexys-a7-amd-artix-7-fpga-trainer-board-recommended-for-ece-curriculum/) boards.

2. A C++ [library](tennlab_fpga) to support talking to the FPGA from a microcontroller (e.g., Arduino)
or desktop C++ program.

3. Support for non-volatile loading of a network to the Cmod A7-35t board.

4. Support for talking to a network already resident on the board, without rebuilding or
   reprogramming it.
   
5. Support for communicating with the Cmod A7-35t over the UART on its PMOD pins.
   
