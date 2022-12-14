#include "comm.h"
#include "rx_threads.h"
#include "rx_term.h"
#include "rx_trace.h"
#include "tcp_ip.h"
#include "rx_error.h"
#include "lb702.h"
#include "lbrob.h"
#include "fpga_stepper.h"

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

#include "robot_client.h"
#include "robi_lb702.h"
#include "robi_def.h"

#define STEPS_PER_REV				51200
#define DISTANCE_UM_PER_REV			36000   // 36000
#define TIME_BEFORE_TURN_SCREWER    2500    // ms

#define ROBI_SHUTOFF_TIME           1000    // ms
#define ROBI_CONNECTION_TIME        3500    // ms

#define MAX_VARIANCE                100     // um

#define MAX_VAR_SCREW_POS           6000    // um

// Turn Directions
#define TURN_LEFT                   -1
#define TURN_0                      0
#define TURN_RIGHT                  1

// Screw Values
#define SCREW_STEPS                 213333
#define SCREW_SEARCH_DIVISOR        3

// Digital Outputs
#define RO_ROBI_POWER               0x800   // o11

static int _steps_2_micron(int steps);
static int _micron_2_steps(int micron);
static void _check_robi_stalled(void);

static uint32_t _MsgsCaptured;


static int _CmdRunning = 0;
static int _NewCmd = 0;
static int _Value = 0;
static int _CmdStarted = FALSE;
static int _TurnDirection = TURN_0;

static int _Search_Screw_Time = 0;
static int _Loose_Screw_Time = 0;

static int _RobiConnection_Time = 0;
static int _ErrorFlag = FALSE;
static int _RobiSteps;
static int _UpNotReached = FALSE;

//--- robi_lb702_init -----------------------------------------------------
void robi_lb702_init(void)
{
    static int _Init=FALSE;
    if (!_Init)
    {
        _Init = TRUE;
        rc_init();
    }
	
    return;
}

//--- robi_lb702_main -----------------------------------------------------------
void robi_lb702_main(int ticks, int menu)
{
    rc_main(ticks, menu);

    /*
    if (!rc_isConnected() && _RobiConnection_Time == 0)
    {
        _RobiConnection_Time = rx_get_ticks();
        Fpga.par->output &= ~RO_ROBI_POWER;
    }
    else if (rx_get_ticks() > _RobiConnection_Time + ROBI_SHUTOFF_TIME && _RobiConnection_Time && rx_get_ticks() <= _RobiConnection_Time + ROBI_CONNECTION_TIME)
    {
        Fpga.par->output |= RO_ROBI_POWER;
    }
    else if (rx_get_ticks() > _RobiConnection_Time + ROBI_CONNECTION_TIME && _RobiConnection_Time)
        _RobiConnection_Time = 0;
    */

    if (_CmdRunning)	RX_StepperStatus.screwerinfo.moving = TRUE;
    else				RX_StepperStatus.screwerinfo.moving = FALSE;


    if (!RX_StepperStatus.screwerinfo.ref_done) _Search_Screw_Time = 0;

    if (RX_StepperStatus.screwerinfo.z_in_down)
        _Loose_Screw_Time = 0;
     
    if (_Loose_Screw_Time && rx_get_ticks() > _Loose_Screw_Time)
    {
        int val = 0;
        if (((RX_StepperStatus.screwerinfo.screwer_blocked_right || (!RX_StepperStatus.screwerinfo.screwer_blocked_left && RX_StepperStatus.screw_posY >= (SCREW_Y_ANGLE + SCREW_Y_STITCH) / 2)) && _TurnDirection > TURN_LEFT) || _TurnDirection == TURN_RIGHT)
		{
            val = -SCREW_STEPS;
            _TurnDirection = TURN_LEFT;
		}
		else
		{
            val = +SCREW_STEPS;
            _TurnDirection = TURN_RIGHT;
		}
            
        rc_turn_ticks(val);
        _Loose_Screw_Time = rx_get_ticks() + TIME_BEFORE_TURN_SCREWER;
    }

    _check_robi_stalled();

    if (!_CmdRunning)
        _CmdStarted = FALSE;
    else if (!rc_move_done())
        _CmdStarted = TRUE;
    
//    if (_CmdRunning && ((robi_move_done() && !rc_isConnected()) || (rc_move_done() && rc_isConnected())) && (_CmdStarted || robi_not_started() || rc_isConnected()))
    if (_CmdRunning && rc_move_done())
    {
        _CmdStarted = FALSE;
        _Search_Screw_Time = 0;
        _Loose_Screw_Time = 0;
        RX_StepperStatus.screwerinfo.moving = FALSE;
        
        switch (_CmdRunning)
        {
        case CMD_ROBI_REFERENCE:
            TrPrintfL(TRUE, "CMD_ROBI_REFERENCE move done, screwer.ref_done=%d, in_garage=%d", RX_StepperStatus.screwerinfo.ref_done, rc_in_garage());
            if (RX_StepperStatus.screwerinfo.ref_done || rc_in_garage() /* runtime condition! */) 
            {
                TrPrintfL(TRUE, "_NewCmd=0x%08x, value=%d", _NewCmd, _Value);
                _CmdRunning = 0;
                if (!rc_in_garage())
                {
                    Error(ERR_CONT, 0, "Robi-Sensor in Garage not high");
                    RX_StepperStatus.screwerinfo.ref_done = FALSE;
                    lb702_reset_variables();
                    lbrob_reset_variables();
                }
                else
                {
                    RX_StepperStatus.screwerinfo.in_garage = TRUE;
                }
            }
            else
            {
                if (!rc_in_garage())
                {
                    Error(ERR_CONT, 0, "Robi-Sensor in Garage not high");
                    lb702_reset_variables();
                    lbrob_reset_variables();
                    _NewCmd = 0;
                    _Value = 0;
                }
                _CmdRunning = 0;
            }
            break;

        case CMD_ROBI_MOVE_TO_GARAGE:
            TrPrintfL(TRUE, "CMD_ROBI_MOVE_TO_GARAGE move done, in_garage=%d", rc_in_garage());
            if (rc_in_garage())
            {
                RX_StepperStatus.screwerinfo.in_garage = TRUE;
                _CmdRunning = 0;
            }
            else rc_reference(0);
            break;

        case CMD_ROBI_MOVE_TO_Y:
            _CmdRunning = 0;            
            break;

        case CMD_ROBI_MOVE_TO_X:
            _CmdRunning = 0;
            break;

        case CMD_ROBI_SCREW_LOOSE:
            if (rc_get_screwer_current())
            {
                rc_set_screwer_current(FALSE);
                RX_StepperStatus.screwerinfo.screwer_blocked_left = FALSE;
            }
            else if (RX_RobiStatus.screwCurrent == TRUE)
            {
                rc_set_screwer_current(FALSE);
                RX_StepperStatus.screwerinfo.screwer_blocked_left = FALSE;
            }
            _CmdRunning = 0;
            break;

        case CMD_ROBI_SCREW_TIGHT:
            if (rc_get_screwer_current())
            {
                rc_set_screwer_current(FALSE);
                RX_StepperStatus.screwerinfo.screwer_blocked_right = FALSE;
            }
            _CmdRunning = 0;
            break;
            
        case CMD_ROBI_MOVE_Y:
        case CMD_ROBI_MOVE_X:
            _CmdRunning = 0;
            break;
        default:
            break;
        }
        if (((RX_StepperStatus.screwerinfo.ref_done || rc_in_garage()) && _NewCmd) || _NewCmd == CMD_ROBI_MOVE_Z_DOWN)
        {
            int loc_new_cmd = _NewCmd;
            int loc_new_value = _Value;
            TrPrintfL(TRUE, "robi_lb702_main call cmd=0x%08x, val=%d", _NewCmd, _Value);
            _NewCmd = 0;

            _Value = 0;
            switch (loc_new_cmd)
            {
            case CMD_ROBI_MOVE_Y:
            case CMD_ROBI_MOVE_X:
            case CMD_ROBI_MOVE_TO_X:
            case CMD_ROBI_MOVE_TO_Y:
                robi_lb702_handle_ctrl_msg(INVALID_SOCKET, loc_new_cmd, &loc_new_value);
                break;
            case CMD_ROBI_MOVE_Z_DOWN:
            case CMD_ROBI_MOVE_Z_UP:
            case CMD_ROBI_MOVE_TO_GARAGE:
                robi_lb702_handle_ctrl_msg(INVALID_SOCKET, loc_new_cmd, NULL);
                break;
            default:
                Error(LOG, 0, "Command %08x not implemented.", loc_new_cmd);
                break;
            }
        }
    }

    return;
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
    int micron, steps, ticks, pos;
    TrPrintfL(TRUE, "robi_lb702_handle_ctrl_msg msgId=0x%08x", msgId);
    switch (msgId)
    {
    case CMD_ROBI_STOP:
        _CmdRunning = 0;
        _Search_Screw_Time = 0;
        _Loose_Screw_Time = 0;
        _NewCmd = FALSE;
        lb702_reset_variables();
        rc_stop();
        break;
    case CMD_ROBI_REFERENCE:
        if (RX_StepperStatus.info.moving || (!RX_StepperStatus.info.z_in_cap && !RX_StepperStatus.info.z_in_wash && !RX_StepperStatus.info.z_in_screw && !RX_StepperStatus.info.z_in_ref) || !RX_StepperStatus.info.ref_done)
        {
            Error(ERR_CONT, 0, "Basket lift is not in position to reference Robi");
            break;
        }
        _CmdRunning = msgId;
        RX_StepperStatus.screwerinfo.moving = TRUE;
        rc_reference(0);
        break;

        /*
    case CMD_ROBI_MOVE_X:
        if (RX_StepperStatus.info.moving || (!RX_StepperStatus.info.z_in_cap && !RX_StepperStatus.info.z_in_wash && !RX_StepperStatus.info.z_in_screw && !RX_StepperStatus.info.z_in_ref) || !RX_StepperStatus.info.ref_done)
        {
            Error(ERR_CONT, 0, "Basket lift is not in position to move Robi in x-Axis");
            break;
        }
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
                Error(ERR_CONT, 0, "Screwer too close to garage to move in x axis");
                break;
            }
            else
            {
                _CmdRunning = msgId;
                RX_StepperStatus.screwerinfo.moving = TRUE;
                _set_moving_variables();
                micron = *((INT32 *)pdata);
                steps = _micron_2_steps(micron);
                robi_move_X_relative_steps(steps);
            }
            break;
        }
        break;
        */
        /*
    case CMD_ROBI_MOVE_Y:
        if (RX_StepperStatus.info.moving || (!RX_StepperStatus.info.z_in_cap && !RX_StepperStatus.info.z_in_wash && !RX_StepperStatus.info.z_in_screw && !RX_StepperStatus.info.z_in_ref) || !RX_StepperStatus.info.ref_done)
        {
            Error(ERR_CONT, 0, "Basket lift is not in position to move Robi in y-Axis");
            break;
        }
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
        */

    case CMD_ROBI_MOVE_Z_UP:
        if (!_CmdRunning)
        {
            if (!RX_StepperStatus.screwerinfo.ref_done)
            {
                _NewCmd = msgId;
                robi_lb702_handle_ctrl_msg(INVALID_SOCKET, CMD_ROBI_REFERENCE, NULL);
                break;
            }
            if (RX_StepperStatus.screw_posY < MIN_Y_POS)
            {
                Error(ERR_CONT, 0, "Screwer too close to garage to move up");
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
            _Search_Screw_Time = rx_get_ticks() + TIME_BEFORE_TURN_SCREWER;
            _RobiSteps = 0;
            rc_move_top(_FL_);
        }
        break;
        
    case CMD_ROBI_MOVE_Z_DOWN:
        if (!_CmdRunning && !RX_StepperStatus.screwerinfo.z_in_down)
        {
            _CmdRunning = msgId;
            RX_StepperStatus.screwerinfo.moving = TRUE;
        //  _Loose_Screw_Time = rx_get_ticks() + TIME_BEFORE_TURN_SCREWER;
            if (RX_StepperStatus.screwerinfo.ref_done)  rc_move_bottom(_FL_);
            else                                        rc_reference(0);
        }
        break;
        

    case CMD_ROBI_SCREW_LOOSE:
        ticks = *((INT32 *)pdata);
        if (!_CmdRunning && ticks)
        {
            if (RX_StepperStatus.screwerinfo.screwer_blocked_left)
            {
                rc_set_screwer_current(TRUE);
            }
            RX_StepperStatus.screwerinfo.moving = TRUE;
            _CmdRunning = msgId;
            rc_turn_ticks_right(ticks);
            _TurnDirection = TURN_RIGHT;
            break; 
        }
        break;

    case CMD_ROBI_MOVE_TO_X:
        if (RX_StepperStatus.info.moving || (!RX_StepperStatus.info.z_in_cap && !RX_StepperStatus.info.z_in_wash && !RX_StepperStatus.info.z_in_screw && !RX_StepperStatus.info.z_in_ref) || !RX_StepperStatus.info.ref_done)
        {
            Error(ERR_CONT, 0, "Basket lift is not in position to move Robi in x-Axis");
            break;
        }
        if (!_CmdRunning)
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
            else if (RX_StepperStatus.screw_posY < MIN_Y_POS && pos != 0)
            {
                Error(ERR_CONT, 0, "Screwer too close to garage to move in x axis");
                break;
            }
            else
            {
                _CmdRunning = msgId;
                RX_StepperStatus.screwerinfo.moving = TRUE;
                rx_sleep(50);
                rc_moveto_x(pos, _FL_);
            }
            break;
        }
        break;

    case CMD_ROBI_MOVE_TO_Y:
        if (RX_StepperStatus.info.moving || (!RX_StepperStatus.info.z_in_cap && !RX_StepperStatus.info.z_in_wash && !RX_StepperStatus.info.z_in_screw && !RX_StepperStatus.info.z_in_ref) || !RX_StepperStatus.info.ref_done)
        {
            Error(ERR_CONT, 0, "Basket lift is not in position to move Robi in y-Axis, Moving: %d, Cap: %d, Wash: %d, Screw: %d, Ref: %d, Ref Done: %d", RX_StepperStatus.info.moving, RX_StepperStatus.info.z_in_cap, RX_StepperStatus.info.z_in_wash, RX_StepperStatus.info.z_in_screw, RX_StepperStatus.info.z_in_ref, RX_StepperStatus.info.ref_done);
            break;
        }
        if (!_CmdRunning || (_CmdRunning == CMD_ROBI_MOVE_Z_UP && !RX_StepperStatus.screwerinfo.z_in_down) || _CmdRunning == CMD_ROBI_MOVE_Z_DOWN)
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
                RX_StepperStatus.screwerinfo.moving = TRUE;
                rx_sleep(100);
                rc_moveto_y(pos, _FL_);
            }
        }
        else
            TrPrintfL(TRUE, "Error: _CmdRunning=0x%08x", _CmdRunning);
        break;

    case CMD_ROBI_MOVE_TO_GARAGE:
        _CmdRunning = msgId;
        lbrob_to_garage();
        break;

    case CMD_ERROR_RESET:
        _ErrorFlag = FALSE;
        break;

    default:
        break;
    }
    return REPLY_OK;
}

//--- _check_robi_stalled -----------------------------------------------------------
static void _check_robi_stalled(void)
{
    if ((_CmdRunning == CMD_ROBI_SCREW_TIGHT || _CmdRunning == CMD_ROBI_SCREW_LOOSE) && ((rc_screwer_stalled() && !rc_get_screwer_current())))
    {
        RX_StepperStatus.screwerinfo.screwer_blocked_left = _CmdRunning == CMD_ROBI_SCREW_TIGHT;
        RX_StepperStatus.screwerinfo.screwer_blocked_right = _CmdRunning == CMD_ROBI_SCREW_LOOSE;
    }
}