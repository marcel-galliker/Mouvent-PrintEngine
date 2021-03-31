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
#include "rx_trace.h"
#include "version.h"
#include "robot_client.h"

//--- Modlue Globals -----------------------------------------------------------------
static char _IpAddr[32]="";
RX_SOCKET	_RC_Socket = INVALID_SOCKET;
HANDLE					_RobotHdl;
static SRobotStatusMsg	_RobotStatus;
static int				_MotorInit;

//--- Prototypes ------------------------------------------------------
static void* _robot_ctrl_thread(void* lpParameter);
static int _handle_robot_ctrl_msg(RX_SOCKET socket, void *msg, int len, struct sockaddr *sender, void *ppar);
static int _connection_closed(RX_SOCKET socket, const char *peerName);

static void _config_motors(void);

static void _configure_xy_motor(uint8_t motor);
static void _configure_screw_motor(void);
static void _configure_z_motor(void);


static void _rc_stop(int motors);
static void _rc_reset(int motor);
static void _rc_motor_moveBy(int motor, int steps);
static void _rc_motor_moveToStop(int motor, int input);
static void _rc_motor_moveToRef(int motor, int direction);


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
	}
}

//--- _robot_ctrl_thread --------------------------------------------------------
static void* _robot_ctrl_thread(void* lpParameter)
{
	int errNo;
	while(TRUE)
	{
		if (_RC_Socket!=INVALID_SOCKET)
		{
		//	sok_send_2(&_RC_Socket, CMD_STATUS_GET, 0, NULL);

			SMsgHdr msg;
			msg.msgId = CMD_STATUS_GET;
			msg.msgLen = sizeof(msg);
			sok_send(&_RC_Socket, &msg);
		}
		else if (_IpAddr[0])
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
static void _config_motors(void)
{
	int motor;
	for (motor=0; motor<MOTOR_COUNT; motor++)
	{
		if (!_RobotStatus.motors[motor].isConfigured) _MotorInit &= ~(1<<motor);
		if ((_MotorInit & (1<<motor)) == 0) 
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
			return;
		}
	}
}

//--- _handle_robot_ctrl_msg --------------------------------------------------------
static int _handle_robot_ctrl_msg(RX_SOCKET socket, void *msg, int len, struct sockaddr *sender, void *ppar)
{
	SMsgHdr			*phdr = (SMsgHdr*)msg;
	switch(phdr->msgId)
	{
    case CMD_STATUS_GET:	memcpy(&_RobotStatus, msg, sizeof(_RobotStatus));
							_config_motors();
							break;

	default:				Error(WARN, 0, "Got unknown messageId=0x%08x", phdr->msgId);
	}
}

//--- configure_xy_motor -------------------------------------
static void _configure_xy_motor(uint8_t motor)
{
	SRobotMotorSetCfgCmd configCommand;
	memset(&configCommand, 0, sizeof(configCommand));
	
	configCommand.header.msgId = CMD_MOTOR_SET_CONFIG;
	configCommand.header.msgLen = sizeof(configCommand);
	
	configCommand.motor = motor;
	
	configCommand.config.gconf = 0;
	configCommand.config.swmode = 0x03;
	configCommand.config.iholdirun = 0x11300;
	configCommand.config.tpowerdown = 0;
	configCommand.config.tpwmthrs = 0;
	configCommand.config.rampmode = 0;
	configCommand.config.vmax = 26844;
	configCommand.config.v1 = 0;
	configCommand.config.amax = 440;
	configCommand.config.dmax = 440;
	configCommand.config.a1 = 440;
	configCommand.config.d1 = 440;
	configCommand.config.vstart = 0;
	configCommand.config.vstop = 10;
	configCommand.config.tzerowait = 0;
	configCommand.config.vdcmin = 0;
	configCommand.config.chopconf = 0x30375;
	configCommand.config.coolconf = 0;
	configCommand.config.pwmconf = 0;
	configCommand.config.tcoolthrs = 0;
	configCommand.config.thigh = 0;
	configCommand.config.encmode = 0x400;
	configCommand.config.encconst = 0x307CB;
	
	sok_send(&_RC_Socket, &configCommand);
}

//--- configure_screw_motor ------------------------
static void _configure_screw_motor(void)
{
	SRobotMotorSetCfgCmd configCommand;
	memset(&configCommand, 0, sizeof(configCommand));
	
	configCommand.header.msgId = CMD_MOTOR_SET_CONFIG;
	configCommand.header.msgLen = sizeof(configCommand);
	
	configCommand.motor = MOTOR_SCREW;
	
	configCommand.config.gconf = 0;
	configCommand.config.swmode = 0x03;
	configCommand.config.iholdirun = 0x11300;
	configCommand.config.tpowerdown = 0;
	configCommand.config.tpwmthrs = 0;
	configCommand.config.rampmode = 0;
	configCommand.config.vmax = 178957;
	configCommand.config.v1 = 0;
	configCommand.config.amax = 2932;
	configCommand.config.dmax = 2932;
	configCommand.config.a1 = 2932;
	configCommand.config.d1 = 2932;
	configCommand.config.vstart = 0;
	configCommand.config.vstop = 10;
	configCommand.config.tzerowait = 0;
	configCommand.config.vdcmin = 0;
	configCommand.config.chopconf = 0x101F5;
	configCommand.config.coolconf = 0;
	configCommand.config.pwmconf = 0;
	configCommand.config.tcoolthrs = 0;
	configCommand.config.thigh = 0;
	configCommand.config.encmode = 0;
	configCommand.config.encconst = 0;
	
	sok_send(&_RC_Socket, &configCommand);
}

//--- configure_z_motor ---------------------
static void _configure_z_motor(void)
{
	SRobotMotorSetCfgCmd configCommand;
	memset(&configCommand, 0, sizeof(configCommand));
	
	configCommand.header.msgId = CMD_MOTOR_SET_CONFIG;
	configCommand.header.msgLen = sizeof(configCommand);
	
	configCommand.motor = MOTOR_Z;
	
	configCommand.config.gconf = 0;
	configCommand.config.swmode = 0x03;
	configCommand.config.iholdirun = 0x10A00;
	configCommand.config.tpowerdown = 0;
	configCommand.config.tpwmthrs = 0;
	configCommand.config.rampmode = 0;
	configCommand.config.vmax = 322122;
	configCommand.config.v1 = 0;
	configCommand.config.amax = 5277;
	configCommand.config.dmax = 5277;
	configCommand.config.a1 = 5277;
	configCommand.config.d1 = 5277;
	configCommand.config.vstart = 0;
	configCommand.config.vstop = 10;
	configCommand.config.tzerowait = 0;
	configCommand.config.vdcmin = 0;
	configCommand.config.chopconf = 0x30475;
	configCommand.config.coolconf = 0;
	configCommand.config.pwmconf = 0;
	configCommand.config.tcoolthrs = 0;
	configCommand.config.thigh = 0;
	configCommand.config.encmode = 0;
	configCommand.config.encconst = 0;
	
	sok_send(&_RC_Socket, &configCommand);
}

//--- rc_display_status -----------------------------------
void rc_display_status(void)
{
	int i;
	
	term_printf("Robot %s ---------------------------------", version);
	
	// Connection information
//	term_printf("\nConnection Status: %d\n", _isConnected);
	
	// Bootloader information
	term_printf("\nBootloader Status: %d\n", _RobotStatus.bootloader.status);
	term_printf("Memory position: %d\n",	_RobotStatus.bootloader.progMemPos);
	term_printf("Memory blocks used: %d\n", _RobotStatus.bootloader.progMemBlocksUsed);
	term_printf("Programm size: %d\n",		_RobotStatus.bootloader.progSize);
	
	// GPIO information
	term_printf("\nInputs:");
	for (i = 0; i < NUMBER_OF_INPUTS; i++)
	{
		if ((i % 4) == 0)
			term_printf(" ");
		
		term_printf("%d", (_RobotStatus.gpio.inputs >> (NUMBER_OF_INPUTS - i - 1)) & 1);
	}
	term_printf("\n");
	
	term_printf("Outputs: ");
	for (i = 0; i < NUMBER_OF_OUTPUTS; i++)
	{
		if ((i % 4) == 0)
			term_printf(" ");
		
		term_printf("%d", (_RobotStatus.gpio.outputs >> (NUMBER_OF_OUTPUTS - i - 1)) & 1);
	}
	term_printf("\n");
	
	term_printf("Edge Count: ");
	for (i = 0; i < NUMBER_OF_INPUTS; i++)
	{
		term_printf("%04d ", _RobotStatus.gpio.inputEdges[(NUMBER_OF_INPUTS - i - 1)]);
	}
	term_printf("\n");
	
	// Motor information
	term_printf("\nMotors\tPos\t\tTarget Pos\tEnc Pos\t\tIsMoving\tIsStalled\tIsConfigured\n");
	for (i=0; i<MOTOR_COUNT; i++)
	{
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
	
	term_flush();
}

//--- rc_stop --------------------------------
static void _rc_stop(int motors)
{
	SRobotMotorsStopCmd cmd;
	cmd.header.msgLen = sizeof(cmd);
	cmd.header.msgId  = CMD_MOTORS_STOP;
	cmd.motors = motors;
	sok_send(&_RC_Socket, &cmd);
}

//--- _rc_reset --------------------------------
static void _rc_reset(int motor)
{
	SRobotMotorsResetCmd cmd;
	cmd.header.msgLen = sizeof(cmd);
	cmd.header.msgId  = CMD_MOTORS_RESET;
	cmd.motors = 1<<motor;
	sok_send(&_RC_Socket, &cmd);
}

//--- _rc_motor_moveBy --------------------------
static void _rc_motor_moveBy(int motor, int steps)
{
	SRobotMotorsMoveCmd cmd;
	memset(&cmd, 0, sizeof(cmd));
	cmd.header.msgLen = sizeof(cmd);
	cmd.header.msgId  = CMD_MOTORS_MOVE;
	cmd.motors = 1<<motor;
	cmd.targetPos[motor] = _RobotStatus.motors[motor].motorTargetPos+steps;

	sok_send(&_RC_Socket, &cmd);
}

//--- _rc_motor_moveToStop --------------------------------
static void _rc_motor_moveToStop(int motor, int stopInput)
{
	SRobotMotorsMoveCmd cmd;
	memset(&cmd, 0, sizeof(cmd));
	cmd.header.msgLen = sizeof(cmd);
	cmd.header.msgId  = CMD_MOTORS_MOVE;
	cmd.motors			 = 1<<motor;
	cmd.targetPos[motor] = 1000000;
	cmd.stopBits[motor]  = 1<<stopInput;
	cmd.stopBitLevels[motor] = 1;
	sok_send(&_RC_Socket, &cmd);	
}

//--- _rc_motor_moveToRef ----------------------------------
static void _rc_motor_moveToRef(int motor, int direction)
{
	switch(motor)
	{
    case MOTOR_XY_0:	_rc_motor_moveToStop(motor, 2*motor+direction);	break;
    case MOTOR_XY_1:	_rc_motor_moveToStop(motor, 2*motor+direction);	break;
    case MOTOR_SCREW:	_rc_motor_moveToStop(motor, 2*motor+direction);	break;
    case MOTOR_Z:		_rc_motor_moveToStop(motor, 2*motor+direction);	break;
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
//		term_printf("R: Reference\n");
		term_printf("r<n><0/1>: move Motor<n> to stop\n");
		term_printf("m<n><steps>: move Motor<n> by <steps>\n");
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
	case 's':	_rc_stop(0x0f); break;
    case 'm':	_rc_motor_moveBy(no, atoi(&str[2]));	break;
    case 'r':	_rc_reset(no); break;
					//_rc_motor_moveToRef(no, atoi(&str[2]));		break;
    default:
        break;
	}
}
