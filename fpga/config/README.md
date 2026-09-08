<!--
 Copyright (c) 2024 Keegan Dent

 This Source Code Form is subject to the terms of the Mozilla Public
 License, v. 2.0. If a copy of the MPL was not distributed with this
 file, You can obtain one at https://mozilla.org/MPL/2.0/.
-->

# FPGA Configuration

## `targets.json` Schema

| Key                        | Type      | Default  | Rules                                                                                                                             | Description                                                                                            |
|----------------------------|-----------|----------|-----------------------------------------------------------------------------------------------------------------------------------|--------------------------------------------------------------------------------------------------------|
| default_tool               | string    | Required | "vivado" \| "quartus"                                                                                                             | EDA tool for synthesis and implementation                                                              |
| parameters.clk_freq        | float     | Required | >0.0                                                                                                                              | Clock frequency for system clock utilized in top modules in Hertz                                      |
| parameters.uart.baud_rates | list[int] | [115200] | all in [list](https://github.com/vsergeev/python-periphery/blob/f3afcd7b5a799a066a6cf321e0456a040dd66c2c/periphery/serial.py#L19) | Validated UART baud rates in Hertz                                                                     |
| parameters.uart.buffer_rx  | int       | Required | >0                                                                                                                                | Size of buffer from peer to FPGA in bytes                                                              |
| parameters.uart.buffer_tx  | int       | Required | >0                                                                                                                                | Size of buffer from FPGA to peer in bytes                                                              |
| tools.`default_tool`       | dict      | Required | may require examining [Edalize source](https://github.com/olofk/edalize/tree/main/edalize)                                        | Edalize [tool_options](https://github.com/olofk/edalize/blob/main/doc/edam/api.rst) for `default_tool` |
| programmer                 | dict      | Optional | absent means Edalize's run stage programs the FPGA over JTAG                                                                      | External tool used to load the design onto the board                                                   |
| programmer.tool            | string    | Required | "openfpgaloader"                                                                                                                  | Programming tool                                                                                       |
| programmer.board           | string    | Required | must be listed by `openFPGALoader --list-boards`                                                                                  | Board identifier passed to `--board`                                                                   |
| programmer.flash           | bool      | true     |                                                                                                                                   | Write the `.bin` to SPI flash, which survives a power cycle, instead of volatile SRAM                  |
| programmer.verify          | bool      | true     | ignored unless `flash`                                                                                                            | Read the flash back and verify it after writing                                                        |
| programmer.quad            | bool      | false    | ignored unless `flash`; required when the design sets `CONFIG_MODE SPIx4`                                                         | Set the flash's quad-enable status bit, without which an x4 bitstream will not boot                    |
| programmer.freq            | int       | Optional | >0                                                                                                                                | JTAG clock in Hertz                                                                                    |
| programmer.executable      | string    | "openFPGALoader" |                                                                                                                           | Program name or path                                                                                   |
