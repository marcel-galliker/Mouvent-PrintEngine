/*
 * cooler.h
 *
 *  Created on: Nov 16, 2017
 *      Author: Cyril Andreatta
 */

#include "cooler.h"
#include "adc_thermistor.h"
#include "system.h"
#include "nios_def_head.h"
#include "i2c_master.h"
#include "i2c.h"

//--- defines ----------------------------------------------------

#define THERMISTOR_DEVICE_ID_ADDR		0x48	// 0x48 + R/W bit = 0x90/0x91

#define MAX_COOLER_TEMP 70000					// 70 deg celsius

//--- cooler_init --------------------------------------------------
void cooler_init(void)
{
	int i;
	BYTE ack = 0xff;
	static const int RETRY = 3;
	int retry = 0;

	pRX_Status->cooler_temp = INVALID_VALUE;

	do {
		ack = I2C_start(I2C_MASTER_1_BASE, THERMISTOR_DEVICE_ID_ADDR, READ);

		for (i = 0; i < 25000; i++)
			;
	} while ((ack != I2C_ACK) && (retry++ < RETRY));

	pRX_Status->info.cooler_pcb_present = (ack == I2C_ACK);
}

//--- cooler_tick_100ms ----------------------------------------------
void cooler_tick_100ms(void)
{
	static int _errCnt=0;	// filter for bad reads
	static int _invalidCnt=0;

	if (!pRX_Status->info.cooler_pcb_present) return;

	// temp sensor ADC result
	if (I2C_start(I2C_MASTER_1_BASE, THERMISTOR_DEVICE_ID_ADDR, READ) == I2C_ACK)
	{
		UINT32 val = (I2C_read(I2C_MASTER_1_BASE, !LAST_BYTE) << 8) | I2C_read(I2C_MASTER_1_BASE, LAST_BYTE);
		pRX_Status->cooler_temp = convert_pt100(val);
	}
	else
	{
		pRX_Status->cooler_temp = INVALID_VALUE;
	}

	if (pRX_Status->cooler_temp == INVALID_VALUE)
	{
		if (++_invalidCnt>20) pRX_Status->error.cooler_temp_hw=TRUE;
	}
	else
	{
		_invalidCnt = 0;
		if (pRX_Status->cooler_temp > MAX_COOLER_TEMP)
		{
			if (++_errCnt>10) pRX_Status->error.cooler_overheated=TRUE;
		}
		else _errCnt=0;
	}
}




