# Copyright (c) 2026 Simon D. Levy
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

all: probe

probe: main.cpp ../usb_serial.hpp ../../src/serial.hpp
	g++ -Wall -I../../src -I.. -o probe main.cpp

run: probe
	./probe

edit:
	vim main.cpp

clean:
	rm -rf probe
