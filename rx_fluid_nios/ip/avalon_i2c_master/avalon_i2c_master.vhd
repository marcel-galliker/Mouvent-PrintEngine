-----------------------------------------------------
-- Project : Fluid Board
-----------------------------------------------------
-- File    : avalon_i2c_master_rtl.vhd
-- Library : fluid_board_lib
--- Author  : sebastian@radex-net.com
--- Company : Radex AG
--- Copyright(C) Radex AG
------------------------------------------------------
--- Last commit:
---   $Author:: sebastian@radex-net.com                 $
---      $Rev:: 0                                    $
---     $Date:: 2016-10-06 #$
-----------------------------------------------------
-- Description : Connect Avalon MM Slave to SPI Master for MAX31865 interfacing PT100 elements
-----------------------------------------------------

LIBRARY ieee;
USE ieee.std_logic_1164.ALL;
USE ieee.numeric_std.ALL;

entity avalon_i2c_master is
  port(
    csi_clock_clk      : in    std_logic;
    rsi_reset_reset_n  : in    std_logic;

    avs_s1_writedata   : in    std_logic_vector(15 downto 0);
    avs_s1_read        : in    std_logic;
    avs_s1_write       : in    std_logic;
    avs_s1_address     : in    std_logic_vector(7 downto 0); -- word
    avs_s1_waitrequest : out   std_logic;
    avs_s1_readdata    : out   std_logic_vector(15 downto 0);

    coe_s2_sclk        : out   std_logic;
    coe_s2_sda         : inout std_logic
  );
end avalon_i2c_master;

architecture rtl of avalon_i2c_master is
  constant c_cpol           : std_ulogic := '1';
  --  constant c_sclk_cnt       : integer := 125 * 2; --125 * 2=100kHz -- length of one intertakt periode = 125*20ns -> 400kHz -> two edges divides by 2 => 200kHz
  --  constant c_half_clken_cnt : integer := 125;
  --  constant c_sclk_cnt       : integer := 125; -- 200kHz
  --  constant c_half_clken_cnt : integer := 63;
  constant c_sclk_cnt       : integer    := 63; -- 400kHz
  constant c_half_clken_cnt : integer    := 32;

  constant c_inactive_cnt : integer := 65;
  constant c_setup_cnt    : integer := 40;
  constant c_bit_cnt      : integer := 8;
  --  constant c_break_bit    : integer := 9; -- bit 0 is always a break bit

  signal i_i2c_cs_n       : std_ulogic;
  signal i_sclk_cnt       : unsigned(8 - 1 downto 0);
  signal i_inactive_cnt   : unsigned(8 - 1 downto 0);
  signal i_setup_cnt      : unsigned(8 - 1 downto 0);
  signal i_bit_cnt        : unsigned(5 - 1 downto 0);
  signal i_shiftout       : std_ulogic_vector(avs_s1_writedata'high + avs_s1_address'length downto 0);
  signal i_shiftin        : std_ulogic_vector(avs_s1_readdata'high downto 0);
  signal i_addr           : std_ulogic_vector(avs_s1_address'high-1 downto 0);
  signal i_rd_enable      : std_ulogic;
  signal i_wr_enable      : std_ulogic;
  signal i_rd_dis         : std_ulogic;
  signal i_wr_dis         : std_ulogic;
  signal i_rd_ready       : std_ulogic;
  signal i_wr_ready       : std_ulogic;
  signal i_sclk_en        : std_ulogic;
  signal i_data_en        : std_ulogic;
  signal i_clk_tog        : std_ulogic;
  signal i_data_rdwr_tog  : std_ulogic;
  signal i_transfer_state : std_ulogic_vector(1 DOWNTO 0);
  signal i_rdwr_d         : std_ulogic;
  signal i_ack            : std_ulogic_vector(3 - 1 downto 0);
  signal i_addr_done      : std_ulogic;
  signal i_d0_done        : std_ulogic;
  signal i_sdo_rdwr       : std_ulogic;
  signal i_ack_rdwr       : std_ulogic;
  signal i_d1_done        : std_ulogic;
  signal i_sdo            : std_ulogic;
  signal i_sdo_en         : std_ulogic;
  signal i_sclk           : std_ulogic;
  signal i_rd_ack         : std_ulogic;
  signal i_rd_ack_ready   : std_ulogic;
  
  signal i_rd_inactive_on : std_ulogic;
  signal i_wr_inactive_on : std_ulogic;

begin

  ----------------------------------------------------------------
  -- Generate 5MHz enable
  ---------------------------------------------------------------- 
  p_clk_gen : PROCESS(csi_clock_clk, rsi_reset_reset_n)
  BEGIN
    IF rsi_reset_reset_n = '0' THEN
      i_sclk_en  <= '0';
      i_data_en  <= '0';
      --i_rdwr_d   <= '0';
      i_sclk_cnt <= to_unsigned(c_half_clken_cnt - 1, i_sclk_cnt'length);
    ELSIF rising_edge(csi_clock_clk) THEN
      i_sclk_en <= '0';                 -- Pulse
      i_data_en <= '0';                 -- Pulse

      -- ============ ============ ===================== 
      IF i_sclk_cnt = 0 THEN            -- Pulse clk rising edge
        -- ------------ ------------ ---------------------
        IF i_wr_enable = '1' OR i_rd_enable = '1' THEN
          i_sclk_en  <= '1';            --i_rdwr_d;
          --i_rdwr_d   <= '1';
          i_sclk_cnt <= to_unsigned(c_sclk_cnt - 1, i_sclk_cnt'length);
        --ELSE
        --i_rdwr_d <= '0';
        END IF;
      -- ------------ ------------ ---------------------
      ELSIF i_sclk_cnt = c_half_clken_cnt THEN -- Pulse clk falling edge
        -- ------------ ------------ ---------------------
        i_data_en  <= '1';
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
  avs_s1_waitrequest <= (avs_s1_read OR avs_s1_write) AND ((NOT (i_rd_ready OR i_wr_ready OR i_rd_ack_ready)) OR (i_rd_inactive_on AND i_wr_inactive_on));
  i_addr             <= std_ulogic_vector(avs_s1_address(avs_s1_address'high-1 DOWNTO 0));
  avs_s1_readdata    <= std_logic_vector(i_shiftin) WHEN i_rd_ack = '0'  ELSE --
                        std_logic_vector(to_unsigned(0, avs_s1_readdata'length-i_ack'length)) & std_logic(i_ack(0)) & std_logic(i_ack(1)) & std_logic(i_ack(i_ack'high));
  -- coe_s2_sdo         <= std_logic(i_i2c_cs_n OR i_i2c_sda);

  ----------------------------------------------------------------
  -- Save avalon command
  ---------------------------------------------------------------- 
  p_save_avalon : PROCESS(csi_clock_clk, rsi_reset_reset_n)
  BEGIN
    IF rsi_reset_reset_n = '0' THEN
      i_i2c_cs_n     <= '1';
      i_rd_enable    <= '0';
      i_wr_enable    <= '0';
      i_rd_dis       <= '0';
      i_wr_dis       <= '0';
	  i_rd_ack       <= '0';
	  i_rd_ack_ready <= '0';
	  i_rd_inactive_on <= '1';
	  i_wr_inactive_on <= '1';
      i_inactive_cnt <= to_unsigned(c_inactive_cnt, i_inactive_cnt'length);
      i_setup_cnt    <= to_unsigned(c_setup_cnt, i_setup_cnt'length);
    ELSIF rising_edge(csi_clock_clk) THEN
	  i_rd_ack <= '0';
	  i_rd_ack_ready <= i_rd_ack;

      -- ============ ============ =====================                                After Read
      IF i_rd_ready = '1' THEN
        -- ------------ ------------ ---------------------
        IF avs_s1_read = '0' THEN
          i_rd_dis <= '1';
        END IF;
        -- ------------ ------------ ---------------------
        i_i2c_cs_n  <= '1';
        i_setup_cnt <= to_unsigned(c_setup_cnt, i_setup_cnt'length);
        -- ============ ============ ===================== 
        IF i_inactive_cnt = 0 THEN
          -- ------------ ------------ ---------------------
		  i_rd_inactive_on <= '0';
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
	    i_rd_inactive_on <= '1';
        -- ============ ============ =====================                            Before Read
        IF avs_s1_read = '1' AND i_wr_enable = '0' THEN
          -- ------------ ------------ ---------------------
		  IF avs_s1_address(avs_s1_address'high) = '1' THEN
		    i_rd_ack <= '1';
			i_rd_inactive_on <= '0';
		  ELSE
            i_i2c_cs_n     <= '0';
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
        i_i2c_cs_n  <= '1';
        i_setup_cnt <= to_unsigned(c_setup_cnt, i_setup_cnt'length);
        -- ============ ============ ===================== 
        IF i_inactive_cnt = 0 THEN
          -- ------------ ------------ ---------------------
		  i_wr_inactive_on <= '0';
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
	    i_wr_inactive_on <= '1';
        -- ============ ============ =====================                              Before Write
        IF avs_s1_write = '1' AND i_rd_enable = '0' THEN
          -- ------------ ------------ ---------------------
          i_i2c_cs_n     <= '0';
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

  i_ack_rdwr <= (i_addr_done AND i_rd_enable);
  i_sdo_rdwr <= NOT i_ack_rdwr;

  ----------------------------------------------------------------
  -- Write SPI
  ---------------------------------------------------------------- 
  p_write_spi : PROCESS(csi_clock_clk, rsi_reset_reset_n)
    variable v_clk_tog_n : std_ulogic;
  BEGIN
    IF rsi_reset_reset_n = '0' THEN
      i_sclk           <= c_cpol;
      i_sdo            <= '0';
      i_sdo_en         <= '1';
      i_clk_tog        <= c_cpol;
      i_data_rdwr_tog  <= '0';          -- '0' = write, '1' = read
      i_rdwr_d         <= '0';
      i_d0_done        <= '0';
      i_d1_done        <= '0';
      v_clk_tog_n      := '0';
      i_addr_done      <= '0';
      i_rd_ready       <= '0';
      i_wr_ready       <= '0';
      i_ack            <= (OTHERS => '0');
      i_transfer_state <= (OTHERS => '0');
      i_shiftout       <= (OTHERS => '0');
      i_shiftin        <= (OTHERS => '0');
      i_bit_cnt        <= to_unsigned(c_bit_cnt, i_bit_cnt'length);
    ELSIF rising_edge(csi_clock_clk) THEN
      v_clk_tog_n := '0';
      i_rdwr_d    <= '0';
      -- ============ ============ =====================     


      -- ============ ============ ===================== 
      IF i_wr_enable = '1' OR i_rd_enable = '1' THEN
        -- ------------ ------------ ---------------------
        i_rdwr_d <= '1';
        IF i_rdwr_d = '0' THEN
          i_shiftout <= i_addr & i_rd_enable & std_ulogic_vector(avs_s1_writedata); -- Sample Avalon
        END IF;
        -- ------------ ------------ ---------------------
        IF i_data_en = '1' THEN         -- Change data
          case i_transfer_state is
            when "00" =>                -- rd/wr
              IF i_data_rdwr_tog = '0' THEN -- wr
                i_sdo_en   <= i_sdo_rdwr;
                i_sdo      <= i_i2c_cs_n OR i_shiftout(i_shiftout'HIGH);
                i_shiftout <= i_shiftout(i_shiftout'HIGH - 1 DOWNTO 0) & '0';
                IF i_bit_cnt = 0 THEN
                  i_transfer_state <= "01"; -- ack
                  i_sdo_en         <= i_ack_rdwr;
                  i_d0_done        <= i_addr_done;
                  i_d1_done        <= i_d0_done;
                  i_sdo            <= i_i2c_cs_n OR i_d0_done;
                END IF;
                i_data_rdwr_tog <= '1';
              ELSE                      -- rd
                i_bit_cnt       <= i_bit_cnt - 1;
                i_shiftin       <= i_shiftin(i_shiftin'HIGH - 1 DOWNTO 0) & coe_s2_sda;
                i_data_rdwr_tog <= '0';
              END IF;
            when "01" =>                -- ack
              i_ack           <= i_ack(i_ack'HIGH - 1 DOWNTO 0) & coe_s2_sda;
              i_addr_done     <= '1';
              i_data_rdwr_tog <= '0';
              i_bit_cnt       <= to_unsigned(c_bit_cnt, i_bit_cnt'length);
              IF i_d1_done = '1' THEN
                i_transfer_state <= "10";
              ELSE
                i_transfer_state <= "00";
              END IF;
            when "10" =>                -- done 
              IF i_data_rdwr_tog = '0' THEN
                i_data_rdwr_tog <= '1';
                i_sdo           <= i_i2c_cs_n;
              ELSE
                i_rd_ready <= i_rd_enable;
                i_wr_ready <= i_wr_enable;
                i_sdo      <= '1';
              END IF;
              i_sdo_en <= '1';
            when "11" =>                -- none
              i_transfer_state <= "10";
            when others => null;
          end case;
        END IF;
        -- ------------ ------------ ---------------------
        IF i_sclk_en = '1' THEN         --                                SCLK edge
          -- ------------ ------------ ---------------------
          v_clk_tog_n := NOT i_clk_tog;
          i_clk_tog   <= v_clk_tog_n;
          i_sclk      <= std_logic(v_clk_tog_n) OR i_i2c_cs_n;
        -- ------------ ------------ ---------------------
        END IF;
      -- ============ ============ ===================== 
      ELSE
        -- ------------ ------------ --------------------- Reset
        i_d0_done        <= '0';
        i_d1_done        <= '0';
        i_addr_done      <= '0';
        i_rd_ready       <= '0';
        i_wr_ready       <= '0';
        i_sdo            <= i_i2c_cs_n;
        i_sdo_en         <= '1';
        i_sclk           <= c_cpol;
        i_clk_tog        <= c_cpol;
        i_data_rdwr_tog  <= '0';
        i_bit_cnt        <= to_unsigned(c_bit_cnt, i_bit_cnt'length);
        i_transfer_state <= "00";
      -- ------------ ------------ ---------------------
      END IF;
    -- ============ ============ ===================== 

    END IF;
  END PROCESS p_write_spi;

  coe_s2_sclk <= i_sclk WHEN i_i2c_cs_n = '0' ELSE 'Z';
  coe_s2_sda  <= i_sdo WHEN i_sdo_en = '1' ELSE 'Z';

END rtl;
