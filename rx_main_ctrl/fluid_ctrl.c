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
#include "chiller.h"
#include "datalogic.h"
#include "plc_ctrl.h"
#include "print_ctrl.h"
#include "fluid_ctrl.h"

//--- SIMULATION ----------------------------------------------

//--- defines -----------------------------------------

static int				_FluidThreadRunning=FALSE;
static UINT32			_Flushed=0x00;
static EnFluidCtrlMode	_FluidCtrlMode = ctrl_undef;
static EnFluidCtrlMode	_RobotCtrlMode = ctrl_undef;
static EnFluidCtrlMode  _PurgeCtrlMode = ctrl_undef;
static int				_PurgeFluidNo=0;
static int				_Scanning;

//--- prototypes -----------------------
static void* _fluid_thread(void *par);

static int _handle_fluid_ctrl_msg	(RX_SOCKET socket, void *msg, int len, struct sockaddr *sender, void *ppar);
static int _connection_closed		(RX_SOCKET socket, const char *peerName);
static void _send_ctrlMode			(int no, EnFluidCtrlMode ctrlMode, int sendToHeads);


static void _do_fluid_stat(int fluidNo, SFluidBoardStat *pstat);
static void _do_scales_stat(SScalesStatMsg   *pstat);
static void _do_scales_get_cfg(SScalesCfgMsg *pmsg);
static void _do_log_evt(int no, SLogMsg *msg);
static void _control(int fluidNo);
static void _control_flush();
static void _fluid_send_flush_time(int no, INT32 time);
static int  _all_fluids_in_fluidCtrlMode(EnFluidCtrlMode ctrlMode);

//--- statics -----------------------------------------------------------------

typedef struct
{
	int no;
	RX_SOCKET	socket;
	int	aliveTime;
} SFluidThreadPar;

static SFluidThreadPar _FluidThreadPar[FLUID_BOARD_CNT];


SInkSupplyStat   _FluidStatus[INK_SUPPLY_CNT+2];
ELogItemType	 _ScalesErr[INK_SUPPLY_CNT+2];
INT32			 _ScalesStatus[MAX_SCALES];
INT32			 _FluidToScales[INK_SUPPLY_CNT+2];


static SHeadStateLight	_HeadState[INK_SUPPLY_CNT];
static SHeadStateLight	_HeadStateCnt[INK_SUPPLY_CNT];
static INT32			_HeadErr[INK_SUPPLY_CNT];
static INT32			_HeadPumpSpeed[INK_SUPPLY_CNT][2];	// min/max
// EnFluidCtrlMode			_FluidMode[INK_SUPPLY_CNT];



//--- fluid_init ----------------------------------------------------------------
int	fluid_init(void)
{
	int i;
	
	memset(_FluidStatus,		0, sizeof(_FluidStatus));
	memset(_ScalesErr,		    0, sizeof(_ScalesErr));
	memset(_FluidThreadPar,		0, sizeof(_FluidThreadPar));
	memset(_HeadStateCnt,		0, sizeof(_HeadStateCnt));
	memset(_HeadState,			0, sizeof(_HeadState));
	memset(_HeadErr,			0, sizeof(_HeadErr));
	RX_PrinterStatus.inkSupilesOff = FALSE;
	RX_PrinterStatus.inkSupilesOn  = FALSE;
	
	setup_fluid_system(PATH_USER FILENAME_FLUID_STATE, &_Flushed, READ);				

	for (i=0; i<SIZEOF(_ScalesStatus); i++) _ScalesStatus[i]=INVALID_VALUE;
	
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
			sok_close(&_FluidThreadPar[i].socket);
			TrPrintf(TRUE, "Fluid[%d]", i);
		}
	}
	return REPLY_OK;
}


//--- fluid_set_config ----------------------------------------------------------
void fluid_set_config(void)
{
	SFluidBoardCfgLight cfg;
	int i, n;
	
	_Scanning = rx_def_is_scanning(RX_Config.printer.type);

	//---------------------------------------------------
	memset(_FluidToScales, 0, sizeof(_FluidToScales));
	
	switch (RX_Config.printer.type)
	{
	case printer_LB701:	_FluidToScales[0] = 5;	// Cyan 
						_FluidToScales[1] = 4;	// Magenta
						_FluidToScales[2] = 3;	// Yellow 
						_FluidToScales[3] = 2;	// black		
						_FluidToScales[INK_SUPPLY_CNT]   = 1;	// flush		
						_FluidToScales[INK_SUPPLY_CNT+1] = 0;	// waste
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
				cfg.cylinderPresSet[n] = RX_Config.inkSupply[i*INK_PER_BOARD+n].cylinderPresSet;
	//			cfg.meniscusSet[n]	  = RX_Config.inkSupply[i*INK_PER_BOARD+n].ink.meniscus;
				cfg.meniscusSet[n]	  = INVALID_VALUE;
	//			cfg.condPresOutSet[n] = RX_Config.inkSupply[i*INK_PER_BOARD+n].ink.condPresOut;
				cfg.inkTemp[n]        = RX_Config.inkSupply[i*INK_PER_BOARD+n].ink.temp;
				cfg.inkTempMax[n]	  = RX_Config.inkSupply[i*INK_PER_BOARD+n].ink.tempMax;
				memcpy(cfg.flushTime[n], RX_Config.inkSupply[i*INK_PER_BOARD+n].ink.flushTime, sizeof(cfg.flushTime[n]));
    			// fluid board
    			//cfg.fluid_P[n] = RX_Config.inkSupply[i*INK_PER_BOARD+n].fluid_P;    
			}
    				
			cfg.headsPerColor = RX_Config.headsPerColor;

			sok_send_2(&_FluidThreadPar[i].socket, CMD_FLUID_CFG, sizeof(cfg), &cfg);
			
			if (i==0) sok_send_2(&_FluidThreadPar[i].socket, CMD_SCALES_SET_CFG, sizeof(RX_Config.scales), &RX_Config.scales);
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

		if  (_HeadStateCnt[i].temp>0)			state[i].temp = _HeadState[i].temp/_HeadStateCnt[i].temp;
		else									state[i].temp = INVALID_VALUE; 
		if  (_HeadStateCnt[i].condPumpSpeed>0 || _HeadPumpSpeed[i][0]>=CALIBRATE_PUMP_SPEED-tol)
		{
			//state[i].condPumpSpeed = _HeadState[i].condPumpSpeed/_HeadStateCnt[i].condPumpSpeed;
			state[i].condPumpSpeed = _HeadState[i].condPumpSpeed;
	//		TrPrintf(TRUE, "PumpSpeed[%d]: min=%d max=%d, avg=%d", i, _HeadPumpSpeed[i][0], _HeadPumpSpeed[i][1], state[i].pumpSpeed);
		}
		else 
		{
			state[i].condPumpSpeed = INVALID_VALUE;
		}
			
		if  (_HeadStateCnt[i].condPumpFeedback > 0)
			state[i].condPumpFeedback = _HeadState[i].condPumpFeedback / _HeadStateCnt[i].condPumpFeedback;
		else 
			state[i].condPumpFeedback = INVALID_VALUE;
		
		if (!chiller_is_running() &&  _FluidStatus[i].ctrlMode>ctrl_off)
			_send_ctrlMode(i, ctrl_off, TRUE);
					
		state[i].canisterEmpty = (_FluidStatus[i].canisterErr >= LOG_TYPE_ERROR_CONT);
	}
	
	memset(_HeadStateCnt,  0, sizeof(_HeadStateCnt));
	memset(_HeadState,     0, sizeof(_HeadState));
	memset(_HeadPumpSpeed, 0, sizeof(_HeadPumpSpeed));

	for (i=0; i<SIZEOF(_FluidThreadPar); i++)
	{
		if (_FluidThreadPar[i].socket!=INVALID_SOCKET) 
		{
			if (_FluidThreadPar[i].aliveTime && _FluidThreadPar[i].aliveTime + TIMEOUT < time)
			{
				sok_close(&_FluidThreadPar[i].socket);
				ErrorEx(dev_fluid, i, LOG, 0, "Connection lost");
			}
			else
				sok_send_2(&_FluidThreadPar[i].socket, CMD_FLUID_STAT, INK_PER_BOARD*sizeof(state[0]), &state[i*INK_PER_BOARD]);
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
			case REP_SCALES_STAT:	 _do_scales_stat	((SScalesStatMsg*)msg);						break;
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
	
	memcpy(&_FluidStatus[fluidNo*INK_PER_BOARD], &pstat->stat[0], INK_PER_BOARD*sizeof(_FluidStatus[0]));
	if      (_FluidCtrlMode>=ctrl_flush_night && _FluidCtrlMode<=ctrl_flush_done) _control_flush();
//	else if (_RobotCtrlMode>=ctrl_wipe        && _RobotCtrlMode<ctrl_fill       && fluidNo==0) _control_robot();
	else _control(fluidNo);

	//--- update overall state --------------------------
	SPrinterStatus stat;
	int i;
	memcpy(&stat, &RX_PrinterStatus, sizeof(stat));
	stat.inkSupilesOff = TRUE;
	stat.inkSupilesOn  = TRUE;
	for (i=0; i<RX_Config.inkSupplyCnt; i++)
	{
		if (RX_Config.inkSupply[i].inkFileName[0]) 
		{
			if (_FluidStatus[i].ctrlMode<=ctrl_off) stat.inkSupilesOn =FALSE;
			if (_FluidStatus[i].ctrlMode>ctrl_off)  stat.inkSupilesOff=FALSE;
		}
	}
	if (stat.printState==ps_off) stat.printState = ps_ready_power;
	if (memcmp(&stat, &RX_PrinterStatus, sizeof(stat)))
	{
		memcpy(&RX_PrinterStatus, &stat, sizeof(stat));
		gui_send_printer_status(&RX_PrinterStatus);
	}
}

//--- _do_scale_stat -------------------------------------------------------
static void _do_scales_stat(SScalesStatMsg *pstat)
{
	int i, isno;
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
	SRxConfig cfg;
	setup_config(PATH_USER FILENAME_CFG, &cfg, READ);
	memcpy(cfg.scales.tara,  pmsg->tara,  sizeof(cfg.scales.tara));
	memcpy(cfg.scales.calib, pmsg->calib, sizeof(cfg.scales.calib));
	setup_config(PATH_USER FILENAME_CFG, &cfg, WRITE);
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

//--- _control -------------------------------------------------
static void _control(int fluidNo)
{
	int i;
	int no = fluidNo*INK_PER_BOARD;
	SInkSupplyStat *_stat = &_FluidStatus[no];

	for (i=0; i<INK_PER_BOARD; i++, _stat++, no++)
	{
		if (ctrl_check_all_heads_in_fluidCtrlMode(no, _stat->ctrlMode))
		{
	//		Error(LOG, 0, "Fluid[%d] in mode >>%s<<", no, FluidCtrlModeStr(_stat->ctrlMode));
			switch(_stat->ctrlMode)
			{
				case ctrl_check_step0:	_send_ctrlMode(no, ctrl_off, TRUE);	break;
			//	case ctrl_check_step0:	_send_ctrlMode(no, ctrl_print_step1, TRUE);	break;
			//	case ctrl_check_step1:	_send_ctrlMode(no, ctrl_print_step2, TRUE);	break;
			//	case ctrl_check_step2:	_send_ctrlMode(no, ctrl_print_step3, TRUE);	break;
			//	case ctrl_check_step3:	_send_ctrlMode(no, ctrl_print_step4, TRUE);	break;
			//	case ctrl_check_step4:	_send_ctrlMode(no, ctrl_print_step5, TRUE);	break;
			//	case ctrl_check_step5:	_send_ctrlMode(no, ctrl_print_step6, TRUE);	break;
			//	case ctrl_check_step6:	_send_ctrlMode(no, ctrl_print_step7, TRUE);	break;
			//	case ctrl_check_step7:	_send_ctrlMode(no, ctrl_print_step8, TRUE);	break;
			//	case ctrl_check_step8:	_send_ctrlMode(no, ctrl_print_step9, TRUE);	break;
			//	case ctrl_check_step9:	_send_ctrlMode(no, ctrl_print_run,   TRUE);	break;
				
				case ctrl_purge:
				case ctrl_purge_micro:	
				case ctrl_purge_soft:
				case ctrl_purge_hard:		step_lift_to_up_pos();
											// step_handle_gui_msg(INVALID_SOCKET, CMD_CAP_UP_POS, NULL, 0);
											_PurgeCtrlMode = _stat->ctrlMode;
											step_handle_gui_msg(INVALID_SOCKET, CMD_CAP_REFERENCE, NULL, 0);
											_send_ctrlMode(no, ctrl_purge_step1, TRUE);	
											break;
				
				case ctrl_purge_step1:		if (step_lift_in_up_pos())
											{
												plc_to_purge_pos();
												_send_ctrlMode(no, ctrl_purge_step2, TRUE);																										
											}
											break;
								
				case ctrl_purge_step2:		if (plc_in_purge_pos())
											{
												_send_ctrlMode(no, ctrl_purge_step3, TRUE);												
											}
											break;
				case ctrl_purge_step3:		_send_ctrlMode(no, ctrl_purge_step4, TRUE);
											break;
				case ctrl_purge_step4:		if (_PurgeCtrlMode==ctrl_purge_hard)
											{
												_Flushed &= ~(0x01<<no);
												setup_fluid_system(PATH_USER FILENAME_FLUID_STATE, &_Flushed, WRITE);				
											}

											if (rx_def_is_tx(RX_Config.printer.type) && step_active(1) && _PurgeFluidNo<0) 
											{
												if(_all_fluids_in_fluidCtrlMode(ctrl_purge_step4)) _send_ctrlMode(no, ctrl_wipe, TRUE);														
											}
 											else if (RX_PrinterStatus.printState==ps_pause) _send_ctrlMode(no, ctrl_print, TRUE);
											else _send_ctrlMode(no, ctrl_off, TRUE);		
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
				
				//--- ctrl_off ---------------------------------------------------------------------
				case ctrl_off:				//step_rob_stop();
											//step_lift_stop();
					break;				
			}
		}
	}
}

//--- _control_flush -------------------------------------------------
static void _control_flush(void)
{
	if (_all_fluids_in_fluidCtrlMode(_FluidCtrlMode))
	{
	//	TrPrintfL(TRUE, "All Fluids in mode %d", _FluidCtrlMode);
	
		switch(_FluidCtrlMode)
		{
		case ctrl_flush_night:		
		case ctrl_flush_weekend:		
		case ctrl_flush_week:	step_lift_to_up_pos();	
								_FluidCtrlMode=ctrl_flush_step1; 
								break;
		
		case ctrl_flush_step1:	if (step_lift_in_up_pos()) 
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
								break;
		
		case ctrl_flush_done:	ErrorEx(dev_fluid, -1, LOG, 0, "Flush complete");
								_FluidCtrlMode=ctrl_off; 
								break; // send to all
		default: break;		
		}
		
		for (int i=0; i<RX_Config.inkSupplyCnt; i++) _send_ctrlMode(i, _FluidCtrlMode, TRUE);

	//	TrPrintfL(TRUE, "Next mode %d", _FluidCtrlMode);
	}
}								

//--- fluid_control_robot -------------------------------------------------
void fluid_control_robot(void)
{
	static int	_printing;		

	if (_all_fluids_in_fluidCtrlMode(_RobotCtrlMode))
	{
		EnFluidCtrlMode	old =  _RobotCtrlMode;
		
		switch(_RobotCtrlMode)
		{
		//--- ctrl_wetwipe --------------------------------------------------------------------------------------
		case ctrl_wetwipe:			step_lift_to_up_pos();
									_RobotCtrlMode = ctrl_wetwipe_step1;
									break;
				
		case ctrl_wetwipe_step1:	if (step_lift_in_up_pos())
									{
										if (!step_rob_reference_done()) step_rob_do_reference();
										plc_to_wipe_pos();
										_RobotCtrlMode=ctrl_wetwipe_step2;
									}
									break;
			
		case ctrl_wetwipe_step2:	if (step_rob_reference_done())
									{
										step_rob_to_wipe_pos(rob_fct_wetwipe);
										_RobotCtrlMode=ctrl_wetwipe_step3;
									}
									break;
			
		case ctrl_wetwipe_step3:	if (plc_in_wipe_pos() && step_rob_in_wipe_pos(rob_fct_wetwipe))
									{
										step_lift_to_wipe_pos(ctrl_wetwipe);
										_RobotCtrlMode=ctrl_wetwipe_step4;
									}
									break;
				
		case ctrl_wetwipe_step4:	if (step_lift_in_wipe_pos(ctrl_wetwipe))
									{
										step_rob_wipe_start(ctrl_wetwipe);
										_RobotCtrlMode=ctrl_wetwipe_step5;
									}
									break;
				
		case ctrl_wetwipe_step5:	if (step_rob_wipe_done(ctrl_wetwipe))
									{
										step_lift_to_up_pos();
										//step_rob_to_center_pos();
										_RobotCtrlMode=ctrl_wetwipe_step6;
									}
									break;
				
		case ctrl_wetwipe_step6:	if (step_lift_in_up_pos())
									{
										_RobotCtrlMode = ctrl_wipe;
									}
									break;
				
		//--- ctrl_wipe -------------------------------------------------------------------------------------
		case ctrl_wipe:						
		case ctrl_wash:				step_lift_to_up_pos();
									_RobotCtrlMode = ctrl_wipe_step1;
									break;
				
		case ctrl_wipe_step1:		if (step_lift_in_up_pos())
									{
										if (!step_rob_reference_done()) step_rob_do_reference();
										plc_to_wipe_pos();
										_RobotCtrlMode=ctrl_wipe_step2;
									}
									break;
			
		case ctrl_wipe_step2:		if (step_rob_reference_done())
									{
										step_rob_to_wipe_pos(rob_fct_wipe);
										_RobotCtrlMode=ctrl_wipe_step3;										
									}
									break;
				
		case ctrl_wipe_step3:		if (plc_in_wipe_pos() && step_rob_in_wipe_pos(rob_fct_wipe))
									{
										step_lift_to_wipe_pos(ctrl_wipe);
										_RobotCtrlMode=ctrl_wipe_step4;
									}
									break;
				
		case ctrl_wipe_step4:		if (step_lift_in_wipe_pos(ctrl_wipe))
									{
										step_rob_wipe_start(ctrl_wipe);
										_RobotCtrlMode=ctrl_wipe_step5;
									}
									break;

		case ctrl_wipe_step5:		if (step_rob_wipe_done(ctrl_wipe))
									{
										step_lift_to_up_pos();
										_RobotCtrlMode=ctrl_wipe_step6;
									}
									break;

		case ctrl_wipe_step6:		if (step_lift_in_up_pos())
									{
										_RobotCtrlMode = ctrl_vacuum;
									}
									break;
				
		//--- ctrl_vacuum ----------------------------------------------------
		case ctrl_vacuum:			step_lift_to_up_pos();
									_RobotCtrlMode = ctrl_vacuum_step1;
									break;
				
		case ctrl_vacuum_step1:		if (step_lift_in_up_pos())
									{
										if (!step_rob_reference_done()) step_rob_do_reference();
										plc_to_wipe_pos();
										_RobotCtrlMode=ctrl_vacuum_step2;
									}
									break;
		case ctrl_vacuum_step2:		if (step_rob_reference_done())
									{
										step_rob_to_wipe_pos(rob_fct_vacuum);
										_RobotCtrlMode=ctrl_vacuum_step3;										
									}
									break;
		case ctrl_vacuum_step3:		if (plc_in_wipe_pos() && step_rob_in_wipe_pos(rob_fct_vacuum))
									{
										step_lift_to_wipe_pos(ctrl_vacuum);
										_RobotCtrlMode=ctrl_vacuum_step4;
									}
									break;
				
		case ctrl_vacuum_step4:		if (step_lift_in_wipe_pos(ctrl_vacuum))
									{
										step_rob_wipe_start(ctrl_vacuum);
										_RobotCtrlMode=ctrl_vacuum_step5;
									}
									break;
				
		case ctrl_vacuum_step5:		if (step_rob_wipe_done(ctrl_vacuum))
									{
										step_lift_to_up_pos();
										_RobotCtrlMode=ctrl_vacuum_step6;
									}
									break;
				
		case ctrl_vacuum_step6:		if (step_lift_in_up_pos())
									{
										step_rob_to_wipe_pos(rob_fct_vacuum_change);
										_RobotCtrlMode=ctrl_vacuum_step7;
									}
									break;
				
		case ctrl_vacuum_step7:		if (step_rob_in_wipe_pos(rob_fct_vacuum_change))
									{
										step_rob_to_wipe_pos(rob_fct_vacuum);
										_RobotCtrlMode=ctrl_vacuum_step8;
									}
									break;
				
		case ctrl_vacuum_step8:		if (step_rob_in_wipe_pos(rob_fct_vacuum))
									{
										step_lift_to_wipe_pos(ctrl_vacuum);
										_RobotCtrlMode=ctrl_vacuum_step9;
									}
									break;
				
		case ctrl_vacuum_step9:		if (step_lift_in_wipe_pos(ctrl_vacuum))
									{
										step_rob_wipe_start(ctrl_vacuum);
										_RobotCtrlMode=ctrl_vacuum_step10;
									}
									break;
				
		case ctrl_vacuum_step10:	if (step_rob_wipe_done(ctrl_vacuum))
									{
										_printing = (RX_PrinterStatus.printState==ps_pause);
										if (_printing) step_lift_to_print_pos();
										else		  step_lift_to_up_pos();
										_RobotCtrlMode=ctrl_vacuum_step11;
									}
									break;
				
		case ctrl_vacuum_step11:	if ((_printing && step_lift_in_print_pos()) || (!_printing && step_lift_in_up_pos()))
									{
										Error(LOG, 0, "ctrl_vacuum_step10 printState=%d", RX_PrinterStatus.printState);
										if (_printing) _RobotCtrlMode = ctrl_print;
										else		   _RobotCtrlMode = ctrl_off;
									}
									break;
				
		//--- ctrl_cap -----------------------------------------------------------------------------
		case ctrl_cap:				step_lift_to_up_pos();
									_RobotCtrlMode=ctrl_cap_step1;
									break;
				
		case ctrl_cap_step1:		if (step_lift_in_up_pos())
									{
										if (!step_rob_reference_done()) step_rob_do_reference();
										plc_to_purge_pos();
										_RobotCtrlMode=ctrl_cap_step2;
									}
									break;	 
				
		case ctrl_cap_step2:		if (plc_in_purge_pos() && step_rob_reference_done()) 
									{
										step_rob_to_wipe_pos(rob_fct_cap);
										_RobotCtrlMode=ctrl_cap_step3;
									}
									break;
			
		case ctrl_cap_step3:		if (step_rob_in_wipe_pos(rob_fct_cap))
									{
										plc_to_wipe_pos();
										_RobotCtrlMode=ctrl_cap_step4;
									}
									break;		
			
		case ctrl_cap_step4:		if (plc_in_wipe_pos())
									{
										step_lift_to_wipe_pos(ctrl_cap);
										_RobotCtrlMode=ctrl_cap_step5;
									}
									break;
			
		case ctrl_cap_step5:		if (step_lift_in_wipe_pos(ctrl_cap))
									{
										_RobotCtrlMode = ctrl_off;
									}
									break;
		default: return;
		}
		if (_RobotCtrlMode!=old)
			_send_ctrlMode(-1, _RobotCtrlMode, TRUE);
	}
}	

//--- fluid_reply_stat ------------------------------------
void fluid_reply_stat(RX_SOCKET socket)	// to GUI
{
	int i;
	int	canisterLow;
	int	canisterEmpty;
	switch (RX_Config.printer.type)
	{
	case printer_TX801:
	case printer_TX802:	canisterLow   = 1500;
						canisterEmpty = 500;
						break;
	default:			canisterLow   = 1500;
						canisterEmpty = 500;
	}

	for (i=0; i<SIZEOF(_FluidStatus); i++)
	{
		if (i<INK_SUPPLY_CNT)
		{
			_FluidStatus[i].info.connected = (_FluidThreadPar[i/INK_PER_BOARD].socket!=INVALID_SOCKET);
			_FluidStatus[i].info.flushed   = ((_Flushed & (0x01<<i))!=0);
		}

		_FluidStatus[i].canisterLevel  = _ScalesStatus[_FluidToScales[i]];

		if(_ScalesErr[i] != LOG_TYPE_UNDEF)
		{
			if(_FluidStatus[i].canisterLevel <= canisterEmpty && _ScalesErr[i] < LOG_TYPE_ERROR_CONT)
			{
				Error(ERR_CONT, 0, "Ink Canister %s EMPTY!", RX_ColorNameLong(RX_Config.inkSupply[i].ink.colorCode));
				_ScalesErr[i] = LOG_TYPE_ERROR_CONT;
			}
			else if(_FluidStatus[i].canisterLevel <= canisterLow && _ScalesErr[i] < LOG_TYPE_WARN)
			{
				Error(WARN, 0, "Ink Canister %s LOW!", RX_ColorNameLong(RX_Config.inkSupply[i].ink.colorCode));
				_ScalesErr[i] = LOG_TYPE_WARN;
			}
			_FluidStatus[i].canisterErr = _ScalesErr[i];		
		}
		else if (_FluidStatus[i].canisterLevel>0 && _FluidStatus[i].canisterLevel<50000)
		{
			_ScalesErr[i] = LOG_TYPE_LOG;
		}
		
		dl_get_barcode(i, _FluidStatus[i].scannerSN, _FluidStatus[i].barcode);
		if (_HeadErr[i]) _FluidStatus[i].err |= err_printhead; 
		_HeadErr[i]=0;
	}

//	sok_send_2(&socket, REP_FLUID_STAT, 10*sizeof(SInkSupplyStat), _FluidStatus);
	{
		SInkSupplyStatMsg msg;
		msg.hdr.msgId  = REP_FLUID_STAT;
		msg.hdr.msgLen = sizeof(msg);
		
//		for (msg.no=0; msg.no<RX_Config.inkSupplyCnt; msg.no++)
		for (msg.no=0; msg.no<SIZEOF(_FluidStatus); msg.no++)
		{
			if (msg.no<RX_Config.inkSupplyCnt || msg.no>=INK_SUPPLY_CNT)
			{
				memcpy(&msg.stat, &_FluidStatus[msg.no], sizeof(msg.stat));
				sok_send(&socket, &msg);					
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
	if (ctrlMode==ctrl_purge_hard) _PurgeFluidNo=no;
		
	_FluidCtrlMode = ctrlMode;
//	Error(LOG, 0, "fluid_send_ctrlMode 0X%04x", ctrlMode);
	_RobotCtrlMode = ctrlMode;
	_send_ctrlMode(no, ctrlMode, sendToHeads);
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
				if (i==no) cmd.ctrlMode = ctrlMode;
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

//--- fluid_send_pressure -------------------------------------------
void fluid_send_pressure(int no, INT32 pressure)
{
	RX_Config.inkSupply[no].cylinderPresSet = pressure;
	fluid_set_config();
}

//--- fluid_send_tara -------------------------------------------
void fluid_send_tara(int no)
{
	int i=0;
	no = _FluidToScales[no];
	sok_send_2(&_FluidThreadPar[i].socket, CMD_SCALES_TARA, sizeof(no), &no);
}

//--- fluid_send_calib -------------------------------------------
void fluid_send_calib(SValue *pmsg)
{
	int i=0;
	sok_send_2(&_FluidThreadPar[i].socket, CMD_SCALES_CALIBRATE, sizeof(*pmsg), pmsg);
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
			_HeadState[no].temp += pstat->tempHead;
			_HeadStateCnt[no].temp++; 
		}		
		if (pstat->pumpSpeed)
		{
			// set min/max
			if ((int)pstat->pumpSpeed < _HeadPumpSpeed[no][0] || !_HeadPumpSpeed[no][0]) _HeadPumpSpeed[no][0] = pstat->pumpSpeed;
			if ((int)pstat->pumpSpeed > _HeadPumpSpeed[no][1])							 _HeadPumpSpeed[no][1] = pstat->pumpSpeed;

			_HeadState[no].condPumpSpeed += pstat->pumpSpeed;
			_HeadStateCnt[no].condPumpSpeed++; 
		}
		if (pstat->pumpFeedback)
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
INT32 fluid_get_amcTemp(int no)
{
	if (no >= 0 && no < SIZEOF(_FluidStatus))
	{
		return _FluidStatus[no].temp;
	}
	return INVALID_VALUE;
}

//--- fluid_get_error ---------------------------------------
INT32 fluid_get_error(int no)
{
    if (no >= 0 && no < SIZEOF(_FluidStatus))
    {
        return _FluidStatus[no].err;
    }
    return INVALID_VALUE;
}