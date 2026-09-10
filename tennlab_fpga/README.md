# Microcontroller / C++ support for TeNNLab FPGA

<img src="media/teensy-cmod.jpg" width=500>

This directory contains support code for communicating between a microntroller
like Arduino and an FPGA programmed as described in the [README](../README.md),
as well as support for those who wish to use C++ instead of Python for 
writing such programs on an ordinary Linux host computer.  Although it's
probably easiest to use a [makefile](examples/xor/Makefile) for all such work,
this directory can be copied into your Arduino libraries folder (e.g.,
```$HOME/Arduino/libraries```) if you prefer to use the Arduino IDE.

To get started, follow the directions in the [Getting Started](../README.md#getting_started)
section of the main repo README for installing the FPGA Python library.  Then try out the
examples below:

## POSIX example

As you already need a POSIX environment for the main repo, it's probably easiest to start here,
before you attempt to run on Arduino or another microcontroller.

## Arduino example
