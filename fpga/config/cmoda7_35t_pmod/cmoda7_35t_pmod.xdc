## 12 MHz Clock Signal
set_property -dict { PACKAGE_PIN L17   IOSTANDARD LVCMOS33 } [get_ports clk]
create_clock -add -name sys_clk_pin -period 83.33 -waveform {0 41.66} [get_ports clk]

## Reset Button (Active Low reset)
set_property -dict { PACKAGE_PIN A18   IOSTANDARD LVCMOS33 } [get_ports btn]

## UART on Pmod JA, named from the FPGA's point of view. These are ja[0] and
## ja[1], the first two pins of the header's top row, so the peer's TX, the
## peer's RX and a common ground land on JA pins 1, 2 and 5.
##   JA pin 1 (G17) <- peer TX
##   JA pin 2 (G19) -> peer RX
##   JA pin 5       <> GND
## The header is 3.3V LVCMOS and the Artix-7 is not 5V tolerant.
set_property -dict { PACKAGE_PIN G17   IOSTANDARD LVCMOS33 } [get_ports uart_rxd]
set_property -dict { PACKAGE_PIN G19   IOSTANDARD LVCMOS33 } [get_ports uart_txd]

## An idle UART line sits high. Unlike the USB bridge, which always drives its
## end, a header pin reads as noise while nothing is wired to it, which the
## receiver would decode as a stream of framing errors.
set_property PULLTYPE PULLUP [get_ports uart_rxd]

## SPI non-volatile loading
set_property BITSTREAM.CONFIG.SPI_BUSWIDTH 4 [current_design]
set_property BITSTREAM.GENERAL.COMPRESS TRUE [current_design]
set_property BITSTREAM.CONFIG.CONFIGRATE 33 [current_design]
set_property CONFIG_MODE SPIx4 [current_design]
