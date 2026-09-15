# Copyright (c) 2026 Simon D. Levy
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

all: test

ROOT = ../..

TFPGA = $(ROOT)/tennlab_fpga

SRC = $(TFPGA)/src

NET = $(ROOT)/networks/xor.txt

test: main.cpp uart.cpp $(SRC)/*.hpp network_config.h
	g++ -Wall -I. -I$(SRC) -I.. -o test main.cpp uart.cpp

network_config.h: $(NET)
	$(TFPGA)/utils/compile.py $(NET)

run: test
	./test

edit:
	vim main.cpp

clean:
	rm -rf test network_config.h
