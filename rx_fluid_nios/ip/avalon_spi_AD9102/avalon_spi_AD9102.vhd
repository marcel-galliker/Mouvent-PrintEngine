-- ###########################################################################
--
-- name:			Peter Brandenberger
--
-- file:			avalon_spi_AD9102.vhd
--
-- function:	ad9102 spi communication
--  
-- date:			rev 1.0 2013
--
-- ###########################################################################

library ieee;
use ieee.std_logic_1164.all;
use ieee.std_logic_unsigned.all;
use ieee.std_logic_arith.all;

entity avalon_spi_AD9102 is
port (
	csi_clock_clk 		: in std_logic;
	csi_clock_reset_n : in std_logic;
	
	avs_s1_writedata 	: in std_logic_vector (15 downto 0);
	avs_s1_read 		: in std_logic;
	avs_s1_write 		: in std_logic;
	avs_s1_address 	: in std_logic_vector(14 downto 0); 	-- word
	avs_s1_waitrequest: out std_logic;
	avs_s1_readdata 	: out std_logic_vector (15 downto 0);
	
	coe_s1_sclk 		: out std_logic;
	coe_s1_cs_n 		: out std_logic;
	coe_s1_sdio 		: out std_logic;
	coe_s1_sdo 			: in std_logic
	);
end avalon_spi_AD9102;

architecture pwm_multi_a of avalon_spi_AD9102 is

signal bcnt : std_logic_vector (8 downto 0); -- 16.04.2015 80 MHz -> 160 MHz
signal rd_done,wr_done,rd_done_q,rd_pend,wr_pend,wr_done_q : std_logic;
signal oe,cs,sdio,sclk,shift	: std_logic;

signal readdata_s	: std_logic_vector (15 downto 0);
signal shiftin		: std_logic_vector (15 downto 0);
signal shiftout	: std_logic_vector (31 downto 0);

begin

avs_s1_readdata		<= readdata_s;
avs_s1_waitrequest	<= '1' when(avs_s1_read  = '1')and(rd_done_q = '0') 
				       else '1' when(avs_s1_write = '1')and(wr_done_q = '0')
						 else '0';

-- avalon read 
read_avalon : process(csi_clock_clk, csi_clock_reset_n)
begin
	if csi_clock_reset_n = '0' then
		readdata_s	<= (others => '0');
		rd_done_q	<= '0';
		wr_done_q	<= '0';
		shiftin		<= (others => '0');

	elsif csi_clock_clk'event and csi_clock_clk = '1' then
		
		if (rd_done = '1') then
			readdata_s	<= shiftin;
		end if;
		rd_done_q 	<= rd_done;
		wr_done_q 	<= wr_done;
		
		if (shift = '1') then
			shiftin	<= shiftin(14 downto 0) & coe_s1_sdo;
		end if;

	end if;
end process;

-- avalon write
	
write_avalon : process (csi_clock_clk, csi_clock_reset_n)
begin
	if csi_clock_reset_n = '0' then
		shiftout  	<= (others => '0');
		rd_pend		<= '0';
		wr_pend		<= '0';

	elsif csi_clock_clk'event and csi_clock_clk = '1' then
	
		if (shift = '1') then
			shiftout	<= shiftout(30 downto 0) & '0';
			
		elsif (avs_s1_write = '1')and(wr_pend = '0')and(rd_done_q = '0')and(wr_done_q = '0') then
			shiftout	<= '0' & avs_s1_address & avs_s1_writedata;
			
		elsif (avs_s1_read = '1')and(rd_pend = '0')and(rd_done_q = '0')and(wr_done_q = '0') then
			shiftout	<= '1' & avs_s1_address & X"FFFF";
		end if;
		
		if (rd_done = '1') then
			rd_pend	<= '0';
			
		elsif (wr_done = '1') then
			wr_pend	<= '0';
			
		elsif (avs_s1_write = '1')and(wr_pend = '0')and(rd_done_q = '0')and(wr_done_q = '0') then
			wr_pend	<= '1';
		
		elsif (avs_s1_read = '1')and(rd_pend = '0')and(rd_done_q = '0')and(wr_done_q = '0') then
			rd_pend	<= '1';
			
		end if;
		
	end if;
end process;

coe_s1_sclk	<= sclk;
coe_s1_cs_n	<= not cs;
coe_s1_sdio	<= sdio when (oe = '1') else 'Z';

-- Timing

timing: process (csi_clock_clk, csi_clock_reset_n)
begin
	if csi_clock_reset_n = '0' then
		bcnt 	<= (others => '1');
		sclk		<= '0';
		cs			<= '0';
		sdio		<= '0';
		oe			<= '1';
		rd_done	<= '0';
		wr_done	<= '0';
		shift		<= '0';
		
	elsif csi_clock_clk'event and csi_clock_clk = '1' then
	
		if (bcnt(8) = '1') then    -- 16.04.2015 80 MHz -> 160 MHz
			if (wr_pend = '1')or(rd_pend = '1') then
				if (wr_done = '0')and(rd_done = '0') then
					bcnt	<= conv_std_logic_vector(128, 9);
				end if;
			end if;
			shift		<= '0';
		else
			bcnt	<= bcnt - 1;
			shift	<= (not bcnt(0)) and bcnt(1);
		end if;
		
		sclk	<= bcnt(1);
		
		if (bcnt /= 0) then
			cs		<= not bcnt(8); -- 16.04.2015 80 MHz -> 160 MHz
		else
			cs	<= '0';
		end if;
		
		if (bcnt > 0)and(bcnt < 65) then	-- 16.04.2015 80 MHz -> 160 MHz
			oe	<= not rd_pend;
		else
			oe	<= '1';
		end if;
		
		sdio	<= shiftout(31);
		
		if (bcnt = 0) then
			rd_done	<= rd_pend;
		else
			rd_done	<= '0';
		end if;
		
		if (bcnt = 0) then
			wr_done	<= wr_pend;
		else
			wr_done	<= '0';
		end if;

	end if;
end process;
end pwm_multi_a;
