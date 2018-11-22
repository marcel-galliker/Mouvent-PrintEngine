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

//--- includes ----------------------------------------------------------------
#include "rx_error.h"
#include "rx_threads.h"
#include "gui_svr.h"
#include "rx_minilabel_def.h"
#include "rx_trace.h"
#include "tcp_ip.h"
#include "setup.h"
#include "network.h"
#include "rx_rexroth.h"
#include "plc_ctrl.h"

//--- SIMULATION ----------------------------------------------
static int _Simulation = FALSE;

//--- defines -----------------------------------------
#define	DIAMETER_MIN  70
#define	DIAMETER_MAX 400

#define NAME_APP		"Application.app"
#define NAME_ACCOUNTS	"accounts.xml"

#define PAR_PREFIX	"Application.USERPARVARGLOBAL."
#define CMD_PREFIX	"Application.COMMANDVARGLOBAL."
#define STA_PREFIX	"Application.STATUSVARGLOBAL."


//--- prototypes -----------------------
static void* _plc_thread(void *par);
static void* _plc_simu_thread(void *par);
static void _simu_init(void);
static void _plc_set_config();
static void _plc_get_status();

static void _do_plc_get_info(RX_SOCKET socket);
static void _do_plc_get_log (RX_SOCKET socket);

//--- statics -----------------------------------------------------------------
static int			_PlcThreadRunning;
static SML_Config	_Config;
static SML_Status	_Status;
static int			_RunTime=0;
static SNetworkItem _NetItem;


//--- plc_init ----------------------------------------------------------------
int	plc_init(void)
{
	_PlcThreadRunning = TRUE;
		
	setup_minilabel(PATH_USER FILENAME_PLC, &_Config, READ);

	if (_Simulation) rx_thread_start(_plc_simu_thread, NULL, 0, "PLC Thread");
	else			 rx_thread_start(_plc_thread, NULL, 0, "PLC Thread");

	_simu_init();

	return REPLY_OK;
}

//--- plc_end -----------------------------------------------------------------
int plc_end(void)
{
	_PlcThreadRunning = FALSE;
	return REPLY_OK;
}

//--- plc_set_cfg -----------------------------------------------------
void plc_set_cfg(SML_Config *pcfg)
{
	memcpy(&_Config, pcfg, sizeof(_Config));
	setup_minilabel(PATH_USER FILENAME_PLC, &_Config, WRITE);
	_plc_set_config();
}

//--- plc_get_cfg -----------------------------------------------------
SML_Config *plc_get_cfg()
{
	return &_Config;
}

//--- plc_errorReset -----------------------------------------
void plc_errorReset(void)
{
	lc_set_value_by_name_UINT32(CMD_PREFIX "CMD_RESET_ERROR",    1);
	sys_reset_error();
}

//--- commands ---------------
int do_plc_cmd(RX_SOCKET socket, UINT32 cmd)
{
	if (_Simulation)
	{
		switch(cmd)
		{
			case CMD_PLC_RSEET_ERROR:	_simu_init();										break;
			case CMD_PLC_TO_MANUAL:		_Status.state = STATE_AUTO_MANUAL;					break;
			case CMD_PLC_TO_PRODUCTION: _Status.state = STATE_AUTO_CURRENT_OFF;				break;
			case CMD_PLC_STOP:			_Status.state = STATE_AUTO_TENSION_OK; 
										_Status.speed = 0;									break;
			case CMD_PLC_START:			_RunTime=0; 
										_Status.state = STATE_AUTO_PRINTING;
										_Status.speed = _Config.speed;						break;
			case CMD_PLC_JOG_FWD:		_Status.state = STATE_AUTO_JOG_FWD;
										_Status.speed = _Config.speed/2;					break;
			case CMD_PLC_JOG_BWD:		_Status.state = STATE_AUTO_JOG_BWD;
										_Status.speed = -1*(_Config.speed/2);				break;
			case CMD_PLC_JOG_STOP:		if (_Status.state!=STATE_AUTO_MANUAL)
											_Status.state = STATE_AUTO_TENSION_OK;
										_Status.speed = 0;									break;

			case CMD_PLC_UNWINDER_CW:	_Status.state = STATE_AUTO_MANUAL; break;
			case CMD_PLC_UNWINDER_CCW:	_Status.state = STATE_AUTO_MANUAL; break;
			case CMD_PLC_REWINDER_CW:	_Status.state = STATE_AUTO_MANUAL; break;
			case CMD_PLC_REWINDER_CCW:	_Status.state = STATE_AUTO_MANUAL; break;
			case CMD_PLC_INFEED_CW:		_Status.state = STATE_AUTO_MANUAL; break;
			case CMD_PLC_INFEED_CCW:	_Status.state = STATE_AUTO_MANUAL; break;
			case CMD_PLC_OUTFEED_CW:	_Status.state = STATE_AUTO_MANUAL; break;
			case CMD_PLC_OUTFEED_CCW:	_Status.state = STATE_AUTO_MANUAL; break;
	
			case CMD_PLC_TENSION_ON:	_Status.state = STATE_AUTO_TENSION_OK;				break;
			case CMD_PLC_TENSION_OFF:	_Status.state = STATE_AUTO_CURRENT_OFF;				break;

			case CMD_PLC_INFEED_OPEN:	_Status.infeedClosed	= FALSE;					break;
			case CMD_PLC_INFEED_CLOSE:	_Status.infeedClosed	= TRUE;						break;
			case CMD_PLC_OUTFEED_OPEN:	_Status.outfeedClosed	= FALSE;					break;
			case CMD_PLC_OUTFEED_CLOSE:	_Status.outfeedClosed	= TRUE;						break;
			case CMD_PLC_SPLICE_1_OPEN:	_Status.splice1Closed	= FALSE;					break;
			case CMD_PLC_SPLICE_1_CLOSE:_Status.splice1Closed	= TRUE;						break;
			case CMD_PLC_SPLICE_2_OPEN:	_Status.splice2Closed	= FALSE;					break;
			case CMD_PLC_SPLICE_2_CLOSE:_Status.splice2Closed	= TRUE;						break;

			case CMD_PLC_TENSION_RST:	_Status.tensionMin          = _Status.tensionMax          = _Status.tension;			break;
			case CMD_PLC_SPEED_RST:		_Status.speedMin            = _Status.speedMax			  = _Status.speed;				break;
			case CMD_PLC_UW_TENSION_RST:_Status.unwinderTensionMin  = _Status.unwinderTensionMax  = _Status.unwinderTension;	break;
			case CMD_PLC_UW_DIAM_RST:	_Status.unwinderDiameterMin = _Status.unwinderDiameterMax = _Status.unwinderDiameter;	break;
			case CMD_PLC_RW_TENSION_RST:_Status.rewinderTensionMin  = _Status.rewinderTensionMax  = _Status.rewinderTension;	break;
			case CMD_PLC_RW_DIAM_RST:	_Status.rewinderDiameterMin = _Status.rewinderDiameterMax = _Status.rewinderDiameter;	break;

			case CMD_PLC_GET_INFO:		_do_plc_get_info(socket)	;											break;
			case CMD_PLC_RESET_ERROR:	/* sys_reset_error(); */												break;
			case CMD_PLC_REBOOT:		/* sys_reboot(); */													break;				
			case CMD_PLC_GET_LOG:		_do_plc_get_log(socket);												break;
		}
		
		gui_send_msg(NO_SOCKET, &_Status);
	}
	else
	{
		switch(cmd)
		{
			case CMD_PLC_RSEET_ERROR:	plc_errorReset();	break;
			case CMD_PLC_TO_MANUAL:		lc_set_value_by_name_UINT32(CMD_PREFIX "CMD_MANUAL_START",  1);	break;
			case CMD_PLC_TO_PRODUCTION:	lc_set_value_by_name_UINT32(CMD_PREFIX "CMD_MANUAL_STOP",   1);	break;

			case CMD_PLC_STOP:			lc_set_value_by_name_UINT32(CMD_PREFIX "CMD_PRINTING_STOP",  1);	break;
			case CMD_PLC_START:			lc_set_value_by_name_UINT32(CMD_PREFIX "CMD_RESET_ERROR",    1);
										lc_set_value_by_name_UINT32(CMD_PREFIX "CMD_PRINTING_START", 1);	
										break;
			case CMD_PLC_JOG_FWD:		lc_set_value_by_name_UINT32(CMD_PREFIX "CMD_JOG_FORWARD",				1);	break;
			case CMD_PLC_JOG_BWD:		lc_set_value_by_name_UINT32(CMD_PREFIX "CMD_JOG_BACKWARD",				1);	break;
			case CMD_PLC_JOG_STOP:		lc_set_value_by_name_UINT32(CMD_PREFIX "CMD_JOG_FORWARD",				0);
										lc_set_value_by_name_UINT32(CMD_PREFIX "CMD_JOG_BACKWARD",				0);
										lc_set_value_by_name_UINT32(CMD_PREFIX "CMD_UNWINDER_MANUAL_FORWARD",  0);
										lc_set_value_by_name_UINT32(CMD_PREFIX "CMD_UNWINDER_MANUAL_BACKWARD", 0);
										lc_set_value_by_name_UINT32(CMD_PREFIX "CMD_REWINDER_MANUAL_FORWARD",  0);
										lc_set_value_by_name_UINT32(CMD_PREFIX "CMD_REWINDER_MANUAL_BACKWARD", 0);
										lc_set_value_by_name_UINT32(CMD_PREFIX "CMD_INFEED_MANUAL_FORWARD",    0);
										lc_set_value_by_name_UINT32(CMD_PREFIX "CMD_INFEED_MANUAL_BACKWARD",   0);
										lc_set_value_by_name_UINT32(CMD_PREFIX "CMD_OUTFEED_MANUAL_FORWARD",   0);
										lc_set_value_by_name_UINT32(CMD_PREFIX "CMD_OUTFEED_MANUAL_BACKWARD",  0);
										break;
			case CMD_PLC_UNWINDER_CW:	lc_set_value_by_name_UINT32(CMD_PREFIX "CMD_UNWINDER_MANUAL_FORWARD",  1);	break;
			case CMD_PLC_UNWINDER_CCW:	lc_set_value_by_name_UINT32(CMD_PREFIX "CMD_UNWINDER_MANUAL_BACKWARD", 1);	break;
			case CMD_PLC_REWINDER_CW:	lc_set_value_by_name_UINT32(CMD_PREFIX "CMD_REWINDER_MANUAL_FORWARD",  1);	break;
			case CMD_PLC_REWINDER_CCW:	lc_set_value_by_name_UINT32(CMD_PREFIX "CMD_REWINDER_MANUAL_BACKWARD", 1);	break;
			case CMD_PLC_INFEED_CW:		lc_set_value_by_name_UINT32(CMD_PREFIX "CMD_INFEED_MANUAL_FORWARD",    1);	break;
			case CMD_PLC_INFEED_CCW:	lc_set_value_by_name_UINT32(CMD_PREFIX "CMD_INFEED_MANUAL_BACKWARD",   1);	break;
			case CMD_PLC_OUTFEED_CW:	lc_set_value_by_name_UINT32(CMD_PREFIX "CMD_OUTFEED_MANUAL_FORWARD",   1);	break;
			case CMD_PLC_OUTFEED_CCW:	lc_set_value_by_name_UINT32(CMD_PREFIX "CMD_OUTFEED_MANUAL_BACKWARD",  1);	break;

			case CMD_PLC_TENSION_ON:	lc_set_value_by_name_UINT32(CMD_PREFIX "CMD_TENSION_ENABLE", 1);	break;
			case CMD_PLC_TENSION_OFF:	lc_set_value_by_name_UINT32(CMD_PREFIX "CMD_TENSION_DISABLE",1);	break;
			case CMD_PLC_INFEED_OPEN:	lc_set_value_by_name_UINT32(CMD_PREFIX "CMD_TENSION_DISABLE",1);
										lc_set_value_by_name_UINT32(CMD_PREFIX "CMD_INFEED_OPEN",    1);	break;
			case CMD_PLC_INFEED_CLOSE:	lc_set_value_by_name_UINT32(CMD_PREFIX "CMD_INFEED_CLOSE",   1);	break;
			case CMD_PLC_OUTFEED_OPEN:	lc_set_value_by_name_UINT32(CMD_PREFIX "CMD_TENSION_DISABLE",1);
										lc_set_value_by_name_UINT32(CMD_PREFIX "CMD_OUTFEED_OPEN",   1);	
										break;
			case CMD_PLC_OUTFEED_CLOSE:	lc_set_value_by_name_UINT32(CMD_PREFIX "CMD_OUTFEED_CLOSE",  1);	break;
			case CMD_PLC_SPLICE_1_OPEN:	lc_set_value_by_name_UINT32(CMD_PREFIX "CMD_SPLICE_1_OPEN",  1);	break;
			case CMD_PLC_SPLICE_1_CLOSE:lc_set_value_by_name_UINT32(CMD_PREFIX "CMD_SPLICE_1_CLOSE", 1);	break;
			case CMD_PLC_SPLICE_2_OPEN:	lc_set_value_by_name_UINT32(CMD_PREFIX "CMD_SPLICE_2_OPEN",  1);	break;
			case CMD_PLC_SPLICE_2_CLOSE:lc_set_value_by_name_UINT32(CMD_PREFIX "CMD_SPLICE_2_CLOSE", 1);	break;

			case CMD_PLC_TENSION_RST:	lc_set_value_by_name_UINT32(CMD_PREFIX "CMD_RESET_TENSION_MINMAX",		1);	break;
			case CMD_PLC_SPEED_RST:		lc_set_value_by_name_UINT32(CMD_PREFIX "CMD_RESET_SPEED_MINMAX",		1);	break;
			case CMD_PLC_UW_TENSION_RST:lc_set_value_by_name_UINT32(CMD_PREFIX "CMD_RESET_UNW_TENSION_MINMAX", 1);	break;
			case CMD_PLC_UW_DIAM_RST:	lc_set_value_by_name_UINT32(CMD_PREFIX "CMD_RESET_UNW_DIAM_MINMAX",	1);	break;
			case CMD_PLC_RW_TENSION_RST:lc_set_value_by_name_UINT32(CMD_PREFIX "CMD_RESET_REW_TENSION_MINMAX", 1);	break;
			case CMD_PLC_RW_DIAM_RST:	lc_set_value_by_name_UINT32(CMD_PREFIX "CMD_RESET_REW_DIAM_MINMAX",	1);	break;

			case CMD_PLC_GET_INFO:		_do_plc_get_info(socket);										break;
			case CMD_PLC_RESET_ERROR:	sys_reset_error();												break;
			case CMD_PLC_REBOOT:		sys_reboot();													break;				
			case CMD_PLC_GET_LOG:		_do_plc_get_log(socket);											break;
		}
		_plc_get_status();
	}
	return REPLY_OK;
}

//--- simu_init -----------------------------
static void _simu_init(void)
{
	if (_Simulation)
	{
		int i;

		memset(&_Status, 0, sizeof(_Status));
		_Status.versionMajor	 = 1;
		_Status.versionMinor	 = 2;
		_Status.versionBuild	 = 3;
		_Status.versionRevision  = 4;

		_Status.state = STATE_AUTO_CURRENT_OFF;

		_Status.splice1Closed = FALSE;
		_Status.splice2Closed = FALSE;

		_Status.unwinderDiameter = DIAMETER_MAX;
		_Status.rewinderDiameter = DIAMETER_MIN;
		for (i=0; i<SIZEOF(_Status.inkLevel); i++)
			_Status.inkLevel[i]=100;
	}
}

//--- plc_simu_thread ---------------------------------------------
static void* _plc_simu_thread(void *par)
{
	int cnt;

	Error(WARN, 0, "Rexroth in SIMULATION");
	while (_PlcThreadRunning)
	{
		rx_sleep(1000);

		//--- SET Status ----------------------------------------------------------------------------------

		_Status.plcHeartBeat++;
		_Status.unwinderDiameter -= _Status.speed/10;
		if (_Status.unwinderDiameter<DIAMETER_MIN) _Status.unwinderDiameter = DIAMETER_MAX;
		if (_Status.unwinderDiameter>DIAMETER_MAX) _Status.unwinderDiameter = DIAMETER_MIN;

		if (_Status.unwinderDiameter>DIAMETER_MIN+10) BitClear(&_Status.err,  ERR_Unwinder_Diameter);
		else                                          BitSet  (&_Status.err,  ERR_Unwinder_Diameter);
		if (_Status.unwinderDiameter>DIAMETER_MIN+20) BitClear(&_Status.warn, ERR_Unwinder_Diameter);
		else                                          BitSet  (&_Status.warn, ERR_Unwinder_Diameter);

		_Status.rewinderDiameter += _Status.speed/10;
		if (_Status.rewinderDiameter<DIAMETER_MIN) _Status.rewinderDiameter = DIAMETER_MAX;
		if (_Status.rewinderDiameter>DIAMETER_MAX) _Status.rewinderDiameter = DIAMETER_MIN;

		if (_Status.rewinderDiameter<DIAMETER_MAX-10) BitClear(&_Status.err,  ERR_Rewinder_Diameter);
		else                                          BitSet  (&_Status.err,  ERR_Rewinder_Diameter);
		if (_Status.rewinderDiameter<DIAMETER_MAX-20) BitClear(&_Status.warn, ERR_Rewinder_Diameter);
		else                                          BitSet  (&_Status.warn, ERR_Rewinder_Diameter);

		if (_Status.state==STATE_AUTO_PRINTING) 
		{
			int i;
			for (i=0; i<SIZEOF(_Status.inkLevel); i++)
			{		
				if (_Status.inkLevel[i]>0) _Status.inkLevel[i]--;
				else                       _Status.inkLevel[i]=100;

				if (_Status.inkLevel[i]>2)  BitClear(&_Status.inkErr,  i);
				else                        BitSet  (&_Status.inkErr,  i); 
				if (_Status.inkLevel[i]>10) BitClear(&_Status.inkWarn, i);
				else                        BitSet  (&_Status.inkWarn, i); 
			}
			_RunTime++;
		}
		_Status.position = (_RunTime*_Status.speed)/60;

		//--- SEND ------------------------------------------------------------------------------

		cnt = gui_send_msg_2(REP_GET_MINILABEL_STAT, sizeof(_Status), &_Status);
	}

	return NULL;
}

//--- _OnPlcConnected ---------------------------------------------
static void _OnPlcConnected(void)
{
	SSystemInfo info;
	int i;
	ULONG no;

	_Status.plcConnected = TRUE;
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
	_plc_set_config();
}

//--- _OnPlcDisconnected ---------------------------------------------
static void _OnPlcDisconnected(void)
{
	_Status.plcConnected = FALSE;
	TrPrintfL(1, "plc: Deconnected");
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
	lc_set_value_by_name_UINT32(PAR_PREFIX "USR_WEB_WIDTH",				_Config.webWidth);
	lc_set_value_by_name_UINT32(PAR_PREFIX "USR_PAPER_DENSITY",			_Config.paperDensity);
	lc_set_value_by_name_UINT32(PAR_PREFIX "USR_PAPER_THICKNESS",		_Config.paperThickness);
	lc_set_value_by_name_UINT32(PAR_PREFIX "USR_SPEED",					_Config.speed);
	lc_set_value_by_name_UINT32(PAR_PREFIX "USR_TENSION",				_Config.tension);
	lc_set_value_by_name_UINT32(PAR_PREFIX "USR_WINDER_TENSION",		_Config.winderTension);
	lc_set_value_by_name_UINT32(PAR_PREFIX "USR_UNWINDER_WINDDIRECTION",_Config.unwinderClockwise);
	lc_set_value_by_name_UINT32(PAR_PREFIX "USR_REWINDER_WINDDIRECTION",_Config.rewinderClockwise);
	lc_set_value_by_name_UINT32(CMD_PREFIX "CMD_SET_PARAMETER",			1);
}

//--- _plc_get_status ---------------------------------
static void _plc_get_status()
{
	int i;
	int err;
	FLOAT	pos;

	sys_get_display_text(&err, _Status.plcDisplay, sizeof(_Status.plcDisplay), &_Status.plcMode);

	lc_get_value_by_name_UINT32(STA_PREFIX "STA_VERSION_MJAOR",				&_Status.versionMajor);
	lc_get_value_by_name_UINT32(STA_PREFIX "STA_VERSION_MINOR",				&_Status.versionMinor);
	lc_get_value_by_name_UINT32(STA_PREFIX "STA_VERSION_BUILD",				&_Status.versionBuild);
	lc_get_value_by_name_UINT32(STA_PREFIX "STA_VERSION_REVISION",				&_Status.versionRevision);

	lc_get_value_by_name_UINT32(STA_PREFIX "STA_MACHINE_STATE",				(UINT32*)&_Status.state);
	lc_get_value_by_name_UINT32(STA_PREFIX "STA_warn",							&_Status.warn);
	lc_get_value_by_name_UINT32(STA_PREFIX "STA_ERROR",						&_Status.err);

	lc_get_value_by_name_UINT32(STA_PREFIX "STA_ACTUAL_SPEED",					&_Status.speed);
	lc_get_value_by_name_UINT32(STA_PREFIX "STA_ACTUAL_SPEED_MIN",				&_Status.speedMin);
	lc_get_value_by_name_UINT32(STA_PREFIX "STA_ACTUAL_SPEED_MAX",				&_Status.speedMax);

	lc_get_value_by_name_UINT32(STA_PREFIX "STA_ACTUAL_TENSION",				&_Status.tension);
	lc_get_value_by_name_UINT32(STA_PREFIX "STA_ACTUAL_TENSION_MIN",			&_Status.tensionMin);
	lc_get_value_by_name_UINT32(STA_PREFIX "STA_ACTUAL_TENSION_MAX",			&_Status.tensionMax);

	lc_get_value_by_name_FLOAT(STA_PREFIX "STA_MOVED_DISTANCE",				&pos);
	_Status.position = (UINT32)(pos*1000);

	lc_get_value_by_name_UINT32(STA_PREFIX "STA_WEB_THICKNESS",				&_Status.paperThickness);

	lc_get_value_by_name_UINT32(STA_PREFIX "STA_UNWINDER_TENSION_ACT",			&_Status.unwinderTension);
	lc_get_value_by_name_UINT32(STA_PREFIX "STA_UNWINDER_TENSION_ACT_MIN",		&_Status.unwinderTensionMin);
	lc_get_value_by_name_UINT32(STA_PREFIX "STA_UNWINDER_TENSION_ACT_MAX",		&_Status.unwinderTensionMax);
	lc_get_value_by_name_UINT32(STA_PREFIX "STA_UNWINDER_DIAMETER",			&_Status.unwinderDiameter);
	lc_get_value_by_name_UINT32(STA_PREFIX "STA_DIAMETER_UNWINDER_MIN",		&_Status.unwinderDiameterMin);
	lc_get_value_by_name_UINT32(STA_PREFIX "STA_DIAMETER_UNWINDER_MAX",		&_Status.unwinderDiameterMax);
	lc_get_value_by_name_UINT32(STA_PREFIX "STA_UNWINDER_UNROLLED_LENGTH",		&_Status.unwinderLength);

	lc_get_value_by_name_UINT32(STA_PREFIX "STA_REWINDER_TENSION_ACT",			&_Status.rewinderTension);
	lc_get_value_by_name_UINT32(STA_PREFIX "STA_REWINDER_TENSION_ACT_MIN",		&_Status.rewinderTensionMin);
	lc_get_value_by_name_UINT32(STA_PREFIX "STA_REWINDER_TENSION_ACT_MAX",		&_Status.rewinderTensionMax);
	lc_get_value_by_name_UINT32(STA_PREFIX "STA_REWINDER_DIAMETER",			&_Status.rewinderDiameter);
	lc_get_value_by_name_UINT32(STA_PREFIX "STA_DIAMETER_REWINDER_MIN",		&_Status.rewinderDiameterMin);
	lc_get_value_by_name_UINT32(STA_PREFIX "STA_DIAMETER_REWINDER_MAX",		&_Status.rewinderDiameterMax);
	lc_get_value_by_name_UINT32(STA_PREFIX "STA_REWINDER_REMAIN_LENGTH",		&_Status.rewinderLength);

	lc_get_value_by_name_UINT32(STA_PREFIX "STA_INFEED_CLAMP",					&_Status.infeedClosed);
	lc_get_value_by_name_UINT32(STA_PREFIX "STA_OUTFEED_CLAMP",				&_Status.outfeedClosed);
	lc_get_value_by_name_UINT32(STA_PREFIX "STA_SPLICE_1_OUT",					&_Status.splice1Closed);
	lc_get_value_by_name_UINT32(STA_PREFIX "STA_SPLICE_2_OUT",					&_Status.splice2Closed);

	//--- Hert Beat ----------------------------------------------
	lc_get_value_by_name_UINT32(STA_PREFIX "STA_HEARTBEAT_OUT",				&_Status.plcHeartBeat);
	_Status.plcHeartBeat &= 0xff;
	lc_set_value_by_name_UINT32(PAR_PREFIX "USR_HEARTBEAT_IN",				     _Status.plcHeartBeat);

	//--- Ink Level ----------------------------------------------
	for (i=0; i<SIZEOF(_Status.inkLevel); i++)
	{	
		_Status.inkLevel[i]=70;
	}	
	_Status.inkWarn = 0;
	_Status.inkErr  = 0;
}


//--- _do_plc_get_info ---------------------------------------------------
static void _do_plc_get_info(RX_SOCKET socket)
{
	SSystemInfo info;

	if (_Simulation)
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
	SPlcLogItem item;

	#ifndef linux
	if (_Simulation)
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
			if (sys_get_log_item(i, &item)) break;
			sok_send_2(&socket, REP_PLC_GET_LOG, sizeof(item), &item);
		}
	}
	sok_send_2(&socket, END_PLC_GET_LOG, 0, NULL);
	TrPrintfL(1, "PlcLog: %d items sent to %d", i, socket);
}

//--- plc_thread ---------------------------------------------
static void* _plc_thread(void *par)
{
	int ret;

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
			if (app_download(PATH_BIN_REXROTH NAME_APP)==REPLY_OK) Error(WARN, 0, "Updated PLC Application");
			ret = app_transfer_file(PATH_BIN_REXROTH NAME_ACCOUNTS);
			if (ret==0) Error(ERR_CONT, 0, "Access rights to rexroth have changed. PLC has to be restarted manually.");
			if (ret==1) Error(ERR_CONT, 0, "File >>%s<< missing", PATH_BIN_REXROTH NAME_ACCOUNTS);
			if (ret==3) Error(ERR_CONT, 0, "Could not tansfer file >>%s<< to PLC. Check Connection", PATH_BIN_REXROTH NAME_ACCOUNTS);
			while (_NetItem.connected)
			{
				_plc_get_status();
				gui_send_msg_2(REP_GET_MINILABEL_STAT, sizeof(_Status), &_Status);
				rx_sleep(1000);
			}
		}
		else rx_sleep(1000);
	}
	return NULL;
}	