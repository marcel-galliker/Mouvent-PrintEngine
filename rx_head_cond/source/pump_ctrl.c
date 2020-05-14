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
#include "debug_printf.h"
#include "main.h"
#include "average.h"
#include "ctr.h"

//--- defines --------------------
#define IMPULSES_PER_REVOLUTION 6

#define WATCHDOG_TIMEOUT		500 // 50
#define COND_ALIVE_TIMEOUT_MS 	10000	// ms
#define RAMP_UP_TIME			500		// 5 Sec rampup time

#define TO_FLUSH	FALSE
#define TO_INK		TRUE

#define CALIBRATION_NB_VAL		30

static const INT32 MENISCUS_TIMEOUT 	= 500; //2000;
static const INT32 NO_INK_TIMEOUT 		= 1000; //2000;
static const INT32 MENISCUS_CHECK_TIME 	= 60*100;
static const INT32 MENISCUS_CHECK_DELAY = 30*100;

static const INT32 DEFAULT_P = 1000;
static const INT32 DEFAULT_I = 2000;
static const INT32 DEFAULT_SETPOINT = 150;

static const INT32 START_PID_OFF = 1;
static const INT32 START_PID_P_REDUCED = 2;
static const INT32 START_PID_P_NORMAL = 3;


//--- prototypes -----------------
static void _InputCapture00_Init(void);
void ICU0_IRQHandler(void);

static void _set_valve(int state);
static void _set_pump_speed(UINT32 speed);
static void _pump_pid(int Meniscus_Error_Enable);
static void _watchdog_check(void);
static void _menicus_minmax_reset(void);
static void _menicus_minmax(void);
static void _presure_in_max(void);
static void _error_cnt(int err, int *errcnt, int errflag, int timeout);

static int	_watchdog;
static int	_timer;
static int  _rampup_time;
static UINT32  	_meniscusMinMaxTimer=MENISCUS_CHECK_TIME;
static int		_menuscusMinMaxDelay=MENISCUS_CHECK_DELAY;
static INT32	_meniscus_min;
static INT32	_meniscus_max;
static INT32	_pin_min;
static INT32	_pin_max;
static INT32	_pout_min;
static INT32	_pout_max;
static INT32    _Start_PID;
static INT32	_TimePIDstable;
// static INT32	_TimeSwitchingOFF;
// static INT32 _PhaseOFFMeniscusPre;
static INT32	_Meniscus_Timeout;
static INT32	_PurgeDelay;
static INT32	_PurgeTime;
static INT32	_SetpointShutdown;

// ---- NEW : flow resistance  -----
// static INT32	_TimeFlowResistancestablePRINT;
// static int	_NbAverageFlowResistancePRINT;
// static int	_SumFlowResistancePRINT;
// static UINT32	_PumpMeasuredStablePRINT;
// static INT32	_TabFlowResistance[100];

// ---- NEW : pump measured  -----
// static UINT32 	_PumpMeasuredFlowCnt = 0;
// static UINT32 	_PumpMeasuredFlow = 0;
// static UINT32 	_NbPulsesCnt = 0;
// static UINT32 	_SumTimePulses = 0;
// static INT32	_TabAvgSpeedPump[50];
// static INT32    _NbAvgPumpSpeed;
// static INT32    _SumPumpSpeed;

// --- shutdown phase ---
static INT32	_ShutdownPrint;
static INT32	_TimeMeniscusStability;
static INT32	_NbPresShutdown;

// --- Calibration ---
/*static INT32 	_MaxDerivPoutCalibration;						
static INT32 	_Nb100msCalibration;
static INT32 	_NbAvgCalibration;
static INT32 	_NbPresCalibration;
static INT32 	_AvgPoutCalibration;
static INT32 	_TimeSatbilityCalibration;
static INT32 	_ThresholdDetectOKCalibration;
static INT32 	_TimeOutCalibration;
static INT32 	_TabAvgPoutCalibration[CALIBRATION_NB_VAL];
static INT32 	_TabDerivPoutCalibration[CALIBRATION_NB_VAL];
static INT32 	_PoutSTEP1Calibration;
static INT32 	_PoutMaxSTEP2Calibration;
static INT32	_PinSTEP2Calibration;
static INT32	_DelatPSTEP2Calibration;
static INT32 	_PinAvgCalibration;
static INT32 	_PoutAvgCalibration;
static INT32 	_SumAvgPoutCalibration;
static INT32 	_ThresholdDetectOKCalibration;
static INT32 	_PinCalibration;
static INT32 	_PoutCalibration; */

// --- Check ---
static INT32 	_TimeStabilityCheck;
static INT32	_CheckSequence;
static INT32	_CheckStep4_Pin;
static INT32	_CheckStep4_Pout;
static INT32	_CheckStep4_Meniscus;
// static INT32	_CheckStep4_Pump;
// static INT32	_CheckStep4_PumpMeasured;

static UINT32 _flow = 0;
// --- END NEW ---
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
	.Setpoint		= DEFAULT_SETPOINT,		// 150
	.P				= DEFAULT_P,			// 1000
	.I				= DEFAULT_I,			// 2000
	.P_start		= 1,
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
	RX_Status.pressure_in_max=INVALID_VALUE;
}

//--- pump_watchdog --------------------------------
void pump_watchdog(void)
{
	_watchdog = WATCHDOG_TIMEOUT;
}

//--- _watchdog_check --------------------------------
static void _watchdog_check(void)
{
	static UINT32 	_alive=0;
	static int		_aliveTimeout;
	if ((_watchdog > 0) && (--_watchdog <= 0)) 								
        RX_Status.error |= COND_ERR_pump_watchdog;
		
	//--- alive test -----------
	if (RX_Config.alive && !(RX_Status.error & COND_ERR_alive)) 
	{
		if (RX_Config.alive != _alive)
		{
			_alive 			= RX_Config.alive;
			_aliveTimeout 	= COND_ALIVE_TIMEOUT_MS;
		}
		_aliveTimeout -= 10;
		if (_aliveTimeout<0)
		{			
			RX_Status.aliveStat = RX_Status.alive;
			RX_Status.aliveCfg 	= RX_Config.alive;
			RX_Status.error |= COND_ERR_alive;
		}
	}
	
    #ifndef DEBUG
//    if (RX_Status.error & (COND_ERR_pump_watchdog | COND_ERR_alive))
    if (RX_Status.error & COND_ERR_pump_watchdog)
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
	static const INT32 cycle_time    = 10;
    
	INT32  max_pressure = MBAR_500; // save value in case of ctrl_off
	
	_watchdog_check();

	if (RX_Status.pressure_out == INVALID_VALUE) // || RX_Status.pressure_in == INVALID_VALUE)
	{
		RX_Status.error |= COND_ERR_pres_out_hw;
		turn_off_pump();
		temp_ctrl_on(FALSE);
		RX_Status.mode=ctrl_off;
		return;
	}
	
	//--- PID Values --------------------------------
	
	//if(RX_Config.meniscus_setpoint > 50) _PumpPID.Setpoint = RX_Config.meniscus_setpoint;
	//else _PumpPID.Setpoint	= DEFAULT_SETPOINT;
	
	RX_Status.pid_P 			= _PumpPID.P;	
	RX_Status.pid_I 			= _PumpPID.I;
	RX_Status.meniscus_setpoint	= _PumpPID.Setpoint;
	
	//--- END PID Values -------------------------
	
	switch(RX_Config.mode)
	{
		case ctrl_shutdown:	

				if(_ShutdownPrint > 0)					
				{					
					_TimeMeniscusStability = 0;
					_NbPresShutdown = 0;
					_PumpPID.Setpoint = -RX_Status.meniscus;
				}
				if(RX_Config.meniscus_setpoint < 180) _SetpointShutdown = 100;		// WB machine setpoint -10 mbars
				else _SetpointShutdown = 180;										// UV machine setpoint -18 mbars (to avoid leakage on the paper after 1 night OFF)
				RX_Status.mode = RX_Config.mode;
				break;
		
		case ctrl_shutdown_done:	
		
				// Bring Meniscus to Setpoint (WF) + 15mbars
				if((_ShutdownPrint > 0)&&(RX_Status.info.valve == TO_INK))	// valve on TO_FLUSH if Error detected, so no shutdown phase
				{
					_ShutdownPrint++;
					_NbPresShutdown++;
					if(_NbPresShutdown > 10)
					{
						if(_PumpPID.Setpoint > _SetpointShutdown) _PumpPID.Setpoint --;
						else _PumpPID.Setpoint = _SetpointShutdown;
						_NbPresShutdown = 0;
					}
					_pump_pid(FALSE);	// disable meniscus error
					if(abs(RX_Status.meniscus) < 20)
					{
						_TimeMeniscusStability++;
						if(_TimeMeniscusStability > 300) RX_Status.mode = RX_Config.mode; 
					}
					else if(_PumpPID.val == _PumpPID.val_min)
					{
						_TimeMeniscusStability++;
						if(_TimeMeniscusStability > 2000) RX_Status.mode = RX_Config.mode;
						pid_reset(&_PumpPID);						
					}
					else _TimeMeniscusStability = 0;
					// Timeout
					if(_ShutdownPrint > 4500) RX_Status.mode = RX_Config.mode;
				}
				else RX_Status.mode = RX_Config.mode;
				break;
		
		case ctrl_off:
		case ctrl_undef:
		case ctrl_error:
						temp_ctrl_on(FALSE);
						turn_off_pump();		
						RX_Status.logCnt = 0;
						_pump_ticks = 0;
						if (RX_Status.mode > ctrl_off) ctr_save();
									
						pid_reset(&_PumpPID);			
						_PumpPID.start_integrator = 0;
						_PumpPID.Setpoint = RX_Config.meniscus_setpoint;
						_Start_PID = START_PID_OFF;
						_TimePIDstable = 0;
//						_TimeSwitchingOFF = 0;
						_Meniscus_Timeout = MENISCUS_TIMEOUT;
						_ShutdownPrint = 0;
					
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
                            _pump_pid(TRUE);
						else 
                            RX_Status.mode=ctrl_readyToPrint;	// ???
                        		
                        RX_Status.mode = RX_Config.mode; 		// ???
						break;        
			
        //--- PRINT --------------------------------------------
		case ctrl_print:  		
						//--- start: reset regulation -----------------
						if (RX_Config.mode != RX_Status.mode)
						{
							_PumpPID.P 			= DEFAULT_P;
							_PumpPID.I 			= DEFAULT_I;
							
							// Calculate the P reduced factor depending of number of heads per color
							// NbHeads = 1 or 4 : P_start = 2 (P divide by 2)
							// NbHeads = 8 : P_start = 3
							// NbHeads = 12 : P_Start = 4
							if(RX_Config.headsPerColor < 8) _PumpPID.P_start	= 2;
							if(RX_Config.headsPerColor == 8) _PumpPID.P_start	= 3;
							else _PumpPID.P_start	= 4;
							_Start_PID = START_PID_P_REDUCED;
							_PumpPID.start_integrator = 0;
							// Meniscus setpoint in WF (if 0, setpoint=default)
							_PumpPID.Setpoint = RX_Config.meniscus_setpoint;
							if(_PumpPID.Setpoint < 50) _PumpPID.Setpoint = DEFAULT_SETPOINT;
							pid_reset(&_PumpPID);
							RX_Status.pressure_in_max=INVALID_VALUE;
							RX_Status.error  &= ~(COND_ERR_meniscus | COND_ERR_pump_no_ink);
							_meniscus_err_cnt=0;
							_no_ink_err_cnt  =0;		
						//	_TimeFlowResistancestablePRINT = 0;							
						}
						
						temp_ctrl_on(TRUE);
						_set_valve(TO_INK);
                        max_pressure = MBAR_500;
						_ShutdownPrint = 1;
        
						_pump_pid(TRUE);
						
						RX_Status.mode = RX_Config.mode; 		
                        break;
        //--- CALIBRATION --------------------------------------------
	/*	case ctrl_cal_start:	// Initialize variables
						
						RX_Status.mode = RX_Config.mode; 						
						temp_ctrl_on(TRUE);
						_set_valve(TO_INK);
						_PumpPID.start_integrator = 0;
						pid_reset(&_PumpPID);
						_ShutdownPrint = 1;
						
						_MaxDerivPoutCalibration = -1000;						
						_Nb100msCalibration = 0;
						_NbAvgCalibration = 0;
						_NbPresCalibration = 0;
						_AvgPoutCalibration = 0;
						_TimeSatbilityCalibration = 0;
						_ThresholdDetectOKCalibration = 100;
						_TimeOutCalibration = 0;
						for(int i=0;i<CALIBRATION_NB_VAL;i++) 
						{
							_TabAvgPoutCalibration[i] = 0;
							_TabDerivPoutCalibration[i] = 0;
						}
						_PumpPID.Setpoint = RX_Config.meniscus_setpoint - 50;
						
						break;
		
		case ctrl_cal_step1:	// Regulate meniscus at setpoint +5mbars, until meniscus and temperature stable			
						
						_pump_pid(FALSE);
						if((abs(RX_Status.meniscus + _PumpPID.Setpoint) < 10)&&(abs(RX_Config.temp - RX_Config.tempHead) < 1000))
						{
							_TimeSatbilityCalibration++;
							if(_TimeSatbilityCalibration > 3000) RX_Status.mode = RX_Config.mode;		
						}
						if(RX_Status.mode == RX_Config.mode) _TimeSatbilityCalibration = 0;
						break;						
			
		case ctrl_cal_step2:  // Fix pump speed 40% and wait for 30 seconds
			
						_TimeSatbilityCalibration++;
						if(_TimeSatbilityCalibration > 3000) RX_Status.mode = RX_Config.mode;							 
						_PoutSTEP1Calibration = RX_Status.pressure_out;
							
						// For the calculation of Pinlet setpoint
						_PinSTEP2Calibration = RX_Status.pressure_in;
						_DelatPSTEP2Calibration = RX_Status.pressure_in - RX_Status.pressure_out;
		
						pid_reset(&_PumpPID);
						
		case ctrl_cal_step3: // IS decrease Pinlet, look for derivate peak and calculate new  flow resistance factor
						
						// Remain pump stable at 40%
						//----------------------------
						if(RX_Status.mode != ctrl_cal_step3) _set_pump_speed(_PumpPID.val_max * 41 / 100);
						else _pump_pid(FALSE);						
		
						// Find time when air is sucked
						// Sliding Average of (Pin - Pout) during 3 seconds
						// Derivative during 1 second
						_Nb100msCalibration++;
						_PinAvgCalibration 	+= RX_Status.pressure_in;
						_PoutAvgCalibration += RX_Status.pressure_out;
						if(_Nb100msCalibration > 9) 	// every 100ms enough
						{							
							_PinAvgCalibration /= _Nb100msCalibration;
							_PoutAvgCalibration /= _Nb100msCalibration;
							_Nb100msCalibration = 0;
							
							// Average 3 seconds
							_NbAvgCalibration++;
							if(_NbAvgCalibration > CALIBRATION_NB_VAL - 1) _NbAvgCalibration = 0;
							_TabAvgPoutCalibration[_NbAvgCalibration] = - _PoutAvgCalibration;
							
							_SumAvgPoutCalibration = 0;
							for(int i=0;i<CALIBRATION_NB_VAL;i++) _SumAvgPoutCalibration += _TabAvgPoutCalibration[i];
							_AvgPoutCalibration = _SumAvgPoutCalibration / 10;
							
							// Derivative 1 second
							if(RX_Config.mode == ctrl_cal_step3) RX_Status.deriv_calibration = _AvgPoutCalibration - _TabDerivPoutCalibration[_NbAvgCalibration];
							else RX_Status.deriv_calibration = 0;
							_TabDerivPoutCalibration[_NbAvgCalibration] = _AvgPoutCalibration;	
														
							// Look for Max Deriv, wait 1 second after to validate the MAX
							if(RX_Config.mode == ctrl_cal_step3)
							{
								// In case of deriv not enough high, ask for PURGE and restart
								_TimeOutCalibration++;
								if((_TimeOutCalibration > 500)&&(_NbPresCalibration < 3))
								{
									RX_Status.flowResistance = RX_Config.flowResistance;
									RX_Status.pout_air_ignition = 1200;									
									RX_Status.mode = RX_Config.mode;
								}
								switch(_NbPresCalibration)
								{
									case 0 : 	if(RX_Status.deriv_calibration > 100) 
												{
													_NbPresCalibration++; 
													_MaxDerivPoutCalibration = RX_Status.deriv_calibration;
												}
												_TimeSatbilityCalibration = 0;
												_PoutMaxSTEP2Calibration = _PoutSTEP1Calibration;
												break;
									case 1 :	// Detect end of deriv detection : 7.5 seconds after last peak > 200
												_ThresholdDetectOKCalibration = _MaxDerivPoutCalibration * 0.3;
												if(_ThresholdDetectOKCalibration < 50) _ThresholdDetectOKCalibration = 50;
										
												if(RX_Status.deriv_calibration < _ThresholdDetectOKCalibration) _TimeSatbilityCalibration++;
												else 
												{
													_TimeSatbilityCalibration = 0;
													if(RX_Status.deriv_calibration > 100)
													{
														if(RX_Status.deriv_calibration > _MaxDerivPoutCalibration) 									
														{
															_MaxDerivPoutCalibration = RX_Status.deriv_calibration;
															_PinCalibration = _PinAvgCalibration;
															_PoutCalibration = _PoutAvgCalibration;															
														}																													
													}													
												}
												
												if(_TimeSatbilityCalibration > 50)
												{
													_NbPresCalibration++;
													//RX_Status.FlowResist_Pin = _PinFlowResistance;
													RX_Status.pout_air_ignition = _PoutCalibration;
													RX_Status.flowResistance = (100 * (_PinCalibration - _PoutCalibration)) / (_PinCalibration + (RX_Config.meniscus_setpoint + 150));													
													RX_Status.pinlet_spt_calibration = _PinSTEP2Calibration - (_DelatPSTEP2Calibration / RX_Config.flowResistance) + (_DelatPSTEP2Calibration / RX_Status.flowResistance);
												}
													
												if(RX_Status.pressure_out < _PoutMaxSTEP2Calibration)
													_PoutMaxSTEP2Calibration = RX_Status.pressure_out;
												
												break;
									default : 	_NbPresCalibration++;
												// Deriv should never go below 0, so a problem appear and I restart the sequence												
												if(_NbPresCalibration> 10) RX_Status.mode = RX_Config.mode; 
												_PumpPID.start_integrator = 0;
												pid_reset(&_PumpPID);
												_TimeSatbilityCalibration = 0;
												// if diff between Pout STEP1 and Pout MAX is too low, the derivate is not good : meniscus setpoint too close to air sucks point
												if(_NbPresCalibration == 3)
												{
													if(abs(_PoutSTEP1Calibration - _PoutMaxSTEP2Calibration) < 100)
													{
														RX_Status.flowResistance = RX_Config.flowResistance;
														RX_Status.pout_air_ignition = 1000;
													}
													else if((RX_Status.flowResistance < 130)||(RX_Status.flowResistance > 500))
													{
														RX_Status.flowResistance = RX_Config.flowResistance;
														RX_Status.pout_air_ignition = 1100;
													}
												}
												break;
								}
							}
							else _NbPresCalibration = 0;
							_PinAvgCalibration = 0;
							_PoutAvgCalibration = 0;
						}
					
						break; 	
		case ctrl_cal_step4: 	// regulate meniscus to remove air in the circuit
						
						_AvgPoutCalibration = 0;						
						_NbPresCalibration++;
						if(_NbPresCalibration > 3)
						{
							if(_PumpPID.Setpoint > RX_Config.meniscus_setpoint) _PumpPID.Setpoint--;
							else if(_PumpPID.Setpoint < RX_Config.meniscus_setpoint) _PumpPID.Setpoint++;
							_NbPresCalibration = 0;
						}
						
						_pump_pid(FALSE);
						
						if((abs(RX_Status.meniscus + _PumpPID.Setpoint) < 10)&&(_PumpPID.Setpoint == RX_Config.meniscus_setpoint))
						{
							_TimeSatbilityCalibration++;
							if(_TimeSatbilityCalibration > 100) RX_Status.mode = RX_Config.mode; 
							_NbAvgCalibration = 0;
							//RX_Status.pinlet_spt_calibration = RX_Status.pressure_in;
						}
						else _TimeSatbilityCalibration = 0; 
						break;
						
		case ctrl_cal_done: 
						RX_Status.mode = RX_Config.mode;				
						break;
        */
		
		//--- Diangnostics --------------------------------------------
		case ctrl_check_step0:	RX_Status.mode = RX_Config.mode; break;
		case ctrl_check_step1:	RX_Status.mode = RX_Config.mode; break;
		case ctrl_check_step2:	RX_Status.mode = RX_Config.mode; break;
	/*	case ctrl_check_step3:	RX_Status.mode = RX_Config.mode; break;
		case ctrl_check_step4:	RX_Status.mode = RX_Config.mode; break;
		case ctrl_check_step5:	RX_Status.mode = RX_Config.mode; break;
		case ctrl_check_step6:	RX_Status.mode = RX_Config.mode; break;
		case ctrl_check_step7:	RX_Status.mode = RX_Config.mode; break;
		case ctrl_check_step8:	RX_Status.mode = RX_Config.mode; break;
		case ctrl_check_step9:	RX_Status.mode = RX_Config.mode; break;		*/  
		case ctrl_check_step3:	
						RX_Status.mode = RX_Config.mode; 						
											
						_PumpPID.start_integrator = 0;
						pid_reset(&_PumpPID);						
						_PumpPID.Setpoint = RX_Config.meniscus_setpoint;
						_TimeStabilityCheck = 0;
						RX_Status.mode = RX_Config.mode; 
						_CheckSequence = 1;
						_PumpPID.val_max = 3000;	// max 75% to avoid high pressure in case of return tube is clogged		 
						break;
		
		case ctrl_check_step4:	
						_set_valve(TO_INK);
						RX_Status.mode = RX_Config.mode;						
						if((_CheckSequence == 0)||(RX_Config.fluidErr))
						{
							_set_valve(TO_FLUSH);
							turn_off_pump();							 
						}
						else _pump_pid(FALSE);
						
						_TimeStabilityCheck++;
						switch(_CheckSequence)
						{
							case 1 : // Save pressures and pump after 2 seconds
								if(_TimeStabilityCheck > 200)
								{
									_CheckStep4_Pin 			= RX_Status.pressure_in;
									_CheckStep4_Pout 			= RX_Status.pressure_out;
									_CheckStep4_Meniscus 		= RX_Status.meniscus;
								//	_CheckStep4_Pump 			= RX_Status.pump;
								//	_CheckStep4_PumpMeasured 	= RX_Status.pump_measured;
									_CheckSequence++;
								}
								break;
								
							case 2 : // detect pump > 75% for 5 seconds and Pout increase and No fluid board error
								if((_PumpPID.val == _PumpPID.val_max)&&(RX_Status.pressure_out > _CheckStep4_Pout)&&(RX_Config.fluidErr == 0))
								{
									// RETURN tube clogged
									_TimeStabilityCheck++;
									
									if(_TimeStabilityCheck > 500)
									{
										RX_Status.error |= COND_ERR_return_pipe;
										_CheckSequence = 0;
									}
								}
								else _TimeStabilityCheck = 0;
						}	
						
						break;
						
		case ctrl_check_step5:
						if((_CheckSequence != 0)&&(RX_Config.fluidErr == 0)&&(RX_Status.error == 0))
						{
							_pump_pid(FALSE);
		
							// Valve not on INK
							if((RX_Status.meniscus < _CheckStep4_Meniscus)&&(RX_Status.pressure_in < _CheckStep4_Pin))
								RX_Status.error |= COND_ERR_valve;
							// Pump problem if command - measure > 25%
							else if(abs(RX_Status.pump_measured - RX_Status.pump * 10) > 250)
								RX_Status.error |= COND_ERR_pump_hw;
							// Pinlet sensor defected
							else if(abs(RX_Status.pressure_in - _CheckStep4_Pin) < 50)
								RX_Status.error |= COND_ERR_pres_in_hw;
							// Pinlet sensor defected
							else if(abs(RX_Status.pressure_out - _CheckStep4_Pout) < 50)
								RX_Status.error |= COND_ERR_pres_out_hw;
						}				
						RX_Status.mode = RX_Config.mode; 
						break;
		
		case ctrl_check_step6:	
		case ctrl_check_step7:	
		case ctrl_check_step8:	
		case ctrl_check_step9:	 
						if((RX_Status.error == 0)&&(RX_Config.fluidErr == 0)) 
						{
							_pump_pid(FALSE);
							_ShutdownPrint = 1;
						}
						else
						{
							temp_ctrl_on(FALSE);
							turn_off_pump();
							_ShutdownPrint = 0;
						}
						_PumpPID.val_max = 4095;	
						RX_Status.mode = RX_Config.mode; 
						break;		  
						
        //--- PURGE --------------------------------------------
		case ctrl_purge_soft:
		case ctrl_purge:
		case ctrl_purge_hard:
		case ctrl_purge_hard_wipe:
		case ctrl_purge_hard_vacc:
						temp_ctrl_on(FALSE);
						turn_off_pump();
						RX_Status.pressure_in_max=INVALID_VALUE;
						max_pressure = MBAR_500;
						pid_reset(&_PumpPID);
						RX_Status.mode = RX_Config.mode;
						break;
		
		case ctrl_purge_step1:
		case ctrl_purge_step2:
		case ctrl_purge_step3:
						temp_ctrl_on(FALSE);
						turn_off_pump();
						_presure_in_max();
						max_pressure = MBAR_500;
						_PurgeDelay = 0;
						_PurgeTime  = 0;
                        RX_Status.mode = RX_Config.mode;
						break;
		
		case ctrl_purge_step4:
						_presure_in_max();
						if (_PurgeDelay<RX_Config.purgeDelay || _PurgeTime>RX_Config.purgeTime)
						{
							_PurgeDelay+=cycle_time;
							temp_ctrl_on(FALSE);
							_set_valve(TO_FLUSH);
						}
						else
						{
							_PurgeTime+=cycle_time;
							temp_ctrl_on(TRUE);
							_set_valve(TO_INK);
						}
						_set_pump_speed(0);
						max_pressure = MBAR_500;
						RX_Status.mode = RX_Config.mode;
						break;
		/*
		case ctrl_purge_step5:
						temp_ctrl_on(FALSE);
						turn_off_pump();
						_presure_in_max();
						max_pressure = MBAR_500;
                        RX_Status.mode = RX_Config.mode;
						break;
		*/
		
		//--- FLUSH ------------------------------------------------
		case ctrl_flush_night:
		case ctrl_flush_weekend:
		case ctrl_flush_week:
						turn_off_pump(); // opens FLUSH valve
						max_pressure 	= MBAR_500;
						RX_Status.mode 	= RX_Config.mode;
                        break;
        
		case ctrl_flush_step1:        
        case ctrl_flush_step2:
        case ctrl_flush_step3:
        case ctrl_flush_step4:
        case ctrl_flush_done:
                        RX_Status.mode = RX_Config.mode;
						break;
		
		//--- EMPTY ------------------------------------------------
		case ctrl_empty:
						temp_ctrl_on(FALSE);
						_set_valve(TO_INK);
						// _set_pump_speed((_PumpPID.val_max + 1) / 2);
						pid_reset(&_PumpPID);
						_pump_pid(TRUE);
						max_pressure = MBAR_500;	
						RX_Status.mode = RX_Config.mode;						
						break;
		
        case ctrl_empty_step1:
						_pump_pid(TRUE);
						max_pressure = MBAR_500;
						RX_Status.mode = RX_Config.mode;
						break;
		case ctrl_empty_step2:			
						if(RX_Status.mode != RX_Config.mode)
						{
							max_pressure = MBAR_500;
							if (RX_Status.error & (COND_ERR_meniscus|COND_ERR_pump_no_ink))
								RX_Status.mode = RX_Config.mode;
							else _pump_pid(TRUE);
						}
						else
						{
							temp_ctrl_on(FALSE);
							turn_off_pump();
							_set_valve(TO_FLUSH);
						}
                        break;
		
		//--- FILL -------------------------------------------------------
		case ctrl_fill:	
		case ctrl_fill_step1:	
						pid_reset(&_PumpPID);
						temp_ctrl_on(FALSE);
						turn_off_pump();
						RX_Status.error  &= ~(COND_ERR_meniscus | COND_ERR_pump_no_ink);
						max_pressure = MBAR_100;
						RX_Status.mode = RX_Config.mode;
						break;
		
		case ctrl_fill_step2:
        case ctrl_fill_step3:  
						temp_ctrl_on(FALSE);
						_set_valve(TO_INK);
						_pump_pid(TRUE);
						if(_PumpPID.val == _PumpPID.val_min) pid_reset(&_PumpPID);
						max_pressure = MBAR_500;
						if (RX_Config.mode==ctrl_fill_step2 
						||  (RX_Config.mode==ctrl_fill_step3 && RX_Status.pressure_in!=INVALID_VALUE && RX_Status.pressure_in>0)) 
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
            
		
		default:		if (RX_Config.mode>=ctrl_wipe && RX_Config.mode<ctrl_fill)
						{						
							temp_ctrl_on(FALSE);
							turn_off_pump();
							max_pressure = MBAR_500;
							RX_Status.mode = RX_Config.mode;
						}
						else
						{
							turn_off_pump();
							temp_ctrl_on(FALSE);
							RX_Status.mode = ctrl_undef;
						}
						break; 
	}
	    
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
	_menuscusMinMaxDelay = MENISCUS_CHECK_DELAY;
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
	if (_menuscusMinMaxDelay>0) 
	{
		_menuscusMinMaxDelay--;
		return;
	}
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

//--- _presure_in_max -------------------------------------------
static void _presure_in_max(void)
{
	if (valid(RX_Status.pressure_in))
	{
		if ( RX_Status.pressure_in_max==INVALID_VALUE || RX_Status.pressure_in>RX_Status.pressure_in_max) RX_Status.pressure_in_max = RX_Status.pressure_in;
	}
}

//--- _error_cnt ------------------------------------
static void _error_cnt(int err, int *errcnt, int errflag, int timeout)
{
	if (err)(*errcnt)++;
	else	(*errcnt)--;
				
	if ((*errcnt) > timeout)
	{
		if (RX_Config.mode==ctrl_empty_step2) RX_Status.mode = ctrl_empty_step2;
		else 
			RX_Status.error |= errflag;
		(*errcnt) = 0;
	}
	else if ((*errcnt)<0) (*errcnt)=0;
}

//--- _pump_pid --------------------------------------
static void _pump_pid(int Meniscus_Error_Enable)
{       
    static const INT32 MENISCUS_MAX = 1;    // [0.1mbar]
 		
//	if (RX_Status.pressure_out == INVALID_VALUE || RX_Status.info.valve == TO_FLUSH || RX_Status.error & COND_ERR_meniscus)
	if (RX_Status.pressure_out == INVALID_VALUE || RX_Status.info.valve == TO_FLUSH || RX_Status.error & (COND_ERR_meniscus|COND_ERR_pump_no_ink))
    {
		turn_off_pump();
    }
	else
	{   
		if(Meniscus_Error_Enable)
		{
			_menicus_minmax();
			
			// meniscus shall never be positive while in PRINT mode		
			if (!RX_Config.cmd.disable_meniscus_check 
			&&  !(RX_Config.mode>=ctrl_fill && RX_Config.mode<ctrl_fill+10)
			&&  !(RX_Config.mode>=ctrl_cal_step1 && RX_Config.mode<ctrl_cal_done)
			&&  (_rampup_time>=RAMP_UP_TIME)
			)
			{        
				int flow = RX_Status.pump_measured * 60 / 1000;
				if (RX_Config.mode==ctrl_empty_step2)	_error_cnt((flow > 95), &_no_ink_err_cnt,   COND_ERR_pump_no_ink, NO_INK_TIMEOUT);
				else 									_error_cnt((flow > 95), &_no_ink_err_cnt,   COND_ERR_pump_no_ink, NO_INK_TIMEOUT);
				_error_cnt((RX_Status.meniscus > MENISCUS_MAX), 	&_meniscus_err_cnt, COND_ERR_meniscus, _Meniscus_Timeout);			
			}
		}

		// regulate on meniscus
		// detection for the start of the integrator : when Meniscus pass over the setpoint
		if(!_PumpPID.start_integrator)
		{
			if(-RX_Status.meniscus < _PumpPID.Setpoint) _PumpPID.start_integrator++;
		}
		
		// P parameter reduced during start-up
		// Conditions to use P normal : Pinlet > 0  AND -1.5 < and Meniscus-setpoint < 1.5 for 1 second
		if(_Start_PID == START_PID_P_REDUCED)
		{
			if((RX_Status.pressure_in > 0)&&(RX_Status.meniscus + _PumpPID.Setpoint > -15)&&(RX_Status.meniscus + _PumpPID.Setpoint < 15))
			{
				_TimePIDstable++;
				if(_TimePIDstable > 100)
				{
					_Start_PID = START_PID_P_NORMAL;
					// Recalculate the integrator with the new P parameter					
					_PumpPID.diff_I /= _PumpPID.P_start;
					_PumpPID.P = DEFAULT_P;
				}
			}
			else 
			{
				_PumpPID.P = DEFAULT_P / _PumpPID.P_start;			
				_TimePIDstable = 0;
			}
			_Meniscus_Timeout = MENISCUS_TIMEOUT * 2;
		}
		else 
		{
			_PumpPID.P = DEFAULT_P;
			_Meniscus_Timeout = MENISCUS_TIMEOUT;
		}
		
		pid_calc(-RX_Status.meniscus, &_PumpPID);
		_rampup_time++;
		RX_Status.pid_sum	 = _PumpPID.diff_I;
		_set_pump_speed(_PumpPID.val);
	
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
	else RX_Status.pump=INVALID_VALUE;
}

//--- turn_off_pump --------------------------------
void turn_off_pump(void)
{
    _set_valve(TO_FLUSH);
    _set_pump_speed(0);
	_menicus_minmax_reset();
	_rampup_time=0;
}

//--- pump_tick_1000ms -------------------
void pump_tick_1000ms(void)
{           
	if (RX_Status.pump_measured) 
	{
		RX_Status.pumptime++;

        // Only save pump time to EEPROM, Flash write takes too long
        if ((RX_Status.pumptime % 60) == 0) ctr_save();
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
