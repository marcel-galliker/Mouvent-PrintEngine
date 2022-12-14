// ****************************************************************************
//
//	DIGITAL PRINTING - nios.c
//
//
// ****************************************************************************
//
//	Copyright 2013 by Radex AG, Switzerland. All rights reserved.
//	Written by Marcel Galliker
//
// ****************************************************************************

//--- includes ----------------------------------------------------------------
#ifdef soc
	#include <sys/mman.h>
	#include <stdio.h>
#endif
#include "rx_common.h"
#include "rx_ink_common.h"
#include "rx_def.h"
#include "rx_error.h"
#include "rx_file.h"
#include "rx_term.h"
#include "rx_threads.c"
#include "rx_trace.h"
#include "hex2bin.h"
#include "args.h"
#include "fpga_fluid.h"
#include "nios_def_fluid.h"
#include "rx_fluid_ctrl.h"
#include "nios_fluid.h"

//--- defines ------------------------------------------------------

#define	NIOS_EXE_ADDR	0xc0000000
// #define	NIOS_EXE_ADDR	0xc0020000
#define NIOS_EXE_SIZE	0x00018000

#define NIOS_MEM_ADDR	0xc0060000
#define NIOS_MEM_SIZE	0x00010000	

#define NIOS_SYSID_ADDR	0xFF200000

#define NIOS_RESET_ADDR	0xFF220000	// 0xFF220010
#define NIOS_RESET_SIZE	0x00001000

//--- structures --------------------------------------------

typedef struct
{
	EnFluidCtrlMode old_mode;
	int time;
} SSimuData;


//--- module globals ---------------------------------------
static int _Init=FALSE;
static int _Loaded=FALSE;
static int _Simulation=FALSE;

SNiosFluidCfg	*_Cfg;

static SNiosFluidStat	*_Stat;
static UINT32			*_NiosSysId;
static UINT32			*_NiosReset;
static UINT32			_NiosResetDummy;
static SNiosFluidMemory	*_NiosMem;
static UINT32			_Error[NIOS_INK_SUPPLY_CNT+1];
	
static FILE				*_TempLogFile = NULL;

static FILE				*_LogFile=NULL;
static int				 _LogCnt=0;
static int				 _NiosAlive;
static int				 _NiosAliveTime;
static int				 _HeaterUsed;
static int				 _MaxTemp;

static SSimuData _SimuData[NIOS_INK_SUPPLY_CNT];

//--- prototypes ---------------------------------------------------
static void _update_status(void);
static void _display_status(void);
static void _simu_fluidsystem(void);
static int  _set_testmode(void);
static void _nios_check_errors(void);

static void _IS_cond_log(int ticks);


//--- nios_init ---------------------------------
int nios_init(void)
{
	if (!fpga_is_init()) return REPLY_ERROR;
	#ifdef soc
	{
		BYTE *mem;
		int fd = open("/dev/mem", O_RDWR);
		_NiosMem   = (SNiosFluidMemory*)mmap(NULL, NIOS_MEM_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, fd, NIOS_MEM_ADDR);
		mem        = (BYTE*)mmap(NULL, NIOS_MEM_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, fd, NIOS_RESET_ADDR);
		_NiosReset = (UINT32*)&mem[0x10];
		close(fd);

		nios_load(PATH_BIN_FLUID FIELNAME_FLUID_NIOS);
		if (!_Loaded) _Simulation =  TRUE;
	}
	#else
		_Simulation = TRUE;
	#endif
	/*
	if (_Simulation)
	{
		_NiosReset = &_NiosResetDummy;
		_NiosMem = (SNiosFluidMemory*)rx_malloc(sizeof(SNiosFluidMemory));
		_NiosMem->stat.info.is_shutdown = TRUE;
		_Loaded = TRUE;
	}
	*/
	
	_Cfg  = &_NiosMem->cfg;
	_Stat = &_NiosMem->stat;

	_NiosAlive		= _Stat->alive;
	_NiosAliveTime	= 0;
	memset(_SimuData, 0, sizeof(_SimuData));
	memset(_Error, 0, sizeof(_Error));
	_Init = _Loaded;

//	TrPrintfL(TRUE, "Nios Version = %d.%d.%d.%d", _Stat->version.major, _Stat->version.minor, _Stat->version.revision, _Stat->version.build);
	return REPLY_OK;
}

//--- nios_end -------------------------------------
int nios_end(void)
{
	_Init = FALSE;
	#ifdef soc
		munmap(_NiosMem, NIOS_MEM_SIZE); 
	#endif
	return REPLY_OK;
}

//--- nios_main ------------------------
void  nios_main(int ticks, int menu)
{
	if (menu) 
	{
		_nios_check_errors();
		_display_status();
		if (_Init) _Cfg->fluid_ctrl_alive++;
	}

	//---  log --------------------------------------------
	_IS_cond_log(ticks);
}

//--- _nios_check_errors -------------------------------------
static void _nios_check_errors(void)
{
	if (!_Init) 
	{
		ErrorFlag (ERR_ABORT, &RX_FluidBoardStatus.err,  err_nios_not_loaded,  0, "NIOS NOT LOADED");			
		return;
	}
	
	if(_Stat->alive != _NiosAlive)
	{
		_NiosAlive = _Stat->alive;
		_NiosAliveTime = 0;
	} 
	else if (_NiosAliveTime++>10)			 ErrorFlag(ERR_CONT,  &RX_FluidBoardStatus.err, err_nios_not_running,    0, "NIOS not running, alive=%d", _Stat->alive);
	
	if (_Stat->error&err_fpga_incompatible)  ErrorFlag(ERR_ABORT, &RX_FluidBoardStatus.err, err_fpga_incompatible,   0, "Fluid NIOS NOT compatible");
	if (_Stat->error&err_amc_fluid)			 ErrorFlag(ERR_ABORT, &RX_FluidBoardStatus.err, err_amc_fluid,           0, "AMC Fluid Temperature Error");
	if (_Stat->error&err_watchdog)			 ErrorFlag(ERR_ABORT, &RX_FluidBoardStatus.err, err_watchdog,            0, "Watchdog");
//	if (_Stat->error&err_inkpres_not_reached)ErrorFlag(ERR_CONT,  &RX_FluidBoardStatus.err, err_inkpres_not_reached, 0, "Ink Tank pressure not reached");
    if (_HeaterUsed && _Stat->error&err_amc_heater) ErrorFlag(ERR_CONT, &RX_FluidBoardStatus.err, err_amc_heater, 0, "No Heater Board connected to Fluid");		

	int isNo;
	for (isNo=0; isNo<SIZEOF(_Stat->ink_supply); isNo++)
	{
		if ((_Stat->ink_supply[isNo].ctrl_state==ctrl_print || (_Stat->ink_supply[isNo].ctrl_state >= ctrl_recovery_start && _Stat->ink_supply[isNo].ctrl_state <= ctrl_recovery_step4)) && _Stat->ink_supply[isNo].IS_Pressure_Actual==INVALID_VALUE)			 
			ErrorFlag(ERR_ABORT, (UINT32*)&_Error[isNo], err_ink_tank_pressure, 0, "InkSupply[%d-%s] Ink Tank pressure sensor not working", isNo+1, RX_ColorNameShort(isNo));
		if (_Stat->ink_supply[isNo].error&err_overpressure)		 
			ErrorFlag(ERR_CONT, (UINT32*)&_Error[isNo], err_overpressure, 0, "InkSupply[%d-%s] Ink Tank overpressure", isNo+1, RX_ColorNameShort(isNo));

		if (_Stat->ink_supply[isNo].error&err_filter_clogged)
		{														
			if(_Stat->ink_supply[isNo].COND_Pressure_Actual > 100) 
				ErrorFlag(WARN,  (UINT32 *)&_Error[isNo], err_filter_clogged,      0, "InkSupply[%d-%s] Filter need to be changed soon (P max AND flow > 10ml/min)", isNo+1, RX_ColorNameShort(isNo));
			else ErrorFlag(ERR_CONT, (UINT32 *)&_Error[isNo], err_filter_clogged,      0, "InkSupply[%d-%s] Filter need to be changed (P max AND flow < 10ml/min)", isNo+1, RX_ColorNameShort(isNo));
		}
			
		//  if ((isNo == 0) &&(_Cfg->cmd.lung_needed))
        {
        //    if (_Stat->ink_supply[0].error&err_degasser_clogged)		 
		//		ErrorFlag(WARN, (UINT32*)&_Error[0], err_ink_tube_suck_air, 0, "Degasser air tube probably clogged (Degas pressure never decreases, Duty=0)");
		//	if (_Stat->ink_supply[0].error&err_degasser_leakage)		 
		//		ErrorFlag(WARN, (UINT32*)&_Error[0], err_degasser_leakage, 0, "Degasser Duty>20% (=%d) : Air leakage in degasser OR a lot of air in the ink %d degasserOn=%d",_Stat->duty_degasser, isNo, _Cfg->cmd.lung_needed);
        }
        
        //		if (_Stat->ink_supply[isNo].error & err_ink_tank_pressure)
        //			ErrorFlag (ERR_CONT, (UINT32*)&_Error[isNo],
        //err_ink_tank_pressure,  0, "InkSupply[%d] Ink Tank Sensor Error",
        //isNo+1);
        if (nios_is_heater_connected())
        {
            if (_Stat->ink_supply[isNo].error & err_heater_temp_frozen)
                ErrorFlag(ERR_CONT, &RX_FluidBoardStatus.err,err_heater_temp_frozen, 0,"Heater Box temperatures frozen : auto-reset done");

	        if (_Stat->ink_supply[isNo].error & err_amc_config_lost)
		        ErrorFlag(ERR_CONT, &RX_FluidBoardStatus.err,err_amc_config_lost,0,"Heater Box ADC config lost. Registers Power=%d, GPIO config=%d, Enable=%d, Gain=%d", _Stat->AMC_Register_Power, _Stat->AMC_Register_GPIO_Config, _Stat->AMC_Register_Enable, _Stat->AMC_Register_Gain);

            if (RX_InkSupplyCfg[isNo].ink.fileName[0] && _Stat->ink_supply[isNo].error & err_heater_board)	
				ErrorFlag(ERR_CONT, (UINT32 *)&_Error[isNo], err_heater_board,0, "InkSupply[%d-%s] Heater Board Error or Openload", isNo + 1, RX_ColorNameShort(isNo));
            if (_Stat->ink_supply[isNo].error & err_watchdog)		ErrorFlag(ERR_CONT, (UINT32 *)&_Error[isNo], err_heater_board, 0, "InkSupply[%d-%s] Heater Board Watchdog Error", isNo + 1, RX_ColorNameShort(isNo));

            if (_Stat->HeaterBoard_Vsupply_3V < 2500)	 ErrorFlag(ERR_CONT, (UINT32*)&_Error[0], err_heater_board, 0, "Heater Board %d.%dV (3.3V)", _Stat->HeaterBoard_Vsupply_3V/1000, _Stat->HeaterBoard_Vsupply_3V%1000);
			if (_Stat->HeaterBoard_Vsupply_5V < 4000)	 ErrorFlag(ERR_CONT, (UINT32*)&_Error[1], err_heater_board, 0, "Heater Board %d.%dV (5.0V)", _Stat->HeaterBoard_Vsupply_5V/1000, _Stat->HeaterBoard_Vsupply_5V%1000);
			if (_Stat->HeaterBoard_Vsupply_24V < 12000)	 ErrorFlag(ERR_CONT, (UINT32*)&_Error[2], err_heater_board, 0, "Heater Board %d.%dV (24V)", _Stat->HeaterBoard_Vsupply_24V/1000, _Stat->HeaterBoard_Vsupply_24V%1000);
			if (_Stat->HeaterBoard_Vsupply_24VP < 12000) ErrorFlag(ERR_CONT, (UINT32*)&_Error[3], err_heater_board, 0, "Heater Board %d.%dV (24V)P", _Stat->HeaterBoard_Vsupply_24VP/1000, _Stat->HeaterBoard_Vsupply_24VP%1000);
		}
		
		if (_Stat->ink_supply[isNo].ctrl_state>=ctrl_check_step0 && _Stat->ink_supply[isNo].ctrl_state<=ctrl_check_step9)
		{
			// Check 0 error
			if (_Stat->ink_supply[isNo].error&err_ink_pump)		 
				ErrorFlag(ERR_CONT, (UINT32 *)&_Error[isNo], err_ink_pump, 0, "Fluid[%d-%s]: Check0 Ink pump not running (defected OR not connected)", isNo+1, RX_ColorNameShort(isNo));
			if (_Stat->ink_supply[isNo].error&err_ink_tube_clogged)		 
				ErrorFlag(ERR_CONT, (UINT32 *)&_Error[isNo], err_ink_tube_clogged, 0, "Fluid[%d-%s]: Check0 INK tube clogged", isNo+1, RX_ColorNameShort(isNo));
			if (_Stat->ink_supply[isNo].error&err_bleed_tube_clogged)		 
				ErrorFlag(ERR_CONT, (UINT32 *)&_Error[isNo], err_bleed_tube_clogged, 0, "Fluid[%d-%s]: Check0 BLEED tube clogged OR AIR valve not opening", isNo+1, RX_ColorNameShort(isNo));
			if (_Stat->ink_supply[isNo].error&err_damper)		 
				ErrorFlag(ERR_CONT, (UINT32 *)&_Error[isNo], err_damper, 0, "Fluid[%d]: Check0 damper defected or not present", isNo+1, RX_ColorNameShort(isNo));
			if (_Stat->ink_supply[isNo].error&err_ink_tube_suck_air)		 
				ErrorFlag(ERR_CONT, (UINT32 *)&_Error[isNo], err_ink_tube_suck_air, 0, "Fluid[%d-%s]: Check0 INK tube sucking air OR canister empty", isNo+1, RX_ColorNameShort(isNo));
			if (_Stat->ink_supply[isNo].error&err_Foam)		 
				ErrorFlag(ERR_CONT, (UINT32 *)&_Error[isNo], err_Foam, 0, "Fluid[%d-%s]: Check0 Foam was present in cylinder and removed (Restart CHECK)", isNo+1, RX_ColorNameShort(isNo));
			if (_Stat->ink_supply[isNo].error&err_cylinder_empty)		 
				ErrorFlag(ERR_CONT, (UINT32 *)&_Error[isNo], err_cylinder_empty, 0, "Fluid[%d-%s]: Check0 Cylinder empty or sucking a lot of air. FILL the cylinder and restart check", isNo+1, RX_ColorNameShort(isNo));
				
			// Check 1 error
			if (_Stat->ink_supply[isNo].error&err_valves_leakage)		 
				ErrorFlag(ERR_CONT, (UINT32 *)&_Error[isNo], err_valves_leakage, 0, "Fluid[%d-%s]: Check1 Valves (air OR bleed) leakage", isNo, RX_ColorNameShort(isNo));
			if (_Stat->ink_supply[isNo].error&err_air_valve)		 
				ErrorFlag(ERR_CONT, (UINT32 *)&_Error[isNo], err_air_valve, 0, "Fluid[%d-%s]: Check1 Air valve opened, P=%d mbars Maximum reachable (possible ink tube crooked, or canister empty, or air valve leakage)", isNo+1, RX_ColorNameShort(isNo), _Stat->ink_supply[isNo].TestBleedLine_Pump_Phase2);
			if (_Stat->ink_supply[isNo].error&err_bleed_valve)		 
				ErrorFlag(ERR_CONT, (UINT32 *)&_Error[isNo], err_bleed_valve, 0, "Fluid[%d-%s]: Check1 Bleed valve opened, P=%d mbars Maximum reachable (possible ink tube crooked, or canister empty, or air valve leakage)", isNo+1, RX_ColorNameShort(isNo), _Stat->ink_supply[isNo].TestBleedLine_Pump_Phase3);
			if (_Stat->ink_supply[isNo].error&err_air_valveOK)		 
				ErrorFlag(LOG, (UINT32 *)&_Error[isNo], err_air_valveOK, 0, "Fluid[%d-%s]: Check1 Air valve opened, P=1bar OK, Ink Pump = %d ", isNo+1, RX_ColorNameShort(isNo), _Stat->ink_supply[isNo].TestBleedLine_Pump_Phase2);
			if (_Stat->ink_supply[isNo].error&err_bleed_valveOK)		 
				ErrorFlag(LOG, (UINT32 *)&_Error[isNo], err_bleed_valveOK, 0, "Fluid[%d-%s]: Check1 Bleed valve opened, P=1bar OK, Ink Pump = %d ", isNo+1, RX_ColorNameShort(isNo), _Stat->ink_supply[isNo].TestBleedLine_Pump_Phase3);
			if (_Stat->ink_supply[isNo].error&err_air_valve_tight)		 
				ErrorFlag(ERR_CONT, (UINT32 *)&_Error[isNo], err_air_valve_tight, 0, "Fluid[%d-%s]: Check1 Air valve opened, Pump speed too low = %d (possible bleed line crooked) ", isNo+1, RX_ColorNameShort(isNo), _Stat->ink_supply[isNo].TestBleedLine_Pump_Phase2);
			if (_Stat->ink_supply[isNo].error&err_bleed_valve_tight)		 
				ErrorFlag(ERR_CONT, (UINT32 *)&_Error[isNo], err_bleed_valve_tight, 0, "Fluid[%d-%s]: Check1 Bleed valve opened, Pump speed too low = %d (possible bleed line crooked)", isNo+1, RX_ColorNameShort(isNo), _Stat->ink_supply[isNo].TestBleedLine_Pump_Phase3);
		
			// Check 3 error
			if (_Stat->ink_supply[isNo].error&err_feed_tube)		 
				ErrorFlag(ERR_ABORT, (UINT32 *)&_Error[isNo], err_feed_tube, 0, "Fluid[%d-%s]: Check3 Feed tube clogged or disconnected", isNo + 1, RX_ColorNameShort(isNo));
			if (_Stat->ink_supply[isNo].error&err_check4_timeout)		 
				ErrorFlag(ERR_CONT, (UINT32 *)&_Error[isNo], err_check4_timeout, 0, "Fluid[%d-%s]: Check4 Conditioners pump 40% could not be reached after 3 minutes", isNo + 1, RX_ColorNameShort(isNo));
			if (_Stat->ink_supply[isNo].error&err_filter_clogged)		 
				ErrorFlag(ERR_CONT, (UINT32 *)&_Error[isNo], err_filter_clogged, 0, "Fluid[%d-%s]: Check4 Filter clogged, need to be changed (IS pressure too high)", isNo + 1, RX_ColorNameShort(isNo));
			if (_Stat->ink_supply[isNo].error&err_return_pipe)		 
				ErrorFlag(ERR_CONT, (UINT32 *)&_Error[isNo], err_return_pipe, 0, "Fluid[%d-%s]: Check3 Return pipe clogged or disconnected", isNo + 1, RX_ColorNameShort(isNo));
			if (_Stat->ink_supply[isNo].error&err_ink_not_heating)		 
				ErrorFlag(ERR_CONT, (UINT32 *)&_Error[isNo], err_ink_not_heating, 0, "Fluid[%d-%s]: Check4 heater board error, ink not heating", isNo + 1, RX_ColorNameShort(isNo));
			if (_Stat->ink_supply[isNo].error&err_ink_too_hot)		 
				ErrorFlag(ERR_CONT, (UINT32 *)&_Error[isNo], err_ink_too_hot, 0, "Fluid[%d-%s]: Check4 heater board error, ink too hot", isNo + 1, RX_ColorNameShort(isNo));     
            if (_Stat->ink_supply[isNo].error&err_pressure_too_high)
				ErrorFlag(WARN, (UINT32 *)&_Error[isNo], err_pressure_too_high, 0, "Fluid[%d-%s]: Ink Tank Pressure still > 100mbar after 1 minute", isNo + 1, RX_ColorNameShort(isNo));
		}
	}
}

//--- nios_error_reset ------------------------------------
void	nios_error_reset(void)
{
	memset(_Error, 0, sizeof(_Error));
	RX_FluidBoardStatus.err = 0;
	if (!_Init) return;
	_Cfg->cmd.error_reset = TRUE;
}

//--- nios_load --------------------------------------
void nios_load(const char *exepath)
{
	TrPrintfL(TRUE, "nios_load >>%s<<", exepath);
	if (!fpga_is_init()) return;

#ifdef soc
	int		fd;
	INT32	size, read, tio;
	UINT32	*exeMem;
	UINT32  data;
	BYTE	buffer[NIOS_EXE_SIZE];

	hex2bin(exepath, buffer, NIOS_EXE_SIZE, &size);
	if (size)
	{
		if (_NiosMem)
		{
			_NiosMem->cfg.cmd.shutdown = TRUE;
			tio=0;
			while (!_NiosMem->stat.info.is_shutdown)
			{
				if (++tio>50) 
				{
					Error(WARN, 0, "nios_load: Timeout on waiting for nios-shutdown");
					break;
	//				return;
				}

				rx_sleep(10);
			}
		}

//		if (fpga_nios_reset(TRUE)!=REPLY_OK) return;
		
		*_NiosReset = TRUE;
		if (_NiosMem) _NiosMem->cfg.cmd.shutdown = FALSE;

		/* Map Physical address of NIOS RAM to virtual address segment with Read/Write Access */ 
		fd = open("/dev/mem", O_RDWR);
		exeMem  = (INT32*)mmap(NULL, NIOS_EXE_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, fd, NIOS_EXE_ADDR);
		memcpy(exeMem, buffer, size);
		mprotect(exeMem, NIOS_EXE_SIZE, PROT_READ);  
		munmap(exeMem,  NIOS_EXE_SIZE); 
		close(fd);
		
		if (arg_offline)
		{
			printf("\nPress <ENTER> when NIOS loaded.\n");
			getchar();
		}
		
		*_NiosReset = FALSE;
		_Loaded = TRUE;
	}
#endif
}

//--- nios_set_is_cfg --------------------------
void nios_set_is_cfg(SInkSupplyCfg *pcfg)
{
    #define RECOVERY_TEMP		40
    #define RECOVERY_MENISCUS	10
    
    int no=pcfg->no;
	if (no==0) _MaxTemp=0;
    if (no>=0 && no<SIZEOF(_Cfg->ink_supply))
    {
		memcpy(&RX_InkSupplyCfg[no], pcfg, sizeof(SInkSupplyCfg));
		RX_ColorNameInit(no, FALSE, RX_InkSupplyCfg[no].ink.fileName, RX_InkSupplyCfg[no].ink.colorCode);
		_Cfg->ink_supply[no].present         = (pcfg->ink.fileName[0]!=0);
		if (pcfg->cylinderPresSet<=INK_PRESSURE_MAX) _Cfg->ink_supply[no].cylinderPresSet = pcfg->cylinderPresSet;
		
        if (_Cfg->ink_supply[no].ctrl_mode >= ctrl_recovery_start && _Cfg->ink_supply[no].ctrl_mode <= ctrl_recovery_step7)
        {
            _Cfg->ink_supply[no].meniscusSet = RECOVERY_MENISCUS;
            _Cfg->ink_supply[no].heaterTemp = RECOVERY_TEMP * 1000;
        }
        else
        {
            _Cfg->ink_supply[no].meniscusSet = pcfg->meniscusSet;
            _Cfg->ink_supply[no].heaterTemp = pcfg->ink.temp * 1000;
        }
		_Cfg->ink_supply[no].heaterTempMax	= pcfg->ink.tempMax*1000;
        memcpy(_Cfg->ink_supply[no].flushTime, pcfg->ink.flushTime, sizeof(_Cfg->ink_supply[no].flushTime));
		if (pcfg->ink.tempMax>_MaxTemp) _MaxTemp=pcfg->ink.tempMax;
	//	_Cfg->ink_supply[i].condPresOutSet	= pcfg->condPresOutSet[i];
	//	_Cfg->ink_supply[i].fluid_P			= pcfg->fluid_P[i];
	}
}

//--- nios_set_cfg -----------------------------------------------------------
void nios_set_cfg(SFluidBoardCfg *pcfg)
{
	int i;
	char str[32];
	if (!_Init) return;

	memcpy(&RX_FluidBoardCfg, pcfg, sizeof(RX_FluidBoardCfg));
	switch(pcfg->printerType)
	{
	case printer_LB701:			_HeaterUsed=TRUE; break;
	case printer_LB702_UV:		_HeaterUsed=TRUE; break;
	case printer_LH702:			_HeaterUsed=TRUE; break;
    case printer_test_slide_HB:	_HeaterUsed=TRUE; break;
    case printer_Dropwatcher:	_HeaterUsed=TRUE; break;
    case printer_test_table:	_HeaterUsed = (_MaxTemp>36); break;
	case printer_cleaf:			_HeaterUsed=TRUE;
								break;
		
	default: _HeaterUsed=FALSE;			
	}
    
	_Cfg->cmd.lung_enabled		= pcfg->lung_enabled;
	_Cfg->printerType			= pcfg->printerType;
	_Cfg->headsPerColor			= pcfg->headsPerColor;
	nios_error_reset();
}

void nios_set_degasser(int *degas)
{
	 _Cfg->cmd.lung_needed		= *degas;
}

//--- nios_set_ctrlmode ------------------------------------
void nios_set_ctrlmode(int isNo, EnFluidCtrlMode mode)
{
	if (isNo<0 || isNo>NIOS_INK_SUPPLY_CNT) return;
	if (mode!=_Cfg->ink_supply[isNo].ctrl_mode && mode>=ctrl_check_step0 && mode<=ctrl_check_step9)
	{
		switch(mode)
		{
        case ctrl_check_step0: Error(LOG, 0, "Fluid[%d]: Checks Started (~1 sec, Initialize testing parameters)", isNo, mode-ctrl_check_step0); break;
        case ctrl_check_step1: Error(LOG, 0, "Fluid[%d]: Check%d Started (~15 sec, Ink Supply diagnostic)", isNo, mode-ctrl_check_step0); break;
        case ctrl_check_step2: Error(LOG, 0, "Fluid[%d]: Check%d Started (~3 min, Ink Supply valves leakage detection)", isNo, mode-ctrl_check_step0); break;
        case ctrl_check_step3: Error(LOG, 0, "Fluid[%d]: Check%d Started (~3 sec, Waiting for cylinder pressure < 50mbars)", isNo, mode-ctrl_check_step0); break;
        case ctrl_check_step4: Error(LOG, 0, "Fluid[%d]: Check%d Started (~30 sec, Check tubes between Ink Supply and clusters)", isNo, mode-ctrl_check_step0); break;
        case ctrl_check_step5: Error(LOG, 0, "Fluid[%d]: Check%d Started (~3 min, Check clusters and reach needed pressures)", isNo, mode-ctrl_check_step0); break;
    //    case ctrl_check_step6: Error(LOG, 0, "Fluid[%d]: Check%d Started (???)", isNo, mode-ctrl_check_step0); break;
    //    case ctrl_check_step7: Error(LOG, 0, "Fluid[%d]: Check%d Started (???)", isNo, mode-ctrl_check_step0); break;
    //    case ctrl_check_step8: Error(LOG, 0, "Fluid[%d]: Check%d Started (???)", isNo, mode-ctrl_check_step0); break;
    //    case ctrl_check_step9: Error(LOG, 0, "Fluid[%d]: Check%d Started (???)", isNo, mode-ctrl_check_step0); break;
		}
	}

	if (mode==ctrl_off) _Cfg->test_airPressure = 0;
	_Cfg->ink_supply[isNo].ctrl_mode = mode;
}

//--- nios_set_purge_par --------------------------------------
void nios_set_purge_par(int isNo, int delay_pos_y, int time, int act_pos_y)
{
	if (isNo<0 || isNo>NIOS_INK_SUPPLY_CNT) return;
	_Cfg->ink_supply[isNo].purgeTime = time;
    _Cfg->ink_supply[isNo].delay_pos_y = delay_pos_y;
    //_Cfg->ink_supply[isNo].act_pos_y = act_pos_y;
}

//--- _set_testmode -----------------------------
static int _set_testmode(void)
{
	int isNo;
	int all, tio;
	for (isNo=0; isNo<NIOS_INK_SUPPLY_CNT; isNo++) _Cfg->ink_supply[isNo].ctrl_mode = ctrl_test;
	//--- wait ---
	for (tio=0; tio<10; tio++)
	{
		for (isNo=0, all=TRUE; isNo<NIOS_INK_SUPPLY_CNT; isNo++) all &= (_Stat->ink_supply[isNo].ctrl_state == ctrl_test);
		if (all) return TRUE;
		rx_sleep(10);
	}
	return FALSE;
}

//--- nios_set_head_state ------------------------------------
void nios_set_head_state(int isNo, SHeadStateLight *pstat)
{
	if (!_Init || isNo<0 || isNo>NIOS_INK_SUPPLY_CNT) return;
	RX_FluidBoardStatus.stat[isNo].info.condTempReady = pstat->condTempReady;
	_Cfg->ink_supply[isNo].headTemp                   = pstat->temp;
	_Cfg->ink_supply[isNo].condPumpSpeed			  = pstat->condPumpSpeed;
	_Cfg->ink_supply[isNo].condPumpFeedback			  = pstat->condPumpFeedback;
	_Cfg->ink_supply[isNo].condPresIn				  = pstat->condPresIn;
	_Cfg->ink_supply[isNo].condPresOut				  = pstat->condPresOut;	
	_Cfg->ink_supply[isNo].condMeniscus				  = pstat->condMeniscus;
	_Cfg->ink_supply[isNo].condMeniscusDiff			  = pstat->condMeniscusDiff;
	_Cfg->ink_supply[isNo].canisterEmpty			  = pstat->canisterEmpty;
    _Cfg->ink_supply[isNo].act_pos_y			      = pstat->act_pos_y;
	_Cfg->ink_supply[isNo].alive++;
}

//--- nios_test_stop -------------------------------------------------
void nios_test_stop(void)
{
	int isNo;
	_Cfg->test_lungPressure	= 0;
	_Cfg->test_flush		= 0;
	_Cfg->test_airPressure	= 0;
	_Cfg->ctc_valves = 0;
	for(isNo=0; isNo<NIOS_INK_SUPPLY_CNT; isNo++) 
	{
		_Cfg->ink_supply[isNo].test_airValve	= FALSE;
		_Cfg->ink_supply[isNo].test_bleed_line	= FALSE;
		_Cfg->ink_supply[isNo].test_bleedValve	= FALSE;
		_Cfg->ink_supply[isNo].test_cylinderPres = 0;
		_Cfg->ink_supply[isNo].ctrl_mode		 = ctrl_off;
	}
}

//--- nios_test_air_valve --------------------------------------------
void nios_test_air_valve(int isNo, int value)
{
	if (_set_testmode()) _Cfg->ink_supply[isNo].test_airValve = value;
}

//--- nios_test_bleed_line --------------------------------------------
void nios_test_bleed_line(int isNo)
{
	if (_set_testmode()) _Cfg->ink_supply[isNo].test_bleed_line = TRUE;
}

//--- nios_test_bleed_valve --------------------------------------------------
void nios_test_bleed_valve(int isNo, int value)
{
	if (_set_testmode()) _Cfg->ink_supply[isNo].test_bleedValve = value;
}

//--- nios_set_ctc_valve --------------------------------------------------
void nios_set_ctc_valve(int valve, int state)
{
    if (state)	 _Cfg->ctc_valves |=  (1<<valve);
    else         _Cfg->ctc_valves &= ~(1<<valve);
}

//--- nios_test_ink_pump ----------------------------------------------
void nios_test_ink_pump(int isNo, int pressure)
{
	if (_set_testmode()) _Cfg->ink_supply[isNo].test_cylinderPres = pressure;
}
	
//--- nios_test_vacuum ------------------------------------------------
void nios_test_vacuum(int pressure)
{
	if (_set_testmode()) _Cfg->test_lungPressure = pressure;
}
		
//--- nios_test_air_pressure ----------------------------------------------
void nios_test_air_pressure(int pressure)
{
	if (_set_testmode()) _Cfg->test_airPressure = pressure;						
}

//--- nios_test_flush ----------------------------
void nios_test_flush(int power)
{
	if (_Cfg) _Cfg->test_flush = power;
}

//--- _simu_fluidsystem ------------------------------------------
void _simu_fluidsystem(void)
{
	int i;
	for (i=0; i<SIZEOF(_Cfg->ink_supply); i++)
	{
		if (_Cfg->ink_supply[i].ctrl_mode!=_SimuData[i].old_mode)
		{
			_SimuData[i].time = 2;
		}
		_SimuData[i].old_mode = _Cfg->ink_supply[i].ctrl_mode;

		if (_SimuData[i].time==0)
		{
			switch(_Cfg->ink_supply[i].ctrl_mode)
			{
				case ctrl_undef:		_Stat->ink_supply[i].ctrl_state = ctrl_print;
										_Cfg->ink_supply[i].ctrl_mode   = ctrl_print;
										break;
				
				default: _Stat->ink_supply[i].ctrl_state = _Cfg->ink_supply[i].ctrl_mode;
			}
		}
		else _SimuData[i].time--;
	}
}


//--- _update_status ----------------------------------
void _update_status(void)
{
	int i;
	int condTempReady;
	SInkSupplyStat *pstat;
		
	for (i=0; i<SIZEOF(RX_FluidBoardStatus.stat); i++)
	{
		pstat = &RX_FluidBoardStatus.stat[i];
		
		condTempReady = pstat->info.condTempReady;
		pstat->info.val = 0x00;
		pstat->warn.val = 0x00;
		pstat->err		= _Stat->ink_supply[i].error;
		if (!_HeaterUsed) pstat->err &= ~err_heater_board;

		pstat->info.connected		= TRUE;
		pstat->bleedValve			= _Stat->ink_supply[i].bleedValve;
		pstat->airValve				= _Stat->ink_supply[i].airValve;
		pstat->info.condTempReady	= condTempReady;
		pstat->info.heaterTempReady = !_HeaterUsed || _Stat->ink_supply[i].heaterTempReady;
		if (_Cfg->cmd.lung_enabled) pstat->presLung = _Stat->degass_pressure;
		else						pstat->presLung = INVALID_VALUE;
		pstat->condPresOut		= _Cfg->ink_supply[i].condPresOut;
		pstat->condPresIn		= _Cfg->ink_supply[i].condPresIn;
		pstat->condPumpSpeed	= _Cfg->ink_supply[i].condPumpFeedback;
		pstat->condTemp			= _Cfg->ink_supply[i].headTemp;
//		pstat->meniscus			= _Cfg->ink_supply[i].condMeniscus; //_Stat->ink_supply[i].meniscus;
		if (_Stat->ink_supply[i].ctrl_state!=pstat->ctrlMode && _Stat->ink_supply[i].ctrl_state>ctrl_check_step0 && _Stat->ink_supply[i].ctrl_state<=ctrl_check_step9)
		{
			Error(LOG, 0, "Fluid[%d]: Check%d Done", i, _Stat->ink_supply[i].ctrl_state-ctrl_check_step0);	
		}
		pstat->ctrlMode			= _Stat->ink_supply[i].ctrl_state;
		pstat->cylinderPresSet	= _Stat->ink_supply[i].cylinderPresSet;		
		pstat->cylinderSetpoint	= _Stat->ink_supply[i].IS_Pressure_Setpoint;
		pstat->cylinderPres		= _Stat->ink_supply[i].IS_Pressure_Actual;
		pstat->cylinderPresDiff	= _Stat->ink_supply[i].IS_Pressure_Diff;
		pstat->flushTime		= _Stat->ink_supply[i].flushTime;
		pstat->airPressureTime	= _Stat->airPressureTime;
		pstat->airPressure		= _Stat->air_pressure;
		pstat->temp				= _Stat->ink_supply[i].heaterTemp;
		pstat->pumpSpeedSet		= _Stat->ink_supply[i].inkPumpSpeed_set;		
		pstat->pumpSpeed		= _Stat->ink_supply[i].inkPumpSpeed_measured;
        pstat->flush_pump_val	= _Stat->flush_pump_val;

        if (pstat->ctrlMode>ctrl_purge_step1 && pstat->ctrlMode<=ctrl_purge_step6) _Cfg->ink_supply[i].purge_putty_pressure=0;

		pstat->purge_putty_ON	= _Cfg->ink_supply[i].purge_putty_pressure>0;
	}
}

//--- _display_status ----------------------------
static void _display_status(void)
{
	int i, val, len;
    int errmask;
	char str[128];
	UCHAR *id;

	if (_Loaded)
	{			
		if (_Simulation) _simu_fluidsystem();
		_update_status();

	//	term_printf("\n--- NIOS Config -------------------------------------------------------\n");
	//	term_printf("\n");
	//	term_printf("ctrl mode:	");	for (i = 0; i < NIOS_INK_SUPPLY_CNT; i++) term_printf("%s   ", FluidCtrlModeStr(_Cfg->ink_supply[i].ctrl_mode)); term_printf("\n");

		term_printf("\n--- NIOS Status -------------------------------------------------------\n");
		sprintf(str, "%d.%d.%d.%d", _Stat->FpgaVersion.major, _Stat->FpgaVersion.minor, _Stat->FpgaVersion.revision, _Stat->FpgaVersion.build);
		term_printf("Version FPGA:    %-16s sysid=%d, timestamp=%d\n", str, fpga_qsys_id(), fpga_qsys_timestamp());
		sprintf(str, "%d.%d.%d.%d", _Stat->version.major, _Stat->version.minor, _Stat->version.revision, _Stat->version.build);
		term_printf("Version NIOS:    %-16s sysid=%d, timestamp=%d  ", str, _Stat->qsys_id, _Stat->qsys_timestamp);
		if (_Stat->error&err_fpga_incompatible) term_printf("ERROR\n");
		else									term_printf("OK\n");
		term_printf("alive:           %d               log:%d", _Stat->alive, _LogCnt);
		if (_HeaterUsed)	term_printf("         heater connected:%d", !(_Stat->error&err_amc_heater));
		term_printf("\n");
		term_printf("error:           0x%08x    Fluid=%d,  Watchdog=%d, Heater=%d\n", 
			_Stat->error,
			(_Stat->error&err_amc_fluid)!=0,
			(_Stat->error&err_watchdog)!=0,
			(_Stat->error&err_amc_heater)!=0
		);
		term_printf("\n");
		
		term_printf("degass pressure: %6s    ", value_str(_Stat->degass_pressure));
		id = (UCHAR*)&_Stat->degas_sensorID;
	//	term_printf("ID: %d.%d.%d.%d", id[0], id[1], id[2], id[3]);		
		term_printf("\n");
		
		term_printf("air pressure:    %6s    ", value_str(_Stat->air_pressure));	
		id = (UCHAR*)&_Stat->air_sensorID;
	//	term_printf("ID: %d.%d.%d.%d", id[0], id[1], id[2], id[3]);		
		if(_Cfg->test_airPressure) term_printf("(Test %d)", _Cfg->test_airPressure); 
		term_printf("\n");
		
		term_printf("flush pressure:  %6s    ", value_str(_Stat->flush_pressure));
		id = (UCHAR*)&_Stat->flush_sensorID;
	//	term_printf("ID: %d.%d.%d.%d", id[0], id[1], id[2], id[3]);		
		term_printf("\n");
		
		term_printf("vacc solenoid:   %d, %d%%", _Stat->vacuum_solenoid, _Stat->duty_degasser); if (_Cfg->test_lungPressure) term_printf("(Test)"); term_printf("\n");
		term_printf("pressure sol:    %d, time=%dms\n", _Stat->air_pressure_solenoid, _Stat->airPressureTime);
		term_printf("flush pump:      %d,  %d%%,   %d\n", _Stat->flush_pump, _Stat->flush_pump_val, _Stat->flush_pres_head);
		term_printf("air pump:        %d\n", _Stat->air_pump);
		term_printf("Heads/Color:     %d\n", _Cfg->headsPerColor);
		term_printf("input:           0x%02x\n", _Stat->input);
		term_printf("p_sensor_error:  "); for (i=0; i<8; i++) term_printf("%5s ", value_str(_Stat->p_sensor_error[i]));	term_printf("\n");
		term_printf("pt100temp:       "); for (i=0; i<8; i++) term_printf("%05d ", _Stat->pt100[i]);			term_printf("\n");
		term_printf("\n");
		/*
		term_printf("ID:               ");
			for (i=0; i<NIOS_INK_SUPPLY_CNT; i++)
			{
				id = (UCHAR*)&_Stat->ink_supply[i].sensorID;
				sprintf(str, "%d.%d.%d.%d", id[0], id[1], id[2], id[3]);		
				term_printf("%11s ", str);
			}
		term_printf("\n");
		*/
		term_printf("Ink Supply---------");		for (i=0; i<NIOS_INK_SUPPLY_CNT; i++) term_printf(" ----%d----  ",i); term_printf("\n");
        
        if (!_HeaterUsed) errmask = ~err_heater_board;
        else errmask=0xffffffff;
		term_printf("error:             ");	for (i=0; i<NIOS_INK_SUPPLY_CNT; i++) term_printf("    0x%04x  ", _Stat->ink_supply[i].error&errmask); term_printf("\n");
		term_printf("ctrl mode:        ");
			for (i=0; i<NIOS_INK_SUPPLY_CNT; i++)
			{
				sprintf(str, "%s(%s)", FluidCtrlModeStr(_Stat->ink_supply[i].ctrl_state), FluidCtrlModeStr(_Cfg->ink_supply[i].ctrl_mode));			
				term_printf("%11s ", str);
			}
		term_printf("\n");
		term_printf("pressure:          ");	
		
		if (_Cfg->ink_supply[i].test_cylinderPres) 
			for (i=0; i<NIOS_INK_SUPPLY_CNT; i++) term_printf("%5s(%03d)  ", value_str(_Stat->ink_supply[i].IS_Pressure_Actual), _Cfg->ink_supply[i].test_cylinderPres);
		else 
//			for (i=0; i<NIOS_INK_SUPPLY_CNT; i++) term_printf("%5s(%03d)  ", value_str(_Stat->ink_supply[i].IS_Pressure_Actual), _Cfg->ink_supply[i].cylinderPresSet);		
			for (i=0; i<NIOS_INK_SUPPLY_CNT; i++) 
			{			
				term_printf("%5s", value_str(_Stat->ink_supply[i].IS_Pressure_Actual));
				term_printf("(~%4s) ", value_str(_Stat->ink_supply[i].IS_Pressure_Diff));		
			}
		term_printf("\n");
		term_printf("inkPump:           ");	for (i=0; i<NIOS_INK_SUPPLY_CNT; i++) term_printf("%4s(%3d%%)  ", value_str(_Stat->ink_supply[i].inkPumpSpeed_measured), _Stat->ink_supply[i].inkPumpSpeed_set); term_printf("\n");
				
		term_printf("bleed valve:       ");	for (i=0; i<NIOS_INK_SUPPLY_CNT; i++) term_printf("       %03d  ", _Stat->ink_supply[i].bleedValve); term_printf("\n");
		term_printf("air valve:         ");	for (i=0; i<NIOS_INK_SUPPLY_CNT; i++) term_printf("       %03d  ", _Stat->ink_supply[i].airValve); term_printf("\n");
		if (RX_FluidBoardCfg.printerType==printer_test_CTC)
		{
			term_printf("ctc valves:        ");	
				term_printf("Return=%d", (_Stat->ctc_valves&(1<<0))!=0); 
				for (i=1; i<5; i++) term_printf(" C%d=%d", i, (_Stat->ctc_valves&(1<<i))!=0); 
				term_printf("  Flush=%d",  (_Stat->ctc_valves&(1<<5))!=0); 
				term_printf(" Release=%d", (_Stat->ctc_valves&(1<<6))!=0); 
			term_printf("\n");
		}
		term_printf("error:             ");	for (i=0; i<NIOS_INK_SUPPLY_CNT; i++) term_printf("    0x%04x  ", _Stat->ink_supply[i].error); term_printf("\n");
		term_printf("Cond. Pressure IN: "); for (i=0; i<NIOS_INK_SUPPLY_CNT; i++) term_printf("  %8s  ", value_str1(_Cfg->ink_supply[i].condPresIn)); term_printf("\n");	
		term_printf("\n");
//		term_printf("Cond. Pres Out Set:"); for (i=0; i<NIOS_INK_SUPPLY_CNT; i++) term_printf("  %8s  ", value_str1(-_Cfg->ink_supply[i].condPresOutSet)); term_printf("\n");	
		term_printf("Cond. Pres In:     "); for (i=0; i<NIOS_INK_SUPPLY_CNT; i++) term_printf("  %8s  ", value_str1(_Cfg->ink_supply[i].condPresIn)); term_printf("\n");	
		term_printf("Cond. Pres Out:    "); for (i=0; i<NIOS_INK_SUPPLY_CNT; i++) term_printf("  %8s  ", value_str1(_Cfg->ink_supply[i].condPresOut)); term_printf("\n");	
		term_printf("Cond. Meniscus:    "); for (i=0; i<NIOS_INK_SUPPLY_CNT; i++) term_printf("  %8s  ", value_str1(_Cfg->ink_supply[i].condMeniscus)); term_printf("\n");	
//		term_printf("Meniscus avg:      "); for (i=0; i<NIOS_INK_SUPPLY_CNT; i++) term_printf("  %8s  ", value_str1(_Stat->ink_supply[i].meniscus)); term_printf("\n");	
		term_printf("flush time:        "); for (i=0; i<NIOS_INK_SUPPLY_CNT; i++) term_printf("  %8s  ", value_str(_Stat->ink_supply[i].flushTime)); term_printf("\n");	
		term_printf("time:              "); for (i=0; i<NIOS_INK_SUPPLY_CNT; i++) term_printf("  %8s  ", value_str(_Stat->ink_supply[i].time)); term_printf("\n");	
		term_printf("diff:              "); for (i=0; i<NIOS_INK_SUPPLY_CNT; i++) term_printf("  %8s  ", value_str(_Stat->ink_supply[i].diff)); term_printf("\n");	
		term_printf("cylinderPresSet:   "); for (i = 0; i < NIOS_INK_SUPPLY_CNT; i++) term_printf("  %8s  ", value_str(_Stat->ink_supply[i].IS_Pressure_Actual)); term_printf("\n");	
		term_printf("Purge pres putty:  "); for (i = 0; i < NIOS_INK_SUPPLY_CNT; i++) term_printf("  %8s  ", value_str(_Cfg->ink_supply[i].purge_putty_pressure)); term_printf("\n");	
		term_printf("Check state (Time):"); for (i = 0; i < NIOS_INK_SUPPLY_CNT; i++) term_printf("  %d(%5d)", _Stat->ink_supply[i].Check_State, _Stat->ink_supply[i].Check_Time_State); term_printf("\n");	
		
		term_printf("Cond. Pump Speed   "); for (i=0; i<NIOS_INK_SUPPLY_CNT; i++) term_printf("  %8s  ", value_str(_Cfg->ink_supply[i].condPumpSpeed)); term_printf("\n");	
		term_printf("Cond. Pump Feedback"); for (i=0; i<NIOS_INK_SUPPLY_CNT; i++) term_printf("  %8s  ", value_str1(_Cfg->ink_supply[i].condPumpFeedback)); term_printf("\n");	
		term_printf("Head Temp:         "); for (i=0; i<NIOS_INK_SUPPLY_CNT; i++) term_printf("  %8s  ", value_str_temp(_Cfg->ink_supply[i].headTemp)); term_printf("\n");	
//		term_printf("PID1-Setpoint  Kp  Ti: "); for (i = 0; i < NIOS_INK_SUPPLY_CNT; i++) term_printf("  %d %d ", _Stat->ink_supply[i].fluid_PIDsetpoint_P, _Stat->ink_supply[i].fluid_PIDsetpoint_I); term_printf("\n");	
//		term_printf("PID2-Pump  Kp  Ti:	"); for (i = 0; i < NIOS_INK_SUPPLY_CNT; i++) term_printf("  %d %d ", _Stat->ink_supply[i].fluid_PIDpump_P, _Stat->ink_supply[i].fluid_PIDpump_I); term_printf("\n");	
		
		term_printf("\nTest Bleed Line -------------------------\n");
		term_printf("Pres - valves OFF"); for (i = 0; i < NIOS_INK_SUPPLY_CNT; i++) term_printf("  %8s  ", value_str(_Stat->ink_supply[i].TestBleedLine_Pump_Phase1)); term_printf("\n");
		term_printf("Pump - Air ON    "); for (i = 0; i < NIOS_INK_SUPPLY_CNT; i++) term_printf("  %8s  ", value_str(_Stat->ink_supply[i].TestBleedLine_Pump_Phase2)); term_printf("\n");
		term_printf("Pump - Bleed ON  "); for (i = 0; i < NIOS_INK_SUPPLY_CNT; i++) term_printf("  %8s  ", value_str(_Stat->ink_supply[i].TestBleedLine_Pump_Phase3)); term_printf("\n");
		
		if (!_HeaterUsed) term_printf(">>> Heater PCB not used\n");
    	else if (nios_is_heater_connected()) 
		{
			term_printf("\n");
			term_printf("--- HEATER ------------------------------------------------------\n");
			term_printf("Heater AMC Volt:         %d.%d(3.3V)  %d.%d(5.0V)  %d.%d(24.0V)  %d.%d(24.0V)\n", _Stat->HeaterBoard_Vsupply_3V / 1000, _Stat->HeaterBoard_Vsupply_3V % 1000, _Stat->HeaterBoard_Vsupply_5V / 1000, _Stat->HeaterBoard_Vsupply_5V % 1000, _Stat->HeaterBoard_Vsupply_24V / 1000, _Stat->HeaterBoard_Vsupply_24V % 1000, _Stat->HeaterBoard_Vsupply_24VP / 1000, _Stat->HeaterBoard_Vsupply_24VP % 1000);
			term_printf("Heater AMC Temp:         %d.%d\n", _Stat->heater_amc_temp / 8, _Stat->heater_amc_temp % 8); 
			term_printf("Temp:              ");  
			for (i = 0; i < NIOS_INK_SUPPLY_CNT; i++) 
			{
				if (_Stat->ink_supply[i].heaterOn)	term_printf("*"); 
				else								term_printf(" ");
				term_printf(" %s", value_str_temp(_Stat->ink_supply[i].heaterTemp));
				term_printf("(%02d/%02d) ", _Cfg->ink_supply[i].heaterTemp / 1000, _Cfg->ink_supply[i].heaterTempMax / 1000);
			}
			term_printf("\n");
			term_printf("openload or error: "); for (i = 0; i < NIOS_INK_SUPPLY_CNT; i++) term_printf("         %d  ", 
        		(_Stat->ink_supply[i].error & err_heater_board)!=0); term_printf("\n");
		}
		else
		{
			term_printf(">>> No Heater PCB present\n");
		}
		
		// Log to file
		if (_TempLogFile)
		{
			fprintf(_TempLogFile, "%d;", _Stat->alive);
			for (i = 0; i < 8; i++) { fprintf(_TempLogFile, "%03d;", _Stat->pt100[i]);}	fprintf(_TempLogFile, "\n");
			fflush(_TempLogFile);
		}
	}
	else 
	{
		term_printf("\n--- NIOS not loaded ---\n");
	}
}

//--- nios_start_temp_log ----------------------------------------------------------
void nios_start_temp_log(void)
{
	int i;
	
	_TempLogFile = fopen(PATH_BIN_FLUID "temp_log.csv", "w");
	if (_TempLogFile != NULL)
	{
		fprintf(_TempLogFile, "Nios Version: %d.%d.%d.%d\n", _Stat->version.major, _Stat->version.minor, _Stat->version.revision, _Stat->version.build);
		fprintf(_TempLogFile, "Nios alive;Temp PT100 0;Temp PT100 1;Temp PT100 2;Temp PT100 3;Temp PT100 4;Temp PT100 5;Temp PT100 6;Temp PT100 7\n");
		fflush(_TempLogFile);
	}
}

//--- _cond_log -----------------------------------------
static int	_plog_on = FALSE;
static FILE	*_plog_file_IS1 = NULL;
static FILE	*_plog_file_IS2 = NULL;
static FILE	*_plog_file_IS3 = NULL;
static FILE	*_plog_file_IS4 = NULL;
static FILE	*_plog_file_Temp = NULL;
static int					_LogTimer;

//--- nios_start_log --------------------------------------
void nios_start_log(void)
{
	_plog_on = !_plog_on;
}

static void _IS_cond_log(int ticks)
{	
	if (!_plog_on) return;
	
	if (_plog_file_IS1 == NULL)
	{
		char name[100];
		sprintf(name, PATH_TEMP "Regulators_IS1.csv");
		_plog_file_IS1 = fopen(name, "w");
		fprintf(_plog_file_IS1, "pump_ticks(ms);PID1_SP-PresINcond (mbar);PID1_FB-PresIN (mbar);PID1_Output-PID2-_P;meniscus cond(mbar);cond pump measured (ml/min x 10);PID1-P;PID1-I; ");
		fprintf(_plog_file_IS1, "PID2_SP-PID1_Output;PID2_Feedback-ISpres (mbar);PID2_Output-PumpSpeed (%);PID2-P;PID2-I;Degas-Duty(%)\n");
		fflush(_plog_file_IS1);
		_LogTimer = rx_get_ticks();
	}
	if (_plog_file_IS2 == NULL)
	{
		char name[100];
		sprintf(name, PATH_TEMP "Regulators_IS2.csv");
		_plog_file_IS2 = fopen(name, "w");
		fprintf(_plog_file_IS2, "pump_ticks(ms);PID1_SP-PresINcond (mbar);PID1_FB-PresIN (mbar);PID1_Output-PID2-_P;meniscus cond(mbar);cond pump measured (ml/min x 10);PID1-P;PID1-I; ");
		fprintf(_plog_file_IS2, "PID2_Setpoint(mbar);PID2_Feedback-ISpres (mbar);PID2_Output-PumpSpeed (%);PID2-P;PID2-I\n");
		fflush(_plog_file_IS2);
	}
	if (_plog_file_IS3 == NULL)
	{
		char name[100];
		sprintf(name, PATH_TEMP "Regulators_IS3.csv");
		_plog_file_IS3 = fopen(name, "w");
		fprintf(_plog_file_IS3, "pump_ticks(ms);PID1_SP-PresINcond (mbar);PID1_FB-PresIN (mbar);PID1_Output-PID2-_P;meniscus cond(mbar);cond pump measured (ml/min x 10);PID1-P;PID1-I; ");
		fprintf(_plog_file_IS3, "PID2_SP-PID1_Output;PID2_Feedback-ISpres (mbar);PID2_Output-PumpSpeed (%);PID2-P;PID2-I\n");
		fflush(_plog_file_IS3);
	}
	if (_plog_file_IS4 == NULL)
	{
		char name[100];
		sprintf(name, PATH_TEMP "Regulators_IS4.csv");
		_plog_file_IS4 = fopen(name, "w");
		fprintf(_plog_file_IS4, "pump_ticks(ms);PID1_SP-PresINcond (mbar);PID1_FB-PresIN (mbar);PID1_Output-PID2-_P;meniscus cond(mbar);cond pump measured (ml/min x 10);PID1-P;PID1-I; ");
		fprintf(_plog_file_IS4, "PID2_SP-PID1_Output;PID2_Feedback-ISpres (mbar);PID2_Output-PumpSpeed (%);PID2-P;PID2-I\n");
		fflush(_plog_file_IS4);
	}
	if (_plog_file_Temp == NULL)
	{
		char name[100];
		sprintf(name, PATH_TEMP "Temperatures.csv");
		_plog_file_Temp = fopen(name, "w");
		fprintf(_plog_file_Temp, "pump_ticks(ms);TempINK1;setpoint;Heads;TempINK2;setpoint;Heads;TempINK3;setpoint;Heads;TempINK4;setpoint;Heads\n ");
		fflush(_plog_file_Temp);
	}

	int time = rx_get_ticks() - _LogTimer;
	static int _lastTime=0;
	
	if (time-_lastTime>10)
	{
		_lastTime = time;
		int i, ISspeed, ISpressure, CondSpeed, Condpressure, Condpout, CondMeniscus;
		for (i = 0; i < 4; i++)
		{
			ISspeed = _Stat->ink_supply[i].PIDpump_Output * 100 / 511;
			if (ISspeed > 100) ISspeed = 100;
			ISpressure = _Stat->ink_supply[i].IS_Pressure_Actual;
			CondSpeed = _Cfg->ink_supply[i].condPumpFeedback;
			if (CondSpeed > 5000) CondSpeed = 0;
			Condpressure = _Cfg->ink_supply[i].condPresIn;
			Condpout = _Cfg->ink_supply[i].condPresOut;
			CondMeniscus = _Cfg->ink_supply[i].condMeniscus;
			switch (i)
			{
			case 0 : 
				fprintf(_plog_file_IS1, "%d;%d;%d;%d;%d;%d;%d;%d;", time, _Cfg->ink_supply[i].cylinderPresSet, Condpressure, _Stat->ink_supply[i].PIDsetpoint_Output, CondMeniscus, CondSpeed, _Stat->ink_supply[i].fluid_PIDsetpoint_P, _Stat->ink_supply[i].fluid_PIDsetpoint_I);
				fprintf(_plog_file_IS1, "%d;%s;%d;%d;%d;%d\n", _Stat->ink_supply[i].PIDsetpoint_Output, value_str(ISpressure), ISspeed, _Stat->ink_supply[i].fluid_PIDpump_P, _Stat->ink_supply[i].fluid_PIDpump_I,  _Stat->duty_degasser); 
				break;
			case 1 : 
				fprintf(_plog_file_IS2, "%d;%d;%d;%d;%d;%d;%d;%d;%d;", time, _Cfg->ink_supply[i].cylinderPresSet, Condpressure, _Stat->ink_supply[i].PIDsetpoint_Output, CondMeniscus, CondSpeed, _Stat->ink_supply[i].fluid_PIDsetpoint_P, _Stat->ink_supply[i].fluid_PIDsetpoint_I, _Stat->ink_supply[i].PIDairvalve_Output);
				fprintf(_plog_file_IS2, "%d;%s;%d;%d;%d\n", _Stat->ink_supply[i].PIDsetpoint_Output, value_str(ISpressure), ISspeed, _Stat->ink_supply[i].fluid_PIDpump_P, _Stat->ink_supply[i].fluid_PIDpump_I); 
				break;
			case 2 : 
				fprintf(_plog_file_IS3, "%d;%d;%d;%d;%d;%d;%d;%d;", time, _Cfg->ink_supply[i].cylinderPresSet, Condpressure, _Stat->ink_supply[i].PIDsetpoint_Output, CondMeniscus, CondSpeed, _Stat->ink_supply[i].fluid_PIDsetpoint_P, _Stat->ink_supply[i].fluid_PIDsetpoint_I);
				fprintf(_plog_file_IS3, "%d;%s;%d;%d;%d\n", _Stat->ink_supply[i].PIDsetpoint_Output, value_str(ISpressure), ISspeed, _Stat->ink_supply[i].fluid_PIDpump_P, _Stat->ink_supply[i].fluid_PIDpump_I); 
				break;
			case 3 : 
				fprintf(_plog_file_IS4, "%d;%d;%d;%d;%d;%d;%d;%d;", time, _Cfg->ink_supply[i].cylinderPresSet, Condpressure, _Stat->ink_supply[i].PIDsetpoint_Output, CondMeniscus, CondSpeed, _Stat->ink_supply[i].fluid_PIDsetpoint_P, _Stat->ink_supply[i].fluid_PIDsetpoint_I);
				fprintf(_plog_file_IS4, "%d;%s;%d;%d;%d\n", _Stat->ink_supply[i].PIDsetpoint_Output, value_str(ISpressure), ISspeed, _Stat->ink_supply[i].fluid_PIDpump_P, _Stat->ink_supply[i].fluid_PIDpump_I); 
				break;
			} 
		}
		fprintf(_plog_file_Temp, "%d", time);
			
		for (i = 0; i < 4; i++)
			fprintf(_plog_file_Temp, ";%d;%d;%d", _Stat->ink_supply[i].heaterTemp, _Cfg->ink_supply[i].heaterTemp, _Cfg->ink_supply[i].headTemp);
		fprintf(_plog_file_Temp, "\n");	
	}
}

// --- nios_is_heater_connected --------------------------------
int nios_is_heater_connected(void)
{
	return _Init && _HeaterUsed && !(_Stat->error&err_amc_heater);
}

// --- nios_set_temp ----------------------------------
void nios_set_temp(int isNo, int temp)
{
	if (isNo > SIZEOF(_Cfg->ink_supply)) return;
	
	_Cfg->ink_supply[isNo].heaterTemp	 = temp *1000;
	_Cfg->ink_supply[isNo].heaterTempMax = (temp+10) * 1000;
}

//--- nios_set_purge_pressure ------------------------------
void nios_set_purge_pressure(int isNo, int pressure)
{
	if (isNo > SIZEOF(_Cfg->ink_supply)) return;
	
	_Cfg->ink_supply[isNo].purge_putty_pressure = pressure;
}