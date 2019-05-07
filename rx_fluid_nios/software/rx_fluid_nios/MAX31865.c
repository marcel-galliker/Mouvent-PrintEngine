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
#include "MAX31865.h"

void init_MAX31865(void)
{
	int		count;

	for(count = 0 ; count < MAX_PT100 ; count++)
	{
		IOWR_8DIRECT(AVALON_SPI_MAX31865_0_BASE + count*0x10, MAX31865_CONFIG, 0xC1);				// Configuration vbias='1'; conversion auto ='1'; 50Hz='1'; => 1100 0001
//		IOWR_8DIRECT(AVALON_SPI_MAX31865_0_BASE + count*0x10, MAX31865_CONFIG, 0xD1);				// Configuration vbias='1'; conversion auto ='1'; two_wires='1'; 50Hz='1'; => 1101 0001
		IOWR_8DIRECT(AVALON_SPI_MAX31865_0_BASE + count*0x10, MAX31865_FAULT_HTHR_MSB, 0xFF);		// High Fault Threshold
		IOWR_8DIRECT(AVALON_SPI_MAX31865_0_BASE + count*0x10, MAX31865_FAULT_HTHR_LSB, 0xFF);		// High Fault Threshold
		IOWR_8DIRECT(AVALON_SPI_MAX31865_0_BASE + count*0x10, MAX31865_FAULT_LTHR_MSB, 0x00);		// Low Fault Threshold
		IOWR_8DIRECT(AVALON_SPI_MAX31865_0_BASE + count*0x10, MAX31865_FAULT_LTHR_LSB, 0x00);		// Low Fault Threshold
	}
}
