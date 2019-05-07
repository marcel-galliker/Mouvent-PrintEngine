/******************************************************************************/
/** heater.h
 **
 ** heater board
 **
 **	Copyright 2016 by radex AG, Switzerland. All rights reserved.
 **	Written by Stefan Weber
 **
 ******************************************************************************/

//--- includes ---------------------------------------------------------------
#include <stdio.h>
#include <string.h>
#include "system.h"
#include "AMC7891.h"
#include "altera_avalon_pio_regs.h"
#include "rx_ink_common.h"
#include "nios_def_fluid.h"
#include "trprintf.h"
#include "pio.h"
#include "heater.h"
#include "adc_thermistor.h"

//--- defines --------------------------------------------
#define HEATER_0_ON 	0x0001
#define HEATER_1_ON 	0x0008
#define HEATER_2_ON 	0x0004
#define HEATER_3_ON 	0x0002
#define HEATER_LED_ON 	0x0010

#define HEATER_ANALOG_COUNT		8

//--- statics -----------------------------------------------

//--- prototypes ----------------------------------
static void _set_heater_out(int i, int newState);


//--- init_heater --------------------------------------------
void heater_init(void)
{
	//int count, temp[HEATER_ANALOG_COUNT];
	IOWR_16DIRECT(AVALON_SPI_AMC7891_1_BASE, AMC7891_GPIO_OUT, HEATER_LED_ON); // All Heater off, LED on
	// Board Test

	/*
	IOWR_16DIRECT(AVALON_SPI_AMC7891_1_BASE,AMC7891_GPIO_OUT,HEATER_LED_ON | HEATER_0_ON);	// Heater 0 on, LED on
	IOWR_16DIRECT(AVALON_SPI_AMC7891_1_BASE,AMC7891_GPIO_OUT,HEATER_LED_ON | HEATER_1_ON);	// Heater 1 on, LED on
	IOWR_16DIRECT(AVALON_SPI_AMC7891_1_BASE,AMC7891_GPIO_OUT,HEATER_LED_ON | HEATER_2_ON);	// Heater 2 on, LED on
	IOWR_16DIRECT(AVALON_SPI_AMC7891_1_BASE,AMC7891_GPIO_OUT,HEATER_LED_ON | HEATER_3_ON);	// Heater 3 on, LED on
	IOWR_16DIRECT(AVALON_SPI_AMC7891_1_BASE,AMC7891_GPIO_OUT,HEATER_LED_ON);				// All Heater off, LED on

	// Thermistor Heaterboard
	for(count = 0 ; count < HEATER_ANALOG_COUNT ; count++)
	{
		temp[count] = IORD_16DIRECT(AVALON_SPI_AMC7891_1_BASE,AMC7891_ADC0_DATA + count) & 0x3ff;
		temp[count] = IORD_16DIRECT(AVALON_SPI_AMC7891_1_BASE,AMC7891_ADC0_DATA + count) & 0x3ff;
	}*/
	int i;
	for (i=0; i<NIOS_INK_SUPPLY_CNT; i++)
	{
		_set_heater_out(i, FALSE);
	}
}

//--- heater_tick_10ms ---------------------------------------------------------------
void heater_tick_10ms(void)
{
	int 		i;
	alt_u16		temp;
	alt_u16 	heater_gpio_in;

	heater_gpio_in = IORD_16DIRECT(AVALON_SPI_AMC7891_1_BASE, AMC7891_GPIO_IN) & 0xfff;
	heater_gpio_in = IORD_16DIRECT(AVALON_SPI_AMC7891_1_BASE, AMC7891_GPIO_IN) & 0xfff;

	pRX_Status->heater_amc_temp = IORD_16DIRECT(AVALON_SPI_AMC7891_1_BASE, AMC7891_TEMP_DATA) & 0x3ff;
	pRX_Status->heater_amc_temp = IORD_16DIRECT(AVALON_SPI_AMC7891_1_BASE, AMC7891_TEMP_DATA) & 0x3ff;

	if (pRX_Status->info.is_shutdown)
	{
		for(i=0 ; i<NIOS_INK_SUPPLY_CNT ; i++) _set_heater_out(i, FALSE);
		return;
	}

	// Thermistor Heaterboard
	for(i=0; i<NIOS_INK_SUPPLY_CNT; i++)
	{
		if (((~heater_gpio_in)>>(8+i)) & 0x01)
		{
			pRX_Status->ink_supply[i].error |= err_heater_board;
			pRX_Status->ink_supply[i].heaterTemp = INVALID_VALUE;
		}
		else
		{
			// Read Thermistor
			temp = IORD_16DIRECT(AVALON_SPI_AMC7891_1_BASE, AMC7891_ADC7_DATA -(2*i)) & 0x3ff;
			temp = IORD_16DIRECT(AVALON_SPI_AMC7891_1_BASE, AMC7891_ADC7_DATA -(2*i)) & 0x3ff;
			//temp = IORD_16DIRECT(AXI_LW_SLAVE_REGISTER_0_BASE, AMC7891_ADC7_DATA -(2*i)) & 0x3ff;
			//temp = IORD_16DIRECT(AXI_LW_SLAVE_REGISTER_0_BASE, AMC7891_ADC7_DATA -(2*i)) & 0x3ff;

			// Convert to Temperature
			pRX_Status->ink_supply[i].heaterTemp = Main_adc_polling(temp);
		}

		if(pRX_Status->ink_supply[i].heaterTemp != INVALID_VALUE && pRX_Config->ink_supply[i].ctrl_mode > ctrl_off)
		{
			if (pRX_Status->ink_supply[i].heaterTemp >= pRX_Config->ink_supply[i].heaterTempMax)
				_set_heater_out(i, FALSE);
			else if (pRX_Config->ink_supply[i].headTemp >= pRX_Config->ink_supply[i].heaterTemp)
				_set_heater_out(i, FALSE);
			else if (pRX_Config->ink_supply[i].condPumpFeedback)
				_set_heater_out(i, TRUE);
			else
				_set_heater_out(i, FALSE);
		}
		else _set_heater_out(i, FALSE);
	}
}

//--- _set_heater_out ----------------------------------------
static void _set_heater_out(int heaterNo, int newState)
{
	volatile int readin;

//	if (newState!=pRX_Status->ink_supply[heaterNo].heaterOn)
	{
		pRX_Status->ink_supply[heaterNo].heaterOn = newState;
		readin=IORD_16DIRECT(AVALON_SPI_AMC7891_1_BASE,AMC7891_GPIO_OUT) & 0xfff;
		readin=IORD_16DIRECT(AVALON_SPI_AMC7891_1_BASE,AMC7891_GPIO_OUT) & 0xfff;
		int out;
		switch (heaterNo)
		{
		case 0:	out = HEATER_0_ON; break;
		case 1:	out = HEATER_1_ON; break;
		case 2: out = HEATER_2_ON; break;
		case 3: out = HEATER_3_ON; break;
		}
		if (newState) IOWR_16DIRECT(AVALON_SPI_AMC7891_1_BASE,AMC7891_GPIO_OUT,(readin | out));	// set ON
		else		  IOWR_16DIRECT(AVALON_SPI_AMC7891_1_BASE,AMC7891_GPIO_OUT,(readin &~out));	// set OFF
	}
}

//--- heater_tick_1000ms -----------------------------------------------------------
void heater_tick_1000ms(void)
{
	IOWR_16DIRECT(AVALON_SPI_AMC7891_1_BASE,AMC7891_GPIO_OUT, 0);	// set All Heaters OFF (Security)
}

