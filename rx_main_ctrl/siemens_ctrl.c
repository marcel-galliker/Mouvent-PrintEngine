// ****************************************************************************
//
//	siemens_ctrl.c
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

#include "siemens_ctrl.h"

#include "opcua.h"

#define PREFIX_PLC "DPU_DB."

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
// Web width in mm
#define inWebWidth PREFIX_PLC "DPU_to_PLC.inWebWidth"

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

//--- Externals ---------------------------------------------------------------
extern SInkSupplyStat FluidStatus[INK_SUPPLY_CNT];
extern int _lh702ThreadRunning;

//--- Statics -----------------------------------------------------------------
static SPrintQueueItem	*_pItem;

static HANDLE _client = NULL;
static INT16 _inLifeMonitor = 0;


//--- Prototypes --------------------------------------------------------------
void *siemems_thread(void *lpParameter);

static void _handle_dist			(int value);
static void _handle_lateral			(int value);
static void _plc_set_var			(const char *format, ...);

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
		if (opcua_get_int(_client, inUvLampStatus, &uvstatus) == 0)
		{
			if (uvstatus != uv_off)
				len = sprintf(answer, "=TRUE");
			else
				len = sprintf(answer, "=FALSE");
		}
	}
	else if (strstr(name, "STA_UV_LAMPS_READY"))
	{
		if (opcua_get_int(_client, inUvLampStatus, &uvstatus) == 0)
		{
			if (uvstatus == uv_ready)
				len = sprintf(answer, "=TRUE");
			else
				len = sprintf(answer, "=FALSE");
		}
	}
	else if (strstr(name, "STA_UV_LAMP_1_TIMER"))
	{

		if (opcua_get_int(_client, inUvLampStatus, &uvstatus) == 0)
		{
			short uvtimer = 0;
			switch (uvstatus)
			{
			case uv_off:
			case uv_ready:
				uvtimer = 0;
				_timer = 0;
				break;
			case uv_warmup:
				// simulate a timer for warmup not managed py Siemend PLC
				if (_timer == 0) _timer = 180; // 90s for a 500ms timer in the UI
				uvtimer = _timer / 2;
				_timer = max(_timer - 1, 2);
				break;
			default: // cooldown or forced cooldown
				opcua_get_int(_client, inUvLampCoolingCountdown, &uvtimer);
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
	if (opcua_get_float(_client, reMachineActualSpeed, &speed)) return 0;
	return (int)(speed + 0.5);
}

void opcua_set_plc_value(char *name, char *val)
{
	if (strstr(name, "CMD_UV_LAMPS_ON") && !strcmp(val, "1"))
	{
		if (opcua_set_bool(_client, boReqPowerOnUvLamps, TRUE) != 0) Error(ERR_ABORT, 0, "Could not contact OPCUA");
	}
	else if (strstr(name, "CMD_UV_LAMPS_OFF") && !strcmp(val, "1"))
	{
        if (opcua_set_bool(_client, boReqPowerOnUvLamps, FALSE) != 0) Error(ERR_ABORT, 0, "Could not contact OPCUA");
	}
	else if (strstr(name, "PAR_WEB_WIDTH"))
	{
        if (opcua_set_int(_client, inWebWidth, atoi(val)) != 0) Error(ERR_ABORT, 0, "Could not contact OPCUA");
	}
}

//--- lh702_error_reset -----------------------
void siemens_error_reset(void)
{
	_error = FALSE;

}


//--- lh702_set_printpar -----------------------------------------------------
int siemens_set_printpar(SPrintQueueItem *pitem)
{
	_pItem = pitem;
	// force the set point by changing the value
    if (opcua_set_float(_client, reMachineSpeedSetpoint, (float)0)
    || opcua_set_float(_client, reMachineSpeedSetpoint, (float)pitem->speed))
        Error(ERR_ABORT, 0, "Could not contact OPCUA");

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
void *siemens_thread(void *lpParameter)
{
	unsigned char status;
	short last_lateral = 0;
	BOOL first = TRUE;
	BOOL restart = FALSE;
	int count = 0;

	while (_lh702ThreadRunning)
	{
		int retval = 0;
		if (opcua_connect(&_client, RX_Config.master_ip_address, RX_Config.master_ip_port, 6, RX_Config.opcua_prefix, PATH_BIN "server_cert.der",
				PATH_BIN "server_key.der"))
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
				retval = opcua_set_int(_client, inLifeMonitor, _inLifeMonitor);
				if (!RX_StepperStatus.info.z_in_print) opcua_set_bool(_client, boIsReadyToPrint, 0);

				// transfer status for ink and clusters
				static int order[7] = {4, 5, 6, 0, 1, 2, 3};
				short cluster_hours[21] = {0};
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
							for (int h=0; h<HEAD_CNT; h++) cluster_hours[index] = max(cluster_hours[index], RX_HBStatus[colorIndex].head[h].printingSeconds / 3600);
						}
					}
					ink_level[i] = FluidStatus[i].canisterLevel;
				}
				retval |= opcua_set_int_array(_client, inHoursCluster, cluster_hours, 21);
                retval |= opcua_set_int_array(_client, inInkLevels, ink_level, 7);
			}

			switch (_state)
			{
			case siemens_stopped:
				restart = FALSE;
				if (RX_PrinterStatus.printState == ps_printing)
				{
					TrPrintfL(TRUE, "OPCUA: Start siemens_printing");
					if ((retval = opcua_set_bool(_client, boReqStart, 1)) == 0) _state = siemens_waittension;
					opcua_set_bool(_client, boReqStop, 0);
					opcua_set_bool(_client, boReqPause, 0);
					first = TRUE;
				}
				break;
			case siemens_waittension:
				if ((retval = opcua_get_bool(_client, boReqPrepareToPrint, &status)) == 0)
				{
					if (status == 1)
					{
						_state = siemens_waitready;
						retval = opcua_set_bool(_client, boReqStart, 0);
						step_handle_gui_msg(INVALID_SOCKET, CMD_LIFT_PRINT_POS, NULL, 0);
						TrPrintfL(TRUE, "OPCUA: Tension ready");
					}
				}
				break;
			case siemens_waitready:
				TrPrintfL(TRUE, "OPCUA: wait printer ready enc_ready %d printpos %d data ready %d", enc_ready() , RX_StepperStatus.info.z_in_print, RX_PrinterStatus.dataReady);
				if (RX_PrinterStatus.dataReady && ((enc_ready() && RX_StepperStatus.info.z_in_print) || arg_simuEncoder))
				{
					if ((retval = opcua_set_bool(_client, boIsReadyToPrint, 1)) == 0)
					{
						_state = siemens_readytoprint;
						TrPrintfL(TRUE, "OPCUA: Printer ready");
					}
				}
				break;
			case siemens_readytoprint:
				if ((retval = opcua_get_bool(_client, boReqPrintOn, &status)) == 0)
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

						retval = opcua_set_int(_client, inFormatLength, pageHeight);
						retval |= opcua_set_bool(_client, boIsPrinting, 1) || opcua_set_bool(_client, boReqStart, 0);
						TrPrintfL(TRUE, "OPCUA: Printing");
					}
				}
				break;
			case siemens_printing:
			{
				short lateral = 0;
				retval = opcua_get_int(_client, inLateralRegister, &lateral);
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
				if ((retval = opcua_get_bool(_client, boMachineIsStopped, &status)) == 0)
				{
					if (status)
					{
						_state = siemens_pause;
						RX_PrinterStatus.printState = ps_pause;
						retval = opcua_set_bool(_client, boReqPause, 0);
						TrPrintfL(TRUE, "OPCUA: Pause");
					}
				}
				if (RX_PrinterStatus.printState == ps_pause)
				{
					retval = opcua_set_bool(_client, boIsPrinting, 0);
				}
				break;
			case siemens_waitforstop:
				if ((retval = opcua_get_bool(_client, boMachineIsStopped, &status)) == 0)
				{
					if (status)
					{
						_state = siemens_stopped;
						RX_PrinterStatus.printState = ps_ready_power;
						retval = opcua_set_bool(_client, boIsPrinting, 0);
						retval |= opcua_set_bool(_client, boIsReadyToPrint, 0);
						retval |= opcua_set_bool(_client, boReqStop, 0);
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
					if ((retval = opcua_set_bool(_client, boReqStart, 1)) == 0) _state = siemens_readytoprint;
				}
				break;

			// TODO recompile open62541 multithread and move these cases in dedicated fonctions
			case siemens_requirestop:
				opcua_set_bool(_client, boReqStart, 0);
				if ((retval = opcua_set_bool(_client, boReqStop, 1)) == 0)
				{
					retval |= opcua_set_bool(_client, boIsPrinting, 0);
					retval |= opcua_set_bool(_client, boIsReadyToPrint, 0);
					_state = siemens_waitforstop;
					RX_PrinterStatus.printState = ps_stopping;
					TrPrintfL(TRUE, "OPCUA: Require Stop");
				}

				break;
			case siemens_requirepause:
				retval = opcua_set_bool(_client, boReqPause, 1); //|| opcua_set_bool(_client, boIsPrinting, 0);				
				_state = siemens_waitforpause;
				TrPrintfL(TRUE, "OPCUA: Require Pause");
				opcua_set_bool(_client, boReqStart, 0);
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
	opcua_close(&_client);
	Error(ERR_ABORT, 0, "Stopping OPCUA communication...");
	return NULL;
}

int siemens_pause_printing()
{
	Error(WARN, 0, "PAUSED by Operator");
	if (_state == siemens_printing)
		_state = siemens_requirepause;
	else
		siemens_stop_printing();
	return REPLY_OK;
}

int siemens_stop_printing()
{
	enc_stop_printing();
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

