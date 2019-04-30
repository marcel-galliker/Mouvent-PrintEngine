// ****************************************************************************
//
//	gui_msg.cpp
//
// ****************************************************************************
//
//	Copyright 2013 by Radex AG, Switzerland. All rights reserved.
//	Written by Marcel Galliker 
//
// ****************************************************************************

#include "rx_common.h"
#include "rx_error.h"
#include "rx_file.h"
#include "rx_log.h"
#include "rx_sok.h"
#include "rx_setup_ink.h"
#include "rx_threads.h"
#include "rx_trace.h"
#include "tcp_ip.h"
#include "gui_svr.h"
#include "print_queue.h"
#include "print_ctrl.h"
#include "machine_ctrl.h"
#include "plc_ctrl.h"
#include "ctrl_svr.h"
#include "enc_ctrl.h"
#include "fluid_ctrl.h"
#include "network.h"
#include "setup.h"
#include "chiller.h"
#include "rx_setup_file.h"
#include "pe_main.h"
#include "rip_clnt.h"
#include "step_ctrl.h"
#include "boot_svr.h"
#include "cleaf_orders.h"
#include "gui_msg.h"

//--- prototypes ---------------------------------------------------

static void _check_format(char *function, void *phdr, int size);

static void _do_ping			(RX_SOCKET socket);
static void _do_status			(RX_SOCKET socket);
static void _do_get_evt			(RX_SOCKET socket);
static void _do_evt_confirm		(RX_SOCKET socket);

static void _do_restart_main	(RX_SOCKET socket);

static void _do_req_log			(RX_SOCKET socket, SLogReqMsg*pmsg);

static void _do_get_network		(RX_SOCKET socket);
static void _do_set_network		(RX_SOCKET socket, SSetNetworkCmd *pmsg);
static void _do_set_network_cfg	(RX_SOCKET socket, SIfConfig      *pmsg);
static void _do_delete_network	(RX_SOCKET socket, SSetNetworkCmd *pmsg);
static void _do_save_network	(RX_SOCKET socket);
static void _do_reload_network	(RX_SOCKET socket);

static void _do_get_print_queue	(RX_SOCKET socket);
static void _do_add_print_queue	(RX_SOCKET socket, SPrintQueueEvt *pmsg);
static void _do_set_print_queue	(RX_SOCKET socket, SPrintQueueEvt *pmsg);
static void _do_get_print_queue_item	(RX_SOCKET socket, SPrintQueueEvt *pmsg);
static void _do_del_print_queue	(RX_SOCKET socket, SPrintQueueEvt *pmsg);
static void _do_up_print_queue	(RX_SOCKET socket, SPrintQueueEvt *pmsg);
static void _do_dn_print_queue	(RX_SOCKET socket, SPrintQueueEvt *pmsg);
static void _do_del_file		(RX_SOCKET socket, SPrintQueueEvt *pmsg);

static void _do_get_print_env	(RX_SOCKET socket);
static void _do_get_ink_def		(RX_SOCKET socket);

static void _do_head_fluidCtrlMode(RX_SOCKET socket, SFluidCtrlCmd* pmsg);
static void _do_fluidCtrlMode	  (RX_SOCKET socket, SFluidCtrlCmd* pmsg);
static void _do_fluid_pressure	  (RX_SOCKET socket, SValue*		pmsg);
static void _do_scales_tara		  (RX_SOCKET socket, SValue*        pmsg);

static void _do_get_printer_cfg	(RX_SOCKET socket);
static void _do_set_printer_cfg	(RX_SOCKET socket, SPrinterCfgMsg *pmsg);

static void _do_get_stepper_cfg	(RX_SOCKET socket);
static void _do_set_stepper_cfg (RX_SOCKET socket, SStepperCfg *pmsg);
static void _do_cmd_stepper_test(RX_SOCKET socket, SStepperMotorTest *pmsg);

static void _do_start_printing	(RX_SOCKET socket);
static void _do_stop_printing	(RX_SOCKET socket);
static void _do_abort_printing	(RX_SOCKET socket);
static void _do_pause_printing	(RX_SOCKET socket);
static void _do_external_data	(RX_SOCKET socket, int state);

static void _do_get_prn_stat	(RX_SOCKET socket);

static void _do_test_start		(RX_SOCKET socket, SPrintQueueEvt *pmsg);
static void _do_clean_start		(RX_SOCKET socket);

//--- handle_gui_msg ---------------------------------------------
int handle_gui_msg(RX_SOCKET socket, void *pmsg, int len, struct sockaddr *sender, void *par)
{
	SMsgHdr* phdr = (SMsgHdr*)pmsg;

//	TrPrintfL(TRUE, "GUI Received id=0x%04x", phdr->msgId);
	
	if (FALSE)
	{
		char str[64];
		sok_get_peer_name(socket, str, NULL, NULL);
		TrPrintfL(TRUE, "handle_gui_msg: sender=>>%s<<", str);
	}
		
	if (phdr->msgId >= CMD_PLC_0 && phdr->msgId < CMD_PLC_END)		plc_handle_gui_msg(socket, phdr->msgId, &phdr[1], phdr->msgLen - sizeof(SMsgHdr));
	else if (phdr->msgId >= CMD_TT_0 && phdr->msgId < CMD_TT_END)	step_handle_gui_msg(socket, phdr->msgId, &phdr[1], phdr->msgLen - sizeof(SMsgHdr));
	else if (phdr->msgId >= CMD_CAP_0 && phdr->msgId < CMD_CAP_END)	step_handle_gui_msg(socket, phdr->msgId, &phdr[1], phdr->msgLen - sizeof(SMsgHdr));
	else if (phdr->msgId >= CMD_CLN_0 && phdr->msgId < CMD_CLN_END)	step_handle_gui_msg(socket, phdr->msgId, &phdr[1], phdr->msgLen - sizeof(SMsgHdr));
	else
	{
		switch (phdr->msgId)
		{
		case CMD_PING:				_do_ping(socket);													break;
		case CMD_STATUS:			_do_status(socket);													break;
		case CMD_GET_EVT:			_do_get_evt(socket);												break;
		case CMD_EVT_CONFIRM:		_do_evt_confirm(socket);											break;
			
		case CMD_RESTART_MAIN:		_do_restart_main(socket);											break;

		case CMD_REQ_LOG:			_do_req_log(socket, (SLogReqMsg*)pmsg);								break;
		
		case CMD_GET_NETWORK:		_do_get_network(socket);											break;
		case CMD_SET_NETWORK:		_do_set_network(socket, (SSetNetworkCmd*)pmsg);						break;
		case CMD_NETWORK_DELETE:	_do_delete_network(socket, (SSetNetworkCmd*)pmsg);					break;	
		case CMD_NETWORK_SAVE:		_do_save_network(socket);											break;
		case CMD_NETWORK_RELOAD:	_do_reload_network(socket);											break;
		case CMD_NETWORK_SETTINGS:	_do_set_network_cfg(socket, (SIfConfig*)&phdr[1]);					break;
			
		case CMD_GET_PRINT_QUEUE:	_do_get_print_queue(socket);										break;
		case CMD_ADD_PRINT_QUEUE:	_do_add_print_queue(socket, (SPrintQueueEvt*) pmsg);				break;
		case CMD_SET_PRINT_QUEUE:	_do_set_print_queue(socket, (SPrintQueueEvt*) pmsg);				break;
		case CMD_DEL_PRINT_QUEUE:	_do_del_print_queue(socket, (SPrintQueueEvt*) pmsg);				break;
		case CMD_GET_PRINT_QUEUE_ITM:_do_get_print_queue_item(socket, (SPrintQueueEvt*) pmsg);			break;
		case CMD_UP_PRINT_QUEUE:	_do_up_print_queue(socket, (SPrintQueueEvt*) pmsg);					break;
		case CMD_DN_PRINT_QUEUE:	_do_dn_print_queue(socket, (SPrintQueueEvt*) pmsg);					break;
		case CMD_DEL_FILE:			_do_del_file(socket, (SPrintQueueEvt*) pmsg);						break;
			
		case CMD_GET_PRINT_ENV:		_do_get_print_env(socket);											break;

		case CMD_GET_INK_DEF:		_do_get_ink_def(socket);											break;

		case CMD_FLUID_STAT:		fluid_reply_stat(socket);											
									chiller_reply_stat(socket);
									break;
		case CMD_HEAD_STAT:			ctrl_reply_stat(socket);											break;

		case CMD_ENCODER_STAT:		enc_reply_stat(socket);												break;
		case CMD_ENCODER_SAVE_PAR:	enc_save_par();														break;

		case CMD_HEAD_FLUID_CTRL_MODE: _do_head_fluidCtrlMode(socket, (SFluidCtrlCmd*) pmsg);			break;
		case CMD_FLUID_CTRL_MODE:	   _do_fluidCtrlMode(socket, (SFluidCtrlCmd*) pmsg);				break;
		case CMD_FLUID_PRESSURE:	   _do_fluid_pressure(socket, (SValue*)&phdr[1]);					break;

		case CMD_SCALES_TARA:		_do_scales_tara(socket, (SValue*)&phdr[1]);							break;				
			
		case CMD_GET_PRINTER_CFG:	_do_get_printer_cfg(socket);										break;
		case CMD_SET_PRINTER_CFG:	_do_set_printer_cfg(socket, (SPrinterCfgMsg*) pmsg);				break;

		case CMD_GET_STEPPER_CFG:	_do_get_stepper_cfg(socket);										break;
		case CMD_SET_STEPPER_CFG:	_do_set_stepper_cfg(socket, (SStepperCfg*)&phdr[1]);				break;
		case CMD_STEPPER_TEST:		_do_cmd_stepper_test(socket, (SStepperMotorTest*)&phdr[1]);			break;

		case CMD_START_PRINTING:	_do_start_printing(socket);											break;
		case CMD_STOP_PRINTING:		_do_stop_printing(socket);											break;
		case CMD_ABORT_PRINTING:	_do_abort_printing(socket);											break;
		case CMD_PAUSE_PRINTING:	_do_pause_printing(socket);											break;

		case CMD_EXTERNAL_DATA_ON:	_do_external_data(socket, TRUE);									break;
		case CMD_EXTERNAL_DATA_OFF:	_do_external_data(socket, FALSE);									break;

		case CMD_GET_PRN_STAT:		_do_get_prn_stat(socket);											break;

		case CMD_TEST_START:		_do_test_start(socket, (SPrintQueueEvt*) pmsg);						break;
		case CMD_CLEAN_START:		_do_clean_start(socket);											break;

		case CMD_ENCODER_UV_ON:		enc_uv_on();														break;
		case CMD_ENCODER_UV_OFF:	enc_uv_off();														break;

		case CMD_CO_SET_ORDER:		co_set_order((char*)&phdr[1]);										break;
		case CMD_CO_SET_ROLL:		co_set_roll((SValue*)&phdr[1]);										break;
		case CMD_CO_SET_OPERATOR:	co_set_operator((char*)&phdr[1]);									break;
			
		default: Error(WARN, 0, "Unknown Command 0x%08x", phdr->msgId);
		}
	}
	return REPLY_OK;
}

//--- _check_format ---------------------------------------------------------------
static void _check_format(char *function, void *phdr, int size)
{
	int msgLen = ((SMsgHdr*)phdr)->msgLen;
	if (msgLen!=size) 
	{
		Error(ERR_ABORT, 0, "Message Size Mismatch in >>%s<<: expected=%d, received=%d", function, size, msgLen);
		#ifdef _DEBUG
			int i=0;
			i=1/i;
		#endif
	}
}


//=== handlers =======================================================================

//--- _do_ping ------------------------------------------------------------------------
static void _do_ping(RX_SOCKET socket)
{
	SMsgHdr hdr;

	TrPrintf(TRUE, "_do_ping");

	hdr.msgLen = sizeof(hdr);
	hdr.msgId = REP_PING;

	send(socket, (char*)&hdr, hdr.msgLen, 0);

	Error(LOG, 0, "Replied PING");

	ctrl_ping_head(0);
}

//--- _do_status ------------------------------------------------------------------------
static void _do_status(RX_SOCKET socket)
{
	SStatusMsg msg;

	TrPrintf(TRUE, "_do_status");
	msg.hdr.msgLen = sizeof(msg);
	msg.hdr.msgId = REP_STATUS;

	sok_send(&socket, &msg);
}

//--- _do_evt_confirm ------------------------------------------------------------------------
static void _do_evt_confirm(RX_SOCKET socket)
{
	TrPrintf(TRUE, "_do_evt_confirm");
	err_clear_all();
	machine_error_reset();
	gui_send_cmd(REP_EVT_CONFIRM);
}

//--- _do_restart_main -----------------------------------------------------------------------
static void _do_restart_main(RX_SOCKET socket)
{
    TrPrintfL(TRUE, "_do_restart_main");
	Error(LOG, 0, "Restarting main");
#ifdef linux
    system("/bin/systemctl restart radex.service");
    exit(100);
#endif
}

//--- _do_get_evt ------------------------------------------------------------------------
static void _do_get_evt(RX_SOCKET socket)
{
	SLogMsg   msg;
	SLogItem *item;
	int i;

	TrPrintf(TRUE, "_do_get_evt");
	msg.hdr.msgLen = sizeof(msg);
	msg.hdr.msgId = EVT_GET_EVT;

	i = 0;
	// TrPrintf(TRUE, "sizeof(item)=%d", sizeof(*item));
	while ((item=err_get_log_item(i++)))
	{
	//	TrPrintf(TRUE, "log >>%s<<", item->formatStr);
		memcpy(&msg.log, item, sizeof(msg.log));
		sok_send(&socket, &msg);
	}
}

//--- _do_req_log ----------------------------------------------------
static void _do_req_log(RX_SOCKET socket, SLogReqMsg *pmsg)
{
	int i;
	int last;
	int cnt;
	SLogReqMsg	reply;
	SLogMsg		msg;
	log_Handle	log=NULL;

	TrPrintf(TRUE, "_do_req_log");
	_check_format("_do_req_log", pmsg, sizeof(*pmsg));
	
	if (pmsg->filepath[0]) log_open(pmsg->filepath, &log, FALSE);
	else log_open(PATH_LOG FILENAME_LOG, &log, FALSE);

	reply.hdr.msgLen = sizeof(reply);
	reply.hdr.msgId	 = REP_REQ_LOG;
	reply.first		 = pmsg->first;
	reply.count		 = log_get_item_cnt(log); 
	last			 = log_get_last_item_no(log);

	if (*pmsg->find)
	{
		if (pmsg->first>=0)	// search downwards
		{
			for(reply.first=pmsg->first; log_get_item(log, reply.first, &msg.log)==REPLY_OK && strstr(msg.log.formatStr, pmsg->find)==NULL; reply.first++)
			{
			}
		}
		else // search upwards
		{
			for(reply.first=-pmsg->first; reply.first>=0 && log_get_item(log, reply.first, &msg.log)==REPLY_OK && strstr(msg.log.formatStr, pmsg->find)==NULL; reply.first--)
			{
			}
		}
	}
	
	if (reply.first>=reply.count) reply.first=reply.count-1;
	if (reply.first<0) reply.first=0;
	gui_send_msg(socket, &reply);

	msg.hdr.msgLen=sizeof(msg);
	msg.hdr.msgId =EVT_GET_LOG;
	for(i = last-reply.first-1, cnt = pmsg->count; cnt-- && log_get_item(log, i, &msg.log)==REPLY_OK; i--)
	{
//		TrPrintf(TRUE, "_do_req_log >>%s<<, size=%d", msg.log.formatStr, sizeof(msg.log));
		gui_send_msg(socket, &msg);
	}
	log_close(&log);
}

//--- _do_get_network ----------------------------------------
static void _do_get_network(RX_SOCKET socket)
{
	net_send_config(socket);
	net_send_items(socket, TRUE);
}

//--- _do_set_network_cfg -----------------------------------------
static void _do_set_network_cfg	(RX_SOCKET socket, SIfConfig *pmsg)
{
	net_set_config(socket, pmsg);
	ctrl_update_hostname();
}

//--- _do_set_network -----------------------------------------
static void _do_set_network(RX_SOCKET socket, SSetNetworkCmd *pmsg)
{
	_check_format("_do_set_network", pmsg, sizeof(*pmsg));
	net_set_item(socket, &pmsg->item, pmsg->flash);
}

//--- _do_delete_network -------------------------------------------------
static void _do_delete_network(RX_SOCKET socket, SSetNetworkCmd *pmsg)
{
	_check_format("_do_delete_network", pmsg, sizeof(*pmsg));
	net_delete_item(socket, &pmsg->item);	
}

//--- _do_save_network -------------------------------------------------
static void _do_save_network(RX_SOCKET socket)
{
	net_save(FILENAME_NETWORK);
	boot_request(CMD_BOOT_INFO_REQ);
}

//--- do_cancel_network -----------------------------------------------
static void _do_reload_network(RX_SOCKET socket)
{
	net_reset();
}

//--- _do_get_print_queue --------------------------------------
static void _do_get_print_queue(RX_SOCKET socket)
{
	SPrintQueueEvt   msg;
	SPrintQueueItem *item;
	int i;

	TrPrintf(TRUE, "_do_get_print_queue");
	msg.hdr.msgLen = sizeof(msg.hdr);
	msg.hdr.msgId = REP_GET_PRINT_QUEUE;
	sok_send(&socket, &msg);

	msg.hdr.msgLen = sizeof(msg);
	msg.hdr.msgId = EVT_GET_PRINT_QUEUE;

	i = 0;
	while (item=pq_get_item_n(i++))
	{
		TrPrintf(TRUE, "queue[%d]: >>%s<<", i, item->filepath);
		memcpy(&msg.item, item, sizeof(msg.item));
		sok_send(&socket, &msg);
		#ifndef linux
		rx_sleep(20);
		#endif
	}
	TrPrintf(TRUE, "_do_get_print_queue END");
}

#ifdef linux
/*
//--- _chmod ---------------------------
static void _chmod(const char *path, UINT32 mode)
{
	int i;
	if (i=str_start(path, "ripped-data:\\"))
	{
		char str[MAX_PATH];
		char *fname=NULL;
		sprintf(str, PATH_RIPPED_DATA "%s", &path[i]);
		for (i=0; i<strlen(str); i++) 
		{
			if (str[i]=='\\' || str[i]=='/')
			{
				str[i]='/';
				fname=&str[i];
			}
		}
		i = chmod(str, mode | S_IRWXU | S_IRWXG | S_IRWXO);			
		sprintf(&str[strlen(str)], "%s.xml", fname);
		i=chmod(str, mode);			
	}
}
*/
//--- _chmod ---------------------------
static void _chmod(const char *path, UINT32 mode)
{
	int i;
	char str[MAX_PATH];
	char *fname=NULL;
	i=str_start(path, "ripped-data");
	if (i>=0)
	{
		while (path[i]==':' || path[i]=='/' || path[i]=='\\') i++;
	
		sprintf(str, PATH_RIPPED_DATA "%s", &path[i]);
		
		for (i=0; i<strlen(str); i++) 
		{
			if (str[i]=='\\' || str[i]=='/')
			{
				str[i]='/';
				fname=&str[i];
			}
		}
		i = chmod(str, mode | S_IRWXU | S_IRWXG | S_IRWXO);			
		sprintf(&str[strlen(str)], "%s.xml", fname);
		i=chmod(str, mode | S_IRWXU | S_IRWXG | S_IRWXO);
	}
}
#endif

//--- _do_add_print_queue ------------------------------------------
static void _do_add_print_queue	(RX_SOCKET socket, SPrintQueueEvt *pmsg)
{
	SPrintQueueItem *item;
	_check_format("_do_add_print_queue", pmsg, sizeof(*pmsg));
	#ifdef linux
	_chmod(pmsg->item.filepath, S_IRWXU | S_IRWXG | S_IRWXO);
	#endif
	item=pq_add_item(&pmsg->item);
	if (item!=NULL) gui_send_print_queue(EVT_GET_PRINT_QUEUE, item);
	pq_save(PATH_USER FILENAME_PQ);
}

//--- _do_set_print_queue ------------------------------------------
static void _do_set_print_queue	(RX_SOCKET socket, SPrintQueueEvt *pmsg)
{
	SPrintQueueItem *item;
	_check_format("_do_set_print_queue", pmsg, sizeof(*pmsg));
	#ifdef linux
	_chmod(pmsg->item.filepath, S_IRWXU | S_IRWXG | S_IRWXO);
	#endif
	item=pq_set_item(&pmsg->item);
	if (item!=NULL) gui_send_print_queue(EVT_GET_PRINT_QUEUE, item);
	pq_save(PATH_USER FILENAME_PQ);
}

//--- _do_get_print_queue_item ------------------------------------------
static void _do_get_print_queue_item(RX_SOCKET socket, SPrintQueueEvt *pmsg)
{
	SPrintQueueItem *item;
	_check_format("_do_req_print_queue", pmsg, sizeof(*pmsg));
	item=pq_get_item(&pmsg->item);
	if (item!=NULL) gui_send_print_queue(EVT_GET_PRINT_QUEUE, item);
	pq_save(PATH_USER FILENAME_PQ);
}

//--- _do_del_print_queue ------------------------------------------
static void _do_del_print_queue	(RX_SOCKET socket, SPrintQueueEvt *pmsg)
{
	int cnt;
	_check_format("_do_del_print_queue", pmsg, sizeof(*pmsg));
	pq_del_item(&pmsg->item);
	cnt  = gui_send_print_queue(EVT_DEL_PRINT_QUEUE, &pmsg->item);
	pq_save(PATH_USER FILENAME_PQ);
}

//--- _do_up_print_queue ------------------------------------------
static void _do_up_print_queue	(RX_SOCKET socket, SPrintQueueEvt *pmsg)
{
	_check_format("_do_up_print_queue", pmsg, sizeof(*pmsg));
	pq_move_item(&pmsg->item, 1);
	gui_send_print_queue(EVT_UP_PRINT_QUEUE, &pmsg->item);
}

//--- _do_dn_print_queue ------------------------------------------
static void _do_dn_print_queue	(RX_SOCKET socket, SPrintQueueEvt *pmsg)
{
	_check_format("_do_dn_print_queue", pmsg, sizeof(*pmsg));
	pq_move_item(&pmsg->item, -1);
	gui_send_print_queue(EVT_DN_PRINT_QUEUE, &pmsg->item);
}

//--- _do_del_file ------------------------------------------
static void _do_del_file	(RX_SOCKET socket, SPrintQueueEvt *pmsg)
{
	_check_format("_do_del_file", pmsg, sizeof(*pmsg));
	pc_del_file(pmsg->item.filepath);
}

//--- _do_get_print_env --------------------------------------
static void _do_get_print_env(RX_SOCKET socket)
{
//	rip_send_print_env(socket);
}

//--- _do_get_ink_def --------------------------------------
static void _do_get_ink_def(RX_SOCKET socket)
{
	SInkDefMsg		msg;

	TrPrintf(TRUE, "_do_get_ink_def");
	msg.hdr.msgLen = sizeof(msg.hdr);
	msg.hdr.msgId = REP_GET_INK_DEF;
	sok_send(&socket, &msg);

	msg.hdr.msgId = BEG_GET_INK_DEF;
	sok_send(&socket, &msg);

	msg.hdr.msgLen = sizeof(msg);
	msg.hdr.msgId = ITM_GET_INK_DEF;
	
	{
		SEARCH_Handle dirSearch;
		int  isDir;
		char familyName[100];
		char path[MAX_PATH];

		dirSearch = rx_search_open(PATH_WAVE_FORM, "*");
		while (rx_search_next(dirSearch, familyName, sizeof(familyName), NULL, NULL, &isDir))
		{
			if (isDir && familyName[0]!='.')
			{
				sprintf(path, PATH_WAVE_FORM "%s", familyName);
				char fname[100];
				char fpath[MAX_PATH];
				SEARCH_Handle search;
				search = rx_search_open(path, "*.wfd");
		
				while (rx_search_next(search, fname, sizeof(fname), NULL, NULL, &isDir))
				{
					if(!isDir)
					{
						sprintf(fpath, "%s/%s", path, fname);
						if (setup_ink(fpath, &msg.ink, READ)==REPLY_OK)
						{
							strcpy(msg.ink.family, familyName);	
							sok_send(&socket, &msg);
						}
					}
				}
				rx_search_close(search);
			}
		}
		rx_search_close(dirSearch);
	}

	msg.hdr.msgLen = sizeof(msg.hdr);
	msg.hdr.msgId = END_GET_INK_DEF;
	sok_send(&socket, &msg);
}

//--- _do_head_fluidCtrlMode ---
static void _do_head_fluidCtrlMode(RX_SOCKET socket, SFluidCtrlCmd* pmsg)
{
	if (pmsg->ctrlMode==ctrl_wipe) step_wipe_start(pmsg->no);
	else ctrl_send_head_fluidCtrlMode(pmsg->no, pmsg->ctrlMode, TRUE, TRUE);
}

//--- _do_fluidCtrlMode ---
static void _do_fluidCtrlMode(RX_SOCKET socket, SFluidCtrlCmd* pmsg)
{
	fluid_send_ctrlMode(pmsg->no, pmsg->ctrlMode, TRUE);
}

//--- _do_fluid_pressure ---
static void _do_fluid_pressure(RX_SOCKET socket, SValue* pmsg)
{
	fluid_send_pressure(pmsg->no, pmsg->value);
}

//--- _do_scales_tara ----------------------------------------------
static void _do_scales_tara(RX_SOCKET socket, SValue* pmsg)
{
	fluid_send_tara(pmsg->no);
}

//--- _do_get_printer_cfg --------------------------------------
static void _do_get_printer_cfg(RX_SOCKET socket)
{
	SPrinterCfgMsg	msg;
	int				i, l, len;

	TrPrintf(TRUE, "_do_get_printer_cfg");
	memset(&msg, 0, sizeof(msg));
	msg.hdr.msgLen	= sizeof(msg);
	msg.hdr.msgId	= REP_GET_PRINTER_CFG;
	msg.type		= RX_Config.printer.type;
	msg.overlap		= RX_Config.printer.overlap;
	msg.headsPerColor = RX_Config.headsPerColor;
	memcpy(msg.headFpVoltage,	RX_Config.headFpVoltage,	sizeof(msg.headFpVoltage));
	memcpy(msg.headDist,		RX_Config.headDist,			sizeof(msg.headDist));
	memcpy(msg.headDistBack,	RX_Config.headDistBack,		sizeof(msg.headDistBack));
	memcpy(msg.colorOffset,		RX_Config.colorOffset,		sizeof(msg.colorOffset));
	for (i=0, len=0; i<RX_Config.inkSupplyCnt; i++)
	{
		if (i) msg.inkFileNames[len++] = ',';
		l = (int)strlen(RX_Config.inkSupply[i].inkFileName);
		memcpy(&msg.inkFileNames[len], RX_Config.inkSupply[i].inkFileName, l);
		len += l;
		msg.rectoVerso[i] = RX_Config.inkSupply[i].rectoVerso; 
	}
	msg.externalData = RX_Config.externalData;
	memcpy(&msg.offset, &RX_Config.printer.offset, sizeof(msg.offset));
//	ctrl_set_config();

	if (socket==INVALID_SOCKET) gui_send_msg(socket, &msg);
	else						sok_send(&socket, &msg);
	
	if (RX_Config.printer.type==printer_cleaf) co_send_order(socket);
}

//--- _do_set_printer_cfg --------------------------------------
static void _do_set_printer_cfg(RX_SOCKET socket, SPrinterCfgMsg* pmsg)
{
	char			*ch, *start;
	int				i;
	int				fluid_P[SIZEOF(RX_Config.inkSupply)];

	if (pmsg->hdr.msgLen != sizeof(SPrinterCfgMsg)) 
	{
		char str[100];
		sok_get_peer_name(socket, str, NULL, NULL);
		Error(ERR_CONT, 0, "Incompatible GUI Version on %s", str);
		return;
	}
	
	TrPrintf(TRUE, "_do_set_printer_cfg");
	_check_format("_do_set_printer_cfg", pmsg, sizeof(*pmsg));

	//--- parse inkfile names ----
	RX_Config.printer.type		  = pmsg->type;
	RX_Config.printer.overlap	  = pmsg->overlap;
	RX_Config.inkSupplyCnt=0;
	memcpy(RX_Config.headFpVoltage, pmsg->headFpVoltage,	sizeof(RX_Config.headFpVoltage));
	memcpy(RX_Config.headDist,		pmsg->headDist,			sizeof(RX_Config.headDist));
	memcpy(RX_Config.headDistBack,	pmsg->headDistBack,		sizeof(RX_Config.headDistBack));
	memcpy(RX_Config.colorOffset,	pmsg->colorOffset,		sizeof(RX_Config.colorOffset));	
	for (i=0; i<SIZEOF(RX_Config.inkSupply); i++)
		fluid_P[i] = RX_Config.inkSupply[i].fluid_P;
	memset(RX_Config.inkSupply, 0, sizeof(RX_Config.inkSupply));
	start=pmsg->inkFileNames;
	for (ch=pmsg->inkFileNames; ; ch++)
	{
		if (*ch==',' || *ch==0)
		{
			memcpy(RX_Config.inkSupply[RX_Config.inkSupplyCnt].inkFileName, start, ch-start);
			RX_Config.inkSupply[RX_Config.inkSupplyCnt].rectoVerso = pmsg->rectoVerso[RX_Config.inkSupplyCnt]; 
			RX_Config.inkSupplyCnt++;
			if (!*ch) break;
			start = ch+1;
		}
	}
	RX_Config.headsPerColor = pmsg->headsPerColor;
	for (i=0; i<RX_Config.inkSupplyCnt; i++)
	{
		RX_Config.inkSupply[i].fluid_P        = fluid_P[i];
		RX_Config.inkSupply[i].cylinderPresSet = fluid_get_cylinderPresSet(i);	
	}
	RX_Config.externalData = pmsg->externalData;
	memcpy(&RX_Config.printer.offset,		&pmsg->offset,				sizeof(RX_Config.printer.offset));
	setup_config(PATH_USER FILENAME_CFG, &RX_Config, WRITE);

	sok_send_2(&socket, REP_SET_PRINTER_CFG, 0, NULL);	 

	_do_get_printer_cfg(INVALID_SOCKET);
}

//--- _do_get_stepper_cfg --------------------------------------
static void _do_get_stepper_cfg	(RX_SOCKET socket)
{
	if (socket==INVALID_SOCKET) gui_send_msg_2(socket, REP_GET_STEPPER_CFG, sizeof(RX_Config.stepper), &RX_Config.stepper);
	else						sok_send_2(&socket, REP_GET_STEPPER_CFG, sizeof(RX_Config.stepper), &RX_Config.stepper);
}

//--- _do_set_stepper_cfg ----------------------------------------
static void _do_set_stepper_cfg (RX_SOCKET socket, SStepperCfg *pmsg)
{
	int print_height = RX_Config.stepper.print_height;
	memcpy(&RX_Config.stepper, pmsg, sizeof(RX_Config.stepper));
	RX_Config.stepper.print_height = print_height;
	
	gui_send_msg_2(0, REP_GET_STEPPER_CFG, sizeof(RX_Config.stepper), &RX_Config.stepper);
	step_set_config();
}

//--- _do_cmd_stepper_test ----------------------------------------
static void _do_cmd_stepper_test (RX_SOCKET socket, SStepperMotorTest *pmsg)
{
	step_do_test(pmsg);
}

//--- _do_start_printing ---------------------------------------------
static void _do_start_printing	(RX_SOCKET socket)
{
	pc_start_printing();
}

//--- _do_stop_printing ---------------------------------------------
static void _do_stop_printing	(RX_SOCKET socket)
{
	pc_stop_printing();
}

//--- _do_abort_printing ---------------------------------------------
static void _do_abort_printing	(RX_SOCKET socket)
{
	pc_abort_printing();
}

//--- _do_pause_printing ---------------------------------------------
static void _do_pause_printing	(RX_SOCKET socket)
{
	pc_pause_printing();
}

//--- _do_external_data --------------------------------------------
static void _do_external_data	(RX_SOCKET socket, int state)
{
	RX_PrinterStatus.externalData = state;
	if (state)	pem_set_config();
	else        pem_end(); 
	gui_send_printer_status(&RX_PrinterStatus);
}

//--- _do_test_start ---------------------------------------------
static void _do_test_start	(RX_SOCKET socket, SPrintQueueEvt* pmsg)
{
	RX_PrinterStatus.testMode = TRUE;
	memcpy(&RX_TestImage, &pmsg->item, sizeof(RX_TestImage));
	RX_TestImage.scans=1;
//	if (rx_def_is_scanning(RX_Config.printer.type) && (RX_TestImage.testImage==PQ_TEST_JETS || RX_TestImage.testImage==PQ_TEST_ENCODER) ) RX_TestImage.scans=RX_Config.inkSupplyCnt;
//	if (rx_def_is_scanning(RX_Config.printer.type) && (RX_TestImage.testImage==PQ_TEST_JETS) ) RX_TestImage.scans=RX_Config.inkSupplyCnt;
	if (RX_TestImage.testImage==PQ_TEST_SCANNING) RX_TestImage.scans=1;
	if (RX_Config.printer.type==printer_test_table && RX_TestImage.testImage==PQ_TEST_JETS) RX_TestImage.scans=RX_Config.inkSupplyCnt;
	if (RX_TestImage.scans<1) RX_TestImage.scans=1;
//	RX_TestImage.scanMode = PQ_SCAN_STD;
	RX_TestImage.id.id = RX_TestImage.testImage;
	pc_start_printing();
}

//--- _do_clean_start ---------------------------------------------
static void _do_clean_start	(RX_SOCKET socket)
{
	machine_clean();
}

//--- _do_get_prn_stat ----------------------------------------------
static void _do_get_prn_stat		(RX_SOCKET socket)
{
	gui_send_printer_status(&RX_PrinterStatus);
}