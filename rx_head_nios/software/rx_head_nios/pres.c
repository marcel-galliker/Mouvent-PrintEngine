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
#define ADDR_SENSOR_1_0_BAR	0x78 	// upper address of sensor (old, 1.0 bar) 0x78
#define ADDR_SENSOR_2_5_BAR	0x35	// upper address of sensor (new, 2.5 bar) 0x35

#define BUF_SIZE		10

//--- types ----------------------------------------

typedef struct
{
	int				i2c;
	UINT32			*resetCnt;
	INT32			*pPressure;
	INT32			buf[BUF_SIZE];
	int				buf_idx;
	int				buf_valid;
	int				power;
	int				power_timer;
	int				power_cnt;
	int				addr;

	//--- for development only ---------
	int				ok;
	int				nok;
	int				poweron;
} SSensor;

//--- statics -----------------------------------
SSensor _Sensor;

//--- prototypes ------------------------

static void _sensor_reset(SSensor *s);
static int  _sensor_read(SSensor *s);
static int _pcb_sensor_25(SSensor *s);


//--- pres_init -----------------------
void pres_init(void)
{
	memset(&_Sensor, 0, sizeof(_Sensor));

	_Sensor.i2c 		= I2C_MASTER_1_BASE;
	_Sensor.pPressure 	= &pRX_Status->cooler_pressure;
	_Sensor.resetCnt	= &pRX_Status->cooler_pressure_reset_cnt;
	*_Sensor.pPressure	= INVALID_VALUE;

	_Sensor.addr = _pcb_sensor_25(&_Sensor) ? ADDR_SENSOR_2_5_BAR : ADDR_SENSOR_1_0_BAR;

	if (_Sensor.addr == ADDR_SENSOR_2_5_BAR)
		trprintf("Sensor is 2.5 bar\n");
	else
		trprintf("Sensor is 1.0 bar\n");

	_sensor_reset(&_Sensor);
}

//--- _sensor_reset -------------
static void _sensor_reset(SSensor *s)
{
	s->buf_idx   = 0;
	s->buf_valid = FALSE;
  (*s->pPressure)= INVALID_VALUE;
}

//--- _pcb_sensor_25 -----------------------
static int _pcb_sensor_25(SSensor *s)
{
	int ret;
	int pressure;

	ret=I2C_start(s->i2c, ADDR_SENSOR_2_5_BAR, READ);
	I2C_read(s->i2c, TRUE);

	ret=I2C_start(s->i2c, ADDR_SENSOR_2_5_BAR, READ);
	pressure = (I2C_read(s->i2c, FALSE) << 8) | I2C_read(s->i2c, TRUE);
	trprintf("_pcb_sensor_25: ret=%d pressure=%d\n", ret, pressure);

	return (ret==0); // && pressure!=0xffff);
}


//--- _sensor_read -------------------------------
static int _sensor_read(SSensor *s)
{	
	int		pressure=0;
	
	//--- repower sensor ---
	if (!s->power)
	{
		if ((--s->power_timer)<=0)
		{
			_sensor_reset(s);
			s->poweron++;
			s->power=TRUE;
		}
		return REPLY_OK;
	}
		
	if(s->power)											//if there is no read error at the moment
	{
		if (I2C_start(s->i2c, s->addr, READ)!=I2C_ACK)
		{
			*s->pPressure=INVALID_VALUE;
			return REPLY_ERROR;	// stop here if sensor is not answering
		}
		pressure = (I2C_read(s->i2c, !LAST_BYTE) << 8) | I2C_read(s->i2c, LAST_BYTE);
		if(pressure == 0xffff || pressure == 0)				// pressure sensor error, reset
		{	
			s->power=FALSE;
			s->power_timer = 10;
			s->power_cnt++;
			(s->resetCnt)++;
			if (s->power_cnt>3)
			{
				*s->pPressure=INVALID_VALUE;
				return REPLY_ERROR;
			}
		}
		else
		{	
			s->power_cnt=0;
			s->ok++;
			
			//--- convert value --------------
			if (s->addr == ADDR_SENSOR_1_0_BAR)
			{
				pressure = ((pressure- 16500) / 13.5);
			}
			else
			{
				pressure = (3500*(pressure-10714)) / (30000-3000);
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
	return REPLY_OK;
}

//--- pres_tick_100ms ------------------
void pres_tick_100ms(void)
{
	#define MAX_COOLER_PRES 800	// 0.8 bar
	int _errcnt=0;

	if (_sensor_read(&_Sensor))
	{
		if (++_errcnt>10) pRX_Status->error.cooler_pressure_hw = TRUE;
	}
	else _errcnt=0;

	if (*_Sensor.pPressure != INVALID_VALUE && *_Sensor.pPressure > MAX_COOLER_PRES)
		pRX_Status->error.cooler_overpressure = TRUE;
}
