// Copyright (c) 2024 Keegan Dent
//
// This source describes Open Hardware and is licensed under the CERN-OHL-W v2
// You may redistribute and modify this documentation and make products using
// it under the terms of the CERN-OHL-W v2 (https:/cern.ch/cern-ohl).
//
// This documentation is distributed WITHOUT ANY EXPRESS OR IMPLIED WARRANTY,
// INCLUDING OF MERCHANTABILITY, SATISFACTORY QUALITY AND FITNESS FOR A
// PARTICULAR PURPOSE. Please see the CERN-OHL-W v2 for applicable conditions.

module uart_top #(
    parameter real CLK_FREQ = 100_000_000,
    parameter integer BAUD_RATE = 115_200
) (
    input wire clk,
    input wire btn,
    input wire RsRx,
    output wire RsTx
);
    uart_processor #(
        .CLK_FREQ(CLK_FREQ),
        .BAUD_RATE(BAUD_RATE)
    ) uart_proc (
        .clk(clk),
        .arstn(!btn),
        .rxd(RsRx),
        .txd(RsTx)
    );

endmodule
