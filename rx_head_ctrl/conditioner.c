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
static FILE			*_LogFile = NULL;
static int			_LogTimer;
static UINT32		_ErrFlags[MAX_HEADS_BOARD];

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

	#ifdef DEBUG
	if (arg_offline) 
		_cond_load(PATH_BIN_HEAD FILENAME_HEAD_COND);
	#else
		_cond_load(PATH_BIN_HEAD FILENAME_HEAD_COND);
	#endif
	
	if (arg_offline)
	{
		for (i=0; i<MAX_HEADS_BOARD; i++) cond_ctrlMode(i, ctrl_print);
	}
	
	/*
	{
		SConditionerCfg cfg;
		memset (&cfg, 0, sizeof(cfg));
		for (i=0; i<MAX_HEADS_BOARD; i++) cond_set_config(i, &cfg);	
	}
	*/
	
	return REPLY_OK;
}

//--- cond_shutdown -------------------------------
void cond_shutdown(void)
{
	int i;
	for (i=0; i<MAX_HEADS_BOARD; i++) cond_ctrlMode(i, ctrl_shutdown);		
}

//--- cond_error_reset -----------------------------------------
void cond_error_reset(void)
{
	int i;
	
	memset(_ErrFlags, 0, sizeof(_ErrFlags));
	_ErrLevel = LOG_TYPE_UNDEF;
		
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
	int no=0;

	if (!_plog_on) return;
	
	//--- log while in print mode ---------------------
	
	if (RX_HBStatus[0].head[no].ctrlMode!=ctrl_print)
	{
		if (_plog_file)
		{
			Error(WARN, 0, "Logging Conditioner done");
			fclose(_plog_file);
			_plog_file = NULL;
		}
		return;
	}
	
	if (_plog_file==NULL)
	{
		char name[100];
		sprintf(name, PATH_TEMP "pump_%d.csv", ++_plog_fileNo);
		_plog_file = fopen(name, "w");
		fprintf(_plog_file, "no\t" "PG\t" "Pos[cm]\t" "pressure-in\t" "pressure-out\t" "meniscus\t" "pump\t" "pump_ticks\t" "cylinder-pressure\t" "\n");
		fflush(_plog_file);
		_plog_time = rx_get_ticks();
		_plog_idx = 0;
		_plog_no  =-1;
		Error(WARN, 0, "Logging Conditioner[%d] to >>%s<<", no, name);			
	}

	int i=0;
	int save=FALSE;
	SCondLogItem *plog;
	
	int time0 = rx_get_ticks();
	
	while(TRUE)
	{
		plog = &_NiosStat->cond[no].log[_plog_idx];
		if(plog->no > _plog_no)
		{
			int pg = RX_HBStatus[0].head[no].printGoCnt;
			int pos = (int)(fpga_enc_position(no) / 8 / 0.254 / 1200.0);
			/* --- scanning
			if (pg!=_plog_lastPg) _LogTimer=10;
			if (pg && _LogTimer)
			{
				fprintf(_plog_file, "%03ld\t%d\t%03d\t%03ld\t%03ld\t%03ld\t%03ld\n", plog->no, pg, pos, plog->pressure_in, plog->pressure_out, plog->meniscus, plog->pump);
				fflush(_plog_file);
			}
			else if (_plog_file && !_LogTimer)//(_lastPg) 
			{
				fclose(_plog_file);
				_plog_file=NULL;
			}
			*/
			//--- test system ------------
			{
				fprintf(_plog_file, "%03ld\t%d\t%03d\t%03ld\t%03ld\t%03ld\t%03ld\t%03ld\t%03ld\n", plog->no, pg, pos, plog->pressure_in, plog->pressure_out, plog->meniscus, plog->pump, plog->pump_ticks, RX_FluidStat[no].cylinderPressure);
			}
			save = TRUE;
			_plog_lastPg = pg;
			_plog_no = plog->no;
			_plog_idx = (_plog_idx + 1) % SIZEOF(_NiosStat->cond[0].log);
		}
		else break;
	}

	int time=rx_get_ticks();
	if (time-_plog_time>1000)
	{
		fflush(_plog_file);
		_plog_time = time;
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
void cond_error_check(void)
{
	if (!nios_loaded()) return;

	int head;
	ELogItemType level=LOG_TYPE_UNDEF;
	SVersion version;
	
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
        	char *headName = RX_HBConfig.head[head].name;
			
        	if (!_NiosStat->cond[head].info.connected) {		
            	ErrorFlag(level=ERR_ABORT, &_ErrFlags[head], 1 << 0, 0, "Conditioner %s: not connected", headName); continue;
        	}
        	if (_NiosStat->cond[head].error&COND_ERR_connection_lost)	{		ErrorFlag(level=ERR_ABORT, &_ErrFlags[head], COND_ERR_connection_lost,			0, "Conditioner %s: connection lost", headName); continue;}
        	if (_NiosStat->cond[head].error&COND_ERR_status_struct_missmatch)	ErrorFlag(level=LOG,       &_ErrFlags[head], COND_ERR_status_struct_missmatch,	0, "Conditioner %s: status_struct_missmatch", headName);
        	if (_NiosStat->cond[head].error&COND_ERR_version)					ErrorFlag(level=ERR_ABORT, &_ErrFlags[head], COND_ERR_version,					0, "Conditioner %s: wrong version", headName);
        	if (_NiosStat->cond[head].error&COND_ERR_alive)						ErrorFlag(level=ERR_ABORT, &_ErrFlags[head], COND_ERR_alive,					0, "Conditioner %s: alive error %d(%d)", headName, _NiosStat->cond[head].alive, _NiosMem->cfg.cond[head].alive);
        	if (_NiosStat->cond[head].error&COND_ERR_pres_in_hw)				ErrorFlag(level=ERR_ABORT, &_ErrFlags[head], COND_ERR_pres_in_hw,				0, "Conditioner %s: Pessure IN Sensor Error", headName);
        	if (_NiosStat->cond[head].error&COND_ERR_pres_out_hw)				ErrorFlag(level=ERR_ABORT, &_ErrFlags[head], COND_ERR_pres_out_hw,				0, "Conditioner %s: Pessure OUT Sensor Error", headName);
        	if (_NiosStat->cond[head].error&COND_ERR_pump_hw)					ErrorFlag(level=ERR_ABORT, &_ErrFlags[head], COND_ERR_pump_hw,					0, "Conditioner %s: Pump Error", headName);
        	if (_NiosStat->cond[head].error&COND_ERR_temp_tank_not_changing)	ErrorFlag(level=LOG,       &_ErrFlags[head], COND_ERR_temp_tank_not_changing,	0, "Conditioner %s: temp_tank_not_changing", headName);
        	if (_NiosStat->cond[head].error&COND_ERR_temp_head_not_changing)	ErrorFlag(level=LOG,       &_ErrFlags[head], COND_ERR_temp_head_not_changing,	0, "Conditioner %s: temp_head_not_changing", headName);
        	if (_NiosStat->cond[head].error&COND_ERR_temp_head_overheat)		ErrorFlag(level=ERR_ABORT, &_ErrFlags[head], COND_ERR_temp_head_overheat,		0, "Conditioner %s: temp_head_overheat", headName);
        	if (_NiosStat->cond[head].error&COND_ERR_temp_ink_overheat)			ErrorFlag(level=ERR_ABORT, &_ErrFlags[head], COND_ERR_temp_ink_overheat,		0, "Conditioner %s: temp_ink_overheat", headName);
        	if (_NiosStat->cond[head].error&COND_ERR_temp_inlet_hw)				ErrorFlag(level=ERR_CONT,  &_ErrFlags[head], COND_ERR_temp_inlet_hw,			0, "Conditioner %s: inlet thermistor hardware", headName);
        	if (_NiosStat->cond[head].error&COND_ERR_temp_heater_hw)			ErrorFlag(level=ERR_CONT,  &_ErrFlags[head], COND_ERR_temp_heater_hw,			0, "Conditioner %s: heater thermistor hardware", headName);
        	if (_NiosStat->cond[head].error&COND_ERR_temp_head_hw)				ErrorFlag(level=WARN,	   &_ErrFlags[head], COND_ERR_temp_head_hw,				0, "Conditioner %s: head temp sensor hardware", headName);				
        	if (_NiosStat->cond[head].error&COND_ERR_temp_tank_falling)			ErrorFlag(level=ERR_CONT,  &_ErrFlags[head], COND_ERR_temp_tank_falling,		0, "Conditioner %s: temp_tank_falling", headName);
        	if (_NiosStat->cond[head].error&COND_ERR_temp_tank_too_low)		    ErrorFlag(level=ERR_CONT,  &_ErrFlags[head], COND_ERR_temp_tank_too_low,		0, "Conditioner %s: temp_tank_too_low", headName);
        	if (_NiosStat->cond[head].error&COND_ERR_p_in_too_high)				ErrorFlag(level=WARN,      &_ErrFlags[head], COND_ERR_p_in_too_high,			0, "Conditioner %s: input pressure too high", headName);
        	if (_NiosStat->cond[head].error&COND_ERR_p_out_too_high)			ErrorFlag(level=ERR_ABORT, &_ErrFlags[head], COND_ERR_p_out_too_high,			0, "Conditioner %s: output pressure too high", headName);
	    	if (_NiosStat->cond[head].error&COND_ERR_pump_no_ink)				
	    	{
		    	if (ErrorFlag(level = ERR_ABORT, &_ErrFlags[head], COND_ERR_pump_no_ink, 0, "Conditioner %s: no ink", headName)) 
		    		_set_fluid_off(head);
		    }
	    	if (_NiosStat->cond[head].error&COND_ERR_pump_watchdog)				ErrorFlag(level=ERR_ABORT, &_ErrFlags[head], COND_ERR_pump_watchdog,			0, "Conditioner %s: pump watchdog", headName);
        	if (_NiosStat->cond[head].error&COND_ERR_meniscus)					
	        { 
		        if (ErrorFlag(level=ERR_ABORT, &_ErrFlags[head], COND_ERR_meniscus,	 0, "Conditioner %s: meniscus error", headName))
					_set_fluid_off(head);
			}
//        	if (_NiosStat->cond[head].error&COND_ERR_sensor_offset)			    ErrorFlag(level=LOG,	   &_ErrFlags[head], COND_ERR_sensor_offset,			0, "Conditioner %s: sensors not calibrated", headName);
	    	if (_NiosStat->cond[head].error&COND_ERR_temp_heater_overheat)		ErrorFlag(level=WARN,      &_ErrFlags[head], COND_ERR_temp_heater_overheat,		0, "Conditioner %s: heater overheated", headName);
        	if (_NiosStat->cond[head].error&COND_ERR_download)					ErrorFlag(level=WARN,	   &_ErrFlags[head], COND_ERR_download,					0, "Conditioner %s: download", headName);				
        	if (_NiosStat->cond[head].error&COND_ERR_power_24V)					ErrorFlag(level=ERR_CONT,  &_ErrFlags[head], COND_ERR_power_24V,				0, "Conditioner %s: 24V Power Supply Overcurrent", headName);				
        	if (_NiosStat->cond[head].error&COND_ERR_power_heater)				ErrorFlag(level=WARN,      &_ErrFlags[head], COND_ERR_power_heater,				0, "Conditioner %s: heater Overcurrent", headName);				
        	if (_NiosStat->cond[head].error&COND_ERR_flush_failed)				ErrorFlag(level=ERR_ABORT, &_ErrFlags[head], COND_ERR_flush_failed,				0, "Conditioner %s: flush failed", headName);
	    	
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


//--- cond_main ---------------------------------------------
void cond_main(int ticks)
{	
	/*
    int headNo;
        
    // handle control modes which can only be changed from putty (not in the GUI) 
    for (headNo = 0; headNo < HEAD_BOARD_CNT; headNo++)
    {
        if (RX_HBStatus[0].head[headNo].ctrlMode == ctrl_offset_cal)        cond_ctrlMode(headNo, ctrl_offset_cal_1); 
        if (RX_HBStatus[0].head[headNo].ctrlMode == ctrl_offset_cal_1)      cond_ctrlMode(headNo, ctrl_offset_cal_done); 
        if (RX_HBStatus[0].head[headNo].ctrlMode == ctrl_offset_cal_done)   cond_ctrlMode(headNo, ctrl_off);
        
        if (RX_HBStatus[0].head[headNo].ctrlMode == ctrl_del_user_offset)   cond_ctrlMode(headNo, ctrl_off);
    }
    */
    
	_cond_preslog(ticks);
    
    if (!(ticks%10))
        _write_log();
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

//--- cond_set_param --------------------------------------------------------------------
void cond_set_param(int headNo, char param, int value)
{
    int i;
    
    if (headNo > MAX_HEADS_BOARD)
        return;

	for (i=0; i<MAX_HEADS_BOARD; i++)
	{
		_NiosMem->cfg.cond[i].controller_P		= RX_NiosStat.cond[i].controller_P;
		_NiosMem->cfg.cond[i].controller_I		= RX_NiosStat.cond[i].controller_I;
		_NiosMem->cfg.cond[i].controller_D		= RX_NiosStat.cond[i].controller_D;
		_NiosMem->cfg.cond[i].controller_offset = RX_NiosStat.cond[i].controller_offset;		
	}
	
	switch (param)
    {
    	case 'm':
            if (headNo < MAX_HEADS_BOARD)              {_NiosMem->cfg.cond[headNo].pressure_out = value; }
            else for (i = 0; i < MAX_HEADS_BOARD; i++) {_NiosMem->cfg.cond[i].pressure_out = value; }
            break;
        case 'P':
            if (headNo < MAX_HEADS_BOARD)		       {_NiosMem->cfg.cond[headNo].controller_P = value; _NiosMem->cfg.cond[headNo].cmd.set_pid = TRUE;}
            else for (i = 0; i < MAX_HEADS_BOARD; i++) {_NiosMem->cfg.cond[i].controller_P = value; _NiosMem->cfg.cond[i].cmd.set_pid = TRUE;}
			break;
        case 'I':
            if (headNo < MAX_HEADS_BOARD)		       {_NiosMem->cfg.cond[headNo].controller_I = value; _NiosMem->cfg.cond[headNo].cmd.set_pid = TRUE;}
            else for (i = 0; i < MAX_HEADS_BOARD; i++) {_NiosMem->cfg.cond[i].controller_I = value; _NiosMem->cfg.cond[i].cmd.set_pid = TRUE;}
            break;
        case 'D':
            if (headNo < MAX_HEADS_BOARD)		       {_NiosMem->cfg.cond[headNo].controller_D = value; _NiosMem->cfg.cond[headNo].cmd.set_pid = TRUE;}
            else for (i = 0; i < MAX_HEADS_BOARD; i++) {_NiosMem->cfg.cond[i].controller_D = value; _NiosMem->cfg.cond[i].cmd.set_pid = TRUE;}
            break;
        case 'O':
            if (headNo < MAX_HEADS_BOARD)		       {_NiosMem->cfg.cond[headNo].controller_offset = value; _NiosMem->cfg.cond[headNo].cmd.set_pid = TRUE;}
            else for (i = 0; i < MAX_HEADS_BOARD; i++) {_NiosMem->cfg.cond[i].controller_offset = value; _NiosMem->cfg.cond[i].cmd.set_pid = TRUE;}
            break;
        }
}

//--- cond_set_config ---------------------------------------
void cond_set_config(int headNo, SConditionerCfg *cfg)
{	
	if (headNo<0 || headNo>=MAX_HEADS_BOARD || _NiosMem==NULL) return;
	_NiosMem->cfg.cond[headNo].pressure_out = cfg->pressure_out;			
	_NiosMem->cfg.cond[headNo].controller_P = cfg->controller_P;			
    _NiosMem->cfg.cond[headNo].controller_I = cfg->controller_I;	
    _NiosMem->cfg.cond[headNo].controller_D = cfg->controller_D;	
	_NiosMem->cfg.cond[headNo].controller_offset = cfg->controller_offset;
	_NiosMem->cfg.cond[headNo].cmd.set_pid = TRUE;
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

//--- cond_heater_set ---------------------------------------------------
void cond_heater_set(int headNo, int temp, int tempMax)
{
	if (_NiosMem && headNo>=0 && headNo<MAX_HEADS_BOARD)
	{
		_NiosMem->cfg.cond[headNo].temp    = 1000*temp;
		_NiosMem->cfg.cond[headNo].tempMax = 1000*tempMax;		
	}
}

//--- cond_presout_set -----------------------------------------------
void cond_presout_set	(int headNo, int meniscus)
{
//	Error(LOG, 0, "cond_presout_set[%d]=%d",  headNo, meniscus);
	if (_NiosMem && headNo>=0 && headNo<MAX_HEADS_BOARD)
	{
		_NiosMem->cfg.cond[headNo].pressure_out  = meniscus;
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
    	fprintf(_LogFile, ";Cond Alive 0;Cond Temp 0;Cond Temp Ht 0;Head Temp 0;Heater 0;Pres in 0;Pres in raw 0;Pres out 0;Pres out raw 0;Pump 0;Pump meas 0;Heater pg; Heater flg;24V pg; 24V flg");
    	fprintf(_LogFile, ";Cond Alive 1;Cond Temp 1;Cond Temp Ht 1;Head Temp 1;Heater 1;Pres in 1;Pres in raw 1;Pres out 1;Pres out raw 1;Pump 1;Pump meas 1;Heater pg; Heater flg;24V pg; 24V flg");
    	fprintf(_LogFile, ";Cond Alive 2;Cond Temp 2;Cond Temp Ht 2;Head Temp 2;Heater 2;Pres in 2;Pres in raw 2;Pres out 2;Pres out raw 2;Pump 2;Pump meas 2;Heater pg; Heater flg;24V pg; 24V flg");
    	fprintf(_LogFile, ";Cond Alive 3;Cond Temp 3;Cond Temp Ht 3;Head Temp 3;Heater 3;Pres in 3;Pres in raw 3;Pres out 3;Pres out raw 3;Pump 3;Pump meas 3;Heater pg; Heater flg;24V pg; 24V flg"); 
		fprintf(_LogFile, "\n");
		fflush(_LogFile);
		printf("\n--- Start log ---\n");
	}
	else
	{
		printf("\n--- File open error! ---\n");
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
    		// todo FPGA temp in log
			fprintf(_LogFile, "%d;%d;", _NiosStat->headcon_amc_temp, 0);
			
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
			
			if (FpgaCfg.encoder->synth.value) fprintf(_LogFile, "%d;", 160000 / FpgaCfg.encoder->synth.value);
			else                              fprintf(_LogFile, "%d;", 0);				
									
			for (i = 0; i < MAX_HEADS_BOARD; i++)
			{
				fprintf(_LogFile, "%lu;", _NiosStat->cond[i].alive);
				fprintf(_LogFile, "%lu;", _NiosStat->cond[i].tempIn);
				fprintf(_LogFile, "%lu;", _NiosStat->cond[i].tempHeater);
				fprintf(_LogFile, "%lu;", _NiosStat->head_temp[i]);
				fprintf(_LogFile, "%lu;", _NiosStat->cond[i].heater_percent);
				fprintf(_LogFile, "%lu;", _NiosStat->cond[i].pressure_in);
				fprintf(_LogFile, "%lu;", _NiosStat->cond[i].pressure_out);
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
