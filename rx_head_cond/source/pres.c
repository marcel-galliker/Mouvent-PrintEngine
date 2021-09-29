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
#include "average.h"
#include "i2c.h"
#include "pres.h"
#include "eeprom.h"
#include "debug_printf.h"

//--- defines ---------------------------
#define ADDR_SENSOR         	0x78 	    // upper address of sensor
#define ADDR_SENSOR_1_0_BAR     0x78 	    // upper address of sensor
#define ADDR_SENSOR_2_5_BAR		0x35
#define ADDR_SENSOR_0_1_BAR		0x1A

#define PRES_IN1	0		// ORDER of sensors is important! (timing on I2C bus!)
#define PRES_OUT	1
#define PRES_IN2	2
#define SENSOR_CNT	3

//--- types ----------------------------------------

#define BUF_SIZE	10

typedef void (*set_power_fct)	(int on);

typedef struct
{
	stc_mfsn_t		*pi2c;
	set_power_fct 	set_power;
	INT32			*pPressure;
	INT32			offset;
	INT32			valFactor;
    INT32			buf[BUF_SIZE];
	int				buf_idx;       
	int				buf_valid;
	int				power;
	int				power_timer;
	int				power_cnt;
	int				low_cnt;
	int				high_cnt;
	int				error;
	UINT8			addr;
} SSensor;


//--- statics -----------------------------------
static  SSensor	_Sensor[SENSOR_CNT];

//--- prototypes ------------------------

static void _PresIn_power(int on);
static void _PresOut_power(int on);
static void _PresAll_power(int on);
static void _sensor_reset(SSensor *s);
static void _sensor_read (SSensor *s);
static void _sensor_error(SSensor *s);

//--- pres_init -----------------------
void pres_init(void)
{
	memset(&_Sensor, 0, sizeof(_Sensor));
	_Sensor[PRES_OUT].valFactor 	= 1;
	_Sensor[PRES_OUT].pPressure 	= &RX_Status.pressure_out;
	eeprom_read_setting16(EE_ADDR_POUT_FACTORY_OFFSET, &_Sensor[PRES_OUT].offset);

	_Sensor[PRES_IN1].pPressure 	= &RX_Status.pressure_in1;
	eeprom_read_setting16(EE_ADDR_PIN_FACTORY_OFFSET, &_Sensor[PRES_IN1].offset);
	
	_Sensor[PRES_IN2].pPressure 	= &RX_Status.pressure_in2;
	eeprom_read_setting16(EE_ADDR_PIN2_FACTORY_OFFSET, &_Sensor[PRES_IN2].offset);

	if(RX_Status.pcb_rev>='n')
	{
		//--- all addresses are changed! -----------------------
		_Sensor[PRES_OUT].pi2c 	   	= (stc_mfsn_t*)FM4_MFS3_BASE;
		_Sensor[PRES_OUT].addr		= ADDR_SENSOR_0_1_BAR;
		_Sensor[PRES_OUT].set_power	= _PresAll_power;
		
		_Sensor[PRES_IN1].pi2c 	  	= (stc_mfsn_t*)FM4_MFS2_BASE;
		_Sensor[PRES_IN1].addr		= ADDR_SENSOR_0_1_BAR;
		_Sensor[PRES_IN1].set_power	= _PresAll_power;
		_Sensor[PRES_IN1].valFactor	= 1;
		
		_Sensor[PRES_IN2].pi2c 	   	= (stc_mfsn_t*)FM4_MFS2_BASE;
		_Sensor[PRES_IN2].addr		= ADDR_SENSOR_2_5_BAR;
		_Sensor[PRES_IN2].set_power	= _PresAll_power;
	}
	else
	{
		_Sensor[PRES_OUT].pi2c 	   	= (stc_mfsn_t*)FM4_MFS2_BASE;
		_Sensor[PRES_OUT].addr 		= ADDR_SENSOR_1_0_BAR;
		_Sensor[PRES_OUT].set_power = _PresOut_power;
		
		_Sensor[PRES_IN1].pi2c 	   	= (stc_mfsn_t*)FM4_MFS3_BASE;
		_Sensor[PRES_IN1].addr  	= ADDR_SENSOR_1_0_BAR;
		_Sensor[PRES_IN1].set_power = _PresIn_power;
		_Sensor[PRES_IN1].valFactor = 10;
	}

	int i;
	for (i=0; i<SENSOR_CNT; i++) 
		_sensor_reset(&_Sensor[i]);
}

//--- _PresIn_power ---------------------
static void _PresIn_power(int on)
{
	Gpio1pin_Put(GPIO1PIN_P30, on);  // DSx_3V3 ON/OFF
}

//--- _PresOut_power --------------------
static void _PresOut_power(int on)
{
	Gpio1pin_Put(GPIO1PIN_P35, on);  // DSx_3V3 ON/OFF
}

//--- _PresAll_power --------------------
static void _PresAll_power(int on)
{	
	Gpio1pin_Put(GPIO1PIN_P30, on);  // DSx_3V3 ON/OFF
}

//--- _sensor_reset -------------
static void _sensor_reset(SSensor *s)
{
	s->buf_valid = FALSE;
  (*s->pPressure)= INVALID_VALUE;
    
    s->buf_idx = 0;
}

//--- _sensor_error -----------------------
static void _sensor_error(SSensor *s)
{
	s->set_power(FALSE); // DS1_3V3 OFF
	s->power = FALSE;
	s->power_timer = 4;
	s->power_cnt++;
	if (s->power_cnt>3)
	{
		s->power_cnt = 0;
		*s->pPressure = INVALID_VALUE;
		//s->error = TRUE;
	}	
}

//--- _sensor_read -------------------------------
static void _sensor_read(SSensor *s)
{	
    INT32	ret;
    INT32   pressure;   
	INT32   pressure10 = 0;   
	UCHAR	*ppressure = (UCHAR*)&pressure;

	if (!s->addr) return;
	
	//--- repower sensor ---
	if (!s->power)
	{
		if (s->power_timer==2)
            s->set_power(TRUE);
        
		if ((--s->power_timer)<=0)
		{
			_sensor_reset(s);
			s->set_power(TRUE);
			s->power = TRUE;
		}
		return;
	}
		
    //if there is no read error at the moment
	if (s->power)											
	{
		pressure=0; // 32 Bits!
		ret = I2cStartRead(s->pi2c, s->addr);

		if (!ret) ret = I2cReceiveByteSeq(s->pi2c, &ppressure[1], FALSE);
		if (!ret) ret = I2cReceiveByteSeq(s->pi2c, &ppressure[0], TRUE);
		I2cStopWrite(s->pi2c);	// must be this!
	
		// reset sensor on error
		if (ret || pressure == 0xffff)				
		{	           
			if(RX_Status.pcb_rev>='n')
			{				
				for(int i=0; i<SENSOR_CNT; i++)  _sensor_error(&_Sensor[i]);
			}
			else _sensor_error(s);
		}
		else if (pressure==0)
		{
			if (++(s->low_cnt)>10) *s->pPressure = VAL_UNDERFLOW;
		}
		else if (pressure==0x7fff)
		{
			if (++(s->high_cnt)>10) *s->pPressure = VAL_OVERFLOW;
		}
		else
		{	
			if (s->low_cnt)  s->low_cnt--;
			if (s->high_cnt) s->high_cnt--;
			
			s->power_cnt = 0;

			//--- convert value ----------------------------------
			if (s->addr==ADDR_SENSOR_2_5_BAR)
					pressure = (35000*(pressure-10714+s->offset)) / (30000-3000); // convert measured value [3000..30000] to -1 .. 2.5 (span 3.5) in (*10) 
			else	pressure = ((pressure-16500+s->offset)* s->valFactor *2)/27; // *13.5
			
			//--- save to buffer -----
			s->buf[s->buf_idx++] = pressure;
			if (s->buf_idx >= BUF_SIZE)
			{
				s->buf_valid = TRUE;
				s->buf_idx   = 0;
			}
			
			if (s->buf_valid)
			{                
				pressure10 = average(s->buf,BUF_SIZE,0);
				*s->pPressure = pressure10; // (Multiplied by 13.5) --> (*2/27, with fix point arithmetics)  
			}
		}
	}
}

//--- pres_valid ---------------------------------------
int pres_valid(void)
{
	return _Sensor[PRES_OUT].buf_valid;
}

//--- pres_tick_10ms ------------------
void pres_tick_10ms(void)
{    		
	int i;
	
    for(i=0; i<SENSOR_CNT; i++) _sensor_read(&_Sensor[i]);
    
	if (RX_Status.pressure_in1==VAL_OVERFLOW) RX_Status.pressure_in = RX_Status.pressure_in2;
	else									  RX_Status.pressure_in = RX_Status.pressure_in1;
	if (_Sensor[PRES_IN1].error) RX_Status.error |= COND_ERR_pres_in_hw;
	if (_Sensor[PRES_IN2].error) RX_Status.error |= COND_ERR_pres_in_hw;
	if (_Sensor[PRES_OUT].error) RX_Status.error |= COND_ERR_pres_out_hw;
	
	if (RX_Config.flowResistance<100 || RX_Config.flowResistance>500)
	{
		RX_Config.flowResistance = 181;
		RX_Status.flowResistance = RX_Config.flowResistance;
	}

	//--- calculating meniscus and flow factor --------------------------------
	if (!valid(RX_Status.pressure_in) || !valid(RX_Status.pressure_out) || RX_Status.error&COND_ERR_p_in_too_high)
    {
        RX_Status.meniscus   = INVALID_VALUE;
		RX_Status.flowFactor = INVALID_VALUE;
    }  
	else
    {
		int diff=RX_Status.pressure_in - RX_Status.pressure_out;
        RX_Status.meniscus = (INT32)(RX_Status.pressure_in - (diff / (0.01 * RX_Config.flowResistance)));
		if (RX_Status.pump_measured==INVALID_VALUE) RX_Status.flowFactor = INVALID_VALUE;
		else if (RX_Status.pump_measured) RX_Status.flowFactor = 100*diff/(RX_Status.pump_measured*60/100);
		else RX_Status.flowFactor = 1000;
    }
	if (RX_Status.mode == ctrl_print) RX_Status.info.flowFactor_ok = (RX_Status.flowFactor < 200);
	else RX_Status.info.flowFactor_ok = TRUE;
}
