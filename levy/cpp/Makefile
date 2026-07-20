# Copyright (c) 2026 Simon D. Levy
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

all: test

test: main.cpp serial.cpp *.hpp
	g++ -Wall -o test main.cpp serial.cpp

run: test
	./test

clean:
	rm -rf test simple *.o
