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
#include "nios_def_fluid.h"
#include "i2c_master.h"
#include "trprintf.h"
#include "pres.h"
#include "average.h"

//--- defines ---------------------------
#define ADDR_SENSOR_1_0_BAR 0x78 	// upper address of sensor (old, 1.0 bar)
#define ADDR_SENSOR_2_5_BAR 0x35	// upper address of sensor (new, 2.5 bar)
#define ADDR_DAC			0x4C

#define 	BUF_SIZE	5

#define 	IS1_SENSOR	0
#define 	IS2_SENSOR	1
#define 	IS3_SENSOR	2
#define 	IS4_SENSOR	3
#define 	F_SENSOR	4
#define 	D_SENSOR	5
#define 	P_SENSOR	6
#define 	SENSOR_CNT	7

#define		WRITE		0
#define		READ		1
#define		LAST_BYTE	1

//--- types ----------------------------------------
typedef void (*set_power_fct)	(int on);

typedef struct
{
	int				i2c;
//	set_power_fct 	set_power;
	UINT32			resetCnt;
	INT32			*pPressure;
	INT32			buf[BUF_SIZE];
	int				buf_idx;
	int				buf_valid;
	int				power;
	int				power_timer;
	int				power_cnt;
	int				error;
	int				addr;
} SSensor;

//--- statics -----------------------------------
static SSensor _Sensor[SENSOR_CNT];

//--- prototypes ------------------------

// static void _PresIn_power(int on);
// static void _PresOut_power(int on);
static void _sensor_reset(SSensor *s);
static void _sensor_read (SSensor *s, int no);
static int _init_sensor_25(SSensor *s);

//--- pres_init -----------------------
void pres_init(void)
{
	int i;
	memset(_Sensor, 0, sizeof(_Sensor));

	_Sensor[IS1_SENSOR].i2c 		= I2C_MASTER_IS1_BASE;
	_Sensor[IS1_SENSOR].pPressure 	= &pRX_Status->ink_supply[0].IS_Pressure_Actual;

	_Sensor[IS2_SENSOR].i2c 		= I2C_MASTER_IS2_BASE;
	_Sensor[IS2_SENSOR].pPressure 	= &pRX_Status->ink_supply[1].IS_Pressure_Actual;

	_Sensor[IS3_SENSOR].i2c 		= I2C_MASTER_IS3_BASE;
	_Sensor[IS3_SENSOR].pPressure 	= &pRX_Status->ink_supply[2].IS_Pressure_Actual;

	_Sensor[IS4_SENSOR].i2c 		= I2C_MASTER_IS4_BASE;
	_Sensor[IS4_SENSOR].pPressure 	= &pRX_Status->ink_supply[3].IS_Pressure_Actual;

	_Sensor[F_SENSOR].i2c 			= I2C_MASTER_F_BASE;
	_Sensor[F_SENSOR].pPressure 	= &pRX_Status->flush_pressure;

	_Sensor[D_SENSOR].i2c 			= I2C_MASTER_D_BASE;
	_Sensor[D_SENSOR].pPressure 	= &pRX_Status->degass_pressure;

	_Sensor[P_SENSOR].i2c 			= I2C_MASTER_P_BASE;
	_Sensor[P_SENSOR].pPressure 	= &pRX_Status->air_pressure;

	for (i=0; i<SENSOR_CNT; i++)
	{
		_Sensor[i].addr = ADDR_SENSOR_1_0_BAR;
		if (i<=IS4_SENSOR)
{
			if(_init_sensor_25(&_Sensor[i])==REPLY_OK)
	{
				trprintf("Sensor[%d] is 2.5 bar\n", i);
				_Sensor[i].addr=ADDR_SENSOR_2_5_BAR;
		}
			else trprintf("Sensor[%d] is 1.0 bar\n", i);
				}
		_sensor_reset(&_Sensor[i]);
			}
		}

//--- _init_sensor_25 -----------------------------------
//--- checks whether an 2.5 mbar sensor is present and switch it on ---
static int _init_sensor_25(SSensor *s)
{
	int ret=0;
	// CTRL/MS-Byte = 00001111 (PD0+PD1= 0, D7-D4 =1) = 0x0f
	// LS-Byte = 11110000 (D0-D3 =1, other bits = 0)  = 0xf0

	// Dummy read
	ret = I2C_start(s->i2c, ADDR_DAC, READ);
	ret = I2C_read(s->i2c, LAST_BYTE);
	ret = 0;

	ret = I2C_start(s->i2c, ADDR_DAC, WRITE);
	if (!ret) ret = I2C_write(s->i2c, 0x0f, !LAST_BYTE);	// write MS-Byte
	if (!ret) ret = I2C_write(s->i2c, 0xf0, LAST_BYTE);		// write LS-Byte
	if(ret)
	{
		ret = I2C_write(s->i2c, 0x00, LAST_BYTE);		// write Stop
		return REPLY_ERROR;
	}
	return REPLY_OK;
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
		if ((--s->power_timer)<=5)
		{
			_sensor_reset(s);
		//	s->set_power(TRUE);
			s->power=TRUE;
			return;	// check next value already ok?
		}
	}
		
	if(s->power)											//if there is no read error at the moment
	{
		I2C_start(s->i2c, s->addr, 1);
		pressure = (I2C_read(s->i2c, FALSE) << 8) | I2C_read(s->i2c, TRUE);
		if (s->power_timer>0)
		{
			s->power_timer--;
			return;
		}
		if(pressure == 0xffff || pressure == 0)				// pressure sensor error, reset
		{
			// s->set_power(FALSE);						// DS1_3V3 OFF
			s->power=FALSE;
			s->power_timer = 20;
			s->power_cnt++;
			(s->resetCnt)++;
			if (s->power_cnt>3)
			{
				*s->pPressure=INVALID_VALUE;
				s->error = TRUE;
		}
		}
		else
		{	
			s->power_cnt=0;
			
			//--- convert value --------------
			if (s->addr==ADDR_SENSOR_1_0_BAR)
			{
			pressure = ((pressure-16500) *2)/27; // *13.5
			}
			else
			{
				pressure = (3500*(pressure-10714)) / (30000-3000); // convert messured value [3000..30000] to -1 .. 2.5 (span 3.5)
			}

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
				s->resetCnt  = 0;
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
		for (i=0; i<SENSOR_CNT; i++)
		{
		_sensor_read(&_Sensor[i], i);
		}
}
