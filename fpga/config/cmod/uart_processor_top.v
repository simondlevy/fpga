// Copyright (c) Simon D. Levy
//
// This source describes Open Hardware and is licensed under the CERN-OHL-W v2
// You may redistribute and modify this documentation and make products using
// it under the terms of the CERN-OHL-W v2 (https:/cern.ch/cern-ohl).
//
// This documentation is distributed WITHOUT ANY EXPRESS OR IMPLIED WARRANTY,
// INCLUDING OF MERCHANTABILITY, SATISFACTORY QUALITY AND FITNESS FOR A
// PARTICULAR PURPOSE. Please see the CERN-OHL-W v2 for applicable conditions.

// CLK_FREQ is the synthesized fabric clock, NOT the 12 MHz board oscillator.
// uart_processor times a bit as (8 * prescale) clocks with prescale an integer,
// so 12 MHz cannot express any baud rate above ~500 kbaud: 4 Mbaud would need
// prescale 0.375. An MMCM raises the fabric clock to 96 MHz, which divides
// exactly for 500k, 1M, 2M and 4M baud (and lands 115200 within 0.16%).
module uart_top #(
    parameter real CLK_FREQ = 96_000_000,
    parameter integer BAUD_RATE = 4_000_000
) (
    input wire clk,
    input wire btn,
    input wire RsRx,
    output wire RsTx
);
    wire clk_fb, clk_raw, mmcm_locked, sysclk;

    // 12 MHz / 1 * 64 = 768 MHz VCO (within the -1 part's 600-1200 MHz range),
    // / 8 = 96 MHz.
    MMCME2_BASE #(
        .CLKIN1_PERIOD(83.333),
        .DIVCLK_DIVIDE(1),
        .CLKFBOUT_MULT_F(64.0),
        .CLKOUT0_DIVIDE_F(8.0)
    ) mmcm (
        .CLKIN1(clk),
        .CLKFBIN(clk_fb),
        .CLKFBOUT(clk_fb),
        .CLKOUT0(clk_raw),
        .LOCKED(mmcm_locked),
        .PWRDWN(1'b0),
        .RST(1'b0)
    );

    BUFG sysclk_buf (
        .I(clk_raw),
        .O(sysclk)
    );

    uart_processor #(
        .CLK_FREQ(CLK_FREQ),
        .BAUD_RATE(BAUD_RATE)
    ) uart_proc (
        .clk(sysclk),
        // held in reset until the MMCM locks
        .arstn(mmcm_locked && !btn),
        .rxd(RsRx),
        .txd(RsTx)
    );

endmodule
