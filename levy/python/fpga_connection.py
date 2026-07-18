# Copyright (c) 2024-2025 Keegan Dent, 2026 Simon D. Levy
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

from enum import IntEnum, auto
from math import ceil, log2
from periphery import Serial
from threading import Thread
from time import sleep
from typing import Iterable
import numpy as np

from heap import Heap

SYSTEM_BUFFER_SIZE_BYTES = 4096
READ_TIMEOUT_SEC = 10.0
MAX_INPUT_SPIKES = 1024
MAX_OUTPUT_NEURONS = 16
MAX_SPIKES_PER_OUTPUT_NEURON = 256


class DispatchOpcode(IntEnum):
    RUN = 0
    SPK = auto()
    SNC = auto()
    CLR = auto()


def clog2(value: float) -> int:
    return int(ceil(log2(value)))


def signed_width(value: int) -> int:
    return clog2(abs(value) + int(value >= 0)) + 1


def unsigned_width(value: int) -> int:
    return signed_width(value) - 1


def width_bytes_to_bits(bytes: int) -> int:
    return int(bytes * 8)


def width_bits_to_bytes(bits: int) -> int:
    return int(ceil(bits / 8))


def width_nearest_byte(bits: int) -> int:
    return width_bytes_to_bits(width_bits_to_bytes(bits))


class Spike:

    def __init__(self, ident: int, time: float, value: float):

        self.id = ident
        self.time = time
        self.value = value

    def __str__(self):
        return "id=%d time=%f value=%f" % (self.id, self.time, self.value)

    def __lt__(self, other):
        return self.time < other.time


class OutputQueue:

    def __init__(self):
        self.times = (-1 * np.ones((MAX_OUTPUT_NEURONS,
                                    MAX_SPIKES_PER_OUTPUT_NEURON)))
        self.counts = [0] * MAX_OUTPUT_NEURONS

    def append(self, index, time):
        self.times[index][self.counts[index]] = time
        self.counts[index] += 1


class FpgaConnection:

    def __init__(
            self,
            port_name: str,
            baudrate: int,
            clock_freq: float,
            num_inputs: int,
            num_outputs: int,
            charge_width: int,
            spike_value_factor: int):

        self._serial = Serial(port_name, baudrate)

        self._num_inputs = num_inputs
        self._num_outputs = num_outputs

        self._spike_value_factor = spike_value_factor

        self._opcode_width = unsigned_width(len(DispatchOpcode) - 1)

        self._charge_width = charge_width

        idx_width = unsigned_width(num_inputs - 1)

        spk_width = idx_width + charge_width

        operand_width = (
            width_nearest_byte(self._opcode_width + spk_width) -
             self._opcode_width)

        self._output_time = 0
        self._input_time = 0

        self._inp_queue = Heap(MAX_INPUT_SPIKES)

        self._out_queue = OutputQueue()

        self._output_idx_width = unsigned_width(num_outputs - 1) 

        output_size_bits = self._opcode_width + self._output_idx_width

        max_bytes_per_run = (
                width_bits_to_bytes(output_size_bits) * (num_outputs + 1))

        self._secs_per_run = (num_outputs / clock_freq +
                              max_bytes_per_run * 10 / baudrate)

        self._max_runs_ahead = SYSTEM_BUFFER_SIZE_BYTES // max_bytes_per_run

        self._max_run = min(
                2 ** operand_width - 1, self._max_runs_ahead)

    def apply_spike(self, spike: Spike) -> None:

        self._inp_queue.push(
            Spike(spike.id, spike.time + self._input_time, spike.value)
        )

        spikes_now = [None] * MAX_INPUT_SPIKES
        count = 0

        while (not self._inp_queue.isempty() and
               self._inp_queue.peek().time == self._input_time):
            # send these spikes as soon as they arrive to reduce latency
            spikes_now[count] = self._inp_queue.pop()
            count += 1

        self._prepare_to_send(spikes_now, count)

    def clear_activity(self) -> None:

        self._send_command(DispatchOpcode.CLR)

        self._serial.flush()

        rx = self._serial.read(1, READ_TIMEOUT_SEC,)[::-1]
        byte = ord(rx)
        opcode = byte >> (8 - self._opcode_width)
        if opcode != DispatchOpcode.CLR:
            raise ValueError()

        self._input_time = 0
        self._output_time = 0

        self._inp_queue = Heap(MAX_INPUT_SPIKES)
        self._out_queue = OutputQueue()

    def output_count(self, out_idx: int) -> int:

        '''
        for k in range(self._out_queue.counts[out_idx]):
            print(self._out_queue.times[out_idx][k], end=' ')
        print()
        '''

        return self._out_queue.counts[out_idx]

    def run(self, time: int) -> None:

        rx_thread = Thread(target=self._receive)
        rx_thread.daemon = True
        rx_thread.start()

        target_time = self._input_time + time

        while self._input_time < target_time:

            spikes = [None] * MAX_INPUT_SPIKES
            count = 0

            # print('%d ===================' % len(self._inp_queue))

            while True:

                if self._inp_queue.isempty():
                    break

                spike = self._inp_queue.peek()

                # print(spike)

                if spike.time != self._input_time:
                    # print("break: %f %d" % (spike.time, self._input_time))
                    break

                spikes[count] = self._inp_queue.pop()
                count += 1

            # print("empty=%d" % self._inp_queue.isempty())

            run_time = (int(self._inp_queue.peek().time)
                        if not self._inp_queue.isempty()
                        else target_time)

            # print("run_time=%d input_time=%d" % (run_time, self._input_time))

            self._prepare_to_send(spikes, count)

            runs = run_time - self._input_time

            while runs > 0:

                to_run = min(min(
                    runs,
                    self._max_run),
                    self._max_runs_ahead + self._output_time - self._input_time)

                if to_run == 0:
                    sleep(0) # yield to other thread
                    continue

                self._send_command(DispatchOpcode.RUN, to_run)

                self._input_time += runs

                sleep(self._secs_per_run * runs)

                runs -= to_run

            if run_time == target_time:

                self._send_command(DispatchOpcode.SNC)

        rx_thread.join()

    def _send_command(self, opcode: int, operand: int = 0) -> None:

        self._write_byte(opcode << (8 - self._opcode_width) | operand)

    def _receive(self) -> None:

        print("\nreceive")

        while True:

            rx = self._serial.read(1, READ_TIMEOUT_SEC,)[::-1]

            byte = ord(rx)

            opcode = byte >> (8 - self._opcode_width)

            print('opcode=x%02X: ' % opcode, end='')

            match opcode:

                case DispatchOpcode.RUN:
                    print("run");
                    operand = (((byte << self._opcode_width) >> self._opcode_width) & 0XFF)
                    self._output_time += operand

                case DispatchOpcode.SPK:
                    print("spk");
                    idx_width = self._output_idx_width
                    mask = 0xFF >> (8 - idx_width)
                    out_idx = ((byte >> 5) & mask) if idx_width > 0 else 0
                    print("append: %d" % out_idx)
                    self._out_queue.append(out_idx, float(self._output_time))

                case DispatchOpcode.SNC:
                    print("snc");
                    break

                case _:
                    raise ValueError()

    def _prepare_to_send(self, spikes: Iterable[Spike], count: int) -> None:

        chgwidth = self._charge_width

        for k in range(count):

            spike = spikes[k]

            byte = (DispatchOpcode.SPK << (self._opcode_width + chgwidth - 1) |
                    (spike.id << chgwidth) |
                    int(spike.value*self._spike_value_factor))

            self._write_byte(byte)

    def _write_byte(self, byte):
        print("WriteByte: x%02X" % byte)
        self._serial.write(bytes([byte]))
