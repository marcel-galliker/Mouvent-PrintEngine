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

int init_AMC7891(void)
{
	alt_u16 readback=0;

	readback=IORD_16DIRECT(AMC7891_0_BASE, AMC7891_AMC_ID);	//first call is not valid
	readback=IORD_16DIRECT(AMC7891_0_BASE, AMC7891_AMC_ID);
	if(readback!=AMC_ID_VALUE)	//check AMC7891 ID
	{
		return (1);
	}

	// Head GPIO
	IOWR_16DIRECT(AMC7891_0_BASE,AMC7891_AMC_RESET,0x6600);
	IOWR_16DIRECT(AMC7891_0_BASE,AMC7891_AMC_POWER,0x7000);		// Ref On, ADC On, DAC 0 on
	IOWR_16DIRECT(AMC7891_0_BASE, AMC7891_DAC0_DATA, 0);
	IOWR_16DIRECT(AMC7891_0_BASE,AMC7891_GPIO_CONFIG,0x0FFF);	// I/O definition
	IOWR_16DIRECT(AMC7891_0_BASE,AMC7891_ADC_ENABLE,0x6DE0);	// all ADC enabled
	IOWR_16DIRECT(AMC7891_0_BASE,AMC7891_ADC_GAIN,0xF000);		// ADC gain x 2 (all without head Thermistor)
	IOWR_16DIRECT(AMC7891_0_BASE,AMC7891_AMC_CONFIG,0x3000);	// ADC Trigger
/*
	readback=IORD_16DIRECT(AMC7891_0_BASE, AMC7891_AMC_POWER);
	readback=IORD_16DIRECT(AMC7891_0_BASE, AMC7891_AMC_POWER);
	readback=IORD_16DIRECT(AMC7891_0_BASE, AMC7891_GPIO_CONFIG);
	readback=IORD_16DIRECT(AMC7891_0_BASE, AMC7891_GPIO_CONFIG);
	readback=IORD_16DIRECT(AMC7891_0_BASE, AMC7891_ADC_ENABLE);
	readback=IORD_16DIRECT(AMC7891_0_BASE, AMC7891_ADC_ENABLE);
	readback=IORD_16DIRECT(AMC7891_0_BASE, AMC7891_ADC_GAIN);
	readback=IORD_16DIRECT(AMC7891_0_BASE, AMC7891_ADC_GAIN);
	readback=IORD_16DIRECT(AMC7891_0_BASE, AMC7891_AMC_CONFIG);
	readback=IORD_16DIRECT(AMC7891_0_BASE, AMC7891_AMC_CONFIG);*/
	return (0);
}

