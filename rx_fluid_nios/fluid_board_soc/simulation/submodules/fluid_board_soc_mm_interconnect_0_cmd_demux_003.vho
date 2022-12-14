--IP Functional Simulation Model
--VERSION_BEGIN 14.0 cbx_mgl 2014:06:17:18:10:38:SJ cbx_simgen 2014:06:17:18:06:03:SJ  VERSION_END


-- Copyright (C) 1991-2014 Altera Corporation. All rights reserved.
-- Your use of Altera Corporation's design tools, logic functions 
-- and other software and tools, and its AMPP partner logic 
-- functions, and any output files from any of the foregoing 
-- (including device programming or simulation files), and any 
-- associated documentation or information are expressly subject 
-- to the terms and conditions of the Altera Program License 
-- Subscription Agreement, the Altera Quartus II License Agreement,
-- the Altera MegaCore Function License Agreement, or other 
-- applicable license agreement, including, without limitation, 
-- that your use is for the sole purpose of programming logic 
-- devices manufactured by Altera and sold by Altera or its 
-- authorized distributors.  Please refer to the applicable 
-- agreement for further details.

-- You may only use these simulation model output files for simulation
-- purposes and expressly not for synthesis or any other purposes (in which
-- event Altera disclaims all warranties of any kind).


--synopsys translate_off

--synthesis_resources = 
 LIBRARY ieee;
 USE ieee.std_logic_1164.all;

 ENTITY  fluid_board_soc_mm_interconnect_0_cmd_demux_003 IS 
	 PORT 
	 ( 
		 clk	:	IN  STD_LOGIC;
		 reset	:	IN  STD_LOGIC;
		 sink_channel	:	IN  STD_LOGIC_VECTOR (18 DOWNTO 0);
		 sink_data	:	IN  STD_LOGIC_VECTOR (130 DOWNTO 0);
		 sink_endofpacket	:	IN  STD_LOGIC;
		 sink_ready	:	OUT  STD_LOGIC;
		 sink_startofpacket	:	IN  STD_LOGIC;
		 sink_valid	:	IN  STD_LOGIC_VECTOR (0 DOWNTO 0);
		 src0_channel	:	OUT  STD_LOGIC_VECTOR (18 DOWNTO 0);
		 src0_data	:	OUT  STD_LOGIC_VECTOR (130 DOWNTO 0);
		 src0_endofpacket	:	OUT  STD_LOGIC;
		 src0_ready	:	IN  STD_LOGIC;
		 src0_startofpacket	:	OUT  STD_LOGIC;
		 src0_valid	:	OUT  STD_LOGIC;
		 src10_channel	:	OUT  STD_LOGIC_VECTOR (18 DOWNTO 0);
		 src10_data	:	OUT  STD_LOGIC_VECTOR (130 DOWNTO 0);
		 src10_endofpacket	:	OUT  STD_LOGIC;
		 src10_ready	:	IN  STD_LOGIC;
		 src10_startofpacket	:	OUT  STD_LOGIC;
		 src10_valid	:	OUT  STD_LOGIC;
		 src11_channel	:	OUT  STD_LOGIC_VECTOR (18 DOWNTO 0);
		 src11_data	:	OUT  STD_LOGIC_VECTOR (130 DOWNTO 0);
		 src11_endofpacket	:	OUT  STD_LOGIC;
		 src11_ready	:	IN  STD_LOGIC;
		 src11_startofpacket	:	OUT  STD_LOGIC;
		 src11_valid	:	OUT  STD_LOGIC;
		 src12_channel	:	OUT  STD_LOGIC_VECTOR (18 DOWNTO 0);
		 src12_data	:	OUT  STD_LOGIC_VECTOR (130 DOWNTO 0);
		 src12_endofpacket	:	OUT  STD_LOGIC;
		 src12_ready	:	IN  STD_LOGIC;
		 src12_startofpacket	:	OUT  STD_LOGIC;
		 src12_valid	:	OUT  STD_LOGIC;
		 src13_channel	:	OUT  STD_LOGIC_VECTOR (18 DOWNTO 0);
		 src13_data	:	OUT  STD_LOGIC_VECTOR (130 DOWNTO 0);
		 src13_endofpacket	:	OUT  STD_LOGIC;
		 src13_ready	:	IN  STD_LOGIC;
		 src13_startofpacket	:	OUT  STD_LOGIC;
		 src13_valid	:	OUT  STD_LOGIC;
		 src14_channel	:	OUT  STD_LOGIC_VECTOR (18 DOWNTO 0);
		 src14_data	:	OUT  STD_LOGIC_VECTOR (130 DOWNTO 0);
		 src14_endofpacket	:	OUT  STD_LOGIC;
		 src14_ready	:	IN  STD_LOGIC;
		 src14_startofpacket	:	OUT  STD_LOGIC;
		 src14_valid	:	OUT  STD_LOGIC;
		 src15_channel	:	OUT  STD_LOGIC_VECTOR (18 DOWNTO 0);
		 src15_data	:	OUT  STD_LOGIC_VECTOR (130 DOWNTO 0);
		 src15_endofpacket	:	OUT  STD_LOGIC;
		 src15_ready	:	IN  STD_LOGIC;
		 src15_startofpacket	:	OUT  STD_LOGIC;
		 src15_valid	:	OUT  STD_LOGIC;
		 src16_channel	:	OUT  STD_LOGIC_VECTOR (18 DOWNTO 0);
		 src16_data	:	OUT  STD_LOGIC_VECTOR (130 DOWNTO 0);
		 src16_endofpacket	:	OUT  STD_LOGIC;
		 src16_ready	:	IN  STD_LOGIC;
		 src16_startofpacket	:	OUT  STD_LOGIC;
		 src16_valid	:	OUT  STD_LOGIC;
		 src17_channel	:	OUT  STD_LOGIC_VECTOR (18 DOWNTO 0);
		 src17_data	:	OUT  STD_LOGIC_VECTOR (130 DOWNTO 0);
		 src17_endofpacket	:	OUT  STD_LOGIC;
		 src17_ready	:	IN  STD_LOGIC;
		 src17_startofpacket	:	OUT  STD_LOGIC;
		 src17_valid	:	OUT  STD_LOGIC;
		 src1_channel	:	OUT  STD_LOGIC_VECTOR (18 DOWNTO 0);
		 src1_data	:	OUT  STD_LOGIC_VECTOR (130 DOWNTO 0);
		 src1_endofpacket	:	OUT  STD_LOGIC;
		 src1_ready	:	IN  STD_LOGIC;
		 src1_startofpacket	:	OUT  STD_LOGIC;
		 src1_valid	:	OUT  STD_LOGIC;
		 src2_channel	:	OUT  STD_LOGIC_VECTOR (18 DOWNTO 0);
		 src2_data	:	OUT  STD_LOGIC_VECTOR (130 DOWNTO 0);
		 src2_endofpacket	:	OUT  STD_LOGIC;
		 src2_ready	:	IN  STD_LOGIC;
		 src2_startofpacket	:	OUT  STD_LOGIC;
		 src2_valid	:	OUT  STD_LOGIC;
		 src3_channel	:	OUT  STD_LOGIC_VECTOR (18 DOWNTO 0);
		 src3_data	:	OUT  STD_LOGIC_VECTOR (130 DOWNTO 0);
		 src3_endofpacket	:	OUT  STD_LOGIC;
		 src3_ready	:	IN  STD_LOGIC;
		 src3_startofpacket	:	OUT  STD_LOGIC;
		 src3_valid	:	OUT  STD_LOGIC;
		 src4_channel	:	OUT  STD_LOGIC_VECTOR (18 DOWNTO 0);
		 src4_data	:	OUT  STD_LOGIC_VECTOR (130 DOWNTO 0);
		 src4_endofpacket	:	OUT  STD_LOGIC;
		 src4_ready	:	IN  STD_LOGIC;
		 src4_startofpacket	:	OUT  STD_LOGIC;
		 src4_valid	:	OUT  STD_LOGIC;
		 src5_channel	:	OUT  STD_LOGIC_VECTOR (18 DOWNTO 0);
		 src5_data	:	OUT  STD_LOGIC_VECTOR (130 DOWNTO 0);
		 src5_endofpacket	:	OUT  STD_LOGIC;
		 src5_ready	:	IN  STD_LOGIC;
		 src5_startofpacket	:	OUT  STD_LOGIC;
		 src5_valid	:	OUT  STD_LOGIC;
		 src6_channel	:	OUT  STD_LOGIC_VECTOR (18 DOWNTO 0);
		 src6_data	:	OUT  STD_LOGIC_VECTOR (130 DOWNTO 0);
		 src6_endofpacket	:	OUT  STD_LOGIC;
		 src6_ready	:	IN  STD_LOGIC;
		 src6_startofpacket	:	OUT  STD_LOGIC;
		 src6_valid	:	OUT  STD_LOGIC;
		 src7_channel	:	OUT  STD_LOGIC_VECTOR (18 DOWNTO 0);
		 src7_data	:	OUT  STD_LOGIC_VECTOR (130 DOWNTO 0);
		 src7_endofpacket	:	OUT  STD_LOGIC;
		 src7_ready	:	IN  STD_LOGIC;
		 src7_startofpacket	:	OUT  STD_LOGIC;
		 src7_valid	:	OUT  STD_LOGIC;
		 src8_channel	:	OUT  STD_LOGIC_VECTOR (18 DOWNTO 0);
		 src8_data	:	OUT  STD_LOGIC_VECTOR (130 DOWNTO 0);
		 src8_endofpacket	:	OUT  STD_LOGIC;
		 src8_ready	:	IN  STD_LOGIC;
		 src8_startofpacket	:	OUT  STD_LOGIC;
		 src8_valid	:	OUT  STD_LOGIC;
		 src9_channel	:	OUT  STD_LOGIC_VECTOR (18 DOWNTO 0);
		 src9_data	:	OUT  STD_LOGIC_VECTOR (130 DOWNTO 0);
		 src9_endofpacket	:	OUT  STD_LOGIC;
		 src9_ready	:	IN  STD_LOGIC;
		 src9_startofpacket	:	OUT  STD_LOGIC;
		 src9_valid	:	OUT  STD_LOGIC
	 ); 
 END fluid_board_soc_mm_interconnect_0_cmd_demux_003;

 ARCHITECTURE RTL OF fluid_board_soc_mm_interconnect_0_cmd_demux_003 IS

	 ATTRIBUTE synthesis_clearbox : natural;
	 ATTRIBUTE synthesis_clearbox OF RTL : ARCHITECTURE IS 1;
	 SIGNAL  wire_w_lg_w_sink_channel_range1w2w	:	STD_LOGIC_VECTOR (0 DOWNTO 0);
	 SIGNAL  wire_w_lg_w_sink_channel_range30w31w	:	STD_LOGIC_VECTOR (0 DOWNTO 0);
	 SIGNAL  wire_w_lg_w_sink_channel_range33w34w	:	STD_LOGIC_VECTOR (0 DOWNTO 0);
	 SIGNAL  wire_w_lg_w_sink_channel_range36w37w	:	STD_LOGIC_VECTOR (0 DOWNTO 0);
	 SIGNAL  wire_w_lg_w_sink_channel_range39w40w	:	STD_LOGIC_VECTOR (0 DOWNTO 0);
	 SIGNAL  wire_w_lg_w_sink_channel_range42w43w	:	STD_LOGIC_VECTOR (0 DOWNTO 0);
	 SIGNAL  wire_w_lg_w_sink_channel_range45w46w	:	STD_LOGIC_VECTOR (0 DOWNTO 0);
	 SIGNAL  wire_w_lg_w_sink_channel_range48w49w	:	STD_LOGIC_VECTOR (0 DOWNTO 0);
	 SIGNAL  wire_w_lg_w_sink_channel_range51w52w	:	STD_LOGIC_VECTOR (0 DOWNTO 0);
	 SIGNAL  wire_w_lg_w_sink_channel_range3w4w	:	STD_LOGIC_VECTOR (0 DOWNTO 0);
	 SIGNAL  wire_w_lg_w_sink_channel_range6w7w	:	STD_LOGIC_VECTOR (0 DOWNTO 0);
	 SIGNAL  wire_w_lg_w_sink_channel_range9w10w	:	STD_LOGIC_VECTOR (0 DOWNTO 0);
	 SIGNAL  wire_w_lg_w_sink_channel_range12w13w	:	STD_LOGIC_VECTOR (0 DOWNTO 0);
	 SIGNAL  wire_w_lg_w_sink_channel_range15w16w	:	STD_LOGIC_VECTOR (0 DOWNTO 0);
	 SIGNAL  wire_w_lg_w_sink_channel_range18w19w	:	STD_LOGIC_VECTOR (0 DOWNTO 0);
	 SIGNAL  wire_w_lg_w_sink_channel_range21w22w	:	STD_LOGIC_VECTOR (0 DOWNTO 0);
	 SIGNAL  wire_w_lg_w_sink_channel_range24w25w	:	STD_LOGIC_VECTOR (0 DOWNTO 0);
	 SIGNAL  wire_w_lg_w_sink_channel_range27w28w	:	STD_LOGIC_VECTOR (0 DOWNTO 0);
	 SIGNAL  s_wire_fluid_board_soc_mm_interconnect_0_cmd_demux_003_wideor0_37_dataout :	STD_LOGIC;
	 SIGNAL  wire_w_sink_channel_range1w	:	STD_LOGIC_VECTOR (0 DOWNTO 0);
	 SIGNAL  wire_w_sink_channel_range30w	:	STD_LOGIC_VECTOR (0 DOWNTO 0);
	 SIGNAL  wire_w_sink_channel_range33w	:	STD_LOGIC_VECTOR (0 DOWNTO 0);
	 SIGNAL  wire_w_sink_channel_range36w	:	STD_LOGIC_VECTOR (0 DOWNTO 0);
	 SIGNAL  wire_w_sink_channel_range39w	:	STD_LOGIC_VECTOR (0 DOWNTO 0);
	 SIGNAL  wire_w_sink_channel_range42w	:	STD_LOGIC_VECTOR (0 DOWNTO 0);
	 SIGNAL  wire_w_sink_channel_range45w	:	STD_LOGIC_VECTOR (0 DOWNTO 0);
	 SIGNAL  wire_w_sink_channel_range48w	:	STD_LOGIC_VECTOR (0 DOWNTO 0);
	 SIGNAL  wire_w_sink_channel_range51w	:	STD_LOGIC_VECTOR (0 DOWNTO 0);
	 SIGNAL  wire_w_sink_channel_range3w	:	STD_LOGIC_VECTOR (0 DOWNTO 0);
	 SIGNAL  wire_w_sink_channel_range6w	:	STD_LOGIC_VECTOR (0 DOWNTO 0);
	 SIGNAL  wire_w_sink_channel_range9w	:	STD_LOGIC_VECTOR (0 DOWNTO 0);
	 SIGNAL  wire_w_sink_channel_range12w	:	STD_LOGIC_VECTOR (0 DOWNTO 0);
	 SIGNAL  wire_w_sink_channel_range15w	:	STD_LOGIC_VECTOR (0 DOWNTO 0);
	 SIGNAL  wire_w_sink_channel_range18w	:	STD_LOGIC_VECTOR (0 DOWNTO 0);
	 SIGNAL  wire_w_sink_channel_range21w	:	STD_LOGIC_VECTOR (0 DOWNTO 0);
	 SIGNAL  wire_w_sink_channel_range24w	:	STD_LOGIC_VECTOR (0 DOWNTO 0);
	 SIGNAL  wire_w_sink_channel_range27w	:	STD_LOGIC_VECTOR (0 DOWNTO 0);
 BEGIN

	wire_w_lg_w_sink_channel_range1w2w(0) <= wire_w_sink_channel_range1w(0) AND src0_ready;
	wire_w_lg_w_sink_channel_range30w31w(0) <= wire_w_sink_channel_range30w(0) AND src10_ready;
	wire_w_lg_w_sink_channel_range33w34w(0) <= wire_w_sink_channel_range33w(0) AND src11_ready;
	wire_w_lg_w_sink_channel_range36w37w(0) <= wire_w_sink_channel_range36w(0) AND src12_ready;
	wire_w_lg_w_sink_channel_range39w40w(0) <= wire_w_sink_channel_range39w(0) AND src13_ready;
	wire_w_lg_w_sink_channel_range42w43w(0) <= wire_w_sink_channel_range42w(0) AND src14_ready;
	wire_w_lg_w_sink_channel_range45w46w(0) <= wire_w_sink_channel_range45w(0) AND src15_ready;
	wire_w_lg_w_sink_channel_range48w49w(0) <= wire_w_sink_channel_range48w(0) AND src16_ready;
	wire_w_lg_w_sink_channel_range51w52w(0) <= wire_w_sink_channel_range51w(0) AND src17_ready;
	wire_w_lg_w_sink_channel_range3w4w(0) <= wire_w_sink_channel_range3w(0) AND src1_ready;
	wire_w_lg_w_sink_channel_range6w7w(0) <= wire_w_sink_channel_range6w(0) AND src2_ready;
	wire_w_lg_w_sink_channel_range9w10w(0) <= wire_w_sink_channel_range9w(0) AND src3_ready;
	wire_w_lg_w_sink_channel_range12w13w(0) <= wire_w_sink_channel_range12w(0) AND src4_ready;
	wire_w_lg_w_sink_channel_range15w16w(0) <= wire_w_sink_channel_range15w(0) AND src5_ready;
	wire_w_lg_w_sink_channel_range18w19w(0) <= wire_w_sink_channel_range18w(0) AND src6_ready;
	wire_w_lg_w_sink_channel_range21w22w(0) <= wire_w_sink_channel_range21w(0) AND src7_ready;
	wire_w_lg_w_sink_channel_range24w25w(0) <= wire_w_sink_channel_range24w(0) AND src8_ready;
	wire_w_lg_w_sink_channel_range27w28w(0) <= wire_w_sink_channel_range27w(0) AND src9_ready;
	s_wire_fluid_board_soc_mm_interconnect_0_cmd_demux_003_wideor0_37_dataout <= (((((((((((((((((wire_w_lg_w_sink_channel_range1w2w(0) OR wire_w_lg_w_sink_channel_range3w4w(0)) OR wire_w_lg_w_sink_channel_range6w7w(0)) OR wire_w_lg_w_sink_channel_range9w10w(0)) OR wire_w_lg_w_sink_channel_range12w13w(0)) OR wire_w_lg_w_sink_channel_range15w16w(0)) OR wire_w_lg_w_sink_channel_range18w19w(0)) OR wire_w_lg_w_sink_channel_range21w22w(0)) OR wire_w_lg_w_sink_channel_range24w25w(0)) OR wire_w_lg_w_sink_channel_range27w28w(0)) OR wire_w_lg_w_sink_channel_range30w31w(0)) OR wire_w_lg_w_sink_channel_range33w34w(0)) OR wire_w_lg_w_sink_channel_range36w37w(0)) OR wire_w_lg_w_sink_channel_range39w40w(0)) OR wire_w_lg_w_sink_channel_range42w43w(0)) OR wire_w_lg_w_sink_channel_range45w46w(0)) OR wire_w_lg_w_sink_channel_range48w49w(0)) OR wire_w_lg_w_sink_channel_range51w52w(0));
	sink_ready <= s_wire_fluid_board_soc_mm_interconnect_0_cmd_demux_003_wideor0_37_dataout;
	src0_channel <= ( "0" & "0" & "0" & "0" & "0" & "0" & "0" & "0" & "0" & "0" & "0" & "0" & "0" & "0" & "0" & "0" & "0" & "0" & sink_channel(18));
	src0_data <= ( sink_data(130 DOWNTO 0));
	src0_endofpacket <= sink_endofpacket;
	src0_startofpacket <= sink_startofpacket;
	src0_valid <= (sink_valid(0) AND sink_channel(0));
	src10_channel <= ( "0" & "0" & "0" & "0" & "0" & "0" & "0" & "0" & "0" & "0" & "0" & "0" & "0" & "0" & "0" & "0" & "0" & "0" & sink_channel(18));
	src10_data <= ( sink_data(130 DOWNTO 0));
	src10_endofpacket <= sink_endofpacket;
	src10_startofpacket <= sink_startofpacket;
	src10_valid <= (sink_valid(0) AND sink_channel(10));
	src11_channel <= ( "0" & "0" & "0" & "0" & "0" & "0" & "0" & "0" & "0" & "0" & "0" & "0" & "0" & "0" & "0" & "0" & "0" & "0" & sink_channel(18));
	src11_data <= ( sink_data(130 DOWNTO 0));
	src11_endofpacket <= sink_endofpacket;
	src11_startofpacket <= sink_startofpacket;
	src11_valid <= (sink_valid(0) AND sink_channel(11));
	src12_channel <= ( "0" & "0" & "0" & "0" & "0" & "0" & "0" & "0" & "0" & "0" & "0" & "0" & "0" & "0" & "0" & "0" & "0" & "0" & sink_channel(18));
	src12_data <= ( sink_data(130 DOWNTO 0));
	src12_endofpacket <= sink_endofpacket;
	src12_startofpacket <= sink_startofpacket;
	src12_valid <= (sink_valid(0) AND sink_channel(12));
	src13_channel <= ( "0" & "0" & "0" & "0" & "0" & "0" & "0" & "0" & "0" & "0" & "0" & "0" & "0" & "0" & "0" & "0" & "0" & "0" & sink_channel(18));
	src13_data <= ( sink_data(130 DOWNTO 0));
	src13_endofpacket <= sink_endofpacket;
	src13_startofpacket <= sink_startofpacket;
	src13_valid <= (sink_valid(0) AND sink_channel(13));
	src14_channel <= ( "0" & "0" & "0" & "0" & "0" & "0" & "0" & "0" & "0" & "0" & "0" & "0" & "0" & "0" & "0" & "0" & "0" & "0" & sink_channel(18));
	src14_data <= ( sink_data(130 DOWNTO 0));
	src14_endofpacket <= sink_endofpacket;
	src14_startofpacket <= sink_startofpacket;
	src14_valid <= (sink_valid(0) AND sink_channel(14));
	src15_channel <= ( "0" & "0" & "0" & "0" & "0" & "0" & "0" & "0" & "0" & "0" & "0" & "0" & "0" & "0" & "0" & "0" & "0" & "0" & sink_channel(18));
	src15_data <= ( sink_data(130 DOWNTO 0));
	src15_endofpacket <= sink_endofpacket;
	src15_startofpacket <= sink_startofpacket;
	src15_valid <= (sink_valid(0) AND sink_channel(15));
	src16_channel <= ( "0" & "0" & "0" & "0" & "0" & "0" & "0" & "0" & "0" & "0" & "0" & "0" & "0" & "0" & "0" & "0" & "0" & "0" & sink_channel(18));
	src16_data <= ( sink_data(130 DOWNTO 0));
	src16_endofpacket <= sink_endofpacket;
	src16_startofpacket <= sink_startofpacket;
	src16_valid <= (sink_valid(0) AND sink_channel(16));
	src17_channel <= ( "0" & "0" & "0" & "0" & "0" & "0" & "0" & "0" & "0" & "0" & "0" & "0" & "0" & "0" & "0" & "0" & "0" & "0" & sink_channel(18));
	src17_data <= ( sink_data(130 DOWNTO 0));
	src17_endofpacket <= sink_endofpacket;
	src17_startofpacket <= sink_startofpacket;
	src17_valid <= (sink_valid(0) AND sink_channel(17));
	src1_channel <= ( "0" & "0" & "0" & "0" & "0" & "0" & "0" & "0" & "0" & "0" & "0" & "0" & "0" & "0" & "0" & "0" & "0" & "0" & sink_channel(18));
	src1_data <= ( sink_data(130 DOWNTO 0));
	src1_endofpacket <= sink_endofpacket;
	src1_startofpacket <= sink_startofpacket;
	src1_valid <= (sink_valid(0) AND sink_channel(1));
	src2_channel <= ( "0" & "0" & "0" & "0" & "0" & "0" & "0" & "0" & "0" & "0" & "0" & "0" & "0" & "0" & "0" & "0" & "0" & "0" & sink_channel(18));
	src2_data <= ( sink_data(130 DOWNTO 0));
	src2_endofpacket <= sink_endofpacket;
	src2_startofpacket <= sink_startofpacket;
	src2_valid <= (sink_valid(0) AND sink_channel(2));
	src3_channel <= ( "0" & "0" & "0" & "0" & "0" & "0" & "0" & "0" & "0" & "0" & "0" & "0" & "0" & "0" & "0" & "0" & "0" & "0" & sink_channel(18));
	src3_data <= ( sink_data(130 DOWNTO 0));
	src3_endofpacket <= sink_endofpacket;
	src3_startofpacket <= sink_startofpacket;
	src3_valid <= (sink_valid(0) AND sink_channel(3));
	src4_channel <= ( "0" & "0" & "0" & "0" & "0" & "0" & "0" & "0" & "0" & "0" & "0" & "0" & "0" & "0" & "0" & "0" & "0" & "0" & sink_channel(18));
	src4_data <= ( sink_data(130 DOWNTO 0));
	src4_endofpacket <= sink_endofpacket;
	src4_startofpacket <= sink_startofpacket;
	src4_valid <= (sink_valid(0) AND sink_channel(4));
	src5_channel <= ( "0" & "0" & "0" & "0" & "0" & "0" & "0" & "0" & "0" & "0" & "0" & "0" & "0" & "0" & "0" & "0" & "0" & "0" & sink_channel(18));
	src5_data <= ( sink_data(130 DOWNTO 0));
	src5_endofpacket <= sink_endofpacket;
	src5_startofpacket <= sink_startofpacket;
	src5_valid <= (sink_valid(0) AND sink_channel(5));
	src6_channel <= ( "0" & "0" & "0" & "0" & "0" & "0" & "0" & "0" & "0" & "0" & "0" & "0" & "0" & "0" & "0" & "0" & "0" & "0" & sink_channel(18));
	src6_data <= ( sink_data(130 DOWNTO 0));
	src6_endofpacket <= sink_endofpacket;
	src6_startofpacket <= sink_startofpacket;
	src6_valid <= (sink_valid(0) AND sink_channel(6));
	src7_channel <= ( "0" & "0" & "0" & "0" & "0" & "0" & "0" & "0" & "0" & "0" & "0" & "0" & "0" & "0" & "0" & "0" & "0" & "0" & sink_channel(18));
	src7_data <= ( sink_data(130 DOWNTO 0));
	src7_endofpacket <= sink_endofpacket;
	src7_startofpacket <= sink_startofpacket;
	src7_valid <= (sink_valid(0) AND sink_channel(7));
	src8_channel <= ( "0" & "0" & "0" & "0" & "0" & "0" & "0" & "0" & "0" & "0" & "0" & "0" & "0" & "0" & "0" & "0" & "0" & "0" & sink_channel(18));
	src8_data <= ( sink_data(130 DOWNTO 0));
	src8_endofpacket <= sink_endofpacket;
	src8_startofpacket <= sink_startofpacket;
	src8_valid <= (sink_valid(0) AND sink_channel(8));
	src9_channel <= ( "0" & "0" & "0" & "0" & "0" & "0" & "0" & "0" & "0" & "0" & "0" & "0" & "0" & "0" & "0" & "0" & "0" & "0" & sink_channel(18));
	src9_data <= ( sink_data(130 DOWNTO 0));
	src9_endofpacket <= sink_endofpacket;
	src9_startofpacket <= sink_startofpacket;
	src9_valid <= (sink_valid(0) AND sink_channel(9));
	wire_w_sink_channel_range1w(0) <= sink_channel(0);
	wire_w_sink_channel_range30w(0) <= sink_channel(10);
	wire_w_sink_channel_range33w(0) <= sink_channel(11);
	wire_w_sink_channel_range36w(0) <= sink_channel(12);
	wire_w_sink_channel_range39w(0) <= sink_channel(13);
	wire_w_sink_channel_range42w(0) <= sink_channel(14);
	wire_w_sink_channel_range45w(0) <= sink_channel(15);
	wire_w_sink_channel_range48w(0) <= sink_channel(16);
	wire_w_sink_channel_range51w(0) <= sink_channel(17);
	wire_w_sink_channel_range3w(0) <= sink_channel(1);
	wire_w_sink_channel_range6w(0) <= sink_channel(2);
	wire_w_sink_channel_range9w(0) <= sink_channel(3);
	wire_w_sink_channel_range12w(0) <= sink_channel(4);
	wire_w_sink_channel_range15w(0) <= sink_channel(5);
	wire_w_sink_channel_range18w(0) <= sink_channel(6);
	wire_w_sink_channel_range21w(0) <= sink_channel(7);
	wire_w_sink_channel_range24w(0) <= sink_channel(8);
	wire_w_sink_channel_range27w(0) <= sink_channel(9);

 END RTL; --fluid_board_soc_mm_interconnect_0_cmd_demux_003
--synopsys translate_on
--VALID FILE
