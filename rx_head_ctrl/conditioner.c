<<<<<<< HEAD
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
#endif
#ifdef linux
	#include <termios.h>
	#include <sys/stat.h> 
	#include <fcntl.h>
#endif
#include "rx_def.h"
#include "rx_error.h"
#include "rx_file.h"
#include "rx_threads.h"
#include "rx_term.h"
#include "rx_trace.h"
#include "rx_head_ctrl.h"
#include "args.h"
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

SFpgaHeadBoardCfg	FpgaCfg;
SVersion			_FileVersion;
static int			_ErrorDelay=0;
static ELogItemType	_ErrLevel = LOG_TYPE_UNDEF;

#define ERROR_DELAY	200	// ms after reset until errors are checked

//--- prototypes ------------------------------------------

static void _write_log(void);
static void _cond_preslog(int ticks);

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
			RX_HBStatus[0].head[i].ctrlMode = ctrl_print_step0;
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
		for(i = 0; i < MAX_HEADS_BOARD; i++) cond_ctrlMode(i, ctrl_print_step0);		
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
	
	/*
	{
		SConditionerCfg cfg;
		memset (&cfg, 0, sizeof(cfg));
		for (i=0; i<MAX_HEADS_BOARD; i++) cond_set_config(i, &cfg);	
	}
	*/
	
	_reload=TRUE;
			
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
		for (i = 0; i < 4; i++) fprintf(_plog_file, ";Pin %d; Meniscus %d;pump %d;P %d;I %d", i, i, i, i, i);
		fprintf(_plog_file, "\n");
		_LogTimer = rx_get_ticks();
	}

	int time = rx_get_ticks() - _LogTimer;
	
	fprintf(_plog_file, "%d", time);
	for (i = 0; i < 4; i++) fprintf(_plog_file, ";%d;%d;%d;%d;%d", _NiosStat->cond[i].pressure_in, _NiosStat->cond[i].meniscus, _NiosStat->cond[i].pump_measured, _NiosStat->cond[i].pid_P, _NiosStat->cond[i].pid_I);
	fprintf(_plog_file, "\n");
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
void cond_error_check(void)
{
	if (!nios_loaded()) return;

	int head;
	ELogItemType level=LOG_TYPE_UNDEF;
	SVersion version;
	SConditionerStat_mcu	*pstat;
	
	for (head=0; head<MAX_HEADS_BOARD; head++)
	{		
		if (_NiosStat->cond[head].cmdConfirm.reset_errors)  _NiosMem->cfg.cond[head].cmd.reset_errors = FALSE;
		if (_NiosStat->cond[head].cmdConfirm.resetPumpTime) _NiosMem->cfg.cond[head].cmd.resetPumpTime= FALSE;
		if (_NiosStat->cond[head].cmdConfirm.del_offset)	_NiosMem->cfg.cond[head].cmd.del_offset   = FALSE;
		if (_NiosStat->cond[head].cmdConfirm.set_pid)		_NiosMem->cfg.cond[head].cmd.set_pid	  = FALSE;
		
		_NiosMem->cfg.cond[head].alive =  _NiosStat->cond[head].alive;
		if (_ErrorDelay && rx_get_ticks()<_ErrorDelay) return;
		_ErrorDelay = 0;
    	if (RX_HBConfig.head[head].enabled == dev_on)
    	{
	    	int abort = (_NiosMem->cfg.cond[head].mode==ctrl_off) ? LOG_TYPE_WARN : LOG_TYPE_ERROR_ABORT;
        	char *headName = RX_HBConfig.head[head].name;
	    	pstat = &_NiosStat->cond[head];
	
	    	UINT32 * perr  = (UINT32*)&RX_HBStatus[0].head[head].err;
	    	UINT32 * pwrn = (UINT32*)&RX_HBStatus[0].head[head].warn;
			
        	if (!_NiosStat->cond[head].info.connected) {		
            	ErrorFlag(level=ERR_ABORT, perr, 1 << 0, 0, "Conditioner %s: not connected", headName); continue;
        	}
        	if (_NiosStat->cond[head].error&COND_ERR_connection_lost)	{		ErrorFlag(level=ERR_ABORT,	perr, COND_ERR_connection_lost,			0, "Conditioner %s: connection lost", headName); continue;}
        	if (_NiosStat->cond[head].error&COND_ERR_status_struct_missmatch)	ErrorFlag(level=LOG,		pwrn, COND_ERR_status_struct_missmatch,	0, "Conditioner %s: status_struct_missmatch", headName);
        	if (_NiosStat->cond[head].error&COND_ERR_version)					ErrorFlag(level=ERR_ABORT,	perr, COND_ERR_version,					0, "Conditioner %s: wrong version", headName);
        	if (_NiosStat->cond[head].error&COND_ERR_alive)						ErrorFlag(level=ERR_CONT,	perr, COND_ERR_alive,					0, "Conditioner %s: alive error %d(%d)", headName, _NiosStat->cond[head].aliveStat, _NiosStat->cond[head].aliveCfg);
        	if (_NiosStat->cond[head].error&COND_ERR_pres_in_hw)				ErrorFlag(level=ERR_ABORT,	perr, COND_ERR_pres_in_hw,				0, "Conditioner %s: Pessure IN Sensor Error", headName);
        	if (_NiosStat->cond[head].error&COND_ERR_pres_out_hw)				ErrorFlag(level=ERR(abort), perr, COND_ERR_pres_out_hw,				0, "Conditioner %s: Pessure OUT Sensor Error", headName);
        	if (_NiosStat->cond[head].error&COND_ERR_pump_hw)					ErrorFlag(level=ERR_ABORT,  perr, COND_ERR_pump_hw,					0, "Conditioner %s: Pump Error", headName);
        	if (_NiosStat->cond[head].error&COND_ERR_temp_tank_not_changing)	ErrorFlag(level=LOG,        pwrn, COND_ERR_temp_tank_not_changing,	0, "Conditioner %s: temp_tank_not_changing", headName);
        	if (_NiosStat->cond[head].error&COND_ERR_temp_head_not_changing)	ErrorFlag(level=LOG,        pwrn, COND_ERR_temp_head_not_changing,	0, "Conditioner %s: temp_head_not_changing", headName);
        	if (_NiosStat->cond[head].error&COND_ERR_temp_head_overheat)		ErrorFlag(level=ERR_ABORT,  perr, COND_ERR_temp_head_overheat,		0, "Conditioner %s: temp_head_overheat", headName);
        	if (_NiosStat->cond[head].error&COND_ERR_temp_ink_overheat)			ErrorFlag(level=ERR_ABORT,  perr, COND_ERR_temp_ink_overheat,		0, "Conditioner %s: temp_ink_overheat", headName);
        	if (_NiosStat->cond[head].error&COND_ERR_temp_inlet_hw)				ErrorFlag(level=ERR_CONT,	perr, COND_ERR_temp_inlet_hw,			0, "Conditioner %s: inlet thermistor hardware", headName);
        	if (_NiosStat->cond[head].error&COND_ERR_temp_heater_hw)			ErrorFlag(level=ERR_CONT,	perr, COND_ERR_temp_heater_hw,			0, "Conditioner %s: heater thermistor hardware", headName);
        	if (_NiosStat->cond[head].error&COND_ERR_temp_head_hw)				ErrorFlag(level=WARN,		pwrn, COND_ERR_temp_head_hw,			0, "Conditioner %s: head temp sensor hardware", headName);				
        	if (_NiosStat->cond[head].error&COND_ERR_temp_tank_falling)			ErrorFlag(level=ERR_CONT,	perr, COND_ERR_temp_tank_falling,		0, "Conditioner %s: temp_tank_falling", headName);
        	if (_NiosStat->cond[head].error&COND_ERR_temp_tank_too_low)		    ErrorFlag(level=ERR_CONT,	perr, COND_ERR_temp_tank_too_low,		0, "Conditioner %s: temp_tank_too_low", headName);
        	if (_NiosStat->cond[head].error&COND_ERR_p_in_too_high)				ErrorFlag(level=WARN,		pwrn, COND_ERR_p_in_too_high,			0, "Conditioner %s: input pressure too high", headName);
        	if (_NiosStat->cond[head].error&COND_ERR_p_out_too_high)			ErrorFlag(level=ERR_ABORT,	perr, COND_ERR_p_out_too_high,			0, "Conditioner %s: output pressure too high", headName);
        	if (_NiosStat->cond[head].error&COND_ERR_pump_no_ink)				
	        	ErrorFlag(level=WARN,      perr, COND_ERR_pump_no_ink,				0, "Conditioner %s: no ink: actVal=%d, setVal=%d, sum=%d", headName, pstat->pressure_out, pstat->pid_setval, pstat->pid_sum);
	    	/*
	    	if (_NiosStat->cond[head].error&COND_ERR_pump_no_ink)				
	    	{
		    	if (ErrorFlag(level = WARN, perr, COND_ERR_pump_no_ink, 0, "Conditioner %s: no ink", headName)) 
		    		cond_ctrlMode(head, ctrl_off);
		    }
		    */
	    	if (_NiosStat->cond[head].error&COND_ERR_pump_watchdog)				ErrorFlag(level=ERR_ABORT, perr, COND_ERR_pump_watchdog,			0, "Conditioner %s: pump watchdog", headName);
        	if (_NiosStat->cond[head].error&COND_ERR_meniscus)					
	        { 
		        cond_ctrlMode(head, ctrl_error);
//		        if (ErrorFlag(level=ERR_ABORT, perr, COND_ERR_meniscus,	 0, "Conditioner %s: meniscus error", headName))
//					_set_fluid_off(head);
			        
		        ErrorFlag(level=ERR_ABORT, perr, COND_ERR_meniscus,	 0, "Conditioner %s: meniscus error", headName);
			}
//        	if (_NiosStat->cond[head].error&COND_ERR_sensor_offset)			    ErrorFlag(level=LOG,	   perr, COND_ERR_sensor_offset,			0, "Conditioner %s: sensors not calibrated", headName);
	    	if (_NiosStat->cond[head].error&COND_ERR_temp_heater_overheat)		ErrorFlag(level=WARN,      pwrn, COND_ERR_temp_heater_overheat,		0, "Conditioner %s: heater overheated", headName);
        	if (_NiosStat->cond[head].error&COND_ERR_download)					ErrorFlag(level=WARN,	   pwrn, COND_ERR_download,					0, "Conditioner %s: download", headName);				
        	if (_NiosStat->cond[head].error&COND_ERR_power_24V)					ErrorFlag(level=ERR_CONT,  perr, COND_ERR_power_24V,				0, "Conditioner %s: 24V Power Supply Overcurrent", headName);				
        	if (_NiosStat->cond[head].error&COND_ERR_power_heater)				ErrorFlag(level=WARN,      pwrn, COND_ERR_power_heater,				0, "Conditioner %s: heater Overcurrent", headName);				
        	if (_NiosStat->cond[head].error&COND_ERR_flush_failed)				ErrorFlag(level=ERR_ABORT, perr, COND_ERR_flush_failed,				0, "Conditioner %s: flush failed", headName);
	    	
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

//--- _update_counters -------------------------------------
static void _update_counters(void)
{
	int condNo, n;
	int clusterNo=-1;
	int machineMeters=0;
	int printing=FALSE;

	for (condNo=0; condNo<MAX_HEADS_BOARD;  condNo++)
	{
		if (_NiosStat->cond[condNo].mode==ctrl_print_run) printing = TRUE;
		if (_NiosStat->cond[condNo].clusterNo)									clusterNo = _NiosStat->cond[condNo].clusterNo;
		if (_NiosStat->cond[condNo].clusterTime   > RX_HBStatus->clusterTime)   RX_HBStatus->clusterTime   = _NiosStat->cond[condNo].clusterTime;
		if (_NiosStat->cond[condNo].machineMeters > machineMeters)				machineMeters = _NiosStat->cond[condNo].machineMeters;
	}
	if (printing) RX_HBStatus->clusterTime++;

	RX_HBStatus->clusterNo     = clusterNo;
	RX_HBStatus->machineMeters = machineMeters;

	for (condNo=0; condNo<MAX_HEADS_BOARD;  condNo++)
	{
		_NiosCfg->cond[0].clusterNo			 = clusterNo;
		_NiosCfg->cond[condNo].clusterTime   = RX_HBStatus->clusterTime;
		if (RX_FluidStat[0].machineMeters!=INVALID_VALUE)// && RX_FluidStat[0].machineMeters>_NiosCfg->cond[condNo].machineMeters)
			_NiosCfg->cond[condNo].machineMeters = RX_FluidStat[0].machineMeters;
	}
}

//--- cond_main ---------------------------------------------
void cond_main(int ticks)
{	
	static int _ticks=0;
	if (ticks-_ticks>1000)
	{
		_update_counters();
		_ticks=ticks;
	}	
	_cond_preslog(ticks);
    
    if (!(ticks%10)) _write_log();
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

//--- cond_ctrlMode -----------------------------------------
void cond_ctrlMode(int headNo, EnFluidCtrlMode ctrlMode)
{
	if (headNo<0 || headNo>=MAX_HEADS_BOARD) return;

	if (arg_simu_conditioner) RX_HBStatus[0].head[headNo].ctrlMode = ctrlMode;
	else if (_NiosMem!=NULL) _NiosMem->cfg.cond[headNo].mode = ctrlMode;		
}

//--- cond_ctrlMode2 --------------------------------------------------------------------
void cond_ctrlMode2(int headNo, EnFluidCtrlMode ctrlMode)
{
	int i;
	if(headNo < MAX_HEADS_BOARD)		  cond_ctrlMode(headNo, ctrlMode);
	else for(i=0; i<MAX_HEADS_BOARD; i++) cond_ctrlMode(i, ctrlMode);
}

//--- cond_offset_del --------------------------
void cond_offset_del(int headNo)
{
	int i;
	if(headNo < MAX_HEADS_BOARD)		  _NiosMem->cfg.cond[headNo].cmd.del_offset = TRUE;
	else for(i=0; i<MAX_HEADS_BOARD; i++) _NiosMem->cfg.cond[i].cmd.del_offset      = TRUE;
}


//--- cond_set_config ---------------------------------------
void cond_set_config(int headNo, SConditionerCfg *cfg)
{	
	if (headNo<0 || headNo>=MAX_HEADS_BOARD || _NiosMem==NULL) return;	
	
//	_NiosMem->cfg.cond[headNo].meniscus_setpoint = cfg->meniscus_setpoint;		
	_NiosMem->cfg.cond[headNo].headsPerColor= cfg->headsPerColor;
	_NiosMem->cfg.cond[headNo].cmd.set_pid	= TRUE;
}

//--- cond_setInk ---------------------------------------
void cond_setInk(int headNo, SInkDefinition *pink)
{
	if (headNo<0 || headNo>=MAX_HEADS_BOARD || _NiosMem==NULL) return;	
	
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
void cond_heater_test(int temp)
{
	int i;
	for (i = 0; i < MAX_HEADS_BOARD; i++)
	{
		if (temp<=65)
		{
			_NiosMem->cfg.cond[i].temp = temp*1000;	
			_NiosMem->cfg.cond[i].mode = ctrl_print_run;
		}
	}
}

//--- toggle_cond_meniscus_check ---------------------------
void cond_toggle_meniscus_check(void)
{
	int i;
	for (i = 0; i < MAX_HEADS_BOARD; i++)
		_NiosMem->cfg.cond[i].cmd.disable_meniscus_check = !_NiosMem->cfg.cond[i].cmd.disable_meniscus_check;
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
				fprintf(_LogFile, "%c", _NiosStat->head_eeprom[i][a]);	
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
			fprintf(_LogFile, "%s; ", value_str_u(_NiosStat->u_minus_36v));
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


//--- cond_toggle_psensor_cali --------------------------------------------------------------------
void cond_toggle_psensor_cali(int headNo)
{
 //   _NiosMem->cfg.cond[headNo].config.disable_psensor_cali = !_NiosMem->cfg.cond[headNo].config.disable_psensor_cali;		
}

//--- cond_toggle_psensor_cali_user ---------------------------------------------------------------
void cond_toggle_psensor_cali_user(int headNo)
{
 //   _NiosMem->cfg.cond[headNo].config.user_calibration = !_NiosMem->cfg.cond[headNo].config.user_calibration;		
}
=======
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
#endif
#ifdef linux
	#include <termios.h>
	#include <sys/stat.h> 
	#include <fcntl.h>
#endif
#include "rx_def.h"
#include "rx_error.h"
#include "rx_file.h"
#include "rx_threads.h"
#include "rx_term.h"
#include "rx_trace.h"
#include "rx_head_ctrl.h"
#include "args.h"
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

SFpgaHeadBoardCfg	FpgaCfg;
SVersion			_FileVersion;
static int			_ErrorDelay=0;
static ELogItemType	_ErrLevel = LOG_TYPE_UNDEF;

#define ERROR_DELAY	200	// ms after reset until errors are checked

//--- prototypes ------------------------------------------

static void _write_log(void);
static void _cond_preslog(int ticks);

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
	
	/*
	{
		SConditionerCfg cfg;
		memset (&cfg, 0, sizeof(cfg));
		for (i=0; i<MAX_HEADS_BOARD; i++) cond_set_config(i, &cfg);	
	}
	*/
	
	_reload=TRUE;
			
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
		for (i = 0; i < 4; i++) fprintf(_plog_file, ";Pin %d; Meniscus %d;pump %d;P %d;I %d", i, i, i, i, i);
		fprintf(_plog_file, "\n");
		_LogTimer = rx_get_ticks();
	}

	int time = rx_get_ticks() - _LogTimer;
	
	fprintf(_plog_file, "%d", time);
	for (i = 0; i < 4; i++) fprintf(_plog_file, ";%d;%d;%d;%d;%d", _NiosStat->cond[i].pressure_in, _NiosStat->cond[i].meniscus, _NiosStat->cond[i].pump_measured, _NiosStat->cond[i].pid_P, _NiosStat->cond[i].pid_I);
	fprintf(_plog_file, "\n");
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
void cond_error_check(void)
{
	if (!nios_loaded()) return;

	int head;
	ELogItemType level=LOG_TYPE_UNDEF;
	SVersion version;
	SConditionerStat_mcu	*pstat;
	
	for (head=0; head<MAX_HEADS_BOARD; head++)
	{		
		if (_NiosStat->cond[head].cmdConfirm.reset_errors)  _NiosMem->cfg.cond[head].cmd.reset_errors = FALSE;
		if (_NiosStat->cond[head].cmdConfirm.resetPumpTime) _NiosMem->cfg.cond[head].cmd.resetPumpTime= FALSE;
		if (_NiosStat->cond[head].cmdConfirm.del_offset)	_NiosMem->cfg.cond[head].cmd.del_offset   = FALSE;
		if (_NiosStat->cond[head].cmdConfirm.set_pid)		_NiosMem->cfg.cond[head].cmd.set_pid	  = FALSE;
		
		_NiosMem->cfg.cond[head].alive =  _NiosStat->cond[head].alive;
		if (_ErrorDelay && rx_get_ticks()<_ErrorDelay) return;
		_ErrorDelay = 0;
    	if (RX_HBConfig.head[head].enabled == dev_on)
    	{
	    	int abort = (_NiosMem->cfg.cond[head].mode==ctrl_off) ? LOG_TYPE_WARN : LOG_TYPE_ERROR_ABORT;
        	char *headName = RX_HBConfig.head[head].name;
	    	pstat = &_NiosStat->cond[head];
	
	    	UINT32 * perr  = (UINT32*)&RX_HBStatus[0].head[head].err;
	    	UINT32 * pwrn = (UINT32*)&RX_HBStatus[0].head[head].warn;
			
        	if (!_NiosStat->cond[head].info.connected) {		
            	ErrorFlag(level=ERR_ABORT, perr, 1 << 0, 0, "Conditioner %s: not connected", headName); continue;
        	}
        	if (_NiosStat->cond[head].error&COND_ERR_connection_lost)	{		ErrorFlag(level=ERR_ABORT,	perr, COND_ERR_connection_lost,			0, "Conditioner %s: connection lost", headName); continue;}
        	if (_NiosStat->cond[head].error&COND_ERR_status_struct_missmatch)	ErrorFlag(level=LOG,		pwrn, COND_ERR_status_struct_missmatch,	0, "Conditioner %s: status_struct_missmatch", headName);
        	if (_NiosStat->cond[head].error&COND_ERR_version)					ErrorFlag(level=ERR_ABORT,	perr, COND_ERR_version,					0, "Conditioner %s: wrong version", headName);
        	if (_NiosStat->cond[head].error&COND_ERR_alive)						ErrorFlag(level=ERR_CONT,	perr, COND_ERR_alive,					0, "Conditioner %s: alive error %d(%d)", headName, _NiosStat->cond[head].aliveStat, _NiosStat->cond[head].aliveCfg);
        	if (_NiosStat->cond[head].error&COND_ERR_pres_in_hw)				ErrorFlag(level=ERR_ABORT,	perr, COND_ERR_pres_in_hw,				0, "Conditioner %s: Pessure IN Sensor Error", headName);
        	if (_NiosStat->cond[head].error&COND_ERR_pres_out_hw)				ErrorFlag(level=ERR(abort), perr, COND_ERR_pres_out_hw,				0, "Conditioner %s: Pessure OUT Sensor Error", headName);
        	if (_NiosStat->cond[head].error&COND_ERR_pump_hw)					ErrorFlag(level=ERR_ABORT,  perr, COND_ERR_pump_hw,					0, "Conditioner %s: Pump Error", headName);
        	if (_NiosStat->cond[head].error&COND_ERR_temp_tank_not_changing)	ErrorFlag(level=LOG,        pwrn, COND_ERR_temp_tank_not_changing,	0, "Conditioner %s: temp_tank_not_changing", headName);
        	if (_NiosStat->cond[head].error&COND_ERR_temp_head_not_changing)	ErrorFlag(level=LOG,        pwrn, COND_ERR_temp_head_not_changing,	0, "Conditioner %s: temp_head_not_changing", headName);
        	if (_NiosStat->cond[head].error&COND_ERR_temp_head_overheat)		ErrorFlag(level=ERR_ABORT,  perr, COND_ERR_temp_head_overheat,		0, "Conditioner %s: temp_head_overheat", headName);
        	if (_NiosStat->cond[head].error&COND_ERR_temp_ink_overheat)			ErrorFlag(level=ERR_ABORT,  perr, COND_ERR_temp_ink_overheat,		0, "Conditioner %s: temp_ink_overheat", headName);
        	if (_NiosStat->cond[head].error&COND_ERR_temp_inlet_hw)				ErrorFlag(level=ERR_CONT,	perr, COND_ERR_temp_inlet_hw,			0, "Conditioner %s: inlet thermistor hardware", headName);
        	if (_NiosStat->cond[head].error&COND_ERR_temp_heater_hw)			ErrorFlag(level=ERR_CONT,	perr, COND_ERR_temp_heater_hw,			0, "Conditioner %s: heater thermistor hardware", headName);
        	if (_NiosStat->cond[head].error&COND_ERR_temp_head_hw)				ErrorFlag(level=WARN,		pwrn, COND_ERR_temp_head_hw,			0, "Conditioner %s: head temp sensor hardware", headName);				
        	if (_NiosStat->cond[head].error&COND_ERR_temp_tank_falling)			ErrorFlag(level=ERR_CONT,	perr, COND_ERR_temp_tank_falling,		0, "Conditioner %s: temp_tank_falling", headName);
        	if (_NiosStat->cond[head].error&COND_ERR_temp_tank_too_low)		    ErrorFlag(level=ERR_CONT,	perr, COND_ERR_temp_tank_too_low,		0, "Conditioner %s: temp_tank_too_low", headName);
        	if (_NiosStat->cond[head].error&COND_ERR_p_in_too_high)				ErrorFlag(level=WARN,		pwrn, COND_ERR_p_in_too_high,			0, "Conditioner %s: input pressure too high", headName);
        	if (_NiosStat->cond[head].error&COND_ERR_p_out_too_high)			ErrorFlag(level=ERR_ABORT,	perr, COND_ERR_p_out_too_high,			0, "Conditioner %s: output pressure too high", headName);
        	if (_NiosStat->cond[head].error&COND_ERR_pump_no_ink)				
	        	ErrorFlag(level=WARN,      perr, COND_ERR_pump_no_ink,				0, "Conditioner %s: no ink: actVal=%d, setVal=%d, sum=%d", headName, pstat->pressure_out, pstat->pid_setval, pstat->pid_sum);
	    	/*
	    	if (_NiosStat->cond[head].error&COND_ERR_pump_no_ink)				
	    	{
		    	if (ErrorFlag(level = WARN, perr, COND_ERR_pump_no_ink, 0, "Conditioner %s: no ink", headName)) 
		    		cond_ctrlMode(head, ctrl_off);
		    }
		    */
	    	if (_NiosStat->cond[head].error&COND_ERR_pump_watchdog)				ErrorFlag(level=ERR_ABORT, perr, COND_ERR_pump_watchdog,			0, "Conditioner %s: pump watchdog", headName);
        	if (_NiosStat->cond[head].error&COND_ERR_meniscus)					
	        { 
		        cond_ctrlMode(head, ctrl_error);
//		        if (ErrorFlag(level=ERR_ABORT, perr, COND_ERR_meniscus,	 0, "Conditioner %s: meniscus error", headName))
//					_set_fluid_off(head);
			        
		        ErrorFlag(level=ERR_ABORT, perr, COND_ERR_meniscus,	 0, "Conditioner %s: meniscus error", headName);
			}
//        	if (_NiosStat->cond[head].error&COND_ERR_sensor_offset)			    ErrorFlag(level=LOG,	   perr, COND_ERR_sensor_offset,			0, "Conditioner %s: sensors not calibrated", headName);
	    	if (_NiosStat->cond[head].error&COND_ERR_temp_heater_overheat)		ErrorFlag(level=WARN,      pwrn, COND_ERR_temp_heater_overheat,		0, "Conditioner %s: heater overheated", headName);
        	if (_NiosStat->cond[head].error&COND_ERR_download)					ErrorFlag(level=WARN,	   pwrn, COND_ERR_download,					0, "Conditioner %s: download", headName);				
        	if (_NiosStat->cond[head].error&COND_ERR_power_24V)					ErrorFlag(level=ERR_CONT,  perr, COND_ERR_power_24V,				0, "Conditioner %s: 24V Power Supply Overcurrent", headName);				
        	if (_NiosStat->cond[head].error&COND_ERR_power_heater)				ErrorFlag(level=WARN,      pwrn, COND_ERR_power_heater,				0, "Conditioner %s: heater Overcurrent", headName);				
        	if (_NiosStat->cond[head].error&COND_ERR_flush_failed)				ErrorFlag(level=ERR_ABORT, perr, COND_ERR_flush_failed,				0, "Conditioner %s: flush failed", headName);
	    	
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

//--- _update_counters -------------------------------------
static void _update_counters(void)
{
	int condNo, n;
	int clusterNo=-1;
	int machineMeters=0;
	int printing=FALSE;

	for (condNo=0; condNo<MAX_HEADS_BOARD;  condNo++)
	{
		if (_NiosStat->cond[condNo].mode==ctrl_print) printing = TRUE;
		if (_NiosStat->cond[condNo].clusterNo)									clusterNo = _NiosStat->cond[condNo].clusterNo;
		if (_NiosStat->cond[condNo].clusterTime   > RX_HBStatus->clusterTime)   RX_HBStatus->clusterTime   = _NiosStat->cond[condNo].clusterTime;
		if (_NiosStat->cond[condNo].machineMeters > machineMeters)				machineMeters = _NiosStat->cond[condNo].machineMeters;
	}
	if (printing) RX_HBStatus->clusterTime++;

	RX_HBStatus->clusterNo     = clusterNo;
	RX_HBStatus->machineMeters = machineMeters;

	for (condNo=0; condNo<MAX_HEADS_BOARD;  condNo++)
	{
		_NiosCfg->cond[0].clusterNo			 = clusterNo;
		_NiosCfg->cond[condNo].clusterTime   = RX_HBStatus->clusterTime;
		if (RX_FluidStat[0].machineMeters!=INVALID_VALUE)// && RX_FluidStat[0].machineMeters>_NiosCfg->cond[condNo].machineMeters)
			_NiosCfg->cond[condNo].machineMeters = RX_FluidStat[0].machineMeters;
	}
}

//--- cond_main ---------------------------------------------
void cond_main(int ticks)
{	
	static int _ticks=0;
	if (ticks-_ticks>1000)
	{
		_update_counters();
		_ticks=ticks;
	}	
	_cond_preslog(ticks);
    
    if (!(ticks%10)) _write_log();
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

//--- cond_ctrlMode -----------------------------------------
void cond_ctrlMode(int headNo, EnFluidCtrlMode ctrlMode)
{
	if (headNo<0 || headNo>=MAX_HEADS_BOARD) return;

	if (arg_simu_conditioner) RX_HBStatus[0].head[headNo].ctrlMode = ctrlMode;
	else if (_NiosMem!=NULL) _NiosMem->cfg.cond[headNo].mode = ctrlMode;		
}

//--- cond_ctrlMode2 --------------------------------------------------------------------
void cond_ctrlMode2(int headNo, EnFluidCtrlMode ctrlMode)
{
	int i;
	if(headNo < MAX_HEADS_BOARD)		  cond_ctrlMode(headNo, ctrlMode);
	else for(i=0; i<MAX_HEADS_BOARD; i++) cond_ctrlMode(i, ctrlMode);
}

//--- cond_offset_del --------------------------
void cond_offset_del(int headNo)
{
	int i;
	if(headNo < MAX_HEADS_BOARD)		  _NiosMem->cfg.cond[headNo].cmd.del_offset = TRUE;
	else for(i=0; i<MAX_HEADS_BOARD; i++) _NiosMem->cfg.cond[i].cmd.del_offset      = TRUE;
}


//--- cond_set_config ---------------------------------------
void cond_set_config(int headNo, SConditionerCfg *cfg)
{	
	if (headNo<0 || headNo>=MAX_HEADS_BOARD || _NiosMem==NULL) return;	
	
//	_NiosMem->cfg.cond[headNo].meniscus_setpoint = cfg->meniscus_setpoint;		
	_NiosMem->cfg.cond[headNo].headsPerColor= cfg->headsPerColor;
	_NiosMem->cfg.cond[headNo].cmd.set_pid	= TRUE;
}

//--- cond_setInk ---------------------------------------
void cond_setInk(int headNo, SInkDefinition *pink)
{
	if (headNo<0 || headNo>=MAX_HEADS_BOARD || _NiosMem==NULL) return;	
	
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
void cond_heater_test(int temp)
{
	int i;
	for (i = 0; i < MAX_HEADS_BOARD; i++)
	{
		if (temp<=65)
		{
			_NiosMem->cfg.cond[i].temp = temp*1000;	
			_NiosMem->cfg.cond[i].mode = ctrl_print;
		}
	}
}

//--- toggle_cond_meniscus_check ---------------------------
void cond_toggle_meniscus_check(void)
{
	int i;
	for (i = 0; i < MAX_HEADS_BOARD; i++)
		_NiosMem->cfg.cond[i].cmd.disable_meniscus_check = !_NiosMem->cfg.cond[i].cmd.disable_meniscus_check;
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
				fprintf(_LogFile, "%c", _NiosStat->head_eeprom[i][a]);	
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
			fprintf(_LogFile, "%s; ", value_str_u(_NiosStat->u_minus_36v));
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


//--- cond_toggle_psensor_cali --------------------------------------------------------------------
void cond_toggle_psensor_cali(int headNo)
{
 //   _NiosMem->cfg.cond[headNo].config.disable_psensor_cali = !_NiosMem->cfg.cond[headNo].config.disable_psensor_cali;		
}

//--- cond_toggle_psensor_cali_user ---------------------------------------------------------------
void cond_toggle_psensor_cali_user(int headNo)
{
 //   _NiosMem->cfg.cond[headNo].config.user_calibration = !_NiosMem->cfg.cond[headNo].config.user_calibration;		
}
>>>>>>> feature-build-server
