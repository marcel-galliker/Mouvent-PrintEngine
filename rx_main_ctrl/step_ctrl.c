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
#include "fluid_ctrl.h"
#include "gui_svr.h"
#include "step_std.h"
#include "step_tx.h"
#include "step_lb.h"
#include "step_dp.h"
#include "step_cleaf.h"
#include "step_test.h"
#include "step_ctrl.h"

//--- statics -----------------------------------------------------------------
#define STEPPER_CNT		4

#define STEPPER_STD		0
#define STEPPER_CLEAF	1
#define STEPPER_TX		2
#define STEPPER_LB		3
#define STEPPER_DP		4
#define STEPPER_TEST	5


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
	for (int i=0; i<SIZEOF(_step_Socket); i++)
	{
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
					ErrorEx(dev_stepper, i, LOG, 0, "Connected");
					_step_set_config(i);
					if (i==1 && (RX_Config.printer.type==printer_TX801 || RX_Config.printer.type==printer_TX802)) 
					{
						RX_PrinterStatus.txRobot = TRUE;
						Error(LOG, 0, "Robot connected");
					}
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
			switch(_StepperType)
			{
			case STEPPER_CLEAF:	stepc_init		(i, INVALID_SOCKET); break;
			case STEPPER_TX:	steptx_init		(i, INVALID_SOCKET); break;
			case STEPPER_LB:	steplb_init		(i, INVALID_SOCKET); break;
			case STEPPER_DP:	stepdp_init		(i, INVALID_SOCKET); break;
			case STEPPER_TEST:	steptest_init	(i, INVALID_SOCKET); break;
			default: 			steps_init		(   INVALID_SOCKET);
			}
			memset(&RX_StepperStatus, 0, sizeof(RX_StepperStatus));
			if (i==1) RX_PrinterStatus.txRobot = FALSE;
			return REPLY_OK;				
		}
	}
	return REPLY_OK;
}

//--- step_active -------------------
int	 step_active(int no)
{
	if (no>=0 && no<SIZEOF(_step_Socket)) return (_step_Socket[no]!=INVALID_SOCKET);
	return FALSE;
}

//--- _step_handle_msg -------------------------------------------------------
static int _step_handle_msg(RX_SOCKET socket, void *msg, int len, struct sockaddr *sender, void *par)
{
	SMsgHdr			*phdr = (SMsgHdr*)msg;
	int no;
	int ret;
	
	for(no=0; no<SIZEOF(_step_Socket); no++)
	{
		if (socket==_step_Socket[no])			
		{
			net_register_by_device(dev_stepper, no);
			
			TrPrintfL(TRUE, "received Stepper[%d].MsgId=0x%08x", no, phdr->msgId);

			switch (phdr->msgId)
			{
				case EVT_GET_EVT:	if (!arg_simuPLC) SlaveError(dev_stepper, no, &((SLogMsg*)msg)->log);	
									return REPLY_OK;
				
				case CMD_PRINT_ABORT:pc_abort_printing(); 
									return REPLY_OK;
				
				case REP_TT_STATUS:	switch(_StepperType)
									{
									case STEPPER_CLEAF: ret = stepc_handle_status		(no, (SStepperStat*)&phdr[1]); break;
									case STEPPER_TX:	ret = steptx_handle_status		(no, (SStepperStat*)&phdr[1]); break;
									case STEPPER_LB:	ret = steplb_handle_status		(no, (SStepperStat*)&phdr[1]); break;
									case STEPPER_DP:	ret = stepdp_handle_status		(no, (SStepperStat*)&phdr[1]); break;
									case STEPPER_TEST:	ret = steptest_handle_status	(no, (SStepperStat*)&phdr[1]); break;
									default:			ret = steps_handle_status		(	 (SStepperStat*)&phdr[1]); break;
									}
									fluid_control_robot();
									return ret;
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
	case STEPPER_DP:	return stepdp_handle_gui_msg  (socket, cmd, data, dataLen);
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

//--- step_lift_to_wipe_pos ----------------------------------
void step_lift_to_wipe_pos(EnFluidCtrlMode mode)
{
	switch (_StepperType)
	{
	case STEPPER_TX:	steptx_lift_to_wipe_pos(mode); break;
	default:			break;
	}
}														

//--- step_lift_in_wipe_pos -------------------------------------
int  step_lift_in_wipe_pos(EnFluidCtrlMode mode)
{
	switch (_StepperType)
	{
	case STEPPER_TX:	return steptx_lift_in_wipe_pos(mode);
	default:			return TRUE;
	}
}

//--- step_lift_to_print_pos ----------------------------------
void step_lift_to_print_pos(void)
{
	switch(_StepperType)
	{
	case STEPPER_TX:	steptx_lift_to_print_pos();	break;
	default:			break;
	}
}														

//--- step_lift_in_print_pos -------------------------------------
int  step_lift_in_print_pos(void)
{
	switch(_StepperType)
	{
	case STEPPER_TX:	return steptx_lift_in_print_pos();
	default:			return TRUE;
	}
}

//--- step_lift_to_up_pos ----------------------------------------------
void  step_lift_to_up_pos(void)
{
	switch(_StepperType)
	{
	case STEPPER_TX:	steptx_lift_to_up_pos(); break;
	case STEPPER_LB:	steplb_lift_to_up_pos(); break;
	default:			break;
	}
}

//--- step_lift_in_up_pos ----------------------------------------------
int  step_lift_in_up_pos(void)
{
	switch(_StepperType)
	{
	case STEPPER_TX:	return steptx_lift_in_up_pos();
	case STEPPER_LB:	return steplb_lift_in_up_pos();
	default:			return TRUE;
	}
}

//--- step_lift_stop -----------------------------------------------------
void step_lift_stop(void)
{
	switch (_StepperType)
	{
	case STEPPER_TX:	steptx_lift_stop(); break;
	default: break;
	}
}

//--- step_rob_reference_done -------------------------------
int step_rob_reference_done(void)
{
	switch (_StepperType)
	{
	case STEPPER_TX:	return steptx_rob_reference_done(); break;
	default:			break;
	}
	return REPLY_OK;
}

//--- step_rob_to_center_position ---------------------------
void step_rob_to_center_pos(void)
{
	switch (_StepperType)
	{
	case STEPPER_TX: steptx_rob_to_center_pos(); break;
	default: break;
	}

}

//--- step_rob_do_reference ---------------------------------
void step_rob_do_reference(void)
{
	switch (_StepperType)
	{
	case STEPPER_TX:	 steptx_rob_do_reference(); break;
	default:			break;
	}
}

//--- step_rob_to_wipe_pos ----------------------------------
void step_rob_to_wipe_pos(ERobotFunctions rob_function)
{
	switch(_StepperType)
	{
	case STEPPER_TX:	steptx_rob_to_wipe_pos(rob_function); break;
	default:			break;
	}	
}

//--- step_rob_in_wipe_pos ----------------------------------
int step_rob_in_wipe_pos(ERobotFunctions rob_function)
{
	switch(_StepperType)
	{
	case STEPPER_TX:	return steptx_rob_in_wipe_pos(rob_function);
	default:			break;
	}
	return FALSE;
}

//--- step_rob_wipe_start ------------------------------------------------
void step_rob_wipe_start(EnFluidCtrlMode mode)
{
	switch(_StepperType)
	{
	case STEPPER_TX:	steptx_rob_wipe_start(mode); break;
	default:			break;
	}
}

//--- step_rob_wipe_done ------------------------------------------------
int  step_rob_wipe_done(EnFluidCtrlMode mode)
{
	switch(_StepperType)
	{
	case STEPPER_TX:	return steptx_rob_wipe_done();
	default:			return TRUE;
	}
}
//--- step_rob_stop -----------------------------------------------------
void step_rob_stop(void)
{
	switch (_StepperType)
	{
	case STEPPER_TX:	steptx_rob_stop(); break;
	default: break;
	}
}

//--- tt_cap_to_print_pos --------------------------------
int	 tt_cap_to_print_pos(void)
{
	switch(_StepperType)
	{
	case STEPPER_CLEAF: stepc_to_print_pos();		break;
	case STEPPER_TX:    steptx_lift_to_print_pos();	break;
	case STEPPER_LB:    steplb_to_print_pos();		break;
	case STEPPER_DP:    stepdp_to_print_pos();		break;
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
void step_set_vent(int speed)
{
	INT32 value;
	if(speed) value=20;
	else      value=0;
	sok_send_2(&_step_Socket[0], CMD_CAP_VENT, sizeof(value), &value);
}

//--- step_handle_gui_msg------------------------------------------------------------------
void  step_abort_printing(void)
{
	switch(_StepperType)
	{
	case STEPPER_CLEAF:	stepc_abort_printing();		break;
	case STEPPER_TX:    step_set_vent(0);			break;
	case STEPPER_LB:    steplb_abort_printing();	break;
	case STEPPER_DP:    stepdp_abort_printing();	break;
	case STEPPER_TEST:  break;
	default:			break;
	}					
}

//--- _step_set_config -------------------------------------------------
static void _step_set_config(int no)				
{
	SStepperCfg cfg;

	memcpy(&cfg, &RX_Config.stepper, sizeof(cfg));
	cfg.printerType		   = RX_Config.printer.type;
	cfg.use_printhead_en   = RX_Config.printer.type==printer_LH702;
	cfg.material_thickness = plc_get_thickness();
	cfg.boardNo=0;
		
	if (RX_Config.printer.type==printer_DP803 && !strcmp(RX_Hostname, "LB701-0001"))
	{
		Error(WARN, 0, "TEST for LB701-0001");
		cfg.printerType = printer_LB701;
	}
	switch(_StepperType)
	{
	case STEPPER_CLEAF:	stepc_init		(no, _step_Socket[no]); cfg.boardNo = no; break;
	case STEPPER_TX:	steptx_init		(no, _step_Socket[no]); cfg.boardNo = no; break;
	case STEPPER_LB:	steplb_init		(no, _step_Socket[no]); break;
	case STEPPER_DP:	stepdp_init		(no, _step_Socket[no]); break;
	case STEPPER_TEST:	steptest_init	(no, _step_Socket[no]); cfg.boardNo = no; break;
	default: 			steps_init		(    _step_Socket[0]);
	}

	sok_send_2(&_step_Socket[no], CMD_STEPPER_CFG, sizeof(cfg), &cfg);
}

//--- step_set_config -------------------------------------------------
int step_set_config(void)				
{
	switch(RX_Config.printer.type)
	{
	case printer_cleaf:			_StepperType = STEPPER_CLEAF;	break;
	case printer_LB701:			_StepperType = STEPPER_LB;		break;		
	case printer_LB702_UV:		_StepperType = STEPPER_LB;		break;		
	case printer_LB702_WB:		_StepperType = STEPPER_LB;		break;		
	case printer_LH702:			_StepperType = STEPPER_LB;		break;		
	case printer_DP803:			_StepperType = STEPPER_DP;		break;		
	case printer_TX801:			_StepperType = STEPPER_TX;		break;		
	case printer_TX802:			_StepperType = STEPPER_TX;		break;		
	default:					_StepperType = STEPPER_STD;		break;
	}

	for(int i=0; i<SIZEOF(_step_Socket); i++)
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
	enc_start_printing(&_PQItem, FALSE);
	
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
