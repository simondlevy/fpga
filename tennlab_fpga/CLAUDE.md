# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## What this is

A header-only C++ port of the TeNNLab `fpga` Python processor driver, packaged as an Arduino
library (see `library.properties`) so that a microcontroller — currently a Teensy 4.1 — can drive a
TeNNLab neuromorphic FPGA over serial in place of a PC. The same headers also build natively for
POSIX, which is the reference path: get behavior right under `posix/`, then bring it to the board.

The FPGA itself, its bitstream, and the Python `neuro` / `fpga` packages live outside this repo.

## Commands

POSIX (talks to the FPGA over `/dev/ttyUSB1`):

```bash
cd posix/xor && make run
```

Teensy 4.1:

```bash
cd examples/xor && make build && make flash && make listen
```

`make build` shells out to `arduino-cli` with `--fqbn teensy:avr:teensy41`, pulling headers from
`src/` and USBHost_t36 from the installed Teensy platform. There is no test suite and no linter.

Both targets depend on a generated `xor.hpp`, produced by `utils/mkhdr.py` from
`networks/xor.txt`. That script imports the TeNNLab `neuro` and `fpga` Python packages, so it only
runs where those are installed; the generated headers are gitignored under `posix/` but committed
under `examples/`. `utils/load.py` is separate — it flashes a network onto the FPGA over serial and
must be run before any of the C++ hosts will do anything useful.

## Architecture

**The wire protocol is bit-packed and network-dependent.** Every command is a single byte whose
fields are packed most-significant-bit first with zero padding on the right, matching Python's
`bitstruct`. A two-bit opcode (RUN/SPK/SNC/CLR, see `MessageParser`) occupies the top bits; a SPK
byte then carries an unsigned neuron index and a *signed* charge. None of these widths are fixed —
they are derived from the network's input/output counts and charge width, which reach C++ only
through the generated header's `Processor(num_inputs, num_outputs, charge_width,
spike_value_factor, debug)` call. When changing packing code, verify it against Python
`bitstruct` for several network shapes, not just the 2-input xor net, whose widths make several
wrong formulas coincidentally right.

**`Processor` (`src/processor.hpp`)** is the port of the Python driver and holds all the protocol
logic: run/sync scheduling, the spike heap, and the shift/mask arithmetic. `MessageParser` owns
field-width computation and decoding of incoming bytes. Neither touches hardware.

**`Serial` (`src/serial.hpp`) is the only hardware seam**, and its contract is not the usual
stream API — get this wrong and the symptom is silence, not a compile error:

- `Available()` gathers an entire reply into the driver's own buffer and returns its length,
  waiting until the link has been quiet for ~20 ms. The FPGA does not answer instantly, so a
  driver that reports only what is buffered at the instant of the call will report nothing.
- `Read(index)` indexes that buffer. It is not a "pop the next byte" call.

`posix_serial.hpp` implements this over termios; `teensy_serial.hpp` over USB host.

## Teensy 4.1 specifics

The Teensy is the USB *host* here, driving the FPGA board's USB-serial bridge through the 5-pin
host header. Three consequences, all learned the hard way:

- The Cmod A7's FT2232H is USB high-speed with 512-byte bulk endpoints, so plain `USBSerial`
  (which caps endpoints at 64 bytes) silently fails to claim it. `USBSerial_BigBuffer` is
  required, and its 4096-byte buffer also matches `Processor::kSystemBufferSizeBytes`.
- The FT2232H exposes two interfaces: channel A (interface 0) is JTAG, channel B (interface 1) is
  the UART. Driver objects claim interfaces in construction order, so `TeensySerial` instantiates
  two and talks on the second. This is the same split that makes `/dev/ttyUSB1` the POSIX default.
- `USBHost_t36`'s serial driver only detects an attached device inside `usb_.Task()`, so
  `TeensySerial::Poll()` must be called often from `loop()`, and the port's `begin()` is deferred
  until the device actually enumerates — calling it earlier stalls five seconds and sets nothing.

Known unfixed: `USBHost_t36`'s FTDI code computes baud rates with the legacy 3 MHz-base formula
and hardcodes `wIndex` to 0, neither of which is right for a multi-channel H-series part, so the
4 Mbaud request does not currently reach channel B. Fixing it means vendoring a patched copy of
the library, or bypassing the FTDI entirely by wiring Teensy `Serial1` to the FPGA's UART pins.

## Dead and stale code

`attic/` is retired. `posix/dronepong/` does not build — it references a `connection.hpp` and
`serial.cpp` that no longer exist — and predates the current `Serial` abstraction; don't take it as
a model.

## Current limitation

`Processor::WriteByte` emits exactly one byte per message. Networks whose opcode + index + charge
fields exceed 8 bits (which `bitstruct` would pad to two bytes) are not yet supported.
