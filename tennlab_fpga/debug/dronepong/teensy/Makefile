# Copyright (c) 2026 Simon D. Levy
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

SKETCH = $(shell basename "`pwd`")

ROOT = ../../..

SRC = $(ROOT)/src

FQBN = teensy:avr:teensy40

FPGA_DIR = $(HOME)/Desktop/fpga

PORT = /dev/ttyACM0

NET = $(fr)/cpp-apps/applications/dronepong/networks/dronepong_risp_train.txt 

LIBS = $(HOME)/Documents/Arduino/libraries

build: $(SKETCH).ino $(SRC)/*.hpp
	arduino-cli compile --libraries $(LIBS) --libraries $(ROOT) \
		--fqbn $(FQBN) $(SKETCH).ino

flash:
	arduino-cli upload -p $(PORT) --fqbn $(FQBN) .

load: $(NET)
	$(FPGA_DIR)/utils/load.py -s -t cmoda7_35t_pmod $(NET)

edit:
	vim $(SKETCH).ino

fclean:
	rm -rf $(HOME)/.cache/neuro_fpga/

listen:
	miniterm.py $(PORT) 115200 --exit-char 3
