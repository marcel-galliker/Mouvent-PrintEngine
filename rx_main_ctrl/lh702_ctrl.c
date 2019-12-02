// ****************************************************************************
//
//	lh702_ctrl.c
//
// ****************************************************************************
//
//	Copyright 2013 by Radex AG, Switzerland. All rights reserved.
//	Written by Marcel Galliker 
//
// ****************************************************************************

#include <stdio.h>
#include "rx_common.h"
#include "rx_def.h"
#include "rx_error.h"
#include "rx_sok.h"
#include "rx_term.h"
#include "rx_threads.h"
#include "rx_trace.h"
#include "args.h"
#include "plc_ctrl.h"
#include "print_queue.h"
#include "print_ctrl.h"
#include "machine_ctrl.h"
#include "setup.h"
#include "gui_msg.h"
#include "ctrl_msg.h"
#include "ctrl_svr.h"
#include "network.h"
#include "label.h"
#include "spool_svr.h"
#include "lh702_tcp_ip.h"
#include "lh702_ctrl.h"

//--- Defines -----------------------------------------------------------------
	

#define IP_ADDR_SERVER	"192.168.20.192"
#define IP_PORT_SERVER	2000

#define IP_ADDRESS_MAIN	"192.168.20.118"

//--- Externals ---------------------------------------------------------------

//--- Statics -----------------------------------------------------------------
static RX_SOCKET _Socket=INVALID_SOCKET;

static int		 _lh702ThreadRunning=FALSE;

static SLH702_State _Status;

static SPrintQueueItem	*_pItem;
static int		_IncPerMeter;
static int		_WakeupLen;
static int		_PrintGo_Dist;
static int		_PrintGo_Mode;
static int		_Scanning=FALSE;
static int		_DistTelCnt=0;
static int		_TotalPgCnt;
static int		_StopPG;
static int		_Printing=FALSE;
static int		_Khz=0;
static UINT32	_WarnMarkReaderPos;

//--- Prototypes --------------------------------------------------------------
static void _set_network_config(void);

static void *_lh702_thread(void *lpParameter);
static int   _lh702_closed(RX_SOCKET socket, const char *peerName);

static int  _lh702_handle_msg		(RX_SOCKET socket, void *msg, int len, struct sockaddr *sender, void *par);
static void _handle_dist			(int value);
static void _handle_lateral			(int value);
static void _handle_material		(char *material);
static void _handle_thickness		(int value);
static void _handle_headheight		(int value);
static void _handle_encoffset		(int value);

//--- lh702_init -------------------------------------------------
void lh702_init(void)
{
	memset(&_Status, 0, sizeof(_Status));
	_Status.hdr.msgLen = sizeof(_Status);
	_Status.hdr.msgId  = EVT_STATE;
		
	_Socket=INVALID_SOCKET;
	if(!_lh702ThreadRunning)
	{
		_lh702ThreadRunning = TRUE;
		rx_thread_start(_lh702_thread, NULL, 0, "_lh702_thread");
	}
}

//--- _set_network_config ----------------------------------
static void _set_network_config(void)
{
	SIfConfig cfg;
	if (sok_get_ifconfig("em2:1", &cfg)==REPLY_ERROR)
	{
		*((UINT32*)&cfg.addr) = sok_addr_32(IP_ADDRESS_MAIN);
		cfg.dhcp = FALSE;
		*((UINT32*)&cfg.mask) = sok_addr_32("255.255.255.0");
		sok_set_ifconfig("em2:1", &cfg);
	}
}

//--- siemensc_end -------------------------------------
void lh702_end(void)
{
	_lh702ThreadRunning = FALSE;		
}

//--- lh702_error_reset -----------------------
void lh702_error_reset(void)
{
//	sok_send_2(&_Socket, CMD_ERROR_RESET, 0, NULL);
}

//--- lh702_save_material ----------------------------------------------
void lh702_save_material	(char *varList)
{
	HANDLE attribute =NULL;
	char attrname[64];
	char path[MAX_PATH];
	char *str=varList;
	char *end;
	char *val;
	char var[128];
		
	//--- find material ---
	if ((end=strchr(str, '\n')))
	{
		end++;
	}
	str = end;

	while ((end=strchr(str, '\n')))
	{
		*end=0;
		val= strchr(str, '=');
		*val=0;
		strcpy(var, str);
		*val++='=';
		printf(">>%s<< = >>%s<<\n",  var, val);
		if (!strcmp(var, "XML_MATERIAL"))			strncpy(_Status.material, val, sizeof(_Status.material)-1);
		if (!strcmp(var, "XML_HEAD_HEIGHT"))		_Status.head_height = (INT32)(atof(val)*1000);
		if (!strcmp(var, "XML_MATERIAL_THICKNESS"))	_Status.thickness   = atoi(val);
		if (!strcmp(var, "XML_ENC_OFFSET"))			_Status.encoder_adj = atoi(val);
		*end++='\n';
		str = end;
	}
	lh702_tick();
}

//--- lh702_set_printpar -----------------------------------------------------
void lh702_set_printpar(SPrintQueueItem *pitem)
{
	_pItem = pitem;
	if (pitem==NULL)
	{
		_Status.id				= 0;
		_Status.dist			= 0;
		_Status.lateral			= 0;
		_Status.copies_printed	= 0;
	}
	else
	{
		_Status.id				= pitem->id.id;
		_Status.dist			= pitem->printGoDist;
		_Status.lateral			= pitem->pageMargin;
		_Status.printState		= PS_STARTING;
	}
	lh702_tick();
}

//--- lh702_on_error --------------------------------------------------------
void lh702_on_error(ELogItemType type, char *deviceStr, int no, char *txt)
{
	SLH702_Message msg;
	memset(&msg, 0, sizeof(msg));
	msg.hdr.msgLen = sizeof(msg);
	switch(type)
	{
	case LOG_TYPE_UNDEF:	return;
	case LOG_TYPE_LOG:		msg.hdr.msgId  = EVT_LOG;		break;
	case LOG_TYPE_WARN:		msg.hdr.msgId  = EVT_WARN;		break;
	default:				msg.hdr.msgId  = EVT_ERROR;		break;	
	}
	strncpy(msg.device, deviceStr, sizeof(msg.device)-1);
	strncpy(msg.msg,    txt,       sizeof(msg.msg)-1);
	sok_send(&_Socket, &msg); 
}


//--- _lh702_thread ------------------------------------------------------------
static void *_lh702_thread(void *lpParameter)
{
	int errNo;
	int first=TRUE;
	
	while (_lh702ThreadRunning)
	{
		if (RX_Config.printer.type==printer_LH702)
		{
			if (first)
			{
				_set_network_config();
				first=FALSE;					
			}
			if (_Socket==INVALID_SOCKET)
			{			
				errNo=sok_open_client_2(&_Socket, IP_ADDR_SERVER, IP_PORT_SERVER, SOCK_STREAM, _lh702_handle_msg, _lh702_closed);
				if (errNo)
				{
				//	char str[256];
				//	Error(ERR_CONT, 0, "Socket Error >>%s<<", err_system_error(errNo, str,  sizeof(str)));
				}
				else
				{
					TrPrintfL(TRUE, "Connected");
					ErrorEx(dev_plc, -1, LOG, 0, "Connected");
				}
			}							
		}
		
		rx_sleep(1000);
	}
	return NULL;
}

//--- _lh702_closed --------------------------------------------
static int _lh702_closed(RX_SOCKET socket, const char *peerName)
{
	sok_close(&_Socket);
//	TrPrintfL(TRUE, "TCP/IP connection closed");
	Error(LOG, 0, "TCP/IP connection closed");
	return REPLY_OK;
}

//--- lh702_tick --------------------------------------------
void  lh702_tick(void)
{
	switch(RX_PrinterStatus.printState)
	{
	case ps_printing:	_Status.printState = PS_PRINTING; break;
	case ps_stopping:	_Status.printState = PS_PRINTING; break;
	default:			_Status.printState = PS_OFF;
	}
	_Status.copies_printed = RX_PrinterStatus.printedCnt;
	sok_send(&_Socket, &_Status); 
}

//--- lh702_menu ----------------------------------------------
void lh702_menu(char *str)
{
	if (RX_Config.printer.type==printer_LH702)
	{
		switch(str[0])
		{
		case '?':	term_printf("--- LH702: commands when not printing -------------\n");
					term_printf("m<name>:    load material\n");
					term_printf("t<microns>: set Thickness\n");
					term_printf("h<microns>: set Head Height above material\n");
					term_printf("e<val>:	    set encoder adjustment value\n");
				
					term_printf("--- LH702: commands while printing ----------------\n");
					term_printf("d<microns>: Add to PrintGo Distace\n");
					term_printf("l<microns>: Add to Page Margin\n");
					term_flush();
					break;	

		case 'd':	_handle_dist(atoi(&str[1]));	   break;
		case 'l':	_handle_lateral(atoi(&str[1]));	   break;
		case 'm':	_handle_material(&str[1]);		   break;		
		case 't':	_handle_thickness(atoi(&str[1]));  break;
		case 'h':	_handle_headheight(atoi(&str[1])); break;
		case 'e':	_handle_encoffset(atoi(&str[1]));  break;
		default:	break;
		}						
	}
}

//--- _lh702_handle_msg ------------------------------------------------------------------
static int _lh702_handle_msg(RX_SOCKET socket, void *pmsg, int len, struct sockaddr *sender, void *par)
{
	SMsgHdr		 *phdr = (SMsgHdr*)pmsg;
	SLH702_Value *val  = (SLH702_Value*)pmsg;
	SLH702_Str   *str  = (SLH702_Str*)pmsg;
	
	if (len)
	{
		net_register_by_device(dev_plc, -1);

//		TrPrintfL(TRUE, "received Siemens.MsgId=0x%08x", phdr->msgId);
		Error(LOG, 0, "received msg (len=%d, id=5x%08x)", phdr->msgLen, phdr->msgId);

		switch(phdr->msgId)
		{
		case CMD_START_PRINTING:  	Error(LOG, 0, "received CMD_START_PRINTING");	
									pc_start_printing();
									break;
			
		case CMD_STOP_PRINTING:		Error(LOG, 0, "received CMD_STOP_PRINTING");	
									pc_stop_printing(TRUE);
									break;
			
		case CMD_CHANGE_JOB:		Error(LOG, 0, "received CMD_CHANGE_JOB");		
									pc_change_job();
									break;
		
		case PAR_MATERIAL:			Error(LOG, 0, "Received PAR_MATERIAL >>%s<<", str->str);
									_handle_material(str->str);
									break;
			
		case PAR_THICKNESS:			Error(LOG, 0, "Received PAR_THICKNESS %d", val->value);
									_handle_thickness(val->value);
									break;
		
		case PAR_HEAD_HEIGHT:		Error(LOG, 0, "Received PAR_HEAD_HEIGHT %d", val->value);
									_handle_headheight(val->value);
									break;
			
		case PAR_ENCODER_ADJ:		Error(LOG, 0, "Received PAR_ENCODER_ADJ %d", val->value);
									_handle_encoffset(val->value);
									break;
			
		case CMD_ADD_DIST:			Error(LOG, 0, "Received CMD_ADD_DIST %d", val->value);
									_handle_dist(val->value);
									break;
			
		case CMD_ADD_LATERAL:		Error(LOG, 0, "Received CMD_ADD_LATERAL %d", val->value);
									_handle_lateral(val->value);
									break;
			
		default: Error(WARN, 0, "Siemens:Got unknown MessageId=0x%08x", phdr->msgId);
		}		
	}
	return REPLY_OK;
}

//--- _handle_dist ----------------------
static void _handle_dist (int value)
{
	if (_pItem!=NULL)
	{
		SPrintQueueEvt evt;
		memset(&evt, 0, sizeof(evt));
		evt.hdr.msgLen = sizeof(evt);
		evt.hdr.msgId  = EVT_SET_PRINT_QUEUE;
		memcpy(&evt.item, _pItem, sizeof(evt.item));
		evt.item.printGoDist += value;
		handle_gui_msg(INVALID_SOCKET, &evt, evt.hdr.msgLen, NULL, 0);
	}											
}

//--- _handle_lateral -----------------------
static void _handle_lateral	(int value)
{
	if (_pItem!=NULL)
	{
		SPrintQueueEvt evt;
		memset(&evt, 0, sizeof(evt));
		evt.hdr.msgLen = sizeof(evt);
		evt.hdr.msgId  = EVT_SET_PRINT_QUEUE;
		memcpy(&evt.item, _pItem, sizeof(evt.item));
		evt.item.pageMargin += value;
		handle_gui_msg(INVALID_SOCKET, &evt, evt.hdr.msgLen, NULL, 0);
	}
}

//--- _handle_material ---------------------------
static void _handle_material(char *material)
{
	plc_load_material(material);
}

//--- _handle_thickness --------------------------
static void _handle_thickness(int value)
{
	char varList[128];
	sprintf(varList, UnitID "\n" "XML_MATERIAL_THICKNESS=%d\n", value);
	plc_handle_gui_msg(INVALID_SOCKET, CMD_PLC_SET_VAR, varList, strlen(varList));
}

//--- _handle_headheight --------------------------
static void _handle_headheight(int value)
{
	char varList[128];
	sprintf(varList, UnitID "\n" "XML_HEAD_HEIGHT=%d\n", value);
	plc_handle_gui_msg(INVALID_SOCKET, CMD_PLC_SET_VAR, varList, strlen(varList));
}

//--- _handle_encoffset --------------------------
static void _handle_encoffset(int value)
{
	char varList[128];
	sprintf(varList, UnitID "\n" "XML_ENC_OFFSET=%d\n", value);
	plc_handle_gui_msg(INVALID_SOCKET, CMD_PLC_SET_VAR, varList, strlen(varList));
}
