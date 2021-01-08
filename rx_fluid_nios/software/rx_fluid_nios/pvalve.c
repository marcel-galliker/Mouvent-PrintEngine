/******************************************************************************/
/** pvalve.h
 **
 ** proportional valve
 **
 **	Copyright 2021 by radex AG, Switzerland. All rights reserved.
 **	Written by Marcel Galliker
 **
 ******************************************************************************/

//--- includes -------------------------------------------

#include <string.h>
#include "system.h"
#include "pio.h"
#include "altera_avalon_pio_regs.h"
#include "nios_def_fluid.h"
#include "i2c_master.h"
#include "trprintf.h"
#include "pvalve.h"

//--- defines ---------------------------
#define ADDR_DAC60502			0x48
#define	REG_AIR					0x08	// register for air   selenoid
#define	REG_BLEED				0x09	// register for bleed selenoid
#define MAX_VAL 				4095	// 24 Volt

#define		WRITE		0
#define		READ		1
#define		LAST_BYTE	1

static const int _I2CBase[NIOS_INK_SUPPLY_CNT] =
{
	I2C_MASTER_IS1_BASE,
	I2C_MASTER_IS2_BASE,
	I2C_MASTER_IS3_BASE,
	I2C_MASTER_IS4_BASE
};

//--- statics -----------------------------------
static int _Active[NIOS_INK_SUPPLY_CNT];

//--- pvalve_active -----------------------------
int	pvalve_active(int isNo)
{
	return _Active[isNo];
}

//--- pvalve_init ------------------------
void pvalve_init(void)
{
	int isNo;
	int ret;
	int dac_id;
	for (isNo=0; isNo<NIOS_INK_SUPPLY_CNT; isNo++)
	{
		ret = I2C_start(_I2CBase[isNo], ADDR_DAC60502, WRITE);
		if (!ret) ret = I2C_write(_I2CBase[isNo], 0x01, LAST_BYTE);				// ask for DAC ID

		// _i2c_wait_time();

		ret = I2C_start(_I2CBase[isNo], ADDR_DAC60502, READ);
		dac_id = 0;
		if (!ret)
		{
			dac_id  = I2C_read(_I2CBase[isNo], !LAST_BYTE) << 8;				// read DAC ID
			dac_id += I2C_read(_I2CBase[isNo], LAST_BYTE);
		}

		if(dac_id == 0x2215)
		{
			int set_gain_reg = 0x0103;
			_Active[isNo]=TRUE;
			I2C_start(_I2CBase[isNo], ADDR_DAC60502, WRITE);
			I2C_write(_I2CBase[isNo], 0x04, !LAST_BYTE);                       // Set Gain
			I2C_write(_I2CBase[isNo], (set_gain_reg >> 8),  !LAST_BYTE);       // Gain MSB
			I2C_write(_I2CBase[isNo], (set_gain_reg & 0xff), LAST_BYTE);       // Gain LSB
		}
	}
}

//--- _set_valve --------------------------------------
static int _set_valve(int isNo, int reg, int value)
{
	if (!_Active[isNo]) 		return REPLY_ERROR;

	int i2c = _I2CBase[isNo];
	int dac_data = MAX_VAL*value/100;

	if (dac_data<0) 	dac_data=0;
	if (dac_data>4095)	dac_data=MAX_VAL;

	I2C_start(i2c, ADDR_DAC60502, WRITE);
	I2C_write(i2c, reg, !LAST_BYTE);
	I2C_write(i2c, (dac_data >> 4),  !LAST_BYTE);
	I2C_write(i2c, ((dac_data << 4) & 0xff), LAST_BYTE);

	return REPLY_OK;
}

//--- pvalve_set_air -----------------------------------
int pvalve_set_air(int nr, int value)
{
	return _set_valve(nr, REG_AIR, value);
}

//--- pvalve_set_bleed -----------------------------------
int pvalve_set_bleed(int nr, int value)
{
	return _set_valve(nr, REG_BLEED, value);
}
