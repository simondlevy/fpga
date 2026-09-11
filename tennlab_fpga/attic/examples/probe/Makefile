# Copyright (c) 2026 Simon D. Levy
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

SKETCH = $(shell basename "`pwd`")

ROOT = ../..

SRC = $(ROOT)/src

FQBN = teensy:avr:teensy40

PORT = /dev/ttyACM0

LIBS = $(HOME)/Documents/Arduino/libraries

build: $(SKETCH).ino $(SRC)/*.hpp
	arduino-cli compile --libraries $(LIBS) --libraries $(ROOT) \
		--fqbn $(FQBN) $(SKETCH).ino

flash:
	arduino-cli upload -p $(PORT) --fqbn $(FQBN) .

edit:
	vim $(SKETCH).ino

listen:
	miniterm.py $(PORT) 115200 --exit-char 3
