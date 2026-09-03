# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## What this is

A Python package (`neuro_fpga`, imported as `fpga`) that compiles TENNLab neuromorphic
networks into FPGA bitstreams and provides a runtime that talks to the board over UART.
It implements `neuro.Processor`, so it is a drop-in for the TENNLab framework's simulated
processors. Networks are baked into fabric at build time — there is no run-time network
reconfiguration.

Upstream is `TENNLab-UTK/fpga` (README links point there); this checkout's remote is a fork.

## Setup

The `neuro` module comes from the TENNLab framework, which is **not** on PyPI and must be
installed first. Note the pyproject dependency is named `framework` but imports as `neuro`.

```bash
git clone git@github.com:TENNLab-UTK/framework-open.git && cd framework-open
bash scripts/create_env.sh && source pyframework/bin/activate
pip install -e .
```

Then, in this repo (inside that venv): `pip install -e .[test]` (or `.[test,dev]` for
black/flake8/isort). Linux x86-64 only, since both the framework and Vivado require it.

## Commands

```bash
SIMS=verilator WAVES=1 pytest tb/       # all sims; SIMS is colon-delimited
pytest tb/test_simple_processor.py      # one test file
find tb -iname "*.fst" -exec sh -c "gtkwave {} >/dev/null 2>&1 &" \;   # open waveforms

uart-loop basys3 /dev/ttyUSB1           # baud-rate loopback test, offers to rewrite targets.json
cd examples && ./simple.py -t cmod      # build, program, and run a network on hardware
rm -rf ~/.cache/neuro_fpga/             # drop all cached builds
nethdl <net.json> -o <out.sv>           # network -> SystemVerilog, no build
nethash <net.json>                      # the hash used for build-cache keys
packet-vis                              # Dash app for inspecting the wire protocol
```

Scripts in `examples/` read their networks via relative paths (`../networks/*.txt`), so they
must be run from inside `examples/`. They hardcode `/dev/ttyUSB1` and default to `basys3`.

Only Verilator works — the RTL is valid SystemVerilog that Icarus rejects. cocotb/Verilator
compatibility is fragile; v5.024 is the known-good Verilator.

Python style comes from `.vscode/settings.json`: black, isort `--profile=black`,
flake8 `--extend-ignore=E203,F541 --max-line-length=88`.

## Architecture

### Build pipeline

`Processor.load_network()` → `_setup_io()` → `_build_network()` → Edalize → Vivado/Quartus.

`fpga/network.py:build_network_sv()` emits a `.sv` containing a `network_config` package
(`NUM_INP`, `NUM_OUT`, `CHARGE_WIDTH`, …) and a `network` module — the neurons and synapses
wired up as literal fabric. `_build_network()` assembles an Edalize EDAM dict (explicit file
list, `toplevel: "uart_top"`, `CLK_FREQ`/`BAUD_RATE` as vlogparams) and runs the tool.

Everything is cached under `~/.cache/neuro_fpga/` (see `fpga/__init__.py` for the path vars),
with EDA projects keyed `eda/<target>/<io_type>/<nethash>`. `hash_network()` deliberately
strips non-architectural fields (node names, most `other` data) so functionally identical
networks reuse a build. **Note the cache key does not include baud rate** — change the baud
rate and you must clear the cache to force a rebuild.

### Variant selection by file list (important)

Several RTL files define the **same** module and package names. Which variant compiles is
decided by what goes into the file list, not by parameters or `ifdef`:

| Names defined | Provided by |
|---|---|
| `package source_config` + `module network_source` | `dispatch_source.sv` **or** `stream_source.sv` |
| `package sink_config` + `module network_sink` | `dispatch_sink.sv` **or** `stream_sink.sv` |
| `package processor_config` | `axis_processor.sv` **or** `axis_loop_proc.sv` **or** `basic_processor.sv` |
| `<proc>_neuron` / `<proc>_synapse` | picked from `proc_name(net)`, e.g. `risp` → `risp_neuron.sv` |

Consequences:
- **Never compile all of `fpga/rtl/` at once** — you get duplicate definitions. Both
  `_build_network()` and `tb/testing.py:runner()` construct explicit ordered file lists;
  add new modules to those lists.
- Out-of-context synthesis of anything above the network level needs the generated
  `network_config` package, so it cannot be elaborated straight from the repo.

### I/O types

`Processor(target, interface, io_type)` takes a 4-character string: first two chars select
the input path (`DI` dispatch, `SI` stream), last two the output (`DO`, `SO`) — e.g. `"DIDO"`.
Parsed in `_setup_io()`, and it also names the build-cache directory.

- **Dispatch** — opcode + operand commands (`RUN`/`SPK`/`SNC`/`CLR`). Compact when spikes
  are sparse.
- **Stream** — one fixed packet per timestep carrying flags plus every input/output value.
  Constant bandwidth regardless of activity.

The host-side packet layouts are built with `bitstruct` in `_IoConfig.__init__`
(`fpga/_processor.py`) and must stay bit-for-bit consistent with the RTL packing in
`io_configs.sv` and the corresponding source/sink module. **Changing one side requires
changing the other**; `packet-vis` helps verify.

### Board targets

A target needs an entry in `fpga/config/targets.json` (schema documented in
`fpga/config/README.md`) plus a `fpga/config/<target>/` directory containing:

- `uart_processor_top.v` — module **must** be named `uart_top`; it adapts board port names
  to `uart_processor`'s `clk`/`arstn`/`rxd`/`txd`.
- `<target>.xdc` for Vivado, or `<target>.qsf` + `<target>.sdc` for Quartus.

Port names in the top module and the constraints file must agree — they differ per board
(`btn` vs `btnC`, some boards wire status LEDs, some don't).

### UART baud rate is clock-constrained

`uart_processor.sv` times one bit as `8 * prescale` clocks, where
`prescale = CLK_FREQ / (8 * BAUD_RATE)` rounded to an integer, and `Processor.__init__`
selects `baud_rates[-1]` — the **last** entry in the target's list. A rate the clock cannot
express fails *silently*: prescale rounds to 0 or 1 and the board returns corrupt data
rather than failing the build.

So when adding or reordering `baud_rates`, check the prescale arithmetic against that
target's `clk_freq` and put the intended rate last. `uart-loop <target> <dev>` measures
what actually works on hardware and offers to rewrite the list.

`targets.json:clk_freq` is the **fabric** clock, which is not necessarily the board
oscillator. The Cmod A7's 12 MHz crystal cannot express anything above ~500 kbaud
(4 Mbaud would need prescale 0.375), so `cmod/uart_processor_top.v` instantiates an
MMCME2_BASE to synthesize a 96 MHz fabric clock — chosen because it divides exactly for
500k/1M/2M/4M baud — and `clk_freq` is 96e6 while `cmod.xdc` still constrains the 12 MHz
input pin. Vivado infers the generated clock automatically; no extra constraint is needed.
A board doing this must gate its reset on MMCM `LOCKED`. Note PLLE2 will not work here:
its minimum input is 19 MHz, so 12 MHz requires an MMCM.

## Licensing conventions

Two licenses, and new files are expected to carry the matching header:

- `fpga/rtl/**` — hardware, CERN-OHL-W-2.0 (`fpga/rtl/LICENSE`)
- everything else — software, MPL-2.0 (`/LICENSE`)
