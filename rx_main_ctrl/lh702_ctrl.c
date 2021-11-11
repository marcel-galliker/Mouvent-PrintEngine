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
#include <stdarg.h>
#include "rx_common.h"
#include "rx_def.h"
#include "rx_error.h"
#include "rx_file.h"
#include "rx_hash.h"
#include "rx_sok.h"
#include "rx_term.h"
#include "rx_threads.h"
#include "rx_trace.h"
#include "args.h"
#include "ctr.h"
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
#include "machine_ctrl.h"
#include "lh702_tcp_ip.h"
#include "lh702_ctrl.h"
#include "siemens_ctrl.h"

//--- Defines -----------------------------------------------------------------

//--- Externals ---------------------------------------------------------------
extern SInkSupplyStat FluidStatus[INK_SUPPLY_CNT];

typedef struct SStringCmd
{
	SMsgHdr		hdr;
	char		str[256];
} SStringCmd;

//--- Statics -----------------------------------------------------------------
static RX_SOCKET _Socket=INVALID_SOCKET;
int _lh702ThreadRunning=FALSE;
static SLH702_State _Status;
static SLH702_State2 _Status2;
static SLH702_State3 _Status3;

static SLH702_Materials _Materials;
static SPrintQueueItem	*_pItem;
static int		_Manipulated=FALSE;
static int		_lastQuery; // ticks of the last query received from LH702 PLC to timeout

//--- Prototypes --------------------------------------------------------------
static void _set_network_config(void);

static void *_lh702_thread(void *lpParameter);
static int   _lh702_closed(RX_SOCKET socket, const char *peerName);

static int  _lh702_handle_msg		(RX_SOCKET socket, void *msg, int len, struct sockaddr *sender, void *par);
static void _handle_dist			(int value);
static void _handle_lateral			(int value);
static void _handle_thickness		(int value);
static void _handle_headheight		(int value);
static void _handle_encoffset		(int value);
static void _lh702_send_status		(void);
static void _lh702_send_materials	(void);
static void _plc_set_var			(const char *format, ...);
static void _ctr_calc_check(time_t time, UCHAR *check);

typedef struct
{
	INT64 counter[3];
	UINT64	macAddr;
	time_t time;
} _sctr;

//--- lh702_init -------------------------------------------------
void lh702_init(void)
{
	if (RX_Config.printer.type != printer_LH702) return; // No init/thread if it's not an LH702
	void* (*fth)(void *lpParameter);

	if (RX_Config.lh702_protocol_version < 10)
	{
		memset(&_Status, 0, sizeof(_Status));
		_Status.hdr.msgLen = sizeof(_Status);
		_Status.hdr.msgId = EVT_STATE;

		memset(&_Status2, 0, sizeof(_Status2));
		_Status2.hdr.msgLen = sizeof(_Status2);
		_Status2.hdr.msgId = EVT_STATE2;
		_Status2.protocol_version_number = 2;

		memset(&_Status3, 0, sizeof(_Status3));
		_Status3.hdr.msgLen = sizeof(_Status3);
		_Status3.hdr.msgId = EVT_STATE3;
		_Status3.nb_cluster_per_color = RX_Config.headsPerColor / MAX_HEADS_BOARD;
		_Status3.protocol_version_number = 3;

		memset(&_Materials, 0, sizeof(_Materials));
	    _Materials.hdr.msgLen = sizeof(_Materials);
	    _Materials.hdr.msgId = REP_LIST_MATERIALS;	
		
	    lh702_load_material(RX_Config.material);
		fth = _lh702_thread;
	} else fth = siemens_thread;

	_Socket=INVALID_SOCKET;
	if(!_lh702ThreadRunning)
	{
		_lh702ThreadRunning = TRUE;
		rx_thread_start(fth, NULL, 0, "_lh702_thread");
	}
}

//--- _set_network_config ----------------------------------
static void _set_network_config(void)
{
	SIfConfig cfg;
	if (sok_get_ifconfig("em2:1", &cfg)==REPLY_ERROR)
	{
        *((UINT32 *)&cfg.addr) = sok_addr_32(RX_Config.em2_1_address);
		cfg.dhcp = FALSE;
        *((UINT32 *)&cfg.mask) = sok_addr_32(RX_Config.em2_1_mask);
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
		if (!strcmp(var, "XML_MATERIAL_THICKNESS"))	_Status.thickness   = (INT32)(atof(val)*1000);
		if (!strcmp(var, "XML_ENC_OFFSET"))			_Status.encoder_adj = atoi(val);
		if (!strcmp(var, "XML_JC_RATIO")) RX_Config.jc_ratio = atoi(val);

		*end++='\n';
		str = end;
	}
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
		_Status2.print_go_mode  = pitem->printGoMode;
		_Status3.print_go_mode = pitem->printGoMode;
		_Status.lateral			= pitem->pageMargin;
		_Status.printState		= PS_STARTING;
	}
}

//--- lh702_request_stop ------------------------------------------------------
void lh702_stop_printing()
{
	if (RX_Config.lh702_protocol_version >= 10) siemens_stop_printing();
	else
	{
		_Status2.stop_printing_request = TRUE;
		_Status3.stop_printing_request = TRUE;
	}
}

//--- _lh702_thread ------------------------------------------------------------
static void *_lh702_thread(void *lpParameter)
{
	int errNo;
	int first=TRUE;
	
	while (_lh702ThreadRunning)
	{		
		if (first)
		{
			_set_network_config();
			first=FALSE;					
		}
		if (_Socket==INVALID_SOCKET)
		{
			errNo=sok_open_client_2(&_Socket, RX_Config.master_ip_address, RX_Config.master_ip_port, SOCK_STREAM, _lh702_handle_msg, _lh702_closed);
			if (errNo)
			{
				char str[256];
				Error(ERR_CONT, 0, "Socket Error >>%s<<", err_system_error(errNo, str,  sizeof(str)));
			}
			else
			{
				TrPrintfL(TRUE, "Connected");
				ErrorEx(dev_plc, -1, LOG, 0, "Connected");
				_lastQuery = rx_get_ticks();
			}
		}
		else
		{
			// as the socket is a client, we should time out on our side
			if (rx_get_ticks() - _lastQuery > 10000) // 10 seconds
			{
				Error(ERR_CONT, 0, "Time out on communication with PLC");
				sok_close(&_Socket);
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
	Error(LOG, 0, "LH702 TCP/IP connection closed");
	return REPLY_OK;
}
//--- _lh702_send_status ---------------------------------------------
static void _lh702_send_status(void)
{
	if (_Socket!=INVALID_SOCKET)
	{
		// Protocol version 1
		switch(RX_PrinterStatus.printState)
		{
		case ps_printing:
			if (RX_StepperStatus.info.z_in_print)
				_Status.printState = PS_PRINTING;
			else
				_Status.printState = PS_STARTING;
			break;
		case ps_stopping:	_Status.printState = PS_PRINTING; break;
		default:			_Status.printState = PS_OFF;
		}
		strncpy(_Status.material, RX_Config.material, sizeof(_Status.material));
		_Status.head_height    = RX_Config.stepper.print_height;
		_Status.thickness	   = RX_Config.stepper.material_thickness;
		_Status.encoder_adj	   = RX_Config.printer.offset.incPerMeter[0];
		_Status.copies_printed = RX_PrinterStatus.printedCnt;
		_Status.meters_k	   = (INT32)RX_PrinterStatus.counterLH702[CTR_LH702_K]/1000;
		_Status.meters_color   = (INT32)RX_PrinterStatus.counterLH702[CTR_LH702_COLOR]/1000;
		_Status.meters_color_w = (INT32)RX_PrinterStatus.counterLH702[CTR_LH702_COLOR_W]/1000;
		sok_send(&_Socket, &_Status);

		static int order[LH702_NB_COLORS] = {4, 5, 6, 0, 1, 2, 3};
		
		// Protocol version 2
		if (RX_Config.lh702_protocol_version == 2)
		{			
			for (int i=0; i<LH702_NB_COLORS; i++)
			{
				for (int j=0; j<LH702_NB_CLUSTER_PER_COLORS_STATE2; j++)
				{
					int index = i * LH702_NB_CLUSTER_PER_COLORS_STATE2 + j;
					int colorIndex = order[i] * LH702_NB_CLUSTER_PER_COLORS_STATE2 + j;
					//_Status2.cluster_hours[index] = RX_HBStatus[index].clusterTime / 3600;
					_Status2.cluster_hours[index] = RX_HBStatus[colorIndex].head[0].printingSeconds / 3600;
				}
				_Status2.ink_level[i] = FluidStatus[i].canisterLevel;
			}
			sok_send(&_Socket, &_Status2);
		}

		if (RX_Config.lh702_protocol_version >= 3)
		{
			for (int i = 0; i < LH702_NB_COLORS; i++)
			{
				for (int j = 0; j < LH702_NB_CLUSTER_PER_COLORS_STATE3; j++)
				{
					int index = i * LH702_NB_CLUSTER_PER_COLORS_STATE3 + j;
					int colorIndex = order[i] * LH702_NB_CLUSTER_PER_COLORS_STATE3 + j;
					_Status3.cluster_hours[index] = RX_HBStatus[colorIndex].head[0].printingSeconds / 3600;
				}
				_Status3.ink_level[i] = FluidStatus[i].canisterLevel;
			}
			sok_send(&_Socket, &_Status3);			
		}

		//TrPrintfL(TRUE, "SendToLH702: version=%d, printState=%d, id=%d, copies=%d", RX_Config.lh702_protocol_version, _Status.printState, _Status.id, _Status.copies_printed);
	}
}

	//--- lh702_menu ----------------------------------------------
void lh702_menu(char *str)
{
	if (RX_Config.printer.type==printer_LH702)
	{
		SLH702_Value val;
		val.hdr.msgLen = sizeof(val);
		val.value = atoi(&str[1]);
		switch(str[0])
		{
		case '?':	term_printf("--- LH702: commands when not printing -------------\n");					
					term_printf("m<name>:    load material\n");
					term_printf("x:			 list materials\n");
					term_printf("t<microns>: set Thickness\n");
					term_printf("h<microns>: set Head Height above material\n");
					term_printf("e<val>:	    set encoder adjustment value\n");
				
					term_printf("--- LH702: commands while printing ----------------\n");
					term_printf("d<microns>: Add to PrintGo Distace\n");
					term_printf("l<microns>: Add to Page Margin\n");
					term_flush();
					break;	

		case 'd':	val.hdr.msgId  = CMD_ADD_DIST;
					_lh702_handle_msg(INVALID_SOCKET, &val, sizeof(val), NULL, NULL);
					break;

		case 'l':	val.hdr.msgId  = CMD_ADD_LATERAL;
					_lh702_handle_msg(INVALID_SOCKET, &val, sizeof(val), NULL, NULL);
					break;

		case 'x':	
		{			
					char materials[64][64];
					int n = plc_list_materials(materials);
					for (int i = 0; i < n; i++) term_printf("%s\n", materials[i]);
					term_flush();
					break;
		}

		case 'm':
					lh702_load_material(&str[1]);	   
					break;

		case 't':	// _handle_thickness(atoi(&str[1]));  
					val.hdr.msgId  = PAR_THICKNESS;
					_lh702_handle_msg(INVALID_SOCKET, &val, sizeof(val), NULL, NULL);					
					break;

		case 'h':	// _handle_headheight(atoi(&str[1])); 
					val.hdr.msgId  = PAR_HEAD_HEIGHT;
					_lh702_handle_msg(INVALID_SOCKET, &val, sizeof(val), NULL, NULL);					
					break;

		case 'e':	// _handle_encoffset(atoi(&str[1]));
					val.hdr.msgId  = PAR_ENCODER_ADJ;
					_lh702_handle_msg(INVALID_SOCKET, &val, sizeof(val), NULL, NULL);					
					break;
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
	_lastQuery = rx_get_ticks();
	if (len)
	{
		net_register_by_device(dev_plc, -1);

		//Error(LOG, 0, "received msg (len=%d, id=0x%08x)", phdr->msgLen, phdr->msgId);
		switch(phdr->msgId)
		{
		case CMD_START_PRINTING:  	Error(LOG, 0, "DM5 -> CMD_START_PRINTING");	
									pc_start_printing();
									break;
			
		case CMD_STOP_PRINTING:		Error(LOG, 0, "DM5 -> CMD_STOP_PRINTING");	
									pc_abort_printing();
									break;
			
		case CMD_CHANGE_JOB:		Error(LOG, 0, "DM5 -> CMD_CHANGE_JOB");		
									pc_change_job();
									break;
		
        case CMD_GET_STATE:			//Error(LOG, 0, "DM5 -> CMD_GET_STATE");		
									_lh702_send_status();
									break;

		case CMD_ACK_STOP_REQUEST:  Error(LOG, 0, "DM5 -> CMD_ACK_STOP_REQUEST");
									machine_error_reset();
									_Status2.stop_printing_request = FALSE;
									_Status3.stop_printing_request = FALSE;
									break;

		case CMD_LIST_MATERIALS:	Error(LOG, 0, "DM5 -> CMD_LIST_MATERIALS");
									_lh702_send_materials();
									break;

		case PAR_MATERIAL:			Error(LOG, 0, "DM5 -> PAR_MATERIAL=>>%s<<", str->str);
									lh702_load_material(str->str);
									break;
			
		case PAR_THICKNESS:			Error(LOG, 0, "DM5 -> PAR_THICKNESS=%d", val->value);
									_handle_thickness(val->value);
									break;
		
		case PAR_HEAD_HEIGHT:		Error(LOG, 0, "DM5 -> PAR_HEAD_HEIGHT=%d", val->value);
									_handle_headheight(val->value);
									break;
			
		case PAR_ENCODER_ADJ:		Error(LOG, 0, "DM5 -> PAR_ENCODER_ADJ=%d", val->value);
									_handle_encoffset(val->value);
									break;
			
		case CMD_ADD_DIST:			Error(LOG, 0, "DM5 -> CMD_ADD_DIST=%d", val->value);
									_handle_dist(val->value);
									break;
			
		case CMD_ADD_LATERAL:		Error(LOG, 0, "DM5 -> CMD_ADD_LATERAL=%d", val->value);
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
		_pItem = pq_get_item(_pItem);
		memcpy(&evt.item, _pItem, sizeof(evt.item));
		int old=evt.item.printGoDist;
		evt.item.printGoDist += value;
		Error(LOG, 0, "CMD_ADD_DIST %d + %d = %d", old, value, evt.item.printGoDist);
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
		_pItem = pq_get_item(_pItem);
		memcpy(&evt.item, _pItem, sizeof(evt.item));
		int old=evt.item.pageMargin;
		evt.item.pageMargin += value;
		Error(LOG, 0, "CMD_ADD_LATERAL %d + %d = %d", old, value, evt.item.pageMargin);
		handle_gui_msg(INVALID_SOCKET, &evt, evt.hdr.msgLen, NULL, 0);
	}
}

//--- _lh702_send_materials ---------------------------
void _lh702_send_materials(void)
{
	_Materials.length = plc_list_materials(_Materials.materials);
	sok_send(&_Socket, &_Materials);
}

//--- lh702_load_material ---------------------------
void lh702_load_material(char *material)
{
	if (RX_Config.printer.type==printer_LH702) plc_load_material(material);
}

//--- _plc_set_var ------------------------------------
static void _plc_set_var(const char *format, ...)
{
	SStringCmd cmd;
	int len;
	va_list args;
	va_start (args, format);
	len=sprintf(cmd.str, UnitID "\n");
	len+=vsprintf(&cmd.str[len], format, args);
	len=sprintf(&cmd.str[len],"\n");	
	va_end (args);
	cmd.hdr.msgId  = CMD_PLC_SET_VAR;
	cmd.hdr.msgLen = (int)(sizeof(cmd.hdr)+strlen(cmd.str)+1);
	plc_handle_gui_msg(INVALID_SOCKET, &cmd, cmd.hdr.msgLen);
}

//--- _handle_thickness --------------------------
static void _handle_thickness(int value)
{
	_plc_set_var("XML_MATERIAL_THICKNESS=%f", value/1000.0);
}

//--- _handle_headheight --------------------------
static void _handle_headheight(int value)
{
	_plc_set_var("XML_HEAD_HEIGHT=%f", value/1000.0);
}

//--- _handle_encoffset --------------------------
static void _handle_encoffset(int value)
{
	_plc_set_var("XML_ENC_OFFSET=%d", value);
}

//--- lh702_ctr_init ------------------------------
void lh702_ctr_init(void)
{	
	if (rx_file_exists(PATH_USER FILENAME_COUNTERS_LH702))
	{
		UCHAR	check1[64];
		UCHAR	check2[64];
		//--- read file ------------	
		HANDLE file = setup_create();
		setup_load(file, PATH_USER FILENAME_COUNTERS_LH702);
	
		if (setup_chapter(file, "Counters", -1, READ)==REPLY_OK)
		{
			setup_int64 (file, "black",   READ, &RX_PrinterStatus.counterLH702[CTR_LH702_K], 0);
			setup_int64 (file, "color",	  READ, &RX_PrinterStatus.counterLH702[CTR_LH702_COLOR], 0);
			setup_int64 (file, "color_w", READ, &RX_PrinterStatus.counterLH702[CTR_LH702_COLOR_W], 0);
			setup_str   (file, "check",  READ, check1, sizeof(check1), "");
		}
		setup_destroy(file);
	
		_ctr_calc_check(rx_file_get_mtime(PATH_USER FILENAME_COUNTERS_LH702), check2);
	
		_Manipulated = (strcmp(check1, check2))!=0;
		if (_Manipulated)
		{
			ctr_calc_reset_key(RX_Hostname, check2);
			if (!strcmp(check1, check2))
			{
				_Manipulated = FALSE;
				RX_PrinterStatus.counterTotal=0;
			}
		}
		if (_Manipulated && !rx_def_is_test(RX_Config.printer.type)) 
			Error(ERR_CONT, 0, "Counters manipulated");
	
		lh702_ctr_save(FALSE, NULL);	
	}
}

//--- lh702_ctr_save ----------------------------------------------------------------
void lh702_ctr_save(int reset, char *machineName)
{
	if (RX_Config.printer.type==printer_LH702)
	{
		char   name[64];
		UCHAR  check[64];
		time_t time=rx_file_get_mtime(PATH_USER FILENAME_COUNTERS_LH702);

		HANDLE file = setup_create();
		if (reset) 
		{	
			RX_PrinterStatus.counterTotal = 0;
			strncpy(name, machineName, sizeof(name)-1);
		}
		else strncpy(name, RX_Hostname, sizeof(name)-1);

		if (setup_chapter(file, "Counters", -1, WRITE)==REPLY_OK)
		{
			setup_int64 (file, "black",   WRITE, &RX_PrinterStatus.counterLH702[CTR_LH702_K], 0);
			setup_int64 (file, "color",	  WRITE, &RX_PrinterStatus.counterLH702[CTR_LH702_COLOR], 0);
			setup_int64 (file, "color_w", WRITE, &RX_PrinterStatus.counterLH702[CTR_LH702_COLOR_W], 0);

			if (reset)
			{
				time = rx_get_system_sec();
				ctr_calc_reset_key(name, check);
			}
			else if (_Manipulated) 
			{
				if (rx_def_is_test(RX_Config.printer.type)) 
				{
					strcpy(check, "TEST");
				}
				else 
				{
					Error(ERR_CONT, 0, "Counters manipulated");
					strcpy(check, "Manipulated");
				}
			}
			else
			{
				time = rx_get_system_sec();
				_ctr_calc_check(time, check);
			}
			setup_str	(file, "check", WRITE, check, sizeof(check), "");
		}

	//	rx_file_set_readonly(PATH_USER FILENAME_COUNTERS, FALSE);
		setup_save(file, PATH_USER FILENAME_COUNTERS_LH702);
		setup_destroy(file);
	//	rx_file_set_readonly(PATH_USER FILENAME_COUNTERS, TRUE);
		rx_file_set_mtime(PATH_USER FILENAME_COUNTERS_LH702, time);
	}
}

//--- _ctr_calc_check ---------------------------------
static void _ctr_calc_check(time_t time, UCHAR *check)
{
	_sctr ctr;
	memset(&ctr, 0, sizeof(ctr));
	sok_get_mac_address("em2", &ctr.macAddr);
	ctr.time  = time;
	for (int i=0; i<3; i++) ctr.counter[i] = RX_PrinterStatus.counterLH702[i];
	rx_hash_mem_str((UCHAR*)&ctr, sizeof(ctr), check);
}

//--- lh702_ctr_add -----------------------------------------
void lh702_ctr_add(int mm, UINT32 colors)
{
	int color;
	int ctr=-1;
	for (color=0; color<32; color++)
	{
		if (colors & (1<<color))
		{
			if      (str_start(RX_ColorName[min(RX_Color[color].color.colorCode, SIZEOF(RX_ColorName)-1)].name, "Black")) ctr = max(ctr, CTR_LH702_K);
			else if (str_start(RX_ColorName[min(RX_Color[color].color.colorCode, SIZEOF(RX_ColorName)-1)].name, "White")) ctr = max(ctr, CTR_LH702_COLOR_W);
			else ctr = max(ctr, CTR_LH702_COLOR);
		}
	}
	if (ctr>=0) RX_PrinterStatus.counterLH702[ctr] += mm;
}
