#include "comm.h"
#include "rx_threads.h"
#include "rx_term.h"
#include "tcp_ip.h"
#include "rx_error.h"
#include "lb702.h"
#include "lbrob.h"

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <pthread.h>
#include <sys/ioctl.h>
#include <linux/sockios.h>

#include "robi.h"
#include "robi_def.h"

#define ROBI_CONNECTION_TIMEOUT		1000
#define ROBI_STATUS_UPDATE_INTERVAL 250

#define STEPS_PER_REV				51200
#define DISTANCE_UM_PER_REV			36000

#define ROBI_MIN_MSG_ID		1
#define ROBI_MAX_MSG_ID		UINT32_MAX

#define ROBI_FIFO_SIZE		32
#define ROBI_SERIAL_PORT	"/dev/ttyS0"

#define TIME_BEFORE_TURN_SCREWER    2600

// define inputs
#define SCREW_IN_UP 0
#define SCREW_IN_DOWN 1
#define SCREW_IN_REF 2
#define X_IN_REF 3
#define Y_IN_REF 4

#define MIN_Y_POS -35000

#define MAX_VARIANCE 100 // um

#define MAX_VAR_SCREW_POS 7000 // um


static int32_t
set_serial_attributs(int fd, int speed, int parity);
static int32_t send_command(uint32_t commandCode, uint8_t len, void *data);
static void* send_thread(void *par);
static void* receive_thread(void *par);
static void robi_set_output(int num, int val);
static int _steps_2_micron(int steps);
static int _micron_2_steps(int micron);
static int _motors_move_done();

static int _usbPort;

volatile static SUsbTxMsg		_txFifo[ROBI_FIFO_SIZE];
volatile static int32_t			_txFifoInIndex;
volatile static int32_t			_txFifoOutIndex;

static uint32_t _tStatus;
static SRobiStatus _robiStatus;

static uint32_t _msgId;
static uint32_t _msgSentCounter;
static uint32_t _msgReceivedCounter;
static uint32_t _MsgsCaptured;
static uint32_t _MsgsSent;

static int32_t _isConnected;
static int32_t _isSync;
static uint32_t _syncMessageId;

static uint32_t _lastMessageTimestamp;

static int32_t _isInit;

static int _CmdRunning = 0;
static int _NewCmd = 0;
static int _Value = 0;
static int _CmdStarted = FALSE;

static int _Search_Screw_Time = 0;
static int _Loosen_Screw_Time = 0;
static int _Loosen_Times = 0;
static int _Robi_Disabled = FALSE;


void robi_init(void)
{
    if (_isInit) return;

    RX_StepperStatus.screwerinfo.moving = FALSE;
    RX_StepperStatus.screwerinfo.ref_done = FALSE;

    _tStatus = 0;
	
	_isConnected = FALSE;
	_isSync = FALSE;
	
	_usbPort = open(ROBI_SERIAL_PORT, O_RDWR | O_NOCTTY | O_SYNC);
	set_serial_attributs(_usbPort, B115200, 0);
	
	_msgId = ROBI_MIN_MSG_ID;
	_msgSentCounter = _msgId;
	_msgReceivedCounter = _msgSentCounter;
	
	memset(_txFifo, 0, sizeof(_txFifo));
	_txFifoInIndex  = 0;
	_txFifoOutIndex = 0;
	
	memset(&_robiStatus, 0, sizeof(_robiStatus));
	
	_isInit = TRUE;
	
	rx_thread_start(send_thread, NULL, 0, "robi_send_thread");
	rx_thread_start(receive_thread, NULL, 0, "robi_receive_thread");
    return;
}

void robi_main(int ticks, int menu)
{
    if (_isInit == FALSE) return;

    if (ticks > (_lastMessageTimestamp + ROBI_CONNECTION_TIMEOUT))
		_isConnected = FALSE;
	
	if (ticks > _tStatus)
	{
		send_command(STATUS_UPDATE, 0, NULL);
		_tStatus = ROBI_STATUS_UPDATE_INTERVAL*(1 + ticks / ROBI_STATUS_UPDATE_INTERVAL); 
	}
    
    RX_StepperStatus.screwerinfo.z_in_down = _robiStatus.gpio.inputs & (1UL << SCREW_IN_DOWN) && !(_robiStatus.gpio.inputs & (1UL << SCREW_IN_UP));
    RX_StepperStatus.screwerinfo.z_in_up    = _robiStatus.gpio.inputs & (1UL << SCREW_IN_UP) && !(_robiStatus.gpio.inputs & (1UL << SCREW_IN_DOWN));

    if (RX_StepperStatus.screwerinfo.z_in_down) _Loosen_Times = 0;
    if (_CmdRunning)	RX_StepperStatus.screwerinfo.moving = TRUE;
    else				RX_StepperStatus.screwerinfo.moving = FALSE;

    if (RX_StepperStatus.screwerinfo.moving)
    {
        RX_StepperStatus.screwerinfo.robi_in_ref = FALSE;
        //RX_StepperStatus.screwerinfo.z_in_down = FALSE;
        RX_StepperStatus.screwerinfo.x_in_pos = FALSE;
        RX_StepperStatus.screwerinfo.y_in_pos = FALSE;
        RX_StepperStatus.screwerinfo.y_in_ref = FALSE;
        RX_StepperStatus.screwerinfo.z_in_up = FALSE;
        RX_StepperStatus.screwerinfo.screw_loosed = FALSE;
        RX_StepperStatus.screwerinfo.screw_tight = FALSE;
    }

    RX_StepperStatus.screwerinfo.ref_done = _robiStatus.motors[MOTOR_XY_0].isReferenced &&
               _robiStatus.motors[MOTOR_XY_1].isReferenced &&
               _robiStatus.motors[MOTOR_SCREW].isReferenced;

    if (!RX_StepperStatus.screwerinfo.ref_done)
    {
        _Search_Screw_Time = 0;
        _Loosen_Screw_Time = 0;
    }

    RX_StepperStatus.screw_posX =
        (_steps_2_micron(_robiStatus.motors[MOTOR_XY_0].motorEncoderPosition +
                         _robiStatus.motors[MOTOR_XY_1].motorEncoderPosition))/2;

    RX_StepperStatus.screw_posY =
        (_steps_2_micron(_robiStatus.motors[MOTOR_XY_1].motorEncoderPosition -
                         _robiStatus.motors[MOTOR_XY_0].motorEncoderPosition))/2;

    if (_Search_Screw_Time && rx_get_ticks() > _Search_Screw_Time + TIME_BEFORE_TURN_SCREWER)
    {
        int val = 0;
        if (RX_StepperStatus.screw_posY >= (SCREW_Y_BACK + SCREW_Y_FRONT)/2)
            val = 213333;
        else
            val = -213333;
        robi_handle_ctrl_msg(INVALID_SOCKET, CMD_ROBI_SCREW_STEPS, &val);
        _Search_Screw_Time = rx_get_ticks();
    }
    
    if (_Loosen_Screw_Time && rx_get_ticks() > _Loosen_Screw_Time + TIME_BEFORE_TURN_SCREWER)
    {
        int val = 0;
        if (RX_StepperStatus.screw_posY <= (SCREW_Y_BACK + SCREW_Y_FRONT) / 2)
            val = -213333;
        else
            val = 213333;
        if (_Loosen_Times == 1)
            val *= (-1);
        else
            _Loosen_Times++;
        robi_handle_ctrl_msg(INVALID_SOCKET, CMD_ROBI_SCREW_STEPS, &val);
        if (_Loosen_Times == 2)
        {
            _Loosen_Screw_Time = 0;
            _Loosen_Times = 0;
        }
        
    }

    if (!_motors_move_done())
    {
        _CmdStarted = TRUE;
    }
    else if (!_CmdRunning)
    {
        _CmdStarted = FALSE;
    }

    if (_CmdRunning && _motors_move_done() && _CmdStarted)
    {
        int loc_new_cmd, loc_new_value;
        
        _CmdStarted = FALSE;
        _Search_Screw_Time = 0;
        _Loosen_Screw_Time = 0;
        RX_StepperStatus.screwerinfo.moving = FALSE;
        switch (_CmdRunning)
        {
        case CMD_ROBI_REFERENCE:
            if (RX_StepperStatus.screwerinfo.ref_done)
            {
                _CmdRunning = 0;
                if (!(_robiStatus.gpio.inputs & (1UL << Y_IN_REF)))
                {
                    Error(ERR_CONT, 0, "Robi-Sensor in Garage not high");
                    RX_StepperStatus.screwerinfo.ref_done = 0;
                }
                else
                {
                    RX_StepperStatus.screwerinfo.y_in_ref = TRUE;
                }
            }
            else
            {
                if (!_isConnected) Error(ERR_CONT, 0, "Connection lost during reference");
                if (!(_robiStatus.gpio.inputs & (1UL << Y_IN_REF))) Error(ERR_CONT, 0, "Robi-Sensor in Garage not high");
                _CmdRunning = 0;
                _NewCmd = 0;
                _Value = 0;
            }
            break;

        case CMD_ROBI_MOVE_TO_GARAGE:
            if (!(_robiStatus.gpio.inputs & (1UL << Y_IN_REF)))
            {
                Error(ERR_CONT, 0, "Robi-Sensor in Garage not high");
                RX_StepperStatus.screwerinfo.ref_done = 0;
            }
            else
            {
                RX_StepperStatus.screwerinfo.y_in_ref = TRUE;
            }
            _CmdRunning = 0;
            break;

        case CMD_ROBI_MOVE_TO_Y:
                if (abs(RX_StepperStatus.screw_posY - SCREW_Y_BACK) < MAX_VAR_SCREW_POS ||
                    abs(RX_StepperStatus.screw_posY - SCREW_Y_FRONT) < MAX_VAR_SCREW_POS)
                    RX_StepperStatus.screwerinfo.y_in_pos = TRUE;
                _CmdRunning = 0;
                break;

            case CMD_ROBI_MOVE_TO_X:
                if (abs(RX_StepperStatus.screw_posX - SCREW_X_LEFT) < MAX_VAR_SCREW_POS ||
                    abs(RX_StepperStatus.screw_posX - SCREW_X_RIGHT) <  MAX_VAR_SCREW_POS)
                    RX_StepperStatus.screwerinfo.x_in_pos = TRUE;
                _CmdRunning = 0;
                break;

            case CMD_ROBI_SCREW_RIGHT:
                if (abs(RX_StepperStatus.screw_posY - SCREW_Y_BACK) < MAX_VAR_SCREW_POS)
                    RX_StepperStatus.screwerinfo.screw_tight = TRUE;
                else if (abs(RX_StepperStatus.screw_posY - SCREW_Y_FRONT) < MAX_VAR_SCREW_POS)
                    RX_StepperStatus.screwerinfo.screw_loosed = TRUE;
                _CmdRunning = 0;
                break;
                
            case CMD_ROBI_SCREW_LEFT:
                if (abs(RX_StepperStatus.screw_posY - SCREW_Y_FRONT) < MAX_VAR_SCREW_POS)
                    RX_StepperStatus.screwerinfo.screw_tight = TRUE;
                else if (abs(RX_StepperStatus.screw_posY - SCREW_Y_BACK) < MAX_VAR_SCREW_POS)
                    RX_StepperStatus.screwerinfo.screw_loosed = TRUE;
                _CmdRunning = 0;
                break;

            case CMD_ROBI_MOVE_Y:
            case CMD_ROBI_MOVE_X:
            case CMD_ROBI_MOVE_Z_UP:
            case CMD_ROBI_MOVE_Z_DOWN:
            

                _CmdRunning = 0;
            break;
        default:
            break;
        }
        if (RX_StepperStatus.screwerinfo.ref_done && _NewCmd)
        {
            loc_new_cmd = _NewCmd;
            loc_new_value = _Value;
            _NewCmd = 0;
            _Value = 0;
            switch (loc_new_cmd)
            {
            case CMD_ROBI_MOVE_Y:
            case CMD_ROBI_MOVE_X:
            case CMD_ROBI_MOVE_TO_X:
            case CMD_ROBI_MOVE_TO_Y:
                robi_handle_ctrl_msg(INVALID_SOCKET, loc_new_cmd, &loc_new_value);
                break;
            case CMD_ROBI_MOVE_Z_UP:
            case CMD_ROBI_MOVE_TO_GARAGE:
                robi_handle_ctrl_msg(INVALID_SOCKET, loc_new_cmd, NULL);
                break;
            default:
                Error(LOG, 0, "Command %d not implemented.", loc_new_cmd);
                break;
            }
        }
    }

    return;
}



void robi_menu(int help)
{
	if (_isInit == FALSE)
		return;
	
	if (help)
	{
		term_printf("Robi MENU ------------------------- (3)\n");
		term_printf("s: STOP\n");
		term_printf("o<num><value>: set output <num> to value <value>\n");
		term_printf("R: Reference\n");
		term_printf("v<micron>: Move x axis <micron>um\n");
		term_printf("w<micron>: Move y axis <micron>um\n");
		term_printf("t<steps>: Turn screw <steps>\n");
		term_printf("l<ticks>: Turn screw left <ticks>\n");
		term_printf("r<ticks>: Turn screw right <ticks>\n");
		term_printf("u: Z up\n");
		term_printf("d: Z down\n");
        term_printf("e<micron>: Move to x pos <micron>\n");
        term_printf("f<micron>: Move to y pos <micron>\n");
        term_printf("g: move to garage position\n");
        term_printf("D: Disable Robi\n");
        term_flush();
	}
	else
	{
		term_printf("?: help\n");
		term_printf("1: LB702 menu\n");
		term_printf("2: LBROB menu\n");
	}
	
	return;
}

void robi_handle_menu(char *str)
{
    int num = 0;
    int val = 0;

    if (_isInit == FALSE)
		return;
	
	_MsgsCaptured++;
	
	switch (str[0])
	{
	case 's': robi_handle_ctrl_msg(INVALID_SOCKET, CMD_ROBI_STOP, NULL); break;
	case 'o':
        // num[0] = str[1];
        // val[0] = str[2];
        num = str[1] - '0';
        val = str[2] - '0';
        robi_set_output(num, val);
		break;
	case 'R':
        robi_handle_ctrl_msg(INVALID_SOCKET, CMD_ROBI_REFERENCE, NULL); break;
	case 'v':
        val = atoi(&str[1]);
        robi_handle_ctrl_msg(INVALID_SOCKET, CMD_ROBI_MOVE_X, &val); break;
	case 'w':
        val = atoi(&str[1]);
        robi_handle_ctrl_msg(INVALID_SOCKET, CMD_ROBI_MOVE_Y, &val); break;
	case 't':
        val = atoi(&str[1]);
        robi_handle_ctrl_msg(INVALID_SOCKET, CMD_ROBI_SCREW_STEPS, &val);
        break;
	case 'l':
        val = atoi(&str[1]);
        robi_handle_ctrl_msg(INVALID_SOCKET, CMD_ROBI_SCREW_LEFT, &val);
        break;
    case 'r':
        val = atoi(&str[1]);
        robi_handle_ctrl_msg(INVALID_SOCKET, CMD_ROBI_SCREW_RIGHT, &val);
        break;
	case 'u':
        robi_handle_ctrl_msg(INVALID_SOCKET, CMD_ROBI_MOVE_Z_UP, NULL); break;
	case 'd': 
        robi_handle_ctrl_msg(INVALID_SOCKET, CMD_ROBI_MOVE_Z_DOWN, NULL); break;
    case 'e':
        val = atoi(&str[1]);
        robi_handle_ctrl_msg(INVALID_SOCKET, CMD_ROBI_MOVE_TO_X, &val);
        break;
        
    case 'f':
        val = atoi(&str[1]);
        robi_handle_ctrl_msg(INVALID_SOCKET, CMD_ROBI_MOVE_TO_Y, &val);
        break;
        
    case 'g':
        robi_handle_ctrl_msg(INVALID_SOCKET, CMD_ROBI_MOVE_TO_GARAGE, NULL);
        break;
    case 'D':
        _Robi_Disabled = !_Robi_Disabled;
        break;
    default:
        break;
	}

    return;
}

void robi_display_status(void)
{
	uint32_t i;
	
	if (_isInit == FALSE)
		return;
	
	term_printf("\n");
	term_printf("Robi system status ---------------------------------\n");
	term_printf("Connection: %d\n", _isConnected);
	//term_printf("Sync: %d\n", _isSync);
    term_printf("moving:         %d		cmd: %08x\n",
                RX_StepperStatus.screwerinfo.moving, _CmdRunning);
	//term_printf("Message id: %d\n", _msgId);
	//term_printf("Message sent: %d\n", _msgSentCounter);
	//term_printf("Message received: %d\n", _msgReceivedCounter);
	//term_printf("User messages captured: %d\n", _MsgsCaptured);
	//term_printf("User messages sent: %d\n", _MsgsSent);
	//term_printf("Command0 running: %d\n", _robiStatus.commandRunning[COMMAND0] & _robiStatus);
	//term_printf("Command1 running: %d\n", _robiStatus.commandRunning[COMMAND1]);
	term_printf("Robi gpio status -----------------------------------\n");
	term_printf("Robi Inputs: ");
	for (i = 0; i < INPUT_COUNT; i++)
	{
		if (_robiStatus.gpio.inputs & (1UL << i))	term_printf("*");
		else										term_printf("_");
		if (i % 4 == 3)			   					term_printf("   ");
	}
	term_printf("\n");
	term_printf("Robi Outputs: ");
	for (i = 0; i < OUTPUT_COUNT; i++)
	{
		if (_robiStatus.gpio.outputs & (1UL << i))	term_printf("*");
		else										term_printf("_");
		if (i % 4 == 3)			   					term_printf("   ");
	}
	term_printf("\n");
	term_printf("Robi motor status ----------------------------------\n");
	term_printf("MOTOR_XY_0:\tMotor pos.: %10d Motor target pos.: %10d Encoder pos.: %10d\n", 
		_steps_2_micron(_robiStatus.motors[MOTOR_XY_0].motorPosition) , 
		_steps_2_micron(_robiStatus.motors[MOTOR_XY_0].motorTargetPosition), 
		_steps_2_micron(_robiStatus.motors[MOTOR_XY_0].motorEncoderPosition));
	term_printf("MOTOR_XY_1:\tMotor pos.: %10d Motor target pos.: %10d Encoder pos.: %10d\n", 
		_steps_2_micron(_robiStatus.motors[MOTOR_XY_1].motorPosition), 
		_steps_2_micron(_robiStatus.motors[MOTOR_XY_1].motorTargetPosition), 
		_steps_2_micron(_robiStatus.motors[MOTOR_XY_1].motorEncoderPosition));
	term_printf("MOTOR_SCREW:\tMotor pos.: %10d Motor target pos.: %10d Encoder pos.: %10d\n", 
		_robiStatus.motors[MOTOR_SCREW].motorPosition, 
		_robiStatus.motors[MOTOR_SCREW].motorTargetPosition, 
		_robiStatus.motors[MOTOR_SCREW].motorEncoderPosition);

    term_printf("Screwer X-Pos: %d\n", RX_StepperStatus.screw_posX);
    term_printf("Screwer Y-Pos: %d\n", RX_StepperStatus.screw_posY);

    term_printf("MOTOR_XY_0 ref:\t\t%d\n", _robiStatus.motors[MOTOR_XY_0].isReferenced);
	term_printf("MOTOR_XY_1 ref:\t\t%d\n", _robiStatus.motors[MOTOR_XY_1].isReferenced);
	term_printf("MOTOR_SCREW ref:\t%d\n", _robiStatus.motors[MOTOR_SCREW].isReferenced);
	term_printf("Z position:            %d\n", _robiStatus.zPos);
	term_printf("Robi position status -------------------------------\n");
	term_printf("Garage:                %d\n", _robiStatus.isInGarage);
	term_printf("Ref:                   %d\n", _robiStatus.isInRef);
    term_printf("Robi disabled:         %d\n", _Robi_Disabled);
}

static void robi_set_output(int num, int val)
{
	uint8_t gpioData[] = {num, val};
	
	send_command(GPIO_SET_OUTPUT, sizeof(gpioData), &gpioData);
}

static int _steps_2_micron(int steps)
{
    int micron;
    micron = (int)(0.5 + ((double )steps * DISTANCE_UM_PER_REV / STEPS_PER_REV));
    return micron;

}

static int _micron_2_steps(int micron)
{
    int steps;
    steps = (int)(0.5 + ((double )micron * STEPS_PER_REV / DISTANCE_UM_PER_REV));
    return steps;
}

//--- robi_handle_ctrl_msg ----------------------------------------------
int robi_handle_ctrl_msg(RX_SOCKET socket, int msgId, void *pdata)
{
    if (_isInit == FALSE || _isConnected == FALSE)
    {
        if (!_isInit) Error(ERR_CONT, 0, "Robi is not initalized");
        if (!_isConnected) Error(ERR_CONT, 0, "Robi is not connected");
        lb702_reset_variables();
        lbrob_reset_variables();
        return REPLY_ERROR;
    }
    

    int micron, steps, ticks, pos;
    switch (msgId)
    {
    case CMD_ROBI_STOP:
        _CmdRunning = 0;
        send_command(MOTOR_ESTOP, 0, NULL);
        break;
    case CMD_ROBI_REFERENCE:
            _CmdRunning = msgId;
            send_command(MOTOR_MOVE_Z_DOWN, 0, NULL);
            send_command(MOTORS_DO_REFERENCE, 0, NULL);
            break;
        break;
    case CMD_ROBI_MOVE_X:
        if (!_CmdRunning)
        {
            if (!RX_StepperStatus.screwerinfo.ref_done)
            {
                _NewCmd = msgId;
                _Value = *((INT32 *)pdata);
                robi_handle_ctrl_msg(INVALID_SOCKET, CMD_ROBI_REFERENCE, NULL);
                break;
            }
            if (!(_robiStatus.gpio.inputs & (1UL << SCREW_IN_DOWN)))
            {
                _NewCmd = msgId;
                _Value = *((INT32 *)pdata);
                robi_handle_ctrl_msg(INVALID_SOCKET, CMD_ROBI_MOVE_Z_DOWN,
                                     NULL);
            }
            else if (RX_StepperStatus.screw_posY - MIN_Y_POS > MAX_VARIANCE)
            {
                Error(ERR_CONT, 0,
                      "Screwer to close to garage to move in x axis");
                break;
            }
            else
            {
                _CmdRunning = msgId;
                micron = *((INT32 *)pdata);
                steps = _micron_2_steps(micron);
                send_command(MOTOR_MOVE_X_RELATIVE, sizeof(steps), &steps);
            }
            break;
        }
        break;
        

    case CMD_ROBI_MOVE_Y:
        if (!_CmdRunning)
        {
            if (!RX_StepperStatus.screwerinfo.ref_done)
            {
                _NewCmd = msgId;
                _Value = *((INT32 *)pdata);
                robi_handle_ctrl_msg(INVALID_SOCKET, CMD_ROBI_REFERENCE, NULL);
                break;
            }
            if (!(_robiStatus.gpio.inputs & (1UL << SCREW_IN_DOWN)))
            {
                _NewCmd = msgId;
                _Value = *((INT32 *)pdata);
                robi_handle_ctrl_msg(INVALID_SOCKET, CMD_ROBI_MOVE_Z_DOWN,
                                     NULL);
                break;
            }
            else
            {
                _CmdRunning = msgId;
                micron = *((INT32 *)pdata);
                steps = _micron_2_steps(micron);
                send_command(MOTOR_MOVE_Y_RELATIVE, sizeof(steps), &steps);
            }
            break;
        }
        break;

    case CMD_ROBI_MOVE_Z_UP:
        if (!_CmdRunning)
        {
            if ((_robiStatus.gpio.inputs & (1UL << SCREW_IN_UP))) {Error(ERR_CONT, 0, "Screwer is already up"); break;}
            if (!RX_StepperStatus.screwerinfo.ref_done)
            {
                _NewCmd = msgId;
                robi_handle_ctrl_msg(INVALID_SOCKET, CMD_ROBI_REFERENCE, NULL);
                break;
            }
            if (RX_StepperStatus.screw_posY - MIN_Y_POS > MAX_VARIANCE)
            {
                Error(ERR_CONT, 0, "Screwer to close to garage to move up");
                break;
            }
            if (_robiStatus.gpio.inputs & (1UL << SCREW_IN_UP))
            {
                Error(LOG, 0, "Screwer already in up pos");
                break;
            }
            if (abs(RX_StepperStatus.screw_posX - SCREW_X_LEFT) > MAX_VAR_SCREW_POS &&
                abs(RX_StepperStatus.screw_posX - SCREW_X_RIGHT) > MAX_VAR_SCREW_POS)
            {
                Error(ERR_CONT, 0, "Screw not in position to lift up");
                break;
            }
            _CmdRunning = msgId;
            _Search_Screw_Time = rx_get_ticks();
            send_command(MOTOR_MOVE_Z_UP, 0, NULL);
            break;
        }
        break;
        
    case CMD_ROBI_MOVE_Z_DOWN:
        if (!_CmdRunning)
        {
            if ((_robiStatus.gpio.inputs & (1UL << SCREW_IN_DOWN))) {Error(ERR_CONT, 0, "Screwer is already down"); break;}
            _CmdRunning = msgId;
            _Loosen_Screw_Time = rx_get_ticks();
            send_command(MOTOR_MOVE_Z_DOWN, 0, NULL);
            break;
        }
        break;
        
    case CMD_ROBI_SCREW_STEPS:
        if (!RX_StepperStatus.screwerinfo.ref_done)
        {
            Error(ERR_CONT, 0, "Robi reference is missing.");
            break;
        }
        steps = *((INT32 *)pdata);
        send_command(MOTOR_TURN_SCREW_RELATIVE, sizeof(steps), &steps);
        break;
        
    case CMD_ROBI_SCREW_LEFT:
        if (!_CmdRunning)
        {
            _CmdRunning = msgId;
            ticks = *((INT32 *)pdata);
            send_command(MOTOR_TURN_SCREW_LEFT, sizeof(ticks), &ticks);
            break; 
        }
        break;

    case CMD_ROBI_SCREW_RIGHT:
        if (!_CmdRunning)
        {
            _CmdRunning = msgId;
            ticks = *((INT32 *)pdata);
            send_command(MOTOR_TURN_SCREW_RIGHT, sizeof(ticks), &ticks);
            break; 
        }
        break;

    case CMD_ROBI_MOVE_TO_X:
        if (!_CmdRunning)
        {
            if (!RX_StepperStatus.screwerinfo.ref_done)
            {
                _NewCmd = msgId;
                _Value = *((INT32 *)pdata);
                robi_handle_ctrl_msg(INVALID_SOCKET, CMD_ROBI_REFERENCE, NULL);
                break;
            }
            else if (!(_robiStatus.gpio.inputs & (1UL << SCREW_IN_DOWN)))
            {
                _NewCmd = msgId;
                _Value = *((INT32 *)pdata);
                robi_handle_ctrl_msg(INVALID_SOCKET, CMD_ROBI_MOVE_Z_DOWN,
                                     NULL);
                break;
            }
            else if (RX_StepperStatus.screw_posY - MIN_Y_POS > MAX_VARIANCE)
            {
                Error(ERR_CONT, 0,
                      "Screwer to close to garage to move in x axis");
                break;
            }
            else
            {
                pos = *((INT32 *)pdata);
                _CmdRunning = msgId;
                micron = pos - RX_StepperStatus.screw_posX;
                steps = _micron_2_steps(micron);
                send_command(MOTOR_MOVE_X_RELATIVE, sizeof(steps), &steps);
            }
            break;
        }
        break;

    case CMD_ROBI_MOVE_TO_Y:
        if (!_CmdRunning)
        {
            if (!RX_StepperStatus.screwerinfo.ref_done)
            {
                _NewCmd = msgId;
                _Value = *((INT32 *)pdata);
                robi_handle_ctrl_msg(INVALID_SOCKET, CMD_ROBI_REFERENCE, NULL);
            }
            else if (!(_robiStatus.gpio.inputs & (1UL << SCREW_IN_DOWN)))
            {
                _NewCmd = msgId;
                _Value = *((INT32 *)pdata);
                robi_handle_ctrl_msg(INVALID_SOCKET, CMD_ROBI_MOVE_Z_DOWN, NULL);
            }
            else
            {
                pos = *((INT32 *)pdata);
                if (pos > 0){Error(ERR_CONT, 0, "Unreachable position %d in y-Axis of Robi", pos); break;}
                _CmdRunning = msgId;
                micron = pos - RX_StepperStatus.screw_posY;
                steps = _micron_2_steps(micron);
                send_command(MOTOR_MOVE_Y_RELATIVE, sizeof(steps), &steps);
            }
            break;
        }
        break;

    case CMD_ROBI_MOVE_TO_GARAGE:

        if (!_CmdRunning)
        {
            if (!RX_StepperStatus.screwerinfo.ref_done)
            {
                robi_handle_ctrl_msg(INVALID_SOCKET, CMD_ROBI_REFERENCE, NULL);
            }
            else if (!(_robiStatus.gpio.inputs & (1UL << SCREW_IN_DOWN)))
            {
                _NewCmd = msgId;
                robi_handle_ctrl_msg(INVALID_SOCKET, CMD_ROBI_MOVE_Z_DOWN, NULL);
            }
            else if (abs(RX_StepperStatus.screw_posX) >  1000)
            {
                _NewCmd = msgId;
                pos = 0;
                robi_handle_ctrl_msg(INVALID_SOCKET, CMD_ROBI_MOVE_TO_X, &pos);
            }
            else
            {
                _CmdRunning = msgId;
                pos = 0;
                micron = pos - RX_StepperStatus.screw_posY;
                steps = _micron_2_steps(micron);
                send_command(MOTOR_MOVE_Y_RELATIVE, sizeof(steps), &steps);
                }
            break;
        }
        break;

    default:
        break;
    }
}



static int _motors_move_done()
{
    if (_robiStatus.commandRunning[COMMAND0] ||
        _robiStatus.commandRunning[COMMAND1])
        return FALSE;
    else
        return TRUE;
}

static int32_t set_serial_attributs(int fd, int speed, int parity)
{
	struct termios tty;
	if (tcgetattr(fd, &tty) != REPLY_OK)
		return REPLY_ERROR;
	
	cfsetospeed(&tty, speed);
	cfsetispeed(&tty, speed);
	
	tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8; 
	
	tty.c_iflag &= ~IGNBRK;          // disable break processing
	tty.c_lflag = 0;                 // no signaling chars, no echo, no canonical processing
	tty.c_oflag = 0;                 // no remapping, no delays
	tty.c_cc[VMIN]  = 0;             // read doesn't block
	tty.c_cc[VTIME] = 0;             // 0.5 seconds read timeout
	tty.c_iflag &= ~(IXON | IXOFF | IXANY);  // shut off xon/xoff ctrl
	tty.c_cflag |= (CLOCAL | CREAD); // ignore modem controls,
	tty.c_cflag &= ~(PARENB | PARODD);       // shut off parity
	tty.c_cflag |= parity;
	tty.c_cflag &= ~CSTOPB;
	tty.c_cflag &= ~CRTSCTS;
	if (tcsetattr(fd, TCSANOW, &tty) != REPLY_OK)
		return REPLY_ERROR;
	
	return REPLY_OK;
}

static int32_t send_command(uint32_t commandCode, uint8_t len, void *data)
{
	SUsbTxMsg *pTxMessage;
	
	int32_t nextFifoIndex = (_txFifoInIndex + 1) % ROBI_FIFO_SIZE;
	if (nextFifoIndex == _txFifoOutIndex) 
	{
		term_printf("FIFO Overfilled\n"); 
		return REPLY_ERROR;
	}
	
	_isSync = FALSE;
	_syncMessageId = _msgId;
		
	pTxMessage = &_txFifo[_txFifoInIndex];
	pTxMessage->command = commandCode;
	pTxMessage->id = _msgId++;
	pTxMessage->length = len;
	if (data != NULL && len != 0) memcpy(pTxMessage->data, data, len);
		
	if (_msgId >= ROBI_MAX_MSG_ID)
		_msgId = ROBI_MIN_MSG_ID;
	
	_txFifoInIndex = nextFifoIndex;
	
	return REPLY_OK;
}

static void* send_thread(void *par)
{
	int32_t	length;
	int32_t bufferLength;
	int32_t pendingBytes;
	uint8_t buffer[512];
	SUsbTxMsg *pTxMessage;

	while (_isInit)
	{
		while (_txFifoOutIndex != _txFifoInIndex)
		{
			pTxMessage = &_txFifo[_txFifoOutIndex];
			
			length = sizeof(SUsbTxMsg) - sizeof(pTxMessage->data) + pTxMessage->length;
			comm_encode(pTxMessage, length, buffer, sizeof(buffer), &bufferLength);
			
			write(_usbPort, buffer, bufferLength);
			tcdrain(_usbPort);
			_msgSentCounter++;
			if (pTxMessage->command != STATUS_UPDATE)
				_MsgsSent++;
			
			_txFifoOutIndex = (_txFifoOutIndex + 1) % ROBI_FIFO_SIZE;
			rx_sleep(125);
		}
	}
}

static void* receive_thread(void *par)
{
	int32_t count;
	int32_t length;
	uint8_t buffer[512];
	uint32_t bytesRead;
	SUsbRxMsg rxMessage;
	
	while (_isInit)
	{
		bytesRead = read(_usbPort, buffer, sizeof(buffer));
		
		for (count = 0; count < bytesRead; count++)
		{
			if (comm_received(0, buffer[count]))
			{
				if (comm_get_data(0, (BYTE *)&rxMessage, sizeof(rxMessage), &length))
				{
					if (length == sizeof(rxMessage) - sizeof(rxMessage.data) + rxMessage.length)
					{
						_msgReceivedCounter++;
						if (_isConnected == FALSE)
							_isConnected = TRUE;
					
						_lastMessageTimestamp = rx_get_ticks();
					
						if (_syncMessageId == rxMessage.id)
							_isSync = TRUE;
					
						memcpy(&_robiStatus, &rxMessage.robi, sizeof(_robiStatus));
					}
				}
			}
		}
	}
}

int robi_disabled(void)
{
    return _Robi_Disabled;
}