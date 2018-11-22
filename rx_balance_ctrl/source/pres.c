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
#include "IOMux.h"
#include "cond_def_head.h"
#include "i2c.h"
#include "pres.h"


//--- defines ---------------------------
#define ADDR_SENSOR    0x78 	// upper address of sensor

#define BUF_SIZE	10

//--- types ----------------------------------------
typedef void (*set_power_fct)	(int on);

typedef struct
{
	stc_mfsn_t		*pi2c;
	set_power_fct 	set_power;
	UINT32			*pResetCnt;
	INT32			*pPressure;
	INT32			buf[BUF_SIZE];
	int				buf_idx;
	int				buf_valid;
	int				power;
	int				power_timer;
	int				power_cnt;
	int				error;
} SSensor;

//--- statics -----------------------------------
/*static */ SSensor	_PressureIn;
/*static */ SSensor	_PressureOut;


//--- prototypes ------------------------

static void _PresIn_power(int on);
static void _PresOut_power(int on);
static void _sensor_reset(SSensor *s);
static void _sensor_read (SSensor *s);

//--- pres_init -----------------------
void pres_init(void)
{
	memset(&_PressureOut, 0, sizeof(_PressureOut));
	_PressureOut.pi2c 	   = (stc_mfsn_t*)FM4_MFS2_BASE;
	_PressureOut.set_power = _PresOut_power;
	_PressureOut.pPressure = &RX_Status.pressure_out;
	_PressureOut.pResetCnt = &RX_Status.pressure_out_reset_cnt;
   *_PressureOut.pPressure = INVALID_VALUE;
	_sensor_reset(&_PressureOut);

	memset(&_PressureIn, 0, sizeof(_PressureIn));
	_PressureIn.pi2c 	  = (stc_mfsn_t*)FM4_MFS3_BASE;
	_PressureIn.set_power = _PresIn_power;
	_PressureIn.pPressure = &RX_Status.pressure_in;
	_PressureIn.pResetCnt = &RX_Status.pressure_in_reset_cnt;
   *_PressureIn.pPressure = INVALID_VALUE;
	_sensor_reset(&_PressureIn);
}

//--- _PresIn_power ---------------------
static void _PresIn_power(int on)
{
	Gpio1pin_Put(GPIO1PIN_P35, on); 
}

//--- _PresOut_power --------------------
static void _PresOut_power(int on)
{
	Gpio1pin_Put(GPIO1PIN_P30, on);
}

//--- _sensor_reset -------------
static void _sensor_reset(SSensor *s)
{
	s->buf_idx   = 0;
	s->buf_valid = FALSE;
  (*s->pPressure)= INVALID_VALUE;
}

//--- _sensor_read -------------------------------
static void _sensor_read(SSensor *s)
{	
	int		pressure=0;
	UCHAR	*ppressure = (UCHAR*)&pressure;
	int		ret;
	
	//--- repower sensor ---
	if (!s->power)
	{
		if ((--s->power_timer)<=0)
		{
			_sensor_reset(s);
			s->set_power(TRUE);
			s->power=TRUE;
			return;	// check next value already ok?
		}
	}
		
	if(s->power)											//if there is no read error at the moment
	{
		ret=I2cStartRead(s->pi2c, ADDR_SENSOR);
		if (!ret) ret = I2cReceiveByteSeq(s->pi2c, &ppressure[1], FALSE);
		if (!ret) ret = I2cReceiveByteSeq(s->pi2c, &ppressure[0], TRUE);
		I2cStopWrite(s->pi2c);
		if(ret || pressure == 0xffff || pressure == 0)				// pressure sensor error, reset
		{	
			s->set_power(FALSE);						// DS1_3V3 OFF
			s->power=FALSE;
			s->power_timer = 10;
			s->power_cnt++;
			(*s->pResetCnt)++;
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
			if(pressure >= 16500)	pressure = ((pressure- 16500) / 13.5);
			else					pressure = -((16500 - pressure) / 13.5);
			
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
				s->pResetCnt  = 0;
				*s->pPressure = (sum-min-max) / (BUF_SIZE-2);
			}
		}
	}
}

//--- pres_valid ---------------------------------------
int  pres_valid(void)
{
	return _PressureOut.buf_valid;
}

//--- pres_tick_10ms ------------------
void pres_tick_10ms(void)
{
	if (RX_Status.gpio_state.valve)
	{
		_sensor_read(&_PressureIn);
		_sensor_read(&_PressureOut);	
	}
	else
	{
		_sensor_reset(&_PressureIn);
		_sensor_reset(&_PressureOut);
	}
	RX_Status.error.pres_in_hw  = _PressureIn.error;
	RX_Status.error.pres_out_hw = _PressureOut.error;
}
