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

//--- statics -----------------------------------
static int _Active=FALSE;

int	pvalve_active(void)
{
	return _Active;
}

//--- pvalve_init ------------------------
void pvalve_init(void)
{
	int ret;
	int i2c = I2C_MASTER_IS1_BASE;
	int dac_id=0;

	ret = I2C_start(i2c, ADDR_DAC60502, WRITE);
	if (!ret) ret = I2C_write(i2c, 0x01, LAST_BYTE);				// ask for DAC ID

	// _i2c_wait_time();

	ret = I2C_start(i2c, ADDR_DAC60502, READ);
	if (!ret)
	{
		dac_id  = I2C_read(i2c, !LAST_BYTE) << 8;				// read DAC ID
		dac_id += I2C_read(i2c, LAST_BYTE);
	}

	if(dac_id == 0x2215) _Active=TRUE;
}

//--- _set_valve --------------------------------------
static int _set_valve(int nr, int reg, int value)
{
	if (!_Active) 		return REPLY_ERROR;

	int i2c;
	int dac_data = MAX_VAL*100/value;

	if (dac_data<0) 	dac_data=0;
	if (dac_data>4095)	dac_data=MAX_VAL;

	switch(nr)
	{
	case 0:	i2c=I2C_MASTER_IS1_BASE; break;
	case 1:	i2c=I2C_MASTER_IS2_BASE; break;
	case 2:	i2c=I2C_MASTER_IS3_BASE; break;
	case 3:	i2c=I2C_MASTER_IS4_BASE; break;
	}

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
