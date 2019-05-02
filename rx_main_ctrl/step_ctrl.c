// ****************************************************************************
//
//	step_ctrl.c		Test Table Controol
//
// ****************************************************************************
//
//	Copyright 2016 by Radex AG, Switzerland. All rights reserved.
//	Written by Marcel Galliker 
//
// ****************************************************************************

#include "rx_error.h"
#include "rx_def.h"
#include "rx_error.h"
#include "rx_sok.h"
#include "rx_threads.h"
#include "rx_trace.h"
#include "tcp_ip.h"
#include "network.h"
#include "ctrl_msg.h"
#include "print_ctrl.h"
#include "args.h"
#include "plc_ctrl.h"
#include "enc_ctrl.h"
#include "gui_svr.h"
#include "step_std.h"
#include "step_tx.h"
#include "step_lb.h"
#include "step_cleaf.h"
#include "step_test.h"
#include "step_ctrl.h"

//--- statics -----------------------------------------------------------------
#define STEPPER_CNT		4

#define STEPPER_STD		0
#define STEPPER_CLEAF	1
#define STEPPER_TX		2
#define STEPPER_LB		3
#define STEPPER_TEST	4


static int				_step_ThreadRunning;
static RX_SOCKET		_step_Socket[STEPPER_CNT];
static int				_StepperType=STEPPER_STD;
static SPrintQueueItem	_PQItem;
	
//--- prototypes -----------------------
static void* _step_thread		(void *par);
static int   _step_handle_msg	(RX_SOCKET socket, void *msg, int len, struct sockaddr	*sender, void *par);
static int   _setp_socket_closed(RX_SOCKET socket, const char *peerName);
static void  _step_set_config	(int no);


//--- step_init --------------------------------------------------
int	 step_init(void)
{	
	int i;
	_step_ThreadRunning = TRUE;
	memset(&RX_StepperStatus, 0, sizeof(RX_StepperStatus));
	//memset(&RX_ClnStatus, 0, sizeof(RX_ClnStatus));
	for (i=0; i<SIZEOF(_step_Socket); i++)
	{
		_step_Socket[i]=INVALID_SOCKET;
	}
	rx_thread_start(_step_thread, NULL, 0, "_step_thread");
	return REPLY_OK;
}

//--- step_end ---------------------------------------------------
int  step_end(void)
{
	_step_ThreadRunning = FALSE;
	return REPLY_OK;
}

//--- step_tick ------------------------------------------
void step_tick(void)
{
	int i;
	for (i=0; i<SIZEOF(_step_Socket); i++)
	{
		if (_step_Socket[i]!=INVALID_SOCKET) 
			sok_send_2(&_step_Socket[i], CMD_TT_STATUS, 0, NULL);			
	}	
}

//--- _step_thread ---------------------------------------------
static void* _step_thread(void *par)
{
	while (_step_ThreadRunning)
	{
		int i;
		char addr[32];
		for (i=0; i<SIZEOF(_step_Socket); i++)
		{
			if (_step_Socket[i]==INVALID_SOCKET && net_port_listening(dev_stepper, i, PORT_CTRL_STEPPER))
			{
				net_device_to_ipaddr(dev_stepper, i, addr, sizeof(addr));
				if (sok_open_client_2(&_step_Socket[i], addr, PORT_CTRL_STEPPER, SOCK_STREAM, _step_handle_msg, _setp_socket_closed)== REPLY_OK)
				{
					_step_set_config(i);
				}			
			}
		}
		rx_sleep(1000);
	}
	return NULL;
}

//--- _setp_socket_closed --------------------------------------------------
static int _setp_socket_closed(RX_SOCKET socket, const char *peerName)
{
	int i;
	for (i=0; i<SIZEOF(_step_Socket); i++)
	{
		if (socket==_step_Socket[i])
		{
			Error(ERR_CONT, 0, "Stepper %d Socket %d closed", i, socket);
			sok_close(&_step_Socket[i]);
			memset(&RX_StepperStatus, 0, sizeof(RX_StepperStatus));
			return REPLY_OK;				
		}
	}
	return REPLY_OK;
}

//--- _step_handle_msg -------------------------------------------------------
static int _step_handle_msg(RX_SOCKET socket, void *msg, int len, struct sockaddr *sender, void *par)
{
	SMsgHdr			*phdr = (SMsgHdr*)msg;
	int no;
	
	for(no=0; no<SIZEOF(_step_Socket); no++)
	{
		if (socket==_step_Socket[no])			
		{
			net_register_by_device(dev_stepper, no);

			switch (phdr->msgId)
			{
				case EVT_GET_EVT:	if (!arg_simuPLC) SlaveError(dev_stepper, no, &((SLogMsg*)msg)->log);	break;
				case CMD_PRINT_ABORT:pc_abort_printing(); break;
				case REP_TT_STATUS:	switch(_StepperType)
									{
									case STEPPER_CLEAF: return stepc_handle_status		(no, (SStepperStat*)&phdr[1]);
									case STEPPER_TX:	return steptx_handle_status		(no, (SStepperStat*)&phdr[1]);
									case STEPPER_LB:	return steplb_handle_status		(no, (SStepperStat*)&phdr[1]);
									case STEPPER_TEST:	return steptest_handle_status	(no, (SStepperStat*)&phdr[1]);
									default:			return steps_handle_status		(	 (SStepperStat*)&phdr[1]);
									}
									break;
			}
		}
	}
	return REPLY_OK;
}

//--- step_handle_gui_msg------------------------------------------------------------------
int	 step_handle_gui_msg(RX_SOCKET socket, UINT32 cmd, void *data, int dataLen)
{	
	TrPrintfL(TRUE, "step_handle_gui_msg(_StepperType=%d, cmd=%d)", _StepperType, cmd);
	switch(_StepperType)
	{
	case STEPPER_CLEAF:	return stepc_handle_gui_msg   (socket, cmd, data, dataLen);
	case STEPPER_TX:	return steptx_handle_gui_msg  (socket, cmd, data, dataLen);
	case STEPPER_LB:	return steplb_handle_gui_msg  (socket, cmd, data, dataLen);
	case STEPPER_TEST:	return steptest_handle_gui_msg(socket, cmd, data, dataLen);
	default:			return steps_handle_gui_msg   (socket, cmd, data, dataLen);
	}
}

/*
//--- step_slide_enabled -----------------
int	 step_slide_enabled(void)
{
	switch(_StepperType)
	{
	case STEPPER_TX:	return steptx_slide_enabled();
	default:			return TRUE;
	}															
}
*/

//--- step_to_purge_pos ----------------------------------
int	 step_to_purge_pos(int no)
{
	switch(_StepperType)
	{
	case STEPPER_TX:	return steptx_to_purge_pos(no);
//	case STEPPER_LB:	return steplb_to_purge_pos(no);
//	case STEPPER_TEST:  return steptest_to_purge_pos(no);
	default:			return REPLY_ERROR;
	}
}
														
//--- step_in_purge_pos -------------------------------------
int  step_in_purge_pos(void)
{
	switch(_StepperType)
	{
	case STEPPER_TX:	return steptx_in_purge_pos();
//	case STEPPER_LB:	return steplb_in_purge_pos();
//	case STEPPER_TEST:	return steptest_in_purge_pos();
	default:			return TRUE;
	}
}


//--- step_wipe_start ------------------------------------------------
int  step_wipe_start(int no)
{
	switch(_StepperType)
	{
	case STEPPER_TX:	return steptx_wipe_start(no);
	default:			return REPLY_ERROR;
	}
}

//--- step_wipe_done ------------------------------------------------
int  step_wipe_done(void)
{
	switch(_StepperType)
	{
	case STEPPER_TX:	return steptx_wipe_done();
	default:			return TRUE;
	}
}

//--- tt_cap_to_print_pos --------------------------------
int	 tt_cap_to_print_pos(void)
{
	switch(_StepperType)
	{
	case STEPPER_CLEAF: stepc_to_print_pos();		break;
	case STEPPER_TX:    steptx_to_print_pos();		break;
	case STEPPER_LB:    steplb_to_print_pos();		break;
	case STEPPER_TEST:  steptest_to_print_pos();	break;
	default:			steps_to_print_pos();		break;
	}
	return REPLY_OK;									
}

//--- step_do_test -------------------------------
int	 step_do_test(SStepperMotorTest *pmsg)
{
	int no = pmsg->boardNo;
	if (no>=0 && no<STEPPER_CNT && _step_Socket[no]!=INVALID_SOCKET)
	{
		sok_send_2(&_step_Socket[no], CMD_STEPPER_TEST, sizeof(*pmsg), pmsg);
	}
	return REPLY_OK;
}

//--- step_set_vent -------------------------------------------------------
int step_set_vent(int speed)
{
	INT32 value;
//	if(speed) value=25;	// controlled PWM
	if(speed) value=60;	// fixed PWM
	else      value=0;
	sok_send_2(&_step_Socket[0], CMD_CAP_VENT, sizeof(value), &value);
	return REPLY_OK;			
}

//--- step_handle_gui_msg------------------------------------------------------------------
int  step_abort_printing(void)
{
	switch(_StepperType)
	{
	case STEPPER_CLEAF:	return stepc_abort_printing();
	case STEPPER_TX:    step_set_vent(0);
						return REPLY_OK;
	case STEPPER_LB:    return steplb_abort_printing();
	case STEPPER_TEST:  return REPLY_OK;
	default:			return REPLY_OK;
	}					
}

//--- _step_set_config -------------------------------------------------
static void _step_set_config(int no)				
{
	SStepperCfg cfg;

	memcpy(&cfg, &RX_Config.stepper, sizeof(cfg));
	cfg.printerType = RX_Config.printer.type;
	cfg.use_printhead_en = arg_simuPLC;
	cfg.boardNo=0;
		
	if (RX_Config.printer.type==printer_DP803 && !strcmp(RX_Hostname, "LB701-0001"))
	{
		Error(WARN, 0, "TEST for LB701-0001");
		cfg.printerType = printer_LB701;
	}
	switch(_StepperType)
	{
	case STEPPER_CLEAF:	stepc_init		(no, &_step_Socket[no]); cfg.boardNo = no; break;
	case STEPPER_TX:	steptx_init		(no, &_step_Socket[no]); cfg.boardNo = no; break;
	case STEPPER_LB:	steplb_init		(no, &_step_Socket[no]); break;
	case STEPPER_TEST:	steptest_init	(no, &_step_Socket[no]); cfg.boardNo = no; break;
	default: 			steps_init		(    &_step_Socket[0]);
	}

	sok_send_2(&_step_Socket[no], CMD_STEPPER_CFG, sizeof(cfg), &cfg);
}

//--- step_set_config -------------------------------------------------
int step_set_config(void)				
{
	int i;
	switch(RX_Config.printer.type)
	{
	case printer_cleaf:			_StepperType = STEPPER_CLEAF;	break;
	case printer_LB701:			_StepperType = STEPPER_LB;		break;		
	case printer_LB702_UV:		_StepperType = STEPPER_LB;		break;		
	case printer_LB702_WB:		_StepperType = STEPPER_LB;		break;		
	case printer_DP803:			_StepperType = STEPPER_LB;		break;		
	case printer_TX801:			_StepperType = STEPPER_TX;		break;		
	case printer_TX802:			_StepperType = STEPPER_TX;		break;		
	default:					_StepperType = STEPPER_STD;		break;
	}

	for(i=0; i<SIZEOF(_step_Socket); i++)
		if (_step_Socket[i]!=INVALID_SOCKET) _step_set_config(i);
	return REPLY_OK;
}

//--- tt_get_scanner_pos ----------------------------------------
UINT32 tt_get_scanner_pos(void)
{
	return 0;
}			

//--- tt_set_printpar ---------------------------------------------
int  tt_set_printpar(SPrintQueueItem *pItem)
{	
	memcpy(&_PQItem, pItem, sizeof(_PQItem));
	if (_PQItem.scans<1) _PQItem.scans=1;
	return REPLY_OK;	
}

//--- tt_set_scans ----------------------------------
int  tt_set_scans	   (int scans)
{
	_PQItem.scans = scans;
	return REPLY_OK;
}

//--- tt_start_printing ---------------------------------------------
int  tt_start_printing(void)
{
	enc_start_printing(&_PQItem);
	
	STestTableScanPar par;
	steps_set_curing(FALSE);

	par.speed	= _PQItem.speed;
	par.scanCnt = _PQItem.scans;
	par.scanMode= _PQItem.scanMode;
	par.yStep   = 26000+1730+RX_Config.printer.offset.step;	// Overlap
	if (!RX_Config.printer.overlap) par.yStep -= 1680;
	if (_PQItem.passes>1) par.yStep /= _PQItem.passes;
	par.offsetAngle  = RX_Config.printer.offset.angle;
	par.curingPasses = _PQItem.curingPasses;
	
	if (RX_PrinterStatus.testMode)
	{
		par.scanCnt = _PQItem.scans*_PQItem.copies;
		if (RX_TestImage.testImage==PQ_TEST_JETS || RX_TestImage.testImage==PQ_TEST_ENCODER) par.yStep  += 2500;
	}
	
	sok_send_2(&_step_Socket[0], CMD_TT_SCAN, sizeof(par), &par);
	
	return REPLY_OK;
}

//--- tt_pause_printing ----------------------------------------------
int  tt_pause_printing(void)
{
	return REPLY_OK;
}	

//--- tt_stop_printing --------------------------------------------
int  tt_stop_printing(void)
{
	enc_stop_printing();
	steps_set_curing(TRUE);
	return REPLY_OK;	
}	
			
//--- tt_abort_printing --------------------------------------------
int  tt_abort_printing(void)
{
	enc_stop_printing();
	sok_send_2(&_step_Socket[0], CMD_TT_ABORT, 0, NULL);
	steps_set_curing(FALSE);
	return REPLY_OK;	
}	

//---  tt_clean --------------------------------------------------
int  tt_clean(void)
{
	return REPLY_OK;	
}	

//--- step_error_reset ----------------------------------------
void step_error_reset(void)
{
	sok_send_2(&_step_Socket[0], CMD_ERROR_RESET, 0, NULL);
	switch (RX_Config.printer.type)
	{
	case printer_cleaf: stepc_error_reset();
						sok_send_2(&_step_Socket[1], CMD_ERROR_RESET, 0, NULL); 
						break;
	default:			break;
	}
}

//--- setp_send_ctrlMode ------------------------------------
void setp_send_ctrlMode(EnFluidCtrlMode ctrlMode)
{
	if (_StepperType == STEPPER_TX)
	{
		int i;
		SValue value;
	
		value.no	= 0;
		value.value	= ctrlMode;

		for (i=0; i<SIZEOF(_step_Socket); i++)
		{
			if (_step_Socket[i]!=INVALID_SOCKET) sok_send_2(&_step_Socket[i], CMD_FLUID_CTRL_MODE, sizeof(value), &value); 
		}				
	}
}

//--- step_all_in_ctrlMode ------------
int  step_all_in_ctrlMode(EnFluidCtrlMode ctrlMode)
{
	switch(_StepperType)
	{
	case STEPPER_CLEAF:	return TRUE;
	case STEPPER_TX:	return steptx_all_in_ctrlMode(ctrlMode);
	case STEPPER_LB:	return TRUE;
	case STEPPER_TEST:	return TRUE;
	default: 			return TRUE;	
	}	
}
