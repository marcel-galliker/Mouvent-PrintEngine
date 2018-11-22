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

 ENTITY  fluid_board_soc_irq_mapper_002 IS 
	 PORT 
	 ( 
		 clk	:	IN  STD_LOGIC;
		 receiver0_irq	:	IN  STD_LOGIC;
		 receiver10_irq	:	IN  STD_LOGIC;
		 receiver1_irq	:	IN  STD_LOGIC;
		 receiver2_irq	:	IN  STD_LOGIC;
		 receiver3_irq	:	IN  STD_LOGIC;
		 receiver4_irq	:	IN  STD_LOGIC;
		 receiver5_irq	:	IN  STD_LOGIC;
		 receiver6_irq	:	IN  STD_LOGIC;
		 receiver7_irq	:	IN  STD_LOGIC;
		 receiver8_irq	:	IN  STD_LOGIC;
		 receiver9_irq	:	IN  STD_LOGIC;
		 reset	:	IN  STD_LOGIC;
		 sender_irq	:	OUT  STD_LOGIC_VECTOR (31 DOWNTO 0)
	 ); 
 END fluid_board_soc_irq_mapper_002;

 ARCHITECTURE RTL OF fluid_board_soc_irq_mapper_002 IS

	 ATTRIBUTE synthesis_clearbox : natural;
	 ATTRIBUTE synthesis_clearbox OF RTL : ARCHITECTURE IS 1;
 BEGIN

	sender_irq <= ( "0" & "0" & "0" & "0" & "0" & "0" & "0" & "0" & "0" & "0" & "0" & "0" & "0" & "0" & "0" & "0" & "0" & "0" & "0" & "0" & "0" & receiver10_irq & receiver9_irq & receiver8_irq & receiver7_irq & receiver6_irq & receiver5_irq & receiver4_irq & receiver3_irq & receiver2_irq & receiver1_irq & receiver0_irq);

 END RTL; --fluid_board_soc_irq_mapper_002
--synopsys translate_on
--VALID FILE
