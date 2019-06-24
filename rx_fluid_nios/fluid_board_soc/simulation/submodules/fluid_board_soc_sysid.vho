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

 ENTITY  fluid_board_soc_sysid IS 
	 PORT 
	 ( 
		 address	:	IN  STD_LOGIC;
		 clock	:	IN  STD_LOGIC;
		 readdata	:	OUT  STD_LOGIC_VECTOR (31 DOWNTO 0);
		 reset_n	:	IN  STD_LOGIC
	 ); 
 END fluid_board_soc_sysid;

 ARCHITECTURE RTL OF fluid_board_soc_sysid IS

	 ATTRIBUTE synthesis_clearbox : natural;
	 ATTRIBUTE synthesis_clearbox OF RTL : ARCHITECTURE IS 1;
	 SIGNAL  wire_w_lg_address6w	:	STD_LOGIC_VECTOR (0 DOWNTO 0);
 BEGIN

	wire_w_lg_address6w(0) <= NOT address;
	readdata <= ( "0" & address & "0" & address & "0" & address & address & "0" & "0" & address & address & address & "1" & "0" & "1" & "0" & "1" & "0" & wire_w_lg_address6w & "1" & wire_w_lg_address6w & "1" & "0" & address & "1" & "1" & "0" & "1" & wire_w_lg_address6w & wire_w_lg_address6w & "1" & "0");

 END RTL; --fluid_board_soc_sysid
--synopsys translate_on
--VALID FILE
