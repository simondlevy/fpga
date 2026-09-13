# Copyright (c) 2026 Simon D. Levy
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

all: test

ROOT = ../../..

SRC = $(ROOT)/src

UART = $(ROOT)/posix/uart.cpp

NET = $(fr)/cpp-apps/applications/dronepong/networks/dronepong_risp_train.txt 

HDR = dronepong_fpga.hpp

$(HDR): $(NET)
	$(ROOT)/utils/mkhdr.py $(NET) > $(HDR)

load: $(NET)
	$(ROOT)/utils/load.py -s -t cmoda7_35t $(NET)

test: main.cpp $(SRC)/*.hpp $(UART) $(HDR)
	g++ -Wall -I$(SRC) -I.. -o test main.cpp $(UART)

run: test
	./test

clean:
	rm -f test $(HDR)

edit:
	vim main.cpp
