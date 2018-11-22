-----------------------------------------------------
-- Project : Fluid Board
-----------------------------------------------------
-- File    : avalon_spi_MAX31865_rtl.vhd
-- Library : fluid_board_lib
--- Author  : sebastian@radex-net.com
--- Company : Radex AG
--- Copyright(C) Radex AG
------------------------------------------------------
--- Last commit:
---   $Author:: sebastian@radex-net.com                 $
---      $Rev:: 0                                    $
---     $Date:: 2015-12-15 #$
-----------------------------------------------------
-- Description : Connect Avalon MM Slave to SPI Master for MAX31865 interfacing PT100 elements
-----------------------------------------------------

LIBRARY ieee;
USE ieee.std_logic_1164.ALL;
USE ieee.numeric_std.ALL;

entity avalon_spi_max31865 is
  generic(
    g_cpol : std_ulogic := '0'          -- clock polarity
  );
  port(
    csi_clock_clk      : in  std_logic;
    rsi_reset_reset_n  : in  std_logic;

    avs_s1_writedata   : in  std_logic_vector(7 downto 0);
    avs_s1_read        : in  std_logic;
    avs_s1_write       : in  std_logic;
    avs_s1_address     : in  std_logic_vector(3 downto 0); -- word
    avs_s1_waitrequest : out std_logic;
    avs_s1_readdata    : out std_logic_vector(7 downto 0);

    coe_s2_drdy_n      : in  std_logic;
    coe_s2_sclk        : out std_logic;
    coe_s2_cs_n        : out std_logic;
    coe_s2_sdi         : out std_logic;
    coe_s2_sdo         : in  std_logic
  );
end avalon_spi_max31865;

architecture rtl of avalon_spi_max31865 is
  constant c_sclk_cnt      : integer := 10;
  constant c_half_sclk_cnt : integer := 5;
  constant c_inactive_cnt  : integer := 20;
  constant c_setup_cnt     : integer := 20;
  constant c_bit_cnt       : integer := 18;
  constant c_break_bit     : integer := 9; -- bit 0 is always a break bit

  signal i_sclk_cnt     : unsigned(4 - 1 downto 0);
  signal i_inactive_cnt : unsigned(5 - 1 downto 0);
  signal i_setup_cnt    : unsigned(5 - 1 downto 0);
  signal i_bit_cnt      : unsigned(5 - 1 downto 0);
  signal i_shiftout     : std_ulogic_vector(8 + 7 - 1 downto 0);
  signal i_shiftin      : std_ulogic_vector(8 - 1 downto 0);
  signal i_addr         : std_ulogic_vector(7 - 1 downto 0);
  signal i_rd_enable    : std_ulogic;
  signal i_wr_enable    : std_ulogic;
  signal i_rd_dis       : std_ulogic;
  signal i_wr_dis       : std_ulogic;
  signal i_rd_ready     : std_ulogic;
  signal i_wr_ready     : std_ulogic;
  signal i_sclk_en      : std_ulogic;
  signal i_sclk_dis     : std_ulogic;

begin

  ----------------------------------------------------------------
  -- Generate 5MHz enable
  ---------------------------------------------------------------- 
  p_clk_gen : PROCESS(csi_clock_clk, rsi_reset_reset_n)
  BEGIN
    IF rsi_reset_reset_n = '0' THEN
      i_sclk_en  <= '0';
      i_sclk_dis <= '0';
      i_sclk_cnt <= to_unsigned(c_half_sclk_cnt - 1, i_sclk_cnt'length);
    ELSIF rising_edge(csi_clock_clk) THEN
      i_sclk_en  <= '0';                -- Pulse
      i_sclk_dis <= '0';                -- Pulse

      -- ============ ============ ===================== 
      IF i_sclk_cnt = 0 THEN            -- Pulse clk rising edge
        -- ------------ ------------ ---------------------
        IF i_wr_enable = '1' OR i_rd_enable = '1' THEN
          i_sclk_en  <= '1';
          i_sclk_cnt <= to_unsigned(c_sclk_cnt - 1, i_sclk_cnt'length);
        END IF;
      -- ------------ ------------ ---------------------
      ELSIF i_sclk_cnt = c_half_sclk_cnt THEN -- Pulse clk falling edge
        -- ------------ ------------ ---------------------
        i_sclk_dis <= '1';
        i_sclk_cnt <= i_sclk_cnt - 1;
      -- ------------ ------------ ---------------------
      ELSE
        -- ------------ ------------ ---------------------
        i_sclk_cnt <= i_sclk_cnt - 1;
      -- ------------ ------------ ---------------------
      END IF;

    END IF;
  END PROCESS p_clk_gen;

  ----------------------------------------------------------------
  -- Concurrend
  ---------------------------------------------------------------- 
  avs_s1_waitrequest <= (avs_s1_read OR avs_s1_write) AND (NOT (i_rd_ready OR i_wr_ready));
  i_addr             <= "000" & std_ulogic_vector(avs_s1_address);
  avs_s1_readdata    <= std_logic_vector(i_shiftin);

  ----------------------------------------------------------------
  -- Save avalon command
  ---------------------------------------------------------------- 
  p_save_avalon : PROCESS(csi_clock_clk, rsi_reset_reset_n)
  BEGIN
    IF rsi_reset_reset_n = '0' THEN
      coe_s2_cs_n    <= '1';
      i_rd_enable    <= '0';
      i_wr_enable    <= '0';
      i_rd_dis       <= '0';
      i_wr_dis       <= '0';
      i_inactive_cnt <= to_unsigned(c_inactive_cnt, i_inactive_cnt'length);
      i_setup_cnt    <= to_unsigned(c_setup_cnt, i_setup_cnt'length);
    ELSIF rising_edge(csi_clock_clk) THEN

      -- ============ ============ =====================                                After Read
      IF i_rd_ready = '1' THEN
        -- ------------ ------------ ---------------------
        IF avs_s1_read = '0' THEN
          i_rd_dis <= '1';
        END IF;
        -- ------------ ------------ ---------------------
        coe_s2_cs_n <= '1';
        i_setup_cnt <= to_unsigned(c_setup_cnt, i_setup_cnt'length);
        -- ============ ============ ===================== 
        IF i_inactive_cnt = 0 THEN
          -- ------------ ------------ ---------------------
          IF i_rd_dis = '1' THEN
            i_rd_enable <= '0';
          END iF;
        -- ------------ ------------ ---------------------
        ELSE
          -- ------------ ------------ ---------------------
          i_inactive_cnt <= i_inactive_cnt - 1;
        -- ------------ ------------ ---------------------
        END IF;
      -- ============ ============ ===================== 
      ELSE
        -- ============ ============ =====================                            Before Read
        IF avs_s1_read = '1' AND i_wr_enable = '0' THEN
          -- ------------ ------------ ---------------------
          coe_s2_cs_n    <= '0';
          i_rd_dis       <= '0';
          i_wr_dis       <= '0';
          i_inactive_cnt <= to_unsigned(c_inactive_cnt, i_inactive_cnt'length);
          -- ============ ============ ===================== 
          IF i_setup_cnt = 0 THEN
            -- ------------ ------------ ---------------------
            i_rd_enable <= '1';
          -- ------------ ------------ ---------------------
          ELSE
            -- ------------ ------------ ---------------------
            i_setup_cnt <= i_setup_cnt - 1;
          -- ------------ ------------ ---------------------
          END IF;
        -- ------------ ------------ ---------------------
        END IF;
      -- ============ ============ ===================== 
      END IF;

      -- ============ ============ =====================                                 After Write
      IF i_wr_ready = '1' THEN
        -- ------------ ------------ ---------------------
        IF avs_s1_write = '0' THEN
          i_wr_dis <= '1';
        END IF;
        -- ------------ ------------ ---------------------
        coe_s2_cs_n <= '1';
        i_setup_cnt <= to_unsigned(c_setup_cnt, i_setup_cnt'length);
        -- ============ ============ ===================== 
        IF i_inactive_cnt = 0 THEN
          -- ------------ ------------ ---------------------
          IF i_wr_dis = '1' THEN
            i_wr_enable <= '0';
          END iF;
        -- ------------ ------------ ---------------------
        ELSE
          -- ------------ ------------ ---------------------
          i_inactive_cnt <= i_inactive_cnt - 1;
        -- ------------ ------------ ---------------------
        END IF;
      -- ============ ============ ===================== 
      ELSE
        -- ============ ============ =====================                              Before Write
        IF avs_s1_write = '1' AND i_rd_enable = '0' THEN
          -- ------------ ------------ ---------------------
          coe_s2_cs_n    <= '0';
          i_rd_dis       <= '0';
          i_wr_dis       <= '0';
          i_inactive_cnt <= to_unsigned(c_inactive_cnt, i_inactive_cnt'length);
          -- ============ ============ ===================== 
          IF i_setup_cnt = 0 THEN
            -- ------------ ------------ ---------------------
            i_wr_enable <= '1';
          -- ------------ ------------ ---------------------
          ELSE
            -- ------------ ------------ ---------------------
            i_setup_cnt <= i_setup_cnt - 1;
          -- ------------ ------------ ---------------------
          END IF;
        -- ------------ ------------ ---------------------
        END IF;
      -- ============ ============ ===================== 
      END IF;

    END IF;
  END PROCESS p_save_avalon;

  ----------------------------------------------------------------
  -- Write SPI
  ---------------------------------------------------------------- 
  p_write_spi : PROCESS(csi_clock_clk, rsi_reset_reset_n)
  BEGIN
    IF rsi_reset_reset_n = '0' THEN
      coe_s2_sclk <= g_cpol;
      coe_s2_sdi  <= '0';
      i_rd_ready  <= '0';
      i_wr_ready  <= '0';
      i_shiftout  <= (OTHERS => '0');
      i_shiftin   <= (OTHERS => '0');
      i_bit_cnt   <= to_unsigned(c_bit_cnt - 1, i_bit_cnt'length);
    ELSIF rising_edge(csi_clock_clk) THEN

      -- ============ ============ =====================           SCLK falling edge


      -- ============ ============ ===================== 
      IF i_wr_enable = '1' OR i_rd_enable = '1' THEN
        -- ------------ ------------ ---------------------
        IF i_sclk_en = '1' THEN         --                                SCLK rising edge
          -- ------------ ------------ ---------------------
          coe_s2_sdi  <= '0';
          coe_s2_sclk <= g_cpol;

          -- ============ ============ ===================== 
          IF i_bit_cnt = 0 THEN         -- Communication finished
            -- ------------ ------------ ---------------------
            i_rd_ready <= i_rd_enable;
            i_wr_ready <= i_wr_enable;
          -- ------------ ------------ ---------------------
          ELSIF i_bit_cnt = c_bit_cnt - 1 THEN -- Initialise Communication
            -- ------------ ------------ ---------------------
            i_shiftout  <= i_addr & std_ulogic_vector(avs_s1_writedata); -- Sample Avalon
            coe_s2_sdi  <= i_wr_enable;
            coe_s2_sclk <= NOT g_cpol;
          -- ------------ ------------ ---------------------
          ELSIF i_bit_cnt = c_break_bit THEN -- Wait one cycle
          -- ------------ ------------ ---------------------
          -- ------------ ------------ ---------------------
          ELSE
            -- ------------ ------------ --------------------- -- Write
            coe_s2_sdi  <= i_shiftout(i_shiftout'HIGH);
            i_shiftout  <= i_shiftout(i_shiftout'HIGH - 1 DOWNTO 0) & '0';
            coe_s2_sclk <= NOT g_cpol;
          -- ------------ ------------ ---------------------
          END IF;
        -- ============ ============ ===================== 
        END IF;
        -- ------------ ------------ ---------------------
        IF i_sclk_dis = '1' THEN
          coe_s2_sclk <= g_cpol;

          -- ============ ============ ===================== 
          IF i_bit_cnt = 0 THEN         -- Communication finished
          -- ------------ ------------ ---------------------
          -- ------------ ------------ ---------------------
          ELSE
            -- ------------ ------------ --------------------- -- Read
            i_bit_cnt <= i_bit_cnt - 1;
            i_shiftin <= i_shiftin(i_shiftin'HIGH - 1 DOWNTO 0) & coe_s2_sdo;
          -- ------------ ------------ ---------------------
          END IF;
        -- ============ ============ ===================== 
        END IF;
      -- ------------ ------------ ---------------------
      ELSE
        -- ------------ ------------ --------------------- Reset
        i_rd_ready  <= '0';
        i_wr_ready  <= '0';
        coe_s2_sdi  <= '0';
        coe_s2_sclk <= g_cpol;
        i_bit_cnt   <= to_unsigned(c_bit_cnt - 1, i_bit_cnt'length);
      -- ------------ ------------ ---------------------
      END IF;
    -- ============ ============ ===================== 

    END IF;
  END PROCESS p_write_spi;

END rtl;
