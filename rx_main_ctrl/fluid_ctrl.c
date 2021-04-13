// ****************************************************************************
//
//	fluid_ctrl.cpp
//
// ****************************************************************************
//
//	Copyright 2016 by Radex AG, Switzerland. All rights reserved.
//	Written by Marcel Galliker 
//
// ****************************************************************************

//--- includes ----------------------------------------------------------------
#include "rx_error.h"
#include "rx_sok.h"
#include "rx_ink_common.h"
#include "rx_threads.h"
#include "rx_trace.h"
#include "args.h"
#include "tcp_ip.h"
#include "setup.h"
#include "network.h"
#include "ctrl_svr.h"
#include "ctrl_msg.h"
#include "gui_svr.h"
#include "step_ctrl.h"
#include "step_tx.h"
#include "step_lb.h"
#include "chiller.h"
#include "datalogic.h"
#include "plc_ctrl.h"
#include "print_ctrl.h"
#include "machine_ctrl.h"
#include "fluid_ctrl.h"

//--- SIMULATION ----------------------------------------------

//--- defines -----------------------------------------
#define		TIME_SOFT_PURGE				2000	// [ms]
#define 	TIME_PURGE					3000	// [ms]
#define 	TIME_HARD_PURGE				10000	// [ms]

//--- statics ----------------------------------------

static int				_FluidThreadRunning=FALSE;
static UINT32			_Flushed=0x00;
static EnFluidCtrlMode	_FluidCtrlMode = ctrl_undef;
static EnFluidCtrlMode	_RobotCtrlMode = ctrl_undef;
static EnFluidCtrlMode  _PurgeCtrlMode = ctrl_undef;
static int				_PurgeAll=FALSE;
static int				_PurgeFluidNo;
static int				_Scanning;
static int				_ScalesFluidNo=-1;
static UINT32			_InitDone = 0x00;
static int				_MeasurementNumber = 0;


//--- prototypes -----------------------
static void* _fluid_thread(void *par);

static int _handle_fluid_ctrl_msg	(RX_SOCKET socket, void *msg, int len, struct sockaddr *sender, void *ppar);
static int _connection_closed		(RX_SOCKET socket, const char *peerName);
static void _send_ctrlMode			(int no, EnFluidCtrlMode ctrlMode, int sendToHeads);
static void _send_purge_par			(int fluidNo, int time);

static void _do_fluid_stat(int fluidNo, SFluidBoardStat *pstat);
static void _do_scales_stat(int fluidNo, SScalesStatMsg   *pstat);
static void _do_scales_get_cfg(SScalesCfgMsg *pmsg);
static void _do_log_evt(int no, SLogMsg *msg);
static void _control(int fluidNo);
static void _control_flush();
static void _fluid_send_flush_time(int no, INT32 time);
static int  _all_fluids_in_fluidCtrlMode(EnFluidCtrlMode ctrlMode);
static int  _all_fluids_in_3fluidCtrlModes(EnFluidCtrlMode ctrlMode1, EnFluidCtrlMode ctrlMode2, EnFluidCtrlMode ctrlMode3);
static int _fluid_get_flush_time(int flush_cycle);

//--- statics -----------------------------------------------------------------

typedef struct
{
	int no;
	RX_SOCKET	socket;
	int	aliveTime;
} SFluidThreadPar;

static SFluidThreadPar _FluidThreadPar[FLUID_BOARD_CNT];


static SInkSupplyStat   _FluidStatus[INK_SUPPLY_CNT+2];
static ELogItemType		_ScalesErr[INK_SUPPLY_CNT+2];
static INT32			_ScalesStatus[MAX_SCALES];
static INT32			_FluidToScales[INK_SUPPLY_CNT+2];


static SHeadStateLight	_HeadState[INK_SUPPLY_CNT];
static SHeadStateLight	_HeadStateCnt[INK_SUPPLY_CNT];
static int				_HeadFlowFactorCnt[INK_SUPPLY_CNT];

static INT32			_HeadErr[INK_SUPPLY_CNT+2];
static INT32			_HeadPumpSpeed[INK_SUPPLY_CNT][2];	// min/max
// EnFluidCtrlMode			_FluidMode[INK_SUPPLY_CNT];

static INT32			_CanisterLevel[INK_SUPPLY_CNT + 2][MEASUREMENT_NUMBER];
static INT64			_CanisterLevelSum[INK_SUPPLY_CNT + 2];



//--- fluid_init ----------------------------------------------------------------
int	fluid_init(void)
{
	int i;
	
	memset(_FluidStatus,		0, sizeof(_FluidStatus));
	memset(_ScalesErr,		    0, sizeof(_ScalesErr));
	memset(_FluidThreadPar,		0, sizeof(_FluidThreadPar));
	memset(_HeadStateCnt,		0, sizeof(_HeadStateCnt));
	memset(_HeadFlowFactorCnt,	0, sizeof(_HeadFlowFactorCnt));	
	memset(_HeadState,			0, sizeof(_HeadState));
	memset(_HeadErr,			0, sizeof(_HeadErr));
	RX_PrinterStatus.inkSupilesOff = FALSE;
	RX_PrinterStatus.inkSupilesOn  = FALSE;
	
	fluid_init_flushed();

	for (i=0; i<SIZEOF(_ScalesStatus); i++) _ScalesStatus[i]=INVALID_VALUE;

	for (i = 0; i < INK_SUPPLY_CNT + 2; i++)
	{
		_CanisterLevelSum[i] = 0;
		for (int j = 0; j < MEASUREMENT_NUMBER; j++) _CanisterLevel[i][j] = 0;
	}
	
	for (i=0; i<SIZEOF(_FluidThreadPar); i++)
	{
		_FluidThreadPar[i].no    =i;
		_FluidThreadPar[i].socket=INVALID_SOCKET;
		_FluidThreadPar[i].aliveTime = 0;
	}
	
	if (!_FluidThreadRunning)
	{
		_FluidThreadRunning = TRUE;
		rx_thread_start(_fluid_thread, NULL, 0, "_fluid_thread");	
	}
	return REPLY_OK;
}

//--- fluid_init_flushed ----------------------------------------------------------------
void fluid_init_flushed(void)
{
	setup_fluid_system(PATH_USER FILENAME_FLUID_STATE, &_Flushed, READ);
}

//--- fluid_end -----------------------------------------------------------------
int fluid_end(void)
{
	_FluidThreadRunning = FALSE;
	return REPLY_OK;
}

//--- _fluid_thread ------------------------------------------------------------
static void *_fluid_thread(void *lpParameter)
{
	int errNo;
	int i;
	char addr[32];
	
	while (_FluidThreadRunning)
	{
		//--- send to board -----
		for (i=0; i<SIZEOF(_FluidThreadPar); i++)
		{
			if (RX_Config.inkSupplyCnt > i*INK_PER_BOARD)
			{
				if (_FluidThreadPar[i].socket==INVALID_SOCKET && net_port_listening(dev_fluid, i, PORT_CTRL_FLUID))
				{
					net_device_to_ipaddr(dev_fluid, i, addr, sizeof(addr));
					errNo=sok_open_client_2(&_FluidThreadPar[i].socket, addr, PORT_CTRL_FLUID, SOCK_STREAM, _handle_fluid_ctrl_msg, _connection_closed);
					if (errNo == REPLY_OK)
					{
						ErrorEx(dev_fluid, i, LOG, 0, "Connected");
						_FluidThreadPar[i].aliveTime=0;
						fluid_set_config();
						/*
						{
							char name[64];
							char peer[64];
							sok_get_socket_name(_FluidThreadPar[i].socket, name, NULL, NULL);
							sok_get_peer_name(_FluidThreadPar[i].socket, peer, NULL, NULL);
							Error(LOG, 0, "Fluid[%d] connected >>%s<< to >>%s<<", i, name, peer);												
						}
						*/
					}
					else
					{
						char str[256];
						printf("Fluid[%d].connect: Socket Error >>%s<<\n", i, err_system_error(errNo, str,  sizeof(str)));
					}
				}
			}
		}
		rx_sleep(1000);
	}
	return NULL;
}

//--- _connection_closed --------------------------------
static int _connection_closed(RX_SOCKET socket, const char *peerName)
{
	int i;
	TrPrintf(TRUE, "Connection Closed >>%s<<", peerName);
	for (i=0; i<SIZEOF(_FluidThreadPar); i++)
	{
		if (socket==_FluidThreadPar[i].socket)	
		{
			if (i==_ScalesFluidNo) _ScalesFluidNo=-1;
			sok_close(&_FluidThreadPar[i].socket);
			TrPrintf(TRUE, "Fluid[%d]", i);
		}
	}
	return REPLY_OK;
}

//--- fluid_set_config ----------------------------------------------------------
#define SCALE(board, scale) (((board)-1)*6+(scale)-1)
void fluid_set_config(void)
{
    SInkSupplyCfg  iscfg;
	SFluidBoardCfg cfg;
	int i, n;
	
	_Scanning = rx_def_is_scanning(RX_Config.printer.type);

	//---------------------------------------------------
	memset(_FluidToScales, 0, sizeof(_FluidToScales));

	switch (RX_Config.printer.type)
	{
	case printer_LB701:
							_FluidToScales[0] = SCALE(1, 1);			  // Cyan
							_FluidToScales[1] = SCALE(1, 2);			  // Magenta
							_FluidToScales[2] = SCALE(1, 3);			  // Yellow
							_FluidToScales[3] = SCALE(1, 4);			  // Black
							_FluidToScales[4] = SCALE(2, 1);			  // White
							_FluidToScales[5] = SCALE(2, 2);			  // Orange
							_FluidToScales[6] = SCALE(2, 3);			  // Violet
							_FluidToScales[INK_SUPPLY_CNT] = SCALE(1, 5); // flush
							break;

	case printer_LB702_UV:	
	case printer_LB703_UV:	
	case printer_LH702:
							_FluidToScales[0] = SCALE(1,1);	// Cyan 
							_FluidToScales[1] = SCALE(1,2);	// Magenta
							_FluidToScales[2] = SCALE(1,3);	// Yellow 
							_FluidToScales[3] = SCALE(1,4);	// Black
							_FluidToScales[4] = SCALE(2,1);	// White
							_FluidToScales[5] = SCALE(2,2);	// Orange
							_FluidToScales[6] = SCALE(2,3);	// Violet
							_FluidToScales[INK_SUPPLY_CNT]   = SCALE(2,4);	// flush		
							break;
		
	case printer_LB702_WB:	_FluidToScales[0] = SCALE(1, 1);// 0;	// Cyan 
							_FluidToScales[1] = SCALE(1, 2);// 1;	// Magenta
							_FluidToScales[2] = SCALE(1, 3);// 2;	// Yellow 
							_FluidToScales[3] = SCALE(1, 4);// 3;	// Black
							_FluidToScales[4] = SCALE(2, 1);// 6;	// Orange
							_FluidToScales[5] = SCALE(2, 2);// 7;	// Violet
							_FluidToScales[INK_SUPPLY_CNT]   = SCALE(2, 3);// 8;	// flush		
							_FluidToScales[INK_SUPPLY_CNT + 1] = SCALE(2, 4);// 9;	// waste		
							break;
		
    case printer_cleaf:		_FluidToScales[0] = SCALE(1, 1);// 0;	// Cyan 
							_FluidToScales[1] = SCALE(1, 2);// 1;	// Magenta
							_FluidToScales[2] = SCALE(1, 3);// 2;	// Yellow 
							_FluidToScales[3] = SCALE(1, 4);// 3;	// Black
						//	_FluidToScales[INK_SUPPLY_CNT]	   = SCALE(2, 3);// flush		
							_FluidToScales[INK_SUPPLY_CNT + 1] = SCALE(2, 1);// waste		
							break;
		
    case printer_TX801:
    case printer_TX802:		
	case printer_TX404:		if (RX_Config.inkSupplyCnt<=4)
							{
								_FluidToScales[3] = SCALE(1,1); // Black
								_FluidToScales[2] = SCALE(1,2); // Cyan
								_FluidToScales[1] = SCALE(1,3); // Magenta
								_FluidToScales[0] = SCALE(1,4); // Yellow
								_FluidToScales[INK_SUPPLY_CNT]   = SCALE(1,1);// 4;	// flush		
							}
							else
							{
								_FluidToScales[0] = SCALE(2,4); // Orange 
							    _FluidToScales[1] = SCALE(2,3); // Red
							    _FluidToScales[2] = SCALE(2,2); // Blue 
							    _FluidToScales[3] = SCALE(2,1); // Yellow
							    _FluidToScales[4] = SCALE(1,5); // Magenta
							    _FluidToScales[5] = SCALE(1,4); // Cyan
							    _FluidToScales[6] = SCALE(1,3); // Black
							    _FluidToScales[7] = SCALE(1,2); // Penetration
							    _FluidToScales[INK_SUPPLY_CNT]   = SCALE(1,1);// 4;	// flush		
							}
						break;
                        
	default:			for (i=0; i<SIZEOF(_FluidToScales); i++) _FluidToScales[i]=i;	
						break; 
	}
	
	//--- flush time ------------------------
	if (_Scanning)
	{
		int flushTime[3];
		memset(flushTime, 0, sizeof(flushTime)); 
		for (i=0; i<RX_Config.inkSupplyCnt; i++)
		{
			for (n=0; n<SIZEOF(flushTime); n++)
				if (RX_Config.inkSupply[i].ink.flushTime[n] > flushTime[n]) flushTime[n] = RX_Config.inkSupply[i].ink.flushTime[n];
		}
		for (i=0; i<RX_Config.inkSupplyCnt; i++)
		{
			memcpy(&RX_Config.inkSupply[i].ink.flushTime, flushTime, sizeof(flushTime)); 
		}
	}

	for (i=0; i<FLUID_BOARD_CNT; i++)
	{
		if (_FluidThreadPar[i].socket!=INVALID_SOCKET)
		{
			cfg.printerType = RX_Config.printer.type;
			cfg.lung_enabled = (i==0);
			for (n=0; n<INK_PER_BOARD; n++) 
			{				                
				memcpy(&iscfg, &RX_Config.inkSupply[i*INK_PER_BOARD+n], sizeof(iscfg));
                iscfg.no			= n;
				iscfg.meniscusSet	= INVALID_VALUE;
				sok_send_2(&_FluidThreadPar[i].socket, CMD_FLUID_IS_CFG, sizeof(iscfg), &iscfg);            
			}    				
			cfg.headsPerColor = RX_Config.headsPerColor;

			sok_send_2(&_FluidThreadPar[i].socket, CMD_FLUID_CFG, sizeof(cfg), &cfg);
			sok_send_2(&_FluidThreadPar[i].socket, CMD_SCALES_SET_CFG, sizeof(RX_Config.scales), &RX_Config.scales);
		}
	}	
}

//--- fluid_error_reset ----------------------------------------
void fluid_error_reset(void)
{
	int i;
	
	memset(_ScalesErr,		    0, sizeof(_ScalesErr));
	
	for (i=0; i<FLUID_BOARD_CNT; i++)
	{
		if (_FluidThreadPar[i].socket!=INVALID_SOCKET) 
		{
			sok_send_2(&_FluidThreadPar[i].socket, CMD_ERROR_RESET, 0, NULL);		
		}
	}
}

//--- fluid_tick ----------------------------------------------------------------
void fluid_tick(void)
{
	int i;
	int tol=2;
	int maxTempReady = 0;
	int time=rx_get_ticks();
	
#define TIMEOUT 5000
	SHeadStateLight state[INK_SUPPLY_CNT];
	
	//---  calculate average value
	for (i=0; i<INK_SUPPLY_CNT; i++)
	{
		if  (_HeadStateCnt[i].condPresIn>0)		state[i].condPresIn = _HeadState[i].condPresIn/_HeadStateCnt[i].condPresIn;
		else									state[i].condPresIn = INVALID_VALUE; 
		if  (_HeadStateCnt[i].condPresOut>0)	state[i].condPresOut = _HeadState[i].condPresOut/_HeadStateCnt[i].condPresOut;
		else									state[i].condPresOut = INVALID_VALUE; 
		if  (_HeadStateCnt[i].condMeniscus>0)	state[i].condMeniscus = _HeadState[i].condMeniscus/_HeadStateCnt[i].condMeniscus;
		else									state[i].condMeniscus = INVALID_VALUE;
		if  (_HeadStateCnt[i].condMeniscusDiff>0)	state[i].condMeniscusDiff = 12;
		else										state[i].condMeniscusDiff = INVALID_VALUE;

		if  (_HeadStateCnt[i].temp>0)
		{
			state[i].temp		   = _HeadState[i].temp/_HeadStateCnt[i].temp;
			state[i].condTempReady = _HeadState[i].condTempReady/_HeadStateCnt[i].temp;
		}
		else
		{
			state[i].temp = INVALID_VALUE; 
			state[i].condTempReady = TRUE;
		}
		if  (_HeadStateCnt[i].condPumpSpeed>0)//|| _HeadPumpSpeed[i][0]>=CALIBRATE_PUMP_SPEED-tol)
		{
			state[i].condPumpSpeed = _HeadState[i].condPumpSpeed/_HeadStateCnt[i].condPumpSpeed;
		//	state[i].condPumpSpeed = _HeadState[i].condPumpSpeed;
	//		TrPrintf(TRUE, "PumpSpeed[%d]: min=%d max=%d, avg=%d", i, _HeadPumpSpeed[i][0], _HeadPumpSpeed[i][1], state[i].pumpSpeed);
		}
		else 
		{
			state[i].condPumpSpeed = INVALID_VALUE;
		}
		if (_HeadStateCnt[i].condPumpSpeed)	_FluidStatus[i].info.cond_flowFactor_ok = _HeadFlowFactorCnt[i]/_HeadStateCnt[i].condPumpSpeed;
		else _FluidStatus[i].info.cond_flowFactor_ok=FALSE;
			
		if  (_HeadStateCnt[i].condPumpFeedback > 0)
			state[i].condPumpFeedback = _HeadState[i].condPumpFeedback / _HeadStateCnt[i].condPumpFeedback;
		else 
			state[i].condPumpFeedback = INVALID_VALUE;
		
		if (!chiller_is_running() &&  _FluidStatus[i].ctrlMode>ctrl_off)
			_send_ctrlMode(i, ctrl_off, TRUE);
					
		state[i].canisterEmpty = (_FluidStatus[i].canisterErr >= LOG_TYPE_ERROR_CONT);
	}
	
	RX_PrinterStatus.tempReady = maxTempReady;

	memset(_HeadStateCnt,		0, sizeof(_HeadStateCnt));
	memset(_HeadState,			0, sizeof(_HeadState));
	memset(_HeadPumpSpeed,		0, sizeof(_HeadPumpSpeed));
	memset(_HeadFlowFactorCnt,	0, sizeof(_HeadFlowFactorCnt));

	for (i=0; i<SIZEOF(_FluidThreadPar); i++)
	{
		if (_FluidThreadPar[i].socket!=INVALID_SOCKET) 
		{
			if (_FluidThreadPar[i].aliveTime && _FluidThreadPar[i].aliveTime + TIMEOUT < time)
			{
				sok_close(&_FluidThreadPar[i].socket);
				ErrorEx(dev_fluid, i, LOG, 0, "Connection lost");
			}
			else sok_send_2(&_FluidThreadPar[i].socket, CMD_FLUID_STAT, INK_PER_BOARD*sizeof(state[0]), &state[i*INK_PER_BOARD]);
		}
	}
}

//--- _handle_fluid_ctrl_msg --------------------------------------------------------
static int _handle_fluid_ctrl_msg(RX_SOCKET socket, void *msg, int len, struct sockaddr *sender, void *ppar)
{
	SFluidThreadPar *par = (SFluidThreadPar*) ppar;
	SMsgHdr			*phdr = (SMsgHdr*)msg;
	char			str[32];
	int no;

	EDevice device;
	ctrl_get_device(socket, &device, &no);
	net_register_by_device(dev_fluid, no);
	
//	TrPrintfL(TRUE, "received Fluid[%d].MsgId=0x%08x", no, phdr->msgId);

	for (no=0; no<SIZEOF(_FluidThreadPar); no++)
	{
		if (socket==_FluidThreadPar[no].socket)
		{
			_FluidThreadPar[no].aliveTime = rx_get_ticks();
			switch(phdr->msgId)
			{
			case REP_PING:			 TrPrintfL(TRUE, "Received REP_PING from %s", sok_get_peer_name(socket, str, NULL, NULL));	break;
			case EVT_GET_EVT:		 _do_log_evt		(no, (SLogMsg*)		  msg);					break;																			
			case REP_FLUID_STAT:	 _do_fluid_stat		(no, (SFluidBoardStat*)&phdr[1]);			break;			
			case REP_SCALES_STAT:	 _do_scales_stat	(no, (SScalesStatMsg*)msg);						break;
			case REP_SCALES_GET_CFG: _do_scales_get_cfg	((SScalesCfgMsg*)msg);						break;
			default:				 Error(WARN, 0, "Got unknown messageId=0x%08x", phdr->msgId);
			}
			return REPLY_OK;
		}
	}
	return REPLY_OK;
}

//--- _do_log_evt -----------------------------------------------------
static void _do_log_evt(int no, SLogMsg *msg)
{
	SlaveError(dev_fluid, no, &msg->log);
	if (msg->log.logType==LOG_TYPE_ERROR_ABORT && !arg_simuPLC) pc_abort_printing();
}

//--- _do_fluid_stat -------------------------------------------------------
static void _do_fluid_stat(int fluidNo, SFluidBoardStat *pstat)
{
	/*
	int i;
	int cnt = INK_PER_BOARD;
	if (FALSE)
	{
		printf("_do_fluid_stat[%d]\n", fluidNo);

		printf("Canister Level: "); for (i=0; i<cnt; i++) printf("%03d     ", pstat->stat[i].canisterLevel);	printf("\n");
		printf("Humidity:       "); for (i=0; i<cnt; i++) printf("%03d     ", pstat->stat[i].humidity);			printf("\n");
		printf("Pressure Tank:  "); for (i=0; i<cnt; i++) printf("%03d     ", pstat->stat[i].presIntTank);		printf("\n");
		printf("Pressure Lung:  "); for (i=0; i<cnt; i++) printf("%03d     ", pstat->stat[i].presLung);			printf("\n");
		printf("Temperature:    "); for (i=0; i<cnt; i++) printf("%03d     ", pstat->stat[i].temp);				printf("\n");
		printf("Pump Speed:     "); for (i=0; i<cnt; i++) printf("%03d     ", pstat->stat[i].pumpSpeed);		printf("\n");
		printf("Control Mode:   "); for (i=0; i<cnt; i++) printf("%s   ", FluidCtrlModeStr(pstat->stat[i].ctrlMode));		printf("\n");
	}
	*/
		
	for (int i=0; i<INK_PER_BOARD; i++)
	{
		int flowFactor_ok = _FluidStatus[fluidNo*INK_PER_BOARD+i].info.cond_flowFactor_ok;
		memcpy(&_FluidStatus[fluidNo*INK_PER_BOARD+i], &pstat->stat[i], sizeof(_FluidStatus[i]));
		_FluidStatus[fluidNo*INK_PER_BOARD+i].info.cond_flowFactor_ok = flowFactor_ok;
	}
		
	if      ((_FluidCtrlMode>=ctrl_flush_night && _FluidCtrlMode<=ctrl_flush_done) || _FluidCtrlMode == ctrl_cap_step3) _control_flush();
//	else if (_RobotCtrlMode>=ctrl_wipe        && _RobotCtrlMode<ctrl_fill       && fluidNo==0) _control_robot();
	else _control(fluidNo);

	//--- update overall state --------------------------
	SPrinterStatus stat;
	int i;
	memcpy(&stat, &RX_PrinterStatus, sizeof(stat));
	int OLdNeedDegasser = stat.NeedDegasser;
	stat.inkSupilesOff = TRUE;
	stat.inkSupilesOn  = TRUE;
	stat.tempReady	   = TRUE;
	stat.NeedDegasser  = FALSE;
	for (i=0; i<RX_Config.inkSupplyCnt; i++)
	{
		if (RX_Config.inkSupply[i].inkFileName[0]) 
		{
			if (_FluidStatus[i].ctrlMode<=ctrl_off)			stat.inkSupilesOn	= FALSE;
			if (_FluidStatus[i].ctrlMode>ctrl_off)			stat.inkSupilesOff	= FALSE;
			if (!_FluidStatus[i].info.condTempReady)        stat.tempReady      = FALSE;
			if (_FluidStatus[i].ctrlMode == ctrl_print)		stat.NeedDegasser	= TRUE;
		}
	}
	if (stat.printState==ps_off) stat.printState = ps_ready_power;
    if (OLdNeedDegasser != stat.NeedDegasser)
    {
        for (i=0; i<FLUID_BOARD_CNT; i++)
		{
			if (_FluidThreadPar[i].socket!=INVALID_SOCKET)
			{
				int degas = stat.NeedDegasser;
				sok_send_2(&_FluidThreadPar[i].socket, CMD_FLUID_DEGASSER, sizeof(degas), &degas);
			}
		}
	}

	if (memcmp(&stat, &RX_PrinterStatus, sizeof(stat)))
	{
		memcpy(&RX_PrinterStatus, &stat, sizeof(stat));
		gui_send_printer_status(&RX_PrinterStatus);
	}
}

//--- _do_scale_stat -------------------------------------------------------
static void _do_scales_stat(int fluidNo, SScalesStatMsg *pstat)
{
	int i, isno;
	if (_ScalesFluidNo>=0 && fluidNo!=_ScalesFluidNo) Error(WARN, 0, "SCALES on several Fluids");
	if (_ScalesFluidNo<0) sok_send_2(&_FluidThreadPar[fluidNo].socket, CMD_SCALES_SET_CFG, sizeof(RX_Config.scales), &RX_Config.scales);
 	_ScalesFluidNo = fluidNo;
	for (i=0; i<SIZEOF(_ScalesStatus); i++)
	{
		if (_ScalesStatus[i]<100 && pstat->val[i]>5000)
		{
			for (isno=0; isno<SIZEOF(_FluidToScales); isno++)
			{
				if (_FluidToScales[isno]==i)
				{
					dl_trigger(isno);
					break;						
				}
			}
		}
	}
	memcpy(_ScalesStatus, pstat->val, sizeof(_ScalesStatus));
}

//--- _do_scales_get_cfg ----------------------------------------
static void _do_scales_get_cfg(SScalesCfgMsg *pmsg)
{
	memcpy(RX_Config.scales.tara,  pmsg->tara,  sizeof(RX_Config.scales.tara));
	memcpy(RX_Config.scales.calib, pmsg->calib, sizeof(RX_Config.scales.calib));
	setup_save_config();
}

//--- static int _all_fluids_in_fluidCtrlMode ---------------------------
static int _all_fluids_in_fluidCtrlMode(EnFluidCtrlMode ctrlMode)
{
	for (int i=0; i<RX_Config.inkSupplyCnt; i++)
	{
		if (*RX_Config.inkSupply[i].ink.fileName)
		{
			if (_FluidStatus[i].ctrlMode!=ctrlMode) return FALSE;
		}
	}
	return TRUE;
}

//--- static int _all_fluids_in_fluidCtrlModes ---------------------------
static int _all_fluids_in_3fluidCtrlModes(EnFluidCtrlMode ctrlMode1, EnFluidCtrlMode ctrlMode2, EnFluidCtrlMode ctrlMode3)
{
	for (int i = 0; i < RX_Config.inkSupplyCnt; i++)
	{
		if (*RX_Config.inkSupply[i].ink.fileName)
		{
			if (_FluidStatus[i].ctrlMode != ctrlMode1 && _FluidStatus[i].ctrlMode != ctrlMode2 && _FluidStatus[i].ctrlMode != ctrlMode3) return FALSE;
		}
	}
	return TRUE;
}

//--- fluid_purge_fluidNo --------------------------------------------------
int	 fluid_purge_fluidNo(void)
{
	return _PurgeFluidNo;				
}

void undefine_PurgeCtrlMode(void)
{
    _PurgeCtrlMode = ctrl_undef;
}

//--- _control -------------------------------------------------
static void _control(int fluidNo)
{
	static int	_txrob;
    static UINT32 _flushedNeeded = 0x00;

    int i;
    for (i = 0; i < RX_Config.inkSupplyCnt; i++)
    {
        if (*RX_Config.inkSupply[i].inkFileName) _flushedNeeded |= (0x01 << i);
    }
    int no = fluidNo*INK_PER_BOARD;
	SInkSupplyStat *pstat = &_FluidStatus[no];
	int	lbrob = RX_StepperStatus.robot_used; //(RX_Config.printer.type==printer_LB702_UV ||RX_Config.printer.type == printer_LB702_WB);
	
	int HeadNo = ctrl_singleHead();
	if (HeadNo != -1) HeadNo %= 8;
   // Error(LOG, 0, "_PurgeCtrlMode: %x", _PurgeCtrlMode);
	
	if (lbrob && RX_Config.stepper.wipe_speed == 0) RX_Config.stepper.wipe_speed = 10;

	for (i=0; i<INK_PER_BOARD; i++, pstat++, no++)
	{
		if (ctrl_check_all_heads_in_fluidCtrlMode(no, pstat->ctrlMode))
		{
	//		Error(LOG, 0, "Fluid[%d] in mode >>%s<<", no, FluidCtrlModeStr(_stat->ctrlMode));
			switch(pstat->ctrlMode)
			{
				case ctrl_shutdown:		_send_ctrlMode(no, ctrl_shutdown_done, TRUE);	break;	
				case ctrl_shutdown_done:
					_txrob = rx_def_is_tx(RX_Config.printer.type) && step_active(1);
					if (_txrob)	fluid_send_ctrlMode(-1, ctrl_cap, TRUE);
					else		_send_ctrlMode(no, ctrl_off, TRUE);				
					break;

			//	case ctrl_check_step0:	_send_ctrlMode(no, ctrl_off, TRUE);				break;
				case ctrl_check_step0:	_send_ctrlMode(no, ctrl_check_step1, TRUE);	break;
				case ctrl_check_step1:	_send_ctrlMode(no, ctrl_check_step2, TRUE);	break;
				case ctrl_check_step2:	_send_ctrlMode(no, ctrl_check_step3, TRUE);	break;
				case ctrl_check_step3:	_send_ctrlMode(no, ctrl_check_step4, TRUE);	break;
				case ctrl_check_step4:	_send_ctrlMode(no, ctrl_check_step5, TRUE);	break;
				case ctrl_check_step5:	_send_ctrlMode(no, ctrl_check_step6, TRUE);	break;
				case ctrl_check_step6:	_send_ctrlMode(no, ctrl_check_step7, TRUE);	break;
				case ctrl_check_step7:	_send_ctrlMode(no, ctrl_check_step8, TRUE);	break;
				case ctrl_check_step8:	_send_ctrlMode(no, ctrl_check_step9, TRUE);	break;
				case ctrl_check_step9:	_send_ctrlMode(no, ctrl_off,		 TRUE);	break;
				
				case ctrl_purge:
				case ctrl_purge_hard_wipe:	
				case ctrl_purge_hard_vacc:	
				case ctrl_purge_soft:
				case ctrl_purge_hard:		if (lbrob) steplb_rob_to_wipe_pos(no / 2, rob_fct_purge_all);  //steplb_rob_to_wipe_pos(no / 2, HeadNo + rob_fct_purge_head0);
											else	   step_lift_to_top_pos();
											
											_PurgeCtrlMode = pstat->ctrlMode;
											_txrob = rx_def_is_tx(RX_Config.printer.type) && step_active(1);
                                            int time = (RX_Config.printer.type==printer_TX802 || RX_Config.printer.type == printer_TX404)? (2*TIME_HARD_PURGE) : TIME_HARD_PURGE;
											if (pstat->purge_putty_ON) time=0;
											switch(pstat->ctrlMode)
											{
											case ctrl_purge_soft:		_send_purge_par(no, TIME_SOFT_PURGE); _txrob=FALSE; break;
											case ctrl_purge:			_send_purge_par(no, TIME_PURGE);	  _txrob=FALSE; break;
											case ctrl_purge_hard_wipe:	_send_purge_par(no, time); break;
											case ctrl_purge_hard_vacc:	_send_purge_par(no, time); break;
											case ctrl_purge_hard:		_send_purge_par(no, time); _txrob=FALSE; break;
											}
                                            if (_txrob && _PurgeFluidNo < 0 && state_RobotCtrlMode() != ctrl_wash_step1 && state_RobotCtrlMode() != ctrl_wash_step2)
                                            {
                                                if (_InitDone == _flushedNeeded)
													steptx_rob_wash_start();
                                            }
                                            else
												_send_ctrlMode(no, ctrl_purge_step1, TRUE);
                                            
											break;
				
				case ctrl_wash_step6:		if (steptx_rob_wash_done())
											{
                                                if (_PurgeCtrlMode == ctrl_undef)	fluid_send_ctrlMode(-1, ctrl_vacuum, TRUE);
												else								_send_ctrlMode(_PurgeFluidNo, ctrl_purge_step1, TRUE);
												break;
											}													
											break;

                case ctrl_purge_step1:		if ((!lbrob && step_lift_in_top_pos()) || (lbrob && steplb_rob_in_wipe_pos(no / 2, rob_fct_purge_all)))  // steplb_lift_in_up_pos_individually(no / 2))
											{
												if (_txrob && _PurgeFluidNo < 0 && !steptx_rob_wash_done()) break;
												/*
												if (lbrob && !steplb_rob_in_wipe_pos(no/2, HeadNo + rob_fct_purge_head0))
												{
													if (!RX_StepperStatus.robinfo.moving && !RX_StepperStatus.info.moving)	steplb_rob_to_wipe_pos(no/2, HeadNo + rob_fct_purge_head0);
													break;
												}
												*/
												if (!RX_PrinterStatus.scanner_off)
												{
													plc_to_purge_pos();
													_send_ctrlMode(no, ctrl_purge_step2, TRUE);
												}																								
											}
											// else if (lbrob && !RX_StepperStatus.info.moving && RX_StepperStatus.robinfo.moving) steplb_rob_to_wipe_pos(no/2, HeadNo + rob_fct_purge_head0);
											break;
								
				case ctrl_purge_step2:		if (plc_in_purge_pos())
											{
												_send_ctrlMode(no, ctrl_purge_step3, TRUE);
												if (_txrob && _PurgeCtrlMode == ctrl_purge_hard_wipe)
													step_rob_to_wipe_pos(rob_fct_wipe);
												else if (_txrob && _PurgeCtrlMode == ctrl_purge_hard_vacc)
													step_rob_to_wipe_pos(rob_fct_vacuum_all);
											}
											else if (!plc_in_purge_pos() && !RX_PrinterStatus.scanner_off)
											{
												plc_to_purge_pos();
											}
											break;

				case ctrl_purge_step3:		_send_ctrlMode(no, ctrl_purge_step4, TRUE);
											// if (lbrob) steplb_rob_wipe_start(no/2, HeadNo + rob_fct_purge_head0);
											break;

				case ctrl_purge_step4:		if (_PurgeCtrlMode==ctrl_purge_hard || _PurgeCtrlMode==ctrl_purge_hard_wipe || _PurgeCtrlMode==ctrl_purge_hard_vacc)
											{
                                                if ( _all_fluids_in_fluidCtrlMode(ctrl_purge_step4))	_Flushed &= ~_flushedNeeded;
												else													_Flushed &= ~(0x01<<no);
												setup_fluid_system(PATH_USER FILENAME_FLUID_STATE, &_Flushed, WRITE);				
											}
                                            if (!RX_StepperStatus.robinfo.moving && rx_def_is_tx(RX_Config.printer.type) && step_active(1)) step_empty_waste(0);

											if (_txrob && _PurgeFluidNo < 0) 
											{
												if (_PurgeCtrlMode!=ctrl_undef && _all_fluids_in_fluidCtrlMode(ctrl_purge_step4)) 
												{	
                                                    if (_PurgeCtrlMode==ctrl_purge_hard_wipe)
														fluid_send_ctrlMode(-1, ctrl_wipe, TRUE);
													else if (_PurgeCtrlMode==ctrl_purge_hard_vacc)
														fluid_send_ctrlMode(-1, ctrl_vacuum, TRUE);
													_PurgeCtrlMode = ctrl_undef;
												}
											}
											else if (_txrob)
											{
												if (_PurgeCtrlMode!=ctrl_undef && _all_fluids_in_3fluidCtrlModes(ctrl_purge_step4, ctrl_off, ctrl_print))
												{
													fluid_send_ctrlMode(-1, ctrl_vacuum, TRUE);
													_PurgeCtrlMode = ctrl_undef;
												}
											}
 											else if (RX_PrinterStatus.printState==ps_pause)
                                            {
												if (_PurgeCtrlMode!=ctrl_undef && _all_fluids_in_3fluidCtrlModes(ctrl_purge_step4, ctrl_off, ctrl_print))
												{
	                                                _send_ctrlMode(-1, ctrl_print, TRUE);
													_PurgeCtrlMode = ctrl_undef;
                                                }											
                                            }
											else 
											{
												_send_ctrlMode(no, ctrl_off, TRUE);
												if (!_Flushed) _PurgeCtrlMode = ctrl_undef;
											}
											break;

				
				//--- ctrl_fill ------------------------------------------------------------------
				case ctrl_fill:				_send_ctrlMode(no, ctrl_fill_step1, TRUE);		break;
			//	case ctrl_fill_step1:		wait for user input 
				case ctrl_fill_step2:		_send_ctrlMode(no, ctrl_fill_step3, TRUE);		break;
				case ctrl_fill_step3:		_send_ctrlMode(no, ctrl_print,TRUE);			break;

				case ctrl_empty:			_send_ctrlMode(no, ctrl_empty_step1, TRUE);		break;					
			//	case ctrl_empty_step1:		wait for user input 
				case ctrl_empty_step2:		_send_ctrlMode(no, ctrl_off, TRUE);				break;
				
				case ctrl_cal_start:		_send_ctrlMode(no, ctrl_cal_step1,	 TRUE);		break;				
				case ctrl_cal_step1:		_send_ctrlMode(no, ctrl_cal_step2,   TRUE);		break;
				case ctrl_cal_step2:		_send_ctrlMode(no, ctrl_cal_step3,	 TRUE);		break;
				case ctrl_cal_step3:		_send_ctrlMode(no, ctrl_print, TRUE);    
											break;
				
				//--- ctrl_print -------------------------------------------------------------------
				case ctrl_print:			_PurgeAll=FALSE;
                                            break;
				
				//--- ctrl_off ---------------------------------------------------------------------
				case ctrl_off:				_PurgeAll=FALSE;
											//step_rob_stop();
											//step_lift_stop();
					break;				
			}
		}
	}
}

//--- _control_flush -------------------------------------------------
static void _control_flush(void)
{
    static int _txrob;
    static int _startCtrlMode;
    if (_all_fluids_in_fluidCtrlMode(_FluidCtrlMode))
	{
	//	TrPrintfL(TRUE, "All Fluids in mode %d", _FluidCtrlMode);
	
		switch(_FluidCtrlMode)
		{
		case ctrl_flush_night:		
		case ctrl_flush_weekend:		
		case ctrl_flush_week:
								_txrob = rx_def_is_tx(RX_Config.printer.type) && step_active(1);
								step_lift_to_top_pos();
                                _startCtrlMode = _FluidCtrlMode;
                                if (_txrob)
                                {
                                    steptx_rob_cap_for_flush();
                                    _FluidCtrlMode = ctrl_cap;
                                }
                                else
								_FluidCtrlMode=ctrl_flush_step1; 
                                break;
                                
        case ctrl_cap_step3:	if (steptx_rob_cap_flush_prepared()) _FluidCtrlMode = ctrl_flush_step1;
								break;

        case ctrl_flush_step1:	if (step_lift_in_up_pos() && (steptx_rob_cap_flush_prepared() || !_txrob) && !RX_PrinterStatus.scanner_off)
								{
									plc_to_purge_pos();
									_FluidCtrlMode=ctrl_flush_step2;
								}
								break;
		
		case ctrl_flush_step2:	if (plc_in_purge_pos())
								{
									_FluidCtrlMode=ctrl_flush_step3;
								}
								break;

		case ctrl_flush_step3:	_FluidCtrlMode=ctrl_flush_step4;
								break;
		
		case ctrl_flush_step4:	_FluidCtrlMode=ctrl_flush_done;
								if (!RX_StepperStatus.robinfo.moving && rx_def_is_tx(RX_Config.printer.type) && step_active(1)) step_empty_waste(_fluid_get_flush_time(_startCtrlMode - ctrl_flush_night));
								break;
		
		case ctrl_flush_done:	ErrorEx(dev_fluid, -1, LOG, 0, "Flush complete");
								if (!RX_StepperStatus.robinfo.moving && rx_def_is_tx(RX_Config.printer.type) && step_active(1)) step_empty_waste(0);
								if (_txrob)
								{
									fluid_send_ctrlMode(-1, ctrl_cap_step4, TRUE);
									_FluidCtrlMode = ctrl_cap_step4;
								}
								else
								{
									_FluidCtrlMode = ctrl_off;
								}
								break; // send to all
		default: break;		
		}
		
		if (_FluidCtrlMode != ctrl_cap_step4 && _FluidCtrlMode != ctrl_cap_step3)
		{
			for (int i = 0; i < RX_Config.inkSupplyCnt; i++) _send_ctrlMode(i, _FluidCtrlMode, TRUE);
		}
			

	//	TrPrintfL(TRUE, "Next mode %d", _FluidCtrlMode);
	}
}								


//--- fluid_reply_stat ------------------------------------
void fluid_reply_stat(RX_SOCKET socket)	// to GUI
{
	int i;
	int	canisterLow;
	int	canisterEmpty;
	int canisterHysteresis;
	int wasteHigh;
	int wasteFull;
	switch (RX_Config.printer.type)
	{
	case printer_TX801:
	case printer_TX802:	
	case printer_TX404:	canisterLow   = 1500;
						canisterEmpty = 500;
						canisterHysteresis = 100;
						break;

    case printer_cleaf:	canisterLow   = 1500;
						canisterEmpty =  500;
						canisterHysteresis = 100;
						wasteHigh	  = 6000;
						wasteFull	  = 7000;
						break;

	default:			canisterLow   = 1500;
						canisterEmpty = 500;
						canisterHysteresis = 100;
						wasteHigh	  = 18500;
						wasteFull	  = 19500;
	}

	for (i=0; i<SIZEOF(_FluidStatus)-1; i++) // not for waste!
	{
		if (i<INK_SUPPLY_CNT)
		{
			_FluidStatus[i].info.connected = (_FluidThreadPar[i/INK_PER_BOARD].socket!=INVALID_SOCKET);
			_FluidStatus[i].info.flushed   = ((_Flushed & (0x01<<i))!=0);
		}

		_FluidStatus[i].canisterLevel = moving_average_canisterLevel(_CanisterLevel, _CanisterLevelSum, i, _MeasurementNumber, _ScalesStatus[_FluidToScales[i]]);

		if(*RX_Config.inkSupply[i].ink.fileName)
		{
			if (_FluidStatus[i].canisterLevel <= canisterEmpty || _FluidStatus[i].canisterLevel <= canisterEmpty + canisterHysteresis && _ScalesErr[i] == LOG_TYPE_ERROR_CONT)
			{
				if (_ScalesErr[i] != LOG_TYPE_ERROR_CONT)
				{
					if (i == INK_SUPPLY_CNT)
						Error(ERR_CONT, 0, "Flush Canister EMPTY!");
					else
						Error(ERR_CONT, 0, "Ink Canister %s EMPTY = %dg (<%dg)", RX_ColorNameLong(RX_Config.inkSupply[i].ink.colorCode), _FluidStatus[i].canisterLevel, canisterEmpty);
					_ScalesErr[i] = LOG_TYPE_ERROR_CONT;
					if (RX_PrinterStatus.printState == ps_printing) pc_pause_printing(FALSE);
				}
			}
			else if (_FluidStatus[i].canisterLevel <= canisterLow || _FluidStatus[i].canisterLevel <= canisterLow + canisterHysteresis && _ScalesErr[i] == LOG_TYPE_WARN)
			{
				if (_ScalesErr[i] != LOG_TYPE_WARN)
				{
					if (i == INK_SUPPLY_CNT)
						Error(WARN, 0, "Flush Canister LOW!");
					else
						Error(WARN, 0, "Ink Canister %s LOW = %dg (<%dg)", RX_ColorNameLong(RX_Config.inkSupply[i].ink.colorCode), _FluidStatus[i].canisterLevel, canisterLow);
					_ScalesErr[i] = LOG_TYPE_WARN;
				}
			}
			else if (_FluidStatus[i].canisterLevel > canisterLow && _FluidStatus[i].canisterLevel < 50000 && _ScalesErr[i] != LOG_TYPE_LOG)
			{
				_ScalesErr[i] = LOG_TYPE_LOG;
			}
		}
		_FluidStatus[i].canisterErr = _ScalesErr[i];
		
		dl_get_barcode(i, _FluidStatus[i].scannerSN, _FluidStatus[i].barcode);
		if (_HeadErr[i]) _FluidStatus[i].err |= err_printhead; 
		_HeadErr[i]=0;
	}
	
	// waste
	
	if (RX_Config.printer.type == printer_LB702_WB || RX_Config.printer.type == printer_cleaf )
	{
		_FluidStatus[INK_SUPPLY_CNT + 1].canisterLevel = moving_average_canisterLevel(_CanisterLevel, _CanisterLevelSum, INK_SUPPLY_CNT + 1, _MeasurementNumber, _ScalesStatus[_FluidToScales[INK_SUPPLY_CNT + 1]]);
		
		if (_FluidStatus[INK_SUPPLY_CNT + 1].canisterLevel < 50000)
		{
			if (_FluidStatus[INK_SUPPLY_CNT + 1].canisterLevel >= wasteFull || _FluidStatus[INK_SUPPLY_CNT + 1].canisterLevel >= wasteFull - canisterHysteresis && _ScalesErr[INK_SUPPLY_CNT + 1] == LOG_TYPE_ERROR_CONT)
			{
				if (_ScalesErr[INK_SUPPLY_CNT + 1] != LOG_TYPE_ERROR_CONT)
				{
					Error(ERR_CONT, 0, "Waste Canister FULL = %dg > %dg", _FluidStatus[INK_SUPPLY_CNT + 1].canisterLevel, wasteFull);
					_ScalesErr[INK_SUPPLY_CNT + 1] = LOG_TYPE_ERROR_CONT;
				}
			}
			else if (_FluidStatus[INK_SUPPLY_CNT + 1].canisterLevel >= wasteHigh || _FluidStatus[INK_SUPPLY_CNT + 1].canisterLevel >= wasteHigh - canisterHysteresis && _ScalesErr[INK_SUPPLY_CNT + 1] == LOG_TYPE_WARN)
			{
				if (_ScalesErr[INK_SUPPLY_CNT + 1] != LOG_TYPE_WARN)
				{
					Error(WARN, 0, "Waste Canister HIGH = %dg (> %dg)", _FluidStatus[INK_SUPPLY_CNT + 1].canisterLevel, wasteHigh);
					_ScalesErr[INK_SUPPLY_CNT + 1] = LOG_TYPE_WARN;
				}
			}	
		}
		
		_FluidStatus[INK_SUPPLY_CNT + 1].canisterErr = _ScalesErr[INK_SUPPLY_CNT + 1];
	}

	_MeasurementNumber++;
	_MeasurementNumber = _MeasurementNumber % MEASUREMENT_NUMBER;

	{
		SInkSupplyStatMsg msg;
		msg.hdr.msgId  = REP_FLUID_STAT;
		msg.hdr.msgLen = sizeof(msg);
		
		for (msg.no=0; msg.no<SIZEOF(_FluidStatus); msg.no++)
		{
			if (msg.no<RX_Config.inkSupplyCnt || msg.no>=INK_SUPPLY_CNT)	// send also flush and waste
			{
				memcpy(&msg.stat, &_FluidStatus[msg.no], sizeof(msg.stat));
				gui_send_msg(socket, &msg);					
			}
		}		
	}
}

//--- fluid_send_ctrlMode -------------------------------
void fluid_send_ctrlMode(int no, EnFluidCtrlMode ctrlMode, int sendToHeads)
{
	if (ctrlMode==ctrl_off && _FluidCtrlMode>=ctrl_flush_step1 && _FluidCtrlMode<=ctrl_flush_done)
	{
		for (int i=0; i<RX_Config.inkSupplyCnt; i++) _send_ctrlMode(i, ctrlMode, sendToHeads);	
	}
    if ((RX_StepperStatus.info.z_in_cap || !RX_StepperStatus.info.ref_done) &&
        ctrlMode == ctrl_print && rx_def_is_scanning(RX_Config.printer.type))
    {
            steptx_lift_to_print_pos();
    }
    
	if (ctrlMode==ctrl_off) step_rob_stop();
    if (ctrlMode == ctrl_purge_hard || ctrlMode == ctrl_purge_hard_wipe || ctrlMode == ctrl_purge_hard_vacc || ctrlMode == ctrl_purge || ctrlMode == ctrl_purge_soft)
    {
        _PurgeFluidNo = no;
        _InitDone = 0;
    }

	if (ctrlMode >= ctrl_flush_night && ctrlMode <= ctrl_empty_step5 && ctrlMode != ctrl_cap)
	{
		machine_set_capping_timer(TRUE);
	}

    _FluidCtrlMode = ctrlMode;
	_RobotCtrlMode = ctrlMode;
	_send_ctrlMode(no, ctrlMode, sendToHeads);
//	Error(LOG, 0, "fluid_send_ctrlMode 0X%04x", ctrlMode);
	switch (RX_Config.printer.type)
	{
	case printer_TX801:
	case printer_TX802:		
	case printer_TX404:		steptx_set_robCtrlMode(ctrlMode);
							break;
	case printer_LB701:
	case printer_LB702_UV:	break;
	case printer_LB702_WB:	if (ctrlMode == ctrl_cap) steplb_rob_start_cap_all();
							break;
	default:				break;
	}
}

//--- _send_ctrlMode --------------------------------------
void _send_ctrlMode(int no, EnFluidCtrlMode ctrlMode, int sendToHeads)
{
	SFluidCtrlCmd cmd;
	int i;
	if (no<0)
	{
		for (i=0; i<RX_Config.inkSupplyCnt; i++) _send_ctrlMode(i, ctrlMode, TRUE);				
		return;		
	}

	for (i=0; i<INK_SUPPLY_CNT; i++)
	{
		if (_FluidThreadPar[i/INK_PER_BOARD].socket!=INVALID_SOCKET)
		{
			if (ctrlMode>=ctrl_flush_night && ctrlMode<=ctrl_flush_week )
			{
				cmd.hdr.msgId	= CMD_FLUID_CTRL_MODE;
				cmd.hdr.msgLen	= sizeof(cmd);
				cmd.no			= i%INK_PER_BOARD;
				cmd.ctrlMode	= ctrlMode;
				if (!rx_def_is_scanning(RX_Config.printer.type))
				{
					if (i==no) cmd.ctrlMode = ctrlMode;
					else	   cmd.ctrlMode = ctrl_off;
				}
				_Flushed |= (0x1<<i);
				sok_send(&_FluidThreadPar[i/INK_PER_BOARD].socket, &cmd);
				ctrl_send_all_heads_fluidCtrlMode(i, ctrlMode);	
				setup_fluid_system(PATH_USER FILENAME_FLUID_STATE, &_Flushed, WRITE);
			}
			else if (i==no || ctrlMode==ctrl_empty)
			{
				if ((ctrlMode==ctrl_print) && !RX_Config.inkSupply[i].ink.fileName[0]) continue;
				if ((ctrlMode==ctrl_print) && (_Flushed & (0x01<<i))) 
				{
					ErrorEx(dev_fluid, i, WARN, 0, "Is FLUSHED. It must be purged before it can go to print mode.");
					continue;						
				}
				
				RX_PrinterStatus.cleaning = (ctrlMode>ctrl_purge_soft) && (ctrlMode<ctrl_fill);

//				if (ctrlMode==ctrl_shutdown && _FluidStatus[i].ctrlMode<=ctrl_off)   continue;
					
//				if (ctrlMode==ctrl_cal_start)
//				{
//					switch(RX_Config.printer.type)
//					{
//					case printer_TX801:	fluid_send_pressure(no, 150); break;
//					case printer_TX802:	fluid_send_pressure(no, 150); break;
//					default: break;
//					}
//				}
				
				cmd.hdr.msgId	= CMD_FLUID_CTRL_MODE;
				cmd.hdr.msgLen	= sizeof(cmd);
				cmd.no			= i%INK_PER_BOARD;
				if (!RX_Config.inkSupply[i].ink.fileName[0]) cmd.ctrlMode = ctrl_off;
				else if (i==no) cmd.ctrlMode = ctrlMode;
				else if (ctrlMode==ctrl_empty) cmd.ctrlMode = ctrl_off;
                sok_send(&_FluidThreadPar[i/INK_PER_BOARD].socket, &cmd);
				
				//--- send mode to heads ---
				if (sendToHeads)
				{
					int head = ctrl_singleHead();
					if (head<0) ctrl_send_all_heads_fluidCtrlMode(i, cmd.ctrlMode);				
					else ctrl_send_head_fluidCtrlMode(head, cmd.ctrlMode, FALSE, FALSE);
				}
			}
		}			
	}
}

//--- _send_purge_par -------------------------------------------------
static void _send_purge_par(int fluidNo, int time)
{
    SPurgePar par;
	par.no    =  fluidNo%INK_PER_BOARD;
    if (RX_StepperStatus.robot_used) par.delay = 3000;
	else                             par.delay = 0;
	par.time  = ctrl_send_purge_par(fluidNo, time);
	sok_send_2(&_FluidThreadPar[fluidNo/INK_PER_BOARD].socket, CMD_SET_PURGE_PAR, sizeof(par), &par);
    _InitDone |= 0x01 << fluidNo;
}

//--- fluid_send_pressure -------------------------------------------
void fluid_send_pressure(int no, INT32 pressure)
{
	RX_Config.inkSupply[no].cylinderPresSet = pressure;
	fluid_set_config();
}

//--- fluid_send_tara -------------------------------------------
void fluid_send_tara(int no)
{
	if (_ScalesFluidNo>=0)
	{
		no = _FluidToScales[no];
		sok_send_2(&_FluidThreadPar[_ScalesFluidNo].socket, CMD_SCALES_TARA, sizeof(no), &no);			
	}
}

//--- fluid_send_calib -------------------------------------------
void fluid_send_calib(SValue *pmsg)
{
	if (_ScalesFluidNo>=0)
	{
		SValue msg;
		memcpy(&msg, pmsg, sizeof(msg));
		msg.no = _FluidToScales[pmsg->no];
		sok_send_2(&_FluidThreadPar[_ScalesFluidNo].socket, CMD_SCALES_CALIBRATE, sizeof(msg), &msg);			
	}
}

//--- fluid_start_printing --------
void fluid_start_printing(void)
{
	int i;
	for (i=0; i<RX_Config.printer.inkSupplyCnt; i++)
	{
		_send_ctrlMode(i, ctrl_print, TRUE);		
	}
}

//--- fluid_get_ctrlMode -------------------------------------------
EnFluidCtrlMode fluid_get_ctrlMode(int no)
{
//	if (_FluidStatus[no].ctrlMode==ctrl_off)
//		TrPrintfL(TRUE, "fluid_get_ctrlMode[%d].off", no);
	if (no>=0 && no<SIZEOF(_FluidStatus)) return _FluidStatus[no].ctrlMode;
	return ctrl_undef;
}

//--- fluid_in_ctrlMode ----------------------------------------
int  fluid_in_ctrlMode  (int no, EnFluidCtrlMode ctrlMode)
{
	if (ctrlMode==ctrl_readyToPrint && _FluidStatus[no].ctrlMode==ctrl_warmup) return TRUE;
	return (_FluidStatus[no].ctrlMode==ctrlMode);
}

//--- fluid_set_head_state ----------------------------
void fluid_set_head_state	(int no, SHeadStat *pstat)
{
	if (no>=0 && no<SIZEOF(_HeadState) && pstat->ctrlMode!=ctrl_off && pstat->ctrlMode!=ctrl_error) 
	{	
		if (pstat->err) _HeadErr[no] = TRUE;

		if (valid(pstat->presIn))
		{
			_HeadState[no].condPresIn += pstat->presIn;
			_HeadStateCnt[no].condPresIn++; 
		}
		if (valid(pstat->presOut))
		{
			_HeadState[no].condPresOut += pstat->presOut;
			_HeadStateCnt[no].condPresOut++; 
		}
		if (valid(pstat->meniscus))
		{
			_HeadState[no].condMeniscus += pstat->meniscus;
			_HeadStateCnt[no].condMeniscus++; 
		}

		if (valid(pstat->tempHead))
		{
			if (pstat->info.temp_ready) _HeadState[no].condTempReady++;
			_HeadState[no].temp += pstat->tempHead;
			_HeadStateCnt[no].temp++; 
		}
		
		if (pstat->info.flowFactor_ok)  _HeadFlowFactorCnt[no]++;

		if (valid(pstat->pumpSpeed))
		{
			// set min/max
			if ((int)pstat->pumpSpeed < _HeadPumpSpeed[no][0] || !_HeadPumpSpeed[no][0]) _HeadPumpSpeed[no][0] = pstat->pumpSpeed;
			if ((int)pstat->pumpSpeed > _HeadPumpSpeed[no][1])							 _HeadPumpSpeed[no][1] = pstat->pumpSpeed;

			_HeadState[no].condPumpSpeed += pstat->pumpSpeed;
			_HeadStateCnt[no].condPumpSpeed++;
		//	TrPrintfL(TRUE, "Head[%d].punpSpeed=%d, cnt=%d", no, pstat->pumpSpeed, _HeadStateCnt[no].condPumpSpeed);
		}
		if (valid(pstat->pumpFeedback))
		{
			//if ((int)pstat->pumpFeedback < _HeadPumpSpeed[no][0] || !_HeadPumpSpeed[no][0]) _HeadPumpSpeed[no][0] = pstat->pumpFeedback;
			//if ((int)pstat->pumpFeedback > _HeadPumpSpeed[no][1])							 _HeadPumpSpeed[no][1] = pstat->pumpFeedback;

			_HeadState[no].condPumpFeedback += pstat->pumpFeedback;
			_HeadStateCnt[no].condPumpFeedback++; 
		}
	}
}

//--- fluid_get_cylinderPresSet ------------------------------------
INT32 fluid_get_cylinderPresSet (int no)
{
	if (no>=0 && no<SIZEOF(_FluidStatus)) 
	{
		return _FluidStatus[no].cylinderPresSet;			
	}
	return INVALID_VALUE;
}

//--- fluid_get_cylinderPres ---------------------------------------
INT32 fluid_get_cylinderPres(int no)
{
	if (no >= 0 && no<SIZEOF(_FluidStatus))
	{
		return _FluidStatus[no].cylinderPres;
	}
	return INVALID_VALUE;
}

//--- fluid_get_cylinderSetpoint ---------------------------------------
INT32 fluid_get_cylinderSetpoint(int no)
{
	if (no >= 0 && no < SIZEOF(_FluidStatus))
	{
		return _FluidStatus[no].cylinderSetpoint;
	}
	return INVALID_VALUE;
}

//--- fluid_get_pumpSpeed ---------------------------------------
INT32 fluid_get_pumpSpeed(int no)
{
	if (no >= 0 && no < SIZEOF(_FluidStatus))
	{
		return _FluidStatus[no].pumpSpeedSet;
	}
	return INVALID_VALUE;
}

//--- fluid_get_pumpFeedback ---------------------------------------
INT32 fluid_get_pumpFeedback(int no)
{
	if (no >= 0 && no < SIZEOF(_FluidStatus))
	{
		return _FluidStatus[no].pumpSpeed;
	}
	return INVALID_VALUE;
}

//--- fluid_get_amcTemp ---------------------------------------
/*
INT32 fluid_get_amcTemp(int no)
{
	if (no >= 0 && no < SIZEOF(_FluidStatus))
	{
		return _FluidStatus[no].temp;
	}
	return INVALID_VALUE;
}
*/
//--- fluid_get_error ---------------------------------------
INT32 fluid_get_error(int no)
{
    if (no >= 0 && no < SIZEOF(_FluidStatus))
    {
        return _FluidStatus[no].err;
    }
    return INVALID_VALUE;
}

//--- _Fluid_get_flush_time ----------------------------------------------
static int _fluid_get_flush_time(int flush_cycle)
{
	if (flush_cycle >= SIZEOF(RX_Config.inkSupply[0].ink.flushTime) || flush_cycle < 0) return 0;
    
    int time_s = 0;
    for (int i = 0; i < RX_Config.inkSupplyCnt; i++)
    {
        if (*RX_Config.inkSupply[i].ink.fileName && RX_Config.inkSupply[i].ink.flushTime[flush_cycle] > time_s)
            time_s = RX_Config.inkSupply[i].ink.flushTime[flush_cycle];
    }
    return time_s;
}

//--- moving_average_canisterLevel ------------------------------------
INT32 moving_average_canisterLevel(INT32 buffer[INK_SUPPLY_CNT + 2][MEASUREMENT_NUMBER], INT64 sum[INK_SUPPLY_CNT + 2], int canisterNumber, int pos, INT32 value)
{
	sum[canisterNumber] = sum[canisterNumber] - buffer[canisterNumber][pos] + value;
	buffer[canisterNumber][pos] = value;
	static int measurementNumber = 1;
	INT32 avergage = sum[canisterNumber] / measurementNumber;
	if (canisterNumber == INK_SUPPLY_CNT && measurementNumber < MEASUREMENT_NUMBER) 
		measurementNumber++;
	return avergage;
}