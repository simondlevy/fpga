# Microcontroller / C++ support for TeNNLab FPGA

<img src="media/teensy-cmod.jpg" width=500>

This directory contains support code for communicating between a microntroller
like Arduino and an FPGA programmed as described in the [README](../README.md),
as well as support for those who wish to use C++ instead of Python for 
writing such programs on an ordinary Linux host computer.  Although it's
probably easiest to use a [makefile](examples/xor/Makefile) for all such work,
this directory can be copied into your Arduino libraries folder (e.g.,
```$HOME/Arduino/libraries```) if you prefer to use the Arduino IDE.

## Installation

1. Follow the directions in the [Getting Started](../README.md#getting_started)
section of the main repo README for installing the FPGA Python library. 

2. Install [openFPGALoader](https://trabucayre.github.io/openFPGALoader/guide/install.html)

## POSIX example

As you already need a POSIX environment for the main repo, it's probably easiest to start here,
before you attempt to run on Arduino or another microcontroller.  

First, you'll need a
[Xlilinx Cmod A7-35T FPGA](https://digilent.com/shop/cmod-a7-35t-breadboardable-artix-7-fpga-module/),
as this is currently the only FPGA supported by the example we'll be using.
Once you've set up the main repo (including the Python environment, do the
following from wherever you installed the main repo:

```bash
rm -rf ~/.cache/neuro_fpga # optional but recommended
python3 tennlab_fpga/utils/load.py -t cmoda7_35t networks/xor.txt # may take several minutes
cd tennlab_fpga/posix/xor
make run
```

## Arduino example
