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
#define ADDR_HONEY_1    		0x08
#define ADDR_HONEY_100  		0x18
    
static const INT32 ZERO_PRESSURE_OFFSET_FIRST = 16500;
//static const INT32 PRES_RAW_MIN_FIRST          = 16095;
//static const INT32 PRES_RAW_MAX_FIRST          = 16905;
static const INT32 FULL_SCALE_SPAN_FIRST = 27;   // 27000

static const INT32 ZERO_PRESSURE_OFFSET_HONEY   = 81915;
//static const INT32 PRES_RAW_MIN_HONEY		    = 7998;
//static const INT32 PRES_RAW_MAX_HONEY         = 8392;
static const INT32 HONEY_DIVIDOR				= 6553;

    
//--- types ----------------------------------------

#define BUF_SIZE_10	        10

typedef void (*set_power_fct)	(int on);

typedef struct
{
	stc_mfsn_t		*pi2c;
	set_power_fct 	set_power;
	INT32			*pPressure;
	INT32			offset_factory;
//	INT32			offset_user;
	INT32			valFactor;
    INT32			buf10[BUF_SIZE_10];
	int				buf_idx10;       
	int				buf_valid;
	int				power;
	int				power_timer;
	int				power_cnt;
	int				low_cnt;
	int				high_cnt;
	int				error;
	int				calibrating;
    uint8_t			addr;      
	uint8_t	  		no_first_try_honey;     
    INT32           EE_ADDR_FACTORY;
    INT32           EE_ADDR_USER;
} SSensor;


//--- statics -----------------------------------
static  SSensor	_PressureIn;
static  SSensor	_PressureOut;

UINT16 _PresInOffset=1234;

//--- prototypes ------------------------

static void _PresIn_power(int on);
static void _PresOut_power(int on);
static void _sensor_reset(SSensor *s);
static void _sensor_read (SSensor *s);

//--- pres_init -----------------------
void pres_init(void)
{
	memset(&_PressureOut, 0, sizeof(_PressureOut));
	_PressureOut.pi2c 	   			= (stc_mfsn_t*)FM4_MFS2_BASE;
	_PressureOut.set_power 			= _PresOut_power;
	_PressureOut.valFactor 			= 1;
	_PressureOut.pPressure 			= &RX_Status.pressure_out;
   *_PressureOut.pPressure 			= INVALID_VALUE;
    _PressureOut.EE_ADDR_FACTORY 	= EE_ADDR_POUT_FACTORY_OFFSET;
    _PressureOut.EE_ADDR_USER    	= EE_ADDR_POUT_USER_OFFSET;
	_PressureOut.addr				= ADDR_HONEY_100;
   
	_sensor_reset(&_PressureOut);
	eeprom_read_setting16(_PressureOut.EE_ADDR_FACTORY, &_PressureOut.offset_factory);
//	eeprom_read_setting16(_PressureOut.EE_ADDR_USER, 	&_PressureOut.offset_user);

	memset(&_PressureIn, 0, sizeof(_PressureIn));
	_PressureIn.pi2c 	  		= (stc_mfsn_t*)FM4_MFS3_BASE;
	_PressureIn.set_power 		= _PresIn_power;
	_PressureIn.valFactor 		= 10;
	_PressureIn.pPressure 		= &RX_Status.pressure_in;
   *_PressureIn.pPressure 		= INVALID_VALUE;
	_PressureIn.addr			= ADDR_HONEY_1;
        
    _PressureIn.EE_ADDR_FACTORY = EE_ADDR_PIN_FACTORY_OFFSET;
    _PressureIn.EE_ADDR_USER    = EE_ADDR_PIN_USER_OFFSET;
    *_PressureIn.pPressure      = INVALID_VALUE;
	_sensor_reset(&_PressureIn);
	eeprom_read_setting16(_PressureIn.EE_ADDR_FACTORY, &_PressureIn.offset_factory);
//	eeprom_read_setting16(_PressureIn.EE_ADDR_USER,    &_PressureIn.offset_user);
	
	_PresInOffset = _PressureIn.offset_factory;
}

//--- _PresIn_power ---------------------
static void _PresIn_power(int on)
{
	Gpio1pin_Put(GPIO1PIN_P30, on); 
}

//--- _PresOut_power --------------------
static void _PresOut_power(int on)
{
	Gpio1pin_Put(GPIO1PIN_P35, on);
}

//--- _sensor_reset -------------
static void _sensor_reset(SSensor *s)
{
	s->buf_valid = FALSE;
  (*s->pPressure)= INVALID_VALUE;
    
    s->buf_idx10 = 0;
}

//--- _sensor_read -------------------------------
static void _sensor_read(SSensor *s)
{	
    INT32	ret;
    INT32   offset = 0;
    INT32   pressure = 0;   
	INT32   pressure10 = 0;   
	UCHAR	*ppressure = (UCHAR*)&pressure;
	UCHAR 	status;

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
		// if we never tried to connect to honeywell sensor, we try firstsensor first for two power up cycles
		if(s->no_first_try_honey<2)		
		{
			ret = I2cStartRead(s->pi2c, ADDR_SENSOR);
			if (!ret) ret = I2cReceiveByteSeq(s->pi2c, &ppressure[1], FALSE);
			if (!ret) ret = I2cReceiveByteSeq(s->pi2c, &ppressure[0], TRUE);
			I2cStopWrite(s->pi2c);	// must be this!
        
			// reset sensor on error
			if (ret || pressure == 0xffff)				
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
					s->no_first_try_honey++;
				}
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
				
				//--- save to buffer -----
				s->buf10[s->buf_idx10++] = pressure;
				if (s->buf_idx10 >= BUF_SIZE_10) s->buf_idx10   = 0;
				
				pressure10 = average(s->buf10,BUF_SIZE_10,0);
				
				s->buf_valid = TRUE;						
				if (s->buf_valid)
				{                
					//--- save calibration -----------------------------
					if (s->calibrating)
					{
						//--- select the offset by DEFINE --------------------
						#if defined (FACTORY_OFFSET)
							s->offset_factory = pressure; 
							eeprom_write_setting16(s->EE_ADDR_FACTORY, s->offset_factory);
						#else
							/*
							s->offset_user = pressure; 
							eeprom_write_setting16(s->EE_ADDR_USER, s->offset_user);
							*/
						#endif
						s->calibrating = FALSE;
					}
					
					//--- convert value --------------
					if (s->offset_factory)	
						offset = ZERO_PRESSURE_OFFSET_FIRST - s->offset_factory;
					else
					{					
						RX_Status.error |= COND_ERR_sensor_offset;
						offset = ZERO_PRESSURE_OFFSET_FIRST;
					}
					
					*s->pPressure = ((pressure10 - offset) * s->valFactor * 2) / FULL_SCALE_SPAN_FIRST; // *13.5
									
				}
			}
			// end firstsensor
		}
		
		// honeywell sensor
		
		else
		{
			ret = I2cStartRead(s->pi2c, s->addr);
			if (!ret) ret = I2cReceiveByteSeq(s->pi2c, &ppressure[1], FALSE);
			if (!ret) ret = I2cReceiveByteSeq(s->pi2c, &ppressure[0], TRUE);
			I2cStopWrite(s->pi2c);	// must be this!
					
					// reset sensor on error
			if (ret || pressure == 0xffff)				
			{	           
				s->set_power(FALSE); // DS1_3V3 OFF
				s->power = FALSE;
				s->power_timer = 4;
				s->power_cnt++;
				if (s->power_cnt>3)
				{
					s->power_cnt = 0;
					*s->pPressure = INVALID_VALUE;
					s->error= TRUE;
				}
			}
			else
			{	
				s->power_cnt = 0;
				status=ppressure[1]&0xC0;   // check status bit
				if(status==0)
				{					
					//--- save to buffer -----
					s->buf10[s->buf_idx10++] = pressure;
					if (s->buf_idx10 >= BUF_SIZE_10) s->buf_idx10   = 0;
					
					pressure10 = average(s->buf10,BUF_SIZE_10,0);
					
					s->buf_valid = TRUE;
					
					if (s->buf_valid)
					{                
						//--- save calibration -----------------------------
						if (s->calibrating)
						{
							//--- select the offset by DEFINE --------------------
							#if defined (FACTORY_OFFSET)
								s->offset_factory = pressure; 
								eeprom_write_setting16(s->EE_ADDR_FACTORY, s->offset_factory);
							#else
								/*
								s->offset_user = pressure; 
								eeprom_write_setting16(s->EE_ADDR_USER, s->offset_user);
								*/
							#endif
							s->calibrating = FALSE;
						}					

						//--- convert value --------------
						if (s->offset_factory)	
							offset = s->offset_factory;
						else
						{					
							RX_Status.error |= COND_ERR_sensor_offset;
							offset = 0;
						}
						
						*s->pPressure = (10 * pressure10 - ZERO_PRESSURE_OFFSET_HONEY + offset) * s->valFactor *100 / HONEY_DIVIDOR;
					
					}
				}
			}

		}
	}
}

//--- pres_del_user_offset -----------------------
void pres_del_user_offset(void)
{
	/*
	_PressureIn.offset_user  = 0;
	_PressureOut.offset_user = 0;
	eeprom_write_setting32(EE_ADDR_PIN_USER_OFFSET,  _PressureIn.offset_user);
	eeprom_write_setting32(EE_ADDR_POUT_USER_OFFSET, _PressureOut.offset_user);
	*/
}

//--- pres_calibration_start ---------------------------
void pres_calibration_start(void)
{
    _PressureIn.calibrating  = TRUE;
    _PressureOut.calibrating = TRUE;
}

//--- pres_calibration_done -----------------
int pres_calibration_done(void)
{
	return !_PressureIn.calibrating && !_PressureOut.calibrating;
}

//--- pres_valid ---------------------------------------
int pres_valid(void)
{
	return _PressureOut.buf_valid;
}

//--- pres_tick_10ms ------------------
void pres_tick_10ms(void)
{    		
    // todo: sensor never resets
	if (TRUE || RX_Status.info.valve)
	{
		_sensor_read(&_PressureIn);
		_sensor_read(&_PressureOut);
	}
	else
	{
		_sensor_reset(&_PressureIn);
		_sensor_reset(&_PressureOut);
	}
    
	if (_PressureIn.error)  RX_Status.error |= COND_ERR_pres_in_hw;
	if (_PressureOut.error) RX_Status.error |= COND_ERR_pres_out_hw;
	
	if (!valid(RX_Status.pressure_in) || !valid(RX_Status.pressure_out) || RX_Status.error&COND_ERR_p_in_too_high)
    {
        RX_Status.meniscus = INVALID_VALUE;
    }  
	else
    {
        RX_Status.meniscus = (INT32)(RX_Status.pressure_in - ((RX_Status.pressure_in - RX_Status.pressure_out) / 1.81));
    }
}
