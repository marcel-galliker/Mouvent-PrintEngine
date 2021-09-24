// ****************************************************************************
//
//	DIGITAL PRINTING - robot_client.c
//
//	Communication between ARM processor and FPGA
//
// ****************************************************************************
//
//	Copyright 2016 by radex AG, Switzerland. All rights reserved.
//	Written by Marcel Galliker
//
// ****************************************************************************


// stepper chip documentation: https://www.trinamic.com/fileadmin/assets/Products/ICs_Documents/TMC5130_datasheet_Rev1.17.pdf

#include "rx_common.h"
#include "rx_error.h"
#include "rx_sok.h"
#include "rx_threads.h"
#include "../rx_robot_ctrl/includes/rx_robot_def.h"
#include "../rx_robot_ctrl/includes/rx_robot_tcpip.h"
#include "rx_term.h"
#include "rx_trace.h"
#include "version.h"
#include "stepper_ctrl.h"
#include "lb702.h"
#include "lbrob.h"
#include "robot_client.h"

//--- defines -----------------------------------------------------
// Inputs
#define IN_SCREW_EDGE	0
#define	IN_Z_DOWN		1
#define IN_Z_UP			2
#define IN_Y_END		3
#define IN_GARAGE		4
#define IN_X_END		5

#define MOTOR_XY_0		1
#define MOTOR_XY_1		0
#define MOTOR_SCREW		2
#define MOTOR_Z			3
#define MOTORS_ALL		0x0f
#define MOTORS_XY		((1<<MOTOR_XY_0) | (1<<MOTOR_XY_1))
#define MOTOR_CNT		4

#define ROB_IN(in)   ((_RobotStatus.gpio.inputs>>(in)) & 1)

// Outputs
#define OUTPUT_Z_UP		1
#define OUTPUT_Z_DOWN	0

#define STEPS_PER_REV				51200
#define DISTANCE_UM_PER_REV			36000   // mirco meters per revolution
#define MOTOR_X_GARAGE_POS			20000


#define SCREW_STEPS					213333			// 1/6 turn
#define SCREW_SPEED_HIGH			1280000			// steps/sec
#define SCREW_SPEED_LOW				160000			// steps/sec

#define DISTANCE_PER_TURN			11          // um
#define STEPS_PER_TURN				6           // "encoder"-steps

#define MAX_LENGTH_Z				440000
// #define MAX_LENGTH_Z				350000

#define ENCODER_TOL					250
#define ENCODER_TOL_XY				1000
#define ENCODER_TOL_SCREW			230000

//---													  ddddrrrrrhhhhh
#define IHOLD_IRUN(hold, run, delay)	((((delay)&0xf)<<16) | (((run)&0x1f)<<8) | ((hold)&0x1f))
#define SCREW_CURRENT_HIGH			IHOLD_IRUN(0, 28, 1)		// 0x00011C00
#define SCREW_CURRENT_LOW			IHOLD_IRUN(0, 18, 1)		// 0x00011200
#define XY_CURRENT					IHOLD_IRUN(0, 18, 1)		// 0x00011200
#define Z_CURRENT					IHOLD_IRUN(0, 14, 1)		// 0x00010E00

#define DRIVER_ERROR	0x02		

#define MAX_STEPS_STITCH    (30 * 6)  // 30 turns with 6 steps each turn
#define MAX_STEPS_ANGLE     (16 * 6)  // 16 turns with 6 steps each turn
#define OVERTURN            (2*6)     // 2 turns
#define LOOSE_TURN          (6/2)     // half turn for reserve, that the mechanical end should never be reached anymore

#define CORR_STEP                500
#define CORR_MAX                1000

#define ROB_POSY_FREE           30000   // minimum y-position to allow moving in x

//--- Modlue Globals -----------------------------------------------------------------
static char _IpAddr[32]="";
RX_SOCKET	_RC_Socket = INVALID_SOCKET;
static int	_Connected = FALSE;
static int	_AppRunning = FALSE;
static int	_Timeout;

static char     *_AxisName[2]  = {"ANGLE", "STITCH"};

static int				_RobotUsed=FALSE;
HANDLE					_RobotHdl;
static SRobotStatusMsg	_RobotStatus;

static int				_StatusRecvCnt;
static int				_StatusReqCnt;
static int				_MotorInit;
static SMotorConfig		_MotorCfg[MOTOR_CNT];
static UINT8			_MoveId[MOTOR_CNT];

static SVersion			_FileVersion;
static FILE				*_BinaryFile;
static SVersion			_BinVersion;
static UINT32			_FilePos;
static UINT32			_FileSize;

static int				_RC_State;
static char				_RC_Error[MAX_PATH];
static int				_PosX_ref;

static int				_Error_Written[MOTOR_CNT] = {FALSE};
static int				_StallErrorEnabled = TRUE;

static int				_SpeedFaxt_XY = 1;

static int _ScrewFunction=0;
static int  _ScrewTurned;
static int _RobStateMachine_Step = 0;
static SScrewPositions _ScrewPos;
static SHeadAdjustment _ScrewPar;

static int             _ScrewerStalled;
static int             _GarageWithoutAdjust = FALSE;


#define RC_STATE_REF	1000

//--- Prototypes ------------------------------------------------------
static void* _robot_ctrl_thread(void* lpParameter);
static int _handle_robot_ctrl_msg(RX_SOCKET socket, void *msg, int len, struct sockaddr *sender, void *ppar);
static int _connection_closed(RX_SOCKET socket, const char *peerName);

static int  _config_motors(void);

static void _configure_xy_motor(uint8_t motor);
static void _configure_screw_motor(int current, int speed);
static void _configure_z_motor(void);
static void _send_MotorCfg(int motor);

static void _rc_error(const char *file, int line, char *msg);
static void _rc_reset_motors(int motors);
static void _rc_reset_edgeCtr(int inputs);
static int  _rc_motor_moveBy(int motor, int steps, int stopIn, const char *file, int line);
static int  _rc_motor_moveToStop(int motor, int steps, int input, int level, const char *file, int line);
static int  _rc_moveto_x_stop(int x, int stop, const char *file, int line);
static int  _rc_moveto_y_stop(int y, int stop, const char *file, int line);
static int	_rc_move_xy_done(void);
static int _rc_move_screw_done(void);
static void _rc_set_screwer_speed(int high);

static void _rob_state_machine(void);
static void _set_ScrewPos(SScrewPositions *pos);
static void _save_ScrewPos(void);

static void _rc_state_machine(void);
static void _check_version(void);
static void _check_move(void);

static int _steps_2_micron(int steps);
static int _micron_2_steps(int micron);


static void _download_start(void);
static void _download_data(SBootloaderDataRequestCmd *req);
static void _download_end(void);
static void _setSerialNo(UINT16 no);
static void _robot_error_check(void);
static char *_motor_name(int no);


//--- rc_init ----------------------------------------------------
void rc_init(int robotUsed)
{
	_RobotUsed = robotUsed;
	if (_RobotUsed)
	{
		RX_StepperStatus.screwerinfo.moving = FALSE;
		RX_StepperStatus.screwerinfo.ref_done = FALSE;
	//	RX_StepperStatus.screwerinfo.ref_x_done = FALSE;
	//	RX_StepperStatus.screwerinfo.ref_y_done = FALSE;
	
		_MotorInit = 0;
		if (!_RobotHdl)
			_RobotHdl  = rx_thread_start(_robot_ctrl_thread, NULL, 0, "RC_Client");
	}
	else
	{
		RX_StepperStatus.screwerinfo.moving    = FALSE;
		RX_StepperStatus.screwerinfo.ref_done  = TRUE;
		RX_StepperStatus.screwerinfo.z_in_down = TRUE;
		RX_StepperStatus.screwerinfo.in_garage = TRUE;
	}
}

//--- _level -------------------------------------
static char _level(int l)
{
	if (l) return '*';
	return '_';
}

//--- _pos_str ------------------------------------
static char* _pos_str(int x, int y) 
{
    static char str[30];
    sprintf(str, "(x=%d.%03d, y=%d.%03d)", x/1000, abs(x)%1000, y/1000, abs(y)%1000);
    return str;
}

//--- _corr_pos_str --------------------------------
static char* _corr_pos_str(SScrewPos pos, SPoint corr) 
{
    static char str[60];
    int xx=pos.x+corr.x;
    int yy=pos.y+corr.y;
    sprintf(str, "(x=%d.%03d(%d%03d+%d%03d), y=%d.%03d(%d%03d+%d%03d))", 
        xx/1000, abs(xx)%1000, pos.x/1000, abs(pos.x)%1000, corr.x/1000, abs(corr.x)%1000, 
        yy/1000, abs(yy)%1000, pos.y/1000, abs(pos.y)%1000, corr.y/1000, abs(corr.y)%1000);
    return str;
}

//--- rc_used -------------------------
int rc_used(void)
{
	return _RobotUsed;
}

//--- rc_config ------------------------------------------------
void rc_config(int boardNo)
{
//	boardNo=1;
//	Error(WARN, 0, "TEST: rc_config(%d)", boardNo);
	sprintf(_IpAddr, RX_CTRL_SUBNET "%d", 50+10*boardNo);
}

//--- rc_main -----------------------------------------------
void rc_main(int ticks, int menu)
{
    if (_RC_Socket!=INVALID_SOCKET)
	{
        RX_StepperStatus.screwerinfo.in_garage   = ROB_IN(IN_GARAGE);
        RX_StepperStatus.screwerinfo.z_in_down   = ROB_IN(IN_Z_DOWN);
        RX_StepperStatus.screwerinfo.z_in_up     = ROB_IN(IN_Z_UP);

		_robot_error_check();

		if (!_RobStateMachine_Step && !RX_StepperStatus.info.moving && !RX_StepperStatus.robinfo.moving && !RX_StepperStatus.screwerinfo.moving && 
				!RX_StepperStatus.screwerinfo.screwer_blocked_left && !RX_StepperStatus.screwerinfo.screwer_blocked_right)
			RX_StepperStatus.screwerinfo.screwer_ready = TRUE;
		else
			RX_StepperStatus.screwerinfo.screwer_ready = FALSE;


		if (_RobStateMachine_Step) _rob_state_machine();
		else _ScrewTurned = TRUE;
	}
}

//--- rx_enable_stall_error ---------------------------------------
void rx_enable_stall_error(int enable)
{
	_StallErrorEnabled = enable;
}

//--- _robot_error_check ---------------------------------------------
static void _robot_error_check(void)
{
	for (int i = 0; i < SIZEOF(_RobotStatus.motor); i++)
	{
		if (_RobotStatus.motor[i].status & DRIVER_ERROR && !_Error_Written[i])
		{
			Error(ERR_CONT, 0, "Driver Error on Robot-Motor %s", _motor_name(i));
			_Error_Written[i] = TRUE;
		}
		else if (!(_RobotStatus.motor[i].status & DRIVER_ERROR))
			_Error_Written[i] = FALSE;
    }
}

//--- rc_clear_error ---------------------------------------------------
void rc_clear_error()
{
	memset(_Error_Written, 0, sizeof(_Error_Written));
}

//--- _robot_ctrl_thread --------------------------------------------------------
static void* _robot_ctrl_thread(void* lpParameter)
{
	int errNo;
	TrPrintfL(TRUE, "_robot_ctrl_thread started");
	while(TRUE)
	{
	//	TrPrintfL(TRUE, "_robot_ctrl_thread _RC_Socket=%d, _IpAddr=>>%s<<", _RC_Socket, _IpAddr);
		if (_RC_Socket!=INVALID_SOCKET)
		{
			SMsgHdr msg;
			msg.msgId = CMD_STATUS_GET;
			msg.msgLen = sizeof(msg);
			sok_send(&_RC_Socket, &msg);
			_StatusReqCnt++;
            #ifndef DEBUG
			if (_Connected && _Timeout && (--_Timeout)==0)
			{
				Error(ERR_CONT, 0, "Connection to robot lost");
				_Connected=FALSE;
				_MotorInit = 0;
			}
            #endif
		//	TrPrintfL(TRUE, "sent CMD_STATUS_GET");
		}
		else if (_IpAddr[0] && sok_ping(_IpAddr)==REPLY_OK)
		{
			errNo=sok_open_client_2(&_RC_Socket, _IpAddr, PORT_UDP_CTRL, SOCK_DGRAM, _handle_robot_ctrl_msg, _connection_closed);
			if (errNo == REPLY_OK)
			{
				Error(LOG, 0, "Robot Connected %s", _IpAddr);
			}
		}
		rx_sleep(1000);
	}
	return NULL;
}

//--- _connection_closed --------------------------------
static int _connection_closed(RX_SOCKET socket, const char *peerName)
{
	Error(ERR_CONT, 0, "Connection to robot lost");
	_RC_Socket = INVALID_SOCKET;
	_MotorInit = 0;
}

//--- _config_motors -------------------
static int _config_motors(void)
{
	int motor;
	int ok=TRUE;
	for (motor=0; motor<MOTOR_CNT; motor++)
	{
		if (!_RobotStatus.motor[motor].isConfigured) _MotorInit &= ~(1<<motor);
		if (!(_MotorInit & (1<<motor))) 
		{
			switch(motor)
			{
            case MOTOR_XY_0:	_configure_xy_motor(motor);	break;
            case MOTOR_XY_1:	_configure_xy_motor(motor);	break;
            case MOTOR_SCREW:	_configure_screw_motor(SCREW_CURRENT_LOW, SCREW_SPEED_HIGH);	break;
            case MOTOR_Z:		_configure_z_motor();		break;
			}
			_rc_reset_motors(1<<motor);
			_MotorInit |= (1<<motor);
			_MoveId[motor] = _RobotStatus.motor[motor].moveIdDone;
			ok=FALSE;
		}
	}
	if (!ok) _rc_reset_edgeCtr(0xff);
 	return ok;
}

//--- _handle_robot_ctrl_msg --------------------------------------------------------
static int _handle_robot_ctrl_msg(RX_SOCKET socket, void *msg, int len, struct sockaddr *sender, void *ppar)
{
	SMsgHdr			*phdr   = (SMsgHdr*)msg;
	SRobotTraceMsg  *ptrace = (SRobotTraceMsg*)msg;

	_Connected  = TRUE;
	_Timeout	= 3;

	switch(phdr->msgId)
	{
    case CMD_STATUS_GET:	memcpy(&_RobotStatus, msg, sizeof(_RobotStatus));
							_StatusRecvCnt++;
						//	TrPrintfL(TRUE, "got CMD_STATUS_GET moveIdStarted=%d, moveIdDone=%d, isMoving=%d, isStalled=%d", 
						//		_RobotStatus.motor[MOTOR_Z].moveIdStarted, _RobotStatus.motor[MOTOR_Z].moveIdDone, _RobotStatus.motor[MOTOR_Z].isMoving, _RobotStatus.motor[MOTOR_Z].isStalled);
							_check_version();
							if (_AppRunning) 
                            {
							    _check_move();
                                _rc_state_machine();
                            }
                            break;

    case CMD_BOOTLOADER_DATA: _download_data((SBootloaderDataRequestCmd*) msg); break;
    case CMD_BOOTLOADER_END:  _download_end(); break;	

    case CMD_TRACE:			TrPrintfL(TRUE, "ROBOT: %s", ptrace->message);
							break;

	default:				Error(WARN, 0, "Got unknown messageId=0x%08x", phdr->msgId);
	}
	return REPLY_OK;
}

//---- rc_reference -----------------------
void rc_reference(int x)
{
	TrPrintfL(TRUE, "rc_reference");
	_PosX_ref = x;
	_rc_reset_motors(MOTORS_ALL);
	_rc_reset_edgeCtr(0xff);
	_RC_State = RC_STATE_REF;
	_rc_state_machine();
}

//--- _steps_2_micron ----------------------------------------------------------------
static int _steps_2_micron(int steps)
{
    int micron;
    micron = (int)(0.5 + ((double )steps * DISTANCE_UM_PER_REV / STEPS_PER_REV));
    return micron;
}

//--- _micron_2_steps --------------------------------------------------------------
static int _micron_2_steps(int micron)
{
    int steps;
    steps = (int)(0.5 + ((double )micron * STEPS_PER_REV / DISTANCE_UM_PER_REV));
    return steps;
}

//--- _get_file_version ---------------------------
static int _get_file_version(char *path, SVersion *pversion)
{
	FILE	*file;
	char	str[64];

	//--- read from bin file ----------------
	file = fopen(path, "rb");
	if (!file)
	{
		Error(ERR_CONT, 0, "read file error >>%s<<", path);
		term_printf("read file error >>%s<<\n", path); 
		term_flush();
		return REPLY_ERROR;
	}
	fseek(file, 0, SEEK_END);
	long fsize = ftell(file);

	fseek(file, fsize-sizeof(str)+1, SEEK_SET);
	memset(str, 0, sizeof(str));
	fread(str, 1, sizeof(str)-1, file);
	//---  get file version ---------------
	for (int i=0; i<sizeof(str); i++)
	{
		if (sscanf(&str[i], "Version=%lu.%lu.%lu.%lu", &pversion->major, &pversion->minor, &pversion->revision, &pversion->build)==4)
			return REPLY_OK;			
	}
	fclose(file);
	return REPLY_ERROR;
}

//--- _check_move --------------------------------------
static void _check_move(void)
{
	static int _movedIdDone[MOTOR_CNT];
	for (int motor=0; motor<MOTOR_CNT; motor++)
	{
		if (_RobotStatus.motor[motor].moveIdDone != _movedIdDone[motor])
		{
			SMotorStatus *stat=&_RobotStatus.motor[motor];
			if (stat->isStalled)
			{
				if (_StallErrorEnabled)
					Error(ERR_CONT, 0, "ROBOT: Motor[%d].MoveId=%d, STALLED: motorPos=%d, encPos=%d, diff=%d", motor, 
						stat->moveIdDone, stat->motorPos, stat->encPos, stat->motorPos-stat->encPos);
			}
			else
				TrPrintfL(TRUE, "ROBOT: Motor[%d].MoveId=%d, DONE", motor, stat->moveIdDone);
			_movedIdDone[motor] = _RobotStatus.motor[motor].moveIdDone;
		}
	}
}

//--- _check_version ----------------------------
static void _check_version(void)
{
	static char _versionStr[32]="";
	if (strcmp(_RobotStatus.version, _versionStr) && (_RobotStatus.serialNo==RX_StepperCfg.robotSerialNo || RX_StepperCfg.boardNo>0))
	{
		TrPrintf(TRUE, "new Version %s", _RobotStatus.version);
		memcpy(_versionStr, _RobotStatus.version, sizeof(_versionStr));
		if (!strcmp(_RobotStatus.version, "0.0.0.0"))
			_download_start();
		else
		{
			//---  get file version ---------------
			sscanf(_versionStr, "%lu.%lu.%lu.%lu", &_BinVersion.major, &_BinVersion.minor, &_BinVersion.revision, &_BinVersion.build);
			if (_get_file_version(PATH_BIN_STEPPER FILENAME_ROBOT_CTRL, &_FileVersion)==REPLY_OK)	
			{
				if (_BinVersion.major!=_FileVersion.major || _BinVersion.minor!=_FileVersion.minor ||  _BinVersion.revision!=_FileVersion.revision || _BinVersion.build!=_FileVersion.build)
				{
					Error(ERR_CONT, 0, "Robot version changed: Repower robot board for download");
				}
			}
			_AppRunning = TRUE;
		}
	}
}

//--- _download_start ------------------------------------
static void _download_start(void)
{
	TrPrintfL(TRUE, "_download_start");
	Error(LOG, 0, "Robot #%d download start", _RobotStatus.serialNo);
	_BinaryFile = fopen(PATH_BIN_STEPPER FILENAME_ROBOT_CTRL, "rb");
	if (_BinaryFile)
	{
		fseek(_BinaryFile, 0, SEEK_END);
		SBootloaderStartCmd cmd;
		cmd.header.msgId  = CMD_BOOTLOADER_START;
		cmd.header.msgLen = sizeof(cmd);
		cmd.size		  = _FileSize = ftell(_BinaryFile);
		sok_send(&_RC_Socket, &cmd);
	}			
}

//--- _download_data --------------------------
static void _download_data(SBootloaderDataRequestCmd *req)
{
//	TrPrintfL(TRUE, "_download_data (filePos=0x%x) blk=%d, rest=%d", req->filePos, req->filePos/BOOTLOADER_DATA_FRAME_SIZE, req->filePos%BOOTLOADER_DATA_FRAME_SIZE);
	if (_BinaryFile)
	{
		SBootloaderDataCmd cmd;
		cmd.header.msgId = CMD_BOOTLOADER_DATA;
		cmd.header.msgLen= sizeof(cmd); 
		cmd.filePos		 = _FilePos = req->filePos;
		fseek(_BinaryFile, req->filePos, SEEK_SET);
		cmd.length=fread(cmd.data, 1, sizeof(cmd.data), _BinaryFile);
		sok_send(&_RC_Socket, &cmd);
	}
}

//--- _download_end ------------------------
static void _download_end(void)
{
	TrPrintfL(TRUE, "_download_end");
	if (_BinaryFile)
	{
		fclose(_BinaryFile);
		_BinaryFile = NULL;
		SMsgHdr cmd;
		cmd.msgId = CMD_BOOTLOADER_REBOOT;
		cmd.msgLen= sizeof(cmd); 
		sok_send(&_RC_Socket, &cmd);
	}
}

//--- _setSerialNo ---------------------------------------
static void _setSerialNo(UINT16 no)
{
	if (_AppRunning)
	{
	    SBootloaderSerialNoCmd cmd;
	    cmd.header.msgId = CMD_BOOTLOADER_SERIALNO;
	    cmd.header.msgLen= sizeof(cmd);
	    cmd.serialNo     = no;
	    sok_send(&_RC_Socket, &cmd);
	}
}

//--- _rc_state_machine --------------------------------
static void _rc_state_machine(void)
{
	if (!_config_motors()) return;
	static int _started = FALSE;

    RX_StepperStatus.screw_posX = (_steps_2_micron(_RobotStatus.motor[MOTOR_XY_1].encPos + _RobotStatus.motor[MOTOR_XY_0].encPos))/2;
    RX_StepperStatus.screw_posY = (_steps_2_micron(_RobotStatus.motor[MOTOR_XY_1].encPos - _RobotStatus.motor[MOTOR_XY_0].encPos))/2;
	RX_StepperStatus.screw_posZ = (_RobotStatus.motor[MOTOR_Z].motorPos);
		
	switch(_RC_State)
	{        
		case 0:					break;
	
		case RC_STATE_REF:		// move Z to bottom
								memset(_RC_Error, 0, sizeof(_RC_Error));
								TrPrintfL(TRUE, "RC_STATE_REF: down=%d, moving=%d, start=%d, done=%d", ROB_IN(IN_Z_DOWN), _RobotStatus.motor[MOTOR_Z].isMoving, _RobotStatus.motor[MOTOR_Z].moveIdStarted, _RobotStatus.motor[MOTOR_Z].moveIdDone);
								if (!ROB_IN(IN_Z_DOWN))
                                    rc_move_bottom(_FL_);
								_RC_State++;
							//	_RC_State = RC_STATE_REF +101;
								break;

		case RC_STATE_REF+1:	// z at bottom
								_started = FALSE;
								_SpeedFaxt_XY = 1;
								_configure_xy_motor(MOTOR_XY_0);
								_configure_xy_motor(MOTOR_XY_1);
								TrPrintfL(TRUE, "RC_STATE_REF+1: down=%d, moving=%d, start=%d, done=%d", ROB_IN(IN_Z_DOWN), _RobotStatus.motor[MOTOR_Z].isMoving, _RobotStatus.motor[MOTOR_Z].moveIdStarted, _RobotStatus.motor[MOTOR_Z].moveIdDone);
								if (ROB_IN(IN_Z_DOWN))
								{
                                    _rc_moveto_y_stop(1000000, IN_Y_END, _FL_);
									rc_screwer_to_ref();
									_RC_State++;
								}
								else if (_RobotStatus.motor[MOTOR_Z].moveIdDone == _MoveId[MOTOR_Z]) 
								{
									_rc_error(_FL_, "Robot Z-Axis down sensor not found");
								}
								break;

		case RC_STATE_REF+2:	// Y in end sensor (out)
								if (rc_move_started()) _started = TRUE;
								if (_rc_move_xy_done() && _started)  
								{
									TrPrintfL(TRUE, "RC_STATE_REF+2: y-end=%d, start=%d %d, done=%d %d", ROB_IN(IN_Y_END), 
										_RobotStatus.motor[MOTOR_XY_0].moveIdStarted, _RobotStatus.motor[MOTOR_XY_1].moveIdStarted, 
										_RobotStatus.motor[MOTOR_XY_0].moveIdDone, _RobotStatus.motor[MOTOR_XY_1].moveIdDone);
									if (ROB_IN(IN_Y_END))
									{
										if (ROB_IN(IN_X_END))
										{
											TrPrintfL(TRUE, "Move from X-sensor");
											_rc_reset_motors(MOTORS_XY);
											rc_moveto_x(RX_StepperStatus.screw_posX+5000, _FL_);
										}
									//	rc_move_top(_FL_);
										_RC_State++;
									}
									else
									{
										_rc_error(_FL_, "Robot Y-Axis end sensor not found");
										_RC_State = RC_STATE_REF;
									}
								}													
								break;

		case RC_STATE_REF+3:	if (_rc_move_xy_done() && _rc_move_screw_done() )  
								{
									TrPrintfL(TRUE, "RC_STATE_REF+3: x-end=%d, start=%d %d, done=%d %d", ROB_IN(IN_X_END), 
										_RobotStatus.motor[MOTOR_XY_0].moveIdStarted, _RobotStatus.motor[MOTOR_XY_1].moveIdStarted, 
										_RobotStatus.motor[MOTOR_XY_0].moveIdDone, _RobotStatus.motor[MOTOR_XY_1].moveIdDone);
									_rc_moveto_x_stop(-150000, IN_X_END, _FL_);
									_RC_State++;
								}
								break;

		case RC_STATE_REF+4:	// X in end sensor
								if (_rc_move_xy_done())  
								{
									TrPrintfL(TRUE, "RC_STATE_REF+4: y-end=%d, start=%d %d, done=%d %d", ROB_IN(IN_Y_END), 
										_RobotStatus.motor[MOTOR_XY_0].moveIdStarted, _RobotStatus.motor[MOTOR_XY_1].moveIdStarted, 
										_RobotStatus.motor[MOTOR_XY_0].moveIdDone, _RobotStatus.motor[MOTOR_XY_1].moveIdDone);
									if (ROB_IN(IN_X_END))
									{
										_rc_reset_motors(MOTORS_XY);
										_SpeedFaxt_XY = 4;
										_configure_xy_motor(MOTOR_XY_0);
										_configure_xy_motor(MOTOR_XY_1);
										_RC_State++;
									}
									else
									{
										_RC_State = 0;
										_rc_error(_FL_, "X-Axis end sensor not found");
									}
								}
								break;

		case RC_STATE_REF+5:	// XY-Axes reseted
								if (!_RobotStatus.motor[MOTOR_XY_0].isStalled && !_RobotStatus.motor[MOTOR_XY_1].isStalled)
								{
									rc_moveto_x(MOTOR_X_GARAGE_POS, _FL_);
									_RC_State++;
								}
								break;

		case RC_STATE_REF+6:	// X in garage-pos
								if (_rc_move_xy_done())  
								{
									TrPrintfL(TRUE, "RC_STATE_REF+5: y-end=%d, start=%d %d, done=%d %d", ROB_IN(IN_Y_END), 
										_RobotStatus.motor[MOTOR_XY_0].moveIdStarted, _RobotStatus.motor[MOTOR_XY_1].moveIdStarted, 
										_RobotStatus.motor[MOTOR_XY_0].moveIdDone, _RobotStatus.motor[MOTOR_XY_1].moveIdDone);	
									if (_RobotStatus.motor[MOTOR_XY_0].isStalled || _RobotStatus.motor[MOTOR_XY_1].isStalled)
									{
										_rc_error(_FL_, "Robot X-Axis blocked");
										_RC_State=RC_STATE_REF+3;
									}
									else 
									{
										_rc_moveto_y_stop(-200000, IN_GARAGE, _FL_);
										_RC_State++;
									}
								}
								break;

		case RC_STATE_REF+7:	// Y in garage sensor
								if (_rc_move_xy_done())  								
								{									
									TrPrintfL(TRUE, "RC_STATE_REF+6: y-end=%d, start=%d %d, done=%d %d", ROB_IN(IN_Y_END), 
										_RobotStatus.motor[MOTOR_XY_0].moveIdStarted, _RobotStatus.motor[MOTOR_XY_1].moveIdStarted, 
										_RobotStatus.motor[MOTOR_XY_0].moveIdDone, _RobotStatus.motor[MOTOR_XY_1].moveIdDone);	
									if (ROB_IN(IN_GARAGE))					
									{
										_rc_reset_motors(MOTORS_XY);
										_RC_State=0;
										Error(LOG, 0, "Robot Reference done");
                                        RX_StepperStatus.screwerinfo.ref_done = TRUE;
                                    }
									else _rc_error(_FL_, "Robot Y-Axis garage sensor not found");
								}
								break;				

		default:				Error(WARN, 0, "rc_state_machine: State %d not implemented", _RC_State);
								break;
	}
}

//--- _send_MotorCfg ----------------------------
static void _send_MotorCfg(int motor)
{
	if (_AppRunning)
	{
	    SRobotMotorSetCfgCmd configCommand;
	    memset(&configCommand, 0, sizeof(configCommand));
	    
	    configCommand.header.msgId = CMD_MOTOR_SET_CONFIG;
	    configCommand.header.msgLen = sizeof(configCommand);
	    
	    configCommand.motor = motor;
	    memcpy(&configCommand.config, &_MotorCfg[motor], sizeof(configCommand.config));	
	    
	    sok_send(&_RC_Socket, &configCommand);
	    TrPrintfL(TRUE, "sent CMD_MOTOR_SET_CONFIG");
	}
}

//--- configure_xy_motor -------------------------------------
static void _configure_xy_motor(uint8_t motor)
{
	_MotorCfg[motor].gconf = 0;
	_MotorCfg[motor].swmode = 0x03;
    _MotorCfg[motor].iholdirun = XY_CURRENT;
	_MotorCfg[motor].tpowerdown = 0;
	_MotorCfg[motor].tpwmthrs = 0;
	_MotorCfg[motor].rampmode = 0;
	_MotorCfg[motor].vmax = _SpeedFaxt_XY*26844;
	_MotorCfg[motor].v1 = 0;
	_MotorCfg[motor].amax = 440;
	_MotorCfg[motor].dmax = 440;
	_MotorCfg[motor].a1 = 440;
	_MotorCfg[motor].d1 = 440;
	_MotorCfg[motor].vstart = 0;
	_MotorCfg[motor].vstop = 10;
	_MotorCfg[motor].tzerowait = 0;
	_MotorCfg[motor].vdcmin = 0;
	_MotorCfg[motor].chopconf = 0x30375;
	_MotorCfg[motor].coolconf = 0;
	_MotorCfg[motor].pwmconf = 0;
	_MotorCfg[motor].tcoolthrs = 0;
	_MotorCfg[motor].thigh = 0;
	_MotorCfg[motor].encmode = 0x400;
	_MotorCfg[motor].encconst = 0x307CB;

	_send_MotorCfg(motor);
}

//--- configure_screw_motor ------------------------
static void _configure_screw_motor(int current, int speed)
{		
	_MotorCfg[MOTOR_SCREW].gconf = 0;
	_MotorCfg[MOTOR_SCREW].swmode = 0x03;
	_MotorCfg[MOTOR_SCREW].iholdirun = current;
	_MotorCfg[MOTOR_SCREW].tpowerdown = 0;
	_MotorCfg[MOTOR_SCREW].tpwmthrs = 0;
	_MotorCfg[MOTOR_SCREW].rampmode = 0;
    _MotorCfg[MOTOR_SCREW].vmax = speed;
	_MotorCfg[MOTOR_SCREW].v1 = 0;
	_MotorCfg[MOTOR_SCREW].amax = 4*2932;
	_MotorCfg[MOTOR_SCREW].dmax = 4*2932;
	_MotorCfg[MOTOR_SCREW].a1 = 2932;
	_MotorCfg[MOTOR_SCREW].d1 = 2932;
	_MotorCfg[MOTOR_SCREW].vstart = 0;
	_MotorCfg[MOTOR_SCREW].vstop = 10;
	_MotorCfg[MOTOR_SCREW].tzerowait = 0;
	_MotorCfg[MOTOR_SCREW].vdcmin = 0;
	_MotorCfg[MOTOR_SCREW].chopconf = 0x101F5;
	_MotorCfg[MOTOR_SCREW].coolconf = 0;
	_MotorCfg[MOTOR_SCREW].pwmconf = 0;
	_MotorCfg[MOTOR_SCREW].tcoolthrs = 0;
	_MotorCfg[MOTOR_SCREW].thigh = 0;
	_MotorCfg[MOTOR_SCREW].encmode = 0;
	_MotorCfg[MOTOR_SCREW].encconst = 0;
	
	_send_MotorCfg(MOTOR_SCREW);
}

//--- configure_z_motor ---------------------
static void _configure_z_motor(void)
{
	_MotorCfg[MOTOR_Z].gconf = 0;
	_MotorCfg[MOTOR_Z].swmode = 0x03;
	_MotorCfg[MOTOR_Z].iholdirun = Z_CURRENT;
	_MotorCfg[MOTOR_Z].tpowerdown = 0;
	_MotorCfg[MOTOR_Z].tpwmthrs = 0;
	_MotorCfg[MOTOR_Z].rampmode = 0;
	_MotorCfg[MOTOR_Z].vmax = 6*322122;
	_MotorCfg[MOTOR_Z].v1 = 0;
	_MotorCfg[MOTOR_Z].amax = 5277;
	_MotorCfg[MOTOR_Z].dmax = 5277;
	_MotorCfg[MOTOR_Z].a1 = 5277;
	_MotorCfg[MOTOR_Z].d1 = 5277;
	_MotorCfg[MOTOR_Z].vstart = 0;
	_MotorCfg[MOTOR_Z].vstop = 10;
	_MotorCfg[MOTOR_Z].tzerowait = 0;
	_MotorCfg[MOTOR_Z].vdcmin = 0;
	_MotorCfg[MOTOR_Z].chopconf = 0x30475;
	_MotorCfg[MOTOR_Z].coolconf = 0;
	_MotorCfg[MOTOR_Z].pwmconf = 0;
	_MotorCfg[MOTOR_Z].tcoolthrs = 0;
	_MotorCfg[MOTOR_Z].thigh = 0;
	_MotorCfg[MOTOR_Z].encmode = 0;
	_MotorCfg[MOTOR_Z].encconst = 0;
		
	_send_MotorCfg(MOTOR_Z);
}

//--- _rc_error --------------------------------------
static void _rc_error(const char *file, int line, char *msg)
{
	if (!*_RC_Error)
	{
		strcpy(_RC_Error, msg);
		Error(LOG_TYPE_ERROR_CONT, file, line, 0, msg);
	}
}

//--- rc_stop_motors --------------------------------
void rc_stop_motors(int motors)
{
	if (_AppRunning)
	{
	    SRobotMotorsStopCmd cmd;
	    cmd.header.msgLen = sizeof(cmd);
	    cmd.header.msgId  = CMD_MOTORS_STOP;
	    cmd.motors = motors;
	    sok_send(&_RC_Socket, &cmd);
	    TrPrintfL(TRUE, "sent CMD_MOTORS_STOP");
	}
}

//--- rc_stop --------------------------------
void rc_stop(void)
{
	if (_AppRunning)
	{
		rc_stop_motors(0xff);
        _RC_State = 0;
	    _RobStateMachine_Step = 0;
	//	RX_StepperStatus.screwerinfo.ref_done   = FALSE;
	//	RX_StepperStatus.screwerinfo.ref_x_done = FALSE;
	//	RX_StepperStatus.screwerinfo.ref_y_done = FALSE;
	}
}

//--- _rc_reset_motors --------------------------------
static void _rc_reset_motors(int motors)
{
	if (_AppRunning)
	{
		if (_RobotStatus.motor[MOTOR_SCREW].isStalled)
			printf("test\n");
	    SRobotMotorsResetCmd cmd;
	    cmd.header.msgLen = sizeof(cmd);
	    cmd.header.msgId  = CMD_MOTORS_RESET;
	    cmd.motors		  = motors;
	    sok_send(&_RC_Socket, &cmd);
	    TrPrintfL(TRUE, "sent CMD_MOTORS_RESET");
	    for (int motor=0; motor<MOTOR_CNT; motor++)
		{
		    if (motors & (1<<motor)) 
			{
				_RobotStatus.motor[motor].isMoving  = FALSE;
				_RobotStatus.motor[motor].isStalled = FALSE;
			}
		}
	}
}

//--- _rc_reset_edgeCtr ----------------------------
static void _rc_reset_edgeCtr(int inputs)
{
	if (_AppRunning)
	{
	    SGpioSetCmd cmd;
	    cmd.header.msgLen = sizeof(cmd);
	    cmd.header.msgId  = CMD_GPIO_IN_RESET;
	    cmd.outputs		  = inputs;
	    sok_send(&_RC_Socket, &cmd);
	    TrPrintfL(TRUE, "sent CMD_GPIO_RESET");
	}
}

//--- _rc_motor_moveBy --------------------------
static int _rc_motor_moveBy(int motor, int steps, int stopIn, const char *file, int line)
{
	if (_AppRunning)
	{
	    if (_RobotStatus.motor[motor].isMoving)  return Error(LOG_TYPE_ERROR_CONT, file, line, 0, "_rc_motor_moveBy: Motor[%d].isMoving", motor);
	    if (_RobotStatus.motor[motor].isStalled && motor != MOTOR_SCREW) return Error(LOG_TYPE_ERROR_CONT, file, line, 0, "_rc_motor_moveBy: Motor[%d].isStalled", motor);
        
        SRobotMotorsMoveCmd cmd;
	    memset(&cmd, 0, sizeof(cmd));
	    cmd.header.msgLen	 = sizeof(cmd);
	    cmd.header.msgId	 = CMD_MOTORS_MOVE;
	    cmd.motors			 = 1<<motor;
        cmd.moveId[motor] = ++_MoveId[motor];
        if (motor == MOTOR_SCREW)
        {
            _rc_reset_motors(1 << MOTOR_SCREW);
			cmd.targetPos[motor] = steps;
        }
		else cmd.targetPos[motor] = _RobotStatus.motor[motor].targetPos + steps;
	    if (_MotorCfg[motor].encmode)
	    {
            cmd.encoderCheck[motor] = ENC_CHECK_ENC;
		    cmd.encoderTol[motor]= ENCODER_TOL;
	    }
	    if (motor==MOTOR_SCREW)
	    {
			if (stopIn>=0)
			{
				cmd.stopBits[motor]     = 1<<stopIn;
				cmd.stopBitLevel[motor] = STOP_ON_EDGE; 
			}
			else cmd.stopBits[motor] = 0;
		    cmd.encoderCheck[motor] = ENC_CHECK_IN;
			cmd.edgeCheckIn[motor]  = IN_SCREW_EDGE;
		    cmd.encoderTol[motor]   = ENCODER_TOL_SCREW;		
	    }
	    
        sok_send(&_RC_Socket, &cmd);
	    
	    TrPrintfL(TRUE, "sent CMD_MOTORS_MOVE");
	}
	return REPLY_OK;
}

//--- _rc_motor_moveToStop --------------------------------
static int _rc_motor_moveToStop(int motor, int steps, int stopInput, int level, const char *file, int line)
{
	if (_AppRunning)
	{
	    if (_RobotStatus.motor[motor].isMoving)  
        {
			Error(LOG_TYPE_ERROR_CONT, file, line, 0, "_rc_motor_moveToStop: Motor[%d].isMoving, MoveId=%d, started=%d, done=%d", motor, _MoveId[motor], _RobotStatus.motor[motor].moveIdStarted, _RobotStatus.motor[motor].moveIdDone);
			return REPLY_OK;
        }
	    if (_RobotStatus.motor[motor].isStalled) 
        {
			return Error(LOG_TYPE_ERROR_CONT, file, line, 0, "_rc_motor_moveToStop: Motor[%d].isStalled", motor);
        }
    
	    SRobotMotorsMoveCmd cmd;
	    memset(&cmd, 0, sizeof(cmd));
	    cmd.header.msgLen		 = sizeof(cmd);
	    cmd.header.msgId		 = CMD_MOTORS_MOVE;
	    cmd.motors				 = 1<<motor;
	    cmd.moveId[motor]		 = ++_MoveId[motor];
	    cmd.targetPos[motor]	 =  steps;
		if (stopInput>=0)
		{
			cmd.stopBits[motor] = (1<<stopInput);
			cmd.stopBitLevel[motor]  = level;
		}
    
	    TrPrintfL(TRUE, "sent CMD_MOTORS_MOVE[%d].moveIdStarted=%d", motor, _MoveId[motor]);
    
	    sok_send(&_RC_Socket, &cmd);
    }
	return REPLY_OK;
}

//--- rc_moveto_x -------------------------------------------
int rc_moveto_x(int x, const char *file, int line)
{
    x = _micron_2_steps(x);
    return _rc_moveto_x_stop(x, -1, file, line);
}

//--- rc_moveto_y -------------------------------------------
int rc_moveto_y(int y, const char *file, int line)
{
    y = _micron_2_steps(y);
	if (y>=0) return _rc_moveto_y_stop(y, -1, file, line);
	else	  return _rc_moveto_y_stop(-200000, IN_GARAGE, file, line);
}

//--- rc_moveby_z ------------------------------------
int  rc_moveby_z(int z, const char *file, int line)
{
    _rc_reset_motors(1<<MOTOR_Z);
    return _rc_motor_moveToStop(MOTOR_Z, z, IN_Z_UP, STOP_ON_HIGH, _FL_);	
}

//--- _rc_moveto_x_stop -----------------------------------
static int _rc_moveto_x_stop(int x, int stop, const char *file, int line)
{
	if (_AppRunning)
	{
	    for (int motor=MOTOR_XY_0; motor<=MOTOR_XY_1; motor++)
	    {
		    if (_RobotStatus.motor[motor].isMoving)  return Error(LOG_TYPE_ERROR_CONT, file, line, 0, "_rc_moveto_x_stop: Motor[%d].isMoving", motor);
		    if (_RobotStatus.motor[motor].isStalled) return Error(LOG_TYPE_ERROR_CONT, file, line, 0, "_rc_moveto_x_stop: Motor[%d].isStalled", motor);
	    }
	    
	    TrPrintfL(TRUE, "sent CMD_MOTORS_MOVE_X (%d)", x);
    
        int dx = x - _micron_2_steps(RX_StepperStatus.screw_posX);
    
	    if (_RobotStatus.motor[MOTOR_XY_0].moveIdDone!=_MoveId[MOTOR_XY_0])	
		    TrPrintfL(TRUE, "Motor[MOTOR_XY_0].moveIdDone=%d (%d)", _RobotStatus.motor[MOTOR_XY_0].moveIdDone, _MoveId[MOTOR_XY_0]);
	    if (_RobotStatus.motor[MOTOR_XY_1].moveIdDone!=_MoveId[MOTOR_XY_1])	
		    TrPrintfL(TRUE, "Motor[MOTOR_XY_1].moveIdDone=%d (%d)", _RobotStatus.motor[MOTOR_XY_1].moveIdDone, _MoveId[MOTOR_XY_1]);
    
	    SRobotMotorsMoveCmd cmd;
	    memset(&cmd, 0, sizeof(cmd));
	    cmd.header.msgLen	 = sizeof(cmd);
	    cmd.header.msgId	 = CMD_MOTORS_MOVE;

	    cmd.motors			|= 1<<MOTOR_XY_0;
	    cmd.targetPos[MOTOR_XY_0] = _RobotStatus.motor[MOTOR_XY_0].encPos+dx;
	    cmd.moveId [MOTOR_XY_0] = ++_MoveId[MOTOR_XY_0];

	    cmd.motors			|= 1<<MOTOR_XY_1;
	    cmd.targetPos[MOTOR_XY_1] = _RobotStatus.motor[MOTOR_XY_1].encPos+dx;
	    cmd.moveId [MOTOR_XY_1] = ++_MoveId[MOTOR_XY_1];
	    if (stop>=0)
	    {
		    cmd.stopBits[MOTOR_XY_0]     = cmd.stopBits[MOTOR_XY_1]     = 1<<stop;
		    cmd.stopBitLevel[MOTOR_XY_0] = cmd.stopBitLevel[MOTOR_XY_1] = STOP_ON_HIGH;
	    }
	    else 
	    {
		    cmd.encoderCheck[MOTOR_XY_0] = cmd.encoderCheck[MOTOR_XY_1] = ENC_CHECK_ENC;
		    cmd.encoderTol[MOTOR_XY_0]   = cmd.encoderTol[MOTOR_XY_1]   = ENCODER_TOL_XY;
	    }
        sok_send(&_RC_Socket, &cmd);
	    
        TrPrintfL(TRUE, "sent CMD_MOTORS_MOVE_XY (%d, %d) moveIdStarted=%d %d", cmd.targetPos[MOTOR_XY_0], cmd.targetPos[MOTOR_XY_1], _MoveId[MOTOR_XY_0], _MoveId[MOTOR_XY_1]);
    }
	return REPLY_OK;
}

//--- _rc_moveto_y_stop -----------------------------------
static int _rc_moveto_y_stop(int y, int stop, const char *file, int line)
{
	if (_AppRunning)
	{
	    for (int motor=MOTOR_XY_0; motor<=MOTOR_XY_1; motor++)
	    {
		    if (_RobotStatus.motor[motor].isMoving)  return Error(LOG_TYPE_ERROR_CONT, file, line, 0, "_rc_moveto_y_stop: Motor[%d].isMoving", motor);
		    if (_RobotStatus.motor[motor].isStalled) return Error(LOG_TYPE_ERROR_CONT, file, line, 0, "_rc_moveto_y_stop: Motor[%d].isStalled", motor);
	    }
	    
	    TrPrintfL(TRUE, "CMD_MOTORS_MOVE_Y (x=%d, y=%d) new (y=%d)", RX_StepperStatus.screw_posX, RX_StepperStatus.screw_posY, y);
    
	    int dy = y - _micron_2_steps(RX_StepperStatus.screw_posY);
    
	    if (_RobotStatus.motor[MOTOR_XY_0].moveIdDone!=_MoveId[MOTOR_XY_0])	
		    TrPrintfL(TRUE, "Motor[MOTOR_XY_0].moveIdDone=%d (%d)", _RobotStatus.motor[MOTOR_XY_0].moveIdDone, _MoveId[MOTOR_XY_0]);
	    if (_RobotStatus.motor[MOTOR_XY_1].moveIdDone!=_MoveId[MOTOR_XY_1])	
		    TrPrintfL(TRUE, "Motor[MOTOR_XY_1].moveIdDone=%d (%d)", _RobotStatus.motor[MOTOR_XY_1].moveIdDone, _MoveId[MOTOR_XY_1]);
    
	    SRobotMotorsMoveCmd cmd;
	    memset(&cmd, 0, sizeof(cmd));
	    cmd.header.msgLen	 = sizeof(cmd);
	    cmd.header.msgId	 = CMD_MOTORS_MOVE;
	    cmd.motors			|= 1<<MOTOR_XY_0;
	    cmd.targetPos[MOTOR_XY_0] = _RobotStatus.motor[MOTOR_XY_0].encPos-dy;
	    cmd.moveId   [MOTOR_XY_0] = ++_MoveId[MOTOR_XY_0];

	    cmd.motors			|= 1<<MOTOR_XY_1;
	    cmd.targetPos[MOTOR_XY_1] = _RobotStatus.motor[MOTOR_XY_1].encPos+dy;
	    cmd.moveId   [MOTOR_XY_1] = ++_MoveId[MOTOR_XY_1];
	    if (stop>=0)
	    {
		    cmd.stopBits[MOTOR_XY_0]     = cmd.stopBits[MOTOR_XY_1]     = 1<<stop;
		    cmd.stopBitLevel[MOTOR_XY_0] = cmd.stopBitLevel[MOTOR_XY_1] = STOP_ON_HIGH;
	    }
	    else 
	    {
            cmd.encoderCheck[MOTOR_XY_0] = cmd.encoderCheck[MOTOR_XY_1] = ENC_CHECK_ENC;
		    cmd.encoderTol[MOTOR_XY_0]   = cmd.encoderTol[MOTOR_XY_1]   = ENCODER_TOL_XY;
	    }
        sok_send(&_RC_Socket, &cmd);
	    
        TrPrintfL(TRUE, "sent CMD_MOTORS_MOVE_XY (%d, %d) moveIdStarted=%d %d", cmd.targetPos[MOTOR_XY_0], cmd.targetPos[MOTOR_XY_1], _MoveId[MOTOR_XY_0], _MoveId[MOTOR_XY_1]);
    }
	return REPLY_OK;
}

//--- _rc_move_xy_done -------------------------------------------
static int	_rc_move_xy_done(void)
{
	return _RobotStatus.motor[MOTOR_XY_0].moveIdDone==_MoveId[MOTOR_XY_0] &&  _RobotStatus.motor[MOTOR_XY_1].moveIdDone==_MoveId[MOTOR_XY_1];
}

//--- _rc_move_screw_done --------------------------------------------------------
static int _rc_move_screw_done(void)
{
    return _RobotStatus.motor[MOTOR_SCREW].moveIdDone == _MoveId[MOTOR_SCREW];
}

//--- rc_move_top --------------------------------------
int  rc_move_top(const char *file, int line)
{
    _rc_reset_motors(1<<MOTOR_Z);
    return _rc_motor_moveToStop(MOTOR_Z, -MAX_LENGTH_Z, IN_Z_UP, STOP_ON_HIGH, _FL_);	
}

//--- rc_move_bottom ---------------------------------------
int  rc_move_bottom(const char *file, int line)
{
	TrPrintfL(TRUE, "rc_move_bottom");
	_rc_reset_motors(1<<MOTOR_Z);
	return _rc_motor_moveToStop(MOTOR_Z, MAX_LENGTH_Z*2, IN_Z_DOWN, STOP_ON_HIGH, _FL_);	
}

//--- rc_find_screw --------------------------------
int rc_find_screw(int steps)
{
	_rc_set_screwer_speed(0);
	rc_set_screwer_current(0);

	_rc_reset_motors(1 << MOTOR_Z);
	_rc_motor_moveToStop(MOTOR_Z, -MAX_LENGTH_Z, IN_Z_UP, STOP_ON_HIGH, _FL_);
	_rc_motor_moveBy(MOTOR_SCREW, steps*SCREW_STEPS, -1, _FL_);

	return REPLY_OK;
}

//--- _rob_state_machine --------------------------------------------------------------------
static void _rob_state_machine(void)
{
    static SScrewPos _pos;
    static SPoint    _default;
    static SPoint    _corr;
    static int       _corrMax_X;
    static int       _posy_base;
    static int       _posy;
    static int _time=0;
    int trace;
    int pos, distance = 0;
    int _turns = _ScrewPos.printbar[_ScrewPar.printbar].head[_ScrewPar.head][_ScrewPar.axis].turns;
            
    int t=rx_get_ticks();
    if (t>_time)
    {
        trace=TRUE;
        _time=t+1000;
    }

    if (!RX_StepperStatus.info.moving && !RX_StepperStatus.robinfo.moving && !RX_StepperStatus.screwerinfo.moving)
    {
        switch (_RobStateMachine_Step)
        {
        case 0: if (!RX_StepperStatus.robinfo.ref_done)
                    lbrob_reference_slide();
                _RobStateMachine_Step++;
                // no break

        case 1: // new screw
            if (!RX_StepperStatus.robinfo.ref_done) break;
            if (RX_StepperCfg.headsPerColor>2) 
            {
                RX_StepperCfg.headsPerColor=2;
                Error(WARN, 0, "TEST headsPerColor=%d", RX_StepperCfg.headsPerColor);
            }
            TrPrintfL(TRUE, "_rob_state_machine: State=%d", _RobStateMachine_Step);
            _posy_base = (SLIDE_SCREW_POS_BACK + ((_ScrewPar.head+1) * (SLIDE_SCREW_POS_FRONT - SLIDE_SCREW_POS_BACK)) / 8);
            TrPrintfL(TRUE, "_rob_state_machine: State=%d head=%d, axis=%d, _posy_base=%d, posy=%d", _RobStateMachine_Step, _ScrewPar.head, _ScrewPar.axis, _posy_base, _pos.y);            
            memset(&_pos, 0x00, sizeof(_pos));
            if (_ScrewPar.printbar<2)
            {
                int defaults = (_ScrewFunction==CMD_FIND_ALL_SCREWS);
                defaults = FALSE;
                if (_ScrewPar.head==-1)
                {
                    memcpy(&_pos, &_ScrewPos.printbar[_ScrewPar.printbar].stitch, sizeof(_pos));
                    if (_ScrewPar.axis==AXE_STITCH)
                    {
						if (_ScrewPar.printbar == 0)                    _default.x = SCREW_X_LEFT;
						else if (_ScrewPos.printbar[0].stitch.x == 0)   _default.x = SCREW_X_RIGHT;
                        else                                            _default.x = SCREW_X_RIGHT + _ScrewPos.printbar[0].stitch.x - SCREW_X_LEFT;

                        if (_ScrewPar.printbar == 0 || _ScrewPos.printbar[0].stitch.y < MIN_Y_POS)  _default.y = _posy_base-SCREW_Y_STITCH;
                        else                                                                        _default.y = _ScrewPos.printbar[0].stitch.y;

                        _corrMax_X = 3*CORR_MAX;
                    }
                }
                else 
                {                
                    memcpy(&_pos, &_ScrewPos.printbar[_ScrewPar.printbar].head[_ScrewPar.head][_ScrewPar.axis], sizeof(_pos));
                    if (_ScrewPar.axis==AXE_ANGLE)
                    {
                        if (_ScrewPar.head==0) _default.x = _ScrewPos.printbar[_ScrewPar.printbar].stitch.x;
                        else                   _default.x = _ScrewPos.printbar[_ScrewPar.printbar].head[_ScrewPar.head-1][AXE_ANGLE].x;
                    }
                    else _default.x = _ScrewPos.printbar[_ScrewPar.printbar].head[_ScrewPar.head][AXE_ANGLE].x;

                    if (_ScrewPar.axis==AXE_ANGLE)
                    {
						if ((_ScrewPar.head == 0 && _ScrewPos.printbar[_ScrewPar.printbar].stitch.y < MIN_Y_POS) 
							    || (_ScrewPos.printbar[_ScrewPar.printbar].head[_ScrewPar.head - 1][AXE_ANGLE].y < MIN_Y_POS && _ScrewPar.head != 0))
							_default.y = _posy_base-SCREW_Y_ANGLE;
						else if (_ScrewPar.head==0)                                                             
							_default.y = _ScrewPos.printbar[_ScrewPar.printbar].stitch.y + SCREW_Y_STITCH - SCREW_Y_ANGLE;
                        else                                            
							_default.y = _ScrewPos.printbar[_ScrewPar.printbar].head[_ScrewPar.head-1][AXE_ANGLE].y;
                    }
                    else _default.y = _ScrewPos.printbar[_ScrewPar.printbar].head[_ScrewPar.head][AXE_ANGLE].y + SCREW_Y_ANGLE - SCREW_Y_STITCH;
                    _corrMax_X = CORR_MAX;
                }

                if ( abs(_pos.x - _default.x)>(_corrMax_X+3*CORR_STEP)) _pos.x = _default.x;
                if ( abs(_pos.y - _default.y)>(CORR_MAX+3*CORR_STEP)) _pos.y = _default.y;
                
                if (FALSE)
                {
                    Error(WARN, 0, "TEST");
                    _pos.x = _default.x;
                    _pos.y = _default.y;
                }
            }

            if (_pos.x)
            {
            //    if (abs(_pos.x-RX_StepperStatus.screw_posX)>5000)
            //        rx_stop();
                
                _RobStateMachine_Step++;
            }
            else
            {
                Error(ERR_CONT, 0, "Screw (printbar=%d, head=%d, axis=%d) does not exist", _ScrewPar.printbar, _ScrewPar.head, _ScrewPar.axis);
                return;
            }
            if (_ScrewFunction == CMD_HEAD_ADJUST)
            {
                // adjustment range check 
                int end=_turns+_ScrewPar.steps;
                Error(LOG, 0, "Screw (printbar=%d, head=%d, axis=%d) from %d.%d to %d.%d turns", _ScrewPar.printbar, _ScrewPar.head, _ScrewPar.axis, _turns/6, abs(_turns)%6, end/6, abs(end)%6);
                if (_turns+_ScrewPar.steps>0)
                {
                    _ScrewPar.steps = -_turns;
                    Error(WARN, 0, "Screw (printbar=%d, head=%d, axis=%d) adjustment limited to %d.%d turns", _ScrewPar.printbar, _ScrewPar.head, _ScrewPar.axis, _ScrewPar.steps/6, abs(_ScrewPar.steps)%6);
                }
                else if (_ScrewPar.axis==AXE_ANGLE && end>MAX_STEPS_ANGLE-OVERTURN)
                {
                    _ScrewPar.steps = MAX_STEPS_ANGLE-OVERTURN-_turns;
                    Error(WARN, 0, "Screw (printbar=%d, head=%d, axis=%d) adjustment limited to %d.%d turns", _ScrewPar.printbar, _ScrewPar.head, _ScrewPar.axis, _ScrewPar.steps/6, abs(_ScrewPar.steps)%6);
                }
                else if (_ScrewPar.axis==AXE_STITCH && end>MAX_STEPS_STITCH-OVERTURN)
                {
                    _ScrewPar.steps = MAX_STEPS_STITCH-OVERTURN-_turns;
                    Error(WARN, 0, "Screw (printbar=%d, head=%d, axis=%d) adjustment limited to %d.%d turns", _ScrewPar.printbar, _ScrewPar.head, _ScrewPar.axis, _ScrewPar.steps/6, abs(_ScrewPar.steps)%6);
                }
            }
            if (RX_StepperStatus.posY[0]-1000 < _pos.y)
            {
                lbrob_cln_move_to(_pos.y);
                break;
            }
            // no break here!

       case 2:  // move slide 
       //   if (!rc_move_done() || _CmdRunning) break;
            if (!rc_move_done()) break;
            _corr.x = _corr.y = 0;
            TrPrintfL(TRUE, "_rob_state_machine: State=%d", _RobStateMachine_Step);
            TrPrintfL(TRUE, "Move to (pb=%d, h=%d, a=%d) x=%d, y=%d, slide=%d", _ScrewPar.printbar, _ScrewPar.head, _ScrewPar.axis, _pos.x, _pos.y, _pos.y+ROB_POSY_FREE);
            lbrob_cln_move_to(_pos.y+ROB_POSY_FREE);
            if (_ScrewTurned) rc_screwer_to_ref();
            _ScrewTurned = FALSE;
            _RobStateMachine_Step++;
            // no break here!

        case 3: // move rob to y (move robi and slide simultanious)
            TrPrintfL(TRUE, "_rob_state_machine: State=%d, head=%d/%d", _RobStateMachine_Step, _ScrewPar.printbar, _ScrewPar.head);
            if (rc_move_xy_error())
            {
                Error(ERR_CONT, 0, "Motor stalled");
                _RobStateMachine_Step=0;
            }
            else if (RX_StepperStatus.robinfo.ref_done && (RX_StepperStatus.info.z_in_screw || RX_StepperStatus.info.z_in_ref || RX_StepperStatus.info.z_in_wash || RX_StepperStatus.info.z_in_cap) && RX_StepperStatus.info.ref_done)
            {
                _posy = ROB_POSY_FREE;
                TrPrintfL(TRUE, "CMD_ROBI_MOVE_TO_Y(%d)", _pos.y);
                Error(LOG, 0, "Robot Move to %s", _pos_str(_pos.x, _pos.y));
                rc_moveto_y(_posy, _FL_);
                _RobStateMachine_Step++;
            }
            break;

        case 4: // move rob to x
            if (!rc_move_done()) break;
            TrPrintfL(trace, "_rob_state_machine: State=%d, screw_posY=%d, _posy=%d, diff=%d", _RobStateMachine_Step, RX_StepperStatus.screw_posY, _posy, abs(RX_StepperStatus.screw_posY-_posy));
            if (abs(RX_StepperStatus.screw_posY-_posy)>1000) break;

            TrPrintfL(trace, "_rob_state_machine: State=%d, moveDone=%d, head=%d/%d-%d", _RobStateMachine_Step, rc_move_done(), _ScrewPar.printbar, _ScrewPar.head, _ScrewPar.axis);
            if (rc_move_xy_error())
            {
                Error(ERR_CONT, 0, "Motor stalled");
                _RobStateMachine_Step=0;
            }
            else if (rc_move_done())
            {
                TrPrintfL(TRUE, "_rob_state_machine: State=%d, head=%d/%d-%d", _RobStateMachine_Step, _ScrewPar.printbar, _ScrewPar.head, _ScrewPar.axis);
                TrPrintfL(TRUE, "CMD_ROBI_MOVE_TO_X(%d), y=%d", _pos.x, RX_StepperStatus.screw_posY);
                rc_moveto_x(_pos.x, _FL_);
                _RobStateMachine_Step++;
            }
            break;

        case 5: 
            if (!rc_move_done()) break;
            TrPrintfL(TRUE, "_rob_state_machine: State=%d, head=%d/%d", _RobStateMachine_Step, _ScrewPar.printbar, _ScrewPar.head);
            if (rc_move_xy_error())
            {
                Error(ERR_CONT, 0, "Motor stalled");
                _RobStateMachine_Step=0;
            }
            else
            {
                _RobStateMachine_Step++;
                _rob_state_machine();
            }
            break;

        case 6:
            TrPrintfL(trace, "_rob_state_machine: State=%d, movedone=%d", _RobStateMachine_Step, rc_move_done());
            if (rc_move_done())
            {
                if (rc_move_xy_error())
                {
                    TrPrintfL(TRUE, "_rob_state_machine: State=%d, Motor stalled", _RobStateMachine_Step);
                    Error(ERR_CONT, 0, "Motor stalled");
                    _RobStateMachine_Step=0;
                }
                else
                {
                    TrPrintfL(TRUE, "_rob_state_machine: State=%d", _RobStateMachine_Step);
                    if (!RX_StepperStatus.info.z_in_screw) lb702_handle_ctrl_msg(INVALID_SOCKET, CMD_LIFT_SCREW, NULL, _FL_);
                    _RobStateMachine_Step++;
                }
            }
            break;

        case 7:
            if (RX_StepperStatus.info.z_in_screw)
            {
                TrPrintfL(TRUE, "_rob_state_machine: State=%d", _RobStateMachine_Step);
                TrPrintfL(TRUE, "CMD_ROBI_MOVE_Z_UP X=%d, Y=%d", RX_StepperStatus.posY[0], RX_StepperStatus.screw_posX, RX_StepperStatus.posY[0]-RX_StepperStatus.screw_posY);
                rc_move_top(_FL_);
                _RobStateMachine_Step++;
            }
            break;

        case 8:
            if (RX_StepperStatus.screwerinfo.z_in_up)
            {
                TrPrintfL(TRUE, "_rob_state_machine: State=%d, zpos=%d, z_in_up=%d", _RobStateMachine_Step, RX_StepperStatus.motor[3].motor_pos, RX_StepperStatus.screwerinfo.z_in_up);
                
                _RobStateMachine_Step=13;
            }
            else
            {
              //  TrPrintfL(TRUE, "z_in_up=%d, screw_posY=%d, turn screwer, movedone=%d", RX_StepperStatus.screwerinfo.z_in_up, RX_StepperStatus.screw_posY, rc_move_done());
                if (rc_move_done())
                {
                    TrPrintfL(TRUE, "_rob_state_machine: State=%d, zpos=%d, z_in_up=%d", _RobStateMachine_Step, RX_StepperStatus.motor[3].motor_pos, RX_StepperStatus.screwerinfo.z_in_up);
                    TrPrintfL(TRUE, "Turn screw");
                    rc_find_screw(1);
                    _RobStateMachine_Step++;
                }
            }
            break;
      
        case 9:
            if (rc_move_done())
            {
                rc_find_screw(-1);
                _RobStateMachine_Step++;
            }
            break;

        case 10:
            if (rc_move_done())
            {
                _ScrewTurned = TRUE;
                if (RX_StepperStatus.screwerinfo.z_in_up)
                {
                    TrPrintfL(TRUE, "_rob_state_machine: State=%d, z_in_up", _RobStateMachine_Step);
                    _RobStateMachine_Step=12;
                }
                else
                {                
                    TrPrintfL(TRUE, "_rob_state_machine: State=%d, move done", _RobStateMachine_Step);             
                    rc_move_bottom(_FL_);
                    _RobStateMachine_Step++;
                }
            }
            break;

        case 11:
            if (RX_StepperStatus.screwerinfo.z_in_down)
            {
                TrPrintfL(TRUE, "_rob_state_machine: State=%d, screwer move done, posz=%d", _RobStateMachine_Step, RX_StepperStatus.motor[3].motor_pos);
                if (_ScrewFunction==CMD_FIND_ALL_SCREWS || _ScrewFunction==CMD_RESET_ALL_SCREWS)
                {
                    if (_ScrewPar.head<0) ErrorEx(dev_head, _ScrewPar.printbar*RX_StepperCfg.headsPerColor, LOG, 0, "PrintBar %s: screw not found %s, trying again",  _AxisName[_ScrewPar.axis], _pos_str(RX_StepperStatus.screw_posX, RX_StepperStatus.posY[0]-RX_StepperStatus.screw_posY));
                    else                  ErrorEx(dev_head, _ScrewPar.printbar*RX_StepperCfg.headsPerColor, LOG, 0, "Head %d %s: screw not found %s, trying again", _ScrewPar.head+1, _AxisName[_ScrewPar.axis], _pos_str(RX_StepperStatus.screw_posX , RX_StepperStatus.posY[0]-RX_StepperStatus.screw_posY));
                }

                TrPrintfL(TRUE, "z_in_up=%d, posy=%d, screw_posY=%d, Y=%d, X=%d", RX_StepperStatus.screwerinfo.z_in_up, RX_StepperStatus.posY[0], RX_StepperStatus.screw_posY, RX_StepperStatus.posY[0]+RX_StepperStatus.screw_posY, RX_StepperStatus.screw_posX);
                
                if (_corr.y>0) _corr.y = -_corr.y;
                else           _corr.y = (-_corr.y)+CORR_STEP;

                if (abs(_corr.y)>CORR_MAX && _ScrewPar.head<0 && abs(_corr.x)<=_corrMax_X)
                {
                    if (_corr.x>0) _corr.x = -_corr.x;
                    else           _corr.x = (-_corr.x)+CORR_STEP;
                    _corr.y=0;
                }

                if (abs(_corr.y)>CORR_MAX)
                {
                    rc_move_bottom(_FL_);
                    Error(ERR_CONT, 0, "Screw (printbar %d, head %d, axis %d) not found", _ScrewPar.printbar, _ScrewPar.head, _ScrewPar.axis);
                    _RobStateMachine_Step = 0;
                    RX_StepperStatus.adjustDoneCnt++;
                    break;
                }
                rc_screwer_to_ref();

                TrPrintfL(TRUE, "_rob_state_machine: State=%d head=%d, axis=%d, _posy_base=%d, posy=%d", _RobStateMachine_Step, _ScrewPar.head, _ScrewPar.axis, _posy_base, _pos.y+_corr.y);            
                TrPrintfL(TRUE, "CMD_ROB_MOVE_POS(%d)", pos+ROB_POSY_FREE);
                Error(LOG, 0, "Robot Move to %s", _corr_pos_str(_pos, _corr));
                rc_moveto_y(ROB_POSY_FREE, _FL_);
                lbrob_cln_move_to(_pos.y+_corr.y+ROB_POSY_FREE);

                _RobStateMachine_Step++;
            }
            break;

        case 12:
          //  if (rc_move_done())
          //   if (!_CmdRunning)
			if (rc_move_done())
            {
               TrPrintfL(TRUE, "_rob_state_machine: State=%d, Y move done", _RobStateMachine_Step);
               rc_moveto_x(_pos.x+_corr.x, _FL_);
               rc_move_top(_FL_);
               _RobStateMachine_Step=8;
            }
            break;

        case 13:    // screw found 
            TrPrintfL(TRUE, "_rob_state_machine: State=%d", _RobStateMachine_Step);
            pos = RX_StepperStatus.posY[0]-RX_StepperStatus.screw_posY;
            if (_ScrewPar.head<0)
            {
                TrPrintfL(TRUE, "screw found: (pb=%d, h=%d, a=%d) posy=%d, _posy_base=%d, slidepos=%d, screw_posY_old=%d, screw_posY_new=%d", _ScrewPar.printbar, _ScrewPar.head, _ScrewPar.axis, pos, _posy_base, RX_StepperStatus.posY[0], _ScrewPos.printbar[_ScrewPar.printbar].stitch.y, pos);
                if (TRUE || _ScrewFunction==CMD_FIND_ALL_SCREWS) ErrorEx(dev_head, _ScrewPar.printbar*RX_StepperCfg.headsPerColor, LOG, 0, "PrintBar %s: screw found %s",  _AxisName[_ScrewPar.axis], _pos_str(RX_StepperStatus.screw_posX, pos));
                _ScrewPos.printbar[_ScrewPar.printbar].stitch.x = RX_StepperStatus.screw_posX;
                _ScrewPos.printbar[_ScrewPar.printbar].stitch.y = pos;
            }
            else
            {
                TrPrintfL(TRUE, "screw found: (pb=%d, h=%d, a=%d) posy=%d, _posy_base=%d, slidepos=%d, screw_posY_old=%d, screw_posY_new=%d", _ScrewPar.printbar, _ScrewPar.head, _ScrewPar.axis, pos, _posy_base, RX_StepperStatus.posY[0], RX_StepperStatus.screw_posY, pos);
                if (TRUE || _ScrewFunction==CMD_FIND_ALL_SCREWS) ErrorEx(dev_head, _ScrewPar.printbar*RX_StepperCfg.headsPerColor, LOG, 0, "Head %d %s: screw found %s", _ScrewPar.head+1, _AxisName[_ScrewPar.axis], _pos_str(RX_StepperStatus.screw_posX, pos));
                _ScrewPos.printbar[_ScrewPar.printbar].head[_ScrewPar.head][_ScrewPar.axis].x = RX_StepperStatus.screw_posX;
                _ScrewPos.printbar[_ScrewPar.printbar].head[_ScrewPar.head][_ScrewPar.axis].y = pos;
            }
            //--- 
            switch(_ScrewFunction)
            {
            case CMD_RESET_ALL_SCREWS:  _RobStateMachine_Step=100; break;
            case CMD_HEAD_ADJUST:       _RobStateMachine_Step=200; break;
            case CMD_FIND_ALL_SCREWS:   rc_move_bottom(_FL_);
                                        _RobStateMachine_Step=900; break;
            }
            break;
        
        //--- CMD_RESET_ALL_SCREWS --------------------------
        case 100:
            rx_enable_stall_error(FALSE);
            TrPrintfL(TRUE, "_rob_state_machine: State=%d", _RobStateMachine_Step);
            _ScrewTurned = TRUE;
            // turn right to mechanical stop
            if (_ScrewPar.axis==AXE_ANGLE) pos = MAX_STEPS_ANGLE  + 2*OVERTURN;
            else                           pos = MAX_STEPS_STITCH + 2*OVERTURN;
            TrPrintfL(TRUE, "Motor[2]: (PB=%d H=%d, A=%d) rc_turn_ticks_left(%d)", _ScrewPar.printbar, _ScrewPar.head, _ScrewPar.axis, pos);
            rc_turn_ticks_left(pos);
            _RobStateMachine_Step++;
            break;

        case 101:
            TrPrintfL(TRUE, "_rob_state_machine: State=%d, screwerPos=%d, steps=%d", _RobStateMachine_Step, rc_get_screwer_pos(), rc_get_screwer_steps(STEPS_PER_TURN));

            // turn left to middle + 2 turns
            if (_ScrewPar.axis==AXE_ANGLE) pos = MAX_STEPS_ANGLE/2  + OVERTURN + LOOSE_TURN;
            else                           pos = MAX_STEPS_STITCH/2 + OVERTURN + LOOSE_TURN;
            TrPrintfL(TRUE, "Motor[2]: (PB=%d H=%d, A=%d) rc_turn_ticks_right(%d)", _ScrewPar.printbar, _ScrewPar.head, _ScrewPar.axis, pos);
            rc_turn_ticks_right(pos);
            _RobStateMachine_Step++;
            break;

        case 102:
            TrPrintfL(TRUE, "_rob_state_machine: State=%d, screwerPos=%d, steps=%d", _RobStateMachine_Step, rc_get_screwer_pos(), rc_get_screwer_steps(STEPS_PER_TURN));

            _ScrewerStalled = rc_screwer_stalled();
            // turn right 2 turns
            pos = OVERTURN;
            TrPrintfL(TRUE, "Motor[2]: (PB=%d H=%d, A=%d) rc_turn_ticks_left(%d)", _ScrewPar.printbar, _ScrewPar.head, _ScrewPar.axis, pos);
            rc_turn_ticks_left(pos);
            _RobStateMachine_Step++;
            break;

        case 103:
            TrPrintfL(TRUE, "_rob_state_machine: State=%d, screwerPos=%d", _RobStateMachine_Step, rc_get_screwer_pos());

            if (_ScrewerStalled)
            {
                if (_ScrewPar.head<0) ErrorEx(dev_head, _ScrewPar.printbar*RX_StepperCfg.headsPerColor, ERR_CONT, 0, "Centering PrintBar %s: Screwer  Blocked",  _AxisName[_ScrewPar.axis]);
                else                  ErrorEx(dev_head, _ScrewPar.printbar*RX_StepperCfg.headsPerColor, ERR_CONT, 0, "Centering Head %d %s: Screwer Blocked", _ScrewPar.head+1, _AxisName[_ScrewPar.axis]);
            }
            else
            {
                if (_ScrewPar.head<0) ErrorEx(dev_head, _ScrewPar.printbar*RX_StepperCfg.headsPerColor, LOG, 0, "Centering PrintBar %s: done", _AxisName[_ScrewPar.axis]);
                else                  ErrorEx(dev_head, _ScrewPar.printbar*RX_StepperCfg.headsPerColor, LOG, 0, "Centering Head %d %s: done", _ScrewPar.head+1, _AxisName[_ScrewPar.axis]);
            }
            TrPrintfL(TRUE, "Turn right 2 turns, steps=%d", rc_get_screwer_steps(STEPS_PER_TURN));

            // save
            if (_ScrewPar.head<0) _ScrewPos.printbar[_ScrewPar.printbar].stitch.turns = -MAX_STEPS_STITCH/2;
            else if (_ScrewPar.axis==AXE_ANGLE) _ScrewPos.printbar[_ScrewPar.printbar].head[_ScrewPar.head][AXE_ANGLE].turns = -MAX_STEPS_ANGLE/2;
            else _ScrewPos.printbar[_ScrewPar.printbar].head[_ScrewPar.head][AXE_STITCH].turns = -MAX_STEPS_STITCH/2;
            
            rc_move_bottom(_FL_);
            _RobStateMachine_Step=900;
            break;

        //--- CMD_HEAD_ADJUST ------------------------------------------
        case 200:
            TrPrintfL(TRUE, "_rob_state_machine: State=%d", _RobStateMachine_Step);
            RX_StepperStatus.screwerinfo.screwed_OK = FALSE;
            _ScrewTurned = TRUE;
            if (_ScrewPar.steps < 0)
            {
                pos = abs(_ScrewPar.steps - 2*6);
                rc_turn_ticks_right(pos);
            }
            _RobStateMachine_Step++;
            break;

        case 201:
            TrPrintfL(TRUE, "_rob_state_machine: State=%d, blockedLeft=%d, blockedRight=%d", _RobStateMachine_Step, RX_StepperStatus.screwerinfo.screwer_blocked_left, RX_StepperStatus.screwerinfo.screwer_blocked_right);
            if (_ScrewPar.steps < 0) pos = 2*6;
            else                     pos = _ScrewPar.steps;
            rc_turn_ticks_left(pos);
            _RobStateMachine_Step++;
            break;
            
        case 202:
            TrPrintfL(TRUE, "_rob_state_machine: State=%d, blockedLeft=%d, blockedRight=%d", _RobStateMachine_Step, RX_StepperStatus.screwerinfo.screwer_blocked_left, RX_StepperStatus.screwerinfo.screwer_blocked_right);

            RX_StepperStatus.screwerinfo.screwed_OK = !RX_StepperStatus.screwerinfo.screwer_blocked_left && !RX_StepperStatus.screwerinfo.screwer_blocked_right;
            if (_ScrewPar.head<0 && RX_StepperStatus.screwerinfo.screwer_blocked_left)
                _ScrewPos.printbar[_ScrewPar.printbar].stitch.turns = 0;
            else if (_ScrewPar.head<0 && RX_StepperStatus.screwerinfo.screwer_blocked_right)
                _ScrewPos.printbar[_ScrewPar.printbar].stitch.turns = MAX_STEPS_STITCH;
            else if (_ScrewPar.head<0) _ScrewPos.printbar[_ScrewPar.printbar].stitch.turns += _ScrewPar.steps;
            else if (RX_StepperStatus.screwerinfo.screwer_blocked_left)
                _ScrewPos.printbar[_ScrewPar.printbar].head[_ScrewPar.head][_ScrewPar.axis].turns = 0;
            else if (RX_StepperStatus.screwerinfo.screwer_blocked_right && _ScrewPar.axis == AXE_ANGLE)
                _ScrewPos.printbar[_ScrewPar.printbar].head[_ScrewPar.head][AXE_ANGLE].turns = -MAX_STEPS_ANGLE;
            else if (RX_StepperStatus.screwerinfo.screwer_blocked_left && _ScrewPar.axis == AXE_STITCH)
                _ScrewPos.printbar[_ScrewPar.printbar].head[_ScrewPar.head][AXE_STITCH].turns = -MAX_STEPS_STITCH;
            else if (_ScrewPar.axis==AXE_ANGLE) _ScrewPos.printbar[_ScrewPar.printbar].head[_ScrewPar.head][AXE_ANGLE].turns += _ScrewPar.steps;
            else _ScrewPos.printbar[_ScrewPar.printbar].head[_ScrewPar.head][AXE_STITCH].turns += _ScrewPar.steps;
        //  _save_ScrewPos(_ScrewDist);
            _save_ScrewPos();
            
            if (!RX_StepperStatus.screwerinfo.screwer_blocked_left && !RX_StepperStatus.screwerinfo.screwer_blocked_right)
            {
                _RobStateMachine_Step++;
            }
            else
            {
                if (RX_StepperStatus.screwerinfo.screwer_blocked_left)
                {
                    pos = LOOSE_TURN;
                    rc_turn_ticks_right(pos);
                }
                else
                {
                    pos = OVERTURN + LOOSE_TURN;
                    rc_turn_ticks_left(pos);
                }
                _RobStateMachine_Step++;    
            }
            break;
          
        case 203:
            TrPrintfL(TRUE, "_rob_state_machine: State=%d", _RobStateMachine_Step);
            rc_screwer_to_ref();
             _RobStateMachine_Step++;
            break;

        case 204:
            if (!rc_move_done()) break;
            TrPrintfL(TRUE, "_rob_state_machine: State=%d", _RobStateMachine_Step);
            rc_move_bottom(_FL_);
             _RobStateMachine_Step++;
            break;

        case 205:
            TrPrintfL(TRUE, "_rob_state_machine: State=%d", _RobStateMachine_Step);
            _RobStateMachine_Step = 0; // wait for next command
            RX_StepperStatus.adjustDoneCnt++;
            break;

        //--------------- next screw ---------------
        case 900:
            TrPrintfL(TRUE, "_rob_state_machine: State=%d", _RobStateMachine_Step);
            _save_ScrewPos();
            _RobStateMachine_Step++;
            break;

        case 901:
            TrPrintfL(TRUE, "_rob_state_machine: State=%d", _RobStateMachine_Step);
            TrPrintfL(TRUE, "Act Screw: printbar=%d, head=%d, axis=%d", _ScrewPar.printbar, _ScrewPar.head, _ScrewPar.axis);
            if (_ScrewPar.axis<AXE_MAX) _ScrewPar.axis++;
            else
            {
                _ScrewPar.axis = 0;
                _ScrewPar.head++;
            }
            if (_ScrewPar.head==RX_StepperCfg.headsPerColor-1 && _ScrewPar.axis==AXE_STITCH) 
            {
                // stitch of last head not used, goto stitch of next printbar instead
                _ScrewPar.axis   = AXE_STITCH;
                _ScrewPar.head = -1;
                _ScrewPar.printbar++;
            }

            if (RX_StepperCfg.printbarUsed & (1<<_ScrewPar.printbar))
            {
                _RobStateMachine_Step = 0;
                _rob_state_machine();
            }
            else
            {
                _ScrewPar.printbar = 0;
                _RobStateMachine_Step = 1000;
                _GarageWithoutAdjust = FALSE;
                switch(_ScrewFunction)
                {
                case CMD_RESET_ALL_SCREWS:  Error(LOG, 0, "CMD_RESET_ALL_SCREWS done"); break;
                case CMD_HEAD_ADJUST:       Error(LOG, 0, "CMD_HEAD_ADJUST done");      break;
                case CMD_FIND_ALL_SCREWS:   Error(LOG, 0, "CMD_FIND_ALL_SCREWS done");  break;
                }
            }
            TrPrintfL(TRUE, "New Screw: printbar=%d, head=%d, axis=%d", _ScrewPar.printbar, _ScrewPar.head, _ScrewPar.axis);
            break;

        //---------------- move to garage ------------------------------------------------
        case 1000: 
            if (rc_move_done())
            {
                TrPrintfL(TRUE, "_rob_state_machine: State=%d", _RobStateMachine_Step);
                if (RX_StepperStatus.screwerinfo.z_in_down)
                {
                    TrPrintfL(TRUE, "motorpos: x=%d, y=%d", RX_StepperStatus.screw_posX, RX_StepperStatus.screw_posY);
                    TrPrintfL(TRUE, "CMD_ROBI_MOVE_TO_Y(%d)", ROB_POSY_FREE);
                    rc_moveto_y(ROB_POSY_FREE, _FL_);
                    _RobStateMachine_Step++;
                }
                else 
                {
                    rc_reference(0);
                    _RobStateMachine_Step=0;
                }
            }
            break;

        case 1001: 
            if (rc_move_done())
            {
                TrPrintfL(TRUE, "_rob_state_machine: State=%d", _RobStateMachine_Step);
                lbrob_move_to_pos(0, _micron_2_steps(3000), FALSE);
                TrPrintfL(TRUE, "CMD_ROBI_MOVE_TO_X(%d)", 0);
                rc_moveto_x(0, _FL_);
                _RobStateMachine_Step++;
            }
            break;

        case 1002: // move to garage ---
            if (rc_move_done())
            {
                TrPrintfL(TRUE, "_rob_state_machine: State=%d", _RobStateMachine_Step);
                rc_moveto_y(-1000, _FL_);
                _RobStateMachine_Step++;
            }
            break;
        
        case 1003:
            if (rc_move_done())
            {
                TrPrintfL(TRUE, "_rob_state_machine: State=%d", _RobStateMachine_Step);
                _RobStateMachine_Step=0;
                if (!_GarageWithoutAdjust)
                RX_StepperStatus.adjustDoneCnt++;
                _GarageWithoutAdjust = FALSE;
            }
            break;
        }
    }
    if (_RobStateMachine_Step==0) rx_enable_stall_error(TRUE);
}

//--- rc_to_garage --------------------------------
void rc_to_garage(void)
{
    TrPrintfL(TRUE, "lbrob_to_garage: _RobStateMachine_Step=%d, rc_in_garage=%d, screwerinfo.ref_done=%d", _RobStateMachine_Step, rc_in_garage(), RX_StepperStatus.screwerinfo.ref_done);
    
    if (_RobStateMachine_Step==0)
    {
        if (!rc_in_garage())
        {
            if (!RX_StepperStatus.screwerinfo.ref_done)
            {
                rc_reference(0);
            }
            else
            {
                _GarageWithoutAdjust = TRUE;
                _RobStateMachine_Step = 1000;
                _rob_state_machine();
            }
        }
    }
}

//--- _set_ScrewPos ---------------------------------------
static void _set_ScrewPos(SScrewPositions *pos)
{
    if (RX_StepperCfg.headsPerColor>ROB_HEADS_PER_BAR) 
        Error(ERR_ABORT, 0, "headsPerColor: Array overflow!");
    else memcpy(&_ScrewPos, pos, sizeof(_ScrewPos));
}

//--- _save_ScrewPos ------------------------------------------------
static void _save_ScrewPos()
{
    ctrl_send_2(REP_SET_SCREW_POS, sizeof(_ScrewPos), &_ScrewPos);
}

//--- rc_display_status -----------------------------------
void rc_display_status(void)
{
	int i;
	
	if (!_RobotUsed)
	{
		term_printf("Robot not used\n");
		return;
	}

	term_printf("Robot #%d --- v %s --- socket=%d ----- StatusReq=%d/%d ----- alive=%d ----- connected=%d ----- \n", _RobotStatus.serialNo, _RobotStatus.version, _RC_Socket, _StatusReqCnt, _StatusRecvCnt, _RobotStatus.alive, _Connected);
	
	// Connection information
//	term_printf("\nConnection Status: %d\n", _isConnected);
	
	if (_BinaryFile) 
	{
		term_printf("Downloading: v=%d.%d.%d.%d %d/%d\n", _FileVersion.major, _FileVersion.minor, _FileVersion.revision, _FileVersion.build, _FilePos, _FileSize);
	}
	else if (_Connected)
	{
		// GPIO information
		term_printf("Inputs:    ");
		for (i = 0; i < INPUT_CNT; i++)
		{
			if ((i % 4) == 0) term_printf(" ");
			if (_RobotStatus.gpio.inputs & (1<<i)) term_printf("*");
			else	                               term_printf("_");
		}

		term_printf("       IN: screw[%d]=%c down[%d]=%c up[%d]=%c y[%d]=%c garage[%d]=%c x[%d]=%c", 
			IN_SCREW_EDGE, _level(ROB_IN(IN_SCREW_EDGE)),
			IN_Z_DOWN,	_level(ROB_IN(IN_Z_DOWN)), 
			IN_Z_UP,	_level(ROB_IN(IN_Z_UP)), 
			IN_Y_END, _level(ROB_IN(IN_Y_END)), 
			IN_GARAGE, _level(ROB_IN(IN_GARAGE)), 
			IN_X_END, _level(ROB_IN(IN_X_END)) 
			);
		term_printf("\n");

		term_printf("Outputs:   ");
		for (i = 0; i < OUTPUT_CNT; i++)
		{
			if ((i % 4) == 0) term_printf(" ");
			if (_RobotStatus.gpio.outputs & (1<<i)) term_printf("*");
			else	                                term_printf("_");
		}
		term_printf("\n");
	
		term_printf("Edge Count: ");
		for (i = 0; i <INPUT_CNT; i++)
		{
			term_printf("%04d ", _RobotStatus.gpio.inputEdges[i]);
		}
		term_printf("\n");

		// Motor information
		term_printf("Motors\tPos\t\tTarget Pos\tEnc Pos\t\tmoveId\tdone\tmoving\tstalled\tconfigured\n");
		for (i=0; i<MOTOR_CNT; i++)
		{
			term_printf("%d-", i);
			switch(i)
			{
			case MOTOR_XY_0:	term_printf("XY0"); break;
			case MOTOR_XY_1:	term_printf("XY1"); break;
			case MOTOR_Z:		term_printf("Z"); break;
			case MOTOR_SCREW:	term_printf("Screw"); break;
			}
			term_printf("\t%010d\t%010d\t%010d\t  %d\t%d\t  %1d\t  %d\t  %d\n", 
			_RobotStatus.motor[i].motorPos, 
			_RobotStatus.motor[i].targetPos, 
			_RobotStatus.motor[i].encPos, 

			_RobotStatus.motor[i].moveIdStarted,
			_RobotStatus.motor[i].moveIdDone,
			_RobotStatus.motor[i].isMoving,
			_RobotStatus.motor[i].isStalled, 
			_RobotStatus.motor[i].isConfigured);
		}
	
		term_printf("\n");
        term_printf("State:\t\t\t %d \tprintbar=%d head=%d screw=%s\n", _RobStateMachine_Step, _ScrewPar.printbar, _ScrewPar.head, _AxisName[_ScrewPar.axis]);
		term_printf("State Machine: \t\t %d\t%s\n", _RC_State, _RC_Error);
		term_printf("Screwer X-Pos: \t\t %d\n", RX_StepperStatus.screw_posX);
		term_printf("Screwer Y-Pos: \t\t %d\n", RX_StepperStatus.screw_posY);
        term_printf("Screwer Current: \t %d\n", rc_get_screwer_current());
    }	
	term_printf("\n");
	term_flush();
}

//--- rc_handle_ctrl_msg -----------------------------------
int rc_handle_ctrl_msg(RX_SOCKET socket, int msgId, void *pdata)
{
	int val;

	switch (msgId)
	{
    case CMD_SET_SCREW_POS:
        _set_ScrewPos((SScrewPositions *)pdata);
        break;

    case CMD_HEAD_ADJUST:
        _ScrewFunction = CMD_HEAD_ADJUST;
        memcpy(&_ScrewPar, pdata, sizeof(_ScrewPar));
        Error(LOG, TRUE, "lbrob_handle_ctrl_msg CMD_HEAD_ADJUST: PrintBar=%d, head=%d, axis=%d, steps=%d", _ScrewPar.printbar, _ScrewPar.head, _ScrewPar.axis, _ScrewPar.steps);
        if (RX_StepperStatus.robinfo.ref_done) _rob_state_machine();
        else Error(ERR_CONT, 0, "Robot reference not done");
        break;

    case CMD_FIND_ALL_SCREWS:
        Error(WARN, 0, "CMD_FIND_ALL_SCREWS started");
        ctrl_send_2(CMD_GET_SCREW_POS, 0, NULL);
        for (_ScrewPar.printbar=0; _ScrewPar.printbar<2; _ScrewPar.printbar++)
        {
            if (RX_StepperCfg.printbarUsed & (1<<_ScrewPar.printbar))
            {
                _ScrewFunction = CMD_FIND_ALL_SCREWS;
                _ScrewPar.head=-1;
                _ScrewPar.axis=AXE_STITCH;
                _ScrewPar.steps=0;
                _rob_state_machine();
                return REPLY_OK;
            }
        }
        Error(ERR_CONT, 0, "No Prinbar used on this Stepperboard");
        break;
        
    case CMD_RESET_ALL_SCREWS:
        val = (*(INT32 *)pdata);
        Error(WARN, 0, "CMD_RESET_ALL_SCREWS Started");
        ctrl_send_2(CMD_GET_SCREW_POS, 0, NULL);
        _ScrewFunction = CMD_RESET_ALL_SCREWS;
        if (!(RX_StepperCfg.printbarUsed & (1 << 0)) && !(RX_StepperCfg.printbarUsed & (1 << 1)))
            Error(ERR_CONT, 0, "No Prinbar used on this Stepperboard");
        else if (val == 1 && !(RX_StepperCfg.printbarUsed & (1 << 1)))
            Error(ERR_CONT, 0, "This Prinbar is not used on this Stepperboard");
        if (val == 0 && !(RX_StepperCfg.printbarUsed & (1 << 0)) && RX_StepperCfg.printbarUsed & (1 << 1))
            _ScrewPar.printbar = 1;
        else
            _ScrewPar.printbar=val;
        _ScrewPar.head=-1;
        _ScrewPar.axis=AXE_STITCH;
        _ScrewPar.steps=0;
        _rob_state_machine();
        break;

    case CMD_ROBI_MOVE_TO_GARAGE:
        rc_to_garage();
        break;

    default:
        lb702_handle_ctrl_msg(socket, msgId, pdata, _FL_);
	}
}

//--- rc_menu ----------------------------------------------
void rc_menu(int help)
{
	if (help)
	{
		term_printf("Robot MENU ------------------------- (3)\n");
		term_printf("s: STOP\n");
		term_printf("o<num><value>: set output <num> to value <value>\n");
		term_printf("R: Reference\n");
		term_printf("r<n>: reset Motor<n>\n");
		term_printf("m<n><steps>: move Motor<n> by <steps>\n");
		term_printf("x<steps>: move X-Position by <steps>\n");
		term_printf("y<steps>: move Y-Position by <steps>\n");
		term_printf("z<steps>: move Z-Position by <steps>\n");
		term_printf("u: move Z to up\n");
		term_printf("d: move Z to down\n");
		term_printf("t: turn screw\n");
        term_printf("F: Find all the screws\n");
        term_printf("Z <printbar>: Move all Screws to default position\n");

//		term_printf("v<micron>: Move x axis <micron>um\n");
//		term_printf("w<micron>: Move y axis <micron>um\n");
//		term_printf("t<steps>: Turn screw <steps>\n");
//		term_printf("l<ticks>: Turn screw left <ticks>\n");
//		term_printf("r<ticks>: Turn screw right <ticks>\n");
//		term_printf("u: Z up\n");
//		term_printf("d: Z down\n");
//      term_printf("e<micron>: Move to x pos <micron>\n");
//      term_printf("f<micron>: Move to y pos <micron>\n");
//      term_printf("g: move to garage position\n");
        term_flush();
	}
	else
	{
		term_printf("?: help\n");
		term_printf("1: LB702 menu\n");
		term_printf("2: LBROB menu\n");
	}
}

//--- rc_handle_menu ----------------------------------------
void rc_handle_menu(char *str)
{
	int no;
	SHeadAdjustment adjust;

    if (no=str_start(str, "serialno"))
		_setSerialNo(atoi(&str[no]));
	else
	{
		no = str[1]-'0';
		switch (str[0])
		{
		case 'r':	_rc_reset_motors(MOTORS_ALL);									break;
		case 's':	rc_stop();														break;
		case 'R':	rc_reference(0);												break;
		case 'm':	_rc_motor_moveBy(no, atoi(&str[2]),0,  _FL_);					break;
        case 'x':	rc_moveto_x(atoi(&str[1]), _FL_);								break;
        case 'y':	rc_moveto_y(atoi(&str[1]), _FL_);								break;
		case 'z':	_rc_motor_moveBy(MOTOR_Z, atoi(&str[1]), 0, _FL_);				break;		
		case 'd':	rc_move_bottom(_FL_);											break;
		case 'u':	rc_move_top(_FL_);												break;
		case 't':	_rc_motor_moveBy(MOTOR_SCREW, atoi(&str[1]), 0, _FL_);			break;

		case 'F':   rc_handle_ctrl_msg(INVALID_SOCKET, CMD_FIND_ALL_SCREWS, NULL);	break;
		case 'Z':	rc_handle_ctrl_msg(INVALID_SOCKET, CMD_RESET_ALL_SCREWS, &no);	break;
        case 'a':	adjust.printbar = 0;
					adjust.head     = 0;
					adjust.axis     = 0;
					adjust.steps    = 0;
					rc_handle_ctrl_msg(INVALID_SOCKET, CMD_HEAD_ADJUST, &adjust);
					break;

		default:	break;
		}
	}
}

//--- rc_in_garage ----------------------------------------------
int rc_in_garage(void)
{
    return RX_StepperStatus.screwerinfo.in_garage;
}

//--- rc_screwer_to_ref ------------------------------
void rc_screwer_to_ref(void)
{
	TrPrintfL(TRUE, "rc_screwer_to_ref");
	_rc_set_screwer_speed(TRUE);
	_rc_motor_moveBy(MOTOR_SCREW, 2*SCREW_STEPS, IN_SCREW_EDGE, _FL_);
}

//--- rc_turn_ticks_left ------------------------------------------------
void rc_turn_ticks_left(int ticks)
{
    _rc_set_screwer_speed(TRUE);
    _rc_motor_moveBy(MOTOR_SCREW, - ticks * SCREW_STEPS, -1, _FL_);
}

//--- rc_turn_ticks_right ------------------------------------------------
void rc_turn_ticks_right(int ticks)
{
    _rc_set_screwer_speed(TRUE);
    _rc_motor_moveBy(MOTOR_SCREW, ticks * SCREW_STEPS, -1, _FL_);
}

/*
//--- rc_turn_ticks ------------------------------------------------
void rc_turn_ticks(int ticks, int stopOnUp)
{
    _rc_set_screwer_speed(FALSE);
	if (stopOnUp) _rc_motor_moveBy(MOTOR_SCREW, ticks*SCREW_STEPS/6, IN_Z_UP, _FL_);
    else          _rc_motor_moveBy(MOTOR_SCREW, ticks*SCREW_STEPS/6, 0, _FL_);
}
*/

//--- rc_turn_steps ------------------------------------------------
void rc_turn_steps(int steps)
{
    _rc_set_screwer_speed(FALSE);
    _rc_motor_moveBy(MOTOR_SCREW, steps, 0, _FL_);
}

//--- rc_set_screwer_current ---------------------------------------------
void rc_set_screwer_current(int high)
{
	if (high!=rc_get_screwer_current())
	{
		if (high) _configure_screw_motor(SCREW_CURRENT_HIGH, _MotorCfg[MOTOR_SCREW].vmax);
		else	  _configure_screw_motor(SCREW_CURRENT_LOW, _MotorCfg[MOTOR_SCREW].vmax);
	}
}

//--- rc_get_screwer_current --------------------------------------------------
int rc_get_screwer_current(void)
{
    return (_MotorCfg[MOTOR_SCREW].iholdirun == SCREW_CURRENT_HIGH);
}

//--- _rc_set_screwer_speed -----------------------------------------------
void _rc_set_screwer_speed(int high)
{
	int h=_MotorCfg[MOTOR_SCREW].vmax == SCREW_SPEED_HIGH;
	if (high!=h)
	{
		if (high)	_configure_screw_motor(_MotorCfg[MOTOR_SCREW].iholdirun, SCREW_SPEED_HIGH);
		else		_configure_screw_motor(_MotorCfg[MOTOR_SCREW].iholdirun, SCREW_SPEED_LOW);
	}
}

//--- rc_get_screwer_pos ------------------------------------------
int rc_get_screwer_pos(void)
{
    return _RobotStatus.motor[MOTOR_SCREW].motorPos;
}

int rc_get_screwer_steps(int steps_per_turn)
{
    return (_RobotStatus.motor[MOTOR_SCREW].motorPos+steps_per_turn/2)/steps_per_turn;
}

//--- rc_screwer_stalled -------------------------------------------------
int rc_screwer_stalled(void)
{
    return _RobotStatus.motor[MOTOR_SCREW].isStalled;
}

//--- rc_move_started ----------------------------------------------------------
int rc_move_started(void)
{
    for (int i = 0; i < MOTOR_CNT; i++)
    {
        if (_RobotStatus.motor[i].moveIdStarted != _RobotStatus.motor[i].moveIdDone) return TRUE;
    }
    return FALSE;
}

//--- rc_move_done --------------------------------------
int rc_move_done(void)
{
    if (_RC_State) return FALSE;
    for (int i = 0; i < MOTOR_CNT; i++)
    {
        if (_RobotStatus.motor[i].moveIdDone != _MoveId[i]) return FALSE;
    }
    return TRUE;
}

//--- rc_move_screw_done ---
int rc_move_screw_done(void)
{
    if (_RC_State) return FALSE;
	return (_RobotStatus.motor[MOTOR_SCREW].moveIdDone == _MoveId[MOTOR_SCREW]);
}

//--- rc_move_xy_error ------------------------------
int rc_move_xy_error(void)
{	
	for (int motor=0; motor<MOTOR_CNT; motor++)
    {
        if ((MOTORS_XY & (1<<motor)) && (_RobotStatus.motor[motor].moveIdDone==_MoveId[motor]) && _RobotStatus.motor[motor].isStalled)
		{
			RX_StepperStatus.screwerinfo.ref_done = FALSE;
		//	RX_StepperStatus.screwerinfo.ref_x_done = FALSE;
		//	RX_StepperStatus.screwerinfo.ref_y_done = FALSE;
			return TRUE;
		}
    }
	return FALSE;
}

//--- _motor_name ------------------------------
static char *_motor_name(int no)
{
	switch (no)
	{
	case MOTOR_XY_1:	return "XY-1";
	case MOTOR_XY_0:	return "XY-0";
	case MOTOR_SCREW:	return "Screwer";
	case MOTOR_Z:		return "Z";
	default:			return "";
	}
}