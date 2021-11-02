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
#include "enc_ctrl.h"
#include "step_ctrl.h"

#include "lh702_ctrl.h"

#include "opcua.h"

//--- Defines -----------------------------------------------------------------

//#define PREFIX_PLC "DPU_DB.PLC_1.DPU_DB."
#define PREFIX_PLC "S71500ET200MP station_1.PLC_1.DPU_DB."

// DPU_to_PLC :Struct
// life monitor (increment by 1 every 3 seconds)
#define inLifeMonitor PREFIX_PLC "DPU_to_PLC.inLifeMonitor"
// Cluster life hours
#define inHoursCluster PREFIX_PLC "DPU_to_PLC.inHoursCluster"
// ink level for each color tank
#define inInkLevels PREFIX_PLC "DPU_to_PLC.inInkLevels"
// ack send to confirm the request boReqPrepareToPrint
#define boReqPrepareToPrintAck PREFIX_PLC "DPU_to_PLC.boReqPrepareToPrintAck"
// indicate to PLC the Digital Printer is ready to print
#define boIsReadyToPrint PREFIX_PLC "DPU_to_PLC.boIsReadyToPrint"
// ack send to confirm the request boPrintOn
#define boReqPrintOnAck PREFIX_PLC "DPU_to_PLC.boReqPrintOnAck"
// indicate to PLC the Digital Printer is siemens_printing
#define boIsPrinting PREFIX_PLC "DPU_to_PLC.boIsPrinting"
// request to PLC to start the machine
#define boReqStart PREFIX_PLC "DPU_to_PLC.boReqStart"
// request to PLC to stop the machine
#define boReqStop PREFIX_PLC "DPU_to_PLC.boReqStop"
// request to PLC to siemens_pause siemens_printing (pullback)
#define boReqPause PREFIX_PLC "DPU_to_PLC.boReqPause"
// Length of a page
#define inFormatLength PREFIX_PLC "DPU_to_PLC.inFormatLength"
// cmd for the UV Lamp
#define boReqPowerOnUvLamps PREFIX_PLC "DPU_to_PLC.boReqPowerOnUvLamps"
// set point for speed (m/min)
#define reMachineSpeedSetpoint PREFIX_PLC "DPU_to_PLC.reMachineSpeedSetpoint"


// PLC_to_DPU : Struct
// offset to correct the lateral register
#define inLateralRegister PREFIX_PLC "PLC_to_DPU.inLateralRegister"
// request to DPU to prepare cluster to print
#define boReqPrepareToPrint PREFIX_PLC "PLC_to_DPU.boReqPrepareToPrint"
// request to DPU to print
#define boReqPrintOn PREFIX_PLC "PLC_to_DPU.boReqPrintOn"
// ack send to confirm the request boReqStart
#define boReqStartAck PREFIX_PLC "PLC_to_DPU.boReqStartAck"
// indicate the machine is started
#define boMachineIsRunning PREFIX_PLC "PLC_to_DPU.boMachineIsRunning"
// ack send to confirm the request boReqStop
#define boReqStopAck PREFIX_PLC "PLC_to_DPU.boReqStopAck"
// indicate the machine is siemens_stopped
#define boMachineIsStopped PREFIX_PLC "PLC_to_DPU.boMachineIsStopped"
// indicate the print is paused
#define boReqPauseAck PREFIX_PLC "PLC_to_DPU.boReqPauseAck"
// indicate the sequence of pullback is running
#define boSequencePauseIsRunning PREFIX_PLC "PLC_to_DPU.boSequencePauseIsRunning"
// status of the UV Lamp
#define inUvLampStatus PREFIX_PLC "PLC_to_DPU.inUvLampStatus"
// time of the UV Lamp
#define inUvLampCoolingCountdown PREFIX_PLC "PLC_to_DPU.inUvLampCoolingCountdown"
// Actual speed (m/min)
#define reMachineActualSpeed PREFIX_PLC "PLC_to_DPU.reMachineActualSpeed"


typedef struct
{
	INT64 counter[3];
	UINT64 macAddr;
	time_t time;
} _sctr;
//--- Externals ---------------------------------------------------------------
extern SInkSupplyStat FluidStatus[INK_SUPPLY_CNT];

//--- Statics -----------------------------------------------------------------
static int _lh702ThreadRunning=FALSE;
static SPrintQueueItem	*_pItem;
static int		_Manipulated=FALSE;

static INT16 _inLifeMonitor = 0;


//--- Prototypes --------------------------------------------------------------
static void *_lh702_thread(void *lpParameter);

static void _handle_dist			(int value);
static void _handle_lateral			(int value);
static void _handle_thickness		(int value);
static void _handle_headheight		(int value);
static void _handle_encoffset		(int value);
static void _plc_set_var			(const char *format, ...);
static void _ctr_calc_check(time_t time, UCHAR *check);


static int _error;

typedef enum
{
	uv_off,			 // 00
	uv_warmup,		 // 01
	uv_ready,		 // 02
	uv_shuttingdownn // 03
} EUvLampStatus;

int opcua_get_plc_value(char *name, char* answer)
{
	int len = 0;
	short uvstatus;
	static short _timer = 0;
	if (strstr(name, "STA_UV_POWER_ON"))
	{
		if (opcua_get_int(inUvLampStatus, &uvstatus) == 0)
		{
			if (uvstatus != uv_off)
				len = sprintf(answer, "=TRUE");
			else
				len = sprintf(answer, "=FALSE");
		}
	}
	else if (strstr(name, "STA_UV_LAMPS_READY"))
	{
		if (opcua_get_int(inUvLampStatus, &uvstatus) == 0)
		{
			if (uvstatus == uv_ready)
				len = sprintf(answer, "=TRUE");
			else
				len = sprintf(answer, "=FALSE");
		}
	}
	else if (strstr(name, "STA_UV_LAMP_1_TIMER"))
	{

		if (opcua_get_int(inUvLampStatus, &uvstatus) == 0)
		{
			int uvtimer;
			switch (uvstatus)
			{
			case uv_off:
			case uv_ready:
				uvtimer = 0;
				_timer = 0;
				break;
			case uv_warmup:
				// simulate a timer for warmup not managed py Siemend PLC
				if (_timer == 0) _timer = 30; // 500ms timer in the UI
				uvtimer = _timer / 2;
				_timer = max(_timer - 1, 2);
				break;
			case uv_shuttingdownn:
				opcua_get_int(inUvLampCoolingCountdown, &uvtimer);
				uvtimer = -uvtimer;
				_timer = 0;
				break;
			}
			len = sprintf(answer, "=%hd", uvtimer);
		}
	}
	
	return len;
}

int opcua_get_speed()
{
	float speed;
	opcua_get_float(reMachineActualSpeed, &speed);

    return (int)(speed + 0.5);
}

void opcua_set_plc_value(char *name, char *val)
{
	if (strstr(name, "CMD_UV_LAMPS_ON") && !strcmp(val, "1"))
	{
		if (opcua_set_bool(boReqPowerOnUvLamps, TRUE) != 0) Error(ERR_ABORT, 0, "Could not contact OPCUA");
	}
	else if (strstr(name, "CMD_UV_LAMPS_OFF") && !strcmp(val, "1"))
	{
		if (opcua_set_bool(boReqPowerOnUvLamps, FALSE) != 0) Error(ERR_ABORT, 0, "Could not contact OPCUA");
	}
}

//--- lh702_init -------------------------------------------------
void lh702_init(void)
{
	if (RX_Config.printer.type != printer_LH702) return; // No init/thread if it's not an LH702
	
	if(!_lh702ThreadRunning)
	{
		_lh702ThreadRunning = TRUE;
		rx_thread_start(_lh702_thread, NULL, 0, "_lh702_thread");
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
	_error = FALSE;

}


//--- lh702_set_printpar -----------------------------------------------------
int lh702_set_printpar(SPrintQueueItem *pitem)
{
	_pItem = pitem;
	// force the set point by changing the value
	opcua_set_float(reMachineSpeedSetpoint, (float)0);
	opcua_set_float(reMachineSpeedSetpoint, (float)pitem->speed);
	return REPLY_OK;
}


typedef enum
{
	siemens_stopped,
	siemens_waitforpause,
	siemens_waitforstop,
	siemens_pause,
	siemens_waittension,
	siemens_waitready,
	siemens_readytoprint,
	siemens_printing,
	siemens_requirestop,
	siemens_requirepause,
} ESiemensState;

static ESiemensState _state = siemens_stopped;

//--- _lh702_thread ------------------------------------------------------------
static void *_lh702_thread(void *lpParameter)
{
	unsigned char status;
	short last_lateral = 0;
	BOOL first = TRUE;
	BOOL restart = FALSE;
	int count = 0;

	while (_lh702ThreadRunning)
	{
		int retval = 0;
		if (opcua_connect(RX_Config.master_ip_address, RX_Config.master_ip_port, PATH_BIN "server_cert.der", PATH_BIN "server_key.der"))
		{
			if (_error == FALSE) Error(ERR_CONT, 0, "Failed to connect OPCUA server");
			_error = TRUE;
		}
		else
		{
			if (_error || _inLifeMonitor == 0)
			{
				Error(LOG, 0, "Connected to OPCUA server");
				_inLifeMonitor = 1;
			}
			_error = FALSE;
			if (++count % 6 == 0) // every 3 seconds
			{
				// set live monitor
				_inLifeMonitor++;
				retval = opcua_set_int(inLifeMonitor, _inLifeMonitor);
				if (!RX_StepperStatus.info.z_in_print) opcua_set_bool(boIsReadyToPrint, 0);

				// transfer status for ink and clusters
				static int order[7] = {4, 5, 6, 0, 1, 2, 3};
				short cluster_hours[21];
				short ink_level[7] = {0};
				for (int i = 0; i < 7; i++)
				{
					int nbcluster = RX_Config.headsPerColor / HEAD_CNT;
					for (int j = 0; j < nbcluster; j++)
					{
						int index = i * nbcluster + j;
						int colorIndex = order[i] * nbcluster + j;
						if (RX_HBStatus[colorIndex].info.connected)
						{
							cluster_hours[index] = RX_HBStatus[colorIndex].head[0].printingSeconds / 3600;
						}
					}
					ink_level[i] = FluidStatus[i].canisterLevel;
				}
				opcua_set_int_array(inHoursCluster, cluster_hours, 21);
				opcua_set_int_array(inInkLevels, ink_level, 7);
			}

			switch (_state)
			{
			case siemens_stopped:
				restart = FALSE;
				if (RX_PrinterStatus.printState == ps_printing)
				{
					TrPrintfL(TRUE, "OPCUA: Start siemens_printing");
					if ((retval = opcua_set_bool(boReqStart, 1)) == 0) _state = siemens_waittension;
					opcua_set_bool(boReqStop, 0);
					opcua_set_bool(boReqPause, 0);
					first = TRUE;
				}
				break;
			case siemens_waittension:
				if ((retval = opcua_get_bool(boReqPrepareToPrint, &status)) == 0)
				{
					if (status == 1)
					{
						_state = siemens_waitready;
						retval = opcua_set_bool(boReqStart, 0);
						step_handle_gui_msg(INVALID_SOCKET, CMD_LIFT_PRINT_POS, NULL, 0);
						TrPrintfL(TRUE, "OPCUA: Tension ready");
					}
				}
				break;
			case siemens_waitready:
				TrPrintfL(TRUE, "OPCUA: wait printer ready enc_ready %d printpos %d data ready %d", enc_ready() , RX_StepperStatus.info.z_in_print, RX_PrinterStatus.dataReady);
				if (RX_PrinterStatus.dataReady && ((enc_ready() && RX_StepperStatus.info.z_in_print) || arg_simuEncoder))
				{
					if ((retval = opcua_set_bool(boIsReadyToPrint, 1)) == 0)
					{
						_state = siemens_readytoprint;
						TrPrintfL(TRUE, "OPCUA: Printer ready");
					}
				}
				break;
			case siemens_readytoprint:
				if ((retval = opcua_get_bool(boReqPrintOn, &status)) == 0)
				{
					if (status)
					{
						_state = siemens_printing;
						if (!restart) enc_start_printing(_pItem, FALSE);
						else enc_restart_pg();

						restart = FALSE;

						int pageHeight = _pItem->pageHeight / 1000;
						int printGoDist = _pItem->printGoDist / 1000;
						if (_pItem->printGoMode == PG_MODE_LENGTH) pageHeight = printGoDist;
						if (_pItem->printGoMode == PG_MODE_GAP) pageHeight = pageHeight + printGoDist;

						retval = opcua_set_int(inFormatLength, pageHeight);
						retval |= opcua_set_bool(boIsPrinting, 1) || opcua_set_bool(boReqStart, 0);
						TrPrintfL(TRUE, "OPCUA: Printing");
					}
				}
				break;
			case siemens_printing:
			{
				short lateral = 0;
				retval = opcua_get_int(inLateralRegister, &lateral);
				if (first)
				{
					first = FALSE;
					last_lateral = lateral;
				}
				else if (lateral != last_lateral)
				{
					_handle_lateral(lateral - last_lateral);
					last_lateral = lateral;
				}
			}
				break;
			case siemens_waitforpause:
				if ((retval = opcua_get_bool(boMachineIsStopped, &status)) == 0)
				{
					if (status)
					{
						_state = siemens_pause;
						RX_PrinterStatus.printState = ps_pause;
						retval = opcua_set_bool(boReqPause, 0);
						TrPrintfL(TRUE, "OPCUA: Pause");
					}
				}
				if (RX_PrinterStatus.printState == ps_pause)
				{
					retval = opcua_set_bool(boIsPrinting, 0);
				}
				break;
			case siemens_waitforstop:
				if ((retval = opcua_get_bool(boMachineIsStopped, &status)) == 0)
				{
					if (status)
					{
						_state = siemens_stopped;
						RX_PrinterStatus.printState = ps_ready_power;
						retval = opcua_set_bool(boIsPrinting, 0);
						retval |= opcua_set_bool(boIsReadyToPrint, 0);
						retval |= opcua_set_bool(boReqStop, 0);
						if (RX_StepperStatus.info.z_in_print) step_handle_gui_msg(INVALID_SOCKET, CMD_LIFT_UP_POS, NULL, 0);
						TrPrintfL(TRUE, "OPCUA: Stop");
					}
				}
				break;
			case siemens_pause:
				if (RX_PrinterStatus.printState == ps_printing)
				{
					TrPrintfL(TRUE, "OPCUA: Restart printing");
					restart = TRUE;
					if ((retval = opcua_set_bool(boReqStart, 1)) == 0) _state = siemens_readytoprint;
				}
				break;

			// TODO recompile open62541 multithread and move these cases in dedicated fonctions
			case siemens_requirestop:
				opcua_set_bool(boReqStart, 0);
				if ((retval = opcua_set_bool(boReqStop, 1)) == 0)
				{
					retval |= opcua_set_bool(boIsPrinting, 0);
					retval |= opcua_set_bool(boIsReadyToPrint, 0);
					_state = siemens_waitforstop;
					RX_PrinterStatus.printState = ps_stopping;
					TrPrintfL(TRUE, "OPCUA: Require Stop");
				}

				break;
			case siemens_requirepause:
				retval = opcua_set_bool(boReqPause, 1); //|| opcua_set_bool(boIsPrinting, 0);				
				_state = siemens_waitforpause;
				TrPrintfL(TRUE, "OPCUA: Require Pause");
				opcua_set_bool(boReqStart, 0);
				break;
			}
			if (retval != 0)
			{
				Error(ERR_ABORT, 0, "Could not contact OPCUA");
				_state = siemens_stopped;
			}

		}
		rx_sleep(500);
	}
	Error(ERR_ABORT, 0, "Stopping OPCUA communication...");
	return NULL;
}

int lh702_pause_printing()
{
	// TODO recompile open62541 multithread and move siemens_requirepause
	Error(WARN, 0, "PAUSED by Operator");
	if (_state == siemens_printing)
		_state = siemens_requirepause;
	else
		lh702_stop_printing();
	return REPLY_OK;
}

int lh702_stop_printing()
{
	enc_stop_printing();
	// TODO recompile open62541 multithread and move siemens_requirestop
	_state = siemens_requirestop;
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

		setup_save(file, PATH_USER FILENAME_COUNTERS_LH702);
		setup_destroy(file);
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
