// (C) 2001-2014 Altera Corporation. All rights reserved.
// Your use of Altera Corporation's design tools, logic functions and other 
// software and tools, and its AMPP partner logic functions, and any output 
// files any of the foregoing (including device programming or simulation 
// files), and any associated documentation or information are expressly subject 
// to the terms and conditions of the Altera Program License Subscription 
// Agreement, Altera MegaCore Function License Agreement, or other applicable 
// license agreement, including, without limitation, that your use is for the 
// sole purpose of programming logic devices manufactured by Altera and sold by 
// Altera or its authorized distributors.  Please refer to the applicable 
// agreement for further details.


`timescale 1 ns / 1 ns

import verbosity_pkg::*;
import avalon_mm_pkg::*;
import mgc_axi_pkg::*;

module fluid_board_soc_hps_0_fpga_interfaces
(
   output wire [  0:  0] h2f_rst_n,
   input  wire [  0:  0] h2f_lw_axi_clk,
   output wire [ 11:  0] h2f_lw_AWID,
   output wire [ 20:  0] h2f_lw_AWADDR,
   output wire [  3:  0] h2f_lw_AWLEN,
   output wire [  2:  0] h2f_lw_AWSIZE,
   output wire [  1:  0] h2f_lw_AWBURST,
   output wire [  1:  0] h2f_lw_AWLOCK,
   output wire [  3:  0] h2f_lw_AWCACHE,
   output wire [  2:  0] h2f_lw_AWPROT,
   output wire [  0:  0] h2f_lw_AWVALID,
   input  wire [  0:  0] h2f_lw_AWREADY,
   output wire [ 11:  0] h2f_lw_WID,
   output wire [ 31:  0] h2f_lw_WDATA,
   output wire [  3:  0] h2f_lw_WSTRB,
   output wire [  0:  0] h2f_lw_WLAST,
   output wire [  0:  0] h2f_lw_WVALID,
   input  wire [  0:  0] h2f_lw_WREADY,
   input  wire [ 11:  0] h2f_lw_BID,
   input  wire [  1:  0] h2f_lw_BRESP,
   input  wire [  0:  0] h2f_lw_BVALID,
   output wire [  0:  0] h2f_lw_BREADY,
   output wire [ 11:  0] h2f_lw_ARID,
   output wire [ 20:  0] h2f_lw_ARADDR,
   output wire [  3:  0] h2f_lw_ARLEN,
   output wire [  2:  0] h2f_lw_ARSIZE,
   output wire [  1:  0] h2f_lw_ARBURST,
   output wire [  1:  0] h2f_lw_ARLOCK,
   output wire [  3:  0] h2f_lw_ARCACHE,
   output wire [  2:  0] h2f_lw_ARPROT,
   output wire [  0:  0] h2f_lw_ARVALID,
   input  wire [  0:  0] h2f_lw_ARREADY,
   input  wire [ 11:  0] h2f_lw_RID,
   input  wire [ 31:  0] h2f_lw_RDATA,
   input  wire [  1:  0] h2f_lw_RRESP,
   input  wire [  0:  0] h2f_lw_RLAST,
   input  wire [  0:  0] h2f_lw_RVALID,
   output wire [  0:  0] h2f_lw_RREADY,
   input  wire [  0:  0] h2f_axi_clk,
   output wire [ 11:  0] h2f_AWID,
   output wire [ 29:  0] h2f_AWADDR,
   output wire [  3:  0] h2f_AWLEN,
   output wire [  2:  0] h2f_AWSIZE,
   output wire [  1:  0] h2f_AWBURST,
   output wire [  1:  0] h2f_AWLOCK,
   output wire [  3:  0] h2f_AWCACHE,
   output wire [  2:  0] h2f_AWPROT,
   output wire [  0:  0] h2f_AWVALID,
   input  wire [  0:  0] h2f_AWREADY,
   output wire [ 11:  0] h2f_WID,
   output wire [ 63:  0] h2f_WDATA,
   output wire [  7:  0] h2f_WSTRB,
   output wire [  0:  0] h2f_WLAST,
   output wire [  0:  0] h2f_WVALID,
   input  wire [  0:  0] h2f_WREADY,
   input  wire [ 11:  0] h2f_BID,
   input  wire [  1:  0] h2f_BRESP,
   input  wire [  0:  0] h2f_BVALID,
   output wire [  0:  0] h2f_BREADY,
   output wire [ 11:  0] h2f_ARID,
   output wire [ 29:  0] h2f_ARADDR,
   output wire [  3:  0] h2f_ARLEN,
   output wire [  2:  0] h2f_ARSIZE,
   output wire [  1:  0] h2f_ARBURST,
   output wire [  1:  0] h2f_ARLOCK,
   output wire [  3:  0] h2f_ARCACHE,
   output wire [  2:  0] h2f_ARPROT,
   output wire [  0:  0] h2f_ARVALID,
   input  wire [  0:  0] h2f_ARREADY,
   input  wire [ 11:  0] h2f_RID,
   input  wire [ 63:  0] h2f_RDATA,
   input  wire [  1:  0] h2f_RRESP,
   input  wire [  0:  0] h2f_RLAST,
   input  wire [  0:  0] h2f_RVALID,
   output wire [  0:  0] h2f_RREADY,
   input  wire [ 31:  0] f2h_irq_p0,
   input  wire [ 31:  0] f2h_irq_p1,
   input  wire [  0:  0] uart1_cts,
   input  wire [  0:  0] uart1_dsr,
   input  wire [  0:  0] uart1_dcd,
   input  wire [  0:  0] uart1_ri,
   output wire [  0:  0] uart1_dtr,
   output wire [  0:  0] uart1_rts,
   output wire [  0:  0] uart1_out1_n,
   output wire [  0:  0] uart1_out2_n,
   input  wire [  0:  0] uart1_rxd,
   output wire [  0:  0] uart1_txd
);




   altera_avalon_reset_source #(
      .ASSERT_HIGH_RESET(0),
      .INITIAL_RESET_CYCLES(1)
   ) h2f_reset_inst (
      .reset(h2f_rst_n),
      .clk('0)
   );

   mgc_axi_master #(
      .AXI_ID_WIDTH(12),
      .AXI_ADDRESS_WIDTH(30),
      .AXI_WDATA_WIDTH(64),
      .AXI_RDATA_WIDTH(64)
   ) h2f_axi_master_inst (
      .ARSIZE(h2f_ARSIZE),
      .AWUSER(),
      .WVALID(h2f_WVALID),
      .RLAST(h2f_RLAST),
      .ACLK(h2f_axi_clk),
      .RRESP(h2f_RRESP),
      .ARREADY(h2f_ARREADY),
      .ARPROT(h2f_ARPROT),
      .ARADDR(h2f_ARADDR),
      .BVALID(h2f_BVALID),
      .ARID(h2f_ARID),
      .BID(h2f_BID),
      .ARBURST(h2f_ARBURST),
      .ARCACHE(h2f_ARCACHE),
      .AWVALID(h2f_AWVALID),
      .WDATA(h2f_WDATA),
      .ARUSER(),
      .RID(h2f_RID),
      .RVALID(h2f_RVALID),
      .WREADY(h2f_WREADY),
      .AWLOCK(h2f_AWLOCK),
      .BRESP(h2f_BRESP),
      .ARLEN(h2f_ARLEN),
      .AWSIZE(h2f_AWSIZE),
      .AWLEN(h2f_AWLEN),
      .BREADY(h2f_BREADY),
      .AWID(h2f_AWID),
      .RDATA(h2f_RDATA),
      .AWREADY(h2f_AWREADY),
      .ARVALID(h2f_ARVALID),
      .WLAST(h2f_WLAST),
      .ARESETn(h2f_rst_n),
      .AWPROT(h2f_AWPROT),
      .WID(h2f_WID),
      .AWADDR(h2f_AWADDR),
      .AWCACHE(h2f_AWCACHE),
      .ARLOCK(h2f_ARLOCK),
      .AWBURST(h2f_AWBURST),
      .RREADY(h2f_RREADY),
      .WSTRB(h2f_WSTRB)
   );

   fluid_board_soc_hps_0_fpga_interfaces_uart1 uart1_inst (
      .sig_uart1_txd(uart1_txd),
      .sig_uart1_cts(uart1_cts),
      .sig_uart1_out1_n(uart1_out1_n),
      .sig_uart1_dtr(uart1_dtr),
      .sig_uart1_rts(uart1_rts),
      .sig_uart1_out2_n(uart1_out2_n),
      .sig_uart1_rxd(uart1_rxd),
      .sig_uart1_dsr(uart1_dsr),
      .sig_uart1_ri(uart1_ri),
      .sig_uart1_dcd(uart1_dcd)
   );

   altera_avalon_interrupt_sink #(
      .ASSERT_HIGH_IRQ(1),
      .AV_IRQ_W(32),
      .ASYNCHRONOUS_INTERRUPT(1)
   ) f2h_irq1_inst (
      .irq(f2h_irq_p1),
      .reset('0),
      .clk('0)
   );

   altera_avalon_interrupt_sink #(
      .ASSERT_HIGH_IRQ(1),
      .AV_IRQ_W(32),
      .ASYNCHRONOUS_INTERRUPT(1)
   ) f2h_irq0_inst (
      .irq(f2h_irq_p0),
      .reset('0),
      .clk('0)
   );

   mgc_axi_master #(
      .AXI_ID_WIDTH(12),
      .AXI_ADDRESS_WIDTH(21),
      .AXI_WDATA_WIDTH(32),
      .AXI_RDATA_WIDTH(32)
   ) h2f_lw_axi_master_inst (
      .ARSIZE(h2f_lw_ARSIZE),
      .AWUSER(),
      .WVALID(h2f_lw_WVALID),
      .RLAST(h2f_lw_RLAST),
      .ACLK(h2f_lw_axi_clk),
      .RRESP(h2f_lw_RRESP),
      .ARREADY(h2f_lw_ARREADY),
      .ARPROT(h2f_lw_ARPROT),
      .ARADDR(h2f_lw_ARADDR),
      .BVALID(h2f_lw_BVALID),
      .ARID(h2f_lw_ARID),
      .BID(h2f_lw_BID),
      .ARBURST(h2f_lw_ARBURST),
      .ARCACHE(h2f_lw_ARCACHE),
      .AWVALID(h2f_lw_AWVALID),
      .WDATA(h2f_lw_WDATA),
      .ARUSER(),
      .RID(h2f_lw_RID),
      .RVALID(h2f_lw_RVALID),
      .WREADY(h2f_lw_WREADY),
      .AWLOCK(h2f_lw_AWLOCK),
      .BRESP(h2f_lw_BRESP),
      .ARLEN(h2f_lw_ARLEN),
      .AWSIZE(h2f_lw_AWSIZE),
      .AWLEN(h2f_lw_AWLEN),
      .BREADY(h2f_lw_BREADY),
      .AWID(h2f_lw_AWID),
      .RDATA(h2f_lw_RDATA),
      .AWREADY(h2f_lw_AWREADY),
      .ARVALID(h2f_lw_ARVALID),
      .WLAST(h2f_lw_WLAST),
      .ARESETn(h2f_rst_n),
      .AWPROT(h2f_lw_AWPROT),
      .WID(h2f_lw_WID),
      .AWADDR(h2f_lw_AWADDR),
      .AWCACHE(h2f_lw_AWCACHE),
      .ARLOCK(h2f_lw_ARLOCK),
      .AWBURST(h2f_lw_AWBURST),
      .RREADY(h2f_lw_RREADY),
      .WSTRB(h2f_lw_WSTRB)
   );

endmodule 

