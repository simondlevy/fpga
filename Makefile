# Copyright (c) 2026 Simon D. Levy
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

NETWORK = $(HOME)/Desktop/embedded-framework-simon/fpga/networks/dronepong_risp_train.txt

load:
	./load.py $(NETWORK)

iload:
	./load.py -t icebreaker $(NETWORK)

qload:
	./load.py -t c5g $(NETWORK)

clean:
	rm -rf ~/.cache/neuro_fpga/
