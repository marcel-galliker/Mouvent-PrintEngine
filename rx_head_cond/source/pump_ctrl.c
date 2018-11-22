/******************************************************************************/
/** \file pump_ctrl.h
 ** 
 ** 
 **
 **	Copyright 2016 by radex AG, Switzerland. All rights reserved.
 **	Written by Marcel Galliker
 **
 ******************************************************************************/

//--- includes --------------------------------------------------
#include "IOMux.h"
#include "pid.h"
#include "cond_def_head.h"
#include "work_flash_operation.h"
#include "temp_ctrl.h"
#include "pres.h"
#include "pump_ctrl.h"
#include "debug_printf.h"
#include "main.h"
#include "average.h"
#include "pumptime.h"

//--- defines --------------------
#define IMPULSES_PER_REVOLUTION 6

#define WATCHDOG_TIMEOUT		500 // 50
#define COND_ALIVE_MAX  		100 // 100

#define TO_FLUSH	FALSE
#define TO_INK		TRUE

static const INT32 MENISCUS_TIMEOUT 	= 200; //2000;
static const INT32 MENISCUS_CHECK_TIME 	= 60*100;

static const INT32 DEFAULT_P = 1000;
static const INT32 DEFAULT_I = 50;
static const INT32 DEFAULT_D = 0;
static const INT32 DEFAULT_OFFSET = 1400; // ~34mL/min

static const INT32 FLOW_RATE = 40000; // mL/min


//--- prototypes -----------------
static void _InputCapture00_Init(void);
void ICU0_IRQHandler(void);

static void _set_valve(int state);
static void _set_pump_speed(UINT32 speed);
static void _pump_pid(void);
static void _watchdog_check(void);
static void _menicus_minmax(void);
static void _menicus_minmax_reset(void);
static void _error_cnt(int err, int *errcnt, int errflag);

static int	_watchdog;
static int	_timer;
static UINT32  	_meniscusMinMaxTimer=MENISCUS_CHECK_TIME;
static INT32	_meniscus_min;
static INT32	_meniscus_max;
static INT32	_pin_min;
static INT32	_pin_max;
static INT32	_pout_min;
static INT32	_pout_max;


static UINT64 _flow = 0;
static UINT32 _flow_cnt = 0;
static UINT32 _pump_ticks = 0;

static INT32 _meniscus_err_cnt = 0;
static INT32 _no_ink_err_cnt = 0;

int	_test;
int	_test1;
    
/*
 * Values from FHNW
 * P    705.585     [1/mbar]
 * I    0.0975681   [1/(ms*mbar)]
 * D    989739      [ms/mbar]
 */

SPID_par _PumpPID = 
{
	.P				= DEFAULT_P,
	.I				= DEFAULT_I,
	.D				= DEFAULT_D,
	.norm			= 100,
	.offset         = DEFAULT_OFFSET,   // value the pump should run in idle
 //   .I_rampup_to    = I_RAMPUP_TO,
    
	.val_min		= 91,   // 91, => 0.22V start of linear pump function 
	.val_max		= 4095, // 0xfff, 	//max value for pump DAC voltage
							// Value for DAC, 0x0fff = 9.7V => max, 0x0000 => 32mV => min
};

INT32 abs(INT32 val)
{
    if (val < 0) return -val;
    else		 return  val;
}

//--- pump_init ------------------------------------
void pump_init(void)
{
	_InputCapture00_Init();    
	pid_reset(&_PumpPID);
}

//--- pump_watchdog --------------------------------
void pump_watchdog(void)
{
	_watchdog = WATCHDOG_TIMEOUT;
}

//--- _watchdog_check --------------------------------
static void _watchdog_check(void)
{
	if ((_watchdog > 0) && (--_watchdog <= 0)) 								
        RX_Status.error |= COND_ERR_pump_watchdog;
		
	//--- alive test -----------
	if (RX_Config.alive && RX_Status.alive-RX_Config.alive > COND_ALIVE_MAX) RX_Status.error |= COND_ERR_alive;
	
    #ifndef DEBUG
    if (RX_Status.error & (COND_ERR_pump_watchdog | COND_ERR_alive))
	{
		turn_off_pump();
		temp_ctrl_on(FALSE);        
	}
    #endif
}

//--- pump_tick_10ms -------------------------------
void pump_tick_10ms(void)
{
    static const UINT32 MBAR_500     = 5000;
    static const UINT32 MBAR_100     = 1000;
    static const UINT32 SD_PUMP_TIME = 2000;
	static const INT32 cycle_time    = 10;
	static const INT32 FLUSH_TIMEOUT = 5000;
    
	INT32  			max_pressure = MBAR_500; // save value in case of ctrl_off
    static UINT32 _sd_pump_time  = SD_PUMP_TIME;
	
    static UINT32 _flush_timeout;
	static UINT32 _flush_done;
	static UINT32 _pumpTime;
	static UINT32 _prev_mode = ctrl_undef; 

	if (RX_Status.pressure_out == INVALID_VALUE) // || RX_Status.pressure_in == INVALID_VALUE)
	{
		turn_off_pump();
		temp_ctrl_on(FALSE);
		RX_Status.mode=ctrl_off;
		return;
	}
	
	//--- PID Values --------------------------------
	/*
	if (RX_Config.cmd.set_pid)
	{			
		if (RX_Config.mode<ctrl_cal_step1 || RX_Config.mode>ctrl_cal_done)
		{				
			// update PID values
			_PumpPID.P = RX_Config.controller_P;
			_PumpPID.I = RX_Config.controller_I;
			_PumpPID.D = RX_Config.controller_D;
			_PumpPID.offset = RX_Config.controller_offset;
		}
		RX_Status.cmdConfirm.set_pid = TRUE;
	}
    */
	
	RX_Status.controller_P 		= _PumpPID.P;	
	RX_Status.controller_I 		= _PumpPID.I;
	RX_Status.controller_D		= _PumpPID.D;
	RX_Status.controller_offset	= _PumpPID.offset;
	//--- END PID Values -------------------------
	
	switch(RX_Config.mode)
	{
		case ctrl_off: 
		case ctrl_shutdown_done:         
						temp_ctrl_on(FALSE);
						turn_off_pump();
						RX_Status.logCnt = 0;
						_pump_ticks = 0;
						if (RX_Status.mode>ctrl_off)
                            save_pumptime(RX_Status.pumptime, TRUE);
                        
                        RX_Status.mode = RX_Config.mode;
						break;		
        
		case ctrl_warmup:
						temp_ctrl_on(TRUE);
						if (RX_Status.mode==ctrl_readyToPrint) break;
						if (RX_Status.mode!=ctrl_warmup)
							_timer = 0;
                        
						_timer+=cycle_time;
						_set_valve(TO_INK);
						max_pressure = MBAR_500;
						if (_timer<5000)
                            _set_pump_speed(_PumpPID.val_max*75/100+1);
						else if (_timer<20000) 
                            _pump_pid();
						else 
                            RX_Status.mode=ctrl_readyToPrint;	// ???
                        		
                        RX_Status.mode = RX_Config.mode; 		// ???
						break;
		
        //--- PRINT --------------------------------------------
		case ctrl_print:   
						temp_ctrl_on(TRUE);
						if (RX_Status.mode!=ctrl_print)
						{
							_PumpPID.P 		= DEFAULT_P;
							_PumpPID.I 		= DEFAULT_I;
							_PumpPID.D 		= DEFAULT_D;
							pid_reset(&_PumpPID);
							_meniscus_err_cnt=0;
							_no_ink_err_cnt  =0;
						}
                        _set_valve(TO_INK);
                        max_pressure = MBAR_500;
        
						_pump_pid();
                        
                        RX_Status.mode = RX_Config.mode;
						break;
        
        //--- CALIBRATE ----------------------------------------
        case ctrl_cal_start:
						temp_ctrl_on(TRUE);
                        _set_valve(TO_INK);
                        _set_pump_speed(0);
                        RX_Status.mode  = ctrl_cal_start;
                        break;
                                
        case ctrl_cal_step2:    
						temp_ctrl_on(TRUE);
						_pump_pid();
                        RX_Status.mode = RX_Config.mode;
                        break;

        case ctrl_cal_step1:	// calculate PID.offset to pump 40 ml/min                    
        case ctrl_cal_step3:
						if (_prev_mode!=RX_Config.mode) 
						{
							if (RX_Config.mode==ctrl_cal_step1)
							{
								_PumpPID.P 		= 0;
								_PumpPID.I 		= 0;
								_PumpPID.D 		= 0;
								_PumpPID.offset = 1400;
							}
							else
							{
								_PumpPID.P 		= DEFAULT_P;
								_PumpPID.I 		= DEFAULT_I;
								_PumpPID.D 		= DEFAULT_D;
							}
							pid_reset(&_PumpPID);
							_pumpTime	   = RX_Status.pumptime;
						}
							
						temp_ctrl_on(TRUE);
						_pump_pid();
						if (RX_Status.mode != RX_Config.mode)
						{
							if (RX_Status.pumptime>_pumpTime+2)
							{
								int flow = (int)RX_Status.pump_measured * 60;
								if (abs(flow - FLOW_RATE) < 1000) RX_Status.mode = RX_Config.mode;
								else 
								{
									int diff=(FLOW_RATE-flow) / 50;
									if (diff>0 && diff<10)  diff=10;
									if (diff<0 && diff>-10) diff=-10;
									_PumpPID.offset = ((_PumpPID.offset+diff+9)/10)*10;
									_pumpTime = RX_Status.pumptime;
									if (_PumpPID.offset<1000) 
									{
										_PumpPID.offset = 1000;
										RX_Status.mode = RX_Config.mode;
									}
									if (_PumpPID.offset>3000) 
									{
										_PumpPID.offset = 3000;
										RX_Status.mode = RX_Config.mode;
									}
								}
							}
						}
                        break;

        case ctrl_cal_done:                
						temp_ctrl_on(TRUE);
						_pump_pid();
                        RX_Status.mode = RX_Config.mode;
                        break;
	
		//--- PURGE --------------------------------------------
		case ctrl_purge_soft:
		case ctrl_purge:
		case ctrl_purge_hard:
		case ctrl_purge_micro:
						temp_ctrl_on(FALSE);
						turn_off_pump();
						max_pressure = MBAR_500;
                        RX_Status.mode = RX_Config.mode;
						break;
		
		case ctrl_purge_step1:
						temp_ctrl_on(FALSE);
						turn_off_pump();
						max_pressure = MBAR_500;
                        RX_Status.mode = RX_Config.mode;
						break;
		
		case ctrl_purge_step2:
						temp_ctrl_on(TRUE);
                        _set_valve(TO_INK);
						_set_pump_speed(0);
						max_pressure = MBAR_500;
                        RX_Status.mode = RX_Config.mode;
						break;
		
		case ctrl_purge_step3:
						temp_ctrl_on(FALSE);
						turn_off_pump();
						max_pressure = MBAR_500;
                        RX_Status.mode = RX_Config.mode;
						break;

		case ctrl_wipe:
						temp_ctrl_on(FALSE);
						turn_off_pump();
						max_pressure = MBAR_500;
                        RX_Status.mode = RX_Config.mode;
						break;

		//--- FLUSH ------------------------------------------------
		case ctrl_flush:
                        turn_off_pump(); // opens FLUSH valve
						max_pressure 	= MBAR_500;
                        RX_Status.mode 	= RX_Config.mode;
                        break;
        
		case ctrl_flush_step1:
						_flush_done = 0;
                        _flush_timeout = FLUSH_TIMEOUT;
                        RX_Status.mode = RX_Config.mode;
                        break;
        
        case ctrl_flush_step2:
						/*
                        if (RX_Status.pressure_in >= FLUSH_PRESSURE)
                            ++_flush_done;
                        
                        // wait for ~30 seconds
                        if (_flush_done >= 1000)
                        {
                            RX_Status.mode = RX_Config.mode;
                        }
                        else if (--_flush_timeout <= 0)
                        {
                            RX_Status.mode = RX_Config.mode;
                            RX_Status.error |= COND_ERR_flush_failed;
                        }
						*/
						if (valid(RX_Status.pressure_in)) _flush_done += RX_Status.pressure_in;
						if (--_flush_timeout <= 0)
                        {
							printf("flushDone=%d\n", _flush_done);
                            RX_Status.mode = RX_Config.mode;
                        }
                        break;                        
		
        case ctrl_flush_done:
                        RX_Status.mode = RX_Config.mode;
						break;
		
		//--- EMPTY ------------------------------------------------
		case ctrl_empty:
		case ctrl_empty_step1:
						temp_ctrl_on(FALSE);
						_set_valve(TO_INK);
                        // _set_pump_speed((_PumpPID.val_max + 1) / 2);
						_pump_pid();
						max_pressure = MBAR_500;
                        RX_Status.mode = RX_Config.mode;
						break;
        
		case ctrl_empty_step2:
						_pump_pid();
						max_pressure = MBAR_500;
						if (RX_Status.error & (COND_ERR_meniscus|COND_ERR_pump_no_ink))
						{
							temp_ctrl_on(FALSE);
							turn_off_pump();
							RX_Status.mode = RX_Config.mode;
						}
                        break;
		
		//--- FILL -------------------------------------------------------
		case ctrl_fill:	
		case ctrl_fill_step1:	
						temp_ctrl_on(FALSE);
						turn_off_pump();
						max_pressure = MBAR_100;
                        RX_Status.mode = RX_Config.mode;
						break;
		
		case ctrl_fill_step2:
        case ctrl_fill_step3:  
						temp_ctrl_on(FALSE);
						_set_valve(TO_INK);
						_pump_pid();
						max_pressure = MBAR_500;
						if (RX_Config.mode==ctrl_fill_step2 
						||  (RX_Config.mode==ctrl_fill_step3 && RX_Status.pressure_in!=INVALID_VALUE && RX_Status.pressure_in>0)) 
							RX_Status.mode = RX_Config.mode;
						break;
                
		//--- Shut Down -------------------------------------------------------
		case ctrl_shutdown:               
						temp_ctrl_on(FALSE);
                        if (RX_Config.mode != RX_Status.mode)
                            _sd_pump_time = SD_PUMP_TIME;
                        
                        if (--_sd_pump_time > 0)
                            _set_pump_speed((_PumpPID.val_max + 1) / 2);
                        else
                            turn_off_pump();
                        
						temp_ctrl_on(FALSE);
                        RX_Status.mode = RX_Config.mode;
						break;
                        
        //--- SENSOR CALIBRATION ------------------------------------------------
        case ctrl_offset_cal:
						temp_ctrl_on(FALSE);
						turn_off_pump();
						if (RX_Status.mode!=ctrl_offset_cal && RX_Status.mode!=ctrl_offset_cal_done) 
						{
							pres_calibration_start();
						}
						if (pres_calibration_done()) RX_Status.mode = ctrl_offset_cal_done;
						else 						 RX_Status.mode = ctrl_offset_cal;
                        break;    
                        
		default:				
                        turn_off_pump();
                        temp_ctrl_on(FALSE);
						break; 
	}
	_prev_mode = RX_Config.mode;
        
    // machine safety
	//--- check input sensor in all modes -------
	if (pres_valid()
    &&  max_pressure > 0
    &&  RX_Status.pressure_in != INVALID_VALUE
    &&  RX_Status.pressure_in > max_pressure)
	{
		//  turn_off_pump(); // emergency mode
        RX_Status.error |= COND_ERR_p_in_too_high;   
		return;
	}
}

//--- _set_valve --------------------------------------
static void _set_valve(int state)
{
	RX_Status.info.valve = state;
	Gpio1pin_Put(GPIO1PIN_P12, state);
}

//--- _menicus_minmax_reset ------------------------------
static void _menicus_minmax_reset(void)
{
	_meniscusMinMaxTimer = 0;
	RX_Status.meniscus_diff = INVALID_VALUE;
	_meniscus_min = INVALID_VALUE;
	_meniscus_max = INVALID_VALUE;
	
	RX_Status.pressure_in_diff = INVALID_VALUE;
	_pin_min	  = INVALID_VALUE;
	_pin_max	  = INVALID_VALUE;

	RX_Status.pressure_out_diff = INVALID_VALUE;
	_pout_min	  = INVALID_VALUE;
	_pout_max	  = INVALID_VALUE;
}

//--- _menicus_minmax ---------------------------------
static void _menicus_minmax(void)
{
	if (++_meniscusMinMaxTimer>MENISCUS_CHECK_TIME)
	{
		if (_pin_min==INVALID_VALUE) RX_Status.pressure_in_diff = INVALID_VALUE;
		else RX_Status.pressure_in_diff = _pin_max-_pin_min;		
		_pin_min	  = INVALID_VALUE;
		_pin_max	  = INVALID_VALUE;

		if (_pout_min==INVALID_VALUE) RX_Status.pressure_out_diff = INVALID_VALUE;
		else RX_Status.pressure_out_diff = _pout_max-_pout_min;		
		_pout_min	  = INVALID_VALUE;
		_pout_max	  = INVALID_VALUE;

		if (_meniscus_min==INVALID_VALUE) RX_Status.meniscus_diff = INVALID_VALUE;
		else RX_Status.meniscus_diff = _meniscus_max-_meniscus_min;
		_meniscus_min = INVALID_VALUE;
		_meniscus_max = INVALID_VALUE;

		_meniscusMinMaxTimer=0;
	}
	
	if (valid(RX_Status.pressure_in))
	{
		if (_pin_min==INVALID_VALUE || RX_Status.pressure_in<_pin_min) _pin_min = RX_Status.pressure_in;
		if (_pin_max==INVALID_VALUE || RX_Status.pressure_in>_pin_max) _pin_max = RX_Status.pressure_in;
	}
	
	if (valid(RX_Status.pressure_out))
	{
		if (_pout_min==INVALID_VALUE || RX_Status.pressure_out<_pout_min) _pout_min = RX_Status.pressure_out;
		if (_pout_max==INVALID_VALUE || RX_Status.pressure_out>_pout_max) _pout_max = RX_Status.pressure_out;
	}
	
	if (valid(RX_Status.meniscus))
	{
		if (_meniscus_min==INVALID_VALUE || RX_Status.meniscus<_meniscus_min) _meniscus_min = RX_Status.meniscus;
		if (_meniscus_max==INVALID_VALUE || RX_Status.meniscus>_meniscus_max) _meniscus_max = RX_Status.meniscus;
	}
}

//--- _error_cnt ------------------------------------
static void _error_cnt(int err, int *errcnt, int errflag)
{
	if (err) 
		(*errcnt)++;
	else	 
		(*errcnt)--;
				
	if ((*errcnt) > MENISCUS_TIMEOUT)
	{
		if (RX_Config.mode==ctrl_empty_step2) RX_Status.mode = ctrl_empty_step2;
		else RX_Status.error |= errflag;
		(*errcnt) = 0;
	}
	else if ((*errcnt)<0) (*errcnt)=0;
}

//--- _pump_pid --------------------------------------
static void _pump_pid(void)
{       
    static const INT32 MENISCUS_MAX = 1;    // [0.1mbar]
 
	_watchdog_check();
		
	if (RX_Status.pressure_out == INVALID_VALUE || RX_Status.info.valve == TO_FLUSH || RX_Status.error & (COND_ERR_meniscus|COND_ERR_pump_no_ink))
    {
		turn_off_pump();
    }
	else
	{   			
		_menicus_minmax();

        // meniscus shall never be positive while in PRINT mode		
        if (!RX_Config.cmd.disable_meniscus_check 
		&&  !(RX_Config.mode>=ctrl_fill && RX_Config.mode<ctrl_fill+10)
		&&  !(RX_Config.mode>=ctrl_cal_step1 && RX_Config.mode<ctrl_cal_done)
		)
        {        
			int flow = RX_Status.pump_measured * 60 / 1000;
			if (RX_Config.mode==ctrl_empty_step2)	_error_cnt((flow > 80), &_no_ink_err_cnt,   COND_ERR_pump_no_ink);
			else 									_error_cnt((flow > 80), &_no_ink_err_cnt,   COND_ERR_pump_no_ink);
			_error_cnt((RX_Status.meniscus > MENISCUS_MAX), 	&_meniscus_err_cnt, COND_ERR_meniscus);			
        }

		if (FALSE)
		{ // regulate on meniscus
			INT32 meniscus_target = -RX_Config.pressure_out; // read from presout.xml     
			pid_calc(abs(RX_Status.meniscus), abs(meniscus_target), &_PumpPID);
		}
		else
		{	// regulate on p_out
			pid_calc(abs(RX_Status.pressure_out), RX_Config.pressure_out, &_PumpPID);
			_set_pump_speed(_PumpPID.val);
        }
	
		// --- log ------------------------------
		{
			int i = RX_Status.logCnt%SIZEOF(RX_Status.log);
			RX_Status.log[i].no			  = (INT16)RX_Status.logCnt;
			RX_Status.log[i].pressure_in  = (INT16)RX_Status.pressure_in;
			RX_Status.log[i].pressure_out = (INT16)RX_Status.pressure_out;
			RX_Status.log[i].meniscus     = (INT16)RX_Status.meniscus;
			RX_Status.log[i].pump	      = (INT16)RX_Status.pump;
			RX_Status.log[i].pump_ticks   = _pump_ticks;
			RX_Status.logCnt++;
		}
	}
}

//--- _set_pump_speed --------------------------------
static void _set_pump_speed(UINT32 speed)
{
	if (speed != INVALID_VALUE)
	{
		FM4_DAC->DADR0_f.DA = speed;
		// RX_Status.pump = speed;
        // report actual pump speed, not in percent anymore
		RX_Status.pump  = (100 * speed) / _PumpPID.val_max;
	}
}

//--- turn_off_pump --------------------------------
void turn_off_pump(void)
{
    _set_valve(TO_FLUSH);
    _set_pump_speed(0);
	_menicus_minmax_reset();
}

//--- pump_tick_1000ms -------------------
void pump_tick_1000ms(void)
{           
	if (RX_Status.pump_measured) 
	{
		RX_Status.pumptime++;

        // Only save pump time to EEPROM, Flash write takes too long
        if ((RX_Status.pumptime % 60) == 0)
            save_pumptime(RX_Status.pumptime, FALSE);
	}
	            
    /* 
     * calculate how much the pump is currently promoting
     * (measured Impulses * 50 [ml/min]) / (2300 [1/min] * 6 Imp) = 3.6957 [ul/Imp]
     */
    RX_Status.pump_measured = _flow_cnt * 50 * 1000 / 2300 / 6; // in [ul/s]
    _flow = 0;
    _flow_cnt = 0;
}

//---  _InputCapture00_Init ------------------------
void _InputCapture00_Init(void)
{	
    // old Pin 22, P38, IC00_0
	// new Pin  2, P54, IC02_0
    
	FM4_GPIO->PFR5_f.P54=1;				// use peripheral function not GPIO

	FM4_GPIO->EPFR01_f.IC02S=0x00;		// use ic02_0
	
    FM4_MFT0_FRT->TCCP0 = 0xFFFF;   	// set value FRT ch.0
    FM4_MFT0_FRT->TCSA0 = 0x17;     	// 0x...0   160MHz / 128 = 1,25MHz
                                        // 0x..1. start timer
      
    //FM4_MFT0_ICU->ICFS10 = 0;     	// Connects FRT ch.0 to ICU ch.0
	FM4_MFT0_ICU->ICFS32_f.FSI00= 0;	// Connects FRT ch.0 to ICU ch.2
	FM4_MFT0_ICU->ICFS32_f.FSI01= 0;
	FM4_MFT0_ICU->ICFS32_f.FSI02= 0;
	FM4_MFT0_ICU->ICFS32_f.FSI03= 0;
	//FM4_MFT0_ICU->ICSA10 = 0x13;      // Enables the operation of ICU ch.(0). 
                                        // Handles both the rising and falling edges 
                                        // of IC(0) signal input as valid edges. 
                                        // Generates interrupt, when "1" is set to ICP0. 
//	FM4_MFT0_ICU->ICSA32 = 0x13;		// Enables the operation of ICU ch.2
	FM4_MFT0_ICU->ICSA32_f.EG00=1;		// falling edge enable
	FM4_MFT0_ICU->ICSA32_f.EG01=1;		// rising edge enable
	FM4_MFT0_ICU->ICSA32_f.ICE0=1;		// Enable interrupt

    /* enable interrupt */
    NVIC_ClearPendingIRQ(ICU0_IRQn); 
    NVIC_EnableIRQ(ICU0_IRQn);
    NVIC_SetPriority(ICU0_IRQn, 5);
}

/**
 ******************************************************************************
 ** \brief  Input Capture 0 IRQ handle.
 **
 ******************************************************************************/
void ICU0_IRQHandler(void) 
{
    UINT16 iccp = 0;    
	UINT16 diff = 0;
    static int _first        = TRUE;
    static UINT16 _last_iccp = 0;
    
    // use the same clock as PCLK = 40MHz
    
    if (bFM4_MFT0_ICU_ICSA32_ICP0 == 1)
    {
		if(FM4_GPIO->PDIR5_f.P54 == 0)  
		{
			if (_first)
			{
				_last_iccp = FM4_MFT0_ICU->ICCP2;
				_first = FALSE;
			}
			else
			{
	            iccp = FM4_MFT0_ICU->ICCP2;
                diff = iccp - _last_iccp;
                _flow += diff;
                _last_iccp = iccp;  
                
	            _flow_cnt++;
				_pump_ticks++;
			}  
		}
		bFM4_MFT0_ICU_ICSA32_ICP0 = 0;
	}
}
