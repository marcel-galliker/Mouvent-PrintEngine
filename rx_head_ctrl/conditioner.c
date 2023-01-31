// ****************************************************************************
//
//	DIGITAL PRINTING - conditioner.c
//
//
// ****************************************************************************
//
//	Copyright 2016 by Radex AG, Switzerland. All rights reserved.
//	Written by Stefan Weber
//
// ****************************************************************************

//--- includes ----------------------------------------------------------------
#ifdef soc
	#include <sys/mman.h>
	#include <stdio.h>
	#include <string.h>

#endif
#ifdef linux
	#include <termios.h>
	#include <sys/stat.h> 
	#include <fcntl.h>
#endif
#include "rx_def.h"
#include "rx_crc.h"
#include "rx_error.h"
#include "rx_file.h"
#include "rx_threads.h"
#include "rx_term.h"
#include "rx_trace.h"
#include "rx_head_ctrl.h"
#include "args.h"
#include "EEprom.h"
#include "fpga.h"
#include "nios.h"
#include "version.h"
#include "math.h"
#include "putty.h"
#include "conditioner.h"

//--- module globals ---------------------------------------
static FILE					*_LogFile = NULL;
static int					_LogTimer;
static SConditionerCfg_mcu	_CfgBackup[MAX_HEADS_BOARD];
static EnFluidCtrlMode		_CtrlMode[MAX_HEADS_BOARD];

SFpgaHeadBoardCfg	FpgaCfg;
SVersion			_FileVersion;
static int			_UpdateClusterTimer;
static int			_ErrorDelay=0;

static int			_Recovery_Freq = 0;

static int _JetSequenceFrequency[128] = {0};
static int _JetSequenceDropSize[128] = {0};
static int _JetSequencePressure[128] = {0};
static int _JetSequenceTime[128] = {0};
static ELogItemType _ErrLevel = LOG_TYPE_UNDEF;

#define ERROR_DELAY	200	// ms after reset until errors are checked

//--- prototypes ------------------------------------------

static void _write_log(void);
static void _cond_copy_status(void);
static void _cond_preslog(int ticks);
static void _cond_test_report(int ticks);
static void	_update_clusterNo(void);
static void _read_jet_sequence(void);

//#define LOG_FLUID

//--- cond init ---------------------------------
int cond_init(void)
{	
	int i;
	static int _reload=FALSE;
	
	cond_error_reset();
		
	memset(&_FileVersion, 0, sizeof(_FileVersion));
	if (arg_simu_conditioner)
	{
		int i;
		Error(WARN, 0, "Conditioner in Simulation");
		for (i = 0; i < SIZEOF(RX_HBStatus[0].head); i++)
		{
			RX_HBStatus[0].head[i].ctrlMode = ctrl_print;
		}
		return REPLY_OK;
	}
	
	rx_sleep(1000); // give NIOS time to update versions

	Error(LOG, 0, "cond_init reload=%d", _reload);
	
	#ifdef DEBUG
	if (arg_offline) 
		_cond_load(PATH_BIN_HEAD FILENAME_HEAD_COND);
	#else
		_cond_load(PATH_BIN_HEAD FILENAME_HEAD_COND);
	#endif
	
	if(arg_offline)
	{
		for(i = 0; i < MAX_HEADS_BOARD; i++) cond_ctrlMode(i, ctrl_print);		
	}
	else 
	{
		for (i=0; i<MAX_HEADS_BOARD; i++)
			memcpy(&_NiosMem->cfg.cond[i], &_CfgBackup[i], sizeof(_NiosMem->cfg.cond[i]));

		if (!_reload) 
		{
			for(i = 0; i < MAX_HEADS_BOARD; i++) cond_ctrlMode(i, ctrl_undef);						
		}
	}
	
	_UpdateClusterTimer = 5;
//	_update_clusterNo();

	
	/*
	{
		SConditionerCfg cfg;
		memset (&cfg, 0, sizeof(cfg));
		for (i=0; i<MAX_HEADS_BOARD; i++) cond_set_config(i, &cfg);	
	}
	*/
	
	_reload=TRUE;
    _read_jet_sequence();

    return REPLY_OK;
}

//--- cond_end --------------------------------
int cond_end(void)
{
	if(_NiosMem!=NULL) 
	{
		int headNo;
		for(headNo=0; headNo<MAX_HEADS_BOARD; headNo++)
			memcpy(&_CfgBackup[headNo],&_NiosMem->cfg.cond[headNo],sizeof(_CfgBackup[headNo]));		
	}
	else memset(_CfgBackup, 0, sizeof(_CfgBackup));
	return REPLY_OK;
}

//--- cond_shutdown -------------------------------
void cond_shutdown(void)
{
	int i;
	for (i=0; i<MAX_HEADS_BOARD; i++) cond_ctrlMode(i, ctrl_off);		
}

//--- cond_error_reset -----------------------------------------
void cond_error_reset(void)
{
	int i;
	
	for (i=0; i<SIZEOF(RX_HBStatus[0].head); i++) 
	{
		RX_HBStatus[0].head[i].err  = 0;
		RX_HBStatus[0].head[i].warn = 0;
	}
		
	_ErrorDelay = rx_get_ticks()+ERROR_DELAY;
	if (_NiosMem!=NULL)
	{
		for (i = 0; i < MAX_HEADS_BOARD; i++) _NiosMem->cfg.cond[i].cmd.reset_errors=TRUE;
	}
}

//--- cond_resetPumpTime ---------------------
void cond_resetPumpTime(int condNo)
{
	_NiosMem->cfg.cond[condNo].cmd.resetPumpTime = TRUE;
}

//--- _cond_log -----------------------------------------
static int	_plog_on=FALSE;
static FILE	*_plog_file = NULL;
static INT16 _plog_no;
static int _plog_fileNo;
static int _plog_lastPg;
static int _plog_time;
static int _plog_idx;
	
void cond_start_preslog(void)
{
	_plog_on = TRUE;
}

//--- _cond_preslog -----------------------------------------
static void _cond_preslog(int ticks)
{
    int valve_ink[4];
    int valve_flush[4];
    
	if (!_plog_on) 
	{
		if (_plog_file)
		{
			Error(WARN, 0, "Logging Conditioner done");
			fclose(_plog_file);
			_plog_file = NULL;
		}
		return;
	}
	int i;
	if (_plog_file == NULL)
	{
		char name[100];
		sprintf(name, PATH_TEMP "Conditioners_Regulation.csv");
		_plog_file = fopen(name, "w");
		fprintf(_plog_file, "pump_ticks(ms)");
		for (i = 0; i < 4; i++) 
		{
			fprintf(_plog_file, ";Pin %d; Meniscus %d; Pin 2 %d; Pout %d; pump %d", i, i, i, i, i);
	        fprintf(_plog_file, ";TempHEAD %d; tempIn %d; tempSTP %d; Heater %d", i, i, i, i);
            fprintf(_plog_file, ";Fluid Pressure %d; Fluid Pressure Set %d", i, i);
	        fprintf(_plog_file, ";Valve Ink %d; Valve Flush %d", i, i);
		}
		fprintf(_plog_file, "\n");
		_LogTimer = rx_get_ticks();
	}
    
    for (i = 0; i < 4; i++)
    {
        valve_ink[i] = RX_NiosStat.cond[i].info.valve_ink ? 1 : 0;
        valve_flush[i] = RX_NiosStat.cond[i].info.valve_flush ? 1 : 0;
    }

	int time = rx_get_ticks() - _LogTimer;
	fprintf(_plog_file, "%d", time);
	for (i = 0; i < 4; i++) fprintf(_plog_file, ";%d;%d;%d;%d;%d;%d;%d;%d;%d;%d;%d;%d;%d", _NiosStat->cond[i].pressure_in, _NiosStat->cond[i].meniscus, _NiosStat->cond[i].pressure_in2, _NiosStat->cond[i].pressure_out, _NiosStat->cond[i].pump_measured, _NiosMem->cfg.cond[i].tempHead / 100, _NiosStat->cond[i].tempIn / 100, _NiosMem->cfg.cond[i].temp / 100, _NiosStat->cond[i].heater_percent, RX_FluidStat[i].cylinderPressure, RX_FluidStat[i].cylinderPressureSet, valve_ink[i], valve_flush[i]);
	fprintf(_plog_file, "\n");
}

//--- _cond_test_report --------------------------------------------------
static void _cond_test_report(int ticks)
{
    int const interval=100;

	static int	_ticks=0;
    static int	_cnt[MAX_HEADS_BOARD];
	static SConditionerStat_mcu	_cond[MAX_HEADS_BOARD];

	int head;
	if (ticks-_ticks>interval)
	{
		_ticks = ticks;
		for (head=0; head<MAX_HEADS_BOARD; head++)
		{
			if (_NiosStat->cond[head].mode == ctrl_test)
			{
				if (_NiosStat->cond[head].tempIn==INVALID_VALUE 
				||  _NiosStat->cond[head].tempHeater==INVALID_VALUE 
				||  _NiosStat->cond[head].pressure_in==INVALID_VALUE 
				||  _NiosStat->cond[head].pressure_out==INVALID_VALUE 
				||  _NiosStat->cond[head].meniscus==INVALID_VALUE 
				)
				{
					_cnt[head]=0;
				}
				else
				{
					_cond[head].tempIn		 += _NiosStat->cond[head].tempIn;
					_cond[head].tempHeater	 += _NiosStat->cond[head].tempHeater;
					_cond[head].pressure_in	 += _NiosStat->cond[head].pressure_in;
					_cond[head].pressure_out += _NiosStat->cond[head].pressure_out;
					_cond[head].meniscus	 += _NiosStat->cond[head].meniscus;
					_cnt[head]++;
				}

				if (_cnt[head]>60000/interval)
				{
					int cnt=_cnt[head];
					_cnt[head]=0;
					//--- write file -----------------------------
					{
						FILE *file;
						char name[100];
						sprintf(name, PATH_TEMP "Conditioner-Test-%d.csv", _NiosStat->cond[head].serialNo);
						file = fopen(name, "w");
						fprintf(file, "SerialNo:\t%d\n",		_NiosStat->cond[head].serialNo);
						fprintf(file, "Printed Liters:\t%s\n",	value_str3(RX_HBStatus->head[head].printed_ml));
						fprintf(file, "Printed Time:\t%s\n",	value_str_time(RX_HBStatus->head[head].eeprom_mvt.printingSec));
						fprintf(file, "Temp Head:\t%s\n",		value_str_time(RX_NiosStat.head_temp_org[head]/cnt));
                        fprintf(file, "Temp Inlet:\t%s\n",		value_str_temp(_cond[head].tempIn/cnt));
						fprintf(file, "Temp Heater:\t%s\n",		value_str_temp(_cond[head].tempHeater/cnt));
						fprintf(file, "Presure In:\t%s\n",		value_str1(_cond[head].pressure_in/cnt));
						fprintf(file, "Presure InDiff:\t%s\n",	value_str1(RX_HBStatus->head[head].presIn_diff));
						fprintf(file, "Presure Out:\t%s\n",		value_str1(_cond[head].pressure_out/cnt));
						fprintf(file, "Presure OutDiff:\t%s\n",	value_str1(RX_HBStatus->head[head].presOut_diff));
						fprintf(file, "Meniscus:\t%s\n",		value_str1(_cond[head].meniscus));
						fprintf(file, "Meniscus Diff:\t%s\n",	value_str1(RX_HBStatus->head[head].meniscus_diff));
						fprintf(file, "Flow Resistance:\t%s\n",	value_str(_NiosMem->cfg.cond[head].flowResistance));
						fprintf(file, "Flowfactor:\t%s\n",		value_str(RX_HBStatus->head[head].flowFactor));
						fclose(file);
					}
				}
			}
			else _cnt[head]=0;

			if (_cnt[head]==0)
			{
				memset(&_cond[head], 0, sizeof(_cond[head]));
			}
		}
	}
}

//--- _set_fluid_off -------------------------------------------
static void _set_fluid_off(int head)
{
	SFluidCtrlCmd msg;
	msg.hdr.msgLen = sizeof(msg);
	msg.hdr.msgId  = CMD_FLUID_CTRL_MODE;
	msg.no		   = RX_HBConfig.head[head].inkSupply;
	msg.ctrlMode   = ctrl_off;			    
	sok_send(&RX_MainSocket, &msg);			
}

//--- cond_error_check ----------------------------------
void cond_error_check(int ticks)
{
	if (!nios_loaded()) return;

	int head;
	ELogItemType level=LOG_TYPE_UNDEF;
	SVersion version;
	SConditionerStat_mcu	*pstat;
	ELogItemType			abort = RX_HBConfig.simuPlc ? LOG_TYPE_WARN : LOG_TYPE_ERROR_ABORT;
	ELogItemType			cont  = RX_HBConfig.simuPlc ? LOG_TYPE_WARN : LOG_TYPE_ERROR_CONT;
	
	for (head=0; head<MAX_HEADS_BOARD; head++)
	{		
		if (_NiosStat->cond[head].cmdConfirm.reset_errors)  _NiosMem->cfg.cond[head].cmd.reset_errors = FALSE;
		if (_NiosStat->cond[head].cmdConfirm.resetPumpTime) _NiosMem->cfg.cond[head].cmd.resetPumpTime= FALSE;
		if (_NiosStat->cond[head].cmdConfirm.set_pid)		_NiosMem->cfg.cond[head].cmd.set_pid	  = FALSE;
		if (_NiosStat->cond[head].cmdConfirm.save_eeprom)   _NiosMem->cfg.cond[head].cmd.save_eeprom  = FALSE;
		
		_NiosMem->cfg.cond[head].alive =  _NiosStat->cond[head].alive;
		if (_ErrorDelay && rx_get_ticks()<_ErrorDelay) return;
		_ErrorDelay = 0;
    	if (RX_HBConfig.head[head].enabled == dev_on)
    	{
//	    	int abort = (_NiosMem->cfg.cond[head].mode==ctrl_off) ? LOG_TYPE_WARN : LOG_TYPE_ERROR_ABORT;
        	char *headName = RX_HBConfig.head[head].name;
	    	pstat = &_NiosStat->cond[head];
	    	UINT32 * perr  = (UINT32*)&RX_HBStatus[0].head[head].err;
	    	UINT32 * pwrn = (UINT32*)&RX_HBStatus[0].head[head].warn;
			
        	if (!_NiosStat->cond[head].info.connected) {
	        	ErrorFlag(level=WARN, perr, 1 << 0, 0, "Conditioner %s: not connected", headName); continue;
            	ErrorFlag(level=ERR(abort), perr, 1 << 0, 0, "Conditioner %s: not connected", headName); continue;
        	}
        	if (_NiosStat->cond[head].error&COND_ERR_connection_lost)	{		ErrorFlag(level=ERR(abort),	perr, COND_ERR_connection_lost,			0, "Conditioner %s: connection lost", headName); continue;}
        	if (_NiosStat->cond[head].error&COND_ERR_status_struct_missmatch)	ErrorFlag(level=LOG,		pwrn, COND_ERR_status_struct_missmatch,	0, "Conditioner %s: status_struct_missmatch", headName);
        	if (_NiosStat->cond[head].error&COND_ERR_version)					ErrorFlag(level=ERR(abort),	perr, COND_ERR_version,					0, "Conditioner %s: wrong version", headName);
        	if (_NiosStat->cond[head].error&COND_ERR_alive)						ErrorFlag(level=ERR(cont),	perr, COND_ERR_alive,					0, "Conditioner %s: alive error %d(%d)", headName, _NiosStat->cond[head].aliveStat, _NiosStat->cond[head].aliveCfg);
        	if (_NiosStat->cond[head].error&COND_ERR_pres_in_hw)				ErrorFlag(level=ERR(abort),	perr, COND_ERR_pres_in_hw,				0, "Conditioner %s: Pessure IN Sensor Error", headName);
	       	if (_NiosStat->cond[head].error&COND_ERR_pres_out_hw)				ErrorFlag(level=ERR(abort), perr, COND_ERR_pres_out_hw,				0, "Conditioner %s: Pessure OUT Sensor Error", headName);
        	if (_NiosStat->cond[head].error&COND_ERR_pump_hw)					ErrorFlag(level=ERR(abort), perr, COND_ERR_pump_hw,					0, "Conditioner %s: Pump Error", headName);
        	if (_NiosStat->cond[head].error&COND_ERR_temp_tank_not_changing)	ErrorFlag(level=LOG,        pwrn, COND_ERR_temp_tank_not_changing,	0, "Conditioner %s: temp_tank_not_changing", headName);
        	if (_NiosStat->cond[head].error&COND_ERR_temp_head_not_changing)	ErrorFlag(level=LOG,        pwrn, COND_ERR_temp_head_not_changing,	0, "Conditioner %s: temp_head_not_changing", headName);
 //       	if (_NiosStat->cond[head].error&COND_ERR_temp_head_overheat)		ErrorFlag(level=ERR(abort), perr, COND_ERR_temp_head_overheat,		0, "Conditioner %s: temp_head_overheat", headName);
        	if (_NiosStat->cond[head].error&COND_ERR_temp_head_overheat)		ErrorFlag(level=ERR(cont),  perr, COND_ERR_temp_head_overheat,		0, "Conditioner %s: temp_head_overheat", headName);
        	if (_NiosStat->cond[head].error&COND_ERR_temp_ink_overheat)			ErrorFlag(level=ERR(abort), perr, COND_ERR_temp_ink_overheat,		0, "Conditioner %s: temp_ink_overheat", headName);
        	if (_NiosStat->cond[head].error&COND_ERR_temp_inlet_hw)				ErrorFlag(level=ERR(cont),	perr, COND_ERR_temp_inlet_hw,			0, "Conditioner %s: inlet thermistor hardware", headName);
        	if (_NiosStat->cond[head].error&COND_ERR_temp_heater_hw)			ErrorFlag(level=ERR(cont),	perr, COND_ERR_temp_heater_hw,			0, "Conditioner %s: heater thermistor hardware", headName);
            if (RX_HBConfig.printerType != printer_test_CTC)
            {
	            if (_NiosStat->cond[head].error&COND_ERR_temp_head_hw)				ErrorFlag(level=WARN,		pwrn, COND_ERR_temp_head_hw,			0, "Conditioner %s: head temp sensor hardware", headName);				            
            }
        	if (_NiosStat->cond[head].error&COND_ERR_temp_tank_falling)			ErrorFlag(level=ERR(cont),	perr, COND_ERR_temp_tank_falling,		0, "Conditioner %s: temp_tank_falling", headName);
        	if (_NiosStat->cond[head].error&COND_ERR_temp_tank_too_low)		    ErrorFlag(level=ERR(cont),	perr, COND_ERR_temp_tank_too_low,		0, "Conditioner %s: temp_tank_too_low", headName);
        	if (_NiosStat->cond[head].error&COND_ERR_p_in_too_high)				ErrorFlag(level=WARN,		pwrn, COND_ERR_p_in_too_high,			0, "Conditioner %s: input pressure too high", headName);
        	if (_NiosStat->cond[head].error&COND_ERR_p_out_too_high)			ErrorFlag(level=ERR(abort),	perr, COND_ERR_p_out_too_high,			0, "Conditioner %s: output pressure too high", headName);
        	if (_NiosStat->cond[head].error&COND_ERR_pump_no_ink)				ErrorFlag(level = ERR(abort),  perr, COND_ERR_pump_no_ink,				0, "Conditioner %s: no ink: actVal=%d, sum=%d", headName, pstat->pressure_out, pstat->pid_sum);
			if (_NiosStat->cond[head].error&COND_ERR_valve)						ErrorFlag(level = ERR(abort), perr, COND_ERR_valve, 0, "Conditioner %s: valve not switching to INK", headName);
	    	if (_NiosStat->cond[head].error&COND_ERR_return_pipe)				ErrorFlag(level = ERR(abort), perr, COND_ERR_return_pipe, 0, "Conditioner %s: Return pipe clogged or disconnected", headName);
	    	
	    	/*
	    	if (_NiosStat->cond[head].error&COND_ERR_pump_no_ink)				
	    	{
		    	if (ErrorFlag(level = WARN, perr, COND_ERR_pump_no_ink, 0, "Conditioner %s: no ink", headName)) 
		    		cond_ctrlMode(head, ctrl_off);
		    }
		    */
	    	if (_NiosStat->cond[head].error&COND_ERR_pump_watchdog)				ErrorFlag(level=ERR(abort), perr, COND_ERR_pump_watchdog,			0, "Conditioner %s: pump watchdog", headName);
        	if (_NiosStat->cond[head].error&COND_ERR_meniscus)					
	        { 
		        cond_ctrlMode(head, ctrl_error);
//		        if (ErrorFlag(level=ERR_ABORT, perr, COND_ERR_meniscus,	 0, "Conditioner %s: meniscus error", headName))
//					_set_fluid_off(head);
			        
		        ErrorFlag(level=ERR(abort), perr, COND_ERR_meniscus,	 0, "Conditioner %s: meniscus error", headName);
			}
//        	if (_NiosStat->cond[head].error&COND_ERR_sensor_offset)			    ErrorFlag(level=LOG,	   perr, COND_ERR_sensor_offset,			0, "Conditioner %s: sensors not calibrated", headName);
	    	if (_NiosStat->cond[head].error&COND_ERR_temp_heater_overheat)		ErrorFlag(level=WARN,      pwrn, COND_ERR_temp_heater_overheat,		0, "Conditioner %s: heater overheated", headName);
        	if (_NiosStat->cond[head].error&COND_ERR_download)					ErrorFlag(level=WARN,	   pwrn, COND_ERR_download,					0, "Conditioner %s: download", headName);				
        	if (_NiosStat->cond[head].error&COND_ERR_power_24V)					ErrorFlag(level=ERR(cont), perr, COND_ERR_power_24V,				0, "Conditioner %s: 24V Power Supply Overcurrent", headName);				
        	if (_NiosStat->cond[head].error&COND_ERR_power_heater)				ErrorFlag(level=WARN,      pwrn, COND_ERR_power_heater,				0, "Conditioner %s: heater Overcurrent", headName);				
        	if (_NiosStat->cond[head].error&COND_ERR_flush_failed)				ErrorFlag(level=ERR(abort),perr, COND_ERR_flush_failed,				0, "Conditioner %s: flush failed", headName);
	    	
	    	if (level>_ErrLevel)
	    		_ErrLevel = level;
    	}
	}
}

//--- cond_err_level -------------------------------------
ELogItemType cond_err_level(void)
{
	return _ErrLevel;	
}

//--- _count -------------
static struct {int no; int cnt;} _cntr[8];
static void _count(INT32 no)
{
	int i;
//	printf("no=%d\n", no);
	if (no<=0) return;
	for (i=0; i<SIZEOF(_cntr); i++)
	{
		if (_cntr[i].no==no)
		{
			_cntr[i].cnt++;
			return;
		}
		if (_cntr[i].no==0)
		{
			_cntr[i].no=no;
			_cntr[i].cnt=1;
			return;
		}
	}
}

//--- _update_clusterNo ------------------------------------
static void _update_clusterNo(void)
{
	int condNo, n;
	int clusterNo;
	SHeadEEpromMvt *mvt;
	if (sizeof(SHeadEEpromMvt)!=EEPROM_DATA_SIZE) Error(ERR_CONT, 0, "Head User EEPROM size mismatch (size=0x%x, expected=0x%x)", sizeof(SHeadEEpromMvt), EEPROM_DATA_SIZE);
	
	memset(_cntr, 0, sizeof(_cntr));
	for (condNo=0; condNo<MAX_HEADS_BOARD;  condNo++)
	{
		mvt = &RX_HBStatus[0].head[condNo].eeprom_mvt;

		_count(mvt->clusterNo);
				
		if (mvt->dropletsPrintedCRC!=rx_crc8(&mvt->dropletsPrinted, sizeof(mvt->dropletsPrinted)))
		{
			mvt->dropletsPrinted = 0;
			mvt->dropletsPrintedCRC = rx_crc8(&mvt->dropletsPrinted, sizeof(mvt->dropletsPrinted));
		}
		RX_HBStatus[0].head[condNo].dropVolume = 0.0000000000024;
	}

	int cnt=0, idx=0;
	for (n=0; n<SIZEOF(_cntr); n++)
	{
		if(_cntr[n].cnt>cnt)
		{
			idx=n;
			cnt=_cntr[n].cnt;
		}
	}
	cond_set_clusterNo(_cntr[idx].no);
}

//--- _update_counters -------------------------------------
static void _update_counters(void)
{
	int condNo;
	int machineMeters=0;
	int printing=FALSE;
				
	for (condNo=0; condNo<MAX_HEADS_BOARD;  condNo++)
	{
		if (_NiosStat->cond[condNo].mode==ctrl_print || (_NiosStat->cond[condNo].mode >= ctrl_recovery_start && _NiosStat->cond[condNo].mode <= ctrl_recovery_step4)) printing = TRUE;
		if (_NiosStat->cond[condNo].clusterTime   > RX_HBStatus->clusterTime)   RX_HBStatus->clusterTime   = _NiosStat->cond[condNo].clusterTime;
	}
	if (printing) RX_HBStatus->clusterTime++;

//	TrPrintfL(TRUE, "_update_counters");
	for (condNo=0; condNo<MAX_HEADS_BOARD;  condNo++)
	{
		_NiosCfg->cond[condNo].clusterTime   = RX_HBStatus->clusterTime;
	}
//	TrPrintfL(TRUE, "_update_counters ok");
}

//--- cond_main ---------------------------------------------
void cond_main(int ticks, int menu)
{	
	static int _ticks=0;
	if (menu) _cond_copy_status();

	if (ticks-_ticks>1000)
	{
		if (_UpdateClusterTimer>0 && --_UpdateClusterTimer<=0)
		{
			_update_clusterNo();			
		}
		_update_counters();
		_ticks=ticks;
	}	
	_cond_preslog(ticks);
	_cond_test_report(ticks);
    
    if (!(ticks%10)) _write_log();
}

//--- _cond_copy_status -------------------------------------------------------------
static void _cond_copy_status(void)
{
	int i;
	if (nios_loaded())
	{
		for (i=0; i<SIZEOF(RX_HBStatus->head); i++)
		{
			// only valid data if conditioner is connected and not in a test mode
			if (RX_NiosStat.cond[i].info.connected)
			{
				RX_HBStatus->head[i].info				= RX_NiosStat.cond[i].info;
				RX_HBStatus->head[i].tempHead			= RX_NiosStat.head_temp[i];
				RX_HBStatus->head[i].tempCond			= RX_NiosStat.cond[i].tempIn;
                RX_HBStatus->head[i].tempHeater			= RX_NiosStat.cond[i].tempHeater;
//				RX_HBStatus->head[i].tempSetpoint		= _NiosMem->cfg.cond[i].temp; //RX_NiosStat.cond[i].tempSetpoint;
				RX_HBStatus->head[i].presIn				= RX_NiosStat.cond[i].pressure_in;
				RX_HBStatus->head[i].presIn2			= RX_NiosStat.cond[i].pressure_in2;
				RX_HBStatus->head[i].presIn_max			= RX_NiosStat.cond[i].pressure_in_max;
				RX_HBStatus->head[i].presIn_diff	    = RX_NiosStat.cond[i].pressure_in_diff;
				RX_HBStatus->head[i].presOut			= RX_NiosStat.cond[i].pressure_out;
				RX_HBStatus->head[i].presOut_diff		= RX_NiosStat.cond[i].pressure_out_diff;
				RX_HBStatus->head[i].meniscus			= RX_NiosStat.cond[i].meniscus;
				RX_HBStatus->head[i].meniscus_diff		= RX_NiosStat.cond[i].meniscus_diff;
				RX_HBStatus->head[i].meniscus_Setpoint  = RX_NiosStat.cond[i].meniscus_setpoint;
				RX_HBStatus->head[i].pumpSpeed			= RX_NiosStat.cond[i].pump;
				RX_HBStatus->head[i].pumpFeedback		= RX_NiosStat.cond[i].pump_measured * 60/100;	// in 0.1 ml
				RX_HBStatus->head[i].condPrintingSec	= RX_NiosStat.cond[i].pumptime;
				RX_HBStatus->head[i].flowFactor			= RX_NiosStat.cond[i].flowFactor;
				RX_HBStatus->head[i].ctrlMode			= RX_NiosStat.cond[i].mode;
				if (rx_crc8(&RX_HBStatus->head[i].eeprom_mvt.dropletsPrinted, sizeof(RX_HBStatus->head[i].eeprom_mvt.dropletsPrinted)) == RX_HBStatus->head[i].eeprom_mvt.dropletsPrintedCRC)
				{
					RX_HBStatus->head[i].printed_ml	= (UINT32)(RX_HBStatus->head[i].eeprom_mvt.dropletsPrinted*1000000.0*1000.0*RX_HBStatus->head[i].dropVolume);
				}
				else RX_HBStatus->head[i].printed_ml = INVALID_VALUE;

				if (RX_NiosStat.cond[i].error & COND_ERR_status_struct_missmatch)
					printf("Error status_struct_missmatch\n");						

				/*
				if (RX_HBStatus->head[i].presIn < -20)
				{		
					TrPrintfL(TRUE, "Sensor Error Conditioner=%d: Version=%lu.%lu.%lu.%lu", i, RX_NiosStat.cond[i].version.major, RX_NiosStat.cond[i].version.minor, RX_NiosStat.cond[i].version.revision, RX_NiosStat.cond[i].version.build);					
				}
				*/
			}
			else				
			{
				RX_HBStatus->head[i].info.flags		= 0;
				RX_HBStatus->head[i].tempHead		= INVALID_VALUE;
				RX_HBStatus->head[i].tempCond		= INVALID_VALUE;
				RX_HBStatus->head[i].tempSetpoint	= INVALID_VALUE;
				RX_HBStatus->head[i].presIn			= INVALID_VALUE;
				RX_HBStatus->head[i].presOut		= INVALID_VALUE;
				RX_HBStatus->head[i].pumpSpeed		= INVALID_VALUE;
				RX_HBStatus->head[i].pumpFeedback	= INVALID_VALUE;
				RX_HBStatus->head[i].condPrintingSec= INVALID_VALUE;
				RX_HBStatus->head[i].printed_ml		= INVALID_VALUE;
				RX_HBStatus->head[i].ctrlMode		= INVALID_VALUE;	
				RX_HBStatus->head[i].presIn_0out    = INVALID_VALUE;
				RX_NiosStat.cond[i].pcb_rev		    = '-';	
			}
		}
	}
}

//---_cond_load --------------------------------
int _cond_load(const char *exepath)
{
	FILE		*file;
	int			tio, ret, i, ok;
	int			time;
	int			condNo;
	char		str[64];
	char		test[512];	
	
	if (!nios_loaded()) return REPLY_ERROR;

	term_printf("load_conditioner >>%s<<\n", exepath);

//	cond_display_status(FALSE, FALSE);
	
	//--- read from bin file ----------------
	file = fopen(exepath, "rb");
	if (!file)
	{
		Error(ERR_CONT, 0, "read file error >>%s<<", exepath);
		term_printf("read file error >>%s<<\n", exepath); 
		term_flush();
		return REPLY_ERROR;
	}
	fseek(file, 0, SEEK_END);
	long fsize = ftell(file);

	fseek(file, fsize-sizeof(str), SEEK_SET);
	fread(str, 1, sizeof(str), file);

	//---  get file version ---------------
	for (i=0; i<sizeof(str); i++)
	{
		if (sscanf(&str[i], "Version=%lu.%lu.%lu.%lu", &_FileVersion.major, &_FileVersion.minor, &_FileVersion.revision, &_FileVersion.build)==4)
			break;			
	}

	term_printf("File Version: %d.%d.%d.%d\n",  _FileVersion.major, _FileVersion.minor, _FileVersion.revision, _FileVersion.build);
	for (ok=TRUE, condNo=0; condNo<MAX_HEADS_BOARD;  condNo++)
	{
		term_printf("Cond Version: %d.%d.%d.%d\n",  
			_NiosStat->cond[condNo].version.major, 
			_NiosStat->cond[condNo].version.minor, 
			_NiosStat->cond[condNo].version.revision, 
			_NiosStat->cond[condNo].version.build);		
		if (_NiosStat->cond[condNo].version.major		!= _FileVersion.major)		ok=FALSE;
		if (_NiosStat->cond[condNo].version.minor		!= _FileVersion.minor)		ok=FALSE;
		if (_NiosStat->cond[condNo].version.revision	!= _FileVersion.revision)	ok=FALSE;
		if (_NiosStat->cond[condNo].version.build		!= _FileVersion.build)		ok=FALSE;
	}
	term_flush();
//	if (ok&!arg_offline)		// flash same version in offline mode possible
	if (ok)
	{
		term_printf("All conditioners up to date.\n");
		term_flush();
		return REPLY_OK;
	}
	
	//--- set the memory ----------------
	_NiosCfg->exe.size		= (UINT16)fsize;
	_NiosCfg->exe.blkAddr	= fsize-EXE_BLOCK_SIZE;
	fseek(file, _NiosCfg->exe.blkAddr, SEEK_SET);
	fread(_NiosCfg->exe.blkData, 1, EXE_BLOCK_SIZE, file);
	memcpy(test, _NiosCfg->exe.blkData, 512);
	//--- start programming and wait until finished -------------
	_NiosCfg->cmd.exe_valid = TRUE;
	ret = REPLY_OK;
	term_printf("\n--- Bootloader State ---------------------------------------------\n");
	term_flush();
	time=0;
	for (tio = 20000; TRUE; )
	{	
		if (rx_get_ticks()>time)
		{
			time = rx_get_ticks()+500;
			for (i = 0; i < MAX_HEADS_BOARD; i++)
			{
				if (_NiosStat->condDnlNo&(1<<i)) str[i] = '*';
				else							 str[i] = '-';
			}
			str[MAX_HEADS_BOARD] = 0;
			term_printf("Conditioner[%s]: %s, alive=%d", 
				str,
				DownloadStateStr(_NiosStat->condDnlState),
				_NiosStat->alive, _NiosStat->condDnlBlkAddr);
			if(_NiosStat->condDnlState == dnl_write_binary) term_printf(" addr 0x%04x\n",_NiosStat->condDnlBlkAddr);
			else term_printf("\n");			
			term_flush();					
		}
		
		if (_NiosStat->condDnlState == dnl_done) break; 		// downlaod done
		
		if (_NiosStat->condDnlState == dnl_write_binary && _NiosStat->condDnlBlkAddr != _NiosCfg->exe.blkAddr)
		{
			memset(_NiosCfg->exe.blkData, 0x00, sizeof(_NiosCfg->exe.blkData));				
			fseek(file, _NiosStat->condDnlBlkAddr, SEEK_SET);
			fread(_NiosCfg->exe.blkData, 1, EXE_BLOCK_SIZE, file);
			_NiosCfg->exe.blkAddr = _NiosStat->condDnlBlkAddr;
		}
				
		rx_sleep(10);
		tio-=10;
		if (tio<0)
		{
			Error(ERR_CONT, 0, "Downloading Conditioner: Timeout");
			break;				
		}
	}
	fclose(file);
	term_flush();
	rx_sleep(1000);
	TrPrintf(TRUE, "Download END");
	return ret;
}

#define RECOVERY_HIGH_FREQ 11000
#define RECOVERY_LOW_FREQ 400
//--- cond_ctrlMode -----------------------------------------
void cond_ctrlMode(int headNo, EnFluidCtrlMode ctrlMode)
{
    if (headNo<0 || headNo>=MAX_HEADS_BOARD) return;

    SHeadEEpromMvt *mem=&_NiosStat->eeprom_mvt[headNo];
	if (mem->flowResistanceCRC==rx_crc8(&mem->flowResistance, sizeof(mem->flowResistance)))
		_NiosMem->cfg.cond[headNo].flowResistance = mem->flowResistance;
	else	
		_NiosMem->cfg.cond[headNo].flowResistance = 0;

    
	if (arg_simu_conditioner) RX_HBStatus[0].head[headNo].ctrlMode = ctrlMode;
	else if (_NiosMem!=NULL) _NiosMem->cfg.cond[headNo].mode = ctrlMode;		

	_CtrlMode[headNo] = ctrlMode;
    
    if (_CtrlMode[headNo] == ctrl_recovery_step3 || _CtrlMode[headNo] == ctrl_recovery_step4)		do_jetting(_Recovery_Freq);
    else if (_CtrlMode[headNo] == ctrl_recovery_step5 || _CtrlMode[headNo] == ctrl_off)				fpga_enc_config(0);
}

//--- cond_ctrlMode2 --------------------------------------------------------------------
void cond_ctrlMode2(int headNo, EnFluidCtrlMode ctrlMode)
{
	int i;
	if(headNo < MAX_HEADS_BOARD)		  cond_ctrlMode(headNo, ctrlMode);
	else for(i=0; i<MAX_HEADS_BOARD; i++) cond_ctrlMode(i, ctrlMode);
}

//--- cond_getCtrlMode --------------------------------------------
EnFluidCtrlMode cond_getCtrlMode(int headNo)
{
	return _CtrlMode[headNo];		
}

//--- cond_set_config ---------------------------------------
void cond_set_config(int headNo, SConditionerCfg *cfg)
{	
	if (headNo<0 || headNo>=MAX_HEADS_BOARD || _NiosMem==NULL) return;	
	
//	_NiosMem->cfg.cond[headNo].meniscus_setpoint = cfg->meniscus_setpoint;		
	_NiosMem->cfg.cond[headNo].headsPerColor= cfg->headsPerColor;
	_NiosMem->cfg.cond[headNo].cmd.set_pid	= TRUE;
}

//--- cond_set_valve_test --------------------------------------
void cond_set_valve_test(int headNo, int valve)
{
	if (headNo<0 || headNo>=MAX_HEADS_BOARD || _NiosMem==NULL) return;	
	_NiosMem->cfg.cond[headNo].test_Valve= valve;
	cond_ctrlMode2(headNo, ctrl_test_valve);
}

//--- cond_set_valve_test_str --------------------------------------
void cond_set_valve_test_str(int headNo, char *str)
{
	if (headNo < 0 || headNo >= MAX_HEADS_BOARD || _NiosMem == NULL) return;
	for (char *ch=str; TRUE; ch++)
	{
		switch (*ch)
		{
		case '\0': return;
		case 'o': _NiosMem->cfg.cond[headNo].test_Valve= 0;
				  cond_ctrlMode2(headNo, ctrl_test_valve);
				  return;

		case 'f': _NiosMem->cfg.cond[headNo].test_Valve= 1;
				  cond_ctrlMode2(headNo, ctrl_test_valve);
				  return;

		case 'i': _NiosMem->cfg.cond[headNo].test_Valve= 2;
				  cond_ctrlMode2(headNo, ctrl_test_valve);
				  return;
		default: break;
		}
	}
}

//--- cond_set_flowResistance --------------------------------
void cond_set_flowResistance(int headNo, int value)
{
	if (headNo<0 || headNo>=MAX_HEADS_BOARD || _NiosMem==NULL) return;	

	_NiosMem->cfg.cond[headNo].flowResistance = value;
	eeprom_set_flowResistance(headNo, value);
}

//--- cond_set_purge_par -----------------------------------------
void cond_set_purge_par(int headNo, int delay_pos_y, int time, int act_pos_y, int delay_time)
{
	if (headNo<0 || headNo>=MAX_HEADS_BOARD || _NiosMem==NULL) return;	
    _NiosMem->cfg.cond[headNo].purgeDelayPos_y = delay_pos_y;
	_NiosMem->cfg.cond[headNo].purgeTime  = time;
    _NiosMem->cfg.cond[headNo].purgeDelayTime = delay_time;
}

//--- cond_set_clusterNo --------------------------------
void cond_set_clusterNo(INT32 clusterNo)
{
	int headNo;
	for (headNo=0; headNo<MAX_HEADS_BOARD; headNo++)
	{
		if (_NiosStat->cond[headNo].clusterNo!=clusterNo)
		{
			_NiosCfg->cond[headNo].clusterNo = clusterNo;				
			_NiosCfg->cond[headNo].cmd.save_eeprom = TRUE;				
		}
		RX_HBStatus[0].head[headNo].eeprom_mvt.clusterNo = clusterNo;
	}
	eeprom_set_clusterNo(clusterNo);
	RX_HBStatus->clusterNo = clusterNo;
}

//--- cond_set_serialNo --------------------------------
void cond_set_serialNo(int headNo, int serialNo)
{
	if (headNo>=0 && headNo<MAX_HEADS_BOARD)
	{
		_NiosCfg->cond[headNo].serialNo = serialNo;
		_NiosCfg->cond[headNo].cmd.save_eeprom = TRUE;			
	}
}

//--- cond_setInk ---------------------------------------
void cond_setInk(int headNo, SInkDefinition *pink)
{
	if (headNo<0 || headNo>=MAX_HEADS_BOARD || _NiosMem==NULL) return;
	RX_HBStatus->head[headNo].tempSetpoint		 = pink->temp    *1000;
	_NiosMem->cfg.cond[headNo].temp				 = pink->temp    *1000;
	_NiosMem->cfg.cond[headNo].tempMax			 = pink->tempMax *1000;		
	_NiosMem->cfg.cond[headNo].meniscus_setpoint = pink->meniscus;
}

//--- cond_volume_printed -----------------------------------
void cond_volume_printed(int headNo, int volume)
{
	if (headNo<0 || headNo>=MAX_HEADS_BOARD || _NiosMem==NULL) return;
	_NiosMem->cfg.cond[headNo].volume_printed = volume;
}

//--- _err -------------------------
static char _err(int err)
{
	if (err) return '*';
	return'.';
}

//--- cond_heater_test ------------------------------------------------------
void cond_heater_test(int time)
{
	for (int i = 0; i < MAX_HEADS_BOARD; i++)
	{
		_NiosMem->cfg.cond[i].mode	= ctrl_test_heater;
        _NiosMem->cfg.cond[i].test_time = time;
    }
}

//--- toggle_cond_meniscus_check ---------------------------
void cond_toggle_meniscus_check(void)
{
    for (int i = 0; i < MAX_HEADS_BOARD; i++)
    {
        _NiosMem->cfg.cond[i].cmd.disable_meniscus_check = !_NiosMem->cfg.cond[i].cmd.disable_meniscus_check;
        RX_HBStatus[i].info.meniscus = _NiosMem->cfg.cond[i].cmd.disable_meniscus_check;
    }
}

//--- cond_set_meniscus_chk ---------------------------
void cond_set_meniscus_chk(int enable)
{
    for (int i = 0; i < MAX_HEADS_BOARD; i++)
    {
        _NiosMem->cfg.cond[i].cmd.disable_meniscus_check = !enable;
        RX_HBStatus[i].info.meniscus = _NiosMem->cfg.cond[i].cmd.disable_meniscus_check;
    }
}

//--- cond_start_log --------------------------------------------------------------
void cond_start_log(void)
{
	char str[512];
	int i,a;
	
	_LogFile  = fopen(PATH_BIN_HEAD "test_log.csv", "w");
	if (_LogFile != NULL)
	{
		fprintf(_LogFile, "Nios Version: %lu.%lu.%lu.%lu\n", _NiosStat->version.major, _NiosStat->version.minor, _NiosStat->version.revision, _NiosStat->version.build);
		fprintf(_LogFile, "FPGA Version: %lu.%lu.%lu.%lu\n", RX_HBStatus->fpgaVersion.major, RX_HBStatus->fpgaVersion.minor, RX_HBStatus->fpgaVersion.revision, RX_HBStatus->fpgaVersion.build);
		fprintf(_LogFile, "rx_head_ctrl Version: %s (%s, %s)\n", version, __DATE__, __TIME__);
		/*
		fprintf(_LogFile, "Cond Nr. / Version: "); 
		for (i = 0; i < MAX_HEADS_BOARD; i++)
		{
			fprintf(_LogFile, "%i / %d.%d.%d.%d\t", i, _NiosStat->cond[i].version.major, _NiosStat->cond[i].version.minor, _NiosStat->cond[i].version.revision, _NiosStat->cond[i].version.build); 
		}
		*/

		for (i = 0; i < MAX_HEADS_BOARD; i++)
		{
			fprintf(_LogFile, "EEPROM Head %d:;", i);
			for (a = 0; a < EEPROM_DATA_SIZE; a++)
			{
				fprintf(_LogFile, "%c", _NiosStat->eeprom_fuji[i][a]);	
			}
			fprintf(_LogFile, "\n");
		}
		fflush(_LogFile);
		
		fprintf(_LogFile, "\nNios alive;Temp amc;Temp FPGA");
				
#ifdef LOG_FLUID
		for (i = 0; i < MAX_HEADS_BOARD; i++)
			fprintf(_LogFile, ";Fluid Pres %d;Fluid Pump %d;Fluid Feedback %d;Fluid Temp %d", i, i, i, i);
#endif // LOG_FLUID
			
		fprintf(_LogFile, ";+2.5V;-5V;+5V;-36V;Cooler Temp;Cooler Pressure;Enc. [kHz]");
    	fprintf(_LogFile, ";Cond Alive 0;Cond Temp 0;Cond Temp Ht 0;Head Temp 0;Heater 0;Pres in 0;Pres out 0;Pump 0;Pump meas 0;Heater pg; Heater flg;24V pg; 24V flg");
    	fprintf(_LogFile, ";Cond Alive 1;Cond Temp 1;Cond Temp Ht 1;Head Temp 1;Heater 1;Pres in 1;Pres out 1;Pump 1;Pump meas 1;Heater pg; Heater flg;24V pg; 24V flg");
    	fprintf(_LogFile, ";Cond Alive 2;Cond Temp 2;Cond Temp Ht 2;Head Temp 2;Heater 2;Pres in 2;Pres out 2;Pump 2;Pump meas 2;Heater pg; Heater flg;24V pg; 24V flg");
    	fprintf(_LogFile, ";Cond Alive 3;Cond Temp 3;Cond Temp Ht 3;Head Temp 3;Heater 3;Pres in 3;Pres out 3;Pump 3;Pump meas 3;Heater pg; Heater flg;24V pg; 24V flg"); 
		fprintf(_LogFile, "\n");
		fflush(_LogFile);
		TrPrintfL(TRUE, "\n--- Start log ---\n");
	}
	else
	{
		TrPrintfL(TRUE, "\n--- File open error! ---\n");
	}
	
	rx_sleep(1500);
}

//--- _write_log ----------------------------------
static void _write_log(void)
{
	int i;
	if (nios_loaded())
	{
		if (_LogFile != NULL)
		{
			fprintf(_LogFile, "%lu;", _NiosStat->alive);
			fprintf(_LogFile, "%d;%d;", _NiosStat->headcon_amc_temp, (RX_FpgaStat.temp - 128)*1000);
			
#ifdef LOG_FLUID
			// Fluid sensors
			for (i = 0; i < MAX_HEADS_BOARD; i++)
			{
				fprintf(_LogFile, "%ld;", RX_FluidStat[i].cylinderPressure);
				fprintf(_LogFile, "%ld;", RX_FluidStat[i].inkPump);
				fprintf(_LogFile, "%ld;", RX_FluidStat[i].inkPumpFeedback);
				fprintf(_LogFile, "%ld;", RX_FluidStat[i].amcTemp);    			
			}
#endif // LOG_FLUID
			
			fprintf(_LogFile, "%s; ", value_str_u(_NiosStat->u_plus_2v5));
			fprintf(_LogFile, "%s; ", value_str_u(_NiosStat->u_minus_5v));
			fprintf(_LogFile, "%s; ", value_str_u(_NiosStat->u_plus_5v));
			fprintf(_LogFile, "%s; ", value_str_u(_NiosStat->u_firepulse));
			fprintf(_LogFile, "%lu;", _NiosStat->cooler_temp);
			fprintf(_LogFile, "%ld;", _NiosStat->cooler_pressure);
			
			if (FpgaCfg.encoder->synth.value) fprintf(_LogFile, "%d;", 140000 / FpgaCfg.encoder->synth.value);
			else                              fprintf(_LogFile, "%d;", 0);				
									
			for (i = 0; i < MAX_HEADS_BOARD; i++)
			{
				fprintf(_LogFile, "%lu;", _NiosStat->cond[i].alive);
				fprintf(_LogFile, "%lu;", _NiosStat->cond[i].tempIn);
				fprintf(_LogFile, "%lu;", _NiosStat->cond[i].tempHeater);
				fprintf(_LogFile, "%lu;", _NiosStat->head_temp[i]);
				fprintf(_LogFile, "%lu;", _NiosStat->cond[i].heater_percent);
				fprintf(_LogFile, "%d.%d;", _NiosStat->cond[i].pressure_in/10, abs(_NiosStat->cond[i].pressure_in)%10);
				fprintf(_LogFile, "%d.%d;", _NiosStat->cond[i].pressure_out/10, abs(_NiosStat->cond[i].pressure_out)%10);
				fprintf(_LogFile, "%lu;", _NiosStat->cond[i].pump);
				fprintf(_LogFile, "%lu;", _NiosStat->cond[i].pump_measured);
				fprintf(_LogFile, "%d;%d;", _NiosStat->cond[i].gpio_state.heater_pg, _NiosStat->cond[i].gpio_state.heater_flg);
				fprintf(_LogFile, "%d;%d;", _NiosStat->cond[i].gpio_state.u_24v_pg, _NiosStat->cond[i].gpio_state.u_24v_flg);
			}
				
			//end
			fprintf(_LogFile, "\n");
			fflush(_LogFile);
		}
	}
}

static void _read_jet_sequence(void)
{
    char *path;
    char line[50];
    char frequency[10];
    char dropSize[10];
    char pressure[10];
    char time[10];
    int i = 0;
    int j = 0;
    int test;
    char *tmp;

    path = PATH_BIN_HEAD "Jet_Sequence.csv";
    FILE *csv = fopen(path, "r");

    if (csv == NULL)
        return;
    else
    {
        while (fgets(line, 1024, csv))
        {
            tmp = strtok(line, ";");
            if (i == 0) i++;		// jump over the first line, because these are just headlines
            else
            {
                strcpy(frequency, tmp);
                strcpy(dropSize, strtok(NULL, ";"));
                strcpy(pressure, strtok(NULL, ";"));
                strcpy(time, strtok(NULL, ";"));

				_JetSequenceFrequency[i-1] = atoi(frequency);
                _JetSequenceDropSize[i-1] = atoi(dropSize);
                _JetSequencePressure[i-1] = atoi(pressure);
                _JetSequenceTime[i-1] = atoi(time);
                
                i++;
            }
        }
        fclose(csv);
        return;
    }
}

//--- set_recovery_freq ---------------------------------------
void set_recovery_freq(int freq_hz)
{
    _Recovery_Freq = freq_hz;
}
