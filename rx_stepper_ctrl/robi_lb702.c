#include "comm.h"
#include "rx_threads.h"
#include "rx_term.h"
#include "tcp_ip.h"
#include "rx_error.h"
#include "lb702.h"
#include "lbrob.h"
#include "robi.h"

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/sockios.h>

#include "robi_lb702.h"
#include "robi_def.h"

#define STEPS_PER_REV				51200
#define DISTANCE_UM_PER_REV			36000   // 36000
#define TIME_BEFORE_TURN_SCREWER    2600    // us
#define SCREW_MOVEMENT_CHECK_TIME   1100    // us

#define MIN_Y_POS                   33000

#define MAX_VARIANCE                100     // um

#define MAX_VAR_SCREW_POS           6000    // um

#define WIPE_POS_LEFT               -7600   // um
#define WIPE_POS_RIGHT              4500    // um

static int _steps_2_micron(int steps);
static int _micron_2_steps(int micron);

static void _check_Screwer_Movement();
static void _check_robi_stalled(void);
static void _set_moving_variables(void);


static uint32_t _MsgsCaptured;


static int _CmdRunning = 0;
static int _NewCmd = 0;
static int _Value = 0;
static int _CmdStarted = FALSE;

static int _Search_Screw_Time = 0;
static int _Loose_Screw_Time = 0;

//--- robi_lb702_init -----------------------------------------------------
void robi_lb702_init(void)
{
    RX_StepperStatus.screwerinfo.moving = FALSE;
    RX_StepperStatus.screwerinfo.ref_done = FALSE;

    robi_init();
	
    return;
}

//--- robi_lb702_main -----------------------------------------------------------
void robi_lb702_main(int ticks, int menu)
{

    robi_main(ticks, menu);

    RX_StepperStatus.screwerinfo.z_in_down = RX_RobiStatus.zPos == POS_DOWN;
    RX_StepperStatus.screwerinfo.z_in_up = RX_RobiStatus.zPos == POS_UP;

    if (_CmdRunning)	RX_StepperStatus.screwerinfo.moving = TRUE;
    else				RX_StepperStatus.screwerinfo.moving = FALSE;

    if (RX_StepperStatus.screwerinfo.moving)
    {
        _set_moving_variables();
    }

    RX_StepperStatus.screwerinfo.ref_done = RX_RobiStatus.motors[MOTOR_XY_0].isReferenced && RX_RobiStatus.motors[MOTOR_XY_1].isReferenced && RX_RobiStatus.motors[MOTOR_SCREW].isReferenced;
    
    RX_StepperStatus.screw_posX = (_steps_2_micron(RX_RobiStatus.motors[MOTOR_XY_0].motorEncoderPosition + RX_RobiStatus.motors[MOTOR_XY_1].motorEncoderPosition))/2;
    RX_StepperStatus.screw_posY = (_steps_2_micron(RX_RobiStatus.motors[MOTOR_XY_1].motorEncoderPosition - RX_RobiStatus.motors[MOTOR_XY_0].motorEncoderPosition))/2;

    if (!RX_StepperStatus.screwerinfo.ref_done) _Search_Screw_Time = 0;

    if (RX_StepperStatus.screwerinfo.z_in_up) _Search_Screw_Time = 0;
    if (_Search_Screw_Time && rx_get_ticks() > _Search_Screw_Time)
    {
        int val = 0;
        if (RX_StepperStatus.screw_posY >= (SCREW_Y_BACK + SCREW_Y_FRONT)/2)
            val = 213333;
        else
            val = -213333;
        robi_lb702_handle_ctrl_msg(INVALID_SOCKET, CMD_ROBI_SCREW_STEPS, &val);
        _Search_Screw_Time = rx_get_ticks() + TIME_BEFORE_TURN_SCREWER;
    }

    if (RX_StepperStatus.screwerinfo.z_in_down)
    {
        _Loose_Screw_Time = 0;
    }
    if (_Loose_Screw_Time && rx_get_ticks() > _Loose_Screw_Time)
    {
        int val = 0;
        if (RX_StepperStatus.screwerinfo.screwer_blocked_right || (!RX_StepperStatus.screwerinfo.screwer_blocked_left && RX_StepperStatus.screw_posY >= (SCREW_Y_BACK + SCREW_Y_FRONT) / 2))
            val = -213333;
        else
            val = +213333;
            
        robi_lb702_handle_ctrl_msg(INVALID_SOCKET, CMD_ROBI_SCREW_STEPS, &val);
        _Loose_Screw_Time = 0;
    }

    _check_robi_stalled();
    _check_Screwer_Movement();
    

    if (!robi_move_done())
        _CmdStarted = TRUE;
    else if (!_CmdRunning)
        _CmdStarted = FALSE;

    if (_CmdRunning && robi_move_done() && _CmdStarted)
    {
        int loc_new_cmd, loc_new_value;
        int val;
        uint8_t current;

        _CmdStarted = FALSE;
        _Search_Screw_Time = 0;
        _Loose_Screw_Time = 0;
        RX_StepperStatus.screwerinfo.moving = FALSE;
        
        switch (_CmdRunning)
        {
        case CMD_ROBI_REFERENCE:
            if (RX_StepperStatus.screwerinfo.ref_done)
            {
                _CmdRunning = 0;
                if (!robi_in_ref())
                {
                    Error(ERR_CONT, 0, "Robi-Sensor in Garage not high");
                    RX_StepperStatus.screwerinfo.ref_done = FALSE;
                }
                else
                {
                    RX_StepperStatus.screwerinfo.y_in_ref = TRUE;
                }
            }
            else
            {
                if (!robi_connected()) Error(ERR_CONT, 0, "Connection lost during reference");
                if (!robi_in_ref()) Error(ERR_CONT, 0, "Robi-Sensor in Garage not high");
                _CmdRunning = 0;
                _NewCmd = 0;
                _Value = 0;
            }
            break;

        case CMD_ROBI_MOVE_TO_GARAGE:
            if (!robi_in_ref())
            {
                Error(ERR_CONT, 0, "Robi-Sensor in Garage not high");
                RX_StepperStatus.screwerinfo.ref_done = FALSE;
            }
            else
            {
                RX_StepperStatus.screwerinfo.y_in_ref = TRUE;
            }
            _CmdRunning = 0;
            break;

        case CMD_ROBI_MOVE_TO_Y:
            if (abs(RX_StepperStatus.screw_posY - SCREW_Y_BACK) < MAX_VAR_SCREW_POS || abs(RX_StepperStatus.screw_posY - SCREW_Y_FRONT) < MAX_VAR_SCREW_POS)
                RX_StepperStatus.screwerinfo.y_in_pos = TRUE;
            _CmdRunning = 0;
            
            break;

        case CMD_ROBI_MOVE_TO_X:
            if (abs(RX_StepperStatus.screw_posX - SCREW_X_LEFT) < MAX_VAR_SCREW_POS || abs(RX_StepperStatus.screw_posX - SCREW_X_RIGHT) <  MAX_VAR_SCREW_POS)
                RX_StepperStatus.screwerinfo.x_in_pos = TRUE;
            _CmdRunning = 0;
            break;

        case CMD_ROBI_SCREW_RIGHT:
            if (RX_RobiStatus.screwCurrent == TRUE)
            {
                robi_set_screw_current(FALSE);
                _NewCmd = CMD_ROBI_MOVE_Z_DOWN;
                RX_StepperStatus.screwerinfo.screwer_blocked_left = FALSE;
            }
            else if (RX_StepperStatus.screw_posY < (SCREW_Y_BACK + SCREW_Y_FRONT) / 2)
                RX_StepperStatus.screwerinfo.screw_tight = TRUE;
            else if (RX_StepperStatus.screw_posY > (SCREW_Y_BACK + SCREW_Y_FRONT) / 2)
                RX_StepperStatus.screwerinfo.screw_loosed = TRUE;
            _CmdRunning = 0;
            break;
            
        case CMD_ROBI_SCREW_LEFT:
            if (RX_RobiStatus.screwCurrent == TRUE)
            {
                robi_set_screw_current(FALSE);
                _NewCmd = CMD_ROBI_MOVE_Z_DOWN;
                RX_StepperStatus.screwerinfo.screwer_blocked_right = FALSE;
            }
            else if (RX_StepperStatus.screw_posY > (SCREW_Y_BACK + SCREW_Y_FRONT) / 2)
                RX_StepperStatus.screwerinfo.screw_tight = TRUE;
            else if (RX_StepperStatus.screw_posY < (SCREW_Y_BACK + SCREW_Y_FRONT) / 2)
                RX_StepperStatus.screwerinfo.screw_loosed = TRUE;
            _CmdRunning = 0;
            break;

        case CMD_ROBI_WIPE_LEFT:
            RX_StepperStatus.screwerinfo.wipe_left_up = TRUE;
            _CmdRunning = 0;
            break;
        case CMD_ROBI_WIPE_RIGHT:
            RX_StepperStatus.screwerinfo.wipe_right_up = TRUE;
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
        if (RX_StepperStatus.screwerinfo.ref_done)
        {
            RX_StepperStatus.screwerinfo.screwer_blocked_left = FALSE;
            RX_StepperStatus.screwerinfo.screwer_blocked_right = FALSE;
        }
        if ((RX_StepperStatus.screwerinfo.ref_done && _NewCmd) || _NewCmd == CMD_ROBI_MOVE_Z_DOWN)
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
            case CMD_ROBI_WIPE_LEFT:
            case CMD_ROBI_WIPE_RIGHT:
                robi_lb702_handle_ctrl_msg(INVALID_SOCKET, loc_new_cmd, &loc_new_value);
                break;
            case CMD_ROBI_MOVE_Z_DOWN:
            case CMD_ROBI_MOVE_Z_UP:
            case CMD_ROBI_MOVE_TO_GARAGE:
                robi_lb702_handle_ctrl_msg(INVALID_SOCKET, loc_new_cmd, NULL);
                break;
            default:
                Error(LOG, 0, "Command %d not implemented.", loc_new_cmd);
                break;
            }
        }
    }

    return;
}


//--- robi_lb702_menu -------------------------------------------------------------
void robi_lb702_menu(int help)
{
	if (robi_is_init() == FALSE)
		return;
	
	if (help)
	{
		term_printf("Robi LB702 MENU ------------------------- (3)\n");
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

//--- robi_lb702_handle_menu ---------------------------------------------------------
void robi_lb702_handle_menu(char *str)
{
    int num = 0;
    int val = 0;
    uint8_t current = 0;

    if (robi_is_init() == FALSE)
		return;
	
	_MsgsCaptured++;
	
	switch (str[0])
	{
	case 's': robi_lb702_handle_ctrl_msg(INVALID_SOCKET, CMD_ROBI_STOP, NULL); break;
	case 'o':
        num = str[1] - '0';
        val = str[2] - '0';
        robi_set_output(num, val);
		break;
	case 'R':
        robi_lb702_handle_ctrl_msg(INVALID_SOCKET, CMD_ROBI_REFERENCE, NULL); break;
	case 'v':
        val = atoi(&str[1]);
        robi_lb702_handle_ctrl_msg(INVALID_SOCKET, CMD_ROBI_MOVE_X, &val); break;
	case 'w':
        val = atoi(&str[1]);
        robi_lb702_handle_ctrl_msg(INVALID_SOCKET, CMD_ROBI_MOVE_Y, &val); break;
	case 't':
        val = atoi(&str[1]);
        robi_lb702_handle_ctrl_msg(INVALID_SOCKET, CMD_ROBI_SCREW_STEPS, &val);
        break;
	case 'l':
        val = atoi(&str[1]);
        robi_lb702_handle_ctrl_msg(INVALID_SOCKET, CMD_ROBI_SCREW_LEFT, &val);
        break;
    case 'r':
        val = atoi(&str[1]);
        robi_lb702_handle_ctrl_msg(INVALID_SOCKET, CMD_ROBI_SCREW_RIGHT, &val);
        break;
	case 'u':
        robi_move_up(); break;
	case 'd': 
        robi_lb702_handle_ctrl_msg(INVALID_SOCKET, CMD_ROBI_MOVE_Z_DOWN, NULL); break;
    case 'e':
        val = atoi(&str[1]);
        robi_lb702_handle_ctrl_msg(INVALID_SOCKET, CMD_ROBI_MOVE_TO_X, &val);
        break;
        
    case 'f':
        val = atoi(&str[1]);
        robi_lb702_handle_ctrl_msg(INVALID_SOCKET, CMD_ROBI_MOVE_TO_Y, &val);
        break;
        
    case 'g':
        robi_lb702_handle_ctrl_msg(INVALID_SOCKET, CMD_ROBI_MOVE_TO_GARAGE, NULL);
        break;
    case 'c':
        current = atoi(&str[1]);
        robi_set_screw_current(current);
        break;
    case 'a':
        val = 1;
        robi_lb702_handle_ctrl_msg(INVALID_SOCKET, CMD_ROBI_WIPE_LEFT, &val);
        break;
    case 'b':
        val = 1;
        robi_lb702_handle_ctrl_msg(INVALID_SOCKET, CMD_ROBI_WIPE_RIGHT, &val);
        break;
    default:
        break;
	}

    return;
}

//--- robi_lb702_display_status -----------------------------------------------------------
void robi_lb702_display_status(void)
{
	uint32_t i;
	
	if (robi_is_init() == FALSE)
		return;
	
	term_printf("\n");
	term_printf("Robi system status ---------------------------------\n");
	term_printf("Connection: %d, Updating: %d, CurrentVersion: %d, BoardVersion: %d, BootloaderStatus: %d\n", robi_connected(), robi_is_updating(), robi_current_version(), RX_RobiStatus.version, RX_RobiStatus.bootloaderStatus);
    term_printf("moving: \t\t %d \t cmd: %08x\n", RX_StepperStatus.screwerinfo.moving, _CmdRunning);
	term_printf("Robi gpio status -----------------------------------\n");
	term_printf("Robi Inputs: ");
	for (i = 0; i < INPUT_COUNT; i++)
	{
        if (RX_RobiStatus.gpio.inputs & (1UL << i))	term_printf("*");
		else										term_printf("_");
		if (i % 4 == 3)			   					term_printf("   ");
	}
	term_printf("\n");
	term_printf("Robi Outputs: ");
	for (i = 0; i < OUTPUT_COUNT; i++)
	{
        if (RX_RobiStatus.gpio.outputs & (1UL << i))	term_printf("*");
		else										    term_printf("_");
		if (i % 4 == 3)			   					    term_printf("   ");
	}
	term_printf("\n");
	term_printf("Robi motor status ----------------------------------\n");
	term_printf("MOTOR_XY_0:\tMotor pos.: %10d Motor target pos.: %10d Encoder pos.: %10d\n", 
		_steps_2_micron(RX_RobiStatus.motors[MOTOR_XY_0].motorPosition) , 
		_steps_2_micron(RX_RobiStatus.motors[MOTOR_XY_0].motorTargetPosition), 
		_steps_2_micron(RX_RobiStatus.motors[MOTOR_XY_0].motorEncoderPosition));
	term_printf("MOTOR_XY_1:\tMotor pos.: %10d Motor target pos.: %10d Encoder pos.: %10d\n", 
		_steps_2_micron(RX_RobiStatus.motors[MOTOR_XY_1].motorPosition), 
		_steps_2_micron(RX_RobiStatus.motors[MOTOR_XY_1].motorTargetPosition), 
		_steps_2_micron(RX_RobiStatus.motors[MOTOR_XY_1].motorEncoderPosition));
	term_printf("MOTOR_SCREW:\tMotor pos.: %10d Motor target pos.: %10d\n", 
		RX_RobiStatus.motors[MOTOR_SCREW].motorPosition, 
		RX_RobiStatus.motors[MOTOR_SCREW].motorTargetPosition);
    term_printf("MOTOR_Z:\tMotor pos.: %10d Motor target pos.: %10d\n",
                RX_RobiStatus.motors[MOTOR_Z].motorPosition,
                RX_RobiStatus.motors[MOTOR_Z].motorTargetPosition);
    term_printf("Screwer-Current: \t %d\n", RX_RobiStatus.screwCurrent);
    term_printf("Screwer X-Pos: \t\t %d\n", RX_StepperStatus.screw_posX);
    term_printf("Screwer Y-Pos: \t\t %d\n", RX_StepperStatus.screw_posY);

    term_printf("MOTOR_XY_0 ref: \t %d\n", RX_RobiStatus.motors[MOTOR_XY_0].isReferenced);
	term_printf("MOTOR_XY_1 ref: \t %d\n", RX_RobiStatus.motors[MOTOR_XY_1].isReferenced);
	term_printf("MOTOR_SCREW ref: \t %d\n", RX_RobiStatus.motors[MOTOR_SCREW].isReferenced);
    term_printf("Z position: \t\t %d\n", RX_RobiStatus.zPos);
	term_printf("Robi position status -------------------------------\n");
    term_printf("Garage: \t\t %d\n", RX_RobiStatus.isInGarage);
    term_printf("Ref: \t\t\t %d\n", RX_RobiStatus.isInRef);
    term_printf("Robi blocked left: \t %d\n", RX_StepperStatus.screwerinfo.screwer_blocked_left);
    term_printf("Robi blocked right: \t %d\n", RX_StepperStatus.screwerinfo.screwer_blocked_right);
    term_printf("Robi screwed: \t\t %d\n", RX_StepperStatus.screwerinfo.screwed);
    term_printf("Screwer ready: \t\t %d\n", RX_StepperStatus.screwerinfo.screwer_ready);
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

//--- robi_lb702_handle_ctrl_msg ----------------------------------------------
int robi_lb702_handle_ctrl_msg(RX_SOCKET socket, int msgId, void *pdata)
{
    if (robi_is_init() == FALSE || robi_connected() == FALSE)
    {
        if (!robi_is_init()) Error(ERR_CONT, 0, "Robi is not initalized");
        if (!robi_connected()) Error(ERR_CONT, 0, "Robi is not connected");
        lb702_reset_variables();
        lbrob_reset_variables();
        return REPLY_ERROR;
    }

    int micron, steps, ticks, pos;
    switch (msgId)
    {
    case CMD_ROBI_STOP:
        _CmdRunning = 0;
        _Search_Screw_Time = 0;
        _Loose_Screw_Time = 0;
        RX_StepperStatus.screwerinfo.y_in_ref = FALSE;
        lb702_reset_variables();
        robi_stop();
        break;
    case CMD_ROBI_REFERENCE:
            _CmdRunning = msgId;
            RX_StepperStatus.screwerinfo.moving = TRUE;
            _set_moving_variables();
            robi_reference();
            break;
        break;
    case CMD_ROBI_MOVE_X:
        if (!_CmdRunning)
        {
            if (!RX_StepperStatus.screwerinfo.ref_done)
            {
                _NewCmd = msgId;
                _Value = *((INT32 *)pdata);
                robi_lb702_handle_ctrl_msg(INVALID_SOCKET, CMD_ROBI_REFERENCE, NULL);
                break;
            }
            if (!RX_StepperStatus.screwerinfo.z_in_down)
            {
                _NewCmd = msgId;
                _Value = *((INT32 *)pdata);
                robi_lb702_handle_ctrl_msg(INVALID_SOCKET, CMD_ROBI_MOVE_Z_DOWN, NULL);
            }
            else if (RX_StepperStatus.screw_posY < MIN_Y_POS)
            {
                Error(ERR_CONT, 0, "Screwer to close to garage to move in x axis");
                break;
            }
            else
            {
                _CmdRunning = msgId;
                RX_StepperStatus.screwerinfo.moving = TRUE;
                _set_moving_variables();
                micron = *((INT32 *)pdata);
                steps = _micron_2_steps(micron);
                robi_move_x_relative_steps(steps);
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
                robi_lb702_handle_ctrl_msg(INVALID_SOCKET, CMD_ROBI_REFERENCE, NULL);
                break;
            }
            if (!RX_StepperStatus.screwerinfo.z_in_down)
            {
                _NewCmd = msgId;
                _Value = *((INT32 *)pdata);
                robi_lb702_handle_ctrl_msg(INVALID_SOCKET, CMD_ROBI_MOVE_Z_DOWN, NULL);
                break;
            }
            else
            {
                _CmdRunning = msgId;
                RX_StepperStatus.screwerinfo.moving = TRUE;
                _set_moving_variables();
                micron = *((INT32 *)pdata);
                steps = _micron_2_steps(micron);
                robi_move_Y_relative_steps(steps);
            }
            break;
        }
        break;

    case CMD_ROBI_MOVE_Z_UP:
        if (!_CmdRunning)
        {
            if (!RX_StepperStatus.screwerinfo.ref_done)
            {
                _NewCmd = msgId;
                robi_lb702_handle_ctrl_msg(INVALID_SOCKET, CMD_ROBI_REFERENCE, NULL);
                break;
            }
            if (RX_StepperStatus.screw_posY < MIN_Y_POS && _NewCmd != CMD_ROBI_WIPE_LEFT && _NewCmd != CMD_ROBI_WIPE_RIGHT)
            {
                Error(ERR_CONT, 0, "Screwer to close to garage to move up");
                break;
            }
            if (RX_StepperStatus.screwerinfo.z_in_up)
            {
                Error(LOG, 0, "Screwer already in up pos");
                break;
            }
            if (abs(RX_StepperStatus.screw_posX - SCREW_X_LEFT) > MAX_VAR_SCREW_POS && abs(RX_StepperStatus.screw_posX - SCREW_X_RIGHT) > MAX_VAR_SCREW_POS)
            {
                Error(ERR_CONT, 0, "Screw not in position to lift up");
                break;
            }
            _CmdRunning = msgId;
            RX_StepperStatus.screwerinfo.moving = TRUE;
            _set_moving_variables();
            _Search_Screw_Time = rx_get_ticks() + TIME_BEFORE_TURN_SCREWER;
            robi_move_up();
        }
        break;
        
    case CMD_ROBI_MOVE_Z_DOWN:
        if (!_CmdRunning)
        {
            if (RX_StepperStatus.screwerinfo.z_in_down) {Error(ERR_CONT, 0, "Screwer is already down"); break;}
            _CmdRunning = msgId;
            RX_StepperStatus.screwerinfo.moving = TRUE;
            _set_moving_variables();
            _Loose_Screw_Time = rx_get_ticks() + TIME_BEFORE_TURN_SCREWER;
            robi_move_down();
        }
        break;
        
    case CMD_ROBI_SCREW_STEPS:
        steps = *((INT32 *)pdata);
        robi_turn_screw_relative(steps);
        break;
        
    case CMD_ROBI_SCREW_LEFT:
        if (!_CmdRunning)
        {
            RX_StepperStatus.screwerinfo.moving = TRUE;
            _set_moving_variables();
            _CmdRunning = msgId;
            ticks = *((INT32 *)pdata);
            robi_turn_screw_left(ticks);
            break; 
        }
        break;

    case CMD_ROBI_SCREW_RIGHT:
        if (!_CmdRunning)
        {
            RX_StepperStatus.screwerinfo.moving = TRUE;
            _set_moving_variables();
            _CmdRunning = msgId;
            ticks = *((INT32 *)pdata);
            robi_turn_screw_right(ticks);
            break; 
        }
        break;

    case CMD_ROBI_MOVE_TO_X:
        if (!_CmdRunning || _CmdRunning == CMD_ROBI_WIPE_LEFT || _CmdRunning == CMD_ROBI_WIPE_RIGHT)
        {
            pos = *((INT32 *)pdata);
            if (!RX_StepperStatus.screwerinfo.ref_done)
            {
                _NewCmd = msgId;
                _Value = *((INT32 *)pdata);
                robi_lb702_handle_ctrl_msg(INVALID_SOCKET, CMD_ROBI_REFERENCE, NULL);
                break;
            }
            else if (!RX_StepperStatus.screwerinfo.z_in_down)
            {
                _NewCmd = msgId;
                _Value = *((INT32 *)pdata);
                robi_lb702_handle_ctrl_msg(INVALID_SOCKET, CMD_ROBI_MOVE_Z_DOWN, NULL);
                break;
            }
            else if (RX_StepperStatus.screw_posY < MIN_Y_POS && _CmdRunning != CMD_ROBI_WIPE_LEFT && _CmdRunning != CMD_ROBI_WIPE_RIGHT && pos != 0)
            {
                Error(ERR_CONT, 0, "Screwer to close to garage to move in x axis");
                break;
            }
            else
            {
                if ( _CmdRunning != CMD_ROBI_WIPE_LEFT && _CmdRunning != CMD_ROBI_WIPE_RIGHT) _CmdRunning = msgId;
                RX_StepperStatus.screwerinfo.moving = TRUE;
                micron = pos - RX_StepperStatus.screw_posX;
                steps = _micron_2_steps(micron);
                robi_move_x_relative_steps(steps);
            }
            break;
        }
        break;

    case CMD_ROBI_MOVE_TO_Y:
        if (!_CmdRunning || (_CmdRunning == CMD_ROBI_MOVE_Z_UP && !RX_StepperStatus.screwerinfo.z_in_down))
        {
            if (!RX_StepperStatus.screwerinfo.ref_done)
            {
                _NewCmd = msgId;
                _Value = *((INT32 *)pdata);
                robi_lb702_handle_ctrl_msg(INVALID_SOCKET, CMD_ROBI_REFERENCE, NULL);
            }
            else if (!RX_StepperStatus.screwerinfo.z_in_down)
            {
                _NewCmd = msgId;
                _Value = *((INT32 *)pdata);
                robi_lb702_handle_ctrl_msg(INVALID_SOCKET, CMD_ROBI_MOVE_Z_DOWN, NULL);
            }
            else
            {
                pos = *((INT32 *)pdata);
                if (pos < 0)
                {
                    Error(ERR_CONT, 0, "Unreachable position %d in y-Axis of Robi", pos); 
                    break;
                }
                _CmdRunning = msgId;
                _set_moving_variables();
                RX_StepperStatus.screwerinfo.moving = TRUE;
                micron = pos - RX_StepperStatus.screw_posY;
                steps = _micron_2_steps(micron);
                robi_move_Y_relative_steps(steps);
            }
        }
        break;

    case CMD_ROBI_MOVE_TO_GARAGE:

        if (!_CmdRunning)
        {
            if (!RX_StepperStatus.screwerinfo.ref_done)
                robi_lb702_handle_ctrl_msg(INVALID_SOCKET, CMD_ROBI_REFERENCE, NULL);
            else if (!RX_StepperStatus.screwerinfo.z_in_down)
            {
                _NewCmd = msgId;
                robi_lb702_handle_ctrl_msg(INVALID_SOCKET, CMD_ROBI_MOVE_Z_DOWN, NULL);
            }
            else if (abs(RX_StepperStatus.screw_posX) >  1000)
            {
                _NewCmd = msgId;
                pos = 0;
                robi_lb702_handle_ctrl_msg(INVALID_SOCKET, CMD_ROBI_MOVE_TO_X, &pos);
            }
            else
            {
                _CmdRunning = msgId;
                RX_StepperStatus.screwerinfo.moving = TRUE;
                _set_moving_variables();
                pos = 0;
                micron = pos - RX_StepperStatus.screw_posY;
                steps = _micron_2_steps(micron);
                robi_move_Y_relative_steps(steps);
            }
        }
        break;

    case CMD_ROBI_WIPE_LEFT:
        pos = *((INT32 *)pdata);
        if (pos == 0 && !RX_StepperStatus.screwerinfo.wipe_left_up && !RX_StepperStatus.screwerinfo.wipe_right_up) break;
        if (!_CmdRunning)
        {
            if (!RX_StepperStatus.screwerinfo.ref_done)
            {
                _NewCmd = msgId;
                _Value = pos;
                robi_lb702_handle_ctrl_msg(INVALID_SOCKET, CMD_ROBI_REFERENCE, NULL);
            }
            else if (!RX_StepperStatus.screwerinfo.z_in_down)
            {
                _NewCmd = msgId;
                _Value = pos;
                robi_lb702_handle_ctrl_msg(INVALID_SOCKET, CMD_ROBI_MOVE_Z_DOWN, NULL);
            }
            else if (abs(RX_StepperStatus.screw_posY) > 700)
            {
                _NewCmd = msgId;
                _Value = pos;
                pos = 0;
                if (abs(RX_StepperStatus.screw_posX) > MAX_VARIANCE)
                    robi_lb702_handle_ctrl_msg(INVALID_SOCKET, CMD_ROBI_MOVE_TO_X, &pos);
                else
                    robi_lb702_handle_ctrl_msg(INVALID_SOCKET, CMD_ROBI_MOVE_Y, &pos);
            }
            else
            {
                _CmdRunning = msgId;
                RX_StepperStatus.screwerinfo.moving = TRUE;
                _set_moving_variables();
                if (pos) pos = WIPE_POS_LEFT;
                robi_lb702_handle_ctrl_msg(INVALID_SOCKET, CMD_ROBI_MOVE_TO_X, &pos);
            }
            
        }
        break;

    case CMD_ROBI_WIPE_RIGHT:
        pos = *((INT32 *)pdata);
        if (pos == 0 && !RX_StepperStatus.screwerinfo.wipe_left_up && !RX_StepperStatus.screwerinfo.wipe_right_up) break;
        if (!_CmdRunning)
        {
            if (!RX_StepperStatus.screwerinfo.ref_done)
            {
                _NewCmd = msgId;
                _Value = pos;
                robi_lb702_handle_ctrl_msg(INVALID_SOCKET, CMD_ROBI_REFERENCE, NULL);
            }
            else if (!RX_StepperStatus.screwerinfo.z_in_down)
            {
                _NewCmd = msgId;
                _Value = pos;
                robi_lb702_handle_ctrl_msg(INVALID_SOCKET, CMD_ROBI_MOVE_Z_DOWN, NULL);
            }
            else if (abs(RX_StepperStatus.screw_posY) > 700)
            {
                _NewCmd = msgId;
                _Value = pos;
                pos = 0;
                if (abs(RX_StepperStatus.screw_posX) > MAX_VARIANCE)
                    robi_lb702_handle_ctrl_msg(INVALID_SOCKET, CMD_ROBI_MOVE_TO_X, &pos);
                else
                    robi_lb702_handle_ctrl_msg(INVALID_SOCKET, CMD_ROBI_MOVE_Y, &pos);
            }
            else
            {
                _CmdRunning = msgId;
                RX_StepperStatus.screwerinfo.moving = TRUE;
                _set_moving_variables();
                if (pos) pos = WIPE_POS_RIGHT;
                robi_lb702_handle_ctrl_msg(INVALID_SOCKET, CMD_ROBI_MOVE_TO_X, &pos);
            }        
        }
        break;

    default:
        break;
    }
}


//--- _check_Screwer_Movement -----------------------------------------------------------
static void _check_Screwer_Movement()
{
    int ticks;
    static int _oldScrewState = 0;
    
    if ((RX_StepperStatus.screwerinfo.screwer_blocked_left || RX_StepperStatus.screwerinfo.screwer_blocked_right) && robi_move_done() && _NewCmd && !_CmdRunning)
    {
        if ((RX_StepperStatus.screwerinfo.screwer_blocked_right &&  RX_StepperStatus.screw_posY >= (SCREW_Y_BACK + SCREW_Y_FRONT) / 2) ||
                (RX_StepperStatus.screwerinfo.screwer_blocked_left && RX_StepperStatus.screw_posY <= (SCREW_Y_BACK + SCREW_Y_FRONT) / 2))
            ticks = 15;
        else
            ticks = 3;
        robi_set_screw_current(TRUE);
        robi_lb702_handle_ctrl_msg(INVALID_SOCKET, _NewCmd, &ticks);
    }
}

//--- _check_robi_stalled -----------------------------------------------------------
static void _check_robi_stalled(void)
{
    if ((_CmdRunning == CMD_ROBI_SCREW_LEFT || _CmdRunning == CMD_ROBI_SCREW_RIGHT) && RX_RobiStatus.screwCurrent == 0 && robi_screwer_stalled())
    {
        RX_StepperStatus.screwerinfo.screwer_blocked_left = _CmdRunning == CMD_ROBI_SCREW_LEFT;
        RX_StepperStatus.screwerinfo.screwer_blocked_right = _CmdRunning == CMD_ROBI_SCREW_RIGHT;
        if (_CmdRunning == CMD_ROBI_SCREW_LEFT)
            _NewCmd = CMD_ROBI_SCREW_RIGHT;
        else if (_CmdRunning == CMD_ROBI_SCREW_RIGHT)
            _NewCmd = CMD_ROBI_SCREW_LEFT;
    }
}

static void _set_moving_variables(void)
{
    RX_StepperStatus.screwerinfo.robi_in_ref = FALSE;
    RX_StepperStatus.screwerinfo.x_in_pos = FALSE;
    RX_StepperStatus.screwerinfo.y_in_pos = FALSE;
    RX_StepperStatus.screwerinfo.y_in_ref = FALSE;
    RX_StepperStatus.screwerinfo.screw_loosed = FALSE;
    RX_StepperStatus.screwerinfo.screw_tight = FALSE;
    RX_StepperStatus.screwerinfo.wipe_left_up = FALSE;
    RX_StepperStatus.screwerinfo.wipe_right_up = FALSE;
}
