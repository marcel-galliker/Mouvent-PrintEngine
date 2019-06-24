	component fluid_board_soc is
		port (
			avalon2fpga_slave_0_s2_writedata                       : out   std_logic_vector(15 downto 0);                    -- writedata
			avalon2fpga_slave_0_s2_read                            : out   std_logic;                                        -- read
			avalon2fpga_slave_0_s2_write                           : out   std_logic;                                        -- write
			avalon2fpga_slave_0_s2_address                         : out   std_logic_vector(6 downto 0);                     -- address
			avalon2fpga_slave_0_s2_waitrequest                     : in    std_logic                     := 'X';             -- waitrequest
			avalon2fpga_slave_0_s2_readdata                        : in    std_logic_vector(15 downto 0) := (others => 'X'); -- readdata
			avalon_spi_amc7891_1_conduit_end_sclk                  : out   std_logic;                                        -- sclk
			avalon_spi_amc7891_1_conduit_end_cs_n                  : out   std_logic;                                        -- cs_n
			avalon_spi_amc7891_1_conduit_end_sdio                  : out   std_logic;                                        -- sdio
			avalon_spi_amc7891_1_conduit_end_sdo                   : in    std_logic                     := 'X';             -- sdo
			avalon_spi_max31865_0_conduit_end_0_drdy_n             : in    std_logic                     := 'X';             -- drdy_n
			avalon_spi_max31865_0_conduit_end_0_sclk               : out   std_logic;                                        -- sclk
			avalon_spi_max31865_0_conduit_end_0_cs_n               : out   std_logic;                                        -- cs_n
			avalon_spi_max31865_0_conduit_end_0_sdi                : out   std_logic;                                        -- sdi
			avalon_spi_max31865_0_conduit_end_0_sdo                : in    std_logic                     := 'X';             -- sdo
			avalon_spi_max31865_1_conduit_end_0_drdy_n             : in    std_logic                     := 'X';             -- drdy_n
			avalon_spi_max31865_1_conduit_end_0_sclk               : out   std_logic;                                        -- sclk
			avalon_spi_max31865_1_conduit_end_0_cs_n               : out   std_logic;                                        -- cs_n
			avalon_spi_max31865_1_conduit_end_0_sdi                : out   std_logic;                                        -- sdi
			avalon_spi_max31865_1_conduit_end_0_sdo                : in    std_logic                     := 'X';             -- sdo
			avalon_spi_max31865_2_conduit_end_0_drdy_n             : in    std_logic                     := 'X';             -- drdy_n
			avalon_spi_max31865_2_conduit_end_0_sclk               : out   std_logic;                                        -- sclk
			avalon_spi_max31865_2_conduit_end_0_cs_n               : out   std_logic;                                        -- cs_n
			avalon_spi_max31865_2_conduit_end_0_sdi                : out   std_logic;                                        -- sdi
			avalon_spi_max31865_2_conduit_end_0_sdo                : in    std_logic                     := 'X';             -- sdo
			avalon_spi_max31865_3_conduit_end_0_drdy_n             : in    std_logic                     := 'X';             -- drdy_n
			avalon_spi_max31865_3_conduit_end_0_sclk               : out   std_logic;                                        -- sclk
			avalon_spi_max31865_3_conduit_end_0_cs_n               : out   std_logic;                                        -- cs_n
			avalon_spi_max31865_3_conduit_end_0_sdi                : out   std_logic;                                        -- sdi
			avalon_spi_max31865_3_conduit_end_0_sdo                : in    std_logic                     := 'X';             -- sdo
			avalon_spi_max31865_4_conduit_end_0_drdy_n             : in    std_logic                     := 'X';             -- drdy_n
			avalon_spi_max31865_4_conduit_end_0_sclk               : out   std_logic;                                        -- sclk
			avalon_spi_max31865_4_conduit_end_0_cs_n               : out   std_logic;                                        -- cs_n
			avalon_spi_max31865_4_conduit_end_0_sdi                : out   std_logic;                                        -- sdi
			avalon_spi_max31865_4_conduit_end_0_sdo                : in    std_logic                     := 'X';             -- sdo
			avalon_spi_max31865_5_conduit_end_0_drdy_n             : in    std_logic                     := 'X';             -- drdy_n
			avalon_spi_max31865_5_conduit_end_0_sclk               : out   std_logic;                                        -- sclk
			avalon_spi_max31865_5_conduit_end_0_cs_n               : out   std_logic;                                        -- cs_n
			avalon_spi_max31865_5_conduit_end_0_sdi                : out   std_logic;                                        -- sdi
			avalon_spi_max31865_5_conduit_end_0_sdo                : in    std_logic                     := 'X';             -- sdo
			axi_lw_slave_register_0_conduit_end_0_wr_strb          : out   std_logic_vector(3 downto 0);                     -- wr_strb
			axi_lw_slave_register_0_conduit_end_0_wr_valid         : out   std_logic;                                        -- wr_valid
			axi_lw_slave_register_0_conduit_end_0_rd_addr          : out   std_logic_vector(15 downto 0);                    -- rd_addr
			axi_lw_slave_register_0_conduit_end_0_rd_data          : in    std_logic_vector(31 downto 0) := (others => 'X'); -- rd_data
			axi_lw_slave_register_0_conduit_end_0_rd_valid         : out   std_logic;                                        -- rd_valid
			axi_lw_slave_register_0_conduit_end_0_rd_ready         : in    std_logic                     := 'X';             -- rd_ready
			axi_lw_slave_register_0_conduit_end_0_wr_data          : out   std_logic_vector(31 downto 0);                    -- wr_data
			axi_lw_slave_register_0_conduit_end_0_wr_addr          : out   std_logic_vector(15 downto 0);                    -- wr_addr
			flush_pump_pwm_duty_cycle_external_connection_export   : out   std_logic_vector(31 downto 0);                    -- export
			flush_pump_pwm_freq_external_connection_export         : out   std_logic_vector(31 downto 0);                    -- export
			hps_0_uart1_cts                                        : in    std_logic                     := 'X';             -- cts
			hps_0_uart1_dsr                                        : in    std_logic                     := 'X';             -- dsr
			hps_0_uart1_dcd                                        : in    std_logic                     := 'X';             -- dcd
			hps_0_uart1_ri                                         : in    std_logic                     := 'X';             -- ri
			hps_0_uart1_dtr                                        : out   std_logic;                                        -- dtr
			hps_0_uart1_rts                                        : out   std_logic;                                        -- rts
			hps_0_uart1_out1_n                                     : out   std_logic;                                        -- out1_n
			hps_0_uart1_out2_n                                     : out   std_logic;                                        -- out2_n
			hps_0_uart1_rxd                                        : in    std_logic                     := 'X';             -- rxd
			hps_0_uart1_txd                                        : out   std_logic;                                        -- txd
			hps_io_hps_io_emac0_inst_TX_CLK                        : out   std_logic;                                        -- hps_io_emac0_inst_TX_CLK
			hps_io_hps_io_emac0_inst_TXD0                          : out   std_logic;                                        -- hps_io_emac0_inst_TXD0
			hps_io_hps_io_emac0_inst_TXD1                          : out   std_logic;                                        -- hps_io_emac0_inst_TXD1
			hps_io_hps_io_emac0_inst_TXD2                          : out   std_logic;                                        -- hps_io_emac0_inst_TXD2
			hps_io_hps_io_emac0_inst_TXD3                          : out   std_logic;                                        -- hps_io_emac0_inst_TXD3
			hps_io_hps_io_emac0_inst_RXD0                          : in    std_logic                     := 'X';             -- hps_io_emac0_inst_RXD0
			hps_io_hps_io_emac0_inst_MDIO                          : inout std_logic                     := 'X';             -- hps_io_emac0_inst_MDIO
			hps_io_hps_io_emac0_inst_MDC                           : out   std_logic;                                        -- hps_io_emac0_inst_MDC
			hps_io_hps_io_emac0_inst_RX_CTL                        : in    std_logic                     := 'X';             -- hps_io_emac0_inst_RX_CTL
			hps_io_hps_io_emac0_inst_TX_CTL                        : out   std_logic;                                        -- hps_io_emac0_inst_TX_CTL
			hps_io_hps_io_emac0_inst_RX_CLK                        : in    std_logic                     := 'X';             -- hps_io_emac0_inst_RX_CLK
			hps_io_hps_io_emac0_inst_RXD1                          : in    std_logic                     := 'X';             -- hps_io_emac0_inst_RXD1
			hps_io_hps_io_emac0_inst_RXD2                          : in    std_logic                     := 'X';             -- hps_io_emac0_inst_RXD2
			hps_io_hps_io_emac0_inst_RXD3                          : in    std_logic                     := 'X';             -- hps_io_emac0_inst_RXD3
			hps_io_hps_io_sdio_inst_CMD                            : inout std_logic                     := 'X';             -- hps_io_sdio_inst_CMD
			hps_io_hps_io_sdio_inst_D0                             : inout std_logic                     := 'X';             -- hps_io_sdio_inst_D0
			hps_io_hps_io_sdio_inst_D1                             : inout std_logic                     := 'X';             -- hps_io_sdio_inst_D1
			hps_io_hps_io_sdio_inst_D4                             : inout std_logic                     := 'X';             -- hps_io_sdio_inst_D4
			hps_io_hps_io_sdio_inst_D5                             : inout std_logic                     := 'X';             -- hps_io_sdio_inst_D5
			hps_io_hps_io_sdio_inst_D6                             : inout std_logic                     := 'X';             -- hps_io_sdio_inst_D6
			hps_io_hps_io_sdio_inst_D7                             : inout std_logic                     := 'X';             -- hps_io_sdio_inst_D7
			hps_io_hps_io_sdio_inst_CLK                            : out   std_logic;                                        -- hps_io_sdio_inst_CLK
			hps_io_hps_io_sdio_inst_D2                             : inout std_logic                     := 'X';             -- hps_io_sdio_inst_D2
			hps_io_hps_io_sdio_inst_D3                             : inout std_logic                     := 'X';             -- hps_io_sdio_inst_D3
			hps_io_hps_io_usb1_inst_D0                             : inout std_logic                     := 'X';             -- hps_io_usb1_inst_D0
			hps_io_hps_io_usb1_inst_D1                             : inout std_logic                     := 'X';             -- hps_io_usb1_inst_D1
			hps_io_hps_io_usb1_inst_D2                             : inout std_logic                     := 'X';             -- hps_io_usb1_inst_D2
			hps_io_hps_io_usb1_inst_D3                             : inout std_logic                     := 'X';             -- hps_io_usb1_inst_D3
			hps_io_hps_io_usb1_inst_D4                             : inout std_logic                     := 'X';             -- hps_io_usb1_inst_D4
			hps_io_hps_io_usb1_inst_D5                             : inout std_logic                     := 'X';             -- hps_io_usb1_inst_D5
			hps_io_hps_io_usb1_inst_D6                             : inout std_logic                     := 'X';             -- hps_io_usb1_inst_D6
			hps_io_hps_io_usb1_inst_D7                             : inout std_logic                     := 'X';             -- hps_io_usb1_inst_D7
			hps_io_hps_io_usb1_inst_CLK                            : in    std_logic                     := 'X';             -- hps_io_usb1_inst_CLK
			hps_io_hps_io_usb1_inst_STP                            : out   std_logic;                                        -- hps_io_usb1_inst_STP
			hps_io_hps_io_usb1_inst_DIR                            : in    std_logic                     := 'X';             -- hps_io_usb1_inst_DIR
			hps_io_hps_io_usb1_inst_NXT                            : in    std_logic                     := 'X';             -- hps_io_usb1_inst_NXT
			hps_io_hps_io_uart0_inst_RX                            : in    std_logic                     := 'X';             -- hps_io_uart0_inst_RX
			hps_io_hps_io_uart0_inst_TX                            : out   std_logic;                                        -- hps_io_uart0_inst_TX
			hps_io_hps_io_gpio_inst_GPIO37                         : inout std_logic                     := 'X';             -- hps_io_gpio_inst_GPIO37
			hps_io_hps_io_gpio_inst_GPIO44                         : inout std_logic                     := 'X';             -- hps_io_gpio_inst_GPIO44
			hps_io_hps_io_gpio_inst_GPIO59                         : inout std_logic                     := 'X';             -- hps_io_gpio_inst_GPIO59
			i2c_master_d_conduit_end_sda                           : inout std_logic                     := 'X';             -- sda
			i2c_master_d_conduit_end_scl                           : inout std_logic                     := 'X';             -- scl
			i2c_master_f_conduit_end_sda                           : inout std_logic                     := 'X';             -- sda
			i2c_master_f_conduit_end_scl                           : inout std_logic                     := 'X';             -- scl
			i2c_master_is1_conduit_end_sda                         : inout std_logic                     := 'X';             -- sda
			i2c_master_is1_conduit_end_scl                         : inout std_logic                     := 'X';             -- scl
			i2c_master_is2_conduit_end_sda                         : inout std_logic                     := 'X';             -- sda
			i2c_master_is2_conduit_end_scl                         : inout std_logic                     := 'X';             -- scl
			i2c_master_is3_conduit_end_sda                         : inout std_logic                     := 'X';             -- sda
			i2c_master_is3_conduit_end_scl                         : inout std_logic                     := 'X';             -- scl
			i2c_master_is4_conduit_end_sda                         : inout std_logic                     := 'X';             -- sda
			i2c_master_is4_conduit_end_scl                         : inout std_logic                     := 'X';             -- scl
			i2c_master_p_conduit_end_sda                           : inout std_logic                     := 'X';             -- sda
			i2c_master_p_conduit_end_scl                           : inout std_logic                     := 'X';             -- scl
			memory_mem_a                                           : out   std_logic_vector(14 downto 0);                    -- mem_a
			memory_mem_ba                                          : out   std_logic_vector(2 downto 0);                     -- mem_ba
			memory_mem_ck                                          : out   std_logic;                                        -- mem_ck
			memory_mem_ck_n                                        : out   std_logic;                                        -- mem_ck_n
			memory_mem_cke                                         : out   std_logic;                                        -- mem_cke
			memory_mem_cs_n                                        : out   std_logic;                                        -- mem_cs_n
			memory_mem_ras_n                                       : out   std_logic;                                        -- mem_ras_n
			memory_mem_cas_n                                       : out   std_logic;                                        -- mem_cas_n
			memory_mem_we_n                                        : out   std_logic;                                        -- mem_we_n
			memory_mem_reset_n                                     : out   std_logic;                                        -- mem_reset_n
			memory_mem_dq                                          : inout std_logic_vector(31 downto 0) := (others => 'X'); -- mem_dq
			memory_mem_dqs                                         : inout std_logic_vector(3 downto 0)  := (others => 'X'); -- mem_dqs
			memory_mem_dqs_n                                       : inout std_logic_vector(3 downto 0)  := (others => 'X'); -- mem_dqs_n
			memory_mem_odt                                         : out   std_logic;                                        -- mem_odt
			memory_mem_dm                                          : out   std_logic_vector(3 downto 0);                     -- mem_dm
			memory_oct_rzqin                                       : in    std_logic                     := 'X';             -- oct_rzqin
			nios2_qsys_0_cpu_resetrequest_conduit_cpu_resetrequest : in    std_logic                     := 'X';             -- cpu_resetrequest
			nios2_qsys_0_cpu_resetrequest_conduit_cpu_resettaken   : out   std_logic;                                        -- cpu_resettaken
			pio_input_external_connection_export                   : in    std_logic_vector(14 downto 0) := (others => 'X'); -- export
			pio_output_external_connection_export                  : out   std_logic_vector(15 downto 0);                    -- export
			pio_reset_nios_external_connection_export              : out   std_logic;                                        -- export
			pio_watchdog_cnt_external_connection_export            : out   std_logic_vector(31 downto 0);                    -- export
			pio_watchdog_freq_external_connection_export           : out   std_logic_vector(31 downto 0);                    -- export
			pll_0_locked_export                                    : out   std_logic;                                        -- export
			pll_0_refclk_clk                                       : in    std_logic                     := 'X';             -- clk
			pll_0_reset_reset                                      : in    std_logic                     := 'X';             -- reset
			pll_0_sys_clk_clk                                      : out   std_logic;                                        -- clk
			pll_0_sys_reset_reset_n                                : out   std_logic;                                        -- reset_n
			qsys_reset_reset_n                                     : in    std_logic                     := 'X';             -- reset_n
			uart_cond_external_connection_rxd                      : in    std_logic                     := 'X';             -- rxd
			uart_cond_external_connection_txd                      : out   std_logic                                         -- txd
		);
	end component fluid_board_soc;

	u0 : component fluid_board_soc
		port map (
			avalon2fpga_slave_0_s2_writedata                       => CONNECTED_TO_avalon2fpga_slave_0_s2_writedata,                       --                        avalon2fpga_slave_0_s2.writedata
			avalon2fpga_slave_0_s2_read                            => CONNECTED_TO_avalon2fpga_slave_0_s2_read,                            --                                              .read
			avalon2fpga_slave_0_s2_write                           => CONNECTED_TO_avalon2fpga_slave_0_s2_write,                           --                                              .write
			avalon2fpga_slave_0_s2_address                         => CONNECTED_TO_avalon2fpga_slave_0_s2_address,                         --                                              .address
			avalon2fpga_slave_0_s2_waitrequest                     => CONNECTED_TO_avalon2fpga_slave_0_s2_waitrequest,                     --                                              .waitrequest
			avalon2fpga_slave_0_s2_readdata                        => CONNECTED_TO_avalon2fpga_slave_0_s2_readdata,                        --                                              .readdata
			avalon_spi_amc7891_1_conduit_end_sclk                  => CONNECTED_TO_avalon_spi_amc7891_1_conduit_end_sclk,                  --              avalon_spi_amc7891_1_conduit_end.sclk
			avalon_spi_amc7891_1_conduit_end_cs_n                  => CONNECTED_TO_avalon_spi_amc7891_1_conduit_end_cs_n,                  --                                              .cs_n
			avalon_spi_amc7891_1_conduit_end_sdio                  => CONNECTED_TO_avalon_spi_amc7891_1_conduit_end_sdio,                  --                                              .sdio
			avalon_spi_amc7891_1_conduit_end_sdo                   => CONNECTED_TO_avalon_spi_amc7891_1_conduit_end_sdo,                   --                                              .sdo
			avalon_spi_max31865_0_conduit_end_0_drdy_n             => CONNECTED_TO_avalon_spi_max31865_0_conduit_end_0_drdy_n,             --           avalon_spi_max31865_0_conduit_end_0.drdy_n
			avalon_spi_max31865_0_conduit_end_0_sclk               => CONNECTED_TO_avalon_spi_max31865_0_conduit_end_0_sclk,               --                                              .sclk
			avalon_spi_max31865_0_conduit_end_0_cs_n               => CONNECTED_TO_avalon_spi_max31865_0_conduit_end_0_cs_n,               --                                              .cs_n
			avalon_spi_max31865_0_conduit_end_0_sdi                => CONNECTED_TO_avalon_spi_max31865_0_conduit_end_0_sdi,                --                                              .sdi
			avalon_spi_max31865_0_conduit_end_0_sdo                => CONNECTED_TO_avalon_spi_max31865_0_conduit_end_0_sdo,                --                                              .sdo
			avalon_spi_max31865_1_conduit_end_0_drdy_n             => CONNECTED_TO_avalon_spi_max31865_1_conduit_end_0_drdy_n,             --           avalon_spi_max31865_1_conduit_end_0.drdy_n
			avalon_spi_max31865_1_conduit_end_0_sclk               => CONNECTED_TO_avalon_spi_max31865_1_conduit_end_0_sclk,               --                                              .sclk
			avalon_spi_max31865_1_conduit_end_0_cs_n               => CONNECTED_TO_avalon_spi_max31865_1_conduit_end_0_cs_n,               --                                              .cs_n
			avalon_spi_max31865_1_conduit_end_0_sdi                => CONNECTED_TO_avalon_spi_max31865_1_conduit_end_0_sdi,                --                                              .sdi
			avalon_spi_max31865_1_conduit_end_0_sdo                => CONNECTED_TO_avalon_spi_max31865_1_conduit_end_0_sdo,                --                                              .sdo
			avalon_spi_max31865_2_conduit_end_0_drdy_n             => CONNECTED_TO_avalon_spi_max31865_2_conduit_end_0_drdy_n,             --           avalon_spi_max31865_2_conduit_end_0.drdy_n
			avalon_spi_max31865_2_conduit_end_0_sclk               => CONNECTED_TO_avalon_spi_max31865_2_conduit_end_0_sclk,               --                                              .sclk
			avalon_spi_max31865_2_conduit_end_0_cs_n               => CONNECTED_TO_avalon_spi_max31865_2_conduit_end_0_cs_n,               --                                              .cs_n
			avalon_spi_max31865_2_conduit_end_0_sdi                => CONNECTED_TO_avalon_spi_max31865_2_conduit_end_0_sdi,                --                                              .sdi
			avalon_spi_max31865_2_conduit_end_0_sdo                => CONNECTED_TO_avalon_spi_max31865_2_conduit_end_0_sdo,                --                                              .sdo
			avalon_spi_max31865_3_conduit_end_0_drdy_n             => CONNECTED_TO_avalon_spi_max31865_3_conduit_end_0_drdy_n,             --           avalon_spi_max31865_3_conduit_end_0.drdy_n
			avalon_spi_max31865_3_conduit_end_0_sclk               => CONNECTED_TO_avalon_spi_max31865_3_conduit_end_0_sclk,               --                                              .sclk
			avalon_spi_max31865_3_conduit_end_0_cs_n               => CONNECTED_TO_avalon_spi_max31865_3_conduit_end_0_cs_n,               --                                              .cs_n
			avalon_spi_max31865_3_conduit_end_0_sdi                => CONNECTED_TO_avalon_spi_max31865_3_conduit_end_0_sdi,                --                                              .sdi
			avalon_spi_max31865_3_conduit_end_0_sdo                => CONNECTED_TO_avalon_spi_max31865_3_conduit_end_0_sdo,                --                                              .sdo
			avalon_spi_max31865_4_conduit_end_0_drdy_n             => CONNECTED_TO_avalon_spi_max31865_4_conduit_end_0_drdy_n,             --           avalon_spi_max31865_4_conduit_end_0.drdy_n
			avalon_spi_max31865_4_conduit_end_0_sclk               => CONNECTED_TO_avalon_spi_max31865_4_conduit_end_0_sclk,               --                                              .sclk
			avalon_spi_max31865_4_conduit_end_0_cs_n               => CONNECTED_TO_avalon_spi_max31865_4_conduit_end_0_cs_n,               --                                              .cs_n
			avalon_spi_max31865_4_conduit_end_0_sdi                => CONNECTED_TO_avalon_spi_max31865_4_conduit_end_0_sdi,                --                                              .sdi
			avalon_spi_max31865_4_conduit_end_0_sdo                => CONNECTED_TO_avalon_spi_max31865_4_conduit_end_0_sdo,                --                                              .sdo
			avalon_spi_max31865_5_conduit_end_0_drdy_n             => CONNECTED_TO_avalon_spi_max31865_5_conduit_end_0_drdy_n,             --           avalon_spi_max31865_5_conduit_end_0.drdy_n
			avalon_spi_max31865_5_conduit_end_0_sclk               => CONNECTED_TO_avalon_spi_max31865_5_conduit_end_0_sclk,               --                                              .sclk
			avalon_spi_max31865_5_conduit_end_0_cs_n               => CONNECTED_TO_avalon_spi_max31865_5_conduit_end_0_cs_n,               --                                              .cs_n
			avalon_spi_max31865_5_conduit_end_0_sdi                => CONNECTED_TO_avalon_spi_max31865_5_conduit_end_0_sdi,                --                                              .sdi
			avalon_spi_max31865_5_conduit_end_0_sdo                => CONNECTED_TO_avalon_spi_max31865_5_conduit_end_0_sdo,                --                                              .sdo
			axi_lw_slave_register_0_conduit_end_0_wr_strb          => CONNECTED_TO_axi_lw_slave_register_0_conduit_end_0_wr_strb,          --         axi_lw_slave_register_0_conduit_end_0.wr_strb
			axi_lw_slave_register_0_conduit_end_0_wr_valid         => CONNECTED_TO_axi_lw_slave_register_0_conduit_end_0_wr_valid,         --                                              .wr_valid
			axi_lw_slave_register_0_conduit_end_0_rd_addr          => CONNECTED_TO_axi_lw_slave_register_0_conduit_end_0_rd_addr,          --                                              .rd_addr
			axi_lw_slave_register_0_conduit_end_0_rd_data          => CONNECTED_TO_axi_lw_slave_register_0_conduit_end_0_rd_data,          --                                              .rd_data
			axi_lw_slave_register_0_conduit_end_0_rd_valid         => CONNECTED_TO_axi_lw_slave_register_0_conduit_end_0_rd_valid,         --                                              .rd_valid
			axi_lw_slave_register_0_conduit_end_0_rd_ready         => CONNECTED_TO_axi_lw_slave_register_0_conduit_end_0_rd_ready,         --                                              .rd_ready
			axi_lw_slave_register_0_conduit_end_0_wr_data          => CONNECTED_TO_axi_lw_slave_register_0_conduit_end_0_wr_data,          --                                              .wr_data
			axi_lw_slave_register_0_conduit_end_0_wr_addr          => CONNECTED_TO_axi_lw_slave_register_0_conduit_end_0_wr_addr,          --                                              .wr_addr
			flush_pump_pwm_duty_cycle_external_connection_export   => CONNECTED_TO_flush_pump_pwm_duty_cycle_external_connection_export,   -- flush_pump_pwm_duty_cycle_external_connection.export
			flush_pump_pwm_freq_external_connection_export         => CONNECTED_TO_flush_pump_pwm_freq_external_connection_export,         --       flush_pump_pwm_freq_external_connection.export
			hps_0_uart1_cts                                        => CONNECTED_TO_hps_0_uart1_cts,                                        --                                   hps_0_uart1.cts
			hps_0_uart1_dsr                                        => CONNECTED_TO_hps_0_uart1_dsr,                                        --                                              .dsr
			hps_0_uart1_dcd                                        => CONNECTED_TO_hps_0_uart1_dcd,                                        --                                              .dcd
			hps_0_uart1_ri                                         => CONNECTED_TO_hps_0_uart1_ri,                                         --                                              .ri
			hps_0_uart1_dtr                                        => CONNECTED_TO_hps_0_uart1_dtr,                                        --                                              .dtr
			hps_0_uart1_rts                                        => CONNECTED_TO_hps_0_uart1_rts,                                        --                                              .rts
			hps_0_uart1_out1_n                                     => CONNECTED_TO_hps_0_uart1_out1_n,                                     --                                              .out1_n
			hps_0_uart1_out2_n                                     => CONNECTED_TO_hps_0_uart1_out2_n,                                     --                                              .out2_n
			hps_0_uart1_rxd                                        => CONNECTED_TO_hps_0_uart1_rxd,                                        --                                              .rxd
			hps_0_uart1_txd                                        => CONNECTED_TO_hps_0_uart1_txd,                                        --                                              .txd
			hps_io_hps_io_emac0_inst_TX_CLK                        => CONNECTED_TO_hps_io_hps_io_emac0_inst_TX_CLK,                        --                                        hps_io.hps_io_emac0_inst_TX_CLK
			hps_io_hps_io_emac0_inst_TXD0                          => CONNECTED_TO_hps_io_hps_io_emac0_inst_TXD0,                          --                                              .hps_io_emac0_inst_TXD0
			hps_io_hps_io_emac0_inst_TXD1                          => CONNECTED_TO_hps_io_hps_io_emac0_inst_TXD1,                          --                                              .hps_io_emac0_inst_TXD1
			hps_io_hps_io_emac0_inst_TXD2                          => CONNECTED_TO_hps_io_hps_io_emac0_inst_TXD2,                          --                                              .hps_io_emac0_inst_TXD2
			hps_io_hps_io_emac0_inst_TXD3                          => CONNECTED_TO_hps_io_hps_io_emac0_inst_TXD3,                          --                                              .hps_io_emac0_inst_TXD3
			hps_io_hps_io_emac0_inst_RXD0                          => CONNECTED_TO_hps_io_hps_io_emac0_inst_RXD0,                          --                                              .hps_io_emac0_inst_RXD0
			hps_io_hps_io_emac0_inst_MDIO                          => CONNECTED_TO_hps_io_hps_io_emac0_inst_MDIO,                          --                                              .hps_io_emac0_inst_MDIO
			hps_io_hps_io_emac0_inst_MDC                           => CONNECTED_TO_hps_io_hps_io_emac0_inst_MDC,                           --                                              .hps_io_emac0_inst_MDC
			hps_io_hps_io_emac0_inst_RX_CTL                        => CONNECTED_TO_hps_io_hps_io_emac0_inst_RX_CTL,                        --                                              .hps_io_emac0_inst_RX_CTL
			hps_io_hps_io_emac0_inst_TX_CTL                        => CONNECTED_TO_hps_io_hps_io_emac0_inst_TX_CTL,                        --                                              .hps_io_emac0_inst_TX_CTL
			hps_io_hps_io_emac0_inst_RX_CLK                        => CONNECTED_TO_hps_io_hps_io_emac0_inst_RX_CLK,                        --                                              .hps_io_emac0_inst_RX_CLK
			hps_io_hps_io_emac0_inst_RXD1                          => CONNECTED_TO_hps_io_hps_io_emac0_inst_RXD1,                          --                                              .hps_io_emac0_inst_RXD1
			hps_io_hps_io_emac0_inst_RXD2                          => CONNECTED_TO_hps_io_hps_io_emac0_inst_RXD2,                          --                                              .hps_io_emac0_inst_RXD2
			hps_io_hps_io_emac0_inst_RXD3                          => CONNECTED_TO_hps_io_hps_io_emac0_inst_RXD3,                          --                                              .hps_io_emac0_inst_RXD3
			hps_io_hps_io_sdio_inst_CMD                            => CONNECTED_TO_hps_io_hps_io_sdio_inst_CMD,                            --                                              .hps_io_sdio_inst_CMD
			hps_io_hps_io_sdio_inst_D0                             => CONNECTED_TO_hps_io_hps_io_sdio_inst_D0,                             --                                              .hps_io_sdio_inst_D0
			hps_io_hps_io_sdio_inst_D1                             => CONNECTED_TO_hps_io_hps_io_sdio_inst_D1,                             --                                              .hps_io_sdio_inst_D1
			hps_io_hps_io_sdio_inst_D4                             => CONNECTED_TO_hps_io_hps_io_sdio_inst_D4,                             --                                              .hps_io_sdio_inst_D4
			hps_io_hps_io_sdio_inst_D5                             => CONNECTED_TO_hps_io_hps_io_sdio_inst_D5,                             --                                              .hps_io_sdio_inst_D5
			hps_io_hps_io_sdio_inst_D6                             => CONNECTED_TO_hps_io_hps_io_sdio_inst_D6,                             --                                              .hps_io_sdio_inst_D6
			hps_io_hps_io_sdio_inst_D7                             => CONNECTED_TO_hps_io_hps_io_sdio_inst_D7,                             --                                              .hps_io_sdio_inst_D7
			hps_io_hps_io_sdio_inst_CLK                            => CONNECTED_TO_hps_io_hps_io_sdio_inst_CLK,                            --                                              .hps_io_sdio_inst_CLK
			hps_io_hps_io_sdio_inst_D2                             => CONNECTED_TO_hps_io_hps_io_sdio_inst_D2,                             --                                              .hps_io_sdio_inst_D2
			hps_io_hps_io_sdio_inst_D3                             => CONNECTED_TO_hps_io_hps_io_sdio_inst_D3,                             --                                              .hps_io_sdio_inst_D3
			hps_io_hps_io_usb1_inst_D0                             => CONNECTED_TO_hps_io_hps_io_usb1_inst_D0,                             --                                              .hps_io_usb1_inst_D0
			hps_io_hps_io_usb1_inst_D1                             => CONNECTED_TO_hps_io_hps_io_usb1_inst_D1,                             --                                              .hps_io_usb1_inst_D1
			hps_io_hps_io_usb1_inst_D2                             => CONNECTED_TO_hps_io_hps_io_usb1_inst_D2,                             --                                              .hps_io_usb1_inst_D2
			hps_io_hps_io_usb1_inst_D3                             => CONNECTED_TO_hps_io_hps_io_usb1_inst_D3,                             --                                              .hps_io_usb1_inst_D3
			hps_io_hps_io_usb1_inst_D4                             => CONNECTED_TO_hps_io_hps_io_usb1_inst_D4,                             --                                              .hps_io_usb1_inst_D4
			hps_io_hps_io_usb1_inst_D5                             => CONNECTED_TO_hps_io_hps_io_usb1_inst_D5,                             --                                              .hps_io_usb1_inst_D5
			hps_io_hps_io_usb1_inst_D6                             => CONNECTED_TO_hps_io_hps_io_usb1_inst_D6,                             --                                              .hps_io_usb1_inst_D6
			hps_io_hps_io_usb1_inst_D7                             => CONNECTED_TO_hps_io_hps_io_usb1_inst_D7,                             --                                              .hps_io_usb1_inst_D7
			hps_io_hps_io_usb1_inst_CLK                            => CONNECTED_TO_hps_io_hps_io_usb1_inst_CLK,                            --                                              .hps_io_usb1_inst_CLK
			hps_io_hps_io_usb1_inst_STP                            => CONNECTED_TO_hps_io_hps_io_usb1_inst_STP,                            --                                              .hps_io_usb1_inst_STP
			hps_io_hps_io_usb1_inst_DIR                            => CONNECTED_TO_hps_io_hps_io_usb1_inst_DIR,                            --                                              .hps_io_usb1_inst_DIR
			hps_io_hps_io_usb1_inst_NXT                            => CONNECTED_TO_hps_io_hps_io_usb1_inst_NXT,                            --                                              .hps_io_usb1_inst_NXT
			hps_io_hps_io_uart0_inst_RX                            => CONNECTED_TO_hps_io_hps_io_uart0_inst_RX,                            --                                              .hps_io_uart0_inst_RX
			hps_io_hps_io_uart0_inst_TX                            => CONNECTED_TO_hps_io_hps_io_uart0_inst_TX,                            --                                              .hps_io_uart0_inst_TX
			hps_io_hps_io_gpio_inst_GPIO37                         => CONNECTED_TO_hps_io_hps_io_gpio_inst_GPIO37,                         --                                              .hps_io_gpio_inst_GPIO37
			hps_io_hps_io_gpio_inst_GPIO44                         => CONNECTED_TO_hps_io_hps_io_gpio_inst_GPIO44,                         --                                              .hps_io_gpio_inst_GPIO44
			hps_io_hps_io_gpio_inst_GPIO59                         => CONNECTED_TO_hps_io_hps_io_gpio_inst_GPIO59,                         --                                              .hps_io_gpio_inst_GPIO59
			i2c_master_d_conduit_end_sda                           => CONNECTED_TO_i2c_master_d_conduit_end_sda,                           --                      i2c_master_d_conduit_end.sda
			i2c_master_d_conduit_end_scl                           => CONNECTED_TO_i2c_master_d_conduit_end_scl,                           --                                              .scl
			i2c_master_f_conduit_end_sda                           => CONNECTED_TO_i2c_master_f_conduit_end_sda,                           --                      i2c_master_f_conduit_end.sda
			i2c_master_f_conduit_end_scl                           => CONNECTED_TO_i2c_master_f_conduit_end_scl,                           --                                              .scl
			i2c_master_is1_conduit_end_sda                         => CONNECTED_TO_i2c_master_is1_conduit_end_sda,                         --                    i2c_master_is1_conduit_end.sda
			i2c_master_is1_conduit_end_scl                         => CONNECTED_TO_i2c_master_is1_conduit_end_scl,                         --                                              .scl
			i2c_master_is2_conduit_end_sda                         => CONNECTED_TO_i2c_master_is2_conduit_end_sda,                         --                    i2c_master_is2_conduit_end.sda
			i2c_master_is2_conduit_end_scl                         => CONNECTED_TO_i2c_master_is2_conduit_end_scl,                         --                                              .scl
			i2c_master_is3_conduit_end_sda                         => CONNECTED_TO_i2c_master_is3_conduit_end_sda,                         --                    i2c_master_is3_conduit_end.sda
			i2c_master_is3_conduit_end_scl                         => CONNECTED_TO_i2c_master_is3_conduit_end_scl,                         --                                              .scl
			i2c_master_is4_conduit_end_sda                         => CONNECTED_TO_i2c_master_is4_conduit_end_sda,                         --                    i2c_master_is4_conduit_end.sda
			i2c_master_is4_conduit_end_scl                         => CONNECTED_TO_i2c_master_is4_conduit_end_scl,                         --                                              .scl
			i2c_master_p_conduit_end_sda                           => CONNECTED_TO_i2c_master_p_conduit_end_sda,                           --                      i2c_master_p_conduit_end.sda
			i2c_master_p_conduit_end_scl                           => CONNECTED_TO_i2c_master_p_conduit_end_scl,                           --                                              .scl
			memory_mem_a                                           => CONNECTED_TO_memory_mem_a,                                           --                                        memory.mem_a
			memory_mem_ba                                          => CONNECTED_TO_memory_mem_ba,                                          --                                              .mem_ba
			memory_mem_ck                                          => CONNECTED_TO_memory_mem_ck,                                          --                                              .mem_ck
			memory_mem_ck_n                                        => CONNECTED_TO_memory_mem_ck_n,                                        --                                              .mem_ck_n
			memory_mem_cke                                         => CONNECTED_TO_memory_mem_cke,                                         --                                              .mem_cke
			memory_mem_cs_n                                        => CONNECTED_TO_memory_mem_cs_n,                                        --                                              .mem_cs_n
			memory_mem_ras_n                                       => CONNECTED_TO_memory_mem_ras_n,                                       --                                              .mem_ras_n
			memory_mem_cas_n                                       => CONNECTED_TO_memory_mem_cas_n,                                       --                                              .mem_cas_n
			memory_mem_we_n                                        => CONNECTED_TO_memory_mem_we_n,                                        --                                              .mem_we_n
			memory_mem_reset_n                                     => CONNECTED_TO_memory_mem_reset_n,                                     --                                              .mem_reset_n
			memory_mem_dq                                          => CONNECTED_TO_memory_mem_dq,                                          --                                              .mem_dq
			memory_mem_dqs                                         => CONNECTED_TO_memory_mem_dqs,                                         --                                              .mem_dqs
			memory_mem_dqs_n                                       => CONNECTED_TO_memory_mem_dqs_n,                                       --                                              .mem_dqs_n
			memory_mem_odt                                         => CONNECTED_TO_memory_mem_odt,                                         --                                              .mem_odt
			memory_mem_dm                                          => CONNECTED_TO_memory_mem_dm,                                          --                                              .mem_dm
			memory_oct_rzqin                                       => CONNECTED_TO_memory_oct_rzqin,                                       --                                              .oct_rzqin
			nios2_qsys_0_cpu_resetrequest_conduit_cpu_resetrequest => CONNECTED_TO_nios2_qsys_0_cpu_resetrequest_conduit_cpu_resetrequest, --         nios2_qsys_0_cpu_resetrequest_conduit.cpu_resetrequest
			nios2_qsys_0_cpu_resetrequest_conduit_cpu_resettaken   => CONNECTED_TO_nios2_qsys_0_cpu_resetrequest_conduit_cpu_resettaken,   --                                              .cpu_resettaken
			pio_input_external_connection_export                   => CONNECTED_TO_pio_input_external_connection_export,                   --                 pio_input_external_connection.export
			pio_output_external_connection_export                  => CONNECTED_TO_pio_output_external_connection_export,                  --                pio_output_external_connection.export
			pio_reset_nios_external_connection_export              => CONNECTED_TO_pio_reset_nios_external_connection_export,              --            pio_reset_nios_external_connection.export
			pio_watchdog_cnt_external_connection_export            => CONNECTED_TO_pio_watchdog_cnt_external_connection_export,            --          pio_watchdog_cnt_external_connection.export
			pio_watchdog_freq_external_connection_export           => CONNECTED_TO_pio_watchdog_freq_external_connection_export,           --         pio_watchdog_freq_external_connection.export
			pll_0_locked_export                                    => CONNECTED_TO_pll_0_locked_export,                                    --                                  pll_0_locked.export
			pll_0_refclk_clk                                       => CONNECTED_TO_pll_0_refclk_clk,                                       --                                  pll_0_refclk.clk
			pll_0_reset_reset                                      => CONNECTED_TO_pll_0_reset_reset,                                      --                                   pll_0_reset.reset
			pll_0_sys_clk_clk                                      => CONNECTED_TO_pll_0_sys_clk_clk,                                      --                                 pll_0_sys_clk.clk
			pll_0_sys_reset_reset_n                                => CONNECTED_TO_pll_0_sys_reset_reset_n,                                --                               pll_0_sys_reset.reset_n
			qsys_reset_reset_n                                     => CONNECTED_TO_qsys_reset_reset_n,                                     --                                    qsys_reset.reset_n
			uart_cond_external_connection_rxd                      => CONNECTED_TO_uart_cond_external_connection_rxd,                      --                 uart_cond_external_connection.rxd
			uart_cond_external_connection_txd                      => CONNECTED_TO_uart_cond_external_connection_txd                       --                                              .txd
		);

