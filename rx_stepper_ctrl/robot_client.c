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

#include "rx_common.h"
#include "rx_error.h"
#include "rx_sok.h"
#include "rx_threads.h"
#include "../rx_robot_ctrl/includes/rx_robot_def.h"
#include "../rx_robot_ctrl/includes/rx_robot_tcpip.h"
#include "rx_term.h"
#include "rx_trace.h"
#include "version.h"
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

#define ROB_IN(in) (_RobotStatus.gpio.inputs & 1<<(in))

// Outputs
#define OUTPUT_Z_UP		1
#define OUTPUT_Z_DOWN	0


#define STEPS_PER_REV				51200
#define DISTANCE_UM_PER_REV			36000   // mirco meters per revolutionR
#define MOTOR_X_GARAGE_POS			21000 //28500

#define SCREW_STEPS					213333
#define SCREW_SPEED					220000			// steps/sec

#define MAX_LENGTH_Z				5500000//550000

#define ENCODER_TOL					250
#define ENCODER_TOL_XY				1000
#define ENCODER_TOL_SCREW			220000

#define SCREW_CURRENT_HIGH			(0b00000000000000010001110000000000)
#define SCREW_CURRENT_LOW			(0b00000000000000010001001100000000)
#define Z_CURRENT_UP				(0b00000000000000010000101000000000)
#define Z_CURRENT_DOWN				(0b00000000000000010000111000000000)

#define LOW		0
#define HIGH	1

//--- Modlue Globals -----------------------------------------------------------------
static char _IpAddr[32]="";
RX_SOCKET	_RC_Socket = INVALID_SOCKET;
static int	_Connected = FALSE;
static int	_Timeout;

HANDLE					_RobotHdl;
static SRobotStatusMsg	_RobotStatus;
static int				_StatusReqCnt;
static int				_MotorInit;
static SMotorConfig		_MotorCfg[MOTOR_CNT];
static UINT8			_MoveId[MOTOR_CNT];

static SVersion			_BinVersion;
static SVersion			_FileVersion;
static FILE				*_BinaryFile;
static UINT32			_FilePos;
static UINT32			_FileSize;

static int				_RC_State;
static char				_RC_Error[MAX_PATH];

#define RC_STATE_REF	1000

//--- Prototypes ------------------------------------------------------
static void* _robot_ctrl_thread(void* lpParameter);
static int _handle_robot_ctrl_msg(RX_SOCKET socket, void *msg, int len, struct sockaddr *sender, void *ppar);
static int _connection_closed(RX_SOCKET socket, const char *peerName);

static int  _config_motors(void);

static void _configure_xy_motor(uint8_t motor);
static void _configure_screw_motor(int current);
static void _configure_z_motor(void);
static void _send_MotorCfg(int motor);

static void _rc_error(const char *file, int line, char *msg);
static void _rc_reset_motors(int motors);
static void _rc_reset_edgeCtr(int inputs);
static int  _rc_motor_moveBy(int motor, int steps, const char *file, int line);
static int  _rc_motor_moveToStop(int motor, int steps, int input, int level, const char *file, int line);
static int  _rc_moveto_x_stop(int x, int stop, const char *file, int line);
static int  _rc_moveto_y_stop(int y, int stop, const char *file, int line);
static int	_rc_move_xy_done(void);
static int _rc_move_screw_done(void);

static void _rc_state_machine(void);
static void _check_version(void);
static void _check_move(void);

static int _steps_2_micron(int steps);
static int _micron_2_steps(int micron);


static void _download_start(void);
static void _download_data(SBootloaderDataRequestCmd *req);
static void _download_end(void);
static void _setSerialNo(UINT16 no);

//--- rc_init ----------------------------------------------------
void rc_init(void)
{
	_MotorInit = 0;
	_RobotHdl  = rx_thread_start(_robot_ctrl_thread, NULL, 0, "RC_Client");
}

//--- rc_config ------------------------------------------------
void rc_config(int boardNo)
{
	sprintf(_IpAddr, RX_CTRL_SUBNET "%d", 50+10*boardNo);
}

//--- rc_isConnected -----------------------------------------
int	rc_isConnected(void)
{
	return (_RC_Socket!=INVALID_SOCKET);
}


//--- rc_main -----------------------------------------------
void rc_main(int ticks, int menu)
{
    if (_RC_Socket!=INVALID_SOCKET)
	{
		if (menu)
		{
		}
        RX_StepperStatus.screwerinfo.z_in_down = ROB_IN(IN_Z_DOWN) ? TRUE : FALSE;
        RX_StepperStatus.screwerinfo.z_in_up = ROB_IN(IN_Z_UP) ? TRUE : FALSE;

    }
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
			errNo=sok_open_client_2(&_RC_Socket, _IpAddr, PORT_UDP_COMMUNICATION, SOCK_DGRAM, _handle_robot_ctrl_msg, _connection_closed);
			if (errNo == REPLY_OK)
			{
				Error(LOG, 0, "Robot Connected");
				_config_motors();
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
            case MOTOR_SCREW:	_configure_screw_motor(SCREW_CURRENT_LOW);	break;
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
	_Timeout	= 2;

	switch(phdr->msgId)
	{
    case CMD_STATUS_GET:	memcpy(&_RobotStatus, msg, sizeof(_RobotStatus));
						//	TrPrintfL(TRUE, "got CMD_STATUS_GET moveIdStarted=%d, moveIdDone=%d, isMoving=%d, isStalled=%d", 
						//		_RobotStatus.motor[MOTOR_Z].moveIdStarted, _RobotStatus.motor[MOTOR_Z].moveIdDone, _RobotStatus.motor[MOTOR_Z].isMoving, _RobotStatus.motor[MOTOR_Z].isStalled);
							_check_move();
							_check_version();
							_rc_state_machine();
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
void rc_reference(void)
{
	TrPrintfL(TRUE, "rc_reference");
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
				TrPrintfL(TRUE, "ROBOT: Motor[%d].MoveId=%d, STALLED: motorPos=%d, encPos=%d, diff=%d", motor, 
					stat->moveIdDone, stat->motorPos, stat->encPos, stat->motorPos-stat->encPos);
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
	if (strcmp(_RobotStatus.version, _versionStr))
	{
		TrPrintf(TRUE, "new Version %s", _RobotStatus.version);
		memcpy(_versionStr, _RobotStatus.version, sizeof(_versionStr));
		//---  get file version ---------------
		sscanf(_versionStr, "%lu.%lu.%lu.%lu", &_BinVersion.major, &_BinVersion.minor, &_BinVersion.revision, &_BinVersion.build);
		if (_get_file_version(PATH_BIN_STEPPER FILENAME_ROBOT_CTRL, &_FileVersion)==REPLY_OK)	
		{
			int test=memcmp(&_BinVersion, &_FileVersion, sizeof(SVersion));
			if (memcmp(&_BinVersion, &_FileVersion, sizeof(SVersion)))
				_download_start();
		}
	}
}

//--- _download_start ------------------------------------
static void _download_start(void)
{
	TrPrintfL(TRUE, "_download_start");
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
	SBootloaderSerialNoCmd cmd;
	cmd.header.msgId = CMD_BOOTLOADER_SERIALNO;
	cmd.header.msgLen= sizeof(cmd);
	cmd.serialNo     = no;
	sok_send(&_RC_Socket, &cmd);
}

//--- _rc_state_machine --------------------------------
static void _rc_state_machine(void)
{
	if (!_config_motors()) return;

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
								break;

		case RC_STATE_REF+1:	// z at bottom
								TrPrintfL(TRUE, "RC_STATE_REF+1: down=%d, moving=%d, start=%d, done=%d", ROB_IN(IN_Z_DOWN), _RobotStatus.motor[MOTOR_Z].isMoving, _RobotStatus.motor[MOTOR_Z].moveIdStarted, _RobotStatus.motor[MOTOR_Z].moveIdDone);
								if (ROB_IN(IN_Z_DOWN))
								{
									_rc_motor_moveToStop(MOTOR_SCREW, 300000, IN_SCREW_EDGE, LOW, _FL_);
									_rc_moveto_y_stop(1000000, IN_Y_END, _FL_);
									_RC_State++;
								}
								else if (_RobotStatus.motor[MOTOR_Z].moveIdDone == _MoveId[MOTOR_Z]) 
								{
									_rc_error(_FL_, "Z-Axis down sensor not found");
								}
								break;

		case RC_STATE_REF+2:	// Y in end sensor (out)
								if (_rc_move_xy_done())  
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
										_RC_State++;
									}
									else _rc_error(_FL_, "Y-Axis end sensor not found");
								}													
								break;

		case RC_STATE_REF+3:	if (_rc_move_xy_done() && _rc_move_screw_done())  
								{
									TrPrintfL(TRUE, "RC_STATE_REF+3: x-end=%d, start=%d %d, done=%d %d", ROB_IN(IN_X_END), 
										_RobotStatus.motor[MOTOR_XY_0].moveIdStarted, _RobotStatus.motor[MOTOR_XY_1].moveIdStarted, 
										_RobotStatus.motor[MOTOR_XY_0].moveIdDone, _RobotStatus.motor[MOTOR_XY_1].moveIdDone);
									_rc_motor_moveToStop(MOTOR_SCREW, 300000, IN_SCREW_EDGE, HIGH, _FL_);
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
										rx_sleep(50);
										_rc_reset_motors(MOTORS_XY);
										rx_sleep(50);
										rc_moveto_x(MOTOR_X_GARAGE_POS, _FL_);
										_RC_State++;
									}
									else _rc_error(_FL_, "X-Axis end sensor not found");
								}
								break;

		case RC_STATE_REF+5:	// X in garage-pos
								if (_rc_move_xy_done())  
								{
									TrPrintfL(TRUE, "RC_STATE_REF+5: y-end=%d, start=%d %d, done=%d %d", ROB_IN(IN_Y_END), 
										_RobotStatus.motor[MOTOR_XY_0].moveIdStarted, _RobotStatus.motor[MOTOR_XY_1].moveIdStarted, 
										_RobotStatus.motor[MOTOR_XY_0].moveIdDone, _RobotStatus.motor[MOTOR_XY_1].moveIdDone);	
									if (_RobotStatus.motor[MOTOR_XY_0].isStalled || _RobotStatus.motor[MOTOR_XY_1].isStalled)
									{
										_rc_error(_FL_, "X-Axis blocked");
										_RC_State=RC_STATE_REF+3;
									}
									else 
									{
										_rc_moveto_y_stop(-200000, IN_GARAGE, _FL_);
										_RC_State++;
									}
								}
								break;

		case RC_STATE_REF+6:	// Y in garage sensor
								if (_rc_move_xy_done())  								
								{									
									TrPrintfL(TRUE, "RC_STATE_REF+6: y-end=%d, start=%d %d, done=%d %d", ROB_IN(IN_Y_END), 
										_RobotStatus.motor[MOTOR_XY_0].moveIdStarted, _RobotStatus.motor[MOTOR_XY_1].moveIdStarted, 
										_RobotStatus.motor[MOTOR_XY_0].moveIdDone, _RobotStatus.motor[MOTOR_XY_1].moveIdDone);	
									if (ROB_IN(IN_GARAGE))					
									{
										_rc_reset_motors(MOTORS_XY);
										_RC_State=0;
										Error(LOG, 0, "Reference done");
                                        RX_StepperStatus.screwerinfo.ref_done = TRUE;
                                    }
									else _rc_error(_FL_, "Y-Axis garage sensor not found");
								}
								break;		
		
		default:				Error(WARN, 0, "rc_state_machine: State %d not implemented", _RC_State);
								break;
	}
}

//--- _send_MotorCfg ----------------------------
static void _send_MotorCfg(int motor)
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

//--- configure_xy_motor -------------------------------------
static void _configure_xy_motor(uint8_t motor)
{
	_MotorCfg[motor].gconf = 0;
	_MotorCfg[motor].swmode = 0x03;
    _MotorCfg[motor].iholdirun = SCREW_CURRENT_LOW;
//	_MotorCfg[motor].iholdirun = SCREW_CURRENT_HIGH;
	_MotorCfg[motor].tpowerdown = 0;
	_MotorCfg[motor].tpwmthrs = 0;
	_MotorCfg[motor].rampmode = 0;
	_MotorCfg[motor].vmax = 26844;
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
static void _configure_screw_motor(int current)
{		
	_MotorCfg[MOTOR_SCREW].gconf = 0;
	_MotorCfg[MOTOR_SCREW].swmode = 0x03;
//	_MotorCfg[MOTOR_SCREW].iholdirun = 0x11300;
	_MotorCfg[MOTOR_SCREW].iholdirun = current;
	_MotorCfg[MOTOR_SCREW].tpowerdown = 0;
	_MotorCfg[MOTOR_SCREW].tpwmthrs = 0;
	_MotorCfg[MOTOR_SCREW].rampmode = 0;
	//_MotorCfg[MOTOR_SCREW].vmax = 178957;
    _MotorCfg[MOTOR_SCREW].vmax = SCREW_SPEED;
	_MotorCfg[MOTOR_SCREW].v1 = 0;
	_MotorCfg[MOTOR_SCREW].amax = 2932;
	_MotorCfg[MOTOR_SCREW].dmax = 2932;
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
//	_MotorCfg[MOTOR_Z].iholdirun = 0x10A00;
	_MotorCfg[MOTOR_Z].iholdirun = Z_CURRENT_DOWN;
	_MotorCfg[MOTOR_Z].tpowerdown = 0;
	_MotorCfg[MOTOR_Z].tpwmthrs = 0;
	_MotorCfg[MOTOR_Z].rampmode = 0;
	_MotorCfg[MOTOR_Z].vmax = 322122;
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

//--- rc_stop --------------------------------
void rc_stop(int motors)
{
	SRobotMotorsStopCmd cmd;
	cmd.header.msgLen = sizeof(cmd);
	cmd.header.msgId  = CMD_MOTORS_STOP;
	cmd.motors = motors;
	sok_send(&_RC_Socket, &cmd);
	TrPrintfL(TRUE, "sent CMD_MOTORS_STOP");
}

void rc_reset_motors(int motors)
{
    _rc_reset_motors(1 << motors);
}

//--- _rc_reset_motors --------------------------------
static void _rc_reset_motors(int motors)
{
	SRobotMotorsResetCmd cmd;
	cmd.header.msgLen = sizeof(cmd);
	cmd.header.msgId  = CMD_MOTORS_RESET;
	cmd.motors		  = motors;
	sok_send(&_RC_Socket, &cmd);
	TrPrintfL(TRUE, "sent CMD_MOTORS_RESET");
	for (int motor=0; motor<MOTOR_CNT; motor++)
		if (motors & (1<<motor)) _RobotStatus.motor[MOTOR_XY_0].isStalled = FALSE;
}

//--- _rc_reset_edgeCtr ----------------------------
static void _rc_reset_edgeCtr(int inputs)
{
	SGpioSetCmd cmd;
	cmd.header.msgLen = sizeof(cmd);
	cmd.header.msgId  = CMD_GPIO_IN_RESET;
	cmd.outputs		  = inputs;
	sok_send(&_RC_Socket, &cmd);
	TrPrintfL(TRUE, "sent CMD_GPIO_RESET");
}

//--- _rc_motor_moveBy --------------------------
static int _rc_motor_moveBy(int motor, int steps, const char *file, int line)
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
        rx_sleep(600);
    }
    /*if (motor == MOTOR_SCREW)
        cmd.targetPos[motor] = _RobotStatus.motor[motor].motorPos + steps;
    else*/
        cmd.targetPos[motor] = _RobotStatus.motor[motor].targetPos + steps;
	if (_MotorCfg[motor].encmode)
	{
        cmd.encoderCheck[motor] = ENC_CHECK_ENC;
		cmd.encoderTol[motor]= ENCODER_TOL;
	}
	if (motor==MOTOR_SCREW)
	{
		cmd.encoderCheck[motor] = ENC_CHECK_IN;
		cmd.edgeCheckIn[motor] = 0;
		cmd.encoderTol[motor]  = ENCODER_TOL_SCREW;		
	}
	
    sok_send(&_RC_Socket, &cmd);
	
	TrPrintfL(TRUE, "sent CMD_MOTORS_MOVE");

	return REPLY_OK;
}

//--- _rc_motor_moveToStop --------------------------------
static int _rc_motor_moveToStop(int motor, int steps, int stopInput, int level, const char *file, int line)
{
	if (_RobotStatus.motor[motor].isMoving)  
    {
            return Error(LOG_TYPE_ERROR_CONT, file, line, 0, "_rc_motor_moveToStop: Motor[%d].isMoving", motor);
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
	cmd.targetPos[motor]	 =  steps;
	cmd.stopBits[motor]		 = 1<<stopInput;
	cmd.stopBitLevels[motor] = level;
	cmd.moveId[motor]		 = ++_MoveId[motor];

	TrPrintfL(TRUE, "sent CMD_MOTORS_MOVE[%d].moveIdStarted=%d", motor, _MoveId[motor]);

	sok_send(&_RC_Socket, &cmd);

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
    return _rc_moveto_y_stop(y, -1, file, line);
}

//--- _rc_moveto_x_stop -----------------------------------
static int _rc_moveto_x_stop(int x, int stop, const char *file, int line)
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
	cmd.motors			|= 1<<MOTOR_XY_1;
	cmd.targetPos[MOTOR_XY_1] = _RobotStatus.motor[MOTOR_XY_1].encPos+dx;
	cmd.moveId [MOTOR_XY_0] = ++_MoveId[MOTOR_XY_0];
	cmd.moveId [MOTOR_XY_1] = ++_MoveId[MOTOR_XY_1];
	if (stop>=0)
	{
		cmd.stopBits[MOTOR_XY_0] = cmd.stopBits[MOTOR_XY_1] = 1<<stop;
		cmd.stopBitLevels[MOTOR_XY_0] = cmd.stopBitLevels[MOTOR_XY_1] = 1;
	}
	else 
	{
		cmd.encoderCheck[MOTOR_XY_0] = cmd.encoderCheck[MOTOR_XY_1] = ENC_CHECK_ENC;
		cmd.encoderTol[MOTOR_XY_0] = cmd.encoderTol[MOTOR_XY_1] = ENCODER_TOL_XY;
	}
    sok_send(&_RC_Socket, &cmd);
	
    TrPrintfL(TRUE, "sent CMD_MOTORS_MOVE_XY (%d, %d) moveIdStarted=%d %d", cmd.targetPos[MOTOR_XY_0], cmd.targetPos[MOTOR_XY_1], _MoveId[MOTOR_XY_0], _MoveId[MOTOR_XY_1]);

	return REPLY_OK;
}

//--- _rc_moveto_y_stop -----------------------------------
static int _rc_moveto_y_stop(int y, int stop, const char *file, int line)
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
	cmd.motors			|= 1<<MOTOR_XY_1;
	cmd.targetPos[MOTOR_XY_1] = _RobotStatus.motor[MOTOR_XY_1].encPos+dy;
	cmd.moveId [MOTOR_XY_0] = ++_MoveId[MOTOR_XY_0];
	cmd.moveId [MOTOR_XY_1] = ++_MoveId[MOTOR_XY_1];
	if (stop>=0)
	{
		cmd.stopBits[MOTOR_XY_0] = cmd.stopBits[MOTOR_XY_1] = 1<<stop;
		cmd.stopBitLevels[MOTOR_XY_0] = cmd.stopBitLevels[MOTOR_XY_1] = 1;
	}
	else 
	{
        cmd.encoderCheck[MOTOR_XY_0] = cmd.encoderCheck[MOTOR_XY_1] = ENC_CHECK_ENC;
		cmd.encoderTol[MOTOR_XY_0] = cmd.encoderTol[MOTOR_XY_1] = ENCODER_TOL_XY;
	}
    sok_send(&_RC_Socket, &cmd);
	
    TrPrintfL(TRUE, "sent CMD_MOTORS_MOVE_XY (%d, %d) moveIdStarted=%d %d", cmd.targetPos[MOTOR_XY_0], cmd.targetPos[MOTOR_XY_1], _MoveId[MOTOR_XY_0], _MoveId[MOTOR_XY_1]);

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
    return _rc_motor_moveToStop(MOTOR_Z, -MAX_LENGTH_Z, IN_Z_UP, HIGH, _FL_);	
}

//--- rc_move_bottom ---------------------------------------
int  rc_move_bottom(const char *file, int line)
{
	return _rc_motor_moveToStop(MOTOR_Z, MAX_LENGTH_Z, IN_Z_DOWN, HIGH, _FL_);	
}

static char _level(int l)
{
	if (l) return '*';
	return '_';
}
//--- rc_display_status -----------------------------------
void rc_display_status(void)
{
	int i;
	
	term_printf("Robot #%d --- v %s --- socket=%d ----- StatusReq=%d ----- alive=%d ----- connected=%d ----- \n", _RobotStatus.serialNo, _RobotStatus.version, _RC_Socket, _StatusReqCnt, _RobotStatus.alive, _Connected);
	
	// Connection information
//	term_printf("\nConnection Status: %d\n", _isConnected);
	
	if (_BinaryFile) 
	{
		term_printf("Downloading: v=%d.%d.%d.%d %d/%d\n", _FileVersion.major, _FileVersion.minor, _FileVersion.revision, _FileVersion.build, _FilePos, _FileSize);
	}
	else
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
		term_printf("Motors\tPos\t\tTarget Pos\tEnc Pos\t\tmoveId\tmoving\tstalled\tstarted\tdone\tconfigured\n");
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
			term_printf("\t%010d\t%010d\t%010d\t  %1d\t  %1d\t  %1d\t%d\t%d\t%d\n", 
			_RobotStatus.motor[i].motorPos, 
			_RobotStatus.motor[i].targetPos, 
			_RobotStatus.motor[i].encPos, 
			_MoveId[i],
			_RobotStatus.motor[i].isMoving, 
			_RobotStatus.motor[i].isStalled, 
			_RobotStatus.motor[i].moveIdStarted,
			_RobotStatus.motor[i].moveIdDone,
			_RobotStatus.motor[i].isConfigured);
		}
	
		term_printf("\n");
		term_printf("State Machine: \t\t %d\t%s\n", _RC_State, _RC_Error);
		term_printf("Screwer X-Pos: \t\t %d\n", RX_StepperStatus.screw_posX);
		term_printf("Screwer Y-Pos: \t\t %d\n", RX_StepperStatus.screw_posY);
        term_printf("Screwer Current: \t %d\n", rc_get_screw_current());
    }	
	term_printf("\n");
	term_flush();
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
    int val;
    if (no=str_start(str, "serialno"))
		_setSerialNo(atoi(&str[no]));
	else
	{
		no = str[1]-'0';
		switch (str[0])
		{
		case 'r':	_rc_reset_motors(MOTORS_ALL); break;
		case 's':	_RC_State = 0;
					rc_stop(0x0f); break;
		case 'R':	rc_reference(); break;
		case 'm':	_rc_motor_moveBy(no, atoi(&str[2]), _FL_);	break;
		case 'x':	rc_moveto_x(RX_StepperStatus.screw_posX+atoi(&str[1]), _FL_); break;
		case 'y':	rc_moveto_y(RX_StepperStatus.screw_posY+atoi(&str[1]), _FL_); break;
        case 'e':	rc_moveto_x(atoi(&str[1]), _FL_); break;
        case 'f':	rc_moveto_y(atoi(&str[1]), _FL_); break;
		case 'z':	_rc_motor_moveBy(MOTOR_Z, atoi(&str[1]), _FL_);		break;		
		case 'd':	rc_move_bottom(_FL_); break;
		case 'u':	rc_move_top(_FL_); break;
		case 't':	val = atoi(&str[1]);
					_rc_motor_moveBy(MOTOR_SCREW, val, _FL_); break;
		default:
			break;
		}
	}
}

//--- rc_in_ref ----------------------------------------------
int rc_in_ref(void)
{
    return _RobotStatus.gpio.inputs & (1UL << IN_GARAGE);
}

//--- rc_screwer_in_ref --------------------------------------
int rc_screwer_in_ref(void)
{
    return _RobotStatus.gpio.inputs & (1UL << IN_SCREW_EDGE);
}

//--- rc_turn_ticks_left ------------------------------------------------
void rc_turn_ticks_left(int ticks)
{
    _rc_motor_moveBy(MOTOR_SCREW, - ticks * SCREW_STEPS, _FL_);
}

//--- rc_turn_ticks_right ------------------------------------------------
void rc_turn_ticks_right(int ticks)
{
    _rc_motor_moveBy(MOTOR_SCREW, ticks * SCREW_STEPS, _FL_);
}

//--- rc_turn_steps ------------------------------------------------
void rc_turn_steps(int steps)
{
    _rc_motor_moveBy(MOTOR_SCREW, steps, _FL_);
}

//--- rc_set_screw_current ---------------------------------------------
void rc_set_screw_current(int high)
{
    if (high) _configure_screw_motor(SCREW_CURRENT_HIGH);
    else	  _configure_screw_motor(SCREW_CURRENT_LOW);
}

//--- rc_screwer_stalled -------------------------------------------------
int rc_screwer_stalled(void)
{
    return _RobotStatus.motor[MOTOR_SCREW].isStalled;
}

//--- rc_get_screw_current --------------------------------------------------
int rc_get_screw_current(void)
{
    return (_MotorCfg[MOTOR_SCREW].iholdirun == SCREW_CURRENT_HIGH);
}

//--- rc_move_started ----------------------------------------------------------
int rc_move_started(void)
{
    for (int i = 0; i < MOTOR_CNT; i++)
    {
        if (_RobotStatus.motor[i].moveIdStarted > _RobotStatus.motor[i].moveIdDone) return TRUE;
    }
    return FALSE;
}

//--- rc_move_done --------------------------------------
int rc_move_done(void)
{
    if (_RC_State) return FALSE;
    for (int i = 0; i < MOTOR_CNT; i++)
    {
        if (_RobotStatus.motor[i].moveIdDone < _MoveId[i]) return FALSE;
    }
    return TRUE;
}

//--- rc_move_xy_error ------------------------------
int rc_move_xy_error(void)
{	
	for (int motor=0; motor<MOTOR_CNT; motor++)
    {
        if ((MOTORS_XY & (1<<motor)) && (_RobotStatus.motor[motor].moveIdDone==_MoveId[motor]) && _RobotStatus.motor[motor].isStalled)
		{
			RX_StepperStatus.screwerinfo.ref_done = FALSE;
			return TRUE;
		}
    }
	return FALSE;
}