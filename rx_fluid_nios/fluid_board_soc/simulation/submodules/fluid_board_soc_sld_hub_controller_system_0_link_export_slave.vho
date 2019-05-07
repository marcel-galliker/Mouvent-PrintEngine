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

 ENTITY  fluid_board_soc_sld_hub_controller_system_0_link_export_slave IS 
	 PORT 
	 ( 
		 clk	:	IN  STD_LOGIC;
		 m0_address	:	OUT  STD_LOGIC_VECTOR (6 DOWNTO 0);
		 m0_burstcount	:	OUT  STD_LOGIC_VECTOR (0 DOWNTO 0);
		 m0_byteenable	:	OUT  STD_LOGIC_VECTOR (3 DOWNTO 0);
		 m0_debugaccess	:	OUT  STD_LOGIC;
		 m0_read	:	OUT  STD_LOGIC;
		 m0_readdata	:	IN  STD_LOGIC_VECTOR (31 DOWNTO 0);
		 m0_readdatavalid	:	IN  STD_LOGIC;
		 m0_waitrequest	:	IN  STD_LOGIC;
		 m0_write	:	OUT  STD_LOGIC;
		 m0_writedata	:	OUT  STD_LOGIC_VECTOR (31 DOWNTO 0);
		 reset	:	IN  STD_LOGIC;
		 s0_address	:	IN  STD_LOGIC_VECTOR (6 DOWNTO 0);
		 s0_burstcount	:	IN  STD_LOGIC_VECTOR (0 DOWNTO 0);
		 s0_byteenable	:	IN  STD_LOGIC_VECTOR (3 DOWNTO 0);
		 s0_debugaccess	:	IN  STD_LOGIC;
		 s0_read	:	IN  STD_LOGIC;
		 s0_readdata	:	OUT  STD_LOGIC_VECTOR (31 DOWNTO 0);
		 s0_readdatavalid	:	OUT  STD_LOGIC;
		 s0_waitrequest	:	OUT  STD_LOGIC;
		 s0_write	:	IN  STD_LOGIC;
		 s0_writedata	:	IN  STD_LOGIC_VECTOR (31 DOWNTO 0)
	 ); 
 END fluid_board_soc_sld_hub_controller_system_0_link_export_slave;

 ARCHITECTURE RTL OF fluid_board_soc_sld_hub_controller_system_0_link_export_slave IS

	 ATTRIBUTE synthesis_clearbox : natural;
	 ATTRIBUTE synthesis_clearbox OF RTL : ARCHITECTURE IS 1;
 BEGIN

	m0_address <= ( s0_address(6 DOWNTO 0));
	m0_burstcount(0) <= ( s0_burstcount(0));
	m0_byteenable <= ( s0_byteenable(3 DOWNTO 0));
	m0_debugaccess <= s0_debugaccess;
	m0_read <= s0_read;
	m0_write <= s0_write;
	m0_writedata <= ( s0_writedata(31 DOWNTO 0));
	s0_readdata <= ( m0_readdata(31 DOWNTO 0));
	s0_readdatavalid <= m0_readdatavalid;
	s0_waitrequest <= m0_waitrequest;

 END RTL; --fluid_board_soc_sld_hub_controller_system_0_link_export_slave
--synopsys translate_on
--VALID FILE
