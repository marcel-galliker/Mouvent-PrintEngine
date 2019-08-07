// ****************************************************************************
//
//	plc_ctrl.cpp
//
// ****************************************************************************
//
//	Copyright 2014 by Radex AG, Switzerland. All rights reserved.
//	Written by Marcel Galliker 
//
// ****************************************************************************

// static void _plc_state_ctrl();

//--- includes ----------------------------------------------------------------
#include "rx_error.h"
#include "rx_threads.h"
#include "rx_trace.h"
#include "gui_svr.h"
#include "args.h"
#include "ctr.h"
#include "tcp_ip.h"
#include "setup.h"
#include "rx_setup_file.h"
#include "network.h"
#include "rx_rexroth.h"
#include "enc_ctrl.h"
#include "step_ctrl.h"
#include "step_lb.h"
#include "ctrl_svr.h"
#include "print_ctrl.h"
#include "chiller.h"
#include "plc_ctrl.h"

//--- SIMULATION ----------------------------------------------

static int _SimuPLC;
static int _SimuEncoder;

//--- defines -----------------------------------------
#define NAME_APP		"Application.app"
#define NAME_ACCOUNTS	"accounts.xml"

#define APP				"Application.GUI_00_001_Main."

#define UPDATE_ALL		FALSE

typedef struct
{
	INT32	bidir;
	INT32	speed;
	double	stepDist;
	double	startPos;
	double	endPos;	
} SPlcPar;

typedef enum
{
	plc_undef,		//	00
    plc_error,		//	01
    plc_bootup,		//	02
    plc_stop,		//	03
    plc_prepare,	//	04
    plc_pause,		//	05
    plc_run,		//	06
    plc_setup,		//	07
    plc_warmup,		//	08
    plc_webin,		//	09
    plc_washing,	//	10
    plc_cleaning,	//	11
    plc_glue,		//	12
    plc_referencing,//	13
    plc_service,	//	14
	plc_webout,		//  15
	plc_maintenance	//  16
} EnPlcState;

static char* _PlcModeOfState[] =
{
	"", // plc_undef,		//	00
    "", // plc_error,		//	01
    "", // plc_bootup,		//	02
    "CMD_PRODUCTION",	// plc_stop,		//	03
    "CMD_PRODUCTION",	// plc_prepare,		//	04
    "CMD_PRODUCTION",	// plc_pause,		//	05
    "CMD_PRODUCTION",	// plc_run,			//	06
    "CMD_SETUP",		// plc_setup,		//	07
    "CMD_SETUP",		// plc_warmup,		//	08
    "CMD_SETUP",		// plc_webin,		//	09
    "CMD_SETUP",		// plc_washing,		//	10
    "CMD_SETUP",		// plc_cleaning,	//	11
    "CMD_SETUP",		// plc_glue,		//	12
    "CMD_SETUP",		// plc_referencing,	//	13
    "CMD_SERVICE",		// plc_service,		//	14
	"CMD_SETUP",		// plc_webout,		//  15
	"",					// plc_maintenance	//  16			
};

typedef enum
{
	scan_moving,	// 00
	scan_start,		// 01
	scan_end,		// 02
	scan_capping,	// 03
	scan_purge,		// 04
	scan_wipe,		// 05
	scan_manual,	// 06
	scan_stopped,	// 07
} EnScanState;

//--- prototypes -----------------------
static void* _plc_thread(void *par);
static void* _plc_simu_thread(void *par);
static void _simu_init(void);
static void _plc_set_time();
static void _plc_set_config();
static void _plc_get_status();
static void _plc_heart_beat();
static void _plc_state_ctrl();

static void _do_plc_get_info(RX_SOCKET socket);
static void _do_plc_get_log (RX_SOCKET socket);

static void _plc_load_par	(void);
static void _plc_save_par	(void);
static void _plc_get_var	(RX_SOCKET socket, char *varList);
static void _plc_set_var	(RX_SOCKET socket, char *varList);
static void _plc_set_cmd	(RX_SOCKET socket, char *cmd);
static void _plc_req_material	(RX_SOCKET socket, char *filename, int cmd);
static void _plc_save_material	(RX_SOCKET socket, char *filename, int cmd, char *varList);
static void _plc_del_material	(RX_SOCKET socket, char *filename, int cmd, char *name);

static ELogItemType  _plc_error_filter(SPlcLogItem *pItem, char *text);
static void _plc_error_filter_reset(void);

static void _plc_set_command(char *mode, char *cmd);
static void _plc_set_par_default(void);
static void _plc_set_par(SPrintQueueItem *pItem, SPlcPar *pPlcPar);
static void _plc_send_par(SPlcPar *pPlcPar);


//--- statics -----------------------------------------------------------------
static int				_PlcAppInitialized=FALSE;
static int				_PlcThreadRunning;
static int				_RunTime=0;
static SNetworkItem		_NetItem;
static ULONG			_LastLogIdx=0;
static int				_StartPrinting=FALSE;
static int				_RequestPause=FALSE;
static int				_SendPause=FALSE;
static int				_SendRun=FALSE;
static int				_SendWebIn=FALSE;
static int				_CanRun=FALSE;
static SPrintQueueItem	_StartEncoderItem;
static int				_ErrorFlags;
static int				_ErrorFilter=0;
static int				_ErrorFilterBuf[100];
#define ERROR_FILTER_TIME	500
static double			_StepDist;
static int				_UnwinderLenMin;

static int				_MpliStarting;
static int				_UvUsed;
static int				_Speed;

static EnPlcState		_PlcState	= plc_undef;
static int				_ResetError = TRUE;

static int				_TEST_UV_Lamp_Mode[5] = {0,0,0,0,0};
static int				_TEST_feeders=0;

static int				_PAR_BELT_ON=TRUE;
static int				_PAR_WINDER_1_ON=TRUE;
static int				_PAR_WINDER_2_ON=TRUE;

static int				_heads_to_print=FALSE;

	
//--- plc_init ----------------------------------------------------------------
int	plc_init(void)
{
	_PlcThreadRunning = TRUE;
	_SimuPLC		  = arg_simuPLC;
	_SimuEncoder	  = arg_simuEncoder;			
	
	memset(&_StartEncoderItem, 0, sizeof(_StartEncoderItem));
	_plc_error_filter_reset();
	_ErrorFlags  = 0;
	_UvUsed = FALSE;
	RX_PrinterStatus.splicing = FALSE;
	
	if (_SimuPLC)     Error(WARN, 0, "PLC in Simulation");
	if (_SimuEncoder) Error(WARN, 0, "Encoder in Simulation");
	
	if (RX_Config.printer.type==printer_LH702) _SimuPLC = TRUE;
	
	if (_SimuPLC) rx_thread_start(_plc_simu_thread, NULL, 0, "_plc_simu_thread");
	else		  rx_thread_start(_plc_thread, NULL, 0, "_plc_thread");

//	_simu_init();

	return REPLY_OK;
}

//--- plc_end -----------------------------------------------------------------
int plc_end(void)
{
	_PlcThreadRunning = FALSE;
	return REPLY_OK;
}

//--- plc_reset -----------------------------------------------------------
void plc_reset(void)
{
}

//--- plc_error_reset -----------------------------------------
void plc_error_reset(void)
{
	_ErrorFlags  = 0;
	_plc_error_filter_reset();
	if (rex_is_connected())
	{
		_plc_set_command("", "CMD_CLEAR_ERROR");
	//	sys_reset_error();
	}
}

//--- plc_get_scanner_pos -----------------------------
UINT32 plc_get_scanner_pos(void)
{
	FLOAT pos;
	if (_SimuPLC)	pos=(FLOAT)70.12;
	else			lc_get_value_by_name_FLOAT(APP "STA_SLIDE_POSITION_ABSOLUTE", &pos);
	return (UINT32)(1000*pos);
}

//--- _plc_set_par_default -----------------
static void _plc_set_par_default(void)
{
	float start, end;
	UINT32 speed, tension;

	if (_SimuPLC) return;	
		
	lc_get_value_by_name_UINT32(APP"CFG_TENSION_SENSOR_OFFSET_1", &tension);
	TrPrintf(TRUE, "_plc_set_par_default: tension=%d", tension);
	if (tension==0) 
		rex_load(PATH_USER FILENAME_PLC_CFG);

	lc_get_value_by_name_UINT32(APP"PAR_PRINTING_SPEED",			&speed);
	if (speed<10) lc_set_value_by_name_UINT32(APP"PAR_PRINTING_SPEED",		 30);

	if(rx_def_is_scanning(RX_Config.printer.type))
	{
		lc_get_value_by_name_FLOAT(APP"PAR_PRINTING_START_POSITION", &start);
		lc_get_value_by_name_FLOAT(APP"PAR_PRINTING_END_POSITION", &end);
		if(start == 0.0 || end == 0.0)
		{
			lc_set_value_by_name_FLOAT(APP"PAR_PRINTING_START_POSITION", 200.0);	
			lc_set_value_by_name_FLOAT(APP"PAR_PRINTING_END_POSITION", 700.0);
		//	_plc_set_command("", "CMD_SET_PARAMETER");
		}
	}
	else if (rx_def_is_web(RX_Config.printer.type))
	{
		UINT32 val;
		lc_get_value_by_name_UINT32(APP"PAR_COREDIAMETER_IN", &val);
		if (val<80)  Error(WARN, 0, "Small Unwinder Core Diameter: %d mm", val);
		if (val>110) Error(WARN, 0, "Large Unwinder Core Diameter: %d mm", val);
		
		lc_get_value_by_name_UINT32(APP"PAR_COREDIAMETER_OUT", &val);
		if (val<80)  Error(WARN, 0, "Small Rewinder Core Diameter: %d mm", val);
		if (val>110) Error(WARN, 0, "Large Rewinder Core Diameter: %d mm", val);
	}
}

//--- _plc_set_command -----------------------------------------------
static void _plc_set_command(char *mode, char *cmd)
{
	char str[128];

	if (*mode)
	{
		if (strcmp(_PlcModeOfState[_PlcState], mode))
		{
// Label	if (!strcmp(_PlcModeOfState[_PlcState], "CMD_SETUP") && _PlcState!=plc_setup)  // SUB-Menu of SETUP?
			if (!strcmp(_PlcModeOfState[_PlcState], "CMD_SETUP"))  // SUB-Menu of SETUP?
			{
				strcpy(str, APP "CMD_SETUP");
				lc_set_value_by_name_UINT32(str, 1);
				rx_sleep(250);
			}	
			sprintf(str, APP "%s", mode);
			lc_set_value_by_name_UINT32(str, 1);
			rx_sleep(250);
		}
	}
	if (strstr(cmd, "WEBIN")) 
	{
		_plc_set_par_default();
		if (!arg_simuEncoder && !RX_StepperStatus.info.ref_done) 
		{
			step_handle_gui_msg(INVALID_SOCKET, CMD_CAP_REFERENCE, NULL, 0);
			_SendWebIn = TRUE;
			return;
		}
		RX_PrinterStatus.printState = ps_webin;
		gui_send_printer_status(&RX_PrinterStatus);
	}
	if (!strcmp(cmd, "CMD_CLEAR_ERROR"))
	{
		_ErrorFlags  = 0;
		_plc_error_filter_reset();
	    sys_reset_error();
		rx_sleep(200);
	}
	if (!strcmp(cmd, "CMD_PAUSE"))
	{
		Error(LOG, 0, "CMD_PAUSE _SendPause=%d", _SendPause);
		if (_SendPause == 1) _SendPause = 2; 
		else _SendPause = 0;
	}
//	Error(LOG, 0, "PLC_COMMAND >>%s<<", cmd);
	sprintf(str, APP "%s", cmd);
	lc_set_value_by_name_UINT32(str, 1);
}

//--- _plc_set_par ----------------------------------------------------------
static void _plc_set_par(SPrintQueueItem *pItem, SPlcPar *pPlcPar)
{
	double		accDistmm=0;
	
	_plc_set_par_default();
	
	switch(RX_Config.printer.type)
	{
		case printer_LB701:		_UnwinderLenMin = 10; break;	
		case printer_LB702_UV:	_UnwinderLenMin = 10; break;	
		case printer_LB702_WB:	_UnwinderLenMin = 20; break;	
		case printer_LH702:		_UnwinderLenMin = 10; break;	
		case printer_DP803:		_UnwinderLenMin = 30; break;	
		default: _UnwinderLenMin = 0;			
	}
		
	pPlcPar->bidir	= (pItem->scanMode==PQ_SCAN_BIDIR);
	pPlcPar->speed	= pItem->speed;
	if (RX_Config.printer.type==printer_cleaf)
	{
		int flexo = lc_get_value_by_name_UINT32(APP"PAR_FLEXO_CONFIGURATION", &flexo);
		if (flexo && (pPlcPar->speed<15))
		{			
			pPlcPar->speed = 15;
			Error(WARN, 0, "Set Speed to %d because flexo used", pPlcPar->speed);
		}
	}
	_StepDist = 43.328;
	if (RX_Config.printer.type==printer_TX802) _StepDist*=2;
	
	if (!RX_Config.printer.overlap) 
	{
		_StepDist -= (128.0*25.4/1200.0);
	}
	
	_StepDist += ((double)(RX_Config.printer.offset.step))/1000.0;	
	if (pItem->passes>1) _StepDist /= pItem->passes;
	if (pItem->testImage==PQ_TEST_SCANNING) _StepDist=0;
	pPlcPar->stepDist = _StepDist;

	if (pItem->speed<20) accDistmm = 100.0;
	
	pPlcPar->startPos = WEB_OFFSET+(pItem->pageMargin)/1000.0-accDistmm;		
	if (pItem->srcHeight<300)  pPlcPar->endPos = WEB_OFFSET+(pItem->pageMargin+300)/1000.0+accDistmm;
	else                       pPlcPar->endPos = WEB_OFFSET+(pItem->pageMargin+pItem->srcHeight)/1000.0+accDistmm;
	pPlcPar->endPos += RX_Config.headDistMax/1000.0;
	pPlcPar->endPos += 10;
}

//--- plc_get_step_dist_mm ------------------------------------------------------
double	 plc_get_step_dist_mm(void)
{
	return _StepDist;
}

//--- plc_get_thickness --------------------
int	plc_get_thickness(void)
{
#ifdef linux
	FLOAT thickness;
	if (_SimuPLC) thickness = 1.0;
	else lc_get_value_by_name_FLOAT(APP"PAR_MATERIAL_THIKNESS",	&thickness);
	return (int)(thickness*1000);
#else
	return 0;
#endif
}

//--- _plc_send_par -------------------------------------
static void _plc_send_par(SPlcPar *pPlcPar)
{
//	Error(LOG, 0, "SET PLC Parameter: PAR_PRINTING_START_POSITION=%d, PAR_PRINTING_END_POSITION=%d", (int)pPlcPar->startPos, (int)pPlcPar->endPos);
	if (_SimuPLC) return;

	if (_PlcState==plc_webin)
	{
		Error(LOG, 0, "_plc_send_par in webin: send CMD_STOP");
		_plc_set_command("CMD_PRODUCTION", "CMD_STOP");
		rx_sleep(200);
	}
	
	/*
	if (RX_Config.printer.type==printer_TX801 || RX_Config.printer.type==printer_TX802)
	{
		lc_get_value_by_name_UINT32(APP"PAR_BELT_ON", &_PAR_BELT_ON);
		lc_get_value_by_name_UINT32(APP"PAR_WINDER_1_ON", &_PAR_WINDER_1_ON);
		lc_get_value_by_name_UINT32(APP"PAR_WINDER_2_ON", &_PAR_WINDER_2_ON);		
	}
	*/
	
	switch(RX_Config.printer.type)
	{
	case printer_TX801:	lc_set_value_by_name_UINT32(APP"CFG_MACHINE_TYPE",	0);	break;
	case printer_TX802:	lc_set_value_by_name_UINT32(APP"CFG_MACHINE_TYPE",	1);	break;
	default:			break;
	}

	lc_set_value_by_name_UINT32(APP"PAR_PRINTING_SPEED",		pPlcPar->speed);
	if(rx_def_is_scanning(RX_Config.printer.type))
	{
		lc_set_value_by_name_UINT32(APP"PAR_BIDIRECTIONAL", pPlcPar->bidir);
		if(pPlcPar->stepDist > 0) lc_set_value_by_name_FLOAT(APP"PAR_STEP_DISTANCE", (float)pPlcPar->stepDist);
		else 
		{
			lc_set_value_by_name_UINT32(APP"PAR_BELT_ON", 0);
			lc_set_value_by_name_UINT32(APP"PAR_WINDER_1_ON", 0);
			lc_set_value_by_name_UINT32(APP"PAR_WINDER_2_ON", 0);
		}
		lc_set_value_by_name_FLOAT(APP"PAR_PRINTING_START_POSITION", (float)pPlcPar->startPos);	
		lc_set_value_by_name_FLOAT(APP"PAR_PRINTING_END_POSITION", (float)pPlcPar->endPos);
		lc_set_value_by_name_UINT32(APP"PAR_DRYER_BLOWER_POWER", 75);
	}

	_plc_set_command("", "CMD_SET_PARAMETER");	
}

//--- plc_set_printpar -----------------------------------------------
int  plc_set_printpar(SPrintQueueItem *pItem)
{		
	SPlcPar par;
	if ((RX_Config.stepper.ref_height || RX_Config.stepper.print_height) 
	&&  (RX_Config.printer.type==printer_TX801 ||RX_Config.printer.type==printer_TX802))
	{
		// else wait webtension is ok
		_heads_to_print=TRUE;
		tt_cap_to_print_pos();
	}
	
	_UvUsed = (RX_Config.printer.type==printer_cleaf || RX_Config.printer.type==printer_LB701 || RX_Config.printer.type==printer_LB702_UV);

//	Error(LOG, 0, "PrintPar: PageMargin=%d.%03d", pItem->pageMargin/1000, pItem->pageMargin%1000);

	_plc_set_par(pItem, &par);
	_plc_send_par(&par);
	memcpy(&_StartEncoderItem, pItem, sizeof(_StartEncoderItem));
	_Speed = _StartEncoderItem.speed;
	_SendPause = 1;
	
	return REPLY_OK;
}

//--- plc_start_printing -----------------------------------------------
int  plc_start_printing(void)
{
	if (_PlcState!=plc_run)	
	{
		Error(LOG, 0, "_StartPrinting=TRUE");
		_StartPrinting		= TRUE;
		_RequestPause		= FALSE;
	}
	if (_CanRun && !_SimuPLC)
	{		
		plc_error_reset();
		if (rx_def_is_web(RX_Config.printer.type) && RX_Config.printer.type!=printer_cleaf) enc_restart_pg();
		_SendRun = TRUE;
	}
	if (_SimuEncoder) ctrl_simu_encoder(_Speed);
	return REPLY_OK;
}

//--- plc_stop_printing -----------------------------------------------
int  plc_stop_printing(void)
{
	if (RX_Config.printer.type==printer_LH702) steplb_is_printing(FALSE);
	if (_SimuEncoder) ctrl_simu_encoder(0);
		Error(LOG,0, "_StartPrinting=FALSE");
	_StartPrinting = FALSE;
	_SendRun       = FALSE;
	_SendPause	   = FALSE;
	_RequestPause  = FALSE;
	if (_SimuPLC)
	{
		RX_PrinterStatus.printState = ps_off;
		if (chiller_is_running()) RX_PrinterStatus.printState = ps_ready_power;
		gui_send_printer_status(&RX_PrinterStatus);
	}
	else
	{
		Error(LOG, 0, "plc_stop_printing: send CMD_STOP");
		_plc_set_command("CMD_PRODUCTION", "CMD_STOP");
		step_set_vent(FALSE);

		/*
		if (RX_Config.printer.type==printer_TX801 || RX_Config.printer.type==printer_TX802)
		{
			lc_set_value_by_name_UINT32(APP"PAR_WINDER_1_ON", _PAR_WINDER_1_ON);
			lc_set_value_by_name_UINT32(APP"PAR_WINDER_2_ON", _PAR_WINDER_2_ON);
			lc_set_value_by_name_UINT32(APP"PAR_BELT_ON", _PAR_BELT_ON);	
		}
		*/
	}
	return REPLY_OK;
}

//--- plc_abort_printing -----------------------------------------------
int  plc_abort_printing(void)
{
	return plc_stop_printing();
}

//--- plc_clean ---------------------------------------------------------
int  plc_clean(void)
{
	if(rx_def_is_tx(RX_Config.printer.type))
	{
		Error(LOG, 0, "plc_clean: send CMD_STOP");
		_plc_set_command("CMD_PRODUCTION", "CMD_STOP");
		step_set_vent(FALSE);
	}
	return REPLY_OK;
}

//--- plc_to_purge_pos -----------------------------------
int	plc_to_purge_pos(void)
{
	if(rx_def_is_tx(RX_Config.printer.type))
		_plc_set_command("CMD_PRODUCTION", "CMD_SLIDE_TO_PURGE");
	return REPLY_OK;
}

//--- plc_to_wipe_pos ------------------------------
int	plc_to_wipe_pos(void)
{
	if(rx_def_is_tx(RX_Config.printer.type))
		_plc_set_command("CMD_PRODUCTION", "CMD_SLIDE_TO_WIPE");
	return REPLY_OK;
}

//--- plc_to_cap_pos ----------------------------------
int	plc_to_cap_pos(void)
{
	if(rx_def_is_tx(RX_Config.printer.type))
	{
		Error(LOG, 0, "plc_to_cap_pos: send CMD_STOP");
		_plc_set_command("CMD_PRODUCTION", "CMD_STOP");
		step_set_vent(FALSE);			
	}
	return REPLY_OK;
}

//--- plc_pause_printing ---------------------------------------
int  plc_pause_printing(void)
{
	Error(LOG,0, "_StartPrinting=FALSE");
	_StartPrinting = FALSE;
	_SendRun       = FALSE;
	_RequestPause  = FALSE;
	if (_SimuEncoder && rx_def_is_scanning(RX_Config.printer.type)) ctrl_simu_encoder(0);

	if (!_SimuPLC)
	{
		_plc_set_par_default();
//		_plc_set_command("CMD_PRODUCTION", "CMD_PAUSE");
		Error(LOG, 0, "send PAUSE");
		_SendPause = 1;
	}
	return REPLY_OK;
}

//--- plc_handle_gui_msg --------------------------------------------------------
int plc_handle_gui_msg(RX_SOCKET socket, UINT32 cmd, void *data, int dataLen)
{
	switch(cmd)
	{
		//---  general commands ------------------------------------------------------------------------
		case CMD_PLC_RSEET_ERROR:	plc_error_reset();								break;
		case CMD_PLC_GET_INFO:		_do_plc_get_info(socket);						break;
		case CMD_PLC_RESET_ERROR:	sys_reset_error();								break;
		case CMD_PLC_REBOOT:		sys_reboot();									break;				
		case CMD_PLC_GET_LOG:		_do_plc_get_log(socket);						break;

		//--- user commands -----------------------------------------------------------------------------
		case CMD_PLC_LOAD_PAR:		_plc_load_par();								break;
		case CMD_PLC_SAVE_PAR:		_plc_save_par();								break;
		case CMD_PLC_GET_VAR:		_plc_get_var (socket, (char*)data);				break;
		case CMD_PLC_SET_VAR:		_plc_set_var (socket, (char*)data);				break;
		case CMD_PLC_SET_CMD:		_plc_set_cmd (socket, (char*)data);				break;

		//--- material database --------------------------------------------------------
		case CMD_PLC_REQ_MATERIAL:	_plc_req_material  (socket, FILENAME_MATERIAL, cmd); break;
		case CMD_PLC_SAVE_MATERIAL:	_plc_save_material (socket, FILENAME_MATERIAL, CMD_PLC_ITM_MATERIAL, (char*)data);		break;
		case CMD_PLC_DEL_MATERIAL:	_plc_del_material  (socket, FILENAME_MATERIAL, cmd, (char*)data);		break;
	}
	return REPLY_OK;
}

//--- simu_init -----------------------------
static void _simu_init(void)
{
	memset(&_NetItem, 0, sizeof(_NetItem));
	_NetItem.deviceType		= dev_plc;
	strcpy(_NetItem.deviceTypeStr, DeviceStr[dev_plc]);
	_NetItem.deviceNo		= 0;
	strncpy(_NetItem.serialNo, "SIMULATION", sizeof(_NetItem.serialNo));
	net_device_to_ipaddr(_NetItem.deviceType, _NetItem.deviceNo, _NetItem.ipAddr, sizeof(_NetItem.ipAddr));
	_NetItem.connected = TRUE;

	net_register(&_NetItem);
}

//--- plc_simu_thread ---------------------------------------------
static void* _plc_simu_thread(void *par)
{
	_simu_init();
	while (_PlcThreadRunning)
	{
		_plc_state_ctrl();
		rx_sleep(1000);
	}
	return NULL;
}

//--- _plc_load_par -------------------------------------------------
static void _plc_load_par(void)
{
	char value[32];

	rex_load(PATH_USER FILENAME_PLC_CFG);
	rex_load(PATH_USER FILENAME_PLC_PAR);

	if (lc_get_value_by_name(APP "PAR_HEAD_HEIGHT", value)==REPLY_OK)
		RX_Config.stepper.print_height=(UINT32)(1000*atof(value));

	if (lc_get_value_by_name(APP "XML_ENC_OFFSET", value)==REPLY_OK)
		RX_Config.printer.offset.incPerMeter[0] = atoi(value);

	if (rx_def_is_scanning(RX_Config.printer.type))
	{
		lc_set_value_by_name_UINT32(APP"CFG_POSITION_PURGE",	90);				
		lc_set_value_by_name_UINT32(APP"CFG_POSITION_CAPPING",	55);				
		lc_set_value_by_name_UINT32(APP"CFG_POSITION_WIPE",		 0);
	}
	switch(RX_Config.printer.type)
	{
	case printer_TX801:	lc_set_value_by_name_UINT32(APP"CFG_MACHINE_TYPE",	0);	break;
	case printer_TX802:	lc_set_value_by_name_UINT32(APP"CFG_MACHINE_TYPE",	1);	break;
	default:			break;
	}

	_plc_set_command("", "CMD_SET_PARAMETER");
	gui_send_msg_2(0, REP_PLC_LOAD_PAR, 0, NULL);
}

//--- _plc_save_par -------------------------------------------------
static void _plc_save_par	(void)
{
	rex_save(PATH_USER FILENAME_PLC_CFG, ".CFG_");
	rex_save(PATH_USER FILENAME_PLC_PAR, ".PAR_;.XML_");
#ifdef linux
	sync();
#endif
}

//--- _plc_get_cpu_status -------------------------------------------------------
static int _plc_get_cpu_status(char *name, char *answer)
{
	int len;
	if (!strcmp(name, "PLC."))
	{
		len = sprintf(answer, "PLC\n");
		SSystemInfo info;
		sys_get_info(&info);

		len += sprintf(&answer[len], "versionHardware=%s\n",	info.versionHardware);
		len += sprintf(&answer[len], "versionFirmware=%s\n",	info.versionFirmware);
		len += sprintf(&answer[len], "versionLogic=%s\n",		info.versionLogic);
		len += sprintf(&answer[len], "versionBsp=%s\n",			info.versionBsp);
		len += sprintf(&answer[len], "versionMlpi=%s\n",		info.versionMlpi);
		len += sprintf(&answer[len], "serialNo=%s\n",			info.serialNo);
		len += sprintf(&answer[len], "hardwareDetails=%s\n",	info.hardwareDetails);
		len += sprintf(&answer[len], "localBusConfig=%s\n",		info.localBusConfig);
		len += sprintf(&answer[len], "modulBusConfig=%s\n",		info.modulBusConfig);
		len += sprintf(&answer[len], "modulHwDetails=%s\n",		info.modulHwDetails);
		len += sprintf(&answer[len], "tempAct=%.1f\n",			info.tempAct);
		len += sprintf(&answer[len], "tempMax=%.1f\n",			info.tempMax);
	//	len += sprintf(&answer[len], "indexOldest=%d\n",		info.indexOldest);
	//	len += sprintf(&answer[len], "indexNewest=%d\n",		info.indexNewest);
	//	len += sprintf(&answer[len], "indexRx=%d\n",			_LastLogIdx);
				
		UINT32 errNo, mode;
		char value[128];

		sys_get_display_text(&errNo, value, sizeof(value), &mode);
		len += sprintf(&answer[len], "display=%s\n",		value);
		return TRUE;
	}
	return FALSE;
}

//--- _plc_set_cpu_cmd ---------------------------------------------------------
static int _plc_set_cpu_cmd(char *name, char *val)
{
	if (!strcmp(name, "PLC.reset_error"))	{ sys_reset_error(); return TRUE;}
	if (!strcmp(name, "PLC.reset_warm"))	{ app_reset();       return TRUE;}
	if (!strcmp(name, "PLC.reboot"))		{ sys_reboot();      return TRUE;}
	if (!strcmp(name, "PLC.start"))			{ app_start();       return TRUE;}
	if (!strcmp(name, "PLC.stop"))			{ app_stop();        return TRUE;}
	return FALSE;
}

//--- _plc_get_var ------------------------------------------------
static void _plc_get_var(RX_SOCKET socket, char *varList)
{
	if (UPDATE_ALL) return;
	
	static int test=1;
	char name[128];
	char value[128];
	char *answer = (char*)malloc(MAX_MESSAGE_SIZE);
	char *str=varList;
	char *end;
	char *var=name;
	int len=0;
	name[0]=0;
	while ((end=strchr(str, '\n')))
	{
		*end++=0; 
		len += sprintf(&answer[len], "%s", str);
		if (!name[0])
		{
			var = name+sprintf(name, "%s.", str);
			if (_plc_get_cpu_status(name, answer)) break;
		} 
		else
		{
			strcpy(var, str);				
			if (lc_get_value_by_name(name, value)==0) len += sprintf(&answer[len], "=%s", value);
			else									  len += sprintf(&answer[len], "=ERROR");
	//		TrPrintfL(!strcmp(name, "Application.GUI_00_001_Main.PAR_FLEXO_CONFIGURATION"), "_plc_get_var socket=%d >>%s=%s<<", socket, name, value);
		}
		len += sprintf(&answer[len], "\n");
		str = end;
	}

//	printf("answer:len=%d\n%s\n", len, answer);
	sok_send_2(&socket, REP_PLC_GET_VAR, (int)strlen(answer), answer);
	free(answer);
}

//--- _plc_set_var ------------------------------------------------
static void _plc_set_var(RX_SOCKET socket, char *varList)
{
	char name[128];
	char *str=varList;
	char *end;
	char *val;
	char *var=name;
	int ret;
	int toWebin=FALSE;
	name[0]=0;

//	TrPrintf(TRUE, "_plc_set_var");
	while ((end=strchr(str, '\n')))
	{
		*end++=0;
		if (!name[0]) var = name+sprintf(name, "%s.", str);
		else
		{
			val= strchr(str, '=');
			*val++=0;
			strcpy(var, str);
			if (!_plc_set_cpu_cmd(name, val))
			{
				if (!strcmp(var, "CMD_SET_PARAMETER") && RX_PrinterStatus.printState==ps_webin && RX_Config.printer.type!=printer_cleaf)
				{
					Error(LOG, 0, "_plc_set_var: send CMD_STOP");
					_plc_set_command("CMD_PRODUCTION", "CMD_STOP");
					rx_sleep(200);
					toWebin = TRUE;
				}
				ret = lc_set_value_by_name(name, val);
				//---------------------------------------------------

			//	Error(LOG, 0, "SET %s=%s", name, val);
			//	TrPrintfL(TRUE, 0, "SET %s=%s", name, val);
			//	TrPrintfL(!strcmp(name, "Application.GUI_00_001_Main.PAR_FLEXO_CONFIGURATION"), "_plc_set_var socket=%d >>%s=%s<< ret=%d", socket, name, val, ret);
				if (ret)  ErrorFlag(ERR_CONT, &_ErrorFlags, 1, 0, "Writing >>%s<<: Error %s", name, mlpi_get_errmsg());
				if (toWebin)
				{
					rx_sleep(200);
					_plc_set_command("CMD_SETUP", "CMD_WEBIN");	
				}
			}
						
			//--- XML_STEPPER_PRINT_HEIGHT special -----------------------------------------------
			if (!strcmp(var, "PAR_HEAD_HEIGHT")) 
				RX_Config.stepper.print_height = (INT32)(0.5+1000*strtod(val, NULL));
			
			//--- XML_ special -----------------------------------------------
			if (!strcmp(var, "XML_ENC_OFFSET")) 
				RX_Config.printer.offset.incPerMeter[0] = atoi(val);
		}
		str = end;
	}
}

//--- _plc_save_material ----------------------------------------------
static void _plc_save_material	(RX_SOCKET socket, char *filename, int cmd, char *varList)
{
	HANDLE attribute =NULL;
	char attrname[64];
	char path[MAX_PATH];
	char *str=varList;
	char *end;
	char *val;
	char var[128];
	
	gui_send_msg_2(0, cmd, strlen(varList), varList);

	sprintf(path, PATH_USER"%s", filename);
	HANDLE file = setup_create();
	setup_load(file, path);
	
	//--- find material ---
	if ((end=strchr(str, '\n')))
	{
		*end++=0;
		while (TRUE)
		{
			setup_chapter_next(file, READ, attrname, sizeof(attrname));
			if (!*attrname) // not found
			{
				setup_chapter_add(file, "Material");
				break;					
			}
			setup_str(file, "XML_MATERIAL", READ, attrname, sizeof(attrname), "");
			if (!stricmp(attrname, str)) break;	// found
		}
	}
	str = end;

	while ((end=strchr(str, '\n')))
	{
		*end++=0;
		val= strchr(str, '=');
		*val++=0;
		strcpy(var, str);
		printf(">>%s<< = >>%s<<\n",  var, val);
		setup_str(file, var, WRITE,  val,	32,	"");

		if (!strcmp(var, "PAR_HEAD_HEIGHT")) 
			RX_Config.stepper.print_height = (INT32)(0.5+1000*strtod(val, NULL));

		str = end;
	}
	setup_save(file, path);
	setup_destroy(file);
}

//--- _plc_del_material ----------------------------------------------
static void _plc_del_material	(RX_SOCKET socket, char *filename, int cmd, char *name)
{
	char attrname[64];
	char val[64];
	char path[2048];
	int len;
	
	gui_send_msg_2(0, cmd, strlen(name), name);	

	HANDLE file = setup_create();
	HANDLE attribute =NULL;
	sprintf(path, PATH_USER"%s", filename);
	setup_load(file, path);
	while (TRUE)
	{
		len=0;
		setup_chapter_next(file, READ, attrname, sizeof(attrname));
		if (!*attrname) 
			return;
		setup_str(file, "XML_MATERIAL", READ, val, sizeof(val), "");
		if (!stricmp(val, name))
		{
			setup_chapter_delete(file);
			setup_save(file, path);
			setup_destroy(file);
			return;					
		}
	}	
}

//--- _plc_req_material ----------------------------------------------
static void _plc_req_material	(RX_SOCKET socket, char *filename, int cmd)
{
	char name[64];
	char val[64];
	char str[2048];
	int len;
	
	cmd &= 0xffff;
	HANDLE file = setup_create();
	HANDLE attribute =NULL;
	sprintf(str, PATH_USER"%s", filename);
	setup_load(file, str);
	sok_send_2(&socket, cmd | REP_X, 0, NULL);	// CMD_PLC_RES_MATERIAL | CMD_PLC_REQ_SPLICEPAR
	while (TRUE)
	{
		len=0;
		setup_chapter_next(file, READ, name, sizeof(name));
		if (!*name) return;
	//	len = sprintf(str, "%s\n", name);
		while (TRUE)
		{
			setup_str_next(file, &attribute, name, sizeof(name), val, sizeof(val));
			if (len==0) len = sprintf(str, "%s\n", val);
			if (!*name) break;
			len += sprintf(&str[len], "%s=%s\n", name, val);
			
			if (!strcmp(name, "PAR_HEAD_HEIGHT")) 
				RX_Config.stepper.print_height = (INT32)(0.5+1000*strtod(val, NULL));

		}
		str[len]=0;
		sok_send_2(&socket, cmd | EVT_X, len, str);	// CMD_PLC_ITM_MATERIAL | CMD_PLC_ITM_SPLICEPAR
	}
}

//--- _plc_set_cmd --------------------------------------------------------------
static void _plc_set_cmd(RX_SOCKET socket, char *cmd)
{
	char mode[128];
	int i;
	strcpy(mode, cmd);
	for (i=0; mode[i]; i++)
	{
		if (mode[i]=='/') 
		{
			mode[i]=0;
			_plc_set_command(mode, &mode[i+1]);
			return;
		}
	}
	_plc_set_command("", mode);
}

//--- _OnPlcConnected ---------------------------------------------
static void _OnPlcConnected(void)
{
	SSystemInfo info;
	int i;
	ULONG no;

	if (!_PlcAppInitialized) return;

//	_Status.plcConnected = TRUE;
	TrPrintfL(1, "plc: Connected");
	sys_get_info(&info);
	strncpy(_NetItem.serialNo, info.serialNo, sizeof(_NetItem.serialNo));
	no = atoi(&_NetItem.serialNo[4]);
	_NetItem.macAddr=0;
	for (i=0; i<6; i++)
	{
		_NetItem.macAddr <<= 8;
		_NetItem.macAddr +=    (no%10); no=no/10;
		_NetItem.macAddr += 16*(no%10); no=no/10;
	}
	_NetItem.connected = TRUE;
	net_register(&_NetItem);
//	_plc_set_time();
	_plc_set_config();
}

//--- _OnPlcDisconnected ---------------------------------------------
static void _OnPlcDisconnected(void)
{
//	_Status.plcConnected = FALSE;
	TrPrintfL(1, "plc: Disconnected");
	_NetItem.connected = FALSE;
	net_disconnnected(&_NetItem);
}

//--- _OnPlcError ---------------------------------------------
static void _OnPlcError(void)
{
}

//--- plc_set_config -----------------------------------------
static void _plc_set_config()
{
	_plc_load_par();
	switch(RX_Config.printer.type)
	{
	case printer_TX801:
	case printer_TX802:
		sys_set_axes_name( 1, "ScanIn");
		sys_set_axes_name( 2, "ScanOut");
		sys_set_axes_name( 3, "ScanChain");
		sys_set_axes_name( 4, "Belt");
		sys_set_axes_name( 5, "Unwinder");
		sys_set_axes_name( 6, "BufInL");
		sys_set_axes_name( 7, "BufInR");
		sys_set_axes_name( 8, "PressRollerL");
		sys_set_axes_name( 9, "PressRollerR");
		sys_set_axes_name(10, "Brushes");
		sys_set_axes_name(11, "BufOutL");
		sys_set_axes_name(12, "BufOutR");
		sys_set_axes_name(13, "Rewinder");
		break;
		
	case printer_LB701:
	case printer_LB702_UV:
		sys_set_axes_name( 1, "Unwinder");
		sys_set_axes_name( 2, "PrinterIN");
		sys_set_axes_name( 3, "PrinterOut");
		sys_set_axes_name( 4, "Rewinder");		
		break;
	
		/*
	case printer_LB701: with finishing
		sys_set_axes_name( 1, "Unwinder");
		sys_set_axes_name( 2, "PrinterIN");
		sys_set_axes_name( 3, "PrinterOut");
		sys_set_axes_name( 4, "DieCutter");
		sys_set_axes_name( 5, "DieNip");
		sys_set_axes_name( 6, "RewinderGrid");
		sys_set_axes_name( 7, "Rewinder1");
		sys_set_axes_name( 8, "Rewinder2");
		break;
		*/
		
	case printer_LB702_WB:
		sys_set_axes_name( 1, "Unwinder");
		sys_set_axes_name( 2, "FlexoPreNip");
		sys_set_axes_name( 3, "FlexoPreCylinder");
		sys_set_axes_name( 4, "FlexoPreCliche");		
		sys_set_axes_name( 5, "PrinterIn");		
		sys_set_axes_name( 6, "PrinterOut");		
		sys_set_axes_name( 7, "FlexoPostNip");		
		sys_set_axes_name( 8, "FlexoPostCliche");		
		sys_set_axes_name( 9, "FlexoPostCliche");		
		sys_set_axes_name(10, "Rewinder");		
		break;

	case printer_DP803:
		sys_set_axes_name( 1, "Unwinder");
		sys_set_axes_name( 2, "Rewinder");
		sys_set_axes_name( 3, "LiftingUnwinder");
		sys_set_axes_name( 4, "PrinterIn");		
		sys_set_axes_name( 5, "PrinterOut");		
		sys_set_axes_name( 6, "LiftingRewinder");		
		break;
		
	case printer_cleaf:
		sys_set_axes_name( 1, "UwR");
		sys_set_axes_name( 2, "UwL");
		sys_set_axes_name( 3, "UwSplInR");
		sys_set_axes_name( 4, "UwSplInL");
		sys_set_axes_name( 5, "UwSplOut");
		sys_set_axes_name( 6, "UwBuf");
		sys_set_axes_name( 7, "Flexo1");
		sys_set_axes_name( 8, "Flexo2");
		sys_set_axes_name( 9, "PrinterIn");
		sys_set_axes_name(10, "PrinterOut");
		sys_set_axes_name(11, "Flexo3");
		sys_set_axes_name( 2, "Flexo4");
		sys_set_axes_name(13, "RwBuf");
		sys_set_axes_name(14, "RwSpl");
		sys_set_axes_name(15, "RwR");
		sys_set_axes_name(16, "RwL");
		break;
	}
}

//--- _plc_set_time ----------------------------------------
static void _plc_set_time()
{
	#ifdef WIN32
		SYSTEMTIME now;
		GetSystemTime(&now);

		SSystemTime time;
		memset(&time, 0, sizeof(time));
		time.year   = now.wYear;
		time.month  = now.wMonth;
		time.day    = now.wDay;
		time.hour   = now.wHour;
		time.minute = now.wMinute;
		time.second = now.wSecond;
		sys_set_time(&time);
	#endif
}

//--- _plc_error_filter ----------------------------------
static ELogItemType _plc_error_filter(SPlcLogItem *pItem, char *text)
{
	char *t;
	
	//--- Mouvent messages ---------------------------------
	if(t= strstr(pItem->text, "ERR:")) 
	{
		strcpy(text, &t[4]);		
		return LOG_TYPE_ERROR_CONT;
	}
	if(t=strstr(pItem->text, "WRN:")) 
	{
		strcpy(text, &t[4]);		
		return LOG_TYPE_WARN;
	}
	if(t=strstr(pItem->text, "LOG:")) 
	{
		strcpy(text, &t[4]);		
		return LOG_TYPE_LOG;
	}
	
	//--- Rexroth messages -----------------------
	ELogItemType logType=LOG_TYPE_UNDEF;

	if      ((pItem->errNo & 0xf0000000) == 0xe0000000) return LOG_TYPE_UNDEF;	// ignore warnings
	else if ((pItem->errNo & 0xf0000000) == 0xf0000000) logType=LOG_TYPE_ERROR_CONT;
	else logType=LOG_TYPE_LOG;
		
	strcpy(text, pItem->text);
	for (int i=0; i<SIZEOF(_ErrorFilterBuf); i++)
	{
		if (_ErrorFilterBuf[i]==pItem->errNo) return LOG_TYPE_UNDEF;		
		if (_ErrorFilterBuf[i]==0)
		{
			_ErrorFilterBuf[i] = pItem->errNo;
			return logType;
		}
	}
	return logType;
}

//--- _plc_error_filter_reset ----------------------------
static void _plc_error_filter_reset(void)
{
	_ErrorFilter = 0; 
	memset(_ErrorFilterBuf, 0, sizeof(_ErrorFilterBuf));
};


//--- _plc_get_status ---------------------------------
static void _plc_get_status()
{
	ELogItemType logType;
	SPlcLogItem item;
	char text[MAX_PATH];
	int ticks=rx_get_ticks();
	while(sys_get_new_log_item(&item, &_LastLogIdx) == REPLY_OK)
	{
		if(_ErrorFilter == 0 || rx_get_ticks() > _ErrorFilter) 
		{
			gui_send_msg_2(0, REP_PLC_GET_LOG, sizeof(item), &item);
			if(!_MpliStarting)
			{
				if (item.state == active)
				{
					logType=_plc_error_filter(&item, text);
					if(logType)
					{
						if(logType==LOG_TYPE_ERROR_CONT)	{   Error(ERR_CONT,	0, "PLC (%X): %s", item.errNo, text); _ErrorFilter = rx_get_ticks() + ERROR_FILTER_TIME; }
						else if(logType==LOG_TYPE_WARN)			Error(WARN,		0, "PLC (%X): %s", item.errNo, text);
						else                                    Error(LOG,		0, "PLC (%X): %s", item.errNo, text);
					}
					else if(item.state == message) Error(LOG, 0, "PLC (%X): %s", item.errNo, item.text);					
				}				
			}
		}
	}
	_MpliStarting=FALSE;
}

//--- _plc_heart_beat ------------------------
static void _plc_heart_beat()
{
	int ret;
	UINT32 val;

	ret=lc_get_value_by_name_UINT32(APP "STA_HEARTBEAT_OUT", &val);
	ret=lc_set_value_by_name_UINT32(APP "USR_HEARTBEAT_IN", val);
}

//--- plc_is_splicing -------------------------------------
int	plc_is_splicing(void)
{
	return 	RX_PrinterStatus.splicing;			
}

//--- _plc_state_ctrl --------------------------------------------
static void _plc_state_ctrl()
{		
	if (_SimuPLC) _PlcState = plc_pause;
	else
	{
		lc_get_value_by_name_UINT32(APP "STA_MACHINE_STATE", (UINT32*)&_PlcState);
		{
			UINT32 splicing;
			if (lc_get_value_by_name_UINT32(APP "STA_SPLICING", &splicing)==REPLY_OK)
			{				
				if (splicing != RX_PrinterStatus.splicing)
				{
					if ( splicing && !RX_PrinterStatus.splicing) Error(LOG, 0, "Splicing started");
					if (RX_PrinterStatus.splicing &&  !splicing) Error(LOG, 0, "Splicing finished");
			
					RX_PrinterStatus.splicing = splicing;
				}
			}
		}
		
	//	if (_PlcState==plc_setup && RX_PrinterStatus.printState==ps_webin && RX_Config.printer.type!=printer_cleaf) _plc_set_command("CMD_SETUP", "CMD_WEBIN");
		if ((RX_Config.stepper.ref_height==0 && RX_Config.stepper.print_height==0) 
			|| RX_StepperStatus.info.z_in_print 
			|| RX_StepperStatus.info.z_in_ref)
		{
			lc_set_value_by_name_UINT32(APP "STA_HEAD_IS_UP", TRUE);						
			if (_SendPause==1)
			{
				_SendPause = 2;
				rx_sleep(200);
				_plc_set_command("CMD_PRODUCTION", "CMD_PAUSE");
				step_set_vent(FALSE);
			}
			if (_SendWebIn)
			{
				_SendWebIn = FALSE;
				rx_sleep(200);
				_plc_set_command("CMD_SETUP", "CMD_WEBIN");
				return;
			}
		}
		else lc_set_value_by_name_UINT32(APP "STA_HEAD_IS_UP", FALSE);	
	}

	/*	Label
	if (_PlcState==plc_setup && RX_PrinterStatus.printState != ps_setup)
	{
		RX_PrinterStatus.printState = ps_setup;
	}
	*/

	if (RX_PrinterStatus.printState==ps_stopping && _PlcState==plc_stop) pc_off();
	if (RX_Config.printer.type!=printer_cleaf && _PlcState==plc_error && _ResetError)
	{		
		_ResetError=FALSE;
		plc_error_reset();
	}
	if (_PlcState==plc_setup && RX_PrinterStatus.printState==ps_webin)
	{
		_plc_set_command("CMD_SETUP", "CMD_WEBIN");
	}
	if(_PlcState == plc_pause)
	{		
		if(_SendPause==2)
		{
			Error(LOG, 0, "_SendPause=%d _StartPrinting=%d", _SendPause, _StartPrinting);
			if (!_StartPrinting) RX_PrinterStatus.printState = ps_pause;
			_SendPause = 0;
		}
		if(_SendRun)
		{
			_SendRun = FALSE;
			_plc_set_command("CMD_PRODUCTION", "CMD_RUN");
			step_set_vent(_Speed);
			RX_PrinterStatus.printState = ps_printing;
		}
		if(_StartEncoderItem.pageWidth)	// send position to encoder
		{			
			if(RX_Config.printer.type == printer_TX801 || RX_Config.printer.type == printer_TX802)
			{ // calculate speed
				UINT32 speed;
				// speed = time for one scanner movement!
				lc_get_value_by_name_UINT32(APP "STA_PRINTING_CYCLE_TIME", &speed);
				if(speed == 0) RX_PrinterStatus.actSpeed = 0;
				else
				{
					/*
					double stepArea = _StartEncoderItem.srcHeight / 1000000.0 * _StepDist/1000.0;
					if (_StartEncoderItem.scanMode!=PQ_SCAN_BIDIR) stepArea /= 2; 
					RX_PrinterStatus.actSpeed = (UINT32)(stepArea * 3600.0 / ((double)speed/1000.0));
					*/
					if (_StartEncoderItem.scanMode!=PQ_SCAN_BIDIR) speed*=2;
					RX_PrinterStatus.actSpeed = (UINT32)((_StepDist/1000.0 * 3600.0) / ((double)speed/1000.0));
				}
			}
			
			EnScanState scan_state;
			if(_SimuPLC || !rx_def_is_scanning(RX_Config.printer.type)) 
				scan_state = scan_start;
			else lc_get_value_by_name_UINT32(APP "STA_SLIDE_POSITION", (UINT32*)&scan_state);
			if(scan_state != scan_moving)
			{
				enc_start_printing(&_StartEncoderItem);
				_StartEncoderItem.pageWidth = 0;
				return;
			}
		}

		if(!_heads_to_print)
		{			
		//	TrPrintfL(TRUE, "_heads_to_print: printhead_en=%d, printState=%d (%d)", RX_StepperStatus.info.printhead_en, RX_PrinterStatus.printState, ps_printing);
		//	if (RX_Config.printer.type!=printer_cleaf || (RX_StepperStatus.info.printhead_en && RX_PrinterStatus.printState==ps_printing))
			if(RX_PrinterStatus.printState == ps_printing && (RX_StepperStatus.info.printhead_en || (RX_Config.printer.type!=printer_cleaf &&  RX_Config.printer.type!=printer_LH702)))
			{
				tt_cap_to_print_pos();
				_heads_to_print = TRUE;													
			}
		}
		
		if(_StartPrinting && _StartEncoderItem.pageWidth == 0)
		{
			Error(LOG, 0, "enc_ready=%d, pq_is_ready2print=%d, printState=%d, z_in_print=%d", enc_ready(), pq_is_ready2print(&_StartEncoderItem), RX_PrinterStatus.printState, RX_StepperStatus.info.z_in_print);
		}

		if(_StartPrinting
			&& _StartEncoderItem.pageWidth == 0 
			&& enc_ready()
			&& pq_is_ready2print(&_StartEncoderItem) 
			&& (RX_PrinterStatus.printState == ps_printing || RX_PrinterStatus.printState == ps_ready_power)
			&& (RX_StepperStatus.info.z_in_print 
			|| _SimuPLC))
		{
			Error(LOG,0, "_StartPrinting=FALSE");
			_StartPrinting = FALSE;
			_CanRun = TRUE;
			if(!_SimuPLC)    _plc_set_command("CMD_PRODUCTION", "CMD_RUN");
			if(_SimuEncoder) ctrl_simu_encoder(_Speed);
			if (RX_Config.printer.type==printer_LH702) steplb_is_printing(TRUE);
			step_set_vent(_Speed);
			memset(&_StartEncoderItem, 0, sizeof(_StartEncoderItem));
			TrPrintfL(TRUE, "PLC: CMD_RUN sent");
		}		
	}
	else if (_PlcState==plc_run)
	{
		if (rx_def_is_web(RX_Config.printer.type)) 
		{
			UINT32 length;
			lc_get_value_by_name_UINT32(APP "STA_PAPERLENGTH_IN", &length);
			if (length && (int)length<_UnwinderLenMin && !_RequestPause) 
			{
				Error(ERR_CONT, 0, "Roll Empty: PAUSE requested");
				RX_PrinterStatus.printState=ps_pause; // suppress pause message
				_RequestPause = TRUE;
				pc_pause_printing();
			}
		}
	}
	else if (_PlcState==plc_stop) 
	{
		RX_PrinterStatus.actSpeed = 0;
		_CanRun = FALSE;
		_heads_to_print=FALSE;
		if (RX_PrinterStatus.printState>plc_stop)
			RX_PrinterStatus.printState = chiller_is_running() ? ps_ready_power : ps_off;
	}
	
	if (rx_def_is_web(RX_Config.printer.type)) 
	{
		static int _time=0;
		int ticks=rx_get_ticks();
		if (ticks-_time>450)
		{			
			_time=ticks;
			lc_get_value_by_name_UINT32(APP "STA_PRINTING_SPEED", &RX_PrinterStatus.actSpeed);
		}
		
		int pause;
		int ret = lc_get_value_by_name_UINT32(APP "STA_PAUSE_REQUEST", &pause);
		if (RX_PrinterStatus.printState==ps_printing && !_RequestPause && pause)
		{
			Error(ERR_CONT, 0, "PAUSE requested by finishing");
			RX_PrinterStatus.printState=ps_pause; // suppress pause message
			_RequestPause = TRUE;
			pc_pause_printing();
		}
	}
	gui_send_printer_status(&RX_PrinterStatus);
}

//--- _do_plc_get_info ---------------------------------------------------
static void _do_plc_get_info(RX_SOCKET socket)
{
	SSystemInfo info;

	if (_SimuPLC)
	{
		memset(&info, 0, sizeof(info));
		sprintf(info.versionHardware,	"version hw");
		sprintf(info.versionFirmware,	"version fw");
		sprintf(info.versionLogic,		"version logic");
		sprintf(info.versionBsp,		"version bsp");
		sprintf(info.versionMlpi,		"version mlpi");
		sprintf(info.serialNo,			"sn:simu 0001");
		sprintf(info.hardwareDetails,	"Hardware Details");
		sprintf(info.localBusConfig,	"Local Bus Config");
		sprintf(info.modulBusConfig,	"Modul Bus Config");
		sprintf(info.modulHwDetails,	"Modul HW Details");
		info.tempAct = (FLOAT)40.0;
		info.tempMax = (FLOAT)43.7;
	}
	else sys_get_info(&info);
	sok_send_2(&socket, REP_PLC_GET_INFO, sizeof(info), &info);
}

//--- _do_plc_get_log ---------------------------------------------------
static void _do_plc_get_log(RX_SOCKET socket)
{
	int i;
	ULONG idx;
	SPlcLogItem item;

	#ifndef linux
	if (_SimuPLC)
	{
		SYSTEMTIME time;
		GetSystemTime(&time);
		for (i=0; i<100; i++)
		{
			memset(&item, 0, sizeof(item));
			item.no = i;
			sprintf(item.date, "%d.%d.%d %d:%d:%d.%d", time.wDay, time.wMonth, time.wYear, time.wHour, time.wMinute, time.wSecond, 0);
			item.errNo = 0x1000+i;
			sprintf(item.text, "Simu Log %03d", i+1);
			sok_send_2(&socket, REP_PLC_GET_LOG, sizeof(item), &item);
		}
	}
	else
	#endif
	{
		for (i=0; i<100; i++)
		{
			if (sys_get_log_item(i, &item, &idx)) break;
			sok_send_2(&socket, REP_PLC_GET_LOG, sizeof(item), &item);
		}
		if (idx>_LastLogIdx) _LastLogIdx = idx;
	}
//	sok_send_2(&socket, END_PLC_GET_LOG, 0, NULL);
	TrPrintfL(1, "PlcLog: %d items sent to %d", i, socket);
}

//--- plc_in_cap_pos ------------------------------------------
int	 plc_in_cap_pos(void)
{
	if(rx_def_is_tx(RX_Config.printer.type))
	{
		EnScanState state;
		lc_get_value_by_name_UINT32(APP "STA_SLIDE_POSITION", (UINT32*)&state);
		return state == scan_capping;	
	}
	return TRUE;
}

//--- plc_in_purge_pos ------------------------------------------
int	 plc_in_purge_pos(void)
{
	if(rx_def_is_tx(RX_Config.printer.type))
	{
		EnScanState state;
		lc_get_value_by_name_UINT32(APP "STA_SLIDE_POSITION", (UINT32*)&state);
		return state==scan_purge;
	}
	return TRUE;
}

//--- plc_in_wipe_pos ------------------------------------------
int	 plc_in_wipe_pos(void)
{
	if(rx_def_is_scanning(RX_Config.printer.type))
	{
		EnScanState state;
		lc_get_value_by_name_UINT32(APP "STA_SLIDE_POSITION", (UINT32*)&state);
		return state==scan_wipe;
	}
	return TRUE;
}

//--- plc_thread ---------------------------------------------
static void* _plc_thread(void *par)
{
	int ret;
	int config_timer;
	struct 
	{
		SMsgHdr hdr;
		char	data[4096];
	} msg;

	memset(&_NetItem, 0, sizeof(_NetItem));
	_NetItem.deviceType		= dev_plc;
	strcpy(_NetItem.deviceTypeStr, DeviceStr[dev_plc]);
	_NetItem.deviceNo		= 0;
	net_device_to_ipaddr(_NetItem.deviceType, _NetItem.deviceNo, _NetItem.ipAddr, sizeof(_NetItem.ipAddr));
	_NetItem.connected = FALSE;

	while (_PlcThreadRunning)
	{
		if ((ret = rex_connect(_NetItem.ipAddr, _OnPlcConnected, _OnPlcDisconnected, _OnPlcError))==REPLY_OK)
		{
			if (FALSE)
			{
				//--- accounts -----------------------------
				ret = app_transfer_file(PATH_BIN_REXROTH NAME_ACCOUNTS, "ProjectData");
				if (ret==0) 
				{
					ret = app_reload_user_rights();	// this command need already the permission rights!
					if (ret) ret = sys_reboot();				// this command needs already the permission rights!
					if (ret) Error(ERR_CONT, 0, "Access rights to rexroth have changed. PLC has to be restarted manually.");
				}
				if (ret==1) Error(ERR_CONT, 0, "File >>%s<< missing", PATH_BIN_REXROTH NAME_ACCOUNTS);
				if (ret==3) Error(ERR_CONT, 0, "Could not tansfer file >>%s<< to PLC. Check Connection", PATH_BIN_REXROTH NAME_ACCOUNTS);			
			}
			//--- application -----------------
			/*
			if (app_download(PATH_BIN_REXROTH NAME_APP)==REPLY_OK) 
			{
				Error(WARN, 0, "Updated PLC Application");
			}
			else
			*/
			{
				_PlcAppInitialized = TRUE;
				_MpliStarting=TRUE;
				config_timer = 10;
				_OnPlcConnected();
				while (_NetItem.connected)
				{
					while (sys_in_run_mode())
					{
						net_register(&_NetItem);
			//			TrPrintfL(TRUE, "PLC Config_timer=%d", config_timer);
						switch(config_timer)
						{
						case 0:	_plc_heart_beat();
								_plc_get_status();
								_plc_state_ctrl();							
								break;
						case 1:	_plc_set_config();
						default: config_timer--;
						}
						rx_sleep(500);
					}

					//--- update PLC variables ---------------
					if (UPDATE_ALL)
					{
						lc_get_value_all(msg.data, sizeof(msg.data));	
						msg.hdr.msgId  = REP_PLC_GET_VAR;
						msg.hdr.msgLen = sizeof(msg.hdr) + strlen(msg.data); 
						gui_send_msg(0, &msg);
					}

					rx_sleep(1000);
				}
			}
		}
		else rx_sleep(1000);
	}
	return NULL;
}	