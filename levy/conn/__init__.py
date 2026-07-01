# Copyright (c) 2024-2025 Keegan Dent, 2026 Simon D. Levy
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

from enum import Enum, IntEnum, auto
from heapq import heapify, heappop, heappush
from threading import Thread
from time import sleep
from typing import Iterable

import bitstruct as bs
import neuro
from periphery import Serial

from fpga._math import unsigned_width, width_bits_to_bytes, width_nearest_byte

from fpga.network import charge_width, spike_value_factor

SYSTEM_BUFFER = 4096

# we're hacking Spike to support comparison
neuro.Spike.__lt__ = lambda self, other: self.time < other.time
neuro.Spike.__le__ = lambda self, other: self.time <= other.time
neuro.Spike.__gt__ = lambda self, other: self.time > other.time
neuro.Spike.__ge__ = lambda self, other: self.time >= other.time


class _InpQueue(list):
    def __init__(self, data: Iterable[neuro.Spike]):
        super().__init__(data)
        heapify(self)

    def append(self, item: neuro.Spike) -> None:
        heappush(self, item)

    def extend(self, iterable: Iterable[neuro.Spike]) -> None:
        [self.append(item) for item in iterable]

    def popleft(self) -> neuro.Spike:
        return heappop(self)


class IoType(Enum):
    DISPATCH = auto()
    STREAM = auto()


class DispatchOpcode(IntEnum):
    RUN = 0
    SPK = auto()
    SNC = auto()
    CLR = auto()


class StreamFlag(IntEnum):
    SNC = 0
    CLR = auto()


def dispatch_operand_widths(
    opc_width: int, net_num_io: int, net_charge_width: int, is_axi: bool = True
) -> tuple[int, int]:
    idx_width = unsigned_width(net_num_io - 1)
    spk_width = idx_width + net_charge_width
    operand_width = (
        width_nearest_byte(opc_width + spk_width) - opc_width
        if is_axi
        else spk_width
    )
    return idx_width, operand_width


class _IoConfig:
    def __init__(
        self,
        io_type: IoType,
        net: neuro.Network,
        num_neurons: int,
        charge_width: int = 0,
        is_axi: bool = True,
    ):
        self._network = net
        self.type = io_type
        self._num_neurons = num_neurons

        match self.type:
            case IoType.DISPATCH:
                opc_width = unsigned_width(len(DispatchOpcode) - 1)
                spk_names = ["opcode"]
                spk_fmt_str = f"u{opc_width}"
                idx_width, operand_width = dispatch_operand_widths(
                    opc_width, self._num_net_io(), self._charge_width(), is_axi
                )

                cmd_names = spk_names + ["operand"]
                cmd_fmt_str = spk_fmt_str + f"u{operand_width}"
                self.cmd_fmt = bs.compile(cmd_fmt_str, cmd_names)

                if idx_width:
                    spk_names.append("idx")
                    spk_fmt_str += f"u{idx_width}"
                if self._charge_width():
                    spk_names.append("val")
                    spk_fmt_str += f"s{self._charge_width()}"
            case IoType.STREAM:
                spk_names = [flg.name for flg in StreamFlag]
                spk_fmt_str = "b1" * len(StreamFlag)
                spk_fmt_elem = (
                    f"s{self._charge_width()}"
                    if self._charge_width()
                    else "b1"
                )
                for io in range(self._num_net_io()):
                    spk_names.append(io)
                    spk_fmt_str += spk_fmt_elem
            case _:
                raise ValueError()
        self.spk_fmt = bs.compile(spk_fmt_str, spk_names)

        self.clear()

    def clear(self):
        self.time = 0


class InpConfig(_IoConfig):

    def clear(self):
        super().clear()
        self.queue = _InpQueue([])

    def _num_net_io(self):
        return self._num_neurons

    def _charge_width(self):
        # TODO: support fires input
        return charge_width(self._network)


class OutConfig(_IoConfig):

    def clear(self):
        super().clear()
        self.queue = {out: [] for out in range(self._num_neurons)}

    def _num_net_io(self):
        return self._num_neurons

    def _charge_width(cls):
        # TODO: support charges output
        return 0

##############################################################################


class Connection:

    def __init__(
            self,
            port_name: str,
            baudrate: int,
            clock_freq: float,
            net: neuro.Network,
            num_inputs: int,
            num_outputs: int,
            charge_width: int,
            io_type: str = "DISO"):

        self._interface = Serial(port_name, baudrate)

        self._io_type = io_type.upper()

        self._network = None

        self.clear()

        self._network = net

        self._num_inputs = num_inputs

        match self._io_type[:2]:
            case "DI":
                self._inp = InpConfig(IoType.DISPATCH, self._network, num_inputs)
            case "SI":
                self._inp = InpConfig(IoType.STREAM, self._network, num_inputs)
            case _:
                raise ValueError(
                    f"Invalid inp type: {self._io_type[:2]}\nExpected: (D|S)I"
                )
        match self._io_type[2:]:
            case "DO":
                self._out = OutConfig(IoType.DISPATCH, self._network, num_outputs)
            case "SO":
                self._out = OutConfig(IoType.STREAM, self._network, num_outputs)
            case _:
                raise ValueError(
                    f"Invalid out type: {self._io_type[2:]}\nExpected: (D|S)O"
                )

        self._secs_per_run = 0.0

        max_bytes_per_run = width_bits_to_bytes(self._out.spk_fmt.calcsize())
        match self._out.type:
            case IoType.DISPATCH:
                max_bytes_per_run *= num_outputs + 1
                self._secs_per_run += (num_outputs / clock_freq)
            case IoType.STREAM:
                pass
            case _:
                raise ValueError()
        self._secs_per_run += max_bytes_per_run * 10 / baudrate
        self._max_run = SYSTEM_BUFFER // max_bytes_per_run
        self._max_runs_ahead = self._max_run

        match self._inp.type:
            case IoType.DISPATCH:
                # limited by both buffer size and command field width
                self._max_run = min(
                    2 ** (self._inp.cmd_fmt._infos[1].size) - 1,
                    self._max_run,
                )
            case IoType.STREAM:
                pass
            case _:
                raise ValueError()

    def apply_spike(self, spike: neuro.Spike) -> None:

        if spike.time < 0:
            raise RuntimeError("Spikes cannot be scheduled in the past.")
        self._inp.queue.append(
            neuro.Spike(spike.id, spike.time + self._inp.time, spike.value)
        )
        if self._inp.type == IoType.DISPATCH:
            spikes_now = []
            while (self._inp.queue and
                   self._inp.queue[0].time == self._inp.time):
                # send these spikes as soon as they arrive to reduce latency
                spikes_now.append(self._inp.queue.popleft())
            self._hw_tx(spikes_now, 0, False)

    def apply_spikes(self, spikes: list[neuro.Spike]) -> None:
        [self.apply_spike(spike) for spike in spikes]

    def clear(self) -> None:
        if self._network:
            self.clear_activity()
        self._network = None

    def clear_activity(self) -> None:

        if self._inp.type == IoType.DISPATCH:
            self._interface.write(
                self._inp.cmd_fmt.pack(
                    {
                        "opcode": DispatchOpcode.CLR,
                        "operand": 0,
                    }
                )[::-1]
            )
        self._interface.flush()
        match (self._inp.type, self._out.type):
            case (IoType.DISPATCH, IoType.DISPATCH):
                self._hw_rx(self._max_run, True)
            case _:
                while self._interface.poll(1):
                    self._interface.read(self._interface.input_waiting())

        self._inp.clear()
        self._out.clear()

    def output_count(self, out_idx: int) -> int:
        return len(self.output_vector(out_idx))

    def output_counts(self) -> list[int]:
        return [
            self.output_count(out_idx)
            for out_idx in range(self._num_outputs)]
        

    def output_last_fire(self, out_idx: int) -> float:
        outs = self.output_vector(out_idx)
        return outs[-1] if outs else -1

    def output_last_fires(self) -> list[float]:
        return [
            self.output_last_fire(out_idx)
            for out_idx in range(self._num_outputs)
        ]

    def output_vector(self, out_idx: int) -> list[float]:

        return [
            t - self._last_run
            for t in self._out.queue[out_idx] if t >= self._last_run
        ]

    def output_vectors(self) -> list[list[float]]:
        return [
            self.output_vector(out_idx)
            for out_idx in range(self._num_outputs)
        ]

    def run(self, time: int) -> None:

        if time < 1:
            raise ValueError("Cannot run for less than 1 timestep")
        target_time = self._inp.time + time
        rx_thread = Thread(target=self._hw_rx, args=(target_time,))
        rx_thread.daemon = True
        rx_thread.start()
        self._last_run = self._inp.time
        while self._inp.time < target_time:
            spikes = []
            while (self._inp.queue and
                   int(self._inp.queue[0].time) == self._inp.time):
                spikes.append(self._inp.queue.popleft())
            run_time = (int(self._inp.queue[0].time)
                        if self._inp.queue
                        else target_time)
            self._hw_tx(
                spikes,
                run_time - self._inp.time,
                (run_time == target_time),
            )
        rx_thread.join()

    def _hw_rx(self, target: int, seek_clr: bool = False) -> None:
        num_rx_bytes = width_bits_to_bytes(self._out.spk_fmt.calcsize())

        while True:
            if (
                self._inp.time != target
                and self._out.time == self._inp.time
                and not seek_clr
            ):
                sleep(100e-9)
                continue
            rx = self._interface.read(
                num_rx_bytes,
                10.0,
            )[::-1]
            if len(rx) != num_rx_bytes:
                raise RuntimeError(
                        "Did not receive coherent response from target.")

            match self._out.type:
                case IoType.DISPATCH:
                    out_dict = self._out.spk_fmt.unpack(rx)
                    match out_dict["opcode"]:
                        case DispatchOpcode.RUN:
                            ran = self._out.cmd_fmt.unpack(rx)["operand"]
                            self._out.time += ran
                        case DispatchOpcode.SPK:
                            out_idx = (
                                out_dict["idx"]
                                if len(self._out.spk_fmt._infos) > 1
                                and self._out.spk_fmt._infos[1].name == "idx"
                                else 0
                            )
                            self._out.queue[out_idx].append(
                                    float(self._out.time))
                        case DispatchOpcode.SNC:
                            break
                        case DispatchOpcode.CLR:
                            if seek_clr:
                                return
                            elif (self._out.time and
                                  self._inp.type == IoType.DISPATCH):
                                raise RuntimeError(
                                    "Should not have received CLR during run()"
                                )
                            else:
                                self._out.clear()
                        case _:
                            raise ValueError()

                case IoType.STREAM:
                    out_dict = self._out.spk_fmt.unpack(rx)
                    for out_idx in range(self._num_outputs):
                        if out_dict[out_idx]:
                            self._out.queue[out_idx].append(
                                    float(self._out.time))
                    if out_dict[StreamFlag.CLR.name] and self._out.time:
                        raise RuntimeError(
                                "Should not have received CLR during run()")

                    self._out.time += 1

                    # don't check DISO because SNC can't travel back in time
                    if self._inp.type == IoType.STREAM and (
                        out_dict[StreamFlag.SNC.name] !=
                        (self._out.time == target)
                    ):
                        raise RuntimeError(
                            f"SNC flag {bool(out_dict[StreamFlag.SNC.name])}"
                            f" does NOT match timing {self._out.time}/{target}"
                        )
                    if self._out.time == target:
                        break

    def _hw_tx(
        self, spikes: Iterable[neuro.Spike], runs: int, sync: bool = False
    ) -> None:
        spike_dict = {
            self._network.get_node(s.id).input_id: int(
                s.value * spike_value_factor(self._network)
            )
            for s in spikes
        }
        if any(key < 0 for key in spike_dict.keys()):
            raise ValueError("Cannot send spikes to non-input node.")

        # TODO: magic timing will be resolved by buffers PR
        def pause(runs: int) -> None:
            self._inp.time += runs
            sleep(self._secs_per_run * runs)

        match self._inp.type:
            case IoType.DISPATCH:
                [
                    self._interface.write(
                        self._inp.spk_fmt.pack(
                            {
                                "opcode": DispatchOpcode.SPK,
                                "idx": idx,
                                "val": val,
                            }
                        )[::-1]
                    )
                    for idx, val in spike_dict.items()
                ]
                while runs:
                    to_run = min(
                        [
                            runs,
                            self._max_run,
                            self._max_runs_ahead + self._out.time -
                            self._inp.time,
                        ]
                    )
                    if not to_run:
                        sleep(100e-9)
                        continue
                    self._interface.write(
                        self._inp.cmd_fmt.pack(
                            {
                                "opcode": DispatchOpcode.RUN,
                                "operand": to_run,
                            }
                        )[::-1]
                    )
                    pause(to_run)
                    runs -= to_run
                if sync:
                    self._interface.write(
                        self._inp.cmd_fmt.pack(
                            {
                                "opcode": DispatchOpcode.SNC,
                                "operand": 0,
                            }
                        )[::-1]
                    )

            case IoType.STREAM:
                if not runs:
                    raise RuntimeError(
                        "Cannot send spikes to stream source without running."
                    )
                run_dict = {inp_idx: 0
                            for inp_idx in range(self._num_inputs)}
                run_dict[StreamFlag.SNC.name] = False
                run_dict[StreamFlag.CLR.name] = False
                temp = run_dict.copy()
                temp.update(spike_dict)
                spike_dict = temp

                spike_dict[StreamFlag.SNC.name] = sync and (runs == 1)
                if self._inp.time == 0:
                    spike_dict[StreamFlag.CLR.name] = True
                self._interface.write(self._inp.spk_fmt.pack(spike_dict)[::-1])
                pause(1)

                for r in reversed(range(runs - 1)):
                    if sync and r == 0:
                        run_dict[StreamFlag.SNC.name] = True
                    self._interface.write(
                            self._inp.spk_fmt.pack(run_dict)[::-1])
                    pause(1)
