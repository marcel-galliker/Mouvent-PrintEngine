
module fluid_board_soc (
	avalon2fpga_slave_0_s2_writedata,
	avalon2fpga_slave_0_s2_read,
	avalon2fpga_slave_0_s2_write,
	avalon2fpga_slave_0_s2_address,
	avalon2fpga_slave_0_s2_waitrequest,
	avalon2fpga_slave_0_s2_readdata,
	avalon_spi_amc7891_1_conduit_end_sclk,
	avalon_spi_amc7891_1_conduit_end_cs_n,
	avalon_spi_amc7891_1_conduit_end_sdio,
	avalon_spi_amc7891_1_conduit_end_sdo,
	avalon_spi_max31865_0_conduit_end_0_drdy_n,
	avalon_spi_max31865_0_conduit_end_0_sclk,
	avalon_spi_max31865_0_conduit_end_0_cs_n,
	avalon_spi_max31865_0_conduit_end_0_sdi,
	avalon_spi_max31865_0_conduit_end_0_sdo,
	avalon_spi_max31865_1_conduit_end_0_drdy_n,
	avalon_spi_max31865_1_conduit_end_0_sclk,
	avalon_spi_max31865_1_conduit_end_0_cs_n,
	avalon_spi_max31865_1_conduit_end_0_sdi,
	avalon_spi_max31865_1_conduit_end_0_sdo,
	avalon_spi_max31865_2_conduit_end_0_drdy_n,
	avalon_spi_max31865_2_conduit_end_0_sclk,
	avalon_spi_max31865_2_conduit_end_0_cs_n,
	avalon_spi_max31865_2_conduit_end_0_sdi,
	avalon_spi_max31865_2_conduit_end_0_sdo,
	avalon_spi_max31865_3_conduit_end_0_drdy_n,
	avalon_spi_max31865_3_conduit_end_0_sclk,
	avalon_spi_max31865_3_conduit_end_0_cs_n,
	avalon_spi_max31865_3_conduit_end_0_sdi,
	avalon_spi_max31865_3_conduit_end_0_sdo,
	avalon_spi_max31865_4_conduit_end_0_drdy_n,
	avalon_spi_max31865_4_conduit_end_0_sclk,
	avalon_spi_max31865_4_conduit_end_0_cs_n,
	avalon_spi_max31865_4_conduit_end_0_sdi,
	avalon_spi_max31865_4_conduit_end_0_sdo,
	avalon_spi_max31865_5_conduit_end_0_drdy_n,
	avalon_spi_max31865_5_conduit_end_0_sclk,
	avalon_spi_max31865_5_conduit_end_0_cs_n,
	avalon_spi_max31865_5_conduit_end_0_sdi,
	avalon_spi_max31865_5_conduit_end_0_sdo,
	axi_lw_slave_register_0_conduit_end_0_wr_strb,
	axi_lw_slave_register_0_conduit_end_0_wr_valid,
	axi_lw_slave_register_0_conduit_end_0_rd_addr,
	axi_lw_slave_register_0_conduit_end_0_rd_data,
	axi_lw_slave_register_0_conduit_end_0_rd_valid,
	axi_lw_slave_register_0_conduit_end_0_rd_ready,
	axi_lw_slave_register_0_conduit_end_0_wr_data,
	axi_lw_slave_register_0_conduit_end_0_wr_addr,
	flush_pump_pwm_duty_cycle_external_connection_export,
	flush_pump_pwm_freq_external_connection_export,
	hps_0_uart1_cts,
	hps_0_uart1_dsr,
	hps_0_uart1_dcd,
	hps_0_uart1_ri,
	hps_0_uart1_dtr,
	hps_0_uart1_rts,
	hps_0_uart1_out1_n,
	hps_0_uart1_out2_n,
	hps_0_uart1_rxd,
	hps_0_uart1_txd,
	hps_io_hps_io_emac0_inst_TX_CLK,
	hps_io_hps_io_emac0_inst_TXD0,
	hps_io_hps_io_emac0_inst_TXD1,
	hps_io_hps_io_emac0_inst_TXD2,
	hps_io_hps_io_emac0_inst_TXD3,
	hps_io_hps_io_emac0_inst_RXD0,
	hps_io_hps_io_emac0_inst_MDIO,
	hps_io_hps_io_emac0_inst_MDC,
	hps_io_hps_io_emac0_inst_RX_CTL,
	hps_io_hps_io_emac0_inst_TX_CTL,
	hps_io_hps_io_emac0_inst_RX_CLK,
	hps_io_hps_io_emac0_inst_RXD1,
	hps_io_hps_io_emac0_inst_RXD2,
	hps_io_hps_io_emac0_inst_RXD3,
	hps_io_hps_io_sdio_inst_CMD,
	hps_io_hps_io_sdio_inst_D0,
	hps_io_hps_io_sdio_inst_D1,
	hps_io_hps_io_sdio_inst_D4,
	hps_io_hps_io_sdio_inst_D5,
	hps_io_hps_io_sdio_inst_D6,
	hps_io_hps_io_sdio_inst_D7,
	hps_io_hps_io_sdio_inst_CLK,
	hps_io_hps_io_sdio_inst_D2,
	hps_io_hps_io_sdio_inst_D3,
	hps_io_hps_io_usb1_inst_D0,
	hps_io_hps_io_usb1_inst_D1,
	hps_io_hps_io_usb1_inst_D2,
	hps_io_hps_io_usb1_inst_D3,
	hps_io_hps_io_usb1_inst_D4,
	hps_io_hps_io_usb1_inst_D5,
	hps_io_hps_io_usb1_inst_D6,
	hps_io_hps_io_usb1_inst_D7,
	hps_io_hps_io_usb1_inst_CLK,
	hps_io_hps_io_usb1_inst_STP,
	hps_io_hps_io_usb1_inst_DIR,
	hps_io_hps_io_usb1_inst_NXT,
	hps_io_hps_io_uart0_inst_RX,
	hps_io_hps_io_uart0_inst_TX,
	hps_io_hps_io_gpio_inst_GPIO37,
	hps_io_hps_io_gpio_inst_GPIO44,
	hps_io_hps_io_gpio_inst_GPIO59,
	i2c_master_d_conduit_end_sda,
	i2c_master_d_conduit_end_scl,
	i2c_master_f_conduit_end_sda,
	i2c_master_f_conduit_end_scl,
	i2c_master_is1_conduit_end_sda,
	i2c_master_is1_conduit_end_scl,
	i2c_master_is2_conduit_end_sda,
	i2c_master_is2_conduit_end_scl,
	i2c_master_is3_conduit_end_sda,
	i2c_master_is3_conduit_end_scl,
	i2c_master_is4_conduit_end_sda,
	i2c_master_is4_conduit_end_scl,
	i2c_master_p_conduit_end_sda,
	i2c_master_p_conduit_end_scl,
	memory_mem_a,
	memory_mem_ba,
	memory_mem_ck,
	memory_mem_ck_n,
	memory_mem_cke,
	memory_mem_cs_n,
	memory_mem_ras_n,
	memory_mem_cas_n,
	memory_mem_we_n,
	memory_mem_reset_n,
	memory_mem_dq,
	memory_mem_dqs,
	memory_mem_dqs_n,
	memory_mem_odt,
	memory_mem_dm,
	memory_oct_rzqin,
	nios2_qsys_0_cpu_resetrequest_conduit_cpu_resetrequest,
	nios2_qsys_0_cpu_resetrequest_conduit_cpu_resettaken,
	pio_input_external_connection_export,
	pio_output_external_connection_export,
	pio_reset_nios_external_connection_export,
	pio_watchdog_cnt_external_connection_export,
	pio_watchdog_freq_external_connection_export,
	pll_0_locked_export,
	pll_0_refclk_clk,
	pll_0_reset_reset,
	pll_0_sys_clk_clk,
	pll_0_sys_reset_reset_n,
	qsys_reset_reset_n,
	uart_cond_external_connection_rxd,
	uart_cond_external_connection_txd);	

	output	[15:0]	avalon2fpga_slave_0_s2_writedata;
	output		avalon2fpga_slave_0_s2_read;
	output		avalon2fpga_slave_0_s2_write;
	output	[6:0]	avalon2fpga_slave_0_s2_address;
	input		avalon2fpga_slave_0_s2_waitrequest;
	input	[15:0]	avalon2fpga_slave_0_s2_readdata;
	output		avalon_spi_amc7891_1_conduit_end_sclk;
	output		avalon_spi_amc7891_1_conduit_end_cs_n;
	output		avalon_spi_amc7891_1_conduit_end_sdio;
	input		avalon_spi_amc7891_1_conduit_end_sdo;
	input		avalon_spi_max31865_0_conduit_end_0_drdy_n;
	output		avalon_spi_max31865_0_conduit_end_0_sclk;
	output		avalon_spi_max31865_0_conduit_end_0_cs_n;
	output		avalon_spi_max31865_0_conduit_end_0_sdi;
	input		avalon_spi_max31865_0_conduit_end_0_sdo;
	input		avalon_spi_max31865_1_conduit_end_0_drdy_n;
	output		avalon_spi_max31865_1_conduit_end_0_sclk;
	output		avalon_spi_max31865_1_conduit_end_0_cs_n;
	output		avalon_spi_max31865_1_conduit_end_0_sdi;
	input		avalon_spi_max31865_1_conduit_end_0_sdo;
	input		avalon_spi_max31865_2_conduit_end_0_drdy_n;
	output		avalon_spi_max31865_2_conduit_end_0_sclk;
	output		avalon_spi_max31865_2_conduit_end_0_cs_n;
	output		avalon_spi_max31865_2_conduit_end_0_sdi;
	input		avalon_spi_max31865_2_conduit_end_0_sdo;
	input		avalon_spi_max31865_3_conduit_end_0_drdy_n;
	output		avalon_spi_max31865_3_conduit_end_0_sclk;
	output		avalon_spi_max31865_3_conduit_end_0_cs_n;
	output		avalon_spi_max31865_3_conduit_end_0_sdi;
	input		avalon_spi_max31865_3_conduit_end_0_sdo;
	input		avalon_spi_max31865_4_conduit_end_0_drdy_n;
	output		avalon_spi_max31865_4_conduit_end_0_sclk;
	output		avalon_spi_max31865_4_conduit_end_0_cs_n;
	output		avalon_spi_max31865_4_conduit_end_0_sdi;
	input		avalon_spi_max31865_4_conduit_end_0_sdo;
	input		avalon_spi_max31865_5_conduit_end_0_drdy_n;
	output		avalon_spi_max31865_5_conduit_end_0_sclk;
	output		avalon_spi_max31865_5_conduit_end_0_cs_n;
	output		avalon_spi_max31865_5_conduit_end_0_sdi;
	input		avalon_spi_max31865_5_conduit_end_0_sdo;
	output	[3:0]	axi_lw_slave_register_0_conduit_end_0_wr_strb;
	output		axi_lw_slave_register_0_conduit_end_0_wr_valid;
	output	[15:0]	axi_lw_slave_register_0_conduit_end_0_rd_addr;
	input	[31:0]	axi_lw_slave_register_0_conduit_end_0_rd_data;
	output		axi_lw_slave_register_0_conduit_end_0_rd_valid;
	input		axi_lw_slave_register_0_conduit_end_0_rd_ready;
	output	[31:0]	axi_lw_slave_register_0_conduit_end_0_wr_data;
	output	[15:0]	axi_lw_slave_register_0_conduit_end_0_wr_addr;
	output	[31:0]	flush_pump_pwm_duty_cycle_external_connection_export;
	output	[31:0]	flush_pump_pwm_freq_external_connection_export;
	input		hps_0_uart1_cts;
	input		hps_0_uart1_dsr;
	input		hps_0_uart1_dcd;
	input		hps_0_uart1_ri;
	output		hps_0_uart1_dtr;
	output		hps_0_uart1_rts;
	output		hps_0_uart1_out1_n;
	output		hps_0_uart1_out2_n;
	input		hps_0_uart1_rxd;
	output		hps_0_uart1_txd;
	output		hps_io_hps_io_emac0_inst_TX_CLK;
	output		hps_io_hps_io_emac0_inst_TXD0;
	output		hps_io_hps_io_emac0_inst_TXD1;
	output		hps_io_hps_io_emac0_inst_TXD2;
	output		hps_io_hps_io_emac0_inst_TXD3;
	input		hps_io_hps_io_emac0_inst_RXD0;
	inout		hps_io_hps_io_emac0_inst_MDIO;
	output		hps_io_hps_io_emac0_inst_MDC;
	input		hps_io_hps_io_emac0_inst_RX_CTL;
	output		hps_io_hps_io_emac0_inst_TX_CTL;
	input		hps_io_hps_io_emac0_inst_RX_CLK;
	input		hps_io_hps_io_emac0_inst_RXD1;
	input		hps_io_hps_io_emac0_inst_RXD2;
	input		hps_io_hps_io_emac0_inst_RXD3;
	inout		hps_io_hps_io_sdio_inst_CMD;
	inout		hps_io_hps_io_sdio_inst_D0;
	inout		hps_io_hps_io_sdio_inst_D1;
	inout		hps_io_hps_io_sdio_inst_D4;
	inout		hps_io_hps_io_sdio_inst_D5;
	inout		hps_io_hps_io_sdio_inst_D6;
	inout		hps_io_hps_io_sdio_inst_D7;
	output		hps_io_hps_io_sdio_inst_CLK;
	inout		hps_io_hps_io_sdio_inst_D2;
	inout		hps_io_hps_io_sdio_inst_D3;
	inout		hps_io_hps_io_usb1_inst_D0;
	inout		hps_io_hps_io_usb1_inst_D1;
	inout		hps_io_hps_io_usb1_inst_D2;
	inout		hps_io_hps_io_usb1_inst_D3;
	inout		hps_io_hps_io_usb1_inst_D4;
	inout		hps_io_hps_io_usb1_inst_D5;
	inout		hps_io_hps_io_usb1_inst_D6;
	inout		hps_io_hps_io_usb1_inst_D7;
	input		hps_io_hps_io_usb1_inst_CLK;
	output		hps_io_hps_io_usb1_inst_STP;
	input		hps_io_hps_io_usb1_inst_DIR;
	input		hps_io_hps_io_usb1_inst_NXT;
	input		hps_io_hps_io_uart0_inst_RX;
	output		hps_io_hps_io_uart0_inst_TX;
	inout		hps_io_hps_io_gpio_inst_GPIO37;
	inout		hps_io_hps_io_gpio_inst_GPIO44;
	inout		hps_io_hps_io_gpio_inst_GPIO59;
	inout		i2c_master_d_conduit_end_sda;
	inout		i2c_master_d_conduit_end_scl;
	inout		i2c_master_f_conduit_end_sda;
	inout		i2c_master_f_conduit_end_scl;
	inout		i2c_master_is1_conduit_end_sda;
	inout		i2c_master_is1_conduit_end_scl;
	inout		i2c_master_is2_conduit_end_sda;
	inout		i2c_master_is2_conduit_end_scl;
	inout		i2c_master_is3_conduit_end_sda;
	inout		i2c_master_is3_conduit_end_scl;
	inout		i2c_master_is4_conduit_end_sda;
	inout		i2c_master_is4_conduit_end_scl;
	inout		i2c_master_p_conduit_end_sda;
	inout		i2c_master_p_conduit_end_scl;
	output	[14:0]	memory_mem_a;
	output	[2:0]	memory_mem_ba;
	output		memory_mem_ck;
	output		memory_mem_ck_n;
	output		memory_mem_cke;
	output		memory_mem_cs_n;
	output		memory_mem_ras_n;
	output		memory_mem_cas_n;
	output		memory_mem_we_n;
	output		memory_mem_reset_n;
	inout	[31:0]	memory_mem_dq;
	inout	[3:0]	memory_mem_dqs;
	inout	[3:0]	memory_mem_dqs_n;
	output		memory_mem_odt;
	output	[3:0]	memory_mem_dm;
	input		memory_oct_rzqin;
	input		nios2_qsys_0_cpu_resetrequest_conduit_cpu_resetrequest;
	output		nios2_qsys_0_cpu_resetrequest_conduit_cpu_resettaken;
	input	[14:0]	pio_input_external_connection_export;
	output	[15:0]	pio_output_external_connection_export;
	output		pio_reset_nios_external_connection_export;
	output	[31:0]	pio_watchdog_cnt_external_connection_export;
	output	[31:0]	pio_watchdog_freq_external_connection_export;
	output		pll_0_locked_export;
	input		pll_0_refclk_clk;
	input		pll_0_reset_reset;
	output		pll_0_sys_clk_clk;
	output		pll_0_sys_reset_reset_n;
	input		qsys_reset_reset_n;
	input		uart_cond_external_connection_rxd;
	output		uart_cond_external_connection_txd;
endmodule
