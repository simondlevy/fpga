# Copyright (c) 2024-2025 Keegan Dent, 2026 Simon D. Levy
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

import bitstruct as bs
from enum import IntEnum, auto
from heapq import heapify, heappop, heappush
from math import ceil, log2
from periphery import Serial
from threading import Thread
from time import sleep
from typing import Iterable

SYSTEM_BUFFER_SIZE_BYTES = 4096
READ_TIMEOUT_SEC = 10.0
MAX_INPUTS = 55555

def clog2(value: float) -> int:
    return int(ceil(log2(value)))


def signed_width(value: int) -> int:
    return clog2(abs(value) + int(value >= 0)) + 1


def unsigned_width(value: int) -> int:
    if value < 0:
        raise ValueError(f"Value of {value} is negative")
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

    def __lt__(self, other):
        return self.time < other.time

    def __le__(self, other):
        return self.time <= other.time

    def __gt__(self, other):
        return self.time > other.time

    def __ge__(self, other):
        return self.time >= other.time


class _InpQueue(list):
    def __init__(self, data: Iterable[Spike]):
        super().__init__(data)
        heapify(self)

    def append(self, item: Spike) -> None:
        heappush(self, item)

    def extend(self, iterable: Iterable[Spike]) -> None:
        [self.append(item) for item in iterable]

    def popleft(self) -> Spike:
        return heappop(self)


class DispatchOpcode(IntEnum):
    RUN = 0
    SPK = auto()
    SNC = auto()
    CLR = auto()


class IoConfig:

    def __init__(self,
            num_neurons: int,
            charge_width: int,
            usable_charge_width: int,
            is_axi: bool = True):

        self._num_neurons = num_neurons
        self._charge_width = charge_width

        self.opc_width = unsigned_width(len(DispatchOpcode) - 1)

        self.idx_width = unsigned_width(num_neurons - 1)

        self.usable_charge_width = usable_charge_width
  
        spk_width = self.idx_width + usable_charge_width

        self.operand_width = (
            width_nearest_byte(self.opc_width + spk_width) - self.opc_width
            if is_axi
            else spk_width)

        newsize = self.opc_width

        spk_names = ["opcode"]
        self.spk_fmt_str = f"u{self.opc_width}" ##############
        cmd_names = spk_names + ["operand"]
        cmd_fmt_str = self.spk_fmt_str + f"u{self.operand_width}"
        self.cmd_fmt = bs.compile(cmd_fmt_str, cmd_names)

        if self.idx_width:
            spk_names.append("idx")
            self.spk_fmt_str += f"u{self.idx_width}" ###############
            newsize += self.idx_width

        if usable_charge_width > 0:
            spk_names.append("val")
            self.spk_fmt_str += f"s{usable_charge_width}" ##############
            newsize += usable_charge_width

        self.spk_fmt = bs.compile(self.spk_fmt_str, spk_names)

        self.clear()

        self.num_bytes = width_bits_to_bytes(newsize)


    def __str__(self):

        s = ('opc_width=%d idx_width=%d usable_charge_width=%d spike_ft_str=%s' %
            (self.opc_width, self.idx_width, self.usable_charge_width,
             self.spk_fmt_str))
        '''
        print('spike_names=', spk_names, end=' | ')
        print(('idx_width=%d self.opc_width=%d operand_width=%d ' + 
              'spk_fmt_str=%s size=%d newsize=%d\n') %
                (self.idx_width, self.opc_width, operand_width, spk_fmt_str,
                 self.spk_fmt.calcsize(), newsize))
        '''
        return s

 
    def clear(self):
        self.time = 0


##############################################################################


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

        self._inp_config = IoConfig(num_inputs, charge_width, charge_width)

        print(self._inp_config)

        self._out_config = IoConfig(num_outputs, charge_width, 0)

        self._inp_queue = None
        self._out_queue = None

        output_size_bits = self._out_config.spk_fmt.calcsize()

        max_bytes_per_run = (
                width_bits_to_bytes(output_size_bits) * (num_outputs + 1))

        self._secs_per_run = (num_outputs / clock_freq +
                max_bytes_per_run * 10 / baudrate)

        self._max_runs_ahead = SYSTEM_BUFFER_SIZE_BYTES // max_bytes_per_run

        self._max_run = min(2 ** self._inp_config.cmd_fmt._infos[1].size - 1,
            self._max_runs_ahead)

    def apply_spike(self, spike: Spike) -> None:

        if spike.time < 0:
            raise RuntimeError("Spikes cannot be scheduled in the past.")

        self._inp_queue.append(
            Spike(spike.id, spike.time + self._inp_config.time, spike.value)
        )

        spikes_now = []
        while (self._inp_queue and
               self._inp_queue[0].time == self._inp_config.time):
            # send these spikes as soon as they arrive to reduce latency
            spikes_now.append(self._inp_queue.popleft())

        self._prepare_to_send(spikes_now)

    def clear_activity(self) -> None:

        dct = { "opcode": DispatchOpcode.CLR, "operand": 0 }
        msg = self._inp_config.cmd_fmt.pack(dct)
        self._write(msg)
 
        self._serial.flush()

        self._receive()

        self._inp_config.clear()
        self._out_config.clear()

        self._inp_queue = _InpQueue([])
        self._out_queue = {out: [] for out in range(self._num_outputs)}

    def output_count(self, out_idx: int) -> int:
        return len(self.output_vector(out_idx))

    def output_last_fire(self, out_idx: int) -> float:
        outs = self.output_vector(out_idx)
        return outs[-1] if outs else -1

    def output_vector(self, out_idx: int) -> list[float]:

        return [
            t - self._last_run
            for t in self._out_queue[out_idx] if t >= self._last_run
        ]

    def output_vectors(self) -> list[list[float]]:
        return [
            self.output_vector(out_idx)
            for out_idx in range(self._num_outputs)
        ]

    def run(self, time: int) -> None:

        if time < 1:
            raise ValueError("Cannot run for less than 1 timestep")

        target_time = self._inp_config.time + time
        rx_thread = Thread(target=self._receive)
        rx_thread.daemon = True
        rx_thread.start()
        self._last_run = self._inp_config.time

        while self._inp_config.time < target_time:

            spikes = []

            while (self._inp_queue and
                   int(self._inp_queue[0].time) == self._inp_config.time):
                spikes.append(self._inp_queue.popleft())

            run_time = (int(self._inp_queue[0].time)
                        if self._inp_queue
                        else target_time)

            self._prepare_to_send(spikes)

            runs = run_time - self._inp_config.time

            while runs:

                to_run = min(
                    [
                        runs,
                        self._max_run,
                        self._max_runs_ahead + self._out_config.time -
                        self._inp_config.time,
                    ]
                )

                if not to_run:
                    sleep(100e-9)
                    continue

                dct = { "opcode": DispatchOpcode.RUN, "operand": to_run }

                msg = self._inp_config.cmd_fmt.pack(dct)
                self._write(msg)

                self._inp_config.time += runs
                sleep(self._secs_per_run * runs)

                runs -= to_run

            if run_time == target_time:
                dct = { "opcode": DispatchOpcode.SNC, "operand": 0 }
                msg = self._inp_config.cmd_fmt.pack(dct)
                self._write(msg)

        rx_thread.join()

    def _receive(self) -> None:

        num_rx_bytes = self._out_config.num_bytes

        while True:

            rx = self._serial.read( num_rx_bytes, READ_TIMEOUT_SEC,)[::-1]

            byte = ord(rx)

            if len(rx) != num_rx_bytes:
                raise RuntimeError(
                        "Did not receive coherent response from target.")

            opc_width = self._out_config.opc_width
            idx_width = self._out_config.idx_width
            opcode = byte >> (8 - opc_width)

            operand = ((byte << opc_width) >> opc_width) & 0XFF

            match opcode:

                case DispatchOpcode.RUN:
                    ran = operand
                    self._out_config.time += ran

                case DispatchOpcode.SPK:
                    mask = 0xFF >> (8 - idx_width)
                    shift = 8 - idx_width - opc_width
                    out_idx = ((byte>>5) & mask) if idx_width > 0 else 0
                    self._out_queue[out_idx].append(float(self._out_config.time))

                case DispatchOpcode.SNC:
                    break

                case DispatchOpcode.CLR:
                    break

                case _:
                    raise ValueError()

    def _prepare_to_send(self, spikes: Iterable[Spike]) -> None:

        cfg = self._inp_config
        chgwidth = cfg.usable_charge_width

        for spike in spikes:

            dct = {"opcode": DispatchOpcode.SPK, "idx": spike.id,
                   "val": spike.value * self._spike_value_factor }
            msg = self._inp_config.spk_fmt.pack(dct)

            byte = ord(msg)
            print("dct=", dct, "write: x%02x=" % byte, f"{byte:b}")
            newbyte = (DispatchOpcode.SPK << (cfg.opc_width + chgwidth - 1) |
                       (spike.id << chgwidth) |
                       int(spike.value*self._spike_value_factor))
            newmsg = bytes([newbyte])
            print("oldbyte=", f"{byte:b}")
            print("newbyte=", f"{newbyte:b}")
            print("oldmsg=", msg)
            print("newmsg=", newmsg)

            self._write(newmsg)
 
    def _write(self, msg):
        self._serial.write(msg)
