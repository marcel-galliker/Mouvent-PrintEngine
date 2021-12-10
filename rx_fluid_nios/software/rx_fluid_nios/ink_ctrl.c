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
#include "pvalve.h"
#include "pres.h"

//--- defines --------------------------------------------
#define 	MAX_PRES_DEVIATION			20	// % deviation allowed before Bleed solenoid kicks in
#define		MAX_PRINT_PRESSURE_FLUID	300
#define		MAX_PRESSURE_FLUID			2000

#define 	PRESSURE_SOFT_PURGE			80
#define 	PRESSURE_PURGE				160
#define 	PRESSURE_HARD_PURGE			320
#define 	PRESSURE_FLUSH				1000
#define		PRESSURE_CHECK_BLEED		1000

// #define	TIME_SOFT_PURGE				2000
// #define 	TIME_PURGE					3000
// #define 	TIME_HARD_PURGE				10000

#define 	TIME_EMPTY					3000
#define 	TIME_PRESSURE_FOR_FLUSH		200

#define		TIME_CALIBRATE				30000

#define 	DEGASSING_VACCUUM_UV		800
#define 	DEGASSING_VACCUUM_WB		800

#define 	INK_PUMP_VAL_MAX			511 // 614

// --- NEW : define for function _pump_ctrl
#define 	PUMP_CTRL_MODE_DEFAULT			0
#define 	PUMP_CTRL_MODE_PRINT			1
#define 	PUMP_CTRL_MODE_NO_AIR_VALVE		2
#define		PUMP_CTRL_INK_RECIRCULATION		5

#define		PID_SETPOINT_P_PRINT_8_HEADS	300
#define		PID_SETPOINT_P_PRINT_4_HEADS	200
#define		PID_SETPOINT_P_PRINT_HEAD		100
#define		PID_SETPOINT_P_CHECK_REDUCED	3

// Check sequence bleed line
#define 	TIME_BLEED_LINE_OK			2000
#define		TIME_BLEED_LINE_TIMEOUT40	4000
#define		TIME_BLEED_LINE_TIMEOUT90	9000

// -- END NEW

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

typedef struct
{
	INT32		Ctrl_Check_State;
	int			Check1_Pressure_Start;
	INT32		TimeState;
	int			TimeBleedLineOK;
	INT32		PressureAverage;
	INT32 		NbAverage;
	int 		PressureMIN;
	int			PressureMAX;
	int			Pressure;

} SCheckSequence;

static SInkSupply 		_InkSupply[NIOS_INK_SUPPLY_CNT];

static int	_LungVacc = 0;
// static int	_PurgeNo;
static int  _PressureTimer;
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
static UINT32 _LastPumpTicks[NIOS_INK_SUPPLY_CNT] = {0};
static UINT32 _PumpSpeed1000[NIOS_INK_SUPPLY_CNT] = {0};
static int 	_FlushTimeISPresStable[NIOS_INK_SUPPLY_CNT] = {0};
static int 	_FlushISPressureStable[NIOS_INK_SUPPLY_CNT] = {0};
static int  _TimeEmpty[NIOS_INK_SUPPLY_CNT] = {0};
static int 	_EmptyDetecTEndState[NIOS_INK_SUPPLY_CNT] = {0};
static int 	_EmptyPressureStored[NIOS_INK_SUPPLY_CNT] = {0};

// --- NEW  ---
static int 	_StartModePRINT[NIOS_INK_SUPPLY_CNT] = {0};

// --- Shutdown phase ---
static int 	_ShutdownPrint[NIOS_INK_SUPPLY_CNT] = {0};
static int 	_ShutdownTimeStability[NIOS_INK_SUPPLY_CNT] = {0};
static int 	_ShutdownTimeOut[NIOS_INK_SUPPLY_CNT] = {0};

// --- Detect Filter clogged in PRINT mode ---
static int 	_FilterCloggedTime[NIOS_INK_SUPPLY_CNT] = {0};

// --- Calcul duty for degasing to detect air leakage ---
static INT32	_DutyDegasserCalcul	= 1; // avoid 0% when first start
static INT32 	_DutyDegasserTimeClogged	= 0;
static INT32 	_DutyDegasserTimeLeakage	= 0;
static INT32    _DutyDegasserDuty	= 0;
static INT32    _DutyDegasserFirst	= 1;

// --- Calibration
// static int 	_TimeStabitilityCalibration[NIOS_INK_SUPPLY_CNT] = {0};
// static int 	_DecreaseSpeedCalibration[NIOS_INK_SUPPLY_CNT] = {0};

// --- Check
static SCheckSequence 	_CheckSequence[NIOS_INK_SUPPLY_CNT];
static int 	_TimeStabitilityCheck[NIOS_INK_SUPPLY_CNT] = {0};
static int 	_CheckStep4_PinletSaved[NIOS_INK_SUPPLY_CNT] = {0};
static int 	_CheckStep4_PoutletSaved[NIOS_INK_SUPPLY_CNT] = {0};
static int 	_CheckHeaterBoard_Temperature[NIOS_INK_SUPPLY_CNT] = {0};

// --- Purge
//static int 	_TimeStabitilityPurgePressure[NIOS_INK_SUPPLY_CNT] = {0};

// --- OLD NEW ---


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
}

//--- ink_error_reset ----------------------------------------------------
void ink_error_reset(void)
{
	int i;
	for (i=0; i<NIOS_INK_SUPPLY_CNT; i++)
	{
		pRX_Status->ink_supply[i].error = 0;
		_FilterCloggedTime[i]=0;
	}
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
			&& (pRX_Config->ink_supply[isNo].ctrl_mode < ctrl_check_step0 ||  pRX_Config->ink_supply[isNo].ctrl_mode > ctrl_check_step9)
			&& 	pRX_Config->ink_supply[isNo].ctrl_mode > ctrl_off
			&& (pRX_Config->ink_supply[isNo].ctrl_mode < ctrl_empty ||  pRX_Config->ink_supply[isNo].ctrl_mode > ctrl_empty+10)
			&& (pRX_Config->ink_supply[isNo].ctrl_mode < ctrl_flush_night ||  pRX_Config->ink_supply[isNo].ctrl_mode > ctrl_flush_done)
			)
			{
				_set_pump_speed(isNo, 0);
				_set_air_valve(isNo, PV_OPEN);
				_InkSupply[isNo].degassing=FALSE;
				_set_air_pump(FALSE);

				pRX_Status->ink_supply[isNo].error |= err_overpressure;
			}
		}

		// duty degasse
		pRX_Status->duty_degasser = _DutyDegasserCalcul;
	}

	for(isNo = 0 ; isNo < NIOS_INK_SUPPLY_CNT ; isNo++)
	{
		switch(pRX_Config->ink_supply[isNo].ctrl_mode)
		{
			case ctrl_shutdown:
				_PressureSetpoint[isNo] = pRX_Config->ink_supply[isNo].condPumpFeedback;
				// if PRINT mode, recalculate integrator with new PI parameters
				// if CALIBRATION or PURGE mode, don't need to recalculate
				if(_ShutdownPrint[isNo] == 1)
				{
					//_InkSupply[isNo].pid_Setpoint.diff_I *= PID_SETPOINT_P_CHECK_REDUCED;
					_ShutdownPrint[isNo] = 3;
				}

				pRX_Status->ink_supply[isNo].ctrl_state = pRX_Config->ink_supply[isNo].ctrl_mode;
				_ShutdownTimeStability[isNo] = 0;
				_ShutdownTimeOut[isNo] = 0;
				break;

			case ctrl_shutdown_done:

				if(_ShutdownPrint[isNo] >= 1)
				{
					if(_PressureSetpoint[isNo] > 30) _PressureSetpoint[isNo]--;
					else _PressureSetpoint[isNo] = 30;
					_pump_ctrl(isNo, _PressureSetpoint[isNo], PUMP_CTRL_INK_RECIRCULATION);

					if(pRX_Config->ink_supply[isNo].condPumpFeedback < 50) _ShutdownTimeStability[isNo]++;
					else _ShutdownTimeStability[isNo] = 0;
					if(_ShutdownTimeStability[isNo] > 500)	pRX_Status->ink_supply[isNo].ctrl_state = pRX_Config->ink_supply[isNo].ctrl_mode;

					_ShutdownTimeOut[isNo]++;
					if(_ShutdownTimeOut[isNo] > 12000) pRX_Status->ink_supply[isNo].ctrl_state = pRX_Config->ink_supply[isNo].ctrl_mode;
				}
				else pRX_Status->ink_supply[isNo].ctrl_state = pRX_Config->ink_supply[isNo].ctrl_mode;
				break;

			case ctrl_undef:
			case ctrl_off:

				pid_reset(&_InkSupply[isNo].pid_Pump);
				pid_reset(&_InkSupply[isNo].pid_Setpoint);

				// Keep the cylinder pressure close to 0 mbar in OFF
				if (pRX_Status->ink_supply[isNo].IS_Pressure_Actual > 10) _set_air_valve(isNo, PV_OPEN);
				else if (pRX_Status->ink_supply[isNo].IS_Pressure_Actual < -10) _set_air_valve(isNo, PV_CLOSED);
				_set_bleed_valve(isNo, PV_CLOSED);

				_ShutdownPrint[isNo] = 0;

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

				_InkSupply[isNo].degassing=pRX_Config->cmd.lung_needed && pRX_Config->cmd.lung_enabled;
				_InkSupply[isNo].purgeTime					= 0;
				_InkSupply[isNo].purgePressure				= 0;
				pRX_Status->ink_supply[isNo].flushTime		= 0;
				pRX_Status->ink_supply[isNo].ctrl_state		= ctrl_off;
				pRX_Status->ink_supply[isNo].PIDpump_Output	= 0;	// just for log file

				break;

			case ctrl_print:
				//_set_bleed_valve(isNo, PV_CLOSED);
				//_set_air_valve(isNo, PV_CLOSED);
				_set_flush_pump(isNo, FALSE);
				_InkSupply[isNo].degassing = pRX_Config->cmd.lung_enabled;
				_PumpOFFTime = 0;
				_ShutdownPrint[isNo] = 1;

				// ----- NEW : Ramp start-up pressure  -------
				if(pRX_Status->ink_supply[isNo].ctrl_state != pRX_Config->ink_supply[isNo].ctrl_mode)
				{
					if(pRX_Status->ink_supply[isNo].cylinderPresSet > 10)
						_PressureSetpoint[isNo] = pRX_Status->ink_supply[isNo].cylinderPresSet / 5;	// start with low setpoint
					else _PressureSetpoint[isNo] = pRX_Status->ink_supply[isNo].cylinderPresSet;
					_StartModePRINT[isNo] = 0;
				}
				else _StartModePRINT[isNo]++;

				if((_PressureSetpoint[isNo] != pRX_Status->ink_supply[isNo].cylinderPresSet)&&(_StartModePRINT[isNo] > 500))
				{
					if(_StartModePRINT[isNo] > 504)
					{
						if(_PressureSetpoint[isNo] < pRX_Status->ink_supply[isNo].cylinderPresSet) _PressureSetpoint[isNo]++;
						else _PressureSetpoint[isNo]--;
						_StartModePRINT[isNo] = 500;
					}
				}
				pRX_Status->ink_supply[isNo].ctrl_state = pRX_Config->ink_supply[isNo].ctrl_mode;

				// --- Detect filter clogged -------
				if(pRX_Status->ink_supply[isNo].IS_Pressure_Actual!=INVALID_VALUE
				&& pRX_Status->ink_supply[isNo].IS_Pressure_Actual > 900)
				{
					_FilterCloggedTime[isNo]++;
					if(_FilterCloggedTime[isNo] > 6000)		// 1 minute over 900 mbars
						pRX_Status->ink_supply[isNo].error |= err_filter_clogged;
				}
				else _FilterCloggedTime[isNo] = 0;
				// ----- END NEW  -------

				//_pump_ctrl(isNo, _PressureSetpoint[isNo], PUMP_CTRL_MODE_PRINT);
				_pump_ctrl(isNo, _PressureSetpoint[isNo], PUMP_CTRL_INK_RECIRCULATION);

				break;

			case ctrl_test:
				if (pRX_Status->ink_supply[isNo].ctrl_state==ctrl_test)
				{
					if (pRX_Config->ink_supply[isNo].test_bleed_line)
					{
						switch(pRX_Status->ink_supply[isNo].TestBleedLine_Phase)
						{
							case 1:
								_set_air_valve(isNo, PV_CLOSED);
								_set_bleed_valve(isNo, PV_CLOSED);

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

							case 2:
								_set_air_valve(isNo, PV_OPEN);
								_set_bleed_valve(isNo, PV_CLOSED);
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

							case 3:
								_set_air_valve(isNo, PV_CLOSED);
								_set_bleed_valve(isNo, PV_OPEN);
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
						_set_air_valve  (isNo, pRX_Config->ink_supply[isNo].test_airValve);
						_set_bleed_valve(isNo, pRX_Config->ink_supply[isNo].test_bleedValve);
						_pump_ctrl(isNo, pRX_Config->ink_supply[isNo].test_cylinderPres, PUMP_CTRL_MODE_NO_AIR_VALVE);		// ink-pump
						_set_pressure_value((pRX_Config->test_airPressure>0) && (pRX_Status->air_pressure < pRX_Config->test_airPressure));	// air-pump
					}
				}
				else
				{
					// --- reset values -----------
					pRX_Config->ink_supply[isNo].test_airValve  	= PV_CLOSED;
					pRX_Config->ink_supply[isNo].test_bleedValve 	= PV_CLOSED;
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
				_set_air_valve(isNo, PV_CLOSED);
				_set_bleed_valve(isNo, PV_CLOSED);
				_InkSupply[isNo].degassing=pRX_Config->cmd.lung_enabled;
				_pump_ctrl(isNo, pRX_Status->ink_supply[isNo].cylinderPresSet, PUMP_CTRL_MODE_DEFAULT);
				pRX_Status->ink_supply[isNo].ctrl_state = ctrl_warmup;
				break;

			case ctrl_readyToPrint:
				break;

			case ctrl_check_step0:
				_set_bleed_valve(isNo, PV_CLOSED);
				_set_air_valve(isNo, PV_CLOSED);
				_set_flush_pump(isNo, FALSE);
				_PumpOFFTime = 0;

				_PressureSetpoint[isNo] = 500;
				_CheckSequence[isNo].Ctrl_Check_State = 1;
				_CheckSequence[isNo].Check1_Pressure_Start = pRX_Status->ink_supply[isNo].IS_Pressure_Actual;
				_CheckSequence[isNo].TimeState = 0;
				_CheckHeaterBoard_Temperature[isNo] = pRX_Status->ink_supply[isNo].heaterTemp;

				pRX_Status->ink_supply[isNo].ctrl_state = pRX_Config->ink_supply[isNo].ctrl_mode;
				break;

			case ctrl_check_step1:

				pRX_Status->ink_supply[isNo].Check_State = _CheckSequence[isNo].Ctrl_Check_State;
				pRX_Status->ink_supply[isNo].Check_Time_State = _CheckSequence[isNo].TimeState;

				// ------ for test (to bypass steps)
				// _CheckSequence[isNo].Ctrl_Check_State = 0;
				// pRX_Status->ink_supply[isNo].ctrl_state = pRX_Config->ink_supply[isNo].ctrl_mode;
				// -----

				// if Error found in the check sequence
				if(_CheckSequence[isNo].Ctrl_Check_State == 0)
				{
					_set_air_valve(isNo, PV_OPEN);
					_set_pump_speed(isNo, 0);
					pRX_Status->ink_supply[isNo].ctrl_state = pRX_Config->ink_supply[isNo].ctrl_mode;
				}
				else _set_air_valve(isNo, FALSE);

				_set_bleed_valve(isNo, PV_CLOSED);
				_set_flush_pump(isNo, FALSE);
				pRX_Status->ink_supply[isNo].IS_Pressure_Setpoint = 500;

				if(pRX_Status->ink_supply[isNo].ctrl_state != ctrl_check_step1)
				{
					switch(_CheckSequence[isNo].Ctrl_Check_State)
					{
						case 1 :	// Regul IS pressure for 10 seconds
							_pump_ctrl(isNo, _PressureSetpoint[isNo], PUMP_CTRL_MODE_DEFAULT);
							_CheckSequence[isNo].TimeState++;
							if(_CheckSequence[isNo].TimeState > 1000) _CheckSequence[isNo].Ctrl_Check_State++;
							break;

						case 2 :	// Check Ink pump defected
							if((_InkSupply[isNo].pid_Pump.val == _InkSupply[isNo].pid_Pump.val_max) && (pRX_Status->ink_supply[isNo].IS_Pressure_Actual - _CheckSequence[isNo].Check1_Pressure_Start < 10))
							{
								// RED fault : Ink pump defected
								pRX_Status->ink_supply[isNo].error |= err_ink_pump;
								_CheckSequence[isNo].TimeState = 0;
								_CheckSequence[isNo].Ctrl_Check_State = 0;
							}
							else _CheckSequence[isNo].Ctrl_Check_State++;
							break;

						case 3 :	// Check INK tube clogged
							if((_InkSupply[isNo].pid_Pump.val == _InkSupply[isNo].pid_Pump.val_max) && (pRX_Status->ink_supply[isNo].IS_Pressure_Actual < 250))
							{
								// RED fault : INK tube clogged
								pRX_Status->ink_supply[isNo].error |= err_ink_tube_clogged;
								_CheckSequence[isNo].TimeState = 0;
								_CheckSequence[isNo].Ctrl_Check_State = 0;
							}
							else if((_InkSupply[isNo].pid_Pump.val > 450) && (pRX_Status->ink_supply[isNo].IS_Pressure_Actual < 400))
							{
								// RED fault : Cylinder empty or sucking a lot of air
								pRX_Status->ink_supply[isNo].error |= err_cylinder_empty;
								_CheckSequence[isNo].TimeState = 0;
								_CheckSequence[isNo].Ctrl_Check_State = 0;
							}
							else _CheckSequence[isNo].Ctrl_Check_State++;
							break;

						case 4 :	// Check BLEED tube clogged
							if((_InkSupply[isNo].pid_Pump.val == _InkSupply[isNo].pid_Pump.val_min) && (pRX_Status->ink_supply[isNo].IS_Pressure_Actual > 525))
							{
								// RED fault : BLEED tube clogged
								pRX_Status->ink_supply[isNo].error |= err_bleed_tube_clogged;
								_CheckSequence[isNo].TimeState = 0;
								_CheckSequence[isNo].Ctrl_Check_State = 0;
							}
							else _CheckSequence[isNo].Ctrl_Check_State++;
							_CheckSequence[isNo].TimeState = 0;
							_CheckSequence[isNo].PressureAverage = 0;
							_CheckSequence[isNo].PressureMIN = 1200;
							_CheckSequence[isNo].PressureMAX = -100;
							break;

						case 5 :	// Continue Regul IS pressure for 3 seconds
							_pump_ctrl(isNo, _PressureSetpoint[isNo], PUMP_CTRL_MODE_DEFAULT);
							// calculate average, Min and Max
							_CheckSequence[isNo].PressureAverage += pRX_Status->ink_supply[isNo].IS_Pressure_Actual;
							_CheckSequence[isNo].NbAverage++;
							if(pRX_Status->ink_supply[isNo].IS_Pressure_Actual < _CheckSequence[isNo].PressureMIN)
								_CheckSequence[isNo].PressureMIN = pRX_Status->ink_supply[isNo].IS_Pressure_Actual;
							if(pRX_Status->ink_supply[isNo].IS_Pressure_Actual > _CheckSequence[isNo].PressureMAX)
								_CheckSequence[isNo].PressureMAX = pRX_Status->ink_supply[isNo].IS_Pressure_Actual;

							_CheckSequence[isNo].TimeState++;
							if(_CheckSequence[isNo].TimeState > 300)
							{
								_CheckSequence[isNo].Ctrl_Check_State++;
								_CheckSequence[isNo].PressureAverage /= _CheckSequence[isNo].NbAverage;
							}
							break;

						case 6 :	// Check Damper defective
							if((_CheckSequence[isNo].PressureAverage > 490)&&(_CheckSequence[isNo].PressureAverage < 510)
									&&(_CheckSequence[isNo].PressureMAX - _CheckSequence[isNo].PressureMIN > 25))
							{
								// RED fault : Damper defective
								pRX_Status->ink_supply[isNo].error |= err_damper;
								_CheckSequence[isNo].TimeState = 0;
								_CheckSequence[isNo].Ctrl_Check_State = 0;
							}
							else _CheckSequence[isNo].Ctrl_Check_State++;
							break;

						case 7 :	// Check air in the circuit
							if((_CheckSequence[isNo].PressureAverage < 490)&&(_CheckSequence[isNo].PressureMAX - _CheckSequence[isNo].PressureMIN > 25))
							{
								_CheckSequence[isNo].Ctrl_Check_State++;
								_CheckSequence[isNo].TimeState = 0;
								_CheckSequence[isNo].PressureAverage = 0;
								_CheckSequence[isNo].PressureMIN = 1200;
								_CheckSequence[isNo].PressureMAX = -100;
							}
							else
							{
								// CHECK 0 OK - No problem
								_CheckSequence[isNo].Ctrl_Check_State = 1;
								_CheckSequence[isNo].TimeState = 0;
								pRX_Status->ink_supply[isNo].ctrl_state = pRX_Config->ink_supply[isNo].ctrl_mode;
							}
							break;

						case 8 : 	// Continue regulIS for 15 seconds, time to remove air in cylinder if foam
							_pump_ctrl(isNo, _PressureSetpoint[isNo], PUMP_CTRL_MODE_DEFAULT);
							_CheckSequence[isNo].TimeState++;
							if(_CheckSequence[isNo].TimeState > 1800)
							{
								_CheckSequence[isNo].Ctrl_Check_State++;
								_CheckSequence[isNo].PressureAverage /= _CheckSequence[isNo].NbAverage;
							}
							else if(_CheckSequence[isNo].TimeState > 1500)
							{
								_CheckSequence[isNo].PressureAverage += pRX_Status->ink_supply[isNo].IS_Pressure_Actual;
								_CheckSequence[isNo].NbAverage++;
								if(pRX_Status->ink_supply[isNo].IS_Pressure_Actual < _CheckSequence[isNo].PressureMIN)
									_CheckSequence[isNo].PressureMIN = pRX_Status->ink_supply[isNo].IS_Pressure_Actual;
								if(pRX_Status->ink_supply[isNo].IS_Pressure_Actual > _CheckSequence[isNo].PressureMAX)
									_CheckSequence[isNo].PressureMAX = pRX_Status->ink_supply[isNo].IS_Pressure_Actual;
							}
							break;

						case 9 :	// Re-check oscillation
							if((_CheckSequence[isNo].PressureAverage < 490)&&(_CheckSequence[isNo].PressureMAX - _CheckSequence[isNo].PressureMIN > 25))
							{
								// RED fault : INK tube sucking air
								pRX_Status->ink_supply[isNo].error |= err_ink_tube_suck_air;
								_CheckSequence[isNo].Ctrl_Check_State = 0;
							}
							else
							{
								// YELLOW fault : Foam removed in top of cylinder (possible air sucked from the clusters or somewhere else)
								pRX_Status->ink_supply[isNo].error |= err_Foam;
								_CheckSequence[isNo].Ctrl_Check_State = 0;
							}
							_CheckSequence[isNo].TimeState = 0;
							break;
					}
				}
				break;

			case ctrl_check_step2:

				pRX_Status->ink_supply[isNo].Check_State = _CheckSequence[isNo].Ctrl_Check_State;
				pRX_Status->ink_supply[isNo].Check_Time_State = _CheckSequence[isNo].TimeState;

				// ------ for test (to bypass steps)
				//_CheckSequence[isNo].Ctrl_Check_State = 0;
				//_CheckSequence[isNo].TimeState = 0;
				//pRX_Status->ink_supply[isNo].ctrl_state = pRX_Config->ink_supply[isNo].ctrl_mode;
				// -----

				// if Error found in the Check sequence
				if(_CheckSequence[isNo].Ctrl_Check_State == 0)
				{
					_set_air_valve(isNo, PV_OPEN);
					_set_bleed_valve(isNo, PV_CLOSED);
					_set_pump_speed(isNo, 0);
					pRX_Status->ink_supply[isNo].ctrl_state = pRX_Config->ink_supply[isNo].ctrl_mode;
				}

				if(pRX_Status->ink_supply[isNo].ctrl_state != ctrl_check_step2)
				{
					switch(_CheckSequence[isNo].Ctrl_Check_State)
					{
						case 1 :
							_CheckSequence[isNo].TimeBleedLineOK = 0;

							_set_air_valve(isNo, PV_OPEN);
							_set_pump_speed(isNo, 0);

							// Wait for IS pressure going down before start the next check (or Timeout 10 seconds)
							_CheckSequence[isNo].TimeState++;
							if((pRX_Status->ink_supply[isNo].IS_Pressure_Actual < 100)||(_CheckSequence[isNo].TimeState > 1000))
									_CheckSequence[isNo].Ctrl_Check_State++;
							break;

						case 2: // Regul IS Pressure without air valve : OK if P>1000 for 20 seconds - Timeout 1 minute
							_set_air_valve(isNo, PV_CLOSED);
							_set_bleed_valve(isNo, PV_CLOSED);
							pRX_Status->ink_supply[isNo].IS_Pressure_Setpoint = PRESSURE_CHECK_BLEED;

							if(pRX_Status->ink_supply[isNo].IS_Pressure_Actual > 700)
							{
								_set_pump_speed(isNo, 0);
								_CheckSequence[isNo].TimeBleedLineOK++;
								if(_CheckSequence[isNo].TimeBleedLineOK > TIME_BLEED_LINE_OK)
								{
									_CheckSequence[isNo].Ctrl_Check_State++;
									_CheckSequence[isNo].TimeBleedLineOK = 0;
									_CheckSequence[isNo].TimeState = 0;
								}
							}
							else
							{
								_pump_ctrl(isNo, 1000,PUMP_CTRL_MODE_NO_AIR_VALVE);
								_CheckSequence[isNo].TimeState++;
								_CheckSequence[isNo].TimeBleedLineOK = 0;
								if(_CheckSequence[isNo].TimeState > TIME_BLEED_LINE_TIMEOUT40)
								{
									pRX_Status->ink_supply[isNo].error |= err_valves_leakage;
									_CheckSequence[isNo].Ctrl_Check_State = 0;
								}
							}
							pRX_Status->ink_supply[isNo].TestBleedLine_Pump_Phase1 = pRX_Status->ink_supply[isNo].IS_Pressure_Actual;
							break;

						case 3: // Regul IS Pressure with air valve OPENED : OK if P>1000 for 20 seconds - Timeout 1 minute
							_set_air_valve(isNo, PV_OPEN);
							_set_bleed_valve(isNo, PV_CLOSED);

							if(pRX_Status->ink_supply[isNo].IS_Pressure_Actual > 700)
							{
								_CheckSequence[isNo].TimeBleedLineOK++;
								if(_CheckSequence[isNo].TimeBleedLineOK > TIME_BLEED_LINE_OK)
								{

									pRX_Status->ink_supply[isNo].TestBleedLine_Pump_Phase2 = 100 * _InkSupply[isNo].pid_Pump.val / _InkSupply[isNo].pid_Pump.val_max;
									if(pRX_Status->ink_supply[isNo].TestBleedLine_Pump_Phase2 < 25)
									{
										pRX_Status->ink_supply[isNo].error |= err_air_valve_tight;
										_CheckSequence[isNo].Ctrl_Check_State = 0;
									}
									else
									{
										//pRX_Status->ink_supply[isNo].error |= err_air_valveOK;
										_CheckSequence[isNo].Ctrl_Check_State++;
										_CheckSequence[isNo].TimeBleedLineOK = 0;
										_CheckSequence[isNo].TimeState = 0;
									}
								}
							}
							else
							{
								_CheckSequence[isNo].TimeState++;
								_CheckSequence[isNo].TimeBleedLineOK = 0;
								if(_CheckSequence[isNo].TimeState > TIME_BLEED_LINE_TIMEOUT90)
								{
									pRX_Status->ink_supply[isNo].error |= err_air_valve;
									pRX_Status->ink_supply[isNo].TestBleedLine_Pump_Phase2 = pRX_Status->ink_supply[isNo].IS_Pressure_Actual;
									_CheckSequence[isNo].Ctrl_Check_State = 0;
								}
							}
							_pump_ctrl(isNo, PRESSURE_CHECK_BLEED,PUMP_CTRL_MODE_NO_AIR_VALVE);
							break;

						case 4: // Regul IS Pressure with air valve OPENED : OK if P>1000 for 20 seconds - Timeout 1 minute
							_set_air_valve(isNo, PV_CLOSED);
							_set_bleed_valve(isNo, PV_OPEN);

							if(pRX_Status->ink_supply[isNo].IS_Pressure_Actual > 700)
							{
								_CheckSequence[isNo].TimeBleedLineOK++;
								if(_CheckSequence[isNo].TimeBleedLineOK > TIME_BLEED_LINE_OK)
								{
									pRX_Status->ink_supply[isNo].TestBleedLine_Pump_Phase3 = 100 * _InkSupply[isNo].pid_Pump.val / _InkSupply[isNo].pid_Pump.val_max;
									if(pRX_Status->ink_supply[isNo].TestBleedLine_Pump_Phase2 < 25)
									{
										pRX_Status->ink_supply[isNo].error |= err_bleed_valve_tight;
										_CheckSequence[isNo].Ctrl_Check_State = 0;
									}
									else
									{
										//pRX_Status->ink_supply[isNo].error |= err_bleed_valveOK;
										pRX_Status->ink_supply[isNo].ctrl_state = pRX_Config->ink_supply[isNo].ctrl_mode;
									}
								}
							}
							else
							{
								_CheckSequence[isNo].TimeState++;
								_CheckSequence[isNo].TimeBleedLineOK = 0;
								if(_CheckSequence[isNo].TimeState > TIME_BLEED_LINE_TIMEOUT90)
								{
									pRX_Status->ink_supply[isNo].error |= err_bleed_valve;
									pRX_Status->ink_supply[isNo].TestBleedLine_Pump_Phase3 = pRX_Status->ink_supply[isNo].IS_Pressure_Actual;
									_CheckSequence[isNo].Ctrl_Check_State = 0;
									_CheckSequence[isNo].Ctrl_Check_State = 0;
								}
							}
							_pump_ctrl(isNo, PRESSURE_CHECK_BLEED,PUMP_CTRL_MODE_NO_AIR_VALVE);
							break;
					}
				}
				else _TimeStabitilityCheck[isNo] = 0;
				break;

			case ctrl_check_step3:

				_PressureSetpoint[isNo] = 400;
				pid_reset(&_InkSupply[isNo].pid_Pump);
				pid_reset(&_InkSupply[isNo].pid_Setpoint);
				_set_air_valve(isNo, PV_OPEN);
				_set_bleed_valve(isNo, PV_CLOSED);
				_set_pump_speed(isNo, 0);
				if((pRX_Status->ink_supply[isNo].IS_Pressure_Actual < 50)||(pRX_Status->ink_supply[isNo].error))
				{
					_TimeStabitilityCheck[isNo]++;
					pRX_Status->ink_supply[isNo].ctrl_state = pRX_Config->ink_supply[isNo].ctrl_mode;
				}

				/*if(!_CheckSequence[isNo].Ctrl_Check_State)*/ _CheckSequence[isNo].Ctrl_Check_State = 1;
				_CheckSequence[isNo].TimeState = 0;
				break;

			case ctrl_check_step4:
				pRX_Status->ink_supply[isNo].Check_State = _CheckSequence[isNo].Ctrl_Check_State;
				pRX_Status->ink_supply[isNo].Check_Time_State = _CheckSequence[isNo].TimeState;
				_set_air_valve(isNo, PV_CLOSED);
				_set_bleed_valve(isNo, PV_CLOSED);
				_set_flush_pump(isNo, FALSE);

				if(_CheckSequence[isNo].Ctrl_Check_State == 0)
					pRX_Status->ink_supply[isNo].ctrl_state = pRX_Config->ink_supply[isNo].ctrl_mode;

				_pump_ctrl(isNo, _PressureSetpoint[isNo], PUMP_CTRL_INK_RECIRCULATION);

				_CheckSequence[isNo].TimeState++;
				if(pRX_Status->ink_supply[isNo].ctrl_state != ctrl_check_step4)
				{
					switch(_CheckSequence[isNo].Ctrl_Check_State)
					{
						case 1 :	// wait 2 seconds and save Pinlets
							if(_CheckSequence[isNo].TimeState > 200)
							{
								_CheckStep4_PinletSaved[isNo] = pRX_Status->ink_supply[isNo].COND_Pressure_Actual;
								_CheckStep4_PoutletSaved[isNo] = pRX_Status->ink_supply[isNo].condPresOut;
								_CheckSequence[isNo].Ctrl_Check_State++;
							}
							break;

						case 2 :
							// after 30 seconds, Pinlet must have increase
							if(_CheckSequence[isNo].TimeState > 3000)
							{
								if((pRX_Status->ink_supply[isNo].COND_Pressure_Actual < _CheckStep4_PinletSaved[isNo])
									&&(pRX_Config->ink_supply[isNo].condPumpFeedback < 50))
								{
									pRX_Status->ink_supply[isNo].error |= err_feed_tube;
									_CheckSequence[isNo].Ctrl_Check_State = 0;
								}
								else
								{
									_CheckSequence[isNo].Ctrl_Check_State++;
									pRX_Status->ink_supply[isNo].ctrl_state = pRX_Config->ink_supply[isNo].ctrl_mode;
								}

								_CheckSequence[isNo].TimeState = 0;
							}
							// or if all cond pump > 60%, return pipe clogged
							if((pRX_Status->ink_supply[isNo].condPresOut > _CheckStep4_PoutletSaved[isNo] - 100)
									&&(pRX_Config->ink_supply[isNo].condPumpFeedback >= 300))
							{
								pRX_Status->ink_supply[isNo].error |= err_return_pipe;
								_CheckSequence[isNo].Ctrl_Check_State = 0;
								_CheckSequence[isNo].TimeState = 0;
							}
							break;
					}
				}

				break;

			case ctrl_check_step5:
				pRX_Status->ink_supply[isNo].Check_State = _CheckSequence[isNo].Ctrl_Check_State;
				pRX_Status->ink_supply[isNo].Check_Time_State = _CheckSequence[isNo].TimeState;
				_set_air_valve(isNo, PV_CLOSED);
				_set_bleed_valve(isNo, PV_CLOSED);
				_set_flush_pump(isNo, FALSE);

				if((_CheckSequence[isNo].Ctrl_Check_State == 0)||(pRX_Status->ink_supply[isNo].error))
					pRX_Status->ink_supply[isNo].ctrl_state = pRX_Config->ink_supply[isNo].ctrl_mode;
				_pump_ctrl(isNo, _PressureSetpoint[isNo], PUMP_CTRL_INK_RECIRCULATION);

				_CheckSequence[isNo].TimeState++;
				if(pRX_Status->ink_supply[isNo].ctrl_state != ctrl_check_step5)
				{
					switch(_CheckSequence[isNo].Ctrl_Check_State)
					{
						case 3 :	// Timeout 3 minutes
							if(_CheckSequence[isNo].TimeState > 18000)
							{
								pRX_Status->ink_supply[isNo].error |= err_check4_timeout;
								_CheckSequence[isNo].Ctrl_Check_State = 0;
							}
							// IS pressure > 900 mbars
							else if(pRX_Status->ink_supply[isNo].IS_Pressure_Actual > 900)
							{
								pRX_Status->ink_supply[isNo].error |= err_filter_clogged;
								_CheckSequence[isNo].Ctrl_Check_State = 0;
							}
							// OK
							else if((pRX_Config->ink_supply[isNo].condPumpFeedback >= 400) && (_CheckSequence[isNo].TimeState > 300))
							{
								pRX_Status->ink_supply[isNo].ctrl_state = pRX_Config->ink_supply[isNo].ctrl_mode;

								// Check heater board
								if(pRX_Config->ink_supply[isNo].heaterTempMax > 39000)	// means heater board connected
								{
									// Not heating
									if(pRX_Config->ink_supply[isNo].heaterTempMax - _CheckHeaterBoard_Temperature[isNo] < 5000)
										pRX_Status->ink_supply[isNo].error |= err_ink_not_heating;
									else if(_CheckHeaterBoard_Temperature[isNo] - pRX_Config->ink_supply[isNo].heaterTempMax > 2000)
										pRX_Status->ink_supply[isNo].error |= err_ink_too_hot;
								}
							}
							break;
					}
				}
				break;

			case ctrl_check_step6:
			case ctrl_check_step7:
			case ctrl_check_step8:
			case ctrl_check_step9:

				if(pRX_Status->ink_supply[isNo].error == 0)
				{
					_pump_ctrl(isNo, _PressureSetpoint[isNo], PUMP_CTRL_INK_RECIRCULATION);
					_ShutdownPrint[isNo] = 2;
				}
				else
				{
					_set_air_valve(isNo, PV_OPEN);
					_set_bleed_valve(isNo, PV_CLOSED);
					_set_pump_speed(isNo, 0);
					_ShutdownPrint[isNo] = 0;
				}

				pRX_Status->ink_supply[isNo].ctrl_state = pRX_Config->ink_supply[isNo].ctrl_mode;
				break;

/*
			case ctrl_cal_start:
				_TimeStabitilityCalibration[isNo] = 0;
				_PressureSetpoint[isNo] = 400;
				pid_reset(&_InkSupply[isNo].pid_Pump);
				pid_reset(&_InkSupply[isNo].pid_Setpoint);
				pRX_Status->ink_supply[isNo].ctrl_state = pRX_Config->ink_supply[isNo].ctrl_mode;
				_ShutdownPrint[isNo] = 2;
				break;

			case ctrl_cal_step1:
				_set_air_valve(isNo, PV_CLOSED);
				_set_bleed_valve(isNo, PV_CLOSED);
				_set_flush_pump(isNo, FALSE);

				if(_TimeStabitilityCalibration[isNo] < 400)
				{
					if(abs(pRX_Config->ink_supply[isNo].condPumpFeedback - _PressureSetpoint[isNo]) < 15)
						_TimeStabitilityCalibration[isNo]++;
					else _TimeStabitilityCalibration[isNo] = 0;
				}
				else pRX_Status->ink_supply[isNo].ctrl_state = pRX_Config->ink_supply[isNo].ctrl_mode;

				_pump_ctrl(isNo, _PressureSetpoint[isNo], PUMP_CTRL_INK_RECIRCULATION);

				break;

			case ctrl_cal_step2:
				// save pressure actual as the first Pressure setpoint
				//_PressureSetpoint[isNo] = pRX_Status->ink_supply[isNo].COND_Pressure_Actual;
				if(_TimeStabitilityCalibration[isNo] == 400)
				{
					_PressureSetpoint[isNo] = pRX_Status->ink_supply[isNo].IS_Pressure_Setpoint;
					_DecreaseSpeedCalibration[isNo] = _PressureSetpoint[isNo] * 15 / 2000;
					if(_DecreaseSpeedCalibration[isNo] < 1) _DecreaseSpeedCalibration[isNo] = 1;
				}

				_TimeStabitilityCalibration[isNo]--;
				if(_TimeStabitilityCalibration[isNo] < 15)
					pRX_Status->ink_supply[isNo].ctrl_state = pRX_Config->ink_supply[isNo].ctrl_mode;
				_pump_ctrl(isNo, _PressureSetpoint[isNo], PUMP_CTRL_MODE_DEFAULT);


				break;

			case ctrl_cal_step3:
				// decrease Pressure setpoint slowly
				//-----------------------------------
				pid_reset(&_InkSupply[isNo].pid_Setpoint);
				_TimeStabitilityCalibration[isNo] -= _DecreaseSpeedCalibration[isNo];
				if(_TimeStabitilityCalibration[isNo] <= 0)
				{
					if(_PressureSetpoint[isNo] > 0) _PressureSetpoint[isNo]-=1;
					_TimeStabitilityCalibration[isNo] = 15;
				}
				_pump_ctrl(isNo, _PressureSetpoint[isNo], PUMP_CTRL_MODE_DEFAULT);
				pRX_Status->ink_supply[isNo].ctrl_state = pRX_Config->ink_supply[isNo].ctrl_mode;

				break;

			case ctrl_cal_step4:
				// increase Pressure setpoint to remove bubbles
				//----------------------------------------------
				if(pRX_Status->ink_supply[isNo].cylinderPresSet != 400)
				{
					pid_reset(&_InkSupply[isNo].pid_Pump);
					_PressureSetpoint[isNo] = 400;
				}
				_pump_ctrl(isNo, _PressureSetpoint[isNo], PUMP_CTRL_INK_RECIRCULATION);

				if(_TimeStabitilityCalibration[isNo] < 2000)
				{
					if(abs(pRX_Config->ink_supply[isNo].condPumpFeedback - _PressureSetpoint[isNo]) < 100)
						_TimeStabitilityCalibration[isNo]++;
					else _TimeStabitilityCalibration[isNo] = 0;
				}
				else pRX_Status->ink_supply[isNo].ctrl_state = pRX_Config->ink_supply[isNo].ctrl_mode;

				break;

			case ctrl_cal_done:

				pRX_Status->ink_supply[isNo].ctrl_state = pRX_Config->ink_supply[isNo].ctrl_mode;

				break;
*/


				/*
			case ctrl_bleed:
				_set_air_valve(isNo, PV_CLOSED);
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
				_set_air_valve(isNo, PV_CLOSED);
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
					_set_air_valve(isNo, PV_CLOSED);
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
			case ctrl_purge_hard_vacc:	_init_purge(isNo, PRESSURE_HARD_PURGE); break;

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
				if (pRX_Config->ink_supply[isNo].purgeTime==0 || _InkSupply[isNo].purgeTime < pRX_Config->ink_supply[isNo].purgeTime)
				{
					_pump_ctrl(isNo, _InkSupply[isNo].purgePressure, PUMP_CTRL_MODE_DEFAULT);
					_set_bleed_valve(isNo, PV_CLOSED);
					_InkSupply[isNo].purgeTime+=cycleTime;
				}
				else
				{
					_InkSupply[isNo].purgePressure = 0;
					_set_pump_speed(isNo, 0);
					_set_air_valve(isNo, PV_OPEN);
					
					if (pRX_Status->ink_supply[isNo].IS_Pressure_Actual < 100) pRX_Status->ink_supply[isNo].ctrl_state = ctrl_purge_step4;
				}
				break;

			case ctrl_purge_step5:	  					
					pRX_Status->ink_supply[isNo].ctrl_state = ctrl_purge_step5;
				break;

			// --- FILL --------------------------------------------------
			case ctrl_fill:
				_InkSupply[isNo].degassing = FALSE;
				_set_air_valve(isNo, PV_OPEN);
				_set_bleed_valve(isNo, PV_CLOSED);
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
				_set_air_valve(isNo, PV_OPEN);
				_set_bleed_valve(isNo, PV_CLOSED);
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
				_set_bleed_valve(isNo, PV_OPEN);
				_set_air_valve(isNo, PV_CLOSED);
				pRX_Status->ink_supply[isNo].ctrl_state = ctrl_empty;
				break;

			case ctrl_empty_step1:
				_set_bleed_valve(isNo, PV_OPEN);
				_set_air_valve(isNo, PV_CLOSED);
				for (i=0; i<NIOS_INK_SUPPLY_CNT; i++)
				{
					_set_pump_speed(i, 0);
					_InkSupply[i].degassing = FALSE;
				}
				_pump_ctrl(isNo, 0,PUMP_CTRL_MODE_NO_AIR_VALVE);		// ink-pump
				_TimeEmpty[isNo] = 0;
				_EmptyDetecTEndState[isNo] = 1;
				empty_pressure = 400;
				pRX_Status->ink_supply[isNo].ctrl_state = ctrl_empty_step1;
				break;

			case ctrl_empty_step2:
				_set_bleed_valve(isNo, PV_OPEN);
				_set_air_valve(isNo, PV_CLOSED);
				if(pRX_Status->ink_supply[isNo].IS_Pressure_Actual< 400) empty_pressure +=1; //100 * pRX_Config->headsPerColor;
				//if(empty_pressure < 400) empty_pressure = 400;
				//if(empty_pressure > 800) empty_pressure = 800;
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
						if(empty_pressure > 900) _EmptyDetecTEndState[isNo]++;
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
				_set_bleed_valve(isNo, PV_CLOSED);
				_set_air_valve(isNo, PV_CLOSED);
				pRX_Status->ink_supply[isNo].ctrl_state = ctrl_empty_step3;
				break;

				/*
			case ctrl_shutdown:
				//--- stop pumps, open air valve and wait until pressure low ---
				if (pRX_Config->ink_supply[isNo].ctrl_mode != pRX_Status->ink_supply[isNo].ctrl_state)
					_shutdown_timeout = 6000;

				if (_shutdown_timeout)
					--_shutdown_timeout;

				_set_bleed_valve(isNo, PV_CLOSED);
				_set_pressure_value(FALSE);
				_set_pump_speed(isNo, 0);
				_set_air_valve(isNo, PV_CLOSED);
				_PurgePressure = 0;
				_PurgeNo = -1;

				if (pRX_Status->ink_supply[isNo].cylinderPres <= INK_LOW_PRESSURE &&
					_shutdown_timeout == 0)
				{
					_set_air_valve(isNo, PV_CLOSED);
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
		trprintf("air pressure:  "); trprintf("%6d\n", pRX_Status->air_pressure);
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
		_set_bleed_valve(isNo, PV_CLOSED);

	//	_PurgeNo	   = isNo;
		if(pRX_Config->ink_supply[isNo].purge_putty_pressure)
		{
			_InkSupply[isNo].purgePressure = pRX_Config->ink_supply[isNo].purge_putty_pressure;
		}
		else
		{
		    if(_MaxPrintPressure[isNo] > 0)
		    {
			    _InkSupply[isNo].purgePressure = _MaxPrintPressure[isNo] + pressure;
			    if(_InkSupply[isNo].purgePressure > 800) _InkSupply[isNo].purgePressure = 800;
		    }
		    else
		    {
			    switch(pRX_Config->printerType)
			    {
				    case printer_TX801 :
				    case printer_TX802 :
									    _InkSupply[isNo].purgePressure = 100 + 100*pRX_Config->headsPerColor + pressure; break;
				    default : 			_InkSupply[isNo].purgePressure = 40 * pRX_Config->headsPerColor + pressure; break;
			    }
		    }
    
		    if (_InkSupply[isNo].purgePressure > MAX_PRESSURE_FLUID)
			    _InkSupply[isNo].purgePressure = MAX_PRESSURE_FLUID;
		}
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
		else if ((-pRX_Status->degass_pressure) <  _LungVacc-50)
		{
			pRX_Status->vacuum_solenoid = TRUE;
			_DutyDegasserTimeClogged 	= 0;
		}
	}

	if (pRX_Status->vacuum_solenoid!=act)
	{
		if (pRX_Status->vacuum_solenoid)
			IOWR_ALTERA_AVALON_PIO_SET_BITS(PIO_OUTPUT_BASE, DEGASS_VALVE_OUT);	//Vaccum Solenoid = on
		else
			IOWR_ALTERA_AVALON_PIO_CLEAR_BITS(PIO_OUTPUT_BASE, DEGASS_VALVE_OUT);	// Vaccum Solenoid = off
	}

	if(degassing)
	{
		// Duty calculation to detect air leakage
		_DutyDegasserTimeClogged++;
		_DutyDegasserTimeLeakage++;
		if(pRX_Status->vacuum_solenoid) _DutyDegasserDuty++;
		// First time calcul to avoid 0%
		if(_DutyDegasserFirst) _DutyDegasserCalcul 	= (_DutyDegasserDuty * 100) / _DutyDegasserTimeLeakage;
		if(_DutyDegasserTimeLeakage >= 30000)	// every 5 minutes
		{
			_DutyDegasserCalcul 		= (_DutyDegasserDuty * 100) / _DutyDegasserTimeLeakage;
			_DutyDegasserTimeClogged 	= 0;
			_DutyDegasserDuty 			= 0;
			_DutyDegasserFirst			= 0;

			// if duty > 20% -> air leakage
			if(_DutyDegasserCalcul > 20) pRX_Status->ink_supply[0].error |= err_degasser_leakage;
		}

		// if air pump never ON during 10 minutes -> tube clogged
		if(_DutyDegasserTimeClogged >= 60000) pRX_Status->ink_supply[0].error |= err_degasser_clogged;
	}

	return (pRX_Status->vacuum_solenoid);
}

//--- _set_air_valve -----------------------------------------
void _set_air_valve(int isNo, int state)
{
	if (!pvalve_active(isNo) && state) state=PV_OPEN;
	if (state != pRX_Status->ink_supply[isNo].airValve)
	{
		if (pvalve_set_air(isNo, state))
		{
			UINT16 val = IORD_16DIRECT(AXI_LW_SLAVE_REGISTER_0_BASE, GPIO_REG_OUT);
			if (state) val |= AIR_CUSSION_OUT(isNo);
			else	   val &= ~AIR_CUSSION_OUT(isNo);
			IOWR_16DIRECT(AXI_LW_SLAVE_REGISTER_0_BASE, GPIO_REG_OUT, val);
		}
		pRX_Status->ink_supply[isNo].airValve = state;
	} 
}

//--- _set_bleed -----------------------------------------
void _set_bleed_valve(int isNo, int state)
{
	if (!pvalve_active(isNo) && state) state=PV_OPEN;
	if (state!=pRX_Status->ink_supply[isNo].bleedValve)
	{
		if (pvalve_set_bleed(isNo, state))
		{
			UINT16 val = IORD_16DIRECT(AXI_LW_SLAVE_REGISTER_0_BASE, GPIO_REG_OUT);
			if (state) 	val |=  BLEED_OUT(isNo);
			else		val &= ~BLEED_OUT(isNo);
			IOWR_16DIRECT(AXI_LW_SLAVE_REGISTER_0_BASE, GPIO_REG_OUT, val);
		}
		pRX_Status->ink_supply[isNo].bleedValve = state;
	}	
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
		if(pRX_Config->headsPerColor == 1)		_InkSupply[isNo].pid_Setpoint.P = PID_SETPOINT_P_PRINT_HEAD;
		else if(pRX_Config->headsPerColor == 4) _InkSupply[isNo].pid_Setpoint.P = PID_SETPOINT_P_PRINT_4_HEADS;
		else									_InkSupply[isNo].pid_Setpoint.P = PID_SETPOINT_P_PRINT_8_HEADS;

		// Regulation of Conditioners Pinlet
		if(print_mode == PUMP_CTRL_MODE_PRINT)
		{
			// MODIF 3 : PID 1 anticipate decreasing if Meniscus out of range, by 
			_InkSupply[isNo].pid_Setpoint.Setpoint = pressure_target;
			if (pRX_Config->ink_supply[isNo].condMeniscus > -120)
				 _InkSupply[isNo].pid_Setpoint.Setpoint -= (pRX_Config->ink_supply[isNo].condMeniscus + 120);
			pid_calc(pRX_Status->ink_supply[isNo].COND_Pressure_Actual, &_InkSupply[isNo].pid_Setpoint);
			pRX_Status->ink_supply[isNo].IS_Pressure_Setpoint 	= _InkSupply[isNo].pid_Setpoint.val;
		
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

		// Regulation of Conditioners Pump speed
		else if(print_mode == PUMP_CTRL_INK_RECIRCULATION)
		{
			if(pRX_Config->headsPerColor <= 4) _InkSupply[isNo].pid_Setpoint.P = PID_SETPOINT_P_PRINT_4_HEADS / PID_SETPOINT_P_CHECK_REDUCED;
			else _InkSupply[isNo].pid_Setpoint.P = PID_SETPOINT_P_PRINT_8_HEADS / PID_SETPOINT_P_CHECK_REDUCED;

			_InkSupply[isNo].pid_Setpoint.Setpoint = pressure_target;
			pid_calc(pRX_Config->ink_supply[isNo].condPumpFeedback, &_InkSupply[isNo].pid_Setpoint);
			pRX_Status->ink_supply[isNo].IS_Pressure_Setpoint 	= _InkSupply[isNo].pid_Setpoint.val;

			// reset pid if output at 0, no reason to accumulate negative value (never want pressure negative)
			if(_InkSupply[isNo].pid_Setpoint.val <= _InkSupply[isNo].pid_Setpoint.val_min)
				pid_reset(&_InkSupply[isNo].pid_Setpoint);
		}

		//-----------------------------------------------------------------------------------------
		//  PID 2 (pump) :  Pressure_IS_setpoint --> PID_pump --> Pump_speed (%)
		//-----------------------------------------------------------------------------------------

		if((print_mode == PUMP_CTRL_MODE_PRINT)||(print_mode == PUMP_CTRL_INK_RECIRCULATION))
			_InkSupply[isNo].pid_Pump.Setpoint = pRX_Status->ink_supply[isNo].IS_Pressure_Setpoint;
		else _InkSupply[isNo].pid_Pump.Setpoint = pressure_target;

		// Special function (putty) : Test bleed line, air valve not used so P divided by 5
		if(print_mode == PUMP_CTRL_MODE_NO_AIR_VALVE) _InkSupply[isNo].pid_Pump.P = 200;
		else _InkSupply[isNo].pid_Pump.P = 1000;
		
		pRX_Status->ink_supply[isNo].PIDsetpoint_Output = _InkSupply[isNo].pid_Pump.Setpoint;
		pid_calc(pRX_Status->ink_supply[isNo].IS_Pressure_Actual, &_InkSupply[isNo].pid_Pump);

		if      (_InkSupply[isNo].pid_Pump.val <=0)
		{
			_set_pump_speed(isNo, 0);
			// reset integrator if pump at 0 because the air valve manage the pressure decreasing
			pid_reset(&_InkSupply[isNo].pid_Pump);
		}
		else if (_InkSupply[isNo].pid_Pump.val <PUMP_THRESHOLD) _set_pump_speed(isNo, PUMP_THRESHOLD);
		else                                              		_set_pump_speed(isNo, _InkSupply[isNo].pid_Pump.val);

		//pRX_Status->ink_supply[isNo].PIDpump_Output		= _InkSupply[isNo].pid_Pump.val;
		_PumpBeforeOFF = _InkSupply[isNo].pid_Pump.val;

		//---------------------------------------------------------------------------------------------------
		//  Controller 3 (Air valve) :  Pressure_IS_actual > Pressure_IS_setpoint --> Air valve open 100%
		//---------------------------------------------------------------------------------------------------

		if(print_mode != PUMP_CTRL_MODE_NO_AIR_VALVE)
		{
			if(pRX_Status->ink_supply[isNo].IS_Pressure_Actual > MAX_PRESSURE)	// for safety
			{
				_set_air_valve(isNo, PV_OPEN);
			}
			else if(pRX_Status->ink_supply[isNo].IS_Pressure_Actual > _InkSupply[isNo].pid_Pump.Setpoint)
			{
				_set_air_valve(isNo, PV_OPEN);
			}
			else
			{
				if (set_valve) _set_air_valve(isNo, PV_CLOSED);		  				
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

	if (pRX_Config->test_flush)
	{
		if (pRX_Status->flush_pressure < 1200) pRX_Status->flush_pump_val = pRX_Config->test_flush;
		else
		{
			pRX_Status->flush_pump_val = 0;
			pRX_Status->ink_supply[isNo].ctrl_state = ctrl_error;
		}
	}
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
