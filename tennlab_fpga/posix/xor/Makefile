# Copyright (c) 2026 Simon D. Levy
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

all: test

ROOT = ../..

SRC = $(ROOT)/src

NET = $(ROOT)/networks/xor.txt

test: main.cpp xor.hpp ../posix.cpp $(SRC)/*.hpp
	g++ -Wall -I$(SRC) -I.. -o test main.cpp ../posix.cpp

xor.hpp: $(NET)
	$(ROOT)/utils/mkhdr.py $(NET) -o ./xor.hpp -d

run: test
	./test

edit:
	vim main.cpp

clean:
	rm -rf test xor.hpp
