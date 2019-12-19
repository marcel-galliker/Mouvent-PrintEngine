/******************************************************************************/
/** ink_ctrl.h
 ** 
 ** pressure sensors
 **
 **	Copyright 2016 by radex AG, Switzerland. All rights reserved.
 **	Written by Marcel Galliker
 **
 ******************************************************************************/

//--- includes ---------------------------------------------------------------
#include <stdio.h>
#include <string.h>
#include "system.h"
#include "AMC7891.h"
#include "altera_avalon_pio_regs.h"
#include "rx_ink_common.h"
#include "nios_def_fluid.h"
#include "trprintf.h"
#include "heater.h"
#include "pid.h"
#include "pio.h"
#include "fpga_def_fluid.h"
#include "ink_ctrl.h"
#include "average.h"
#include "pres.h"

//--- defines --------------------------------------------
#define 	MAX_PRES_DEVIATION			20	// % deviation allowed before Bleed solenoid kicks in
#define		MAX_PRINT_PRESSURE_FLUID	300
#define		MAX_PRESSURE_FLUID			1100

#define 	PRESSURE_SOFT_PURGE			80
#define 	PRESSURE_PURGE				160
#define 	PRESSURE_HARD_PURGE			320
#define 	PRESSURE_FLUSH				1000

#define		TIME_SOFT_PURGE				2000
#define 	TIME_PURGE					3000
#define 	TIME_HARD_PURGE				10000

#define 	TIME_EMPTY					3000
#define 	TIME_PRESSURE_FOR_FLUSH		200

#define		TIME_CALIBRATE				30000

#define 	DEGASSING_VACCUUM_UV		800
#define 	DEGASSING_VACCUUM_WB		800

#define 	INK_PUMP_VAL_MAX			511 // 614

#define 	PUMP_CTRL_MODE_DEFAULT		0
#define 	PUMP_CTRL_MODE_PRINT		1
#define 	PUMP_CTRL_MODE_NO_AIR_VALVE	2
#define 	PUMP_CTRL_MODE_FILL			3
#define		PUMP_CTRL_CHECK_IS			4
#define		PUMP_CTRL_CHECK_HEADS		5
#define		PUMP_CTRL_CHECK_HEADS_FAST	6

//--- statics -----------------------------------------------
typedef struct
{
	SPID_par	pid_Setpoint;		// PID output : setpoint pressure IS
	SPID_par	pid_Pump;			// PID output : IS pump
	SPID_par	pid_Calibration;	// PID output : setpoint cond pressure inlet (only used during calibration)

	int			alive;
	int			connected;
	int			degassing;

	int			inkPresSetCfg;
	int			bleedTime;

	int			purgePressure;
	int			purgeTime;
} SInkSupply;

static SInkSupply _InkSupply[NIOS_INK_SUPPLY_CNT];

static int	_LungVacc = 0;
// static int	_PurgeNo;
static int  _PressureTimer;
static int	_ValveOut;
static int	_FlushPump=0;
static int	_AirPressureTime=0;

// test bleed line sequence
static int _TestBleedLine_Timer[NIOS_INK_SUPPLY_CNT] = {0};
static int _OldPumpSpeed[NIOS_INK_SUPPLY_CNT] = {0};

static int _FillPressure=0;

INT32 _PumpBeforeOFF;
INT32 _PumpOFFTime;

// static int	_PrintingTime[NIOS_INK_SUPPLY_CNT] = {0};
static int	_PressureSetpoint[NIOS_INK_SUPPLY_CNT] = {0};
static int	_MaxPrintPressure[NIOS_INK_SUPPLY_CNT] = {0};
static int	_CalibrationSetpoint[NIOS_INK_SUPPLY_CNT] = {0};
static int	_CalibrationStability[NIOS_INK_SUPPLY_CNT] = {0};
static int  _cylinderPres_10[NIOS_INK_SUPPLY_CNT] = {0};
static UINT32 _LastPumpTicks[NIOS_INK_SUPPLY_CNT] = {0};
static UINT32 _PumpSpeed1000[NIOS_INK_SUPPLY_CNT] = {0};
static int 	_FlushTimeISPresStable[NIOS_INK_SUPPLY_CNT] = {0};
static int 	_FlushISPressureStable[NIOS_INK_SUPPLY_CNT] = {0};
static int  _TimeEmpty[NIOS_INK_SUPPLY_CNT] = {0};
static int 	_EmptyDetecTEndState[NIOS_INK_SUPPLY_CNT] = {0};
static int 	_EmptyPressureStored[NIOS_INK_SUPPLY_CNT] = {0};

static int 	_TimeStabitilityFlowResistance[NIOS_INK_SUPPLY_CNT] = {0};
// static int 	_LastISPressureFlowResistance[NIOS_INK_SUPPLY_CNT] = {0};

//--- calibration -------------------------
// static int	  _CalTime[NIOS_INK_SUPPLY_CNT];

//--- prototypes ------------------------
static int    _degass_ctrl(void);
static void   _init_purge(int isNo, int presure);
static void   _set_pump_speed(int isNo, int speed);
static UINT32 _get_pump_ticks(int isNo);
static void   _set_air_valve(int isNo, int newState);
static void   _set_bleed_valve(int isNo, int newState);

// static void _calibrate_inkpump(UINT32 isNo, UINT32 timeout);
static void _pump_ctrl(INT32 isNo, INT32 pressure_target, INT32 print_mode);
static void _set_flush_pump(int isNo, int state);
static void _set_air_pump(int state);
static void _set_pressure_value(int newState);

static void _trace_pump_ctrl(int pressure);

INT32 abs(INT32 val)
{
    if (val < 0) return (-val);
    else		 return (val);
}

//--- ink_init ------------------------------
void ink_init(void)
{
	int isNo;

	memset(_InkSupply, 0, sizeof(_InkSupply));

	for (isNo=0; isNo<NIOS_INK_SUPPLY_CNT; isNo++)
	{
		pid_reset(&_InkSupply[isNo].pid_Pump);
		pid_reset(&_InkSupply[isNo].pid_Setpoint);

		_InkSupply[isNo].pid_Pump.P 					= 1000;
		_InkSupply[isNo].pid_Pump.I 					= 5000;
		_InkSupply[isNo].pid_Pump.Start_Integrator		= 1;
		_InkSupply[isNo].pid_Pump.val_max    			= INK_PUMP_VAL_MAX;	// according to datasheet is [0..5V], [0..10V] is allowed according to KNF
		_InkSupply[isNo].pid_Pump.val_min				= 0;	// Below a certain voltage the pump has not enough power to start

		_InkSupply[isNo].pid_Setpoint.P 				= 200;
		_InkSupply[isNo].pid_Setpoint.I 				= 1500;
		_InkSupply[isNo].pid_Setpoint.Start_Integrator	= 1;
		_InkSupply[isNo].pid_Setpoint.val_max   		= 1000;	// Max IS pressure 1200 mbar
		_InkSupply[isNo].pid_Setpoint.val_min			= 0;	// Min not 0, just a little more

		_InkSupply[isNo].pid_Calibration.val_max    	= 400;	// Max cond inlet pressure 30 mbars
		_InkSupply[isNo].pid_Calibration.val_min		= 0;	// Min not 0, just a little more : 1 mbar

		_InkSupply[isNo].inkPresSetCfg = INVALID_VALUE;
		_InkSupply[isNo].purgePressure = 0;
		_InkSupply[isNo].purgeTime	   = 0;
		_LastPumpTicks[isNo]		   = _get_pump_ticks(isNo);
		_PumpSpeed1000[isNo]		   = 0;
		pRX_Status->ink_supply[isNo].cylinderPresSet = INVALID_VALUE; // 150;
	}
//	_PurgeNo	   = -1;
	_ValveOut      = 0;
}

//--- ink_tick_10ms -------------------------
void ink_tick_10ms(void)
{
//	static const INT32 INK_LOW_PRESSURE    = 50; // mbar

//	static INT32 _shutdown_timeout = 0;

	const INT32 cycleTime = 10;
	INT32 isNo;
	INT32 i;
	INT32 empty_pressure = 800; // 100 * pRX_Config->headsPerColor;

	_LungVacc = DEGASSING_VACCUUM_WB;

	for(isNo = 0 ; isNo < NIOS_INK_SUPPLY_CNT ; isNo++)
	{
		if(!_CalibrationSetpoint[isNo])
			pRX_Status->ink_supply[isNo].cylinderPresSet = pRX_Config->ink_supply[isNo].cylinderPresSet;

		pRX_Status->ink_supply[isNo].fluid_PIDpump_P 		= _InkSupply[isNo].pid_Pump.P;
		pRX_Status->ink_supply[isNo].fluid_PIDpump_I 		= _InkSupply[isNo].pid_Pump.I;

		pRX_Status->ink_supply[isNo].fluid_PIDsetpoint_P 	= _InkSupply[isNo].pid_Setpoint.P;
		pRX_Status->ink_supply[isNo].fluid_PIDsetpoint_I 	= _InkSupply[isNo].pid_Setpoint.I;

		//--- lung vacuum: UV when any heater is on ---
		if (!(pRX_Status->ink_supply[isNo].error & err_heater_board)) _LungVacc = DEGASSING_VACCUUM_UV;

		//---  check if message received from printhead ---------------------
		if(pRX_Config->ink_supply[isNo].alive != _InkSupply[isNo].alive)
		{
			_InkSupply[isNo].alive = pRX_Config->ink_supply[isNo].alive;
			_InkSupply[isNo].connected	= 1000; // timeout in ms
		}
		else if (_InkSupply[isNo].connected>0)
		{
			_InkSupply[isNo].connected-=cycleTime;
		}

		// overpressure check
		if (pRX_Status->ink_supply[isNo].IS_Pressure_Actual != INVALID_VALUE
		&&  pRX_Status->ink_supply[isNo].IS_Pressure_Actual > MAX_PRESSURE_FLUID
		)
		{
			if (pRX_Config->ink_supply[isNo].ctrl_mode != ctrl_test
			&& 	pRX_Config->ink_supply[isNo].ctrl_mode > ctrl_off
			&& (pRX_Config->ink_supply[isNo].ctrl_mode < ctrl_empty ||  pRX_Config->ink_supply[isNo].ctrl_mode > ctrl_empty+10)
			&& (pRX_Config->ink_supply[isNo].ctrl_mode < ctrl_flush_night ||  pRX_Config->ink_supply[isNo].ctrl_mode > ctrl_flush_done)
			)
			{
				_set_pump_speed(isNo, 0);
				_set_air_valve(isNo, TRUE);
				_InkSupply[isNo].degassing=FALSE;
				_set_air_pump(FALSE);

				pRX_Status->ink_supply[isNo].error |= err_overpressure;
			}
		}
	}

	for(isNo = 0 ; isNo < NIOS_INK_SUPPLY_CNT ; isNo++)
	{
		switch(pRX_Config->ink_supply[isNo].ctrl_mode)
		{
			case ctrl_undef:
			case ctrl_shutdown:
			case ctrl_off:
		/*		if(pRX_Status->ink_supply[isNo].ctrl_state == ctrl_print)
				{
					pRX_Status->ink_supply[isNo].ctrl_state = ctrl_shutdown;
				}
				else if(pRX_Status->ink_supply[isNo].ctrl_state == ctrl_shutdown)
				{
					if(_PumpBeforeOFF > 3)	// ramp for pump shutdown
					{
						_PumpOFFTime++;
						if(_PumpOFFTime > 25)
						{
							_PumpBeforeOFF -= 1;
							_PumpOFFTime=0;
						}

						_set_air_valve(isNo, TRUE);
						_set_pump_speed(isNo, _PumpBeforeOFF);
					}
					else
					{
						pRX_Config->ink_supply[isNo].ctrl_mode = ctrl_off;
						pRX_Status->ink_supply[isNo].ctrl_state = ctrl_off;
					}
				}
				else
				{*/
					pid_reset(&_InkSupply[isNo].pid_Pump);
					pid_reset(&_InkSupply[isNo].pid_Setpoint);
					_set_air_valve(isNo, TRUE);
					_set_bleed_valve(isNo, FALSE);
					_CalibrationSetpoint[isNo] = 0;


					{
						int i, on=FALSE;
						for (i=0; i<NIOS_INK_SUPPLY_CNT; i++)
						{
							on |= (pRX_Config->ink_supply[i].ctrl_mode>ctrl_off);
						}
						if (!on) _set_pressure_value(FALSE);
					}
					_set_flush_pump(isNo, FALSE);
					_set_pump_speed(isNo, 0);

					_InkSupply[isNo].degassing=FALSE;
					_InkSupply[isNo].purgeTime=0;
					_InkSupply[isNo].purgePressure=0;
					pRX_Status->ink_supply[isNo].flushTime = 0;
					pRX_Status->ink_supply[isNo].ctrl_state = ctrl_off;
				//}
				break;

			case ctrl_test:
				if (pRX_Status->ink_supply[isNo].ctrl_state==ctrl_test)
				{
					if (pRX_Config->ink_supply[isNo].test_bleed_line)
					{
						switch(pRX_Status->ink_supply[isNo].TestBleedLine_Phase)
						{
							case 1 :
								_set_air_valve(isNo, FALSE);
								_set_bleed_valve(isNo, FALSE);

								if(pRX_Status->ink_supply[isNo].IS_Pressure_Actual > 1000 || _InkSupply[isNo].pid_Pump.val >= _InkSupply[isNo].pid_Pump.val_max)
								{
									_set_pump_speed(isNo, 0);
									_TestBleedLine_Timer[isNo]++;
									if(_TestBleedLine_Timer[isNo] > 2000)
									{
										pRX_Status->ink_supply[isNo].TestBleedLine_Phase++;
										_TestBleedLine_Timer[isNo] = 0;
									}
								}
								else _pump_ctrl(isNo, 1000,PUMP_CTRL_MODE_NO_AIR_VALVE);

								pRX_Status->ink_supply[isNo].TestBleedLine_Pump_Phase1 = pRX_Status->ink_supply[isNo].IS_Pressure_Actual;

								break;
							case 2 :
								_set_air_valve(isNo, TRUE);
								_set_bleed_valve(isNo, FALSE);
								if(pRX_Status->ink_supply[isNo].IS_Pressure_Actual > 990 || _InkSupply[isNo].pid_Pump.val >= _InkSupply[isNo].pid_Pump.val_max)
								{
									_TestBleedLine_Timer[isNo]++;
									if(_TestBleedLine_Timer[isNo] > 2000)
									{
										pRX_Status->ink_supply[isNo].TestBleedLine_Phase++;
										_TestBleedLine_Timer[isNo] = 0;
									}
								}
								_pump_ctrl(isNo, 1000,PUMP_CTRL_MODE_NO_AIR_VALVE);

								pRX_Status->ink_supply[isNo].TestBleedLine_Pump_Phase2 = 100 * _InkSupply[isNo].pid_Pump.val / _InkSupply[isNo].pid_Pump.val_max;

								break;
							case 3 :
								_set_air_valve(isNo, FALSE);
								_set_bleed_valve(isNo, TRUE);
								if(pRX_Status->ink_supply[isNo].IS_Pressure_Actual > 990 || _InkSupply[isNo].pid_Pump.val >= _InkSupply[isNo].pid_Pump.val_max)
								{
									_pump_ctrl(isNo, 1000,PUMP_CTRL_MODE_NO_AIR_VALVE);
									_TestBleedLine_Timer[isNo]++;
									if(_TestBleedLine_Timer[isNo] > 2000)
									{
										pRX_Config->ink_supply[isNo].test_bleed_line = 0;
										_set_pump_speed(isNo, 0);
										_TestBleedLine_Timer[isNo] = 0;
									}
								}
								else _pump_ctrl(isNo, 1000,PUMP_CTRL_MODE_NO_AIR_VALVE);

								pRX_Status->ink_supply[isNo].TestBleedLine_Pump_Phase3 = 100 * _InkSupply[isNo].pid_Pump.val / _InkSupply[isNo].pid_Pump.val_max;

								break;
						}
					}
					else
					{
						if (pRX_Config->ink_supply[isNo].test_airValve) 	_set_air_valve(isNo, !pRX_Status->ink_supply[isNo].airValve);
						if (pRX_Config->ink_supply[isNo].test_bleedValve)	_set_bleed_valve(isNo, !pRX_Status->ink_supply[isNo].bleedValve);
						pRX_Config->ink_supply[isNo].test_airValve		= FALSE;
						pRX_Config->ink_supply[isNo].test_bleedValve 	= FALSE;

						_pump_ctrl(isNo, pRX_Config->ink_supply[isNo].test_cylinderPres, PUMP_CTRL_MODE_NO_AIR_VALVE);		// ink-pump
						_set_pressure_value(pRX_Status->air_pressure < pRX_Config->test_airPressure);	// air-pump
					}
				}
				else
				{
					// --- reset values -----------
					pRX_Config->ink_supply[isNo].test_airValve  	= FALSE;
					pRX_Config->ink_supply[isNo].test_bleedValve 	= FALSE;
					pRX_Config->ink_supply[isNo].test_cylinderPres 	= 0;
					pRX_Status->ink_supply[isNo].TestBleedLine_Phase = 1;
					_TestBleedLine_Timer[isNo] = 0;
					pRX_Status->ink_supply[isNo].TestBleedLine_Pump_Phase1 = 0;
					pRX_Status->ink_supply[isNo].TestBleedLine_Pump_Phase2 = 0;
					pRX_Status->ink_supply[isNo].TestBleedLine_Pump_Phase3 = 0;
					_OldPumpSpeed[isNo] = -10;
				}
				pRX_Status->ink_supply[isNo].ctrl_state = ctrl_test;
				break;

			case ctrl_warmup:
				_set_air_valve(isNo, FALSE);
				_set_bleed_valve(isNo, FALSE);
				_InkSupply[isNo].degassing=pRX_Config->cmd.lung_enabled;
				_pump_ctrl(isNo, pRX_Status->ink_supply[isNo].cylinderPresSet, PUMP_CTRL_MODE_DEFAULT);
				pRX_Status->ink_supply[isNo].ctrl_state = ctrl_warmup;
				break;

			case ctrl_readyToPrint:
				break;

			case ctrl_check_step0:	pRX_Status->ink_supply[isNo].ctrl_state = pRX_Config->ink_supply[isNo].ctrl_mode; break;
			case ctrl_check_step1:	pRX_Status->ink_supply[isNo].ctrl_state = pRX_Config->ink_supply[isNo].ctrl_mode; break;
			case ctrl_check_step2:	pRX_Status->ink_supply[isNo].ctrl_state = pRX_Config->ink_supply[isNo].ctrl_mode; break;

			case ctrl_check_step3:
				_set_air_valve(isNo, FALSE);
				_set_bleed_valve(isNo, FALSE);
				_set_flush_pump(isNo, FALSE);

				_PressureSetpoint[isNo] = pRX_Status->ink_supply[isNo].cylinderPresSet;
				_InkSupply[isNo].pid_Setpoint.I = 1500;

				_pump_ctrl(isNo, _PressureSetpoint[isNo], PUMP_CTRL_MODE_PRINT);
				pRX_Status->ink_supply[isNo].ctrl_state = pRX_Config->ink_supply[isNo].ctrl_mode;
				break;

			case ctrl_check_step4:  pRX_Status->ink_supply[isNo].ctrl_state = pRX_Config->ink_supply[isNo].ctrl_mode; break;
			case ctrl_check_step5:  pRX_Status->ink_supply[isNo].ctrl_state = pRX_Config->ink_supply[isNo].ctrl_mode; break;

			case ctrl_check_step6:	pRX_Status->ink_supply[isNo].ctrl_state = pRX_Config->ink_supply[isNo].ctrl_mode; break;
			case ctrl_check_step7:	pRX_Status->ink_supply[isNo].ctrl_state = pRX_Config->ink_supply[isNo].ctrl_mode; break;
			case ctrl_check_step8:	pRX_Status->ink_supply[isNo].ctrl_state = pRX_Config->ink_supply[isNo].ctrl_mode; break;
			case ctrl_check_step9:	pRX_Status->ink_supply[isNo].ctrl_state = pRX_Config->ink_supply[isNo].ctrl_mode; break;

			case ctrl_cal_start:
			case ctrl_cal_step1:
			case ctrl_cal_step2:
			case ctrl_cal_step3:
				// decrease Pressure setpoint slowly
				//-----------------------------------
				_TimeStabitilityFlowResistance[isNo]--;
				if(_TimeStabitilityFlowResistance[isNo] <= 0)
				{
					if(_PressureSetpoint[isNo] > 0) _PressureSetpoint[isNo]-=1;
					_TimeStabitilityFlowResistance[isNo] = 15;
				}
				pRX_Status->ink_supply[isNo].cylinderPresSet = _PressureSetpoint[isNo];
				_pump_ctrl(isNo, _PressureSetpoint[isNo], PUMP_CTRL_MODE_DEFAULT);
				pRX_Status->ink_supply[isNo].ctrl_state = pRX_Config->ink_supply[isNo].ctrl_mode;

				break;

			case ctrl_cal_step4:
				// increase Pressure setpoint to remove bubbles
				//----------------------------------------------
				if(pRX_Status->ink_supply[isNo].cylinderPresSet != 400)
				{
					pid_reset(&_InkSupply[isNo].pid_Pump);
					pRX_Status->ink_supply[isNo].cylinderPresSet = 400;
					_PressureSetpoint[isNo] = pRX_Status->ink_supply[isNo].cylinderPresSet;
				}

				_pump_ctrl(isNo, _PressureSetpoint[isNo], PUMP_CTRL_CHECK_HEADS_FAST);

				if(_TimeStabitilityFlowResistance[isNo] < 1000)
				{
					if(abs(pRX_Config->ink_supply[isNo].condPumpFeedback - _PressureSetpoint[isNo]) < 40)
						_TimeStabitilityFlowResistance[isNo]++;
					else _TimeStabitilityFlowResistance[isNo] = 0;
				}
				else pRX_Status->ink_supply[isNo].ctrl_state = pRX_Config->ink_supply[isNo].ctrl_mode;

				break;

			case ctrl_cal_done:
				// increase Pressure setpoint to remove bubbles
				//----------------------------------------------

				pRX_Status->ink_supply[isNo].cylinderPresSet = 30;
				_PressureSetpoint[isNo] = pRX_Status->ink_supply[isNo].cylinderPresSet;

				_pump_ctrl(isNo, _PressureSetpoint[isNo], PUMP_CTRL_CHECK_HEADS_FAST);

				if(pRX_Config->ink_supply[isNo].condPumpFeedback < 70)
					pRX_Status->ink_supply[isNo].ctrl_state = pRX_Config->ink_supply[isNo].ctrl_mode;

				break;


			case ctrl_print:
				if(pRX_Config->ink_supply[isNo].ctrl_mode == ctrl_print) _CalibrationSetpoint[isNo] = 0;
				else
				{
					_CalibrationSetpoint[isNo] = 1;
					if(pRX_Config->ink_supply[isNo].ctrl_mode == ctrl_cal_start)
					{
						pRX_Status->ink_supply[isNo].cylinderPresSet = 0;
						_PressureSetpoint[isNo] = pRX_Status->ink_supply[isNo].cylinderPresSet;
						pRX_Status->ink_supply[isNo].ctrl_state = ctrl_cal_start;
					}
				}

				_set_bleed_valve(isNo, FALSE);
				_set_air_valve(isNo, FALSE);
				_set_flush_pump(isNo, FALSE);
				_InkSupply[isNo].degassing = pRX_Config->cmd.lung_enabled;
				_PumpOFFTime = 0;

				// do not start ramp-up from 0 after purge
				if (pRX_Status->ink_supply[isNo].ctrl_state != pRX_Config->ink_supply[isNo].ctrl_mode
				&&  pRX_Status->ink_supply[isNo].ctrl_state != ctrl_purge_step5)
					_cylinderPres_10[isNo] = 0;

				if(pRX_Config->ink_supply[isNo].ctrl_mode == ctrl_print)
				{
					_PressureSetpoint[isNo] = pRX_Status->ink_supply[isNo].cylinderPresSet;
					if(pRX_Status->ink_supply[isNo].ctrl_state == ctrl_off)
					{
						if(_PressureSetpoint[isNo] > 1000) _InkSupply[isNo].pid_Setpoint.I = 10000;
						else if(_PressureSetpoint[isNo] > 500) _InkSupply[isNo].pid_Setpoint.I = 5000;
						else _InkSupply[isNo].pid_Setpoint.I = 1500;
					}
					pRX_Status->ink_supply[isNo].ctrl_state = pRX_Config->ink_supply[isNo].ctrl_mode;
				}

				_pump_ctrl(isNo, _PressureSetpoint[isNo], PUMP_CTRL_MODE_PRINT);
				break;

				/*
			case ctrl_bleed:
				_set_air_valve(isNo, FALSE);
				_InkSupply[isNo].degassing=pRX_Config->cmd.lung_enabled;
				_pump_ctrl(isNo, pRX_Status->ink_supply[isNo].cylinderPresSet, FALSE);
				pRX_Status->ink_supply[isNo].ctrl_state = ctrl_bleed;
				break;
				*/
/*
			case ctrl_deflate:
				_set_air_cussion(isNo, TRUE);
				_InkSupply[isNo].degassing=pRX_Config->cmd.lung_enabled;
				_pump_ctrl(isNo, PRESSURE_HARD_PURGE);
				pRX_Status->ink_supply[isNo].ctrl_state = ctrl_deflate;
				break;
*/
			// --- FLUSH --------------------------------------------------
			case ctrl_flush_night:
			case ctrl_flush_weekend:
			case ctrl_flush_week:
				_set_air_valve(isNo, FALSE);
				_InkSupply[isNo].degassing = pRX_Config->cmd.lung_enabled;
				_pump_ctrl(isNo, PRESSURE_FLUSH,PUMP_CTRL_MODE_DEFAULT);
				_set_flush_pump(isNo, FALSE);
				pRX_Status->ink_supply[isNo].flushTime  = 1000 * pRX_Config->ink_supply[isNo].flushTime[pRX_Config->ink_supply[isNo].ctrl_mode-ctrl_flush_night];
				pRX_Status->ink_supply[isNo].ctrl_state = pRX_Config->ink_supply[isNo].ctrl_mode;
				break;

			case ctrl_flush_step1:
			case ctrl_flush_step2:
				_pump_ctrl(isNo, PRESSURE_FLUSH,PUMP_CTRL_MODE_DEFAULT);
				_FlushISPressureStable[isNo] = pRX_Status->ink_supply[isNo].IS_Pressure_Actual;
				_FlushTimeISPresStable[isNo] = 0;
				pRX_Status->ink_supply[isNo].ctrl_state = pRX_Config->ink_supply[isNo].ctrl_mode;
				break;

			case ctrl_flush_step3:
				_pump_ctrl(isNo, PRESSURE_FLUSH,PUMP_CTRL_MODE_DEFAULT);
				// detection of IS pressure stability to start the flush
				if(abs(_FlushISPressureStable[isNo] -  pRX_Status->ink_supply[isNo].IS_Pressure_Actual) < 200)
					_FlushTimeISPresStable[isNo]++;
				else
				{
					_FlushISPressureStable[isNo] = pRX_Status->ink_supply[isNo].IS_Pressure_Actual;
					_FlushTimeISPresStable[isNo] = 0;
				}
				// ink pressure must be applied on all cylinders so that no flush is pressed back

				//if (pRX_Config->ink_supply[isNo].cylinderPresSet==0
				//||
				if(pRX_Status->ink_supply[isNo].IS_Pressure_Actual != INVALID_VALUE
						&&  _FlushTimeISPresStable[isNo] > TIME_PRESSURE_FOR_FLUSH
						&& pRX_Status->ink_supply[isNo].IS_Pressure_Actual > 100)
				//	)
					pRX_Status->ink_supply[isNo].ctrl_state = ctrl_flush_step3;
				break;

			case ctrl_flush_step4:
				_InkSupply[isNo].degassing = pRX_Config->cmd.lung_enabled;
				if (pRX_Status->ink_supply[isNo].flushTime>0)
				{
					_pump_ctrl(isNo, PRESSURE_FLUSH,PUMP_CTRL_MODE_DEFAULT);
					if (pRX_Status->flush_pressure < 1200) _set_flush_pump(isNo, TRUE);
					else
					{
						_set_flush_pump(isNo, FALSE);
						pRX_Status->ink_supply[isNo].ctrl_state = ctrl_error;
					}
					pRX_Status->ink_supply[isNo].flushTime -= cycleTime;
					pRX_Status->ink_supply[isNo].ctrl_state = ctrl_flush_step3;
				}
				else
				{
					_set_pump_speed(isNo, 0);
					_set_flush_pump(isNo, FALSE);
					_set_air_valve(isNo, FALSE);
					pRX_Status->ink_supply[isNo].ctrl_state = ctrl_flush_step4;
				}
				break;

			case ctrl_flush_done:
				pRX_Status->ink_supply[isNo].ctrl_state = ctrl_flush_done;
				break;
			case ctrl_purge_soft:		_init_purge(isNo, PRESSURE_SOFT_PURGE); break;
			case ctrl_purge:			_init_purge(isNo, PRESSURE_PURGE); 		break;
			case ctrl_purge_hard:		_init_purge(isNo, PRESSURE_HARD_PURGE); break;
			case ctrl_purge_hard_wipe:	_init_purge(isNo, PRESSURE_HARD_PURGE); break;

			case ctrl_purge_step1: // build up pressure
			case ctrl_purge_step2: // build up pressure
				_pump_ctrl(isNo, _InkSupply[isNo].purgePressure,PUMP_CTRL_MODE_DEFAULT);
				pRX_Status->ink_supply[isNo].ctrl_state = pRX_Config->ink_supply[isNo].ctrl_mode;
				break;

			case ctrl_purge_step3: // build up pressure
				_pump_ctrl(isNo, _InkSupply[isNo].purgePressure,PUMP_CTRL_MODE_DEFAULT);
				_InkSupply[isNo].purgeTime = 0;
				if (pRX_Status->ink_supply[isNo].IS_Pressure_Actual >= (60 * _InkSupply[isNo].purgePressure / 100))
					pRX_Status->ink_supply[isNo].ctrl_state = ctrl_purge_step3;
				break;

			case ctrl_purge_step4:
				if (_InkSupply[isNo].purgeTime<pRX_Config->ink_supply[isNo].purgeTime)
				{
					_pump_ctrl(isNo, _InkSupply[isNo].purgePressure, PUMP_CTRL_MODE_DEFAULT);
					_set_bleed_valve(isNo, FALSE);
					_InkSupply[isNo].purgeTime+=cycleTime;
				}
				else
				{
					_InkSupply[isNo].purgePressure = 0;
					_set_pump_speed(isNo, 0);
					_set_air_valve(isNo, TRUE);
					pRX_Status->ink_supply[isNo].ctrl_state = ctrl_purge_step4;
				}
				break;

			case ctrl_purge_step5:
					pRX_Status->ink_supply[isNo].ctrl_state = ctrl_purge_step5;
				break;

			case ctrl_cap:
			case ctrl_cap_step1:
			case ctrl_cap_step2:
			case ctrl_cap_step3:
			case ctrl_cap_step4:
			case ctrl_wipe:
			case ctrl_wash:
			case ctrl_wipe_step1:
			case ctrl_wipe_step2:
				_set_air_valve(isNo, TRUE);
				_set_pump_speed(isNo, 0);
				pRX_Status->ink_supply[isNo].ctrl_state = pRX_Config->ink_supply[isNo].ctrl_mode;
				break;

			// --- FILL --------------------------------------------------
			case ctrl_fill:
				_InkSupply[isNo].degassing = FALSE;
				_set_air_valve(isNo, TRUE);
				_set_bleed_valve(isNo, FALSE);
				_FillPressure = pRX_Config->headsPerColor * 50;
				if(_FillPressure < 100) _FillPressure = 100;
				if(_FillPressure > 1000) _FillPressure = 1000;
				pRX_Status->ink_supply[isNo].ctrl_state = ctrl_fill;
				break;

			case ctrl_fill_step1:
				if (pRX_Status->ink_supply[isNo].IS_Pressure_Actual <= _FillPressure/2)
					pRX_Status->ink_supply[isNo].ctrl_state = ctrl_fill_step1;
				break;

			case ctrl_fill_step2:
				_set_air_valve(isNo, TRUE);
				_set_bleed_valve(isNo, FALSE);
				_set_pressure_value(FALSE);
				_pump_ctrl(isNo, _FillPressure,PUMP_CTRL_MODE_DEFAULT);
				pRX_Status->ink_supply[isNo].inkPumpSpeed_set = 100;
				pRX_Status->ink_supply[isNo].ctrl_state = ctrl_fill_step2;
				break;

			case ctrl_fill_step3:
				_pump_ctrl(isNo, _FillPressure,PUMP_CTRL_MODE_DEFAULT);
				if ((pRX_Status->ink_supply[isNo].PIDpump_Output < INK_PUMP_VAL_MAX / 2)&&(pRX_Status->ink_supply[isNo].IS_Pressure_Actual > 100))
					pRX_Status->ink_supply[isNo].ctrl_state = ctrl_fill_step3;
				break;

			// --- EMPTY -------------------------------------------------
			case ctrl_empty:
				_set_bleed_valve(isNo, TRUE);
				_set_air_valve(isNo, FALSE);
				pRX_Status->ink_supply[isNo].ctrl_state = ctrl_empty;
				break;

			case ctrl_empty_step1:
				_set_bleed_valve(isNo, TRUE);
				_set_air_valve(isNo, FALSE);
				for (i=0; i<NIOS_INK_SUPPLY_CNT; i++)
				{
					_set_pump_speed(i, 0);
					_InkSupply[i].degassing = FALSE;
				}
				_pump_ctrl(isNo, 0,PUMP_CTRL_MODE_NO_AIR_VALVE);		// ink-pump
				_TimeEmpty[isNo] = 0;
				_EmptyDetecTEndState[isNo] = 1;
				pRX_Status->ink_supply[isNo].ctrl_state = ctrl_empty_step1;
				break;

			case ctrl_empty_step2:
				_set_bleed_valve(isNo, TRUE);
				_set_air_valve(isNo, FALSE);
				empty_pressure = 100 * pRX_Config->headsPerColor;
				if(empty_pressure < 400) empty_pressure = 400;
				if(empty_pressure > 800) empty_pressure = 800;
				if(pRX_Config->ink_supply[isNo].condMeniscus < -50)
					_set_pressure_value(pRX_Status->air_pressure < empty_pressure);
				else _set_pressure_value(FALSE);

				// minimum 20 secondes of running air pump for emptying the cylinder
				_TimeEmpty[isNo]++;
				switch(_EmptyDetecTEndState[isNo])
				{
					case 1 :
						if(_TimeEmpty[isNo] > TIME_EMPTY) _EmptyDetecTEndState[isNo]++;
						break;
					case 2 :
						_EmptyPressureStored[isNo] = pRX_Status->ink_supply[isNo].IS_Pressure_Actual;
						_EmptyDetecTEndState[isNo]++;
						break;
					case 3 :
						if(pRX_Status->ink_supply[isNo].IS_Pressure_Actual < _EmptyPressureStored[isNo] / 2)
							pRX_Status->ink_supply[isNo].ctrl_state = ctrl_empty_step2;
						break;
				}

				break;

			case ctrl_empty_step3:
				_set_pressure_value(FALSE);
				pRX_Status->ink_supply[0].TestBleedLine_Pump_Phase2++;
				_set_bleed_valve(isNo, FALSE);
				_set_air_valve(isNo, FALSE);
				pRX_Status->ink_supply[isNo].ctrl_state = ctrl_empty_step3;
				break;

				/*
			case ctrl_shutdown:
				//--- stop pumps, open air valve and wait until pressure low ---
				if (pRX_Config->ink_supply[isNo].ctrl_mode != pRX_Status->ink_supply[isNo].ctrl_state)
					_shutdown_timeout = 6000;

				if (_shutdown_timeout)
					--_shutdown_timeout;

				_set_bleed_valve(isNo, FALSE);
				_set_pressure_value(FALSE);
				_set_pump_speed(isNo, 0);
				_set_air_valve(isNo, FALSE);
				_PurgePressure = 0;
				_PurgeNo = -1;

				if (pRX_Status->ink_supply[isNo].cylinderPres <= INK_LOW_PRESSURE &&
					_shutdown_timeout == 0)
				{
					_set_air_valve(isNo, FALSE);
					pRX_Status->ink_supply[isNo].ctrl_state = ctrl_shutdown_done;
				}
				else
				{
					_set_air_valve(isNo, TRUE);
					pRX_Status->ink_supply[isNo].ctrl_state = ctrl_shutdown;
				}
				break;
				*/

			default:
				if (pRX_Config->ink_supply[isNo].ctrl_mode>=ctrl_wipe && pRX_Config->ink_supply[isNo].ctrl_mode<ctrl_fill)
				{
					_set_air_valve(isNo, TRUE);
					_set_pump_speed(isNo, 0);
					pRX_Status->ink_supply[isNo].ctrl_state = pRX_Config->ink_supply[isNo].ctrl_mode;
				}
				else
				{
					_set_flush_pump(isNo, FALSE);
					_set_air_valve(isNo, FALSE);
					_set_pump_speed(isNo, 0);
					_InkSupply[isNo].degassing=FALSE;
				}
				break;
		} // end switch
	} // end for

	//--- control air-system -----------------
	if (pRX_Status->air_pressure_solenoid)
		_AirPressureTime+=cycleTime;

	if (_degass_ctrl() || pRX_Status->air_pressure_solenoid || _PressureTimer>0)
	{
		trprintf("AIR PUMP %d %d %d %d\n", pRX_Status->air_pump, _degass_ctrl(), pRX_Status->air_pressure_solenoid , _PressureTimer);

		if (_PressureTimer>0 && (_PressureTimer-=cycleTime)<0)
			_PressureTimer=0;

		_set_air_pump(TRUE);
	}
	else
	{
		_set_air_pump(FALSE);
	}

	// Update the IS Solenoids
	IOWR_16DIRECT(AXI_LW_SLAVE_REGISTER_0_BASE, GPIO_REG_OUT, _ValveOut);
}

//--- ink_tick_1000ms ------------------------------------------------------
#define PRINTF  for(i=0; i<NIOS_INK_SUPPLY_CNT; i++) trprintf
void ink_tick_1000ms(void)
{
	int isNo;

	if (tr_debug_on())
	{
		int i;
		trprintf("\n-------------------------------------------------------------------------------------\n");
		trprintf("ink pressure  :"); PRINTF("  %6d ", pRX_Status->ink_supply[i].IS_Pressure_Actual); trprintf("\n");
	}
	/*
	if (tr_debug_on())
	{
		int i;
		trprintf("\n-------------------------------------------------------------------------------------\n");
		trprintf("Error:    0x%08x\n", pRX_Status->error);
		trprintf("Pressure flush:    %s      pump: %d  val=%d\n", value_str(pRX_Status->flush_pressure), _FlushPump, pRX_Status->flush_pump_val);
		trprintf("Pressure degass:   %s      Air pump: %d   Valve: %d\n", value_str(pRX_Status->degass_pressure), pRX_Status->air_pump, pRX_Status->vacuum_solenoid);
		trprintf("Pressure:          %s      Valve: %d\n", value_str(pRX_Status->air_pressure), pRX_Status->air_pressure_solenoid);

		trprintf("               "); PRINTF(" ---%d--- ", i); trprintf("\n");
		trprintf("ctrl_mode:     "); PRINTF("%8s ", FluidCtrlModeStr(pRX_Config->ink_supply[i].ctrl_mode));   trprintf("\n");
		trprintf("pressure set  :"); PRINTF("  %6s ", value_str(pRX_Status->ink_supply[i].cylinderPresSet)); trprintf("\n");
		trprintf("ink pressure  :"); PRINTF("  %6s ", value_str(pRX_Status->ink_supply[i].cylinderPres)); trprintf("\n");
		trprintf("head in pres  :"); PRINTF("  %6s ", value_str(pRX_Config->ink_supply[i].condPresIn)); trprintf("\n");
		trprintf("bleed valve   :"); PRINTF("       %d ", pRX_Status->ink_supply[i].bleedValve); trprintf("\n");
		trprintf("air valve     :"); PRINTF("       %d ", pRX_Status->ink_supply[i].airValve); trprintf("\n");

		trprintf("pid diff:      "); PRINTF("  %6s ", value_str(_InkSupply[i].pid.diff_old)); trprintf("\n");
		//trprintf("pid diff sum:  "); PRINTF("  %6s ", value_str(_InkSupply[i].pid.diff_sum)); trprintf("\n");
		//trprintf("pid val1:      "); PRINTF("  %6d ", _InkSupply[i].pid.val1); trprintf("\n");
		trprintf("pump speed:    "); PRINTF("  %6s ", value_str(pRX_Status->ink_supply[i].inkPumpSpeed_set)); trprintf("\n");
		trprintf("degassing:     "); trprintf("  %6d ", pRX_Status->vacuum_solenoid); trprintf("\n");
		//trprintf("pressure Valve:"); trprintf("  %6d ", pRX_Status->pressure_solenoid); trprintf("\n");

		trprintf("head pump speed:"); PRINTF("  %6s ", value_str1(pRX_Config->ink_supply[i].condPumpFeedback)); trprintf("\n");

		trprintf("Heater (Max):   "); PRINTF("  %6d ", pRX_Config->ink_supply[i].heaterTempMax);trprintf("\n");
		trprintf("Heater (Temp):  "); PRINTF("  %6s ", value_str(pRX_Status->ink_supply[i].heaterTemp)); trprintf("\n");

		trprintf("purge time: %d, pressure: %d\n", _PurgeTime, _PurgePressure);
	}
	*/

	/* pump volume calculation */
	for (isNo = 0; isNo < NIOS_INK_SUPPLY_CNT; isNo++)
	{

		/* pump PML12247-NF60
		 * throughput (max) P = 600ml/min
		 * 6 Impulses per revolution
		 * measured every second
		 *
		 * \f$ \frac{diff Imp \times 1s \times 600ml} {6Imp \times 60s} \f$
		 */

		UINT32 ticks = _get_pump_ticks(isNo);
		INT32 volume = (ticks - _LastPumpTicks[isNo]) * 94 / 10; // [ul/s]

		pRX_Status->ink_supply[isNo].inkPumpSpeed_measured = volume * 60 / 1000; // [ml/min]

#if (DEBUG)
		INT32 volume = diff * 94 / 10; // [ul/s]
		trprintf("%d) %u - %u = %u -> V = %d ul/s = %d ml/min (%d%)\n", isNo, ticks, _LastPumpTicks[isNo],
				diff, volume, volume * 60 / 1000,
				pRX_Status->ink_supply[isNo].inkPumpSpeed_set);
#endif

		_LastPumpTicks[isNo] = ticks;
		pRX_Status->ink_supply[isNo].inkPumpSpeed_set = _PumpSpeed1000[isNo]/100;
		_PumpSpeed1000[isNo] = 0;
	}
	pRX_Status->airPressureTime = _AirPressureTime;
	_AirPressureTime = 0;
}

//--- _init_purge ---------------------------------------------
static void _init_purge(int isNo, int pressure)
{
	if (_InkSupply[isNo].purgePressure==0)
	{
		_InkSupply[isNo].degassing=FALSE;
		_set_air_valve(isNo, FALSE);
		_set_bleed_valve(isNo, FALSE);

	//	_PurgeNo	   = isNo;
		if(_MaxPrintPressure[isNo] > 0)
		{
			_InkSupply[isNo].purgePressure = _MaxPrintPressure[isNo] + pressure;
			if(_InkSupply[isNo].purgePressure > 800) _InkSupply[isNo].purgePressure = 800;
		}
		else
		{
			switch(pRX_Config->printerType)
			{
				case printer_TX801 : _InkSupply[isNo].purgePressure = 200 + pressure; break;
				default : _InkSupply[isNo].purgePressure = 40 * pRX_Config->headsPerColor + pressure; break;
			}
		}

		if (_InkSupply[isNo].purgePressure > MAX_PRESSURE_FLUID)
			_InkSupply[isNo].purgePressure = MAX_PRESSURE_FLUID;

		pRX_Status->ink_supply[isNo].IS_Pressure_Setpoint 	=  _InkSupply[isNo].purgePressure;
	}
	pRX_Status->ink_supply[isNo].ctrl_state = pRX_Config->ink_supply[isNo].ctrl_mode;
}

//--- _degass_ctrl ---------------------
static int _degass_ctrl(void)
{
	int isNo;
	int degassing=FALSE;
	int act=pRX_Status->vacuum_solenoid;

	for (isNo=0; isNo<NIOS_INK_SUPPLY_CNT; isNo++)
	{
		if (_InkSupply[isNo].degassing) degassing=TRUE;
	}

	if (pRX_Config->test_lungPressure)
	{
		if (pRX_Status->degass_pressure==INVALID_VALUE) pRX_Status->vacuum_solenoid = FALSE;
		else if ((-pRX_Status->degass_pressure) >= pRX_Config->test_lungPressure+25) pRX_Status->vacuum_solenoid = FALSE;
		else if ((-pRX_Status->degass_pressure) <  pRX_Config->test_lungPressure-25) pRX_Status->vacuum_solenoid = TRUE;
	}
	else
	{
		if (!degassing || pRX_Status->degass_pressure==INVALID_VALUE) 	  pRX_Status->vacuum_solenoid = FALSE;
		else if ((-pRX_Status->degass_pressure) >= _LungVacc) 	  pRX_Status->vacuum_solenoid = FALSE;
		else if ((-pRX_Status->degass_pressure) <  _LungVacc-50)  pRX_Status->vacuum_solenoid = TRUE;
	}

	if (pRX_Status->vacuum_solenoid!=act)
	{
		if (pRX_Status->vacuum_solenoid)
			IOWR_ALTERA_AVALON_PIO_SET_BITS(PIO_OUTPUT_BASE, DEGASS_VALVE_OUT);	//Vaccum Solenoid = on
		else
			IOWR_ALTERA_AVALON_PIO_CLEAR_BITS(PIO_OUTPUT_BASE, DEGASS_VALVE_OUT);	// Vaccum Solenoid = off
	}
	return (pRX_Status->vacuum_solenoid);
}

//--- _set_air_cussion -----------------------------------------
void _set_air_valve(int isNo, int newState)
{
	if (pRX_Status->ink_supply[isNo].airValve!=newState)
	{
		pRX_Status->ink_supply[isNo].airValve = newState;
		if (newState) _ValveOut |= AIR_CUSSION_OUT(isNo);
		else		  _ValveOut &= ~AIR_CUSSION_OUT(isNo);
	}
}

//--- _set_bleed -----------------------------------------
void _set_bleed_valve(int isNo, int state)
{
	pRX_Status->ink_supply[isNo].bleedValve = state;
	if (state) 	_ValveOut |=  BLEED_OUT(isNo);
	else		_ValveOut &= ~BLEED_OUT(isNo);
}

//--- _set_pressure_value -----------------------------------------
 static void _set_pressure_value(int state)
{
	if (state != pRX_Status->air_pressure_solenoid)
	{
		pRX_Status->air_pressure_solenoid = state;
		if (state)
			IOWR_ALTERA_AVALON_PIO_SET_BITS(PIO_OUTPUT_BASE,   PRESSURE_VALVE_OUT);
		else
			IOWR_ALTERA_AVALON_PIO_CLEAR_BITS(PIO_OUTPUT_BASE, PRESSURE_VALVE_OUT);
	}
}

//--- _trace_pump_ctrl ---------------------------------------------
static void _trace_pump_ctrl(int pressure)
{
	static int cnt=0;
	int i=0;
	if (FALSE && tr_debug_on())
	{
		if (cnt==0) trprintf("cnt\thead\ttank\tpump\tbleed\n");
		cnt++;
//		if (pRX_Config->ink_supply[i].ctrl_mode==ctrl_print)
		{
			trprintf("%d\tm=%x\ts=%x\th=%d\tt=%d(%d)\tp=%d\tv=%d\n",
					cnt,
					pRX_Config->ink_supply[i].ctrl_mode,
					pRX_Status->ink_supply[i].ctrl_state,
					pRX_Config->ink_supply[i].condPresIn,
					pRX_Status->ink_supply[i].IS_Pressure_Actual,
					pressure,
					pRX_Status->ink_supply[i].inkPumpSpeed_set,
					pRX_Status->ink_supply[i].bleedValve);
		}
	}
}

//--- _pump_ctrl -----------------------------------------------------
static void _pump_ctrl(INT32 isNo, INT32 pressure_target, INT32 print_mode)
{
	#define PUMP_THRESHOLD	5
	#define MAX_PRESSURE	1100


	int set_valve = (pRX_Status->ink_supply[isNo].ctrl_state!=ctrl_test)
			    && !(pRX_Status->ink_supply[isNo].ctrl_state>=ctrl_fill && pRX_Status->ink_supply[isNo].ctrl_state<ctrl_fill+10);

		//-----------------------------------------------------------------------------------------
		//  PID 1 (setpoints) :  Pressure_IN_cond_setpoint --> PID_setpoint --> Pressure_IS_setpoint
		//-----------------------------------------------------------------------------------------

		pRX_Status->ink_supply[isNo].COND_Pressure_Actual 	= pRX_Config->ink_supply[isNo].condPresIn;
		if(pRX_Config->headsPerColor == 1) _InkSupply[isNo].pid_Setpoint.P = 100;
		else _InkSupply[isNo].pid_Setpoint.P 				= 200;

		// TEST CHECK HEADS
		_InkSupply[isNo].pid_Setpoint.P 				= 100;
		_InkSupply[isNo].pid_Setpoint.I 				= 2000;

		if(print_mode == PUMP_CTRL_MODE_PRINT)
		{
			_InkSupply[isNo].pid_Setpoint.Setpoint = pressure_target;
			pid_calc(pRX_Status->ink_supply[isNo].COND_Pressure_Actual, &_InkSupply[isNo].pid_Setpoint);
			pRX_Status->ink_supply[isNo].IS_Pressure_Setpoint 	= _InkSupply[isNo].pid_Setpoint.val;
		//	pRX_Status->ink_supply[isNo].PIDsetpoint_Output		= _InkSupply[isNo].pid_Setpoint.val;

			// Save max pressure for purge pressure
			// if ink supply pressure close to setpoint
			if(_InkSupply[isNo].pid_Setpoint.Setpoint - pRX_Status->ink_supply[isNo].COND_Pressure_Actual > 20)
			{
				if(_InkSupply[isNo].pid_Setpoint.val > _MaxPrintPressure[isNo])
					_MaxPrintPressure[isNo] = _InkSupply[isNo].pid_Setpoint.val;
			}
			// reset pid if output at 0, no reason to accumulate negative value (never want pressure negative)
			if(_InkSupply[isNo].pid_Setpoint.val <= _InkSupply[isNo].pid_Setpoint.val_min)
				pid_reset(&_InkSupply[isNo].pid_Setpoint);
		}
		else if((print_mode == PUMP_CTRL_CHECK_HEADS)||(print_mode == PUMP_CTRL_CHECK_HEADS_FAST))  // regulate 10ml/min
		{
			//if(print_mode == PUMP_CTRL_CHECK_HEADS)
			//{
				_InkSupply[isNo].pid_Setpoint.P 				= 80;
			//}
			_InkSupply[isNo].pid_Setpoint.Setpoint = pressure_target;
			pid_calc(pRX_Config->ink_supply[isNo].condPumpFeedback, &_InkSupply[isNo].pid_Setpoint);
			pRX_Status->ink_supply[isNo].IS_Pressure_Setpoint 	= _InkSupply[isNo].pid_Setpoint.val;
		//	pRX_Status->ink_supply[isNo].PIDsetpoint_Output		= _InkSupply[isNo].pid_Setpoint.val;

			// Save max pressure for purge pressure
			// if ink supply pressure close to setpoint
			if(_InkSupply[isNo].pid_Setpoint.Setpoint - pRX_Status->ink_supply[isNo].COND_Pressure_Actual > 20)
			{
				if(_InkSupply[isNo].pid_Setpoint.val > _MaxPrintPressure[isNo])
					_MaxPrintPressure[isNo] = _InkSupply[isNo].pid_Setpoint.val;
			}
			// reset pid if output at 0, no reason to accumulate negative value (never want pressure negative)
			if(_InkSupply[isNo].pid_Setpoint.val <= _InkSupply[isNo].pid_Setpoint.val_min)
				pid_reset(&_InkSupply[isNo].pid_Setpoint);
		}

		//-----------------------------------------------------------------------------------------
		//  PID 2 (pump) :  Pressure_IS_setpoint --> PID_pump --> Pump_speed (%)
		//-----------------------------------------------------------------------------------------

		if((print_mode == PUMP_CTRL_MODE_PRINT)||(print_mode == PUMP_CTRL_CHECK_HEADS)||(print_mode == PUMP_CTRL_CHECK_HEADS_FAST))
			_InkSupply[isNo].pid_Pump.Setpoint = pRX_Status->ink_supply[isNo].IS_Pressure_Setpoint;
		else _InkSupply[isNo].pid_Pump.Setpoint = pressure_target;

		if(print_mode == PUMP_CTRL_MODE_PRINT) _InkSupply[isNo].pid_Pump.I = 5000;
		else _InkSupply[isNo].pid_Pump.I = 2500;

		// Special function (putty) : Test bleed line, air valve not used so P divided by 5
		pRX_Status->ink_supply[isNo].PIDsetpoint_Output = _InkSupply[isNo].pid_Pump.Setpoint;
		pRX_Status->ink_supply[isNo].IS_Pressure_Setpoint = _InkSupply[isNo].pid_Pump.Setpoint;
		pid_calc(pRX_Status->ink_supply[isNo].IS_Pressure_Actual, &_InkSupply[isNo].pid_Pump);

		if      (_InkSupply[isNo].pid_Pump.val <=0)
		{
			_set_pump_speed(isNo, 0);
			// reset integrator if pump at 0 because the air valve manage the pressure decreasing
			pid_reset(&_InkSupply[isNo].pid_Pump);
		}
		else if (_InkSupply[isNo].pid_Pump.val <PUMP_THRESHOLD) _set_pump_speed(isNo, PUMP_THRESHOLD);
		else                                              		_set_pump_speed(isNo, _InkSupply[isNo].pid_Pump.val);

		pRX_Status->ink_supply[isNo].PIDpump_Output		= _InkSupply[isNo].pid_Pump.val;
		_PumpBeforeOFF = _InkSupply[isNo].pid_Pump.val;

		//---------------------------------------------------------------------------------------------------
		//  Controller 3 (Air valve) :  Pressure_IS_actual > Pressure_IS_setpoint --> Air valve open 100%
		//---------------------------------------------------------------------------------------------------

		if(print_mode != PUMP_CTRL_MODE_NO_AIR_VALVE)
		{
			if(pRX_Status->ink_supply[isNo].IS_Pressure_Actual > MAX_PRESSURE)	// for safety
			{
				_set_air_valve(isNo, TRUE);
			}
			else if(pRX_Status->ink_supply[isNo].IS_Pressure_Actual > _InkSupply[isNo].pid_Pump.Setpoint)
			{
				if (set_valve) _set_air_valve(isNo, TRUE);
				else _set_air_valve(isNo, TRUE);
			}
			else
			{
				if (set_valve) _set_air_valve(isNo, FALSE);
			}
		}

		//---------------------------------------------------------------------------------------------------
		//  Calibration :  Find Pressure_IS_setpoint to have 40ml/min in the  conditioners pumps (average)
		//---------------------------------------------------------------------------------------------------

		if(_CalibrationSetpoint[isNo])
		{
			// detect stability before calibrate the setpoint
			switch(pRX_Config->ink_supply[isNo].ctrl_mode)
			{
				case ctrl_cal_step1 :
					// start calibration : reset stability time count
					_CalibrationStability[isNo] = 0;
					pRX_Status->ink_supply[isNo].ctrl_state = ctrl_cal_step1;
					break;		// start calibration : reset stability time count
				case ctrl_cal_step2 :
					// wait for stability of cond pressure inlet (+/-1 mbar for 3 seconds

					if((pressure_target - pRX_Status->ink_supply[isNo].COND_Pressure_Actual > -10)&&(pressure_target - pRX_Status->ink_supply[isNo].COND_Pressure_Actual < 10))
						_CalibrationStability[isNo]++;
					else _CalibrationStability[isNo] = 0;
					if(_CalibrationStability[isNo] > 300)
					{
						pRX_Status->ink_supply[isNo].ctrl_state = ctrl_cal_step2;
						_CalibrationStability[isNo] = 0;
						_InkSupply[isNo].pid_Calibration.P 					= _InkSupply[isNo].pid_Setpoint.P / 3;
						_InkSupply[isNo].pid_Calibration.I 					= _InkSupply[isNo].pid_Setpoint.I * 2;
						_InkSupply[isNo].pid_Calibration.Start_Integrator	= 1;
						_InkSupply[isNo].pid_Calibration.diff_I = pressure_target * _InkSupply[isNo].pid_Calibration.I * 1000;
					}
					break;
				case ctrl_cal_step3 :
					// start PID to find the best Pressure inlet setpoint for 40ml/min

					_InkSupply[isNo].pid_Calibration.Setpoint = 400;
					pid_calc(pRX_Config->ink_supply[isNo].condPumpFeedback, &_InkSupply[isNo].pid_Calibration);
					_PressureSetpoint[isNo] = _InkSupply[isNo].pid_Calibration.val;
					pRX_Status->ink_supply[isNo].cylinderPresSet = _PressureSetpoint[isNo];

					// Until stability of the cond pump flow (+/- 1ml/min for 2 seconds)
					if((_InkSupply[isNo].pid_Calibration.Setpoint - pRX_Config->ink_supply[isNo].condPumpFeedback > -20)&&(_InkSupply[isNo].pid_Calibration.Setpoint - pRX_Config->ink_supply[isNo].condPumpFeedback < 20))
						_CalibrationStability[isNo]++;
					else _CalibrationStability[isNo] = 0;
					if(pRX_Config->ink_supply[isNo].condPumpFeedback > _InkSupply[isNo].pid_Calibration.Setpoint - 10)
					{
						_CalibrationSetpoint[isNo] = 0;
						pRX_Config->ink_supply[isNo].cylinderPresSet = pRX_Status->ink_supply[isNo].COND_Pressure_Actual;
						pRX_Status->ink_supply[isNo].ctrl_state = ctrl_cal_step3;
					}
					break;
				default : break;
			}

		}

		_trace_pump_ctrl(pressure_target);
}


//--- _set_pump_speed --------------------------------
void _set_pump_speed(int isNo, int speed)
{
	if (speed!=INVALID_VALUE)
	{
		IOWR_16DIRECT(AXI_LW_SLAVE_REGISTER_0_BASE, DAC_REG_0 + (isNo << 2), speed);
		//if (_InkSupply[isNo].pid.val_max) pRX_Status->ink_supply[isNo].inkPumpSpeed_set  = 600 * speed / _InkSupply[isNo].pid.val_max; // 100%=600 ml
		//if (_InkSupply[isNo].pid.val_max) pRX_Status->ink_supply[isNo].inkPumpSpeed_set = speed * 118 * 98 / 10 / 1023; // linear approximation from Excel
		if (_InkSupply[isNo].pid_Pump.val_max) _PumpSpeed1000[isNo] += 100 * speed / _InkSupply[isNo].pid_Pump.val_max; // in [%]
		else                              _PumpSpeed1000[isNo] += 0;
	}
}

//--- _get_pump_ticks -------------------------------------------------
static UINT32 _get_pump_ticks(int isNo)
{
	return IORD_32DIRECT(AXI_LW_SLAVE_REGISTER_0_BASE, ADC_FREQ_0 + (isNo << 2));
}

//--- _set_flush_pump ------------------------------------------
static void _set_flush_pump(int isNo, int state)
{
	if (state) _FlushPump |=  (1<<isNo);
	else	   _FlushPump &= ~(1<<isNo);

	if (pRX_Config->test_flush) pRX_Status->flush_pump_val = pRX_Config->test_flush;
	else
	{
		if (!_FlushPump)  pRX_Status->flush_pump_val=0;
		else
		{
			int i;
			int pres=0;

			for (i=0; i<NIOS_INK_SUPPLY_CNT; i++)
			{
				if ((_FlushPump &  (1<<i)) && (pRX_Config->ink_supply[i].condPresIn!=INVALID_VALUE))
				{
					if (pRX_Config->ink_supply[i].condPresIn>pres) pres=pRX_Config->ink_supply[i].condPresIn;
				}
			};
			if (pRX_Status->flush_pres_head<400) pRX_Status->flush_pump_val=100;
			if (pRX_Status->flush_pres_head>450) pRX_Status->flush_pump_val=0;
		}
	}
	pRX_Status->flush_pump 	= pRX_Status->flush_pump_val>0;
	IOWR_ALTERA_AVALON_PIO_DATA(FLUSH_PUMP_PWM_DUTY_CYCLE_BASE,	0x1000*pRX_Status->flush_pump_val/100);	// ON
}

//--- _set_air_pump ---------------------------------------------
static void _set_air_pump(int state)
{
	if (state)
		IOWR_ALTERA_AVALON_PIO_SET_BITS(PIO_OUTPUT_BASE, AIR_PUMP_OUT);		// Air Pump = on
	else
		IOWR_ALTERA_AVALON_PIO_CLEAR_BITS(PIO_OUTPUT_BASE, AIR_PUMP_OUT);	// Air Pump = off

	pRX_Status->air_pump = state;
}
