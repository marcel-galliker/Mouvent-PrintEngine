#include <stdio.h>
#include "alt_types.h"
#include "sys/alt_irq.h"
#include "altera_avalon_timer.h"
#include "altera_avalon_timer_regs.h"
#include "altera_avalon_pio_regs.h"
#include <unistd.h>
#include "sys/alt_stdio.h"
#include "system.h"
#include "io.h"
#include "sys/alt_alarm.h"
#include "AMC7891.h"
#include "fpga_def_fluid.h"

int init_AMC7891(int base)
{
	int i;
	int readback=0;

	// OLD: FluidBoard (AVALON_SPI_AMC7891_0_BASE) + HeaterBoard (AVALON_SPI_AMC7891_1_BASE)
	// NEW: FluidBoard (AVALON2FPGA_SLAVE_0_BASE)  + HeaterBoard (AVALON_SPI_AMC7891_1_BASE)

	if (base == AVALON2FPGA_SLAVE_0_BASE) {
		IOWR_16DIRECT(AXI_LW_SLAVE_REGISTER_0_BASE, ENABLE_RDWR, 0);

		// delay to make sure bit is set correctly
		for (i = 0; i < 100; i++)
			;
	}

	// Writing 0x6600 to this register forces a reset operation.
	// During reset, all SPI communication is blocked.
	// After issuing the reset, there is a wait of at least 30 us before communication can be resumed.
	IOWR_16DIRECT(base,AMC7891_AMC_RESET,0x6600);

	readback=IORD_16DIRECT(base, AMC7891_AMC_ID);	//first call is not valid
	readback=IORD_16DIRECT(base, AMC7891_AMC_ID);

	if(readback!=AMC_ID_VALUE)	//check AMC7891 ID
	{
		return (1);				// AMC SPI COMMUNICATION ERROR
	}

	// pre AXI
	/*if(base==AVALON_SPI_AMC7891_0_BASE)			//FluidBoard
	{
		IOWR_16DIRECT(AVALON_SPI_AMC7891_0_BASE,AMC7891_AMC_POWER,0x7E00);		// ADC On / Ref On / DAC 0-3 On
		IOWR_16DIRECT(AVALON_SPI_AMC7891_0_BASE,AMC7891_GPIO_CONFIG,0x00FF);	// I/O definition 0->In / 1->Out
		IOWR_16DIRECT(AVALON_SPI_AMC7891_0_BASE,AMC7891_ADC_ENABLE,0x6D00);		// ADC 0-4 enabled
		IOWR_16DIRECT(AVALON_SPI_AMC7891_0_BASE,AMC7891_ADC_GAIN,0xF800);		// ADC 0-4 gain x 2
		IOWR_16DIRECT(AVALON_SPI_AMC7891_0_BASE,AMC7891_AMC_CONFIG,0x3000);		// ADC Trigger
	}*/
	if(base == AVALON2FPGA_SLAVE_0_BASE)			//FluidBoard
	{
		IOWR_16DIRECT(AVALON2FPGA_SLAVE_0_BASE, AMC7891_AMC_POWER,   0x7E00); // ADC On / Ref On / DAC 0-3 On
		IOWR_16DIRECT(AVALON2FPGA_SLAVE_0_BASE, AMC7891_GPIO_CONFIG, 0x00FF); // I/O definition 0->In / 1->Out
		IOWR_16DIRECT(AVALON2FPGA_SLAVE_0_BASE, AMC7891_ADC_ENABLE,  0x6D00); // ADC 0-4 enabled
		IOWR_16DIRECT(AVALON2FPGA_SLAVE_0_BASE, AMC7891_ADC_GAIN,    0xF800); // ADC 0-4 gain x 2
		IOWR_16DIRECT(AVALON2FPGA_SLAVE_0_BASE, AMC7891_AMC_CONFIG,  0x3000); // ADC Trigger

		// Give control of AMC7891 to FPGA
		IOWR_16DIRECT(AXI_LW_SLAVE_REGISTER_0_BASE, ENABLE_RDWR, 1);

		// Set threshold values for
		IOWR_32DIRECT(AXI_LW_SLAVE_REGISTER_0_BASE, ADC_THRESH_0, 400);
		IOWR_16DIRECT(AXI_LW_SLAVE_REGISTER_0_BASE, ADC_THRESH_1, 400);
		IOWR_16DIRECT(AXI_LW_SLAVE_REGISTER_0_BASE, ADC_THRESH_2, 400);
		IOWR_16DIRECT(AXI_LW_SLAVE_REGISTER_0_BASE, ADC_THRESH_3, 400);
		IOWR_16DIRECT(AXI_LW_SLAVE_REGISTER_0_BASE, ADC_THRESH_4, 400);
		IOWR_16DIRECT(AXI_LW_SLAVE_REGISTER_0_BASE, ADC_THRESH_5, 400);
		IOWR_16DIRECT(AXI_LW_SLAVE_REGISTER_0_BASE, ADC_THRESH_6, 400);
		IOWR_16DIRECT(AXI_LW_SLAVE_REGISTER_0_BASE, ADC_THRESH_7, 400);

	}
	else if(base==AVALON_SPI_AMC7891_1_BASE)	//HeaterBoard
	{
		IOWR_16DIRECT(AVALON_SPI_AMC7891_1_BASE,AMC7891_AMC_POWER,	0x6000);	// ADC On / Ref On / DAC Off
		IOWR_16DIRECT(AVALON_SPI_AMC7891_1_BASE,AMC7891_GPIO_CONFIG,0x0FF);		// I/O definition  0->In / 1->Out
		IOWR_16DIRECT(AVALON_SPI_AMC7891_1_BASE,AMC7891_ADC_ENABLE,	0x6DE0);	// ADC 0 - 7 enabled
		IOWR_16DIRECT(AVALON_SPI_AMC7891_1_BASE,AMC7891_ADC_GAIN,	0xFF00);	// ADC 0 - 7 gain x 2
		IOWR_16DIRECT(AVALON_SPI_AMC7891_1_BASE,AMC7891_AMC_CONFIG,	0x3000);	// ADC Trigger
	}
	else
	{
		return (2);				//not supported AMC BASE ADR.
	}

	return 0;
}
