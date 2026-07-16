# Copyright (c) 2026 Simon D. Levy
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

all: test

test: main.cpp serial.o *.hpp
	g++ -o test main.cpp serial.o

serial.o: serial.cpp serial.h
	g++ -Wall -c serial.cpp

run: test
	./test

clean:
	rm -rf test *.o
