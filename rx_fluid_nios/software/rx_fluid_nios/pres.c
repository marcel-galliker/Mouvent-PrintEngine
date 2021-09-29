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
#include "pio.h"
#include "altera_avalon_pio_regs.h"
#include "nios_def_fluid.h"
#include "fpga_def_fluid.h"
#include "i2c_master.h"
#include "trprintf.h"
#include "pvalve.h"
#include "pres.h"

//--- defines ---------------------------
#define ADDR_SENSOR_1_0_BAR 0x78 	// upper address of sensor (old, 1.0 bar) 0x1a
#define ADDR_SENSOR_2_5_BAR 0x35	// upper address of sensor (new, 2.5 bar) 0x35
#define ADDR_DAC			0x4C

#define 	BUF_SIZE	5

#define 	IS1_SENSOR		0
#define 	IS2_SENSOR		1
#define 	IS3_SENSOR		2
#define 	IS4_SENSOR		3
#define 	FLUSH_SENSOR	4
#define 	DEGAS_SENSOR	5
#define 	AIR_SENSOR		6
#define 	SENSOR_CNT		7

#define		WRITE			0
#define		READ			1
#define		LAST_BYTE		1

#define		PCB				-1

//--- types ----------------------------------------
typedef void (*set_power_fct)	(int on);

typedef struct
{
	int				i2c;
	int				isNo;	// number of InkSupply, -1=PSB
	INT32			*pPressure;
	INT32			buf[BUF_SIZE];
	int				buf_idx;
	int				buf_valid;
	int				power;
	int				power_timer;
	int				power_cnt;
	int				addr;
} SSensor;

//--- statics -----------------------------------
static SSensor 	_Sensor[SENSOR_CNT];
static int	  	_PowerState=0;

//--- prototypes ------------------------
static void _sensors_init(void);
static void _sensor_reset(SSensor *s);
static void _sensor_read (SSensor *s);
static int  _is_sensor_25(SSensor *s);
static int  _pcb_sensor_25(SSensor *s);
static void _is_sensor_power(int isNo, int state);

//--- pres_init -----------------------
void pres_init(void)
{
	int i;
	_PowerState=0;
	memset(_Sensor, 0, sizeof(_Sensor));
	for (i=0; i<SENSOR_CNT; i++) _sensor_reset(&_Sensor[i]);
}

//--- _sensor_reset -------------
static void _sensor_reset(SSensor *s)
{
	s->buf_idx   = 0;
	s->buf_valid = FALSE;
  (*s->pPressure)= INVALID_VALUE;
}

//--- _init_sensors --------------------------------
static void _sensors_init(void)
{
	int i;

	_Sensor[IS1_SENSOR].i2c 		= I2C_MASTER_IS1_BASE;
	_Sensor[IS1_SENSOR].pPressure 	= &pRX_Status->ink_supply[0].IS_Pressure_Actual;
	_Sensor[IS1_SENSOR].isNo		= 0;

	_Sensor[IS2_SENSOR].i2c 		= I2C_MASTER_IS2_BASE;
	_Sensor[IS2_SENSOR].pPressure 	= &pRX_Status->ink_supply[1].IS_Pressure_Actual;
	_Sensor[IS2_SENSOR].isNo		= 1;

	_Sensor[IS3_SENSOR].i2c 		= I2C_MASTER_IS3_BASE;
	_Sensor[IS3_SENSOR].pPressure 	= &pRX_Status->ink_supply[2].IS_Pressure_Actual;
	_Sensor[IS3_SENSOR].isNo		= 2;

	_Sensor[IS4_SENSOR].i2c 		= I2C_MASTER_IS4_BASE;
	_Sensor[IS4_SENSOR].pPressure 	= &pRX_Status->ink_supply[3].IS_Pressure_Actual;
	_Sensor[IS4_SENSOR].isNo		= 3;

	_Sensor[FLUSH_SENSOR].i2c 		= I2C_MASTER_F_BASE;
    _Sensor[FLUSH_SENSOR].pPressure = &pRX_Status->flush_pressure;
	_Sensor[FLUSH_SENSOR].isNo		= PCB;

	_Sensor[DEGAS_SENSOR].i2c 		= I2C_MASTER_D_BASE;
    _Sensor[DEGAS_SENSOR].pPressure = &pRX_Status->degass_pressure;
	_Sensor[DEGAS_SENSOR].isNo		= PCB;

	_Sensor[AIR_SENSOR].i2c 		= I2C_MASTER_P_BASE;
    _Sensor[AIR_SENSOR].pPressure 	= &pRX_Status->air_pressure;
	_Sensor[AIR_SENSOR].isNo		= PCB;

	for (i=0; i<SENSOR_CNT; i++)
	{
		if( _Sensor[i].isNo==PCB) _Sensor[i].addr = _pcb_sensor_25(&_Sensor[i]) ? ADDR_SENSOR_2_5_BAR : ADDR_SENSOR_1_0_BAR;
		else                      _Sensor[i].addr = _is_sensor_25 (&_Sensor[i]) ? ADDR_SENSOR_2_5_BAR : ADDR_SENSOR_1_0_BAR;

		if (_Sensor[i].addr == ADDR_SENSOR_2_5_BAR)
		{
			trprintf("Sensor[%d] is 2.5 bar\n", i);
			_Sensor[i].addr = ADDR_SENSOR_2_5_BAR;
		}
		else
		{
			trprintf("Sensor[%d] is 1.0 bar\n", i);
			_Sensor[i].addr = ADDR_SENSOR_1_0_BAR;
		}

		_sensor_reset(&_Sensor[i]);
	}
	trprintf("Initialized\n");
}

//--- ink_set_is_sensor_power --------------------------------
static void _is_sensor_power(int isNo, int state)
{
	//IS-Adapter version "g" or later: Pressure sensors powered by "bleed" GPIO
	if (isNo!=PCB && pvalve_active(isNo))
	{
		UINT16 val = IORD_16DIRECT(AXI_LW_SLAVE_REGISTER_0_BASE, GPIO_REG_OUT);
		if (state) val |=  BLEED_OUT(isNo);
		else	   val &=  ~BLEED_OUT(isNo);
		IOWR_16DIRECT(AXI_LW_SLAVE_REGISTER_0_BASE, GPIO_REG_OUT, val);
	}	 
}

//--- _is_sensor_25 -----------------------------------
//--- checks whether an 2.5 mbar sensor is present and switch it on ---
static int _is_sensor_25(SSensor *s)
{
	return pvalve_active(s->isNo);
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
static void _sensor_read(SSensor *s)
{	
	int		pressure=0;
	
	//--- repower sensor ---
	if (!s->power)
	{
		s->power_timer--;
		if (s->power_timer <= 5)
		{
			_sensor_reset(s);
			_is_sensor_power(s->isNo, TRUE);
			s->power=TRUE;
			return;	// check next value already ok?
		}
	}
		
	if(s->power)											//if there is no read error at the moment
	{
		int ret=I2C_start(s->i2c, s->addr, 1);
		pressure = (I2C_read(s->i2c, FALSE) << 8) | I2C_read(s->i2c, TRUE);
		if (s->addr==ADDR_SENSOR_1_0_BAR) trprintf("sensor[%d]: 1.0 ret=%d pressure=%d power_timer=%d\n", s-_Sensor, ret, pressure, s->power_timer);
		else							  trprintf("sensor[%d]: 2.5 ret=%d pressure=%d power_timer=%d\n", s-_Sensor, ret, pressure, s->power_timer);

		if (s->power_timer>0)
		{
			s->power_timer--;
			return;
		}
		if(pressure == 0xffff || pressure == 0)				// pressure sensor error, reset
		{
			_is_sensor_power(s->isNo, FALSE);
			s->power=FALSE;
			s->power_timer = 20;
			s->power_cnt++;
			trprintf("sensor[%d]: Error-power_cnt=%d\n", s-_Sensor, s->power_cnt);
			if (s->power_cnt>3)
			{
				*s->pPressure=INVALID_VALUE;
				//if (s->isNo==PCB)
				//	_PowerState = 0;
			}  
		}
		else 
		{	
			s->power_cnt=0;
			
			//--- convert value --------------
			if (s->addr==ADDR_SENSOR_1_0_BAR)
				pressure = ((pressure-16500) *2)/27; // *13.5
			else
				pressure = (3500*(pressure-10714)) / (30000-3000); // convert measured value [3000..30000] to -1 .. 2.5 (span 3.5)

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

	trprintf("_PowerState=%d\n", _PowerState);
	switch(_PowerState)
	{
	case 0: // disable 24V ----------------------------
			IOWR_ALTERA_AVALON_PIO_CLEAR_BITS(PIO_OUTPUT_BASE,	SENSOR_POWER_ENABLE);
			for (i=0; i<SENSOR_CNT; i++)
			{
				if (_Sensor[i].isNo==PCB) _sensor_reset(&_Sensor[i]);
				else _is_sensor_power(_Sensor[i].isNo, FALSE);
			}
			_PowerState++;
			break;

	case 1: // enable 24V ---------------------------------------
			IOWR_ALTERA_AVALON_PIO_SET_BITS(PIO_OUTPUT_BASE,	SENSOR_POWER_ENABLE);
			for (i=0; i<NIOS_INK_SUPPLY_CNT; i++)
				_is_sensor_power(i, TRUE);
			_PowerState++;
			break;

	case 2:	// initialize sensors
			_sensors_init();
			_PowerState++;
			break;

	default: // system up and running: read sensors
			for (i=0; i<SENSOR_CNT; i++)
			{
				_sensor_read(&_Sensor[i]);
			}
	}
}
