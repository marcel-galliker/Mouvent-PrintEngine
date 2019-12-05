/******************************************************************************/
/** pres.c
 ** 
 ** pressure sensors
 **
 **	Copyright 2016 by radex AG, Switzerland. All rights reserved.
 **	Written by Marcel Galliker
 **
 ******************************************************************************/

//--- includes -------------------------------------------

#include <string.h>
#include "system.h"
#include "nios_def_head.h"
#include "i2c_master.h"
#include "trprintf.h"
#include "pres.h"
#include "AMC7891.h"
#include "io.h"
#include "i2c.h"


//--- defines ---------------------------
#define ADDR_SENSOR		0x78 	// upper address of sensor

#define BUF_SIZE		10

#define MAX_COOLER_PRES 800	// 0.8 bar

//--- types ----------------------------------------

typedef struct
{
	int				i2c;
	UINT32			*resetCnt;
	UINT32			*pSensorID;
	INT32			*pPressure;
	INT32			buf[BUF_SIZE];
	int				buf_idx;
	int				buf_valid;
	int				power;
	int				power_timer;
} SSensor;

//--- statics -----------------------------------
SSensor _Sensor;

//--- prototypes ------------------------

static void _set_power(int on);
static void _sensor_reset(SSensor *s);
static int  _sensor_read(SSensor *s);
static UINT32 _read_Sensor_ID(SSensor *s);
static void _i2c_wait_time(void);

//--- pres_init -----------------------
void pres_init(void)
{
	memset(&_Sensor, 0, sizeof(_Sensor));

	_Sensor.i2c 		= I2C_MASTER_1_BASE;
	_Sensor.pPressure 	= &pRX_Status->cooler_pressure;
	_Sensor.resetCnt	= &pRX_Status->cooler_pressure_reset_cnt;
	_Sensor.pSensorID   = &pRX_Status->cooler_pressure_ID;
	*_Sensor.pPressure	= INVALID_VALUE;
	_sensor_reset(&_Sensor);
}

//--- _set_power -----------------------------------
static void _set_power(int on)
{
	static int _on=FALSE;
	if (on!=_on)
	{
		if(on)
		{
			IOWR_16DIRECT(AMC7891_0_BASE, AMC7891_DAC0_DATA, _3V3);
			if (_Sensor.power_timer<=0) _Sensor.power_timer=1;
		}
		else
		{
			IOWR_16DIRECT(AMC7891_0_BASE, AMC7891_DAC0_DATA, _0V);
			*_Sensor.pPressure = INVALID_VALUE;
			_Sensor.power=FALSE;
			_Sensor.power_timer = 10;
			_Sensor.resetCnt++;
		}
		_on=on;
	}
}

//--- _sensor_reset -------------
static void _sensor_reset(SSensor *s)
{
	s->buf_idx   = 0;
	s->buf_valid = FALSE;
  (*s->pPressure)= INVALID_VALUE;
}

//--- _sensor_read -------------------------------
static int _sensor_read(SSensor *s)
{	
	int		pressure=0;
	
	//--- repower sensor ---
	if (!s->power)
	{
		if (s->power_timer==2) _set_power(TRUE);
		if ((--s->power_timer)<=0)
		{
			_sensor_reset(s);
			_set_power(TRUE);
			s->power=TRUE;
		}
		return REPLY_OK;
	}
		
	if(s->power)											//if there is no read error at the moment
	{
		I2C_start(s->i2c, ADDR_SENSOR, READ);
		pressure = (I2C_read(s->i2c, !LAST_BYTE) << 8) | I2C_read(s->i2c, LAST_BYTE);
		if(pressure == 0xffff || pressure == 0)				// pressure sensor error, reset
		{	
			_set_power(FALSE);						// DS1_3V3 OFF
			return REPLY_ERROR;
		}
		else
		{	
			//--- convert value --------------
			pressure = (2*(pressure- 16500)) / 27;	// div 13.5
			
			//--- save to buffer -----
			s->buf[s->buf_idx++] = pressure;
			if (s->buf_idx>=BUF_SIZE)
			{
				s->buf_valid = TRUE;
				s->buf_idx   = 0;
			}
			
			if (s->buf_valid)
			{
				//--- filter (average without the two extreme values) -----------
				INT32 min=s->buf[0];
				INT32 max=s->buf[0];
				INT32 sum=0;
				int i;
				for (i=0; i<BUF_SIZE; i++)
				{
					sum += s->buf[i];
					if (s->buf[i]<min) min=s->buf[i];
					if (s->buf[i]>max) max=s->buf[i];
				}
				*s->pPressure = (sum-min-max) / (BUF_SIZE-2);

				if (*s->pPressure > MAX_COOLER_PRES)
					pRX_Status->error.cooler_overpressure = TRUE;
			}
		}
	}
	return REPLY_OK;
}

//--- pres_tick_100ms ------------------
void pres_tick_100ms(void)
{
	static int init=1;

	if (init)
	{
		switch(init++)
		{
		case 1: _set_power(TRUE);
				break;

		case 2: *_Sensor.pSensorID =_read_Sensor_ID(&_Sensor);
				_set_power(FALSE);
				break;

		case 3:  _set_power(TRUE);
				 init=0;
				 break;
		}
	}
	else if (*_Sensor.pSensorID>256 )
	{
		_sensor_read(&_Sensor);
	}
}

//--- _read_Sensor_ID ------------------
static UINT32 _read_Sensor_ID(SSensor *s)
{
	UINT32  id=0;
	UCHAR	*pid = (UCHAR*)&id;

	// Read Sensor ID Back
	if (I2C_start(s->i2c, ADDR_SENSOR, WRITE)) return 1;
	if (I2C_write(s->i2c, 0x4e, LAST_BYTE))    return 2;						// ask for first two Sensor ID values

	_i2c_wait_time();

	if (I2C_start(s->i2c, ADDR_SENSOR, READ)) return 3;
	pid[0] = I2C_read(s->i2c, !LAST_BYTE);				// read first two Sensor ID values
	pid[1] = I2C_read(s->i2c, LAST_BYTE);				// read first two Sensor ID values

	_i2c_wait_time();

	if (I2C_start(s->i2c, ADDR_SENSOR, WRITE)) return 4;
	if (I2C_write(s->i2c, 0x4f, LAST_BYTE))    return 5;						// ask for second two Sensor ID values

	_i2c_wait_time();

	if (I2C_start(s->i2c, ADDR_SENSOR, READ)) return 6;
	pid[2] = I2C_read(s->i2c, !LAST_BYTE);				// read first two Sensor ID values
	pid[3] = I2C_read(s->i2c, LAST_BYTE);				// read first two Sensor ID values

	return id;
}

//--- _i2c_wait_time ------------------
void _i2c_wait_time(void)
{
	int i;
	// dummy wait for about 110us
	for (i = 0; i < 250; i++)
		;
}
