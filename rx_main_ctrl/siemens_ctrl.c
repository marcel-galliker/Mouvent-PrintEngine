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
//--- Externals ---------------------------------------------------------------
extern SInkSupplyStat FluidStatus[INK_SUPPLY_CNT];
extern int _lh702ThreadRunning;

//--- Statics -----------------------------------------------------------------
static SPrintQueueItem	*_pItem;
static int		_Manipulated=FALSE;

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


// Get the OPCUA name of the variable including the full prefix
#define SET 0
#define GET 1
char plcstruct[2][11] = {"DPU_to_PLC", "PLC_to_DPU"}; // prefix for GET and SET
static char *get_name(int dpu2plc, const char* var, char* buffer)
{
	sprintf(buffer, "%s.DPU_DB.%s.%s", RX_Config.opcua_prefix, plcstruct[dpu2plc], var);
	return buffer;
}


int opcua_get_plc_value(char *name, char* answer)
{
	int len = 0;
	short uvstatus;
	static short _timer = 0;
	char buffer[256]; // for opcua variable name
	if (strstr(name, "STA_UV_POWER_ON"))
	{
		if (opcua_get_int(get_name(GET, "inUvLampStatus", buffer), &uvstatus) == 0)
		{
			if (uvstatus != uv_off)
				len = sprintf(answer, "=TRUE");
			else
				len = sprintf(answer, "=FALSE");
		}
	}
	else if (strstr(name, "STA_UV_LAMPS_READY"))
	{
		if (opcua_get_int(get_name(GET, "inUvLampStatus", buffer), &uvstatus) == 0)
		{
			if (uvstatus == uv_ready)
				len = sprintf(answer, "=TRUE");
			else
				len = sprintf(answer, "=FALSE");
		}
	}
	else if (strstr(name, "STA_UV_LAMP_1_TIMER"))
	{

		if (opcua_get_int(get_name(GET, "inUvLampStatus", buffer), &uvstatus) == 0)
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
				opcua_get_int(get_name(GET, "inUvLampCoolingCountdown", buffer), &uvtimer);
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
	char buffer[256];
	if (opcua_get_float(get_name(GET, "reMachineActualSpeed", buffer), &speed)) return 0;
	return (int)(speed + 0.5);
}

void opcua_set_plc_value(char *name, char *val)
{
	char buffer[256];
	if (strstr(name, "CMD_UV_LAMPS_ON") && !strcmp(val, "1"))
	{
		if (opcua_set_bool(get_name(SET, "boReqPowerOnUvLamps", buffer), TRUE) != 0) Error(ERR_ABORT, 0, "Could not contact OPCUA");
	}
	else if (strstr(name, "CMD_UV_LAMPS_OFF") && !strcmp(val, "1"))
	{
		if (opcua_set_bool(get_name(SET, "boReqPowerOnUvLamps", buffer), FALSE) != 0) Error(ERR_ABORT, 0, "Could not contact OPCUA");
	}
	else if (strstr(name, "PAR_WEB_WIDTH"))
	{
		if (opcua_set_int(get_name(SET, "inWebWidth", buffer), atoi(val)) != 0) Error(ERR_ABORT, 0, "Could not contact OPCUA");
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
	char buffer[256];
	_pItem = pitem;
	// force the set point by changing the value
	opcua_set_float(get_name(SET, "reMachineSpeedSetpoint", buffer), (float)0);
	opcua_set_float(get_name(SET, "reMachineSpeedSetpoint", buffer), (float)pitem->speed);
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
	char buffer[256];

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
				retval = opcua_set_int(get_name(SET, "inLifeMonitor", buffer), _inLifeMonitor);
				if (!RX_StepperStatus.info.z_in_print) opcua_set_bool(get_name(SET, "boIsReadyToPrint", buffer), 0);

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
						cluster_hours[index] = 0;
						if (RX_HBStatus[colorIndex].info.connected)
						{
							for (int h=0; h<HEAD_CNT; h++) cluster_hours[index] = max(cluster_hours[index], RX_HBStatus[colorIndex].head[h].printingSeconds / 3600);
						}
					}
					ink_level[i] = FluidStatus[i].canisterLevel;
				}
				opcua_set_int_array(get_name(SET, "inHoursCluster", buffer), cluster_hours, 21);
				opcua_set_int_array(get_name(SET, "inInkLevels", buffer), ink_level, 7);
			}

			switch (_state)
			{
			case siemens_stopped:
				restart = FALSE;
				if (RX_PrinterStatus.printState == ps_printing)
				{
					TrPrintfL(TRUE, "OPCUA: Start siemens_printing");
					if ((retval = opcua_set_bool(get_name(SET, "boReqStart", buffer), 1)) == 0) _state = siemens_waittension;
					opcua_set_bool(get_name(SET, "boReqStop", buffer), 0);
					opcua_set_bool(get_name(SET, "boReqPause", buffer), 0);
					first = TRUE;
				}
				break;
			case siemens_waittension:
				if ((retval = opcua_get_bool(get_name(GET, "boReqPrepareToPrint", buffer), &status)) == 0)
				{
					if (status == 1)
					{
						_state = siemens_waitready;
						retval = opcua_set_bool(get_name(SET, "boReqStart", buffer), 0);
						step_handle_gui_msg(INVALID_SOCKET, CMD_LIFT_PRINT_POS, NULL, 0);
						TrPrintfL(TRUE, "OPCUA: Tension ready");
					}
				}
				break;
			case siemens_waitready:
				TrPrintfL(TRUE, "OPCUA: wait printer ready enc_ready %d printpos %d data ready %d", enc_ready() , RX_StepperStatus.info.z_in_print, RX_PrinterStatus.dataReady);
				if (RX_PrinterStatus.dataReady && ((enc_ready() && RX_StepperStatus.info.z_in_print) || arg_simuEncoder))
				{
					if ((retval = opcua_set_bool(get_name(SET, "boIsReadyToPrint", buffer), 1)) == 0)
					{
						_state = siemens_readytoprint;
						TrPrintfL(TRUE, "OPCUA: Printer ready");
					}
				}
				break;
			case siemens_readytoprint:
				if ((retval = opcua_get_bool(get_name(GET, "boReqPrintOn", buffer), &status)) == 0)
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

						retval = opcua_set_int(get_name(SET, "inFormatLength", buffer), pageHeight);
						retval |= opcua_set_bool(get_name(SET, "boIsPrinting", buffer), 1) || opcua_set_bool(get_name(SET, "boReqStart", buffer), 0);
						TrPrintfL(TRUE, "OPCUA: Printing");
					}
				}
				break;
			case siemens_printing:
			{
				short lateral = 0;
				retval = opcua_get_int(get_name(GET, "inLateralRegister", buffer), &lateral);
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
				if ((retval = opcua_get_bool(get_name(GET, "boMachineIsStopped", buffer), &status)) == 0)
				{
					if (status)
					{
						_state = siemens_pause;
						RX_PrinterStatus.printState = ps_pause;
						retval = opcua_set_bool(get_name(SET, "boReqPause", buffer), 0);
						TrPrintfL(TRUE, "OPCUA: Pause");
					}
				}
				if (RX_PrinterStatus.printState == ps_pause)
				{
					retval = opcua_set_bool(get_name(SET, "boIsPrinting", buffer), 0);
				}
				break;
			case siemens_waitforstop:
				if ((retval = opcua_get_bool(get_name(GET, "boMachineIsStopped", buffer), &status)) == 0)
				{
					if (status)
					{
						_state = siemens_stopped;
						RX_PrinterStatus.printState = ps_ready_power;
						retval = opcua_set_bool(get_name(SET, "boIsPrinting", buffer), 0);
						retval |= opcua_set_bool(get_name(SET, "boIsReadyToPrint", buffer), 0);
						retval |= opcua_set_bool(get_name(SET, "boReqStop", buffer), 0);
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
					if ((retval = opcua_set_bool(get_name(SET, "boReqStart", buffer), 1)) == 0) _state = siemens_readytoprint;
				}
				break;

			// TODO recompile open62541 multithread and move these cases in dedicated fonctions
			case siemens_requirestop:
				opcua_set_bool(get_name(SET, "boReqStart", buffer), 0);
				if ((retval = opcua_set_bool(get_name(SET, "boReqStop", buffer), 1)) == 0)
				{
					retval |= opcua_set_bool(get_name(SET, "boIsPrinting", buffer), 0);
					retval |= opcua_set_bool(get_name(SET, "boIsReadyToPrint", buffer), 0);
					_state = siemens_waitforstop;
					RX_PrinterStatus.printState = ps_stopping;
					TrPrintfL(TRUE, "OPCUA: Require Stop");
				}

				break;
			case siemens_requirepause:
				retval = opcua_set_bool(get_name(SET, "boReqPause", buffer), 1); //|| opcua_set_bool(get_name(SET, "boIsPrinting", buffer), 0);				
				_state = siemens_waitforpause;
				TrPrintfL(TRUE, "OPCUA: Require Pause");
				opcua_set_bool(get_name(SET, "boReqStart", buffer), 0);
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

