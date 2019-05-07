-----------------------------------------------------
-- Project : Encoder
-----------------------------------------------------
-- File    : axi_lw_slave_register_rtl.vhd
-- Library : encoder_lib
-- Author  : sebastian@radex-net.com
-- Company : Radex AG
-- Copyright(C) Radex AG
-----------------------------------------------------
-- Last commit:
--   $Author:: sebastian@radex-net.com  $
--      $Rev:: 2                                 $
--     $Date:: 2016-03-18 #$
-----------------------------------------------------
-- Description : Axi-light-weight slave, Qsys component
-----------------------------------------------------

LIBRARY ieee;
USE ieee.std_logic_1164.ALL;
USE ieee.numeric_std.ALL;
ENTITY axi_lw_slave_register IS
  GENERIC(
    g_master_id_width   : integer := 12;  -- AMBA AXI-light-weight-Bus: id-width
    g_master_data_width : integer := 32;  -- AMBA AXI-light-weight-Bus: data-width
    g_master_addr_width : integer := 16  -- AMBA AXI-light-weight-Bus: addr-width
    );
  PORT(
    -- --------------- global signals ---------------
    rsi_reset_reset_n      : IN  std_logic;
    csi_clock_clk          : IN  std_logic;
    -- --------------- write address channel signals ---------------
    axs_awaddr   : IN  std_logic_vector (g_master_addr_width-1 DOWNTO 0);  --lw
    axs_awprot   : IN  std_logic_vector (2 DOWNTO 0);                      --lw
    axs_awvalid  : IN  std_logic;       --lw
    axs_awready  : OUT std_logic;       --lw
    -- --------------- write data channel signals ---------------
    axs_wdata    : IN  std_logic_vector (g_master_data_width-1 DOWNTO 0);  --lw
    axs_wstrb    : IN  std_logic_vector (3 DOWNTO 0);                      --lw
    axs_wvalid   : IN  std_logic;       --lw
    axs_wready   : OUT std_logic;       --lw
    -- --------------- write response channel signals ---------------
    axs_bresp    : OUT std_logic_vector (1 DOWNTO 0);                      --lw
    axs_bvalid   : OUT std_logic;       --lw
    axs_bready   : IN  std_logic;       --lw
    -- --------------- read address channel signals ---------------
    axs_araddr   : IN  std_logic_vector (g_master_addr_width-1 DOWNTO 0);  --lw
    axs_arprot   : IN  std_logic_vector (2 DOWNTO 0);                      --lw
    axs_arvalid  : IN  std_logic;       --lw
    axs_arready  : OUT std_logic;       --lw
    -- --------------- read data channel signals ---------------
    axs_rdata    : OUT std_logic_vector (g_master_data_width-1 DOWNTO 0);  --lw
    axs_rresp    : OUT std_logic_vector (1 DOWNTO 0);                      --lw
    axs_rvalid   : OUT std_logic;       --lw
    axs_rready   : IN  std_logic;       --lw
    -- --------------- fpga signals ---------------
    coe_wr_addr  : OUT std_logic_vector (g_master_addr_width-1 DOWNTO 0);
    coe_wr_data  : OUT std_logic_vector (g_master_data_width-1 DOWNTO 0);
    coe_wr_strb  : OUT std_logic_vector (3 DOWNTO 0);
    coe_wr_valid : OUT std_logic;
    coe_rd_addr  : OUT std_logic_vector (g_master_addr_width-1 DOWNTO 0);
    coe_rd_data  : IN  std_logic_vector (g_master_data_width-1 DOWNTO 0);
    coe_rd_valid : OUT std_logic;
    coe_rd_ready : IN  std_logic
    );

-- Declarations
-- AMBA AXI4-lite slave from QSYS to memory in FPGA
-- read/write at the same time give one write command, and later the read command
-- connect only AMBA AXI4-lite master (no AXI4 or AXI3 master)

END ENTITY axi_lw_slave_register;


ARCHITECTURE rtl OF axi_lw_slave_register IS
  SIGNAL wr_enable_i     : std_logic;   -- '1' while write is in progress
  SIGNAL wr_done_i       : std_logic;   -- '1' impulse when write is done
  SIGNAL wr_start_addr_i : std_logic_vector (g_master_addr_width-1 DOWNTO 0);  -- start address 
  SIGNAL wr_prot_i       : std_logic_vector (2 DOWNTO 0);  -- [0] 0 normal 1 priveledged [1] 0 secure 1 unsecure [2] 0 data 1 instruction

  SIGNAL rd_enable_i     : std_logic;   -- '1' while write is in progress
  SIGNAL rd_done_i       : std_logic;   -- '1' impulse when write is done
  SIGNAL rd_start_addr_i : std_logic_vector (g_master_addr_width-1 DOWNTO 0);  -- start address
  SIGNAL rd_id_i         : std_logic_vector (g_master_id_width-1 DOWNTO 0);    -- read ID 
  SIGNAL rd_prot_i       : std_logic_vector (2 DOWNTO 0);  -- [0] 0 normal 1 priveledged [1] 0 secure 1 unsecure [2] 0 data 1 instruction

  SIGNAL i_write_response : std_ulogic;
  SIGNAL i_first_ready_check : boolean;
BEGIN

  ASSERT (g_master_data_width = 32 OR
          g_master_data_width = 64) REPORT "AXI-light-data size must be 32 or 64" SEVERITY error;


  ----------------------------------------------------------------
  -- save write command
  ---------------------------------------------------------------- 
  p_wr_save : PROCESS(csi_clock_clk, rsi_reset_reset_n)
  BEGIN
    IF rsi_reset_reset_n = '0' THEN
      axs_awready     <= '0';
      axs_wready      <= '0';
      wr_enable_i     <= '0';
      wr_start_addr_i <= (OTHERS => '0');
      wr_prot_i       <= (OTHERS => '0');
    ELSIF rising_edge(csi_clock_clk) THEN

      -- ============ ============ ===================== 
      IF wr_enable_i = '1' THEN    -- write in progress
      -- ------------ ------------ ---------------------
        IF wr_done_i = '1' THEN         -- write done, ready for next write
          wr_enable_i <= '0';
          axs_awready <= '1';
        ELSE
          axs_awready <= '0';
        END IF;
        IF axs_wvalid = '1' THEN        -- write data arrived
          axs_wready <= '0';
        END IF;
      -- ------------ ------------ ---------------------
      ELSIF rd_enable_i = '1' THEN -- wait for read to be finished
      -- ------------ ------------ ---------------------
        axs_awready <= '0'; 
      -- ------------ ------------ ---------------------
      ELSIF axs_awvalid = '1' THEN -- write init
      -- ------------ ------------ ---------------------
        axs_awready     <= '0';
        axs_wready      <= '1';
        wr_enable_i     <= '1';         -- start process write command
        wr_start_addr_i <= axs_awaddr;  -- save write command
        wr_prot_i       <= axs_awprot;
--        wr_id_i         <= axs_awid;
      -- ------------ ------------ --------------------- 
      ELSE                         -- ready for next write
      -- ------------ ------------ ---------------------
        axs_awready <= '1';             -- default 1 for faster writes
        axs_wready  <= '0';
      -- ------------ ------------ ---------------------
      END IF;
      
    END IF;
  END PROCESS p_wr_save;

  ----------------------------------------------------------------
  -- process write command
  ---------------------------------------------------------------- 
  p_wr : PROCESS(csi_clock_clk, rsi_reset_reset_n)
  BEGIN
    IF rsi_reset_reset_n = '0' THEN
      coe_wr_data      <= (OTHERS => '0');
      coe_wr_addr      <= (OTHERS => '0');
      coe_wr_strb      <= (OTHERS => '0');
      coe_wr_valid     <= '0';
      wr_done_i        <= '0';
      axs_bresp        <= "10";
      axs_bvalid       <= '0';
      i_write_response <= '0';
    ELSIF rising_edge(csi_clock_clk) THEN

      -- ============ ============ =====================      
      IF wr_enable_i = '1' AND axs_wvalid = '1' THEN  -- write one 32bit data
      -- ------------ ------------ ---------------------
        coe_wr_data  <= axs_wdata;
        coe_wr_addr  <= wr_start_addr_i;
        coe_wr_strb  <= axs_wstrb;
        coe_wr_valid <= '1';
        i_write_response <= '1';
      -- ------------ ------------ ---------------------
      ELSE
      -- ------------ ------------ ---------------------
        wr_done_i    <= '0';
        coe_wr_valid <= '0';
      -- ------------ ------------ ---------------------
      END IF;

      -- ============ ============ ===================== 
      IF i_write_response = '1' THEN  -- wait for bready to send write response
      -- ------------ ------------ ---------------------
        IF wr_prot_i(2) = '0' THEN      -- write response
          axs_bresp <= "00";            -- received data
        ELSE
          axs_bresp <= "10";            -- received an instruction
        END IF;
        axs_bvalid  <= '1';
--        axs_bid     <= wr_id_i;
        
        IF  axs_bready = '1' THEN        -- wait for master to be ready
          i_write_response <= '0';
          wr_done_i        <= '1';       -- write finished
        END IF;
      -- ------------ ------------ ---------------------
      ELSE
      -- ------------ ------------ ---------------------
        axs_bresp  <= "10";
        axs_bvalid <= '0';
      -- ------------ ------------ ---------------------
      END IF;

    END IF;
  END PROCESS p_wr;



  ----------------------------------------------------------------
  -- save read command
  ---------------------------------------------------------------- 
  p_rd_save : PROCESS(csi_clock_clk, rsi_reset_reset_n)
  BEGIN
    IF rsi_reset_reset_n = '0' THEN
      axs_arready     <= '0';
      rd_enable_i     <= '0';
      coe_rd_valid    <= '0';
      rd_start_addr_i <= (OTHERS => '0');
      rd_prot_i       <= (OTHERS => '0');
      rd_id_i         <= (OTHERS => '0');
    ELSIF rising_edge(csi_clock_clk) THEN

      -- ============ ============ ===================== 
      IF rd_enable_i = '1' THEN    -- read in progress
      -- ------------ ------------ ---------------------
        axs_arready   <= '0';
        coe_rd_valid  <= '1';
        IF rd_done_i = '1' THEN         -- read done, ready for next write
          rd_enable_i   <= '0';
          axs_arready   <= '1';
          coe_rd_valid  <= '0';
        END IF;
      -- ------------ ------------ ---------------------
      ELSIF axs_arvalid = '1' THEN      -- read init
      -- ------------ ------------ ---------------------
        axs_arready     <= '0';
        coe_rd_valid    <= '1';
        rd_enable_i     <= '1';         -- start process read command
        rd_start_addr_i <= axs_araddr;  -- save read command
        rd_prot_i       <= axs_arprot;
--        rd_id_i         <= axs_arid;
      -- ------------ ------------ ---------------------
      ELSE                              -- ready for next write
      -- ------------ ------------ ---------------------
        axs_arready     <= '1';         -- default 1 for faster writes
        coe_rd_valid    <= '0';
      -- ------------ ------------ ---------------------
      END IF;
      
    END IF;
  END PROCESS p_rd_save;

  ----------------------------------------------------------------
  -- set read address as soon as possible
  ---------------------------------------------------------------- 
  coe_rd_addr  <= rd_start_addr_i;

  ----------------------------------------------------------------
  -- process read command
  ---------------------------------------------------------------- 
  p_rd : PROCESS(csi_clock_clk, rsi_reset_reset_n)
  BEGIN
    IF rsi_reset_reset_n = '0' THEN
      axs_rdata  <= (OTHERS => '0');
      axs_rresp  <= (OTHERS => '0');
      axs_rvalid <= '0';
      rd_done_i  <= '0';
      i_first_ready_check <= false;
    ELSIF rising_edge(csi_clock_clk) THEN

      -- ============ ============ ===================== 
      IF rd_enable_i = '1' AND coe_rd_ready = '1'  AND wr_enable_i = '0' AND rd_done_i  <= '0' THEN  -- write one 32bit data 
      -- ------------ ------------ ---------------------  
        IF rd_prot_i(2) = '0' THEN      -- read response
          axs_rresp <= "00";            -- received data
        ELSE
          axs_rresp <= "10";            -- received an instruction
        END IF;
--        axs_rid    <= rd_id_i;
        axs_rdata  <= coe_rd_data;    
        axs_rvalid <= '1';
        IF axs_rready = '1' THEN        -- wait for master to be ready
          rd_done_i  <= '1';
          IF i_first_ready_check = false THEN
            axs_rvalid <= '0';
          ELSE
            axs_rvalid <= '1';
          END IF;
        END IF;
        i_first_ready_check <= false;
      -- ------------ ------------ ---------------------
      ELSE
      -- ------------ ------------ ---------------------
        axs_rresp  <= "10";
        axs_rvalid <= '0';
        rd_done_i  <= '0';
        i_first_ready_check <= true;
      -- ------------ ------------ ---------------------
      END IF;

    END IF;
  END PROCESS p_rd;
  

END ARCHITECTURE rtl;

