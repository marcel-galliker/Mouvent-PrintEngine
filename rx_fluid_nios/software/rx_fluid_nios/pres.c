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
#include "altera_avalon_pio_regs.h"
#include "nios_def_fluid.h"
#include "pio.h"
#include "i2c_master.h"
#include "trprintf.h"
#include "pres.h"

//--- defines ---------------------------
#define ADDR_SENSOR    0x78 	// upper address of sensor

#define 	POWERUP_TIME	30	// total
#define		POWER_ON_DELAY	20	// wait after enabling power
#define 	BUF_SIZE	5

#define 	IS1_SENSOR	0
#define 	IS2_SENSOR	1
#define 	IS3_SENSOR	2
#define 	IS4_SENSOR	3
#define 	F_SENSOR	4
#define 	D_SENSOR	5
#define 	P_SENSOR	6
#define 	SENSOR_CNT	7

//--- types ----------------------------------------
typedef void (*set_power_fct)	(int on);

typedef struct
{
	int				i2c;
	UINT32			*pSensorID;
	INT32			*pPressure;
	INT32			buf[BUF_SIZE];
	int				buf_idx;
	int				buf_valid;
	int				power;
	int				power_timer;
} SSensor;

//--- statics -----------------------------------
static SSensor _Sensor[SENSOR_CNT];

//--- prototypes ------------------------

static void _set_power(SSensor *s, int on);
static void _sensor_reset(SSensor *s);
static void _sensor_read (SSensor *s, int no);
static int _read_Sensor_ID(SSensor *s);
static void _i2c_wait_time(void);


//--- pres_init -----------------------
void pres_init(void)
{
	int i;
	memset(_Sensor, 0, sizeof(_Sensor));

	_Sensor[IS1_SENSOR].i2c 		= I2C_MASTER_IS1_BASE;
	_Sensor[IS1_SENSOR].pSensorID 	= &pRX_Status->ink_supply[0].sensorID;
	_Sensor[IS1_SENSOR].pPressure 	= &pRX_Status->ink_supply[0].IS_Pressure_Actual;

	_Sensor[IS2_SENSOR].i2c 		= I2C_MASTER_IS2_BASE;
	_Sensor[IS2_SENSOR].pSensorID 	= &pRX_Status->ink_supply[1].sensorID;
	_Sensor[IS2_SENSOR].pPressure 	= &pRX_Status->ink_supply[1].IS_Pressure_Actual;

	_Sensor[IS3_SENSOR].i2c 		= I2C_MASTER_IS3_BASE;
	_Sensor[IS3_SENSOR].pSensorID 	= &pRX_Status->ink_supply[2].sensorID;
	_Sensor[IS3_SENSOR].pPressure 	= &pRX_Status->ink_supply[2].IS_Pressure_Actual;

	_Sensor[IS4_SENSOR].i2c 		= I2C_MASTER_IS4_BASE;
	_Sensor[IS4_SENSOR].pSensorID 	= &pRX_Status->ink_supply[3].sensorID;
	_Sensor[IS4_SENSOR].pPressure 	= &pRX_Status->ink_supply[3].IS_Pressure_Actual;

	_Sensor[F_SENSOR].i2c 			= I2C_MASTER_F_BASE;
	_Sensor[F_SENSOR].pSensorID		= &pRX_Status->flush_sensorID;
	_Sensor[F_SENSOR].pPressure 	= &pRX_Status->flush_pressure;

	_Sensor[D_SENSOR].i2c 			= I2C_MASTER_D_BASE;
	_Sensor[D_SENSOR].pSensorID		= &pRX_Status->degas_sensorID;
	_Sensor[D_SENSOR].pPressure 	= &pRX_Status->degass_pressure;

	_Sensor[P_SENSOR].i2c 			= I2C_MASTER_P_BASE;
	_Sensor[P_SENSOR].pSensorID		= &pRX_Status->air_sensorID;
	_Sensor[P_SENSOR].pPressure 	= &pRX_Status->air_pressure;

	for (i=0; i<SENSOR_CNT; i++)
	{
		_sensor_reset(&_Sensor[i]);
	}
}

//--- _set_power ----------------------------
static void _set_power(SSensor *s, int on)
{
	static int _on=FALSE;
	if (on!=_on)
	{
		int i;
		// turn on/off Power for Fluid Pressure Sensors (not IS-Adapter one's)
		if(on)
		{
			trprintf("_set_power(Sensor=%d, ON)\n", s-_Sensor);
			IOWR_ALTERA_AVALON_PIO_SET_BITS(PIO_OUTPUT_BASE,	SENSOR_POWER_ENABLE);	// Fluid Pressure Sensor Power Enable
			for (i=0; i<SENSOR_CNT; i++)
				if (_Sensor[i].power_timer<=0) _Sensor[i].power_timer=POWER_ON_DELAY-1;
		}
		else
		{
			trprintf("_set_power(Sensor=%d, OFF)\n", s-_Sensor);
			for (i=0; i<SENSOR_CNT; i++)
			{
				if (_Sensor[i].power)
				{
					_Sensor[i].power	   = FALSE;
					_Sensor[i].power_timer = POWERUP_TIME;
					*s->pPressure		   = INVALID_VALUE;
				}
			}
			IOWR_ALTERA_AVALON_PIO_CLEAR_BITS(PIO_OUTPUT_BASE,	SENSOR_POWER_ENABLE);	// Fluid Pressure Sensor Power Enable
			// IS sensors get a power OFF/ON by not requesting data for longer than 90 ms
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
static void _sensor_read(SSensor *s, int no)
{	
	int		pressure=0;
	
	//--- repower sensor ---
	if (!s->power)
	{
		if (s->power_timer==POWER_ON_DELAY) _set_power(s, TRUE);
		if ((--s->power_timer)<=0)
		{
			_set_power(s, TRUE);
			_sensor_reset(s);
			s->power=TRUE;
			return;
		}
	}
	else
	{
		I2C_start(s->i2c, ADDR_SENSOR, 1);
		pressure = (I2C_read(s->i2c, FALSE) << 8) | I2C_read(s->i2c, TRUE);

		trprintf("Sensor[%d]: raw=0x%x\n", s-_Sensor, pressure);

		if(pressure == 0xffff || pressure == 0)				// pressure sensor error, reset
		{
			_set_power(s, FALSE);						// DS1_3V3 OFF
		}
		else if (pressure==0xf1ff)
		{ // first after power on
		}
		else
		{	
			//--- convert value --------------
			pressure = ((pressure-16500) *2)/27; // *13.5

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
			}
		}
	}
}

//--- pres_valid --------------------------
int  pres_valid(int i)
{
	return _Sensor[i].buf_valid;
}

//--- pres_tick_10ms ------------------
void pres_tick_10ms(void)
{
	int i;
	static int init=1;

	if (init)
	{
		switch(init++)
		{
		case 1: _set_power(&_Sensor[0], TRUE); break;
		case 2: //--- activate ----
				for(i=0; i<4; i++)
				{
					I2C_start(_Sensor[i].i2c, ADDR_SENSOR, 1);
				}
				break;

		case 3: for(i=0; i<SENSOR_CNT; i++)
				{
					*_Sensor[i].pSensorID =_read_Sensor_ID(&_Sensor[i]);
				}
				_set_power(&_Sensor[0], FALSE);
				break;

		case 4: _set_power(&_Sensor[0], TRUE);
				 init=0;
				 break;
		}
	}
	else
	{
		for (i=0; i<SENSOR_CNT; i++)
		{
			if (*_Sensor[i].pSensorID>256) _sensor_read(&_Sensor[i], i);
		}
	}
}

//--- _read_Sensor_ID ------------------
static int _read_Sensor_ID(SSensor *s)
{
#define		WRITE		0
#define		READ		1
#define		LAST_BYTE	1

	UINT32  id=0;
	UCHAR	*pid = (UCHAR*)&id;

	// Read Sensor ID Back
	if (I2C_start(s->i2c, ADDR_SENSOR, WRITE))
		return 1;
	if (I2C_write(s->i2c, 0x4e, LAST_BYTE))
		return 2;						// ask for first two Sensor ID values

	_i2c_wait_time();

	if (I2C_start(s->i2c, ADDR_SENSOR, READ))
		return 3;
	pid[0] = I2C_read(s->i2c, !LAST_BYTE);			// read first two Sensor ID values
	pid[1] = I2C_read(s->i2c, LAST_BYTE);				// read first two Sensor ID values

	_i2c_wait_time();

	if (I2C_start(s->i2c, ADDR_SENSOR, WRITE))
		return 4;
	if (I2C_write(s->i2c, 0x4f, LAST_BYTE))
		return 5;						// ask for second two Sensor ID values

	_i2c_wait_time();

	if (I2C_start(s->i2c, ADDR_SENSOR, READ))
		return 6;
	pid[2] = I2C_read(s->i2c, !LAST_BYTE);			// read first two Sensor ID values
	pid[3] = I2C_read(s->i2c, LAST_BYTE);			// read first two Sensor ID values

	trprintf("Sensor[%d]. read ID=%d.%d.%d.%d\n", s-_Sensor, pid[0], pid[1], pid[2], pid[3]);
	return id;
}

//--- _i2c_wait_time ------------------
static void _i2c_wait_time(void)
{
	int i;
	// dummy wait for about 110us //TODO verify on fluid!!
	for (i = 0; i < 500; i++)
		;
}
