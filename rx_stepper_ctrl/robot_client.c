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
#include "rx_robot_def.h"
#include "rx_robot_tcpip.h"
#include "rx_term.h"
#include "rx_trace.h"
#include "version.h"
#include "robot_client.h"

//--- defines -----------------------------------------------------
// Inputs
#define ENDSTOP_SCREW_ROTATION	0
#define	ENDSTOP_Z_DOWN			1
#define ENDSTOP_Z_UP			2
#define ENDSTOP_Y_AXIS			3
#define ENDSTOP_GARAGE			4
#define ENDSTOP_X_AXIS			5

#define ROB_IN(in) (_RobotStatus.gpio.inputs & 1<<(in))

// Outputs
#define OUTPUT_Z_UP		1
#define OUTPUT_Z_DOWN	0


#define STEPS_PER_REV				51200
#define DISTANCE_UM_PER_REV			36000   // mirco meters per revolution
#define MOTOR_X_GARAGE_POS			28500

#define SCREW_CURRENT_HIGH			(0b00000000000000010001110000000000)
#define SCREW_CURRENT_LOW			(0b00000000000000010001001100000000)
#define Z_CURRENT_UP				(0b00000000000000010000101000000000)
#define Z_CURRENT_DOWN				(0b00000000000000010000111000000000)


//--- Modlue Globals -----------------------------------------------------------------
static char _IpAddr[32]="";
RX_SOCKET	_RC_Socket = INVALID_SOCKET;
HANDLE					_RobotHdl;
static SRobotStatusMsg	_RobotStatus;
static int				_MotorInit;
static SMotorConfig		_MotorCfg[MOTOR_COUNT];
static int				_MotorIsMoving[MOTOR_COUNT];

static int				_RC_State;

#define RC_STATE_REF	1000

//--- Prototypes ------------------------------------------------------
static void* _robot_ctrl_thread(void* lpParameter);
static int _handle_robot_ctrl_msg(RX_SOCKET socket, void *msg, int len, struct sockaddr *sender, void *ppar);
static int _connection_closed(RX_SOCKET socket, const char *peerName);

static int  _config_motors(void);

static void _configure_xy_motor(uint8_t motor);
static void _configure_screw_motor(void);
static void _configure_z_motor(void);
static void _send_MotorCfg(int motor);

static void _rc_stop(int motors);
static void _rc_reset(int motor);
static int  _rc_motor_moveBy(int motor, int steps, const char *file, int line);
static int  _rc_motor_moveToStop(int motor, int steps, int input, const char *file, int line);
static int  _rc_moveto_xy_stop(int x, int y, int stop, const char *file, int line);

static void _rc_state_machine(void);

static int _steps_2_micron(int steps);
static int _micron_2_steps(int micron);


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
//		if (_RC_State) _rc_state_machine();
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
			TrPrintfL(TRUE, "sent CMD_STATUS_GET");
		}
		else if (_IpAddr[0] && sok_ping(_IpAddr)==REPLY_OK)
		{
			errNo=sok_open_client_2(&_RC_Socket, _IpAddr, PORT_UDP_COMMUNICATION, SOCK_DGRAM, _handle_robot_ctrl_msg, _connection_closed);
			if (errNo == REPLY_OK)
			{
				Error(LOG, 0, "Robot Connected");
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
	for (motor=0; motor<MOTOR_COUNT; motor++)
	{
		if (!_RobotStatus.motors[motor].isConfigured) _MotorInit &= ~(1<<motor);
		if (!(_MotorInit & (1<<motor))) 
		{
			switch(motor)
			{
            case MOTOR_XY_0:	_configure_xy_motor(motor);	break;
            case MOTOR_XY_1:	_configure_xy_motor(motor);	break;
            case MOTOR_SCREW:	_configure_screw_motor();	break;
            case MOTOR_Z:		_configure_z_motor();		break;
			}
			_rc_reset(motor);
			_MotorInit |= (1<<motor);
			ok=FALSE;
		}
	}
	return ok;
}

//--- _handle_robot_ctrl_msg --------------------------------------------------------
static int _handle_robot_ctrl_msg(RX_SOCKET socket, void *msg, int len, struct sockaddr *sender, void *ppar)
{
	SMsgHdr			*phdr = (SMsgHdr*)msg;
	switch(phdr->msgId)
	{
    case CMD_STATUS_GET:	memcpy(&_RobotStatus, msg, sizeof(_RobotStatus));
							TrPrintfL(TRUE, "got CMD_STATUS_GET MoveStartCnt=%d, MoveDoneCnt=%d, isMoving=%d, isStalled=%d", 
								_RobotStatus.motors[MOTOR_Z].moveStartCnt, _RobotStatus.motors[MOTOR_Z].moveDoneCnt, _RobotStatus.motors[MOTOR_Z].isMoving, _RobotStatus.motors[MOTOR_Z].isStalled);
							_rc_state_machine();
							break;

	default:				Error(WARN, 0, "Got unknown messageId=0x%08x", phdr->msgId);
	}
	return REPLY_OK;
}

//---- rc_reference -----------------------
void rc_reference(void)
{
	int motor;
	for (motor=0; motor<MOTOR_COUNT; motor++)
	{
		_rc_stop(motor);
	}
	_MotorInit = 0;
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

//--- _rc_state_machine --------------------------------
static void _rc_state_machine(void)
{
	if (!_config_motors()) return;

    RX_StepperStatus.screw_posX = (_steps_2_micron(_RobotStatus.motors[MOTOR_XY_1].motorEncPos + _RobotStatus.motors[MOTOR_XY_0].motorEncPos))/2;
    RX_StepperStatus.screw_posY = (_steps_2_micron(_RobotStatus.motors[MOTOR_XY_1].motorEncPos - _RobotStatus.motors[MOTOR_XY_0].motorEncPos))/2;
	RX_StepperStatus.screw_posZ = (_RobotStatus.motors[MOTOR_Z].motorPos);

	switch(_RC_State)
	{        
		case 0:					break;
	
		case RC_STATE_REF:		// move Z to bottom
								if (!ROB_IN(ENDSTOP_Z_DOWN))
									_rc_motor_moveToStop(MOTOR_Z, -1000000, ENDSTOP_Z_DOWN, _FL_);
								_RC_State++;
								break;

		case RC_STATE_REF+1:	// z at bottom
								if (ROB_IN(ENDSTOP_Z_DOWN))
								{
									_rc_motor_moveToStop(MOTOR_SCREW, 1000, ENDSTOP_SCREW_ROTATION, _FL_);
									_rc_moveto_xy_stop(RX_StepperStatus.screw_posX, -1000000, ENDSTOP_Y_AXIS, _FL_);
									_RC_State++;
								}
								else if (!_RobotStatus.motors[MOTOR_Z].isMoving) 
								{
                                    Error(ERR_CONT, 0, "Z-Axis down sensor not found");
									_RC_State=0;
								}
								break;

		case RC_STATE_REF+2:	// Y in end sensor (out)
								if (ROB_IN(ENDSTOP_Y_AXIS))
								{
									_rc_moveto_xy_stop(-1000000, RX_StepperStatus.screw_posY, ENDSTOP_X_AXIS, _FL_);
									_RC_State++;
								}
								else if (!_RobotStatus.motors[MOTOR_XY_0].isMoving || !_RobotStatus.motors[MOTOR_XY_1].isMoving) 
								{
                                    Error(ERR_CONT, 0, "Y-Axis end sensor not found");
									_RC_State=0;
								}
								break;

		case RC_STATE_REF+3:	// X in end sensor
								if (ROB_IN(ENDSTOP_X_AXIS))
								{
									rc_moveto_xy(RX_StepperStatus.screw_posX-MOTOR_X_GARAGE_POS, RX_StepperStatus.screw_posY, _FL_);
									_RC_State++;
								}
								else if (!_RobotStatus.motors[MOTOR_XY_0].isMoving || !_RobotStatus.motors[MOTOR_XY_1].isMoving) 
								{
                                    Error(ERR_CONT, 0, "X-Axis end sensor not found");
									_RC_State=0;
								}
								break;

		case RC_STATE_REF+4:	// X in garage-pos
								if (!_RobotStatus.motors[MOTOR_XY_0].isMoving && !_RobotStatus.motors[MOTOR_XY_1].isMoving)
								{
									if (_RobotStatus.motors[MOTOR_XY_0].isStalled || _RobotStatus.motors[MOTOR_XY_1].isStalled)
									{
										Error(ERR_CONT, 0, "X-Axis blocked");
										_RC_State=0;
									}
									else 
									{
										_rc_moveto_xy_stop(RX_StepperStatus.screw_posX, -100000, ENDSTOP_GARAGE, _FL_);
										_RC_State++;
									}
								}
								break;

		case RC_STATE_REF+5:	// Y in garage sensor
								if (ROB_IN(ENDSTOP_GARAGE))
								{
									Error(LOG, 0, "Reference done");
									_RC_State=0;
								}
								else if (!_RobotStatus.motors[MOTOR_XY_0].isMoving || !_RobotStatus.motors[MOTOR_XY_1].isMoving) 
								{
                                    Error(ERR_CONT, 0, "Y-Axis garage sensor not found");
									_RC_State=0;
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
	_MotorCfg[motor].iholdirun = 0x11300;
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
static void _configure_screw_motor(void)
{		
	_MotorCfg[MOTOR_SCREW].gconf = 0;
	_MotorCfg[MOTOR_SCREW].swmode = 0x03;
//	_MotorCfg[MOTOR_SCREW].iholdirun = 0x11300;
	_MotorCfg[MOTOR_SCREW].iholdirun = SCREW_CURRENT_LOW;
	_MotorCfg[MOTOR_SCREW].tpowerdown = 0;
	_MotorCfg[MOTOR_SCREW].tpwmthrs = 0;
	_MotorCfg[MOTOR_SCREW].rampmode = 0;
	_MotorCfg[MOTOR_SCREW].vmax = 178957;
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
	_MotorCfg[MOTOR_Z].iholdirun = Z_CURRENT_UP;
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

//--- rc_stop --------------------------------
static void _rc_stop(int motors)
{
	SRobotMotorsStopCmd cmd;
	cmd.header.msgLen = sizeof(cmd);
	cmd.header.msgId  = CMD_MOTORS_STOP;
	cmd.motors = motors;
	sok_send(&_RC_Socket, &cmd);
	TrPrintfL(TRUE, "sent CMD_MOTORS_STOP");
}

//--- _rc_reset --------------------------------
static void _rc_reset(int motor)
{
	SRobotMotorsResetCmd cmd;
	cmd.header.msgLen = sizeof(cmd);
	cmd.header.msgId  = CMD_MOTORS_RESET;
	cmd.motors		  = 1<<motor;
	sok_send(&_RC_Socket, &cmd);
	TrPrintfL(TRUE, "sent CMD_MOTORS_RESET");
	_MotorIsMoving[motor]=FALSE;
}

//--- _rc_motor_moveBy --------------------------
static int _rc_motor_moveBy(int motor, int steps, const char *file, int line)
{
	if (_RobotStatus.motors[motor].isMoving)  return Error(LOG_TYPE_ERROR_CONT, file, line, 0, "_rc_motor_moveBy: Motor[%d].isMoving", motor);
	if (_RobotStatus.motors[motor].isStalled) return Error(LOG_TYPE_ERROR_CONT, file, line, 0, "_rc_motor_moveBy: Motor[%d].isStalled", motor);

	SRobotMotorsMoveCmd cmd;
	memset(&cmd, 0, sizeof(cmd));
	cmd.header.msgLen	 = sizeof(cmd);
	cmd.header.msgId	 = CMD_MOTORS_MOVE;
	cmd.motors			 = 1<<motor;
	cmd.targetPos[motor] = _RobotStatus.motors[motor].motorTargetPos+steps;
	if (_MotorCfg[motor].encmode) cmd.encoderTol[motor]= 100;

	sok_send(&_RC_Socket, &cmd);
	
	TrPrintfL(TRUE, "sent CMD_MOTORS_MOVE");

	return REPLY_OK;
}

//--- _rc_motor_moveToStop --------------------------------
static int _rc_motor_moveToStop(int motor, int steps, int stopInput, const char *file, int line)
{
	if (_RobotStatus.motors[motor].isMoving)  return Error(LOG_TYPE_ERROR_CONT, file, line, 0, "_rc_motor_moveToStop: Motor[%d].isMoving", motor);
	if (_RobotStatus.motors[motor].isStalled) return Error(LOG_TYPE_ERROR_CONT, file, line, 0, "_rc_motor_moveToStop: Motor[%d].isStalled", motor);

	SRobotMotorsMoveCmd cmd;
	memset(&cmd, 0, sizeof(cmd));
	cmd.header.msgLen		 = sizeof(cmd);
	cmd.header.msgId		 = CMD_MOTORS_MOVE;
	cmd.motors				 = 1<<motor;
	cmd.targetPos[motor]	 =  steps;
	cmd.stopBits[motor]		 = 1<<stopInput;
	cmd.stopBitLevels[motor] = 1;
	sok_send(&_RC_Socket, &cmd);
	
	TrPrintfL(TRUE, "sent CMD_MOTORS_MOVE");

	return REPLY_OK;
}

//--- rc_moveto_xy -------------------------------------------
int rc_moveto_xy(int x, int y, const char *file, int line)
{
	return _rc_moveto_xy_stop(x, y, -1, file, line);
}

//--- _rc_moveto_xy_stop -----------------------------------
int _rc_moveto_xy_stop(int x, int y, int stop, const char *file, int line)
{
	for (int motor=MOTOR_XY_0; motor<=MOTOR_XY_1; motor++)
	{
		if (_RobotStatus.motors[motor].isMoving)  return Error(LOG_TYPE_ERROR_CONT, file, line, 0, "_rc_moveto_xy_stop: Motor[%d].isMoving", motor);
		if (_RobotStatus.motors[motor].isStalled) return Error(LOG_TYPE_ERROR_CONT, file, line, 0, "_rc_moveto_xy_stop: Motor[%d].isStalled", motor);
	}

	int dx = x-RX_StepperStatus.screw_posX;
	int dy = y-RX_StepperStatus.screw_posY;

	// x
//	move_motor(_motorStatus[MOTOR_XY_0].motorPosition + motorSteps, MOTOR_XY_0);
//	move_motor(_motorStatus[MOTOR_XY_1].motorPosition + motorSteps, MOTOR_XY_1);
//	_rc_motor_moveBy(MOTOR_XY_0, dx);
//	_rc_motor_moveBy(MOTOR_XY_1, dx);

	// y
//	move_motor(_motorStatus[MOTOR_XY_1].motorPosition + motorSteps, MOTOR_XY_1);
//	move_motor(_motorStatus[MOTOR_XY_0].motorPosition - motorSteps, MOTOR_XY_0);
//	_rc_motor_moveBy(MOTOR_XY_0, -dy);
//	_rc_motor_moveBy(MOTOR_XY_1,  dy);
	
	SRobotMotorsMoveCmd cmd;
	memset(&cmd, 0, sizeof(cmd));
	cmd.header.msgLen	 = sizeof(cmd);
	cmd.header.msgId	 = CMD_MOTORS_MOVE;
	cmd.motors			|= 1<<MOTOR_XY_0;
	cmd.targetPos[MOTOR_XY_0] = _RobotStatus.motors[MOTOR_XY_0].motorTargetPos+dx-dy;
	cmd.motors			|= 1<<MOTOR_XY_1;
	cmd.targetPos[MOTOR_XY_1] = _RobotStatus.motors[MOTOR_XY_1].motorTargetPos+dx+dy;

	if (stop>=0) cmd.stopBits[MOTOR_XY_0] = cmd.stopBits[MOTOR_XY_1] = 1<<stop;

	sok_send(&_RC_Socket, &cmd);
	
	TrPrintfL(TRUE, "sent CMD_MOTORS_MOVE");

	return REPLY_OK;
}

//--- rc_move_top --------------------------------------
int  rc_move_top(const char *file, int line)
{
	return _rc_motor_moveToStop(MOTOR_Z, 1000000, ENDSTOP_Z_UP, _FL_);	
}

//--- rc_move_bottom ---------------------------------------
int  rc_move_bottom(const char *file, int line)
{
	return _rc_motor_moveToStop(MOTOR_Z, -1000000, ENDSTOP_Z_DOWN, _FL_);	
}


//--- rc_display_status -----------------------------------
void rc_display_status(void)
{
	int i;
	
	term_printf("Robot %s ---------------------------------", version);
	
	// Connection information
//	term_printf("\nConnection Status: %d\n", _isConnected);
	
	// Bootloader information
	term_printf("\nBootloader: Status=%d,  addr=%d, blocks=%d, size=%d\n", _RobotStatus.bootloader.status, _RobotStatus.bootloader.progMemPos, _RobotStatus.bootloader.progMemBlocksUsed, _RobotStatus.bootloader.progSize);

	// GPIO information
	term_printf("Inputs:    ");
	for (i = 0; i < NUMBER_OF_INPUTS; i++)
	{
		if ((i % 4) == 0) term_printf(" ");
		if (_RobotStatus.gpio.inputs & (1<<(NUMBER_OF_INPUTS - i - 1))) term_printf("*");
		else	                                                        term_printf("_");
	}
	term_printf("\n");
	
	term_printf("Outputs:   ");
	for (i = 0; i < NUMBER_OF_OUTPUTS; i++)
	{
		if ((i % 4) == 0) term_printf(" ");
		if (_RobotStatus.gpio.outputs & (1<<(NUMBER_OF_OUTPUTS - i - 1))) term_printf("*");
		else	                                                          term_printf("_");
	}
	term_printf("\n");
	
	term_printf("Edge Count: ");
	for (i = 0; i < NUMBER_OF_INPUTS; i++)
	{
		term_printf("%04d ", _RobotStatus.gpio.inputEdges[(NUMBER_OF_INPUTS - i - 1)]);
	}
	term_printf("\n");

	// Motor information
	term_printf("Motors\tPos\t\tTarget Pos\tEnc Pos\t\tIsMoving\tIsStalled\tIsConfigured\n");
	for (i=0; i<MOTOR_COUNT; i++)
	{
		term_printf("%d-", i);
		switch(i)
		{
        case MOTOR_XY_0:	term_printf("XY0"); break;
        case MOTOR_XY_1:	term_printf("XY1"); break;
        case MOTOR_Z:		term_printf("Z"); break;
        case MOTOR_SCREW:	term_printf("Screw"); break;
		}
		term_printf("\t%010d\t%010d\t%010d\t  %1d\t\t  %1d\t\t  %1d\n", 
		_RobotStatus.motors[i].motorPos, 
		_RobotStatus.motors[i].motorTargetPos, 
		_RobotStatus.motors[i].motorEncPos, 
		_RobotStatus.motors[i].isMoving, 
		_RobotStatus.motors[i].isStalled, 
		_RobotStatus.motors[i].isConfigured);
	}
	
	term_printf("\n");
    term_printf("State Machine: \t\t %d\n", _RC_State);
    term_printf("Screwer X-Pos: \t\t %d\n", RX_StepperStatus.screw_posX);
    term_printf("Screwer Y-Pos: \t\t %d\n", RX_StepperStatus.screw_posY);

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
		term_printf("t: move Z to top\n");
		term_printf("b: move Z to botton\n");
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
	int no = str[1]-'0';
	switch (str[0])
	{
    case 'r':	_rc_reset(no); break;
	case 's':	_rc_stop(0x0f); break;
    case 'R':	rc_reference(); break;
    case 'm':	_rc_motor_moveBy(no, atoi(&str[2]), _FL_);	break;
    case 'x':	rc_moveto_xy(RX_StepperStatus.screw_posX+atoi(&str[1]), RX_StepperStatus.screw_posY				 , _FL_); break;
    case 'y':	rc_moveto_xy(RX_StepperStatus.screw_posX,				RX_StepperStatus.screw_posY+atoi(&str[1]), _FL_); break;
    case 'z':	_rc_motor_moveBy(MOTOR_Z, atoi(&str[1]), _FL_);		break;
    case 'b':	rc_move_bottom(_FL_); break;
    case 't':	rc_move_top(_FL_); break;
    default:
        break;
	}
}
