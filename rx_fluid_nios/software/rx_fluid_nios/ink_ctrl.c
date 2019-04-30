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

#define 	PRESSURE_MICRO_PURGE		50
#define 	PRESSURE_SOFT_PURGE			80
#define 	PRESSURE_PURGE				160
#define 	PRESSURE_HARD_PURGE			320
#define 	PRESSURE_FLUSH				1000

#define 	TIME_MICRO_PURGE			1000
#define		TIME_SOFT_PURGE				2000
#define 	TIME_PURGE					3000
// #define 	TIME_HARD_PURGE				5000
#define 	TIME_HARD_PURGE				10000

#define		TIME_CALIBRATE				30000

#define 	DEGASSING_VACCUUM_UV		500
#define 	DEGASSING_VACCUUM_WB		800

#define 	INK_PUMP_VAL_MAX			511 // 614

//--- statics -----------------------------------------------
typedef struct
{
	SPID_par	pid;	// Reglerstruktur
	int			alive;
	int			connected;
	int			degassing;

	int			inkPresSetCfg;
	int			bleedTime;
} SInkSupply;

static SInkSupply _InkSupply[NIOS_INK_SUPPLY_CNT];

static int	_LungVacc = 0;
static int	_PurgePressure=0;
static int	_PurgeTime=0;
static int	_PurgeNo;
static int  _PressureTimer;
static int	_ValveOut;
static int	_FlushPump=0;
static int	_AirPressureTime=0;
static int	_PrintingTime[NIOS_INK_SUPPLY_CNT] = {0};
static int  _cylinderPres_10[NIOS_INK_SUPPLY_CNT] = {0};
static UINT32 _LastPumpTicks[NIOS_INK_SUPPLY_CNT] = {0};
static UINT32 _PumpSpeed1000[NIOS_INK_SUPPLY_CNT] = {0};

//--- calibration -------------------------
static int	  _CalTime[NIOS_INK_SUPPLY_CNT];

//--- prototypes ------------------------
static int    _degass_ctrl(void);
static void   _init_purge(int isNo, int presure, int time);
static void   _set_pump_speed(int isNo, int speed);
static UINT32 _get_pump_ticks(int isNo);
static void   _set_air_valve(int isNo, int newState);
static void   _set_bleed_valve(int isNo, int newState);

// static void _calibrate_inkpump(UINT32 isNo, UINT32 timeout);
static void _pump_ctrl(INT32 isNo, INT32 pressure_target);
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
		pid_reset(&_InkSupply[isNo].pid);

		_InkSupply[isNo].pid.norm = 100;			// Normalize values for PID to use INT32
		_InkSupply[isNo].pid.P    = 70*_InkSupply[isNo].pid.norm;
		_InkSupply[isNo].pid.I    = 0;
		_InkSupply[isNo].pid.D    = 0;
		_InkSupply[isNo].pid.offset = 0;
		_InkSupply[isNo].pid.I_rampup_to = 200;

		//_InkSupply[isNo].pid.diff_min    = 1;
		//_InkSupply[isNo].pid.sum_cnt_max = 20;
		//_InkSupply[isNo].pid.val_invalid = 0;
		_InkSupply[isNo].pid.val_max     = INK_PUMP_VAL_MAX;	// according to datasheet is [0..5V], [0..10V] is allowed according to KNF
		_InkSupply[isNo].pid.val_min	 = 0;	// Below a certain voltage the pump has not enough power to start
		//_InkSupply[isNo].pid.val_max     = 0x3ff; // 10 bit DAC
		//_InkSupply[isNo].pid.val_min	 = -_InkSupply[isNo].pid.val_max;

		_InkSupply[isNo].inkPresSetCfg = INVALID_VALUE;
		_LastPumpTicks[isNo]		   = _get_pump_ticks(isNo);
		_PumpSpeed1000[isNo]		   = 0;
		pRX_Status->ink_supply[isNo].cylinderPresSet = INVALID_VALUE; // 150;
	}
	_PurgePressure = 0;
	_PurgeTime     = 0;
	_PurgeNo	   = -1;
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
	INT32 empty_pressure = 100 * pRX_Config->headsPerColor;

	_LungVacc = DEGASSING_VACCUUM_WB;

	for(isNo = 0 ; isNo < NIOS_INK_SUPPLY_CNT ; isNo++)
	{
		pRX_Status->ink_supply[isNo].fluid_P = _InkSupply[isNo].pid.P;

		//--- lung vacuum: UV when any heater is on ---
		if (!(pRX_Status->ink_supply[isNo].error & err_heater_board)) _LungVacc = DEGASSING_VACCUUM_UV;

		//---  get new value ---------
		if (pRX_Config->ink_supply[isNo].cylinderPresSet!=_InkSupply[isNo].inkPresSetCfg)
		{
			pRX_Status->ink_supply[isNo].cylinderPresSet = pRX_Config->ink_supply[isNo].cylinderPresSet;
			_InkSupply[isNo].inkPresSetCfg               = pRX_Config->ink_supply[isNo].cylinderPresSet;
		}

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
		if (pRX_Status->ink_supply[isNo].cylinderPres != INVALID_VALUE
		&&  pRX_Status->ink_supply[isNo].cylinderPres > MAX_PRESSURE_FLUID
		)
		{
			if (pRX_Config->ink_supply[isNo].ctrl_mode != ctrl_test
			&& 	pRX_Config->ink_supply[isNo].ctrl_mode > ctrl_off
			&& (pRX_Config->ink_supply[isNo].ctrl_mode < ctrl_empty ||  pRX_Config->ink_supply[isNo].ctrl_mode > ctrl_empty+10))
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
//			case ctrl_shutdown_done:
			case ctrl_off:
				pid_reset(&_InkSupply[isNo].pid);
				_set_air_valve(isNo, TRUE);
				_set_bleed_valve(isNo, FALSE);
				if (_PurgeNo<0) _set_pressure_value(FALSE);
				if (_PurgeNo<0 || _PurgeNo==isNo)
				{
					_set_flush_pump(isNo, FALSE);
					_set_pump_speed(isNo, 0);

					_InkSupply[isNo].degassing=FALSE;
					if (isNo==_PurgeNo)
					{
						_PurgeTime=0;
						_PurgePressure=0;
						_PurgeNo = -1;
					}
				}
				pRX_Status->ink_supply[isNo].flushTime = 0;
				pRX_Status->ink_supply[isNo].ctrl_state = pRX_Config->ink_supply[isNo].ctrl_mode;
				break;

			case ctrl_test:
				if (pRX_Status->ink_supply[isNo].ctrl_state==ctrl_test)
				{
					if (pRX_Config->ink_supply[isNo].test_airValve) 	_set_air_valve(isNo, !pRX_Status->ink_supply[isNo].airValve);
					if (pRX_Config->ink_supply[isNo].test_bleedValve)	_set_bleed_valve(isNo, !pRX_Status->ink_supply[isNo].bleedValve);
					pRX_Config->ink_supply[isNo].test_airValve		= FALSE;
					pRX_Config->ink_supply[isNo].test_bleedValve 	= FALSE;

					_pump_ctrl(isNo, pRX_Config->ink_supply[isNo].test_cylinderPres);		// ink-pump
					_set_pressure_value(pRX_Status->air_pressure < pRX_Config->test_airPressure);	// air-pump
				}
				else
				{
					// --- reset values -----------
					pRX_Config->ink_supply[isNo].test_airValve  	= FALSE;
					pRX_Config->ink_supply[isNo].test_bleedValve 	= FALSE;
					pRX_Config->ink_supply[isNo].test_cylinderPres 	= 0;
				}
				pRX_Status->ink_supply[isNo].ctrl_state = ctrl_test;
				break;

			case ctrl_warmup:
				_set_air_valve(isNo, FALSE);
				_set_bleed_valve(isNo, FALSE);
				_InkSupply[isNo].degassing=pRX_Config->cmd.lung_enabled;
				_pump_ctrl(isNo, pRX_Status->ink_supply[isNo].cylinderPresSet);
				pRX_Status->ink_supply[isNo].ctrl_state = ctrl_warmup;
				break;

			case ctrl_cal_start:
				_set_bleed_valve(isNo, FALSE);
				_set_air_valve(isNo, FALSE);
				_set_flush_pump(isNo, FALSE);
				_InkSupply[isNo].degassing = pRX_Config->cmd.lung_enabled;

				_PrintingTime[isNo] = 0;
				pRX_Status->ink_supply[isNo].ctrl_state = ctrl_cal_start;
				break;

			case ctrl_cal_step1:
				// pRX_Status->ink_supply[isNo].cylinderPresSet = 100;
				_pump_ctrl(isNo, pRX_Status->ink_supply[isNo].cylinderPresSet);
				_CalTime[isNo]   = 0;
				pRX_Status->ink_supply[isNo].ctrl_state = ctrl_cal_step1;
				break;

			case ctrl_cal_step2:
				_pump_ctrl(isNo, pRX_Status->ink_supply[isNo].cylinderPresSet);
				_CalTime[isNo]+=cycleTime;
				pRX_Status->ink_supply[isNo].time = _CalTime[isNo];
				if (_CalTime[isNo] >= TIME_CALIBRATE)
				{
					int diff = -abs(pRX_Config->ink_supply[isNo].condPresOutSet) - pRX_Config->ink_supply[isNo].condPresOut;
					pRX_Status->ink_supply[isNo].diff = diff;
					if (diff>10 || diff<-10)
					{
						diff /= 5;
						if (diff<-20) diff=-20;
						if (diff>20) diff=20;
						pRX_Status->ink_supply[isNo].cylinderPresSet += diff;
					}
					else	pRX_Status->ink_supply[isNo].ctrl_state = ctrl_cal_step2;
					_CalTime[isNo]    = 0;
				}
				break;

			case ctrl_cal_step3:
			case ctrl_cal_step4:
			case ctrl_cal_done:
				_pump_ctrl(isNo, pRX_Status->ink_supply[isNo].cylinderPresSet);
				pRX_Status->ink_supply[isNo].ctrl_state = pRX_Config->ink_supply[isNo].ctrl_mode;
				break;

			case ctrl_readyToPrint:
				break;

			case ctrl_print:
				_set_bleed_valve(isNo, FALSE);
				_set_air_valve(isNo, FALSE);
				_set_flush_pump(isNo, FALSE);
				_InkSupply[isNo].degassing = pRX_Config->cmd.lung_enabled;

				// do not start ramp-up from 0 after purge
				if (pRX_Status->ink_supply[isNo].ctrl_state != pRX_Config->ink_supply[isNo].ctrl_mode
				&&  pRX_Status->ink_supply[isNo].ctrl_state != ctrl_purge_step3)
					_cylinderPres_10[isNo] = 0;

				//--- rampup ink_pressure ----------
				/*
				_cylinderPres_10[isNo]++;
				if (_cylinderPres_10[isNo] > 10*pRX_Status->ink_supply[isNo].cylinderPresSet)
					_cylinderPres_10[isNo] = 10*pRX_Status->ink_supply[isNo].cylinderPresSet;

				_pump_ctrl(isNo, _cylinderPres_10[isNo]/10);
				*/
				_pump_ctrl(isNo, pRX_Status->ink_supply[isNo].cylinderPresSet);

				pRX_Status->ink_supply[isNo].ctrl_state = pRX_Config->ink_supply[isNo].ctrl_mode;
				break;

			case ctrl_bleed:
				_set_air_valve(isNo, FALSE);
				_InkSupply[isNo].degassing=pRX_Config->cmd.lung_enabled;
				_pump_ctrl(isNo, pRX_Status->ink_supply[isNo].cylinderPresSet);
				pRX_Status->ink_supply[isNo].ctrl_state = ctrl_bleed;
				break;
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
				_pump_ctrl(isNo, PRESSURE_FLUSH);
				_set_flush_pump(isNo, FALSE);
				pRX_Status->ink_supply[isNo].flushTime = 1000 * pRX_Config->ink_supply[isNo].flushTime[pRX_Config->ink_supply[isNo].ctrl_mode-ctrl_flush_night];

				pRX_Status->ink_supply[isNo].ctrl_state = pRX_Config->ink_supply[isNo].ctrl_mode;
				break;

			case ctrl_flush_step1:
				_pump_ctrl(isNo, PRESSURE_FLUSH);
				// ink pressure must be applied on all cylinders so that no flush is pressed back
				if (pRX_Status->ink_supply[isNo].cylinderPres != INVALID_VALUE
				&&  pRX_Status->ink_supply[isNo].cylinderPres >= 500)
					pRX_Status->ink_supply[isNo].ctrl_state = ctrl_flush_step1;
				break;

			case ctrl_flush_step2:
				_InkSupply[isNo].degassing = pRX_Config->cmd.lung_enabled;
				if (pRX_Status->ink_supply[isNo].flushTime>0)
				{
					_pump_ctrl(isNo, PRESSURE_FLUSH);
					_set_flush_pump(isNo, TRUE);
					pRX_Status->ink_supply[isNo].flushTime -= cycleTime;
					pRX_Status->ink_supply[isNo].ctrl_state = ctrl_flush_step1;
				}
				else
				{
					_set_pump_speed(isNo, 0);
					_set_flush_pump(isNo, FALSE);
					_set_air_valve(isNo, FALSE);
					pRX_Status->ink_supply[isNo].ctrl_state = ctrl_flush_step2;
				}
				break;

			case ctrl_flush_done:
				pRX_Status->ink_supply[isNo].ctrl_state = ctrl_flush_done;
				break;

			case ctrl_purge_soft:	_init_purge(isNo, PRESSURE_SOFT_PURGE,  TIME_SOFT_PURGE ); break;
			case ctrl_purge:		_init_purge(isNo, PRESSURE_PURGE,     	TIME_PURGE      ); break;
			case ctrl_purge_hard:	_init_purge(isNo, PRESSURE_HARD_PURGE, 	TIME_HARD_PURGE ); break;
			case ctrl_purge_micro:	_init_purge(isNo, PRESSURE_MICRO_PURGE, TIME_MICRO_PURGE); break;

			case ctrl_purge_step1: // build up pressure
				_pump_ctrl(isNo, _PurgePressure);
				if (pRX_Status->ink_supply[isNo].cylinderPres >= (90 * _PurgePressure / 100))
					pRX_Status->ink_supply[isNo].ctrl_state = ctrl_purge_step1;
				break;

			case ctrl_purge_step2:
				if (_PurgeTime>0)
				{
					_pump_ctrl(isNo, _PurgePressure);
					_set_bleed_valve(isNo, FALSE);
					_PurgeTime-=cycleTime;
				}
				else
				{
					_PurgeTime 	   = 0;
					_PurgePressure = 0;
					_PurgeNo       = -1;
					_set_air_valve(isNo, FALSE);
					pRX_Status->ink_supply[isNo].ctrl_state = ctrl_purge_step2;
				}
				break;

			case ctrl_purge_step3:
			//	_set_pump_speed(isNo, 0);
				_pump_ctrl(isNo, pRX_Status->ink_supply[isNo].cylinderPresSet);
				if (pRX_Status->ink_supply[isNo].cylinderPres<=pRX_Status->ink_supply[isNo].cylinderPresSet)
					pRX_Status->ink_supply[isNo].ctrl_state = ctrl_purge_step3;
				break;

			case ctrl_wipe:
				_set_air_valve(isNo, TRUE);
				_set_pump_speed(isNo, 0);
				pRX_Status->ink_supply[isNo].ctrl_state = ctrl_wipe;
				break;

			// --- FILL --------------------------------------------------
			case ctrl_fill:
				_InkSupply[isNo].degassing = FALSE;
				_set_air_valve(isNo, TRUE);
				_set_bleed_valve(isNo, FALSE);

				pRX_Status->ink_supply[isNo].ctrl_state = ctrl_fill;
				break;

			case ctrl_fill_step1:
				if (pRX_Status->ink_supply[isNo].cylinderPres <= pRX_Status->ink_supply[isNo].cylinderPresSet/2)
					pRX_Status->ink_supply[isNo].ctrl_state = ctrl_fill_step1;
				break;

			case ctrl_fill_step2:
				_set_air_valve(isNo, TRUE);
				_set_bleed_valve(isNo, FALSE);
				_set_pressure_value(FALSE);
				_pump_ctrl(isNo, pRX_Status->ink_supply[isNo].cylinderPresSet);
				pRX_Status->ink_supply[isNo].inkPumpSpeed_set = 100;
				pRX_Status->ink_supply[isNo].ctrl_state = ctrl_fill_step2;
				break;

			case ctrl_fill_step3:
				_pump_ctrl(isNo, pRX_Status->ink_supply[isNo].cylinderPresSet);
			//	if (pRX_Status->ink_supply[isNo].inkPumpSpeed_set<90)
			//		pRX_Status->ink_supply[isNo].ctrl_state = ctrl_fill_step3;
				break;

			// --- EMPTY -------------------------------------------------
			case ctrl_empty:
				_set_bleed_valve(isNo, TRUE);
				_set_air_valve(isNo, FALSE);
				pRX_Status->ink_supply[isNo].ctrl_state = ctrl_empty;
				break;

			case ctrl_empty_step1:
				_PurgeNo = isNo;
				_set_bleed_valve(isNo, TRUE);
				_set_air_valve(isNo, FALSE);
				for (i=0; i<NIOS_INK_SUPPLY_CNT; i++)
				{
					_set_pump_speed(i, 0);
					_InkSupply[i].degassing = FALSE;
				}
				_pump_ctrl(isNo, 0);		// ink-pump

				pRX_Status->ink_supply[isNo].ctrl_state = ctrl_empty_step1;
				break;

			case ctrl_empty_step2:
				_set_bleed_valve(isNo, TRUE);
				_set_air_valve(isNo, FALSE);
				_set_pressure_value(pRX_Status->air_pressure < empty_pressure);
				pRX_Status->ink_supply[isNo].ctrl_state = ctrl_empty_step2;
				break;

			case ctrl_empty_step3:
				_set_pressure_value(FALSE);
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
				_set_flush_pump(isNo, FALSE);
				_set_air_valve(isNo, FALSE);
				_set_pump_speed(isNo, 0);
				_InkSupply[isNo].degassing=FALSE;
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
static void _init_purge(int isNo, int pressure, int time)
{
	int i;
	if (_PurgePressure==0)
	{
		for (i=0; i<NIOS_INK_SUPPLY_CNT; i++)
		{
			if (i==isNo)
			{
				_InkSupply[i].degassing=FALSE;
				_set_air_valve(i, FALSE);
				_set_bleed_valve(i, FALSE);
			}
			else
			{
				_set_air_valve(i, TRUE);
				_set_bleed_valve(i, TRUE);
			}
		}

		_PurgeNo	   = isNo;
		_PurgePressure = pRX_Status->ink_supply[isNo].cylinderPresSet + pressure;

		if (_PurgePressure > MAX_PRESSURE_FLUID)
			_PurgePressure = MAX_PRESSURE_FLUID;

		_PurgeTime     = time;
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
					pRX_Status->ink_supply[i].cylinderPres,
					pressure,
					pRX_Status->ink_supply[i].inkPumpSpeed_set,
					pRX_Status->ink_supply[i].bleedValve);
		}
	}
}

//--- _pump_ctrl -----------------------------------------------------
static void _pump_ctrl(INT32 isNo, INT32 pressure_target)
{
	#define AIR_ON  		1
	#define AIR_OFF			1
	#define PUMP_THRESHOLD	25
	int set_valve = (pRX_Status->ink_supply[isNo].ctrl_state!=ctrl_test)
			    && !(pRX_Status->ink_supply[isNo].ctrl_state>=ctrl_fill && pRX_Status->ink_supply[isNo].ctrl_state<ctrl_fill+10);

	if (pRX_Status->ink_supply[isNo].cylinderPres > (pressure_target + AIR_ON))
	{
		if (set_valve) _set_air_valve(isNo, TRUE);
		_set_pump_speed(isNo, 0);
	}
	else if (pRX_Status->ink_supply[isNo].cylinderPres < (pressure_target - AIR_OFF))
	{
		if (set_valve) _set_air_valve(isNo, FALSE);

		if (pRX_Config->ink_supply[isNo].fluid_P) _InkSupply[isNo].pid.P = pRX_Config->ink_supply[isNo].fluid_P;
		pid_calc(pRX_Status->ink_supply[isNo].cylinderPres, pressure_target, &_InkSupply[isNo].pid);

		if      (_InkSupply[isNo].pid.val<=0)             _set_pump_speed(isNo, 0);
		else if (_InkSupply[isNo].pid.val<PUMP_THRESHOLD) _set_pump_speed(isNo, PUMP_THRESHOLD);
		else                                              _set_pump_speed(isNo, _InkSupply[isNo].pid.val);
	}
	else
	{
		_set_pump_speed(isNo, 0);
		if (set_valve) _set_air_valve(isNo, FALSE);
	}

	_trace_pump_ctrl(pressure_target);
	//trprintf(">>>deviation %d b%d c%d pres %d [%d..%d]\n", deviation, air[isNo], air_cnt[isNo], pressure, -pressure * AIR_OFF, pressure * AIR_ON);
}


//--- _set_pump_speed --------------------------------
void _set_pump_speed(int isNo, int speed)
{
	if (speed!=INVALID_VALUE)
	{
		IOWR_16DIRECT(AXI_LW_SLAVE_REGISTER_0_BASE, DAC_REG_0 + (isNo << 2), speed);
		//if (_InkSupply[isNo].pid.val_max) pRX_Status->ink_supply[isNo].inkPumpSpeed_set  = 600 * speed / _InkSupply[isNo].pid.val_max; // 100%=600 ml
		//if (_InkSupply[isNo].pid.val_max) pRX_Status->ink_supply[isNo].inkPumpSpeed_set = speed * 118 * 98 / 10 / 1023; // linear approximation from Excel
		if (_InkSupply[isNo].pid.val_max) _PumpSpeed1000[isNo] += 100 * speed / _InkSupply[isNo].pid.val_max; // in [%]
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
