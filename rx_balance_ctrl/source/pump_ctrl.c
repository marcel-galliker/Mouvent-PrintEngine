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
#include "temp_ctrl.h"
#include "pres.h"
#include "pump_ctrl.h"

//--- defines --------------------
#define WATCHDOG_TIMEOUT		1000

#define PRESSURE_OUT_SET		280
#define PRESSURE_TOL			50

#define TO_FLUSH	FALSE
#define TO_INK		TRUE

//--- prototypes -----------------
static void _InputCapture00_Init(void);
void ICU0_IRQHandler(void);

static void _set_valve(int state);
static void _set_pump_speed(int speed);
static void _pump_pid(void);
	
static int		_watchdog;
static UINT32 	_pulse_count;
static uint32_t _round_count;
static int		_timer;



SPID_par _PumpPID = 
{
	.interval		= 10,		// Abtastzeit in ms (gegeben durch systick interrupt zeit)
	.I				=  1,		// Integralanteil
	.P				= 10,		// Verstärkung
	.D				=  3,		// Differenzieller Anteil
	
	.val_min		= 0,
	.val_max		= 0xfff, 	//max value for pump DAC voltage
								// Value for DAC, 0x0fff = 9.7V => max 0x0000 => 32mv => min
	.val_invalid	= 0,
	
	.diff_min		= 1,		// Ansprechschwelle
};

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
//	RX_Status.cond_error.bitset.watchdog = FALSE;
}

//--- pump_tick_10ms -------------------------------
void pump_tick_10ms(void)
{
	int cycle_time=10;
	static int max_pressure=0;	// save value in case of ctrl_off
	
	if (RX_Config.mode==ctrl_fw_download)
	{
		_set_valve(TO_FLUSH);
		_set_pump_speed(0);
		temp_ctrl_off();
		RX_Status.mode=RX_Config.mode;		
		return;
	}
	
	if (--_watchdog<=0)
	{
		_set_valve(TO_FLUSH);
		_set_pump_speed(0);
//		RX_Status.cond_error.bitset.watchdog = TRUE;
		RX_Status.mode=ctrl_off;
		return;		
	}
	
	/*
	if (RX_Status.pressure_in_act==INVALID_VALUE ||  RX_Status.pressure_out_act==INVALID_VALUE)
	{
		_set_valve(TO_FLUSH);
		_set_pump_speed(0);
		RX_Status.mode=ctrl_off;
		return;
	}
	*/
	
	/*
	//--- machine protection! ------------------------------------
	if (RX_Status.pressure_in_act>500)
	{
		_set_valve(TO_FLUSH);
		_set_pump_speed(0);
		RX_Status.cond_error.bitset.p_in_too_high=TRUE;
		RX_Status.mode=ctrl_off;
		return;
	}
	
	if (RX_Status.pressure_out_act>400)
	{
		_set_valve(TO_FLUSH);
		_set_pump_speed(0);
		RX_Status.cond_error.bitset.p_out_too_high=TRUE;
		RX_Status.mode=ctrl_off;
		return;
	}
	*/
	
	switch(RX_Config.mode)
	{
		case ctrl_off:	if (RX_Status.mode!=ctrl_off)_timer=100;
						if (--_timer<0) _set_valve(TO_FLUSH);
						_set_pump_speed(0);
						RX_Status.mode=ctrl_off;
						break;		

		case ctrl_print:
						_set_valve(TO_INK);
						_pump_pid();
						RX_Status.mode=RX_Config.mode;
						max_pressure = 100;
						break;
	
		//--- PURGE --------------------------------------------
		case ctrl_purge_soft:
		case ctrl_purge:
		case ctrl_purge_hard:
						_set_valve(TO_FLUSH);
						_set_pump_speed(0);
						_timer = RX_Config.purge_time;
						RX_Status.mode=RX_Config.mode;
						max_pressure = 500;
						break;
		
		case ctrl_purge_step1:
						_timer -= cycle_time;
						if (_timer>0)
						{
							_set_valve(TO_INK);
						}
						else
						{
							_set_valve(TO_FLUSH);
							RX_Status.mode=RX_Config.mode;
						}
						break;
		
		case ctrl_purge_step2:
						_pump_pid();
						_set_valve(TO_INK);
						if (TRUE) // pressure ok
						{
							RX_Status.mode=RX_Config.mode;
						}	
						break;
										
		//--- FLUSH ------------------------------------------------
		case ctrl_flush:
						_set_valve(TO_FLUSH);
						_set_pump_speed(0);
						RX_Status.mode=RX_Config.mode;
						max_pressure = 100;
						break;
		
		//--- EMPTY ------------------------------------------------
		case ctrl_empty:
		case ctrl_empty_step1:
		case ctrl_empty_step2:
						_set_valve(TO_INK);
						_pump_pid();
						RX_Status.mode=RX_Config.mode;	
						max_pressure = 500;
						break;
		
		case ctrl_empty_step3:
						_set_pump_speed (0);
						if (RX_Status.pressure_in<0)
							RX_Status.mode=RX_Config.mode;
						break;
										
		case ctrl_empty_step4:
						RX_Status.mode=RX_Config.mode;
						break;

		case ctrl_empty_step5:
						_set_valve(TO_FLUSH);
						RX_Status.mode=RX_Config.mode;
						break;
		
		//--- FILL -------------------------------------------------------
		case ctrl_fill:	
		case ctrl_fill_step1:	
						_set_valve(TO_FLUSH);
						_set_pump_speed (0);
						RX_Status.mode=RX_Config.mode;
						max_pressure = 100;
						break;
		
		case ctrl_fill_step2:
		case ctrl_fill_step4:
						_set_valve(TO_INK);
						_pump_pid();
						_timer = 5000/cycle_time;
						RX_Status.mode=RX_Config.mode;
						break;
	
		case ctrl_fill_step3:	// wait until output pressure stable for 5 sec
						pid_calc (-RX_Status.pressure_out, PRESSURE_OUT_SET, &_PumpPID);
						_set_pump_speed(_PumpPID.val);
						if (RX_Status.pressure_out>PRESSURE_OUT_SET+PRESSURE_TOL || RX_Status.pressure_out<PRESSURE_OUT_SET-PRESSURE_TOL)
						{
							_timer = 5000/cycle_time;
						}
						else _timer-=cycle_time;
						if (_timer<=0) 
							RX_Status.mode=RX_Config.mode;
						break;
		
		case ctrl_fw_download:
						_set_valve(TO_FLUSH);
						_set_pump_speed(0);
						temp_ctrl_off();
						RX_Status.mode=RX_Config.mode;
						break;
		
		default:				
						_set_valve(TO_FLUSH);
						_set_pump_speed(0);
						break; 
	}

	//--- check input sensor in all modes -------
	if (pres_valid() && RX_Status.pressure_in>max_pressure)
	{
		_set_valve(TO_FLUSH);
		_set_pump_speed(0);
//		RX_Status.cond_error.bitset.p_in_too_high=TRUE;
		RX_Status.mode=ctrl_off;
		return;
	}
}

//--- _set_valve --------------------------------------
static void _set_valve(int state)
{
	RX_Status.gpio_state.valve = state;
	Gpio1pin_Put(GPIO1PIN_P12, state);
}

//--- _pump_pid --------------------------------------
static void _pump_pid(void)
{
	if (RX_Status.pressure_out==INVALID_VALUE) 
		_set_pump_speed(0);
	else
	{
		pid_calc (-RX_Status.pressure_out, PRESSURE_OUT_SET, &_PumpPID);
		_set_pump_speed (_PumpPID.val);
	};
}

//--- _set_pump_speed --------------------------------
static void _set_pump_speed(int speed)
{
	if (speed!=INVALID_VALUE)
	{
		FM4_DAC->DADR0_f.DA = speed;
		RX_Status.pump  = (100*speed)/0xfff;
		/*
		if(speed<59) 		RX_Status.pump_act=0;
		else if(speed<=92)	RX_Status.pump_act=100;
		else				RX_Status.pump_act=100+(((speed-92)*3905)>>12);
		*/
	}
}

//--- pump_tick_2500ms -------------------
void pump_tick_2500ms(void)
{
	RX_Status.pump_measured=_round_count*24;
}

//---  _InputCapture00_Init ------------------------
void _InputCapture00_Init(void)
{
	return;
	
    // old Pin 22, P38, IC00_0
	// new Pin  2, P54, IC02_0
    
  //  bFM4_GPIO_PFR4_P40 = 1;         //use peripheral function not GPIO
	
	FM4_GPIO->PFR5_f.P54=1;				// use peripheral function not GPIO

	FM4_GPIO->EPFR01_f.IC02S=0x00;		//use ic02_0
	
    FM4_MFT0_FRT->TCCP0 = 0xFFFF;   	// set value FRT ch.0
    FM4_MFT0_FRT->TCSA0 = 0x17;     	//0x...0   160MHz / 128 = 1,25MHz
																			//0x..1. start timer
      
    //FM4_MFT0_ICU->ICFS10 = 0;     	//Connects FRT ch.0 to ICU ch.(0)
	FM4_MFT0_ICU->ICFS32_f.FSI00= 0;	//Connects FRT ch.0 to ICU ch.2
	FM4_MFT0_ICU->ICFS32_f.FSI01= 0;
	FM4_MFT0_ICU->ICFS32_f.FSI02= 0;
	FM4_MFT0_ICU->ICFS32_f.FSI03= 0;
	//FM4_MFT0_ICU->ICSA10 = 0x13;    //Enables the operation of ICU ch.(0). 
																			//Handles both the rising and falling edges 
																			//of IC(0) signal input as valid edges. 
																			//Generates interrupt, when "1" is set to ICP0. 
//	FM4_MFT0_ICU->ICSA32 = 0x13;		//Enables the operation of ICU ch.2
	FM4_MFT0_ICU->ICSA32_f.EG00=1;		//falling edge enable
	FM4_MFT0_ICU->ICSA32_f.EG01=1;		//rising edge enable
	FM4_MFT0_ICU->ICSA32_f.ICE0=1;		//Enable interrupt

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
volatile int test=0;
void ICU0_IRQHandler(void) 
{
    static uint16_t u16Start = 0;
    static uint16_t u16Stop = 0;
	uint16_t u16Data;
	
	test++;
    if (bFM4_MFT0_ICU_ICSA32_ICP0 == 1)
    {
        //if (0 == bFM4_GPIO_PDIR3_P38) // only return HIGH level length
		if(FM4_GPIO->PDIR5_f.P54==0)  
		{
			u16Stop = FM4_MFT0_ICU->ICCP2; 
			if (u16Start > u16Stop)
			{
				u16Data = (0xFFFF - u16Start) + u16Stop;
			}
			else
			{
				u16Data = u16Stop - u16Start;
			}

			if(_pulse_count>=6)
			{
				_pulse_count=0;
				_round_count++;
			}
			else
			{
				_pulse_count++;
			}
			}
			else
			{
				u16Start = FM4_MFT0_ICU->ICCP2; 
			}
			bFM4_MFT0_ICU_ICSA32_ICP0 = 0;
		}
    
    /* u16Data is (FM4_BT0_PWM->PDUT + 1) * 2, 
       because sampling rate is 2 times of generation of the signal */
}

