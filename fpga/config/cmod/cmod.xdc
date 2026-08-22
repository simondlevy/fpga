## 12 MHz Clock Signal
set_property -dict { PACKAGE_PIN L17   IOSTANDARD LVCMOS33 } [get_ports clk]
create_clock -add -name sys_clk_pin -period 83.33 -waveform {0 41.66} [get_ports clk]

## Reset Button (Active Low reset)
set_property -dict { PACKAGE_PIN A18   IOSTANDARD LVCMOS33 } [get_ports btn]

## USB-UART Bridge
set_property -dict { PACKAGE_PIN J18   IOSTANDARD LVCMOS33 } [get_ports RsTx]
set_property -dict { PACKAGE_PIN J17   IOSTANDARD LVCMOS33 } [get_ports RsRx]
