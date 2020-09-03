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
#include "pid.h"

//--- defines --------------------------------------------
#define HEATER_0_ON 	0x0001
#define HEATER_1_ON 	0x0008
#define HEATER_2_ON 	0x0004
#define HEATER_3_ON 	0x0002
#define HEATER_LED_ON 	0x0010

#define HEATER_ANALOG_COUNT		8

#define HEATER_TEMP_TOLERANCE	1000

#define VOLTAGES_AVG_NB		50

//--- statics -----------------------------------------------
SPID_par	_pid_Temp[NIOS_INK_SUPPLY_CNT];			// PID temperature heater
static int 	_DutyTemp_Count[NIOS_INK_SUPPLY_CNT];
static int 	_Temp_Tab[NIOS_INK_SUPPLY_CNT][100];
static int 	_Temp_Average[NIOS_INK_SUPPLY_CNT];
static int 	_Temp_Inc[NIOS_INK_SUPPLY_CNT];

//--- CEDRIC --- Error Temperature frozen : Auto-reset + yellow message
static int 	_Temp_Pre[NIOS_INK_SUPPLY_CNT];
static int 	_Heater_Pre[NIOS_INK_SUPPLY_CNT];
static int 	_TimeTempFrozen[NIOS_INK_SUPPLY_CNT];

static int _PS3V_Sum;
static int _PS5V_Sum;
static int _PS24V_Sum;
static int _PS24VP_Sum;
static int _PS_inc;

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

	// initiliazes voltages to theoric values and so avoid error log message
	_PS_inc = 0;
	pRX_Status->HeaterBoard_Vsupply_3V 		= 3300;
	pRX_Status->HeaterBoard_Vsupply_5V 		= 5000;
	pRX_Status->HeaterBoard_Vsupply_24V 	= 24000;
	pRX_Status->HeaterBoard_Vsupply_24VP 	= 24000;
}

//--- heater_tick_10ms ---------------------------------------------------------------
void heater_tick_10ms(void)
{
	int 		i,j;
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

	// Vsupply HeaterBoard
	if(_PS_inc > VOLTAGES_AVG_NB - 1) _PS_inc = 0;
	temp = IORD_16DIRECT(AVALON_SPI_AMC7891_1_BASE, AMC7891_ADC0_DATA) & 0x3ff;
	temp = IORD_16DIRECT(AVALON_SPI_AMC7891_1_BASE, AMC7891_ADC0_DATA) & 0x3ff;
	_PS3V_Sum += temp * 5000 / 1024;

	temp = IORD_16DIRECT(AVALON_SPI_AMC7891_1_BASE, AMC7891_ADC0_DATA + 2) & 0x3ff;
	temp = IORD_16DIRECT(AVALON_SPI_AMC7891_1_BASE, AMC7891_ADC0_DATA + 2) & 0x3ff;
	_PS5V_Sum += temp * 5000 / 1024;

	temp = IORD_16DIRECT(AVALON_SPI_AMC7891_1_BASE, AMC7891_ADC0_DATA + 4) & 0x3ff;
	temp = IORD_16DIRECT(AVALON_SPI_AMC7891_1_BASE, AMC7891_ADC0_DATA + 4) & 0x3ff;
	_PS24V_Sum += temp * 5000 / 1024 * 115 / 15;

	temp = IORD_16DIRECT(AVALON_SPI_AMC7891_1_BASE, AMC7891_ADC0_DATA + 6) & 0x3ff;
	temp = IORD_16DIRECT(AVALON_SPI_AMC7891_1_BASE, AMC7891_ADC0_DATA + 6) & 0x3ff;
	_PS24VP_Sum += temp * 5000 / 1024 * 115 / 15;

	if(++_PS_inc >= VOLTAGES_AVG_NB)
	{
		pRX_Status->HeaterBoard_Vsupply_3V 	= _PS3V_Sum / VOLTAGES_AVG_NB;
		pRX_Status->HeaterBoard_Vsupply_5V 	= _PS5V_Sum / VOLTAGES_AVG_NB;
		pRX_Status->HeaterBoard_Vsupply_24V = _PS24V_Sum / VOLTAGES_AVG_NB;
		pRX_Status->HeaterBoard_Vsupply_24VP= _PS24VP_Sum / VOLTAGES_AVG_NB;
		_PS3V_Sum = 0;
		_PS5V_Sum = 0;
		_PS24V_Sum = 0;
		_PS24VP_Sum = 0;
		_PS_inc = 0;
	}

	// Thermistor Heaterboard
	for(i=0; i<NIOS_INK_SUPPLY_CNT; i++)
	{
		if (((~heater_gpio_in)>>(8+i)) & 0x01)
		{
			if(pRX_Config->ink_supply[i].heaterTempMax > 39000) pRX_Status->ink_supply[i].error |= err_heater_board;
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
			// pRX_Status->ink_supply[i].heaterTemp = Main_adc_polling(temp);

			_Temp_Tab[i][_Temp_Inc[i]] = Main_adc_polling(temp) / 100;
			_Temp_Inc[i]++;
			if(_Temp_Inc[i] > 99) _Temp_Inc[i] = 0;

			_Temp_Average[i] = 0;
			for(j=0;j<100;j++) _Temp_Average[i] += _Temp_Tab[i][j];
			pRX_Status->ink_supply[i].heaterTemp = _Temp_Average[i];
			pRX_Status->ink_supply[i].heaterTempReady = pRX_Status->ink_supply[i].heaterTemp > (pRX_Config->ink_supply[i].heaterTemp-HEATER_TEMP_TOLERANCE);

			//--- CEDRIC --- Error Temperature frozen : Auto-reset + EVENT message
			if(pRX_Config->ink_supply[i].ctrl_mode > ctrl_off)
			{
				if(pRX_Status->ink_supply[i].heaterTemp == _Temp_Pre[i]) // temperature frozen
				{
					_TimeTempFrozen[i]++;
					if(_TimeTempFrozen[i] > 3000)	// during 30 seconds
					{
						init_AMC7891(AVALON_SPI_AMC7891_1_BASE);
						pRX_Status->ink_supply[i].error |= err_heater_temp_frozen;
					}
				}
				else
				{
					_TimeTempFrozen[i] = 0;
					_Heater_Pre[i] = _pid_Temp[i].val;
					_Temp_Pre[i] = pRX_Status->ink_supply[i].heaterTemp;
				}
			}
		}

		if(pRX_Status->ink_supply[i].heaterTemp != INVALID_VALUE && pRX_Config->ink_supply[i].ctrl_mode > ctrl_off)
		{
			/*if (pRX_Status->ink_supply[i].heaterTemp >= pRX_Config->ink_supply[i].heaterTempMax)
				_set_heater_out(i, FALSE);
			else if (pRX_Config->ink_supply[i].headTemp >= pRX_Config->ink_supply[i].heaterTemp)
				_set_heater_out(i, FALSE);
			else if (pRX_Config->ink_supply[i].condPumpFeedback)
				_set_heater_out(i, TRUE);
			else
				_set_heater_out(i, FALSE);*/

			// PID parameters
			_pid_Temp[i].P 					= 20;
			_pid_Temp[i].I 					= 30000;
			_pid_Temp[i].Start_Integrator	= 1;
			_pid_Temp[i].val_max    		= 1000;	// Duty time on 1000 ms
			_pid_Temp[i].val_min			= 0;
			_pid_Temp[i].Setpoint			= pRX_Config->ink_supply[i].heaterTempMax;

			if (pRX_Config->ink_supply[i].condPumpFeedback)
			{
				pid_calc(pRX_Status->ink_supply[i].heaterTemp, &_pid_Temp[i]);
				_DutyTemp_Count[i] += 10;
				if(_DutyTemp_Count[i] > 1000) _DutyTemp_Count[i] = 0;
				// duty on 1 second
				if(_pid_Temp[i].val > _DutyTemp_Count[i]) _set_heater_out(i, TRUE);
				else _set_heater_out(i, FALSE);
			}
			else _set_heater_out(i, FALSE);
		}
		else
		{
			_set_heater_out(i, FALSE);
			pid_reset(&_pid_Temp[i]);
		}
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

	// Read config AMC to detect that it restart
	int readbackPOWER 	= IORD_16DIRECT(AVALON_SPI_AMC7891_1_BASE, AMC7891_AMC_POWER);
	int readbackCONFIG 	= IORD_16DIRECT(AVALON_SPI_AMC7891_1_BASE, AMC7891_GPIO_CONFIG);
	int readbackENABLE 	= IORD_16DIRECT(AVALON_SPI_AMC7891_1_BASE, AMC7891_ADC_ENABLE);
	int readbackGAIN 	= IORD_16DIRECT(AVALON_SPI_AMC7891_1_BASE, AMC7891_ADC_GAIN);

	if((readbackPOWER != 0x6000)||(readbackCONFIG != 0x0FF)||(readbackENABLE != 0x6DE0)||(readbackGAIN != 0xFF00))
	{

		pRX_Status->AMC_Register_Power 			= readbackPOWER;
		pRX_Status->AMC_Register_GPIO_Config 	= readbackCONFIG;
		pRX_Status->AMC_Register_Enable			= readbackENABLE;
		pRX_Status->AMC_Register_Gain 			= readbackGAIN;
		pRX_Status->error |= err_amc_config_lost;
	}
}

