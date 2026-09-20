# Copyright (c) 2026 Simon D. Levy
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

all: test

ROOT = $(HOME)/Desktop/fpga/tennlab_fpga

SRC = $(ROOT)/src

UART = $(ROOT)/posix/uart.cpp

EMBEDDER = $(HOME)/Desktop/embedded-framework/bin/framework_embedder

FPGA_DIR = $(HOME)//Desktop/fpga

FPGA_TL_DIR = $(FPGA_DIR)/tennlab_fpga

HDR = network_config.h

NET = $(fr)/cpp-apps/applications/dronepong/networks/dronepong_risp_train.txt 

test: main.cpp $(SRC)/*.hpp $(UART) $(HDR) embedded_dronepong.cpp $(HDR)
	g++ -Wall -I. -I$(SRC) -I.. -o test main.cpp $(UART) embedded_dronepong.cpp

embedded_dronepong.cpp: $(NET) $(HDR)
	$(EMBEDDER) < $(NET) > embedded_dronepong.cpp

$(HDR): $(NET)
	$(FPGA_TL_DIR)/utils/compile.py -d $(NET)

load: $(NET)
	$(FPGA_DIR)/utils/load.py -s -t cmoda7_35t $(NET)

run: test
	./test

prun:
	./pytest.py

clean:
	rm -f test $(HDR) embedded_dronepong.cpp 

fclean:
	rm -rf $(HOME)./cache/neuro_fpga/

edit:
	vim main.cpp
