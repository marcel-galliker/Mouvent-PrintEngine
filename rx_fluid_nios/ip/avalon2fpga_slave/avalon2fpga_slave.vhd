-----------------------------------------------------
-- Project : Fluid Board
-----------------------------------------------------
-- File    : avalon2fpga_slave.vhd
-- Library : fluidr_board_lib
--- Author  : sebastian@radex-net.com
--- Company : Radex AG
--- Copyright(C) Radex AG
------------------------------------------------------
--- Last commit:
---   $Author:: sebastian@radex-net.com                 $
---      $Rev:: 0                                    $
---     $Date:: 2016-09-14 #$
-----------------------------------------------------
-- Description : Connect Avalon MM Slave to FPGA
-----------------------------------------------------

LIBRARY ieee;
USE ieee.std_logic_1164.ALL;
USE ieee.numeric_std.ALL;

entity avalon2fpga_slave is
  generic(
    g_address_width : integer := 16;
    g_data_width : integer := 16
  );
  port(
    csi_clock_clk      : in  std_logic;
    rsi_reset_reset_n  : in  std_logic;
    -- ins_s0_interrupt   : out std_logic;
    avs_s1_writedata   : in  std_logic_vector(g_data_width-1 downto 0);
    avs_s1_read        : in  std_logic;
    avs_s1_write       : in  std_logic;
    avs_s1_address     : in  std_logic_vector(g_address_width -1 downto 0);
    avs_s1_waitrequest : out std_logic;
    avs_s1_readdata    : out std_logic_vector(g_data_width-1 downto 0);

    avm_s2_writedata   : out  std_logic_vector(g_data_width-1 downto 0);
    avm_s2_read        : out  std_logic;
    avm_s2_write       : out  std_logic;
    avm_s2_address     : out  std_logic_vector(g_address_width -1 downto 0);
    avm_s2_waitrequest : in std_logic;
    avm_s2_readdata    : in std_logic_vector(g_data_width-1 downto 0)
    -- coe_s2_interrupt   : in  std_logic
  );
end avalon2fpga_slave;

architecture rtl of avalon2fpga_slave is

begin
  ----------------------------------------------------------------
  -- Concurrend
  ---------------------------------------------------------------- 
    avm_s2_writedata   <= avs_s1_writedata;
    avm_s2_read        <= avs_s1_read;
    avm_s2_write       <= avs_s1_write;
    avm_s2_address     <= avs_s1_address;
    avs_s1_waitrequest <= avm_s2_waitrequest;
    avs_s1_readdata    <= avm_s2_readdata;

END rtl;
