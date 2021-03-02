// ****************************************************************************
//
//	DIGITAL PRINTING - lbrob.c
//
//	Communication between ARM processor and FPGA
//
// ****************************************************************************
//
//	Copyright 2016 by radex AG, Switzerland. All rights reserved.
//	Written by Marcel Galliker
//
// ****************************************************************************

#include "rx_error.h"
#include "rx_term.h"
#include "tcp_ip.h"
#include "fpga_stepper.h"
#include "power_step.h"
#include "motor_cfg.h"
#include "motor.h"
#include "stepper_ctrl.h"
#include "lbrob.h"
#include "rx_def.h"
#include "robi_def.h"
#include "lb702.h"
#include "robi_lb702.h"

#define MOTOR_X_0               4

#define MOTOR_X_BITS            0x10
#define MOTOR_ALL_BITS          0x13

#define X_STEPS_PER_REV         3200.0
#define X_INC_PER_REV           16000.0
#define X_DIST_PER_REV          54000   // 36000

#define CABLE_CAP_POS           -690000     //	um LB702
#define CABLE_WASH_POS_FRONT    -634000     //	um LB702
#define CABLE_WASH_POS_BACK     -221000     //	um LB702
#define CABLE_PURGE_POS_BACK    -311000     //  um LB702
#define VACUUM_POS              -400000     //  um LB702
#define CABLE_PURGE_POS_FRONT   -622000     //  um LB702    CABLE_PURGE_POS_BACK - (7 * HEAD_WIDTH) - 10000 ->  HEAD_WIDTH = 43000
#define CABLE_SCREW_POS_FRONT   -503000     //  um LB702    
#define CABLE_SCREW_POS_BACK    -156208     //  um LB702    CABLE_SCREW_POS_BACK + (8 * HEAD_WIDTH) ->  HEAD_WIDTH = 43349

#define CURRENT_HOLD            200

#define CAP_FILL_TIME           16000       // ms
#define SCREW_SEARCHING_TIME    5000        // ms
#define VACUUM_PUMP_TIME        25000       // ms
#define WASTE_PUMP_TIME         30000       // ms

// Digital Inputs
#define CABLE_PULL_REF          2
#define CAPPING_ENDSTOP         3
#define DI_INK_PUMP_REED_LEFT   4
#define DI_INK_PUMP_REED_RIGHT  5

// Digital Outputs
#define RO_ALL_OUTPUTS          0x02F       // All used outputs -> o0, o1, o2, o3, o5
#define RO_FLUSH_WIPE_RIGHT     0x001       // o0
#define RO_FLUSH_WIPE_LEFT      0x002       // o1
#define RO_FLUSH_WIPE           0x003       // o0 + o1
#define RO_FLUSH_TO_CAP_RIGHT   0x004       // o2
#define RO_FLUSH_TO_CAP_LEFT    0x008       // o3
#define RO_FLUSH_TO_CAP         0x00c       // o2 + o3
#define RO_VACUUM_CLEANER       0x010       // o4
#define RO_FLUSH_PUMP           0x020       // o5
#define RO_WASTE_VAC            0x040       // o6
#define RO_INK_VALVE_LEFT       0x080       // o7
#define RO_INK_VALVE_RIGHT      0x100       // o8
#define RO_INK_VALVE_BOTH       0x180       // o7 + o8
#define RO_INK_PUMP_LEFT        0x200       // o9
#define RO_INK_PUMP_RIGHT       0x400       // o10

// PWM Signals
#define PWM_INK_PUMP_LEFT       3
#define PWM_INK_PUMP_RIGHT      4

// Analogue Inputs
#define AI_INK_PUMP_LEFT        2
#define AI_INK_PUMP_RIGHT       3

#define MAX_POS_DIFFERENT       4000        // steps

#define HEAD_WIDTH              43000

#define HEADS_PER_COLOR         8
#define COLORS_PER_STEPPER      2


#define MAX_VAR_SCREW_POS       2000        // um

// Times
#define FOLLOW_UP_TIME_INK_BACK 20000       // ms


// globals
static int _CmdRunning_Lift = 0;
static int _CmdRunning_Robi = 0;
static int _ScrewTime = 0;


// static
static SMovePar _ParCable_ref;
static SMovePar _ParCable_drive;
static SMovePar _ParCable_drive_slow;
static SMovePar _ParCable_drive_purge;

static char     *_MotorName[5] = {"BACK", "FRONT", "NONE", "NON", "SLEDGE"};
static char     _CmdName[32];
static int      _CmdRunning = 0;
static int      _NewCmd = 0;
static EWipeSide _WipeSide = wipe_none;

static ERobotFunctions _RobFunction = 0;
static ERobotFunctions _Old_RobFunction = 0;
static SHeadAdjustment _HeadAdjustment = {0, 0, 0, 0};
static int _CmdScrewing = 0;
static int _CmdSearchScrews = 0;
static int _CmdResetScrews = 0;
static int _SearchScrewNr = 0;
static int _ResetScrewNr = 0;
static int _ResetScrewsLeft = FALSE;
static int _ResetScrewsRight = FALSE;
static int _ResetInfinity = 0;
static int _Turns = 0;

static int _HeadScrewPos = 0;

static int _CmdRunning_old = 0;

// Timers
static int _CapFillTime = 0;
static int _TimeSearchScrew = 0;
static int _WastePumpTimer = 0;

static int _PumpValueLeft = 0;
static int _PumpValueRight = 0;
static int _PumpTimeLeft = 0;
static int _PumpTimeRight = 0;

static int _PumpSpeed = 100;


//--- prototypes --------------------------------------------
static void _lbrob_motor_z_test(int steps);
static void _lbrob_motor_test(int motor, int steps);
static void _lbrob_do_reference(void);
static void _lbrob_move_to_pos_slow(int cmd, int pos);
static void _lbrob_move_to_pos(int cmd, int pos);
static int  _micron_2_steps(int micron);
static int  _steps_2_micron(int steps);
static void _cln_move_to(int msgId, ERobotFunctions fct);
static void _rob_wipe(int msgId, EWipeSide side);
static void _turn_screw(SHeadAdjustment headAdjustment);
static int  _check_in_screw_pos(SHeadAdjustment headAdjustment);
static void _search_all_screws();
static void _set_screws_to_zero();
static int  _calculate_average_y_pos(int screwNr);
static void _set_Screwer_Cfg(SRobotOffsets screw_Cfg);
static void _handle_waste_pump(void);
static void _handle_ink_pump_back(void);
static void _set_ink_back_pump_left(int voltage);
static void _set_ink_back_pump_right(int voltage);
static void _vacuum_on();
static void _vacuum_off();
static void _pump_main();


//--- lbrob_init --------------------------------------
void lbrob_init(void)
{
    memset(_CmdName, 0, sizeof(_CmdName));
    memset(&_ParCable_ref, 0, sizeof(SMovePar));
    memset(&_ParCable_drive, 0, sizeof(SMovePar));
    memset(&_ParCable_drive_purge, 0, sizeof(SMovePar));
    
    memcpy(&RX_StepperStatus.screwclusters, &RX_StepperCfg.robot[RX_StepperCfg.boardNo].screwclusters, sizeof(RX_StepperStatus.screwclusters));
    memcpy(&RX_StepperStatus.screwpositions, &RX_StepperCfg.robot[RX_StepperCfg.boardNo].screwpositions, sizeof(RX_StepperStatus.screwpositions));

    // config for referencing cable pull motor (motor 4)
    _ParCable_ref.speed = 1000;
    _ParCable_ref.accel = 4000;
    _ParCable_ref.current_acc = 350.0;
    _ParCable_ref.current_run = 350.0;
    _ParCable_ref.stop_mux = 0;
    _ParCable_ref.dis_mux_in = 0;
    _ParCable_ref.estop_level = TRUE;
    _ParCable_ref.estop_in_bit[MOTOR_X_0] = (1 << CABLE_PULL_REF);
    _ParCable_ref.enc_bwd = TRUE;
    _ParCable_ref.encCheck = chk_lb_ref1;

    // config for moving normal with cable pull motor (motor 4)
    // This commands that use this config need to start the motor with the
    // special encoder mode
    _ParCable_drive.speed = 5000;
    _ParCable_drive.accel = 8000;
    _ParCable_drive.current_acc = 400.0;        // max 420      --> with 400 it's possible, that the motor turns, but jumps over the belt
    _ParCable_drive.current_run = 400.0;        // max 420      --> with 400 it's possible, that the motor turns, but jumps over the belt
    _ParCable_drive.stop_mux = 0;
    _ParCable_drive.dis_mux_in = 0;
    _ParCable_drive.estop_level = 0;
    _ParCable_drive.enc_bwd = TRUE;
    _ParCable_drive.encCheck = chk_std;

    _ParCable_drive_slow.speed = 1000;
    _ParCable_drive_slow.accel = 8000;
    _ParCable_drive_slow.current_acc = 400.0;   // max 420      --> with 400 it's possible, that the motor turns, but jumps over the belt
    _ParCable_drive_slow.current_run = 400.0;   // max 420      --> with 400 it's possible, that the motor turns, but jumps over the belt
    _ParCable_drive_slow.stop_mux = 0;
    _ParCable_drive_slow.dis_mux_in = 0;
    _ParCable_drive_slow.estop_level = 0;
    _ParCable_drive_slow.enc_bwd = TRUE;
    _ParCable_drive_slow.encCheck = chk_std;

    _ParCable_drive_purge.speed = _micron_2_steps(1000 * 10); // multiplied with 1000 to get from mm/s to um/s
    _ParCable_drive_purge.accel = 4000;
    _ParCable_drive_purge.current_acc = 400.0; // max 420       --> with 400 it's possible, that the motor turns, but jumps over the belt
    _ParCable_drive_purge.current_run = 400.0; // max 420       --> with 400 it's possible, that the motor turns, but jumps over the belt
    _ParCable_drive_purge.stop_mux = 0;
    _ParCable_drive_purge.dis_mux_in = 0;
    _ParCable_drive_purge.estop_level = 0;
    _ParCable_drive_purge.enc_bwd = TRUE;
    _ParCable_drive_purge.encCheck = chk_std;

    motor_config(MOTOR_X_0, CURRENT_HOLD, X_STEPS_PER_REV, X_INC_PER_REV, STEPS);

    robi_lb702_init();
}

//--- lbrob_main ------------------------------------------------------------------
void lbrob_main(int ticks, int menu)
{
    int motor;
    int pos, val;

    RX_StepperStatus.posY[0] = _steps_2_micron(motor_get_step(MOTOR_X_0));
    RX_StepperStatus.posY[1] = _steps_2_micron(motor_get_step(MOTOR_X_0)) - CABLE_PURGE_POS_BACK;
    SStepperStat oldSatus;
    memcpy(&oldSatus, &RX_StepperStatus, sizeof(RX_StepperStatus));

    RX_StepperStatus.info.vacuum_running = (Fpga.par->output & RO_VACUUM_CLEANER) >> 4;

    motor_main(ticks, menu);
    robi_lb702_main(ticks, menu);

    _handle_ink_pump_back();
    _handle_waste_pump();    

    RX_StepperStatus.robinfo.moving = (_CmdRunning != 0);
    if (RX_StepperStatus.robinfo.moving)
    {
        RX_StepperStatus.info.x_in_ref = FALSE;
        RX_StepperStatus.info.x_in_cap = FALSE;
        RX_StepperStatus.robinfo.cap_ready = FALSE;
        RX_StepperStatus.robinfo.rob_in_cap = FALSE;
        RX_StepperStatus.robinfo.purge_ready = FALSE;
        RX_StepperStatus.robinfo.wash_done = FALSE;
        RX_StepperStatus.robinfo.vacuum_done = FALSE;
        RX_StepperStatus.robinfo.wipe_done = FALSE;
        RX_StepperStatus.robinfo.x_in_purge4ever = FALSE;
        _HeadScrewPos = FALSE;
    }

    if (RX_StepperStatus.info.moving)
    {
        RX_StepperStatus.robinfo.cap_ready = FALSE;
    }

    if (RX_StepperStatus.screwerinfo.screwer_blocked_left || RX_StepperStatus.screwerinfo.screwer_blocked_right)
    {
        _CmdScrewing = 0;
    }

    if (!RX_StepperStatus.robinfo.ref_done)
    {
        RX_StepperStatus.screwerinfo.screws_found = FALSE;
        _HeadScrewPos = FALSE;
    }
    

    if (_CmdRunning && motors_move_done(MOTOR_X_BITS))
    {
		if (!((_CmdRunning == CMD_ROB_MOVE_POS || _CmdRunning_old == CMD_ROB_MOVE_POS) && _RobFunction >= rob_fct_screw_cluster && _RobFunction <= rob_fct_screw_head7) || !RX_StepperStatus.robinfo.ref_done)
		{
			val = 0;
			lbrob_handle_ctrl_msg(INVALID_SOCKET, CMD_ROB_VACUUM, &val);
		}
        RX_StepperStatus.robinfo.moving = FALSE;


        RX_StepperStatus.info.x_in_ref = fpga_input(CABLE_PULL_REF);
        RX_StepperStatus.info.x_in_cap = fpga_input(CAPPING_ENDSTOP);

        if (_CmdRunning == CMD_ROB_REFERENCE)
        {
            if (!RX_StepperStatus.robinfo.ref_done)
            {
                if (motors_error(MOTOR_X_BITS, &motor))
                {
                    RX_StepperStatus.robinfo.ref_done = FALSE;
                    Error(ERR_CONT, 0, "Stepper: Command %s: Motor[%d] blocked", _CmdName, motor + 1);
                    _CmdRunning = FALSE;
                    _CmdRunning_Lift = 0;
                    _NewCmd = 0;
                    Fpga.par->output &= ~RO_ALL_OUTPUTS;
                }
                if (RX_StepperStatus.info.x_in_ref)
                {
                    motors_reset(MOTOR_X_BITS);
                    RX_StepperStatus.robinfo.ref_done = TRUE;
                }
                else
                {
                    Error(ERR_CONT, 0, "LBROB: Command %s: End Sensor REF NOT HIGH", _CmdName);
                    RX_StepperStatus.robinfo.ref_done = FALSE;
                    _CmdRunning_old = FALSE;
                }
            }
            else
            {
                if (motors_error(MOTOR_X_BITS, &motor))
                {
                    Error(ERR_CONT, 0, "LIFT: Command %s - 1000 steps: Motor %s blocked",_CmdName, _MotorName[motor]);
                    RX_StepperStatus.robinfo.ref_done = FALSE;
                }
                else if (!RX_StepperStatus.info.x_in_ref)
                {
                    Error(ERR_CONT, 0, "LBROB: Command %s: End Sensor REF NOT HIGH", _CmdName);
                    RX_StepperStatus.robinfo.ref_done = FALSE;
                    _CmdRunning_old = FALSE;
                }
                RX_StepperStatus.cmdRunning = FALSE;
            }
        }

        if (_CmdRunning != CMD_ROB_REFERENCE)
        {
            if (motors_error(MOTOR_X_BITS, &motor))
            {
                RX_StepperStatus.robinfo.ref_done = FALSE;
                Error(ERR_CONT, 0, "Stepper: Command %s: Motor %s blocked", _CmdName, _MotorName[motor]);
                _CmdRunning = FALSE;
                _CmdRunning_Lift = 0;
                _NewCmd = 0;
                Fpga.par->output &= ~RO_ALL_OUTPUTS;
            }
        }

        if (_CmdRunning == CMD_ROB_FILL_CAP)
        {
            switch (_RobFunction)
            {
            case rob_fct_cap:

                if (rx_get_ticks() >= _CapFillTime + CAP_FILL_TIME)
                {
                    _CmdRunning = FALSE;
                    Fpga.par->output &= ~RO_ALL_OUTPUTS;
                    RX_StepperStatus.robinfo.cap_ready = RX_StepperStatus.info.x_in_cap && !RX_StepperStatus.robinfo.moving;
                    _CapFillTime = 0;
                }
                else if (rx_get_ticks() >= _CapFillTime + CAP_FILL_TIME / 2)
                {
                    Fpga.par->output |= RO_FLUSH_TO_CAP_RIGHT;
                    Fpga.par->output &= ~RO_FLUSH_TO_CAP_LEFT;
                }
                break;

            case rob_fct_purge_all:
                _CmdRunning = FALSE;
                RX_StepperStatus.robinfo.purge_ready = TRUE;

                break;
            case rob_fct_purge_head0:
            case rob_fct_purge_head1:
            case rob_fct_purge_head2:
            case rob_fct_purge_head3:
            case rob_fct_purge_head4:
            case rob_fct_purge_head5:
            case rob_fct_purge_head6:
            case rob_fct_purge_head7:
                _CmdRunning = FALSE;
                break;

            default:
                Error(ERR_CONT, 0, "Command %s: Robi-Function %d not implemented", _CmdName, _RobFunction);
                break;
            }
            _Old_RobFunction = _RobFunction;
        }
        else if (_CmdRunning == CMD_ROB_MOVE_POS)
        {
            switch (_RobFunction)
            {
            case rob_fct_cap:
                RX_StepperStatus.robinfo.rob_in_cap = fpga_input(CAPPING_ENDSTOP);
                if (!RX_StepperStatus.robinfo.rob_in_cap && _NewCmd != CMD_ROB_MOVE_POS)
                {
                    Error(ERR_CONT, 0, "LBROB: Command %s: End Sensor Capping NOT HIGH", _CmdName);
                    RX_StepperStatus.robinfo.ref_done = FALSE;
                }
                _CmdRunning = FALSE;
                break;

            case rob_fct_purge4ever:
                RX_StepperStatus.robinfo.x_in_purge4ever = fpga_input(CAPPING_ENDSTOP);
                if (!RX_StepperStatus.robinfo.x_in_purge4ever && _NewCmd != CMD_ROB_MOVE_POS)
                {
                    Error(ERR_CONT, 0, "LBROB: Command %s: End Sensor Capping NOT HIGH", _CmdName);
                    RX_StepperStatus.robinfo.ref_done = FALSE;
                }
                _CmdRunning = FALSE;
                break;

            case rob_fct_purge_all:
                pos = abs(motor_get_step(MOTOR_X_0));
                RX_StepperStatus.robinfo.purge_ready = (abs(motor_get_step(MOTOR_X_0) - _micron_2_steps(CABLE_PURGE_POS_BACK)) <= MAX_POS_DIFFERENT);
                if (!RX_StepperStatus.robinfo.purge_ready && _NewCmd != CMD_ROB_MOVE_POS)
                {
                    Error(ERR_CONT, 0, "LBROB: Command %s: Robot not in correct position", _CmdName);
                    RX_StepperStatus.robinfo.ref_done = FALSE;
                }
                _CmdRunning = FALSE;
                break;

            case rob_fct_purge_head0:
                RX_StepperStatus.robinfo.purge_ready = (abs(motor_get_step(MOTOR_X_0) - motor_get_end_step(MOTOR_X_0)) <= MAX_POS_DIFFERENT);
                if (!RX_StepperStatus.robinfo.purge_ready && _NewCmd != CMD_ROB_MOVE_POS)
                {
                    Error(ERR_CONT, 0, "LBROB: Command %s: Robot not in correct position", _CmdName);
                    RX_StepperStatus.robinfo.ref_done = FALSE;
                }
                _CmdRunning = FALSE;
                break;
            case rob_fct_purge_head1:
            case rob_fct_purge_head2:
            case rob_fct_purge_head3:
            case rob_fct_purge_head4:
            case rob_fct_purge_head5:
            case rob_fct_purge_head6:
            case rob_fct_purge_head7:
                RX_StepperStatus.robinfo.purge_ready = TRUE;
                _CmdRunning = FALSE;
                break;
            case rob_fct_screw_cluster:
            case rob_fct_screw_head0:
            case rob_fct_screw_head1:
            case rob_fct_screw_head2:
            case rob_fct_screw_head3:
            case rob_fct_screw_head4:
            case rob_fct_screw_head5:
            case rob_fct_screw_head6:
            case rob_fct_screw_head7:
                _HeadScrewPos = _RobFunction;
                _CmdRunning = FALSE;
                break;
            case rob_fct_vacuum:
            case rob_fct_wash:
            case rob_fct_wipe:
                _Old_RobFunction = _RobFunction;
                _RobFunction = rob_fct_move;
                _CmdRunning = FALSE;
                _CmdRunning_old = FALSE;
                if (!RX_StepperStatus.screwerinfo.y_in_ref || !RX_RobiStatus.isInGarage)
                {
                    _CmdRunning_Robi = CMD_ROBI_MOVE_TO_GARAGE;
                    _NewCmd = CMD_ROB_MOVE_POS;
                    robi_lb702_handle_ctrl_msg(INVALID_SOCKET, _CmdRunning_Robi, NULL);
                }
                else
                    lbrob_handle_ctrl_msg(INVALID_SOCKET, CMD_ROB_MOVE_POS, &_RobFunction);
                break;

            case rob_fct_move:
                _CmdRunning = FALSE;
                Fpga.par->output &= ~RO_FLUSH_WIPE;
                switch (_Old_RobFunction)
                {
                case rob_fct_wash:
                    Error(LOG, 0, "Wash done");
                    RX_StepperStatus.robinfo.wash_done = TRUE;
                    break;
                case rob_fct_vacuum:
                    RX_StepperStatus.robinfo.vacuum_done = TRUE;
                    break;
                case rob_fct_wipe:
                    if ((RX_StepperStatus.screwerinfo.wipe_left_up && _WipeSide == wipe_left) || (RX_StepperStatus.screwerinfo.wipe_right_up && _WipeSide == wipe_right))
                    {
                        RX_StepperStatus.robinfo.wipe_done = TRUE;
                        _WipeSide = wipe_none;
                    }
                    else if (_WipeSide == wipe_all && (RX_StepperStatus.screwerinfo.wipe_left_up || RX_StepperStatus.screwerinfo.wipe_right_up))
                    {
                        if (RX_StepperStatus.screwerinfo.wipe_left_up)  _WipeSide = wipe_right;
                        else                                            _WipeSide = wipe_left;
                        lbrob_handle_ctrl_msg(INVALID_SOCKET, CMD_ROB_WIPE, &_WipeSide);
                    }
                    break;
                default:
                    break;
                }
                _Old_RobFunction = _RobFunction;
                break;

            default:
                Error(ERR_CONT, 0, "LBROB_MAIN: Rob-Function %d not implemented", _RobFunction);
                _CmdRunning = FALSE;
                RX_StepperStatus.robinfo.ref_done = FALSE;
                break;
            }
            if (!_CmdRunning_Lift && !(RX_StepperStatus.info.z_in_wash && (_Old_RobFunction == rob_fct_wash || _Old_RobFunction == rob_fct_wipe)) && _RobFunction != rob_fct_move)
                _Old_RobFunction = _RobFunction;
        }
        else if (_CmdRunning == CMD_ROB_REFERENCE && _CmdRunning_old)
        {
            int loc_new_cmd = 0;
            if (!RX_StepperStatus.robinfo.moving && !RX_StepperStatus.info.moving)
            {
                loc_new_cmd = _CmdRunning_old;
                _CmdRunning_old = FALSE;
                _CmdRunning = FALSE;
            }

            switch (loc_new_cmd)
            {
            case CMD_ROB_MOVE_POS:
                lbrob_handle_ctrl_msg(INVALID_SOCKET, CMD_ROB_MOVE_POS, &_RobFunction);
                break;
            default:
                Error(ERR_CONT, 0, "LBROB_MAIN: Command 0x%08x not implemented", _CmdRunning_old);
                break;
            }
            loc_new_cmd = FALSE;
        }
        else
        {
            _CmdRunning = FALSE;
            _CmdRunning_old = FALSE;
        }
    }

    if (_CmdRunning_Lift || _CmdRunning_Robi)
    {
        int loc_new_cmd = 0;
        if (!RX_StepperStatus.robinfo.moving && !RX_StepperStatus.info.moving && !RX_StepperStatus.screwerinfo.moving)
        {
            loc_new_cmd = _NewCmd;
            _NewCmd = FALSE;
            _CmdRunning_Lift = FALSE;
            _CmdRunning_Robi = FALSE;
        }
        if (loc_new_cmd)
        {
            switch (loc_new_cmd)
            {
            case CMD_ROB_REFERENCE:
                lbrob_handle_ctrl_msg(INVALID_SOCKET, CMD_ROB_REFERENCE, NULL);
                break;
            case CMD_ROB_MOVE_POS:
                lbrob_handle_ctrl_msg(INVALID_SOCKET, CMD_ROB_MOVE_POS, &_RobFunction);
                break;
            case CMD_ROB_WIPE:
                lbrob_handle_ctrl_msg(INVALID_SOCKET, CMD_ROB_WIPE, &_WipeSide);
                break;
            default:
                Error(ERR_CONT, 0, "LBROB_MAIN: Command 0x%08x not implemented", loc_new_cmd);
                break;
            }
        }
        loc_new_cmd = FALSE;
    }

    static int j = 0;

    if (!_CmdSearchScrews && !_CmdScrewing && !_CmdResetScrews && !RX_StepperStatus.info.moving && !RX_StepperStatus.robinfo.moving && !RX_StepperStatus.screwerinfo.moving && 
            !RX_StepperStatus.screwerinfo.screwer_blocked_left && !RX_StepperStatus.screwerinfo.screwer_blocked_right)
        RX_StepperStatus.screwerinfo.screwer_ready = TRUE;
    else
        RX_StepperStatus.screwerinfo.screwer_ready = FALSE;

    if (memcmp(&oldSatus.robinfo, &RX_StepperStatus.robinfo, sizeof(RX_StepperStatus.robinfo)) ||
        memcmp(&oldSatus.screwerinfo, &RX_StepperStatus.screwerinfo, sizeof(RX_StepperStatus.screwerinfo)))
    {
        ctrl_send_2(REP_STEPPER_STAT, sizeof(RX_StepperStatus), &RX_StepperStatus);
    }
    else if (_CmdRunning == CMD_ROB_FILL_CAP)
    {
        if (RX_StepperStatus.posY[1] > j * HEAD_WIDTH)
        {
            j++;
            ctrl_send_2(REP_STEPPER_STAT, sizeof(RX_StepperStatus), &RX_StepperStatus);
        }
    }
    else
        j = 0;

    if (_CmdSearchScrews)
    {
        _CmdScrewing = 0;
        _CmdResetScrews = 0;
        _search_all_screws();
    }
    else
        _SearchScrewNr = 0;

    if (_CmdResetScrews)
    {
        _CmdScrewing = 0;
        _set_screws_to_zero();
    }
    else
        _ResetScrewNr = 0;

    if (_CmdScrewing)
        _turn_screw(_HeadAdjustment);
    else
        memset(&_HeadAdjustment, 0, sizeof(_HeadAdjustment));

    

    _pump_main();
}

//---_micron_2_steps --------------------------------------------------------------
static int _micron_2_steps(int micron)
{
    return (int)(0.5 + X_STEPS_PER_REV / X_DIST_PER_REV * micron);
}

//--- _steps_2_micron -------------------------------------------------------------
static int _steps_2_micron(int steps)
{
    return (int)(0.5 + (double)steps / X_STEPS_PER_REV * X_DIST_PER_REV);
}

//--- _lbrob_display_status --------------------------------------------------------
void lbrob_display_status(void)
{
    term_printf("LB ROB ---------------------------------\n");
    if (RX_StepperStatus.robot_used)
    {
        term_printf("moving: \t\t %d \t cmd: %08x\n", RX_StepperStatus.robinfo.moving, _CmdRunning);
        term_printf("Screwing-Step \t\t %d\n", _CmdScrewing);
        term_printf("reference done: \t %d\n", RX_StepperStatus.robinfo.ref_done);
        term_printf("x in reference: \t %d\n", RX_StepperStatus.info.x_in_ref);
        term_printf("x in cap: \t\t %d\n", RX_StepperStatus.info.x_in_cap);
        term_printf("Cap ready \t\t %d\n", RX_StepperStatus.robinfo.cap_ready);
        term_printf("Purge ready: \t\t %d\n", RX_StepperStatus.robinfo.purge_ready);
        term_printf("actPos Robi: \t\t %dum\n", RX_StepperStatus.posY[0]);
        term_printf("Wipe-Speed: \t\t %d\n", RX_StepperCfg.wipe_speed);
        term_printf("Vacuum done: \t\t %d\n", RX_StepperStatus.robinfo.vacuum_done);
        term_printf("Wash done: \t\t %d\n", RX_StepperStatus.robinfo.wash_done);
        term_printf("Wipe done: \t\t %d\n", RX_StepperStatus.robinfo.wipe_done);
        term_printf("Scrwe-Count: \t\t %d\n", RX_StepperStatus.screw_count);
        term_printf("\n");
    }
    else
        term_printf("No Robot (bridge stepper board not installed)\n");
}

//--- lbrob_menu --------------------------------------------------------------------
void lbrob_menu(int help)
{
    if (help)
    {
        term_printf("LB rob MENU ------------------------- (2)\n");
        term_printf("s: STOP\n");
        term_printf("o: toggle output <no>\n");
        term_printf("R: Reference\n");
        term_printf("r<n>: reset motor<n>\n");
        term_printf("c: Go to Cap-Pos\n");
        term_printf("w: Wash Heads\n");
        term_printf("q<n>: Stop(n=0)/Start(n=1) Vacuum Pump \n");
        term_printf("v: Vacuum Heads\n");
        term_printf("g<n>: Go to purge position of head 1 -8 or for all (0)\n");
        term_printf("p: Move the Robi for Purging all Heads\n");
        term_printf("m<n><steps>: move Motor<n> by <steps>\n");
        term_printf("f: Find all the screws\n");
        term_printf("a<n>: Go to adjustment position of head 0 - 7\n");
        term_printf("t<screw_nr>: Turn <screw_nr> n/6 Turn (n can be choosen by command \"T\")\n");
        term_printf("T<n>: Make <n>/6 turns with the command t\n");
        term_printf("d<n>: Wipe all (n = l: left; n = r: right; n = b: both\n");
        term_printf("b<n>: Set Speed of Waste pump to <n>%, Actual value %d%\n", _PumpSpeed);
        term_printf("z<n>: Move all Screws to end position (n = l: left; n = r: right; n = b: both\n");
        term_flush();
    }
    else
    {
        term_printf("?: help\n");
        term_printf("1: LB702 menu\n");
        term_printf("3: ROBI menu\n");
    }
}

//--- lbrob_handle_menu -----------------------------------------------
void lbrob_handle_menu(char *str)
{
    int pos = 10000;
    int val;
    SHeadAdjustment screw_head;
    EWipeSide wipeside;
    switch (str[0])
    {
    case 's':
        lbrob_handle_ctrl_msg(INVALID_SOCKET, CMD_ROB_STOP, NULL);
        break;
    case 'o':
        Fpga.par->output ^= (1 << atoi(&str[1]));
        break;
    case 'R':
        lbrob_handle_ctrl_msg(INVALID_SOCKET, CMD_ROB_REFERENCE, NULL);
        break;
    case 'r':
        motor_reset(atoi(&str[1]));
        break;
    case 'c':
        pos = rob_fct_cap;
        lbrob_handle_ctrl_msg(INVALID_SOCKET, CMD_ROB_MOVE_POS, &pos);
        break;
    case 'w':
        pos = rob_fct_wash;
        lbrob_handle_ctrl_msg(INVALID_SOCKET, CMD_ROB_MOVE_POS, &pos);
        break;
    case 'q':
        val = atoi(&str[1]);
        lbrob_handle_ctrl_msg(INVALID_SOCKET, CMD_ROB_VACUUM, &val);
        break;
    case 'v':
        pos = rob_fct_vacuum;
        lbrob_handle_ctrl_msg(INVALID_SOCKET, CMD_ROB_MOVE_POS, &pos);
        break;
    case 'g':
        switch (str[1])
        {
        case '0':
            pos = rob_fct_purge_all;
            break;
        case '1':
            pos = rob_fct_purge_head0;
            break;
        case '2':
            pos = rob_fct_purge_head1;
            break;
        case '3':
            pos = rob_fct_purge_head2;
            break;
        case '4':
            pos = rob_fct_purge_head3;
            break;
        case '5':
            pos = rob_fct_purge_head4;
            break;
        case '6':
            pos = rob_fct_purge_head5;
            break;
        case '7':
            pos = rob_fct_purge_head6;
            break;
        case '8':
            pos = rob_fct_purge_head7;
            break;
        }
        lbrob_handle_ctrl_msg(INVALID_SOCKET, CMD_ROB_MOVE_POS, &pos);
        break;
    case 'p':
        pos = rob_fct_purge_all;
        lbrob_handle_ctrl_msg(INVALID_SOCKET, CMD_ROB_FILL_CAP, &pos);
        break;
    case 'm':
        _lbrob_motor_test(str[1] - '0', atoi(&str[2]));
        break;
    case 'f':
        lbrob_handle_ctrl_msg(INVALID_SOCKET, CMD_SEARCH_ALL_SCREWS, NULL);
        break;
    case 'z':
        _ResetScrewsLeft = FALSE;
        _ResetScrewsRight = FALSE;
        _ResetInfinity = 0;
        switch (str[1])
        {
        case 'l':
            _ResetScrewsLeft = TRUE;
            break;
        case 'r':
            _ResetScrewNr = 16;
            _ResetScrewsLeft = FALSE;
            break;
        case 'b':
            _ResetScrewsLeft = FALSE;
            break;
        }
        switch (str[2])
        {
        case 'n':
            _ResetInfinity = 1;
            break;
        default:
            break;
        }
        lbrob_handle_ctrl_msg(INVALID_SOCKET, CMD_RESET_ALL_SCREWS, NULL);
        break;
    case 'a':
        switch (str[1])
        {
        case '0':
            pos = rob_fct_screw_head0;
            break;
        case '1':
            pos = rob_fct_screw_head1;
            break;
        case '2':
            pos = rob_fct_screw_head2;
            break;
        case '3':
            pos = rob_fct_screw_head3;
            break;
        case '4':
            pos = rob_fct_screw_head4;
            break;
        case '5':
            pos = rob_fct_screw_head5;
            break;
        case '6':
            pos = rob_fct_screw_head6;
            break;
        case '7':
            pos = rob_fct_screw_head7;
            break;
        default:
            pos = rob_fct_screw_head0;
            break;
        }
        lbrob_handle_ctrl_msg(INVALID_SOCKET, CMD_ROB_MOVE_POS, &pos);
        break;
    case 't':
        screw_head.printbarNo = atoi(&str[1]) / (SCREWS_PER_HEAD * HEADS_PER_COLOR);
        screw_head.headNo = (atoi(&str[1]) % (SCREWS_PER_HEAD * HEADS_PER_COLOR)) / SCREWS_PER_HEAD;
        screw_head.axis = atoi(&str[1]) % SCREWS_PER_HEAD;
        screw_head.steps = _Turns;
        lbrob_handle_ctrl_msg(INVALID_SOCKET, CMD_HEAD_ADJUST, &screw_head);
        break;
    case 'T':
        _Turns = atoi(&str[1]);
        break;
    case 'd':
        if (str[1] == 'l')
            wipeside = wipe_left;
        else if (str[1] == 'r')
            wipeside = wipe_right;
        else if (str[1] == 'b')
            wipeside = wipe_all;
        lbrob_handle_ctrl_msg(INVALID_SOCKET, CMD_ROB_WIPE, &wipeside);
        break;
    case 'b':
        val = atoi(&str[1]);
        if (val < 0) val = 0;
        if (val > 100) val = 100;
        _PumpSpeed = val;
        break;
    case 'e':
        RX_StepperStatus.inkinfo.ink_pump_error_left = !RX_StepperStatus.inkinfo.ink_pump_error_left;
        break;
    case 'h':
        RX_StepperStatus.inkinfo.ink_pump_error_right = !RX_StepperStatus.inkinfo.ink_pump_error_right;
        break;
    }
}

//--- _lbrob_move_to_pos ---------------------------------------------------------------
static void _lbrob_move_to_pos(int cmd, int pos)
{
    _CmdRunning = cmd;
    RX_StepperStatus.robinfo.moving = TRUE;
    motors_move_to_step(MOTOR_X_BITS, &_ParCable_drive, pos);
}

//--- _lbrob_move_to_pos_slow ---------------------------------------------------------------
static void _lbrob_move_to_pos_slow(int cmd, int pos)
{
    _CmdRunning = cmd;
    RX_StepperStatus.robinfo.moving = TRUE;
	motors_move_to_step(MOTOR_X_BITS, &_ParCable_drive_slow, pos);
}

//--- _lbrob_do_reference ----------------------------------------------------------------------
static void _lbrob_do_reference()
{
    Fpga.par->output &= ~RO_ALL_OUTPUTS;
    _NewCmd = 0;
    RX_StepperStatus.robinfo.moving = TRUE;
    _CmdRunning = CMD_ROB_REFERENCE;
    if (!RX_StepperStatus.robinfo.ref_done)
    {
        _vacuum_on();
        motor_reset(MOTOR_X_0);
        motor_config(MOTOR_X_0, CURRENT_HOLD, X_STEPS_PER_REV, X_INC_PER_REV, STEPS);
        RX_StepperStatus.robinfo.ref_done = FALSE;
        motors_move_by_step(1 << MOTOR_X_0, &_ParCable_ref, 1000000, TRUE);
    }
	else if (_CmdRunning_old == CMD_ROB_MOVE_POS && _RobFunction >= rob_fct_screw_cluster && _RobFunction <= rob_fct_screw_head7)
		motors_move_to_step(MOTOR_X_BITS, &_ParCable_drive, _micron_2_steps(3000));
    else
    {
        _vacuum_on();
        motors_move_to_step(MOTOR_X_BITS, &_ParCable_drive_slow, _micron_2_steps(3000));
    }
        
}

//--- lbrob_handle_ctrl_msg -----------------------------------
int lbrob_handle_ctrl_msg(RX_SOCKET socket, int msgId, void *pdata)
{
    int val, pos;

    switch (msgId)
    {
    case CMD_ROB_STOP:
        strcpy(_CmdName, "CMD_ROB_STOP");
        motors_stop(MOTOR_X_BITS);
        Fpga.par->output &= ~RO_ALL_OUTPUTS;
        _CmdRunning_Lift = 0;
        _CmdRunning = 0;
        _CmdSearchScrews = 0;
        _CmdScrewing = 0;
        _CmdResetScrews = 0;
        _ScrewTime = 0;
        RX_StepperStatus.robinfo.ref_done = FALSE;
        RX_StepperStatus.screwerinfo.screw_reset = FALSE;
        int val = 0;
        lbrob_handle_ctrl_msg(INVALID_SOCKET, CMD_ROB_VACUUM, &val);
        robi_lb702_handle_ctrl_msg(INVALID_SOCKET, CMD_ROBI_STOP, NULL);
        break;

    case CMD_ROB_REFERENCE:
        if (_CmdRunning == CMD_ROB_REFERENCE) break;
        strcpy(_CmdName, "CMD_ROB_REFERENCE");
        if (_CmdRunning)
        {
            lbrob_handle_ctrl_msg(INVALID_SOCKET, CMD_ROB_STOP, NULL);
            _NewCmd = CMD_ROB_REFERENCE;
            break;
        }
        if (!RX_StepperStatus.info.z_in_ref)
        {
            if (!RX_StepperStatus.info.moving)
            {
                _CmdRunning_Lift = CMD_LIFT_REFERENCE;
                lb702_handle_ctrl_msg(INVALID_SOCKET, _CmdRunning_Lift, NULL);
                _NewCmd = msgId;
            }
            break;
        }
        else if (!RX_StepperStatus.screwerinfo.z_in_down)
        {
            _CmdRunning_Robi = CMD_ROBI_MOVE_Z_DOWN;
            _NewCmd = msgId;
            robi_lb702_handle_ctrl_msg(INVALID_SOCKET, _CmdRunning_Robi, NULL);

            break;
        }
        _CmdRunning = msgId;
        _lbrob_do_reference();
        break;

    case CMD_ROB_MOVE_POS:
        strcpy(_CmdName, "CMD_ROB_MOVE_POS");
        _cln_move_to(msgId, *(ERobotFunctions *)pdata);
        break;

    case CMD_ROB_WIPE:
        strcpy(_CmdName, "CMD_ROB_WIPE");
        _rob_wipe(msgId, *(EWipeSide *)pdata);
        break;

    case CMD_ROB_EMPTY_WASTE:
        strcpy(_CmdName, "CMD_ROB_EMPTY_WASTE");
        val = (*(INT32 *)pdata);
        if (val == 0)
        {
            if (Fpga.par->output & RO_INK_VALVE_LEFT)
                _PumpTimeLeft = rx_get_ticks() + FOLLOW_UP_TIME_INK_BACK;
            if (Fpga.par->output & RO_INK_VALVE_RIGHT)
                _PumpTimeRight = rx_get_ticks() + FOLLOW_UP_TIME_INK_BACK;
            _vacuum_off();
        }
        else if (val == 1 && !RX_StepperStatus.inkinfo.ink_pump_error_left)
        {
            Fpga.par->output |= RO_INK_VALVE_LEFT;
        }
        else if (val == 2)
        {
            if (Fpga.par->output & RO_INK_VALVE_LEFT)
                _PumpTimeLeft = rx_get_ticks() + FOLLOW_UP_TIME_INK_BACK;
            _vacuum_off();
        }
        else if (val == 3 && !RX_StepperStatus.inkinfo.ink_pump_error_right)
        {
            Fpga.par->output |= RO_INK_VALVE_RIGHT;
        }
        else if (val == 4)
        {
            if (Fpga.par->output & RO_INK_VALVE_RIGHT)
                _PumpTimeRight = rx_get_ticks() + FOLLOW_UP_TIME_INK_BACK;
            _vacuum_off();
        }
        else if (val == 5)
        {
            Fpga.par->output |= RO_INK_VALVE_BOTH;
        }
        break;

    case CMD_ROB_VACUUM:
        val = (*(INT32 *)pdata);
        if (val <= 0)
            val = VACUUM_PUMP_TIME;
        else
            val = val * 1000;
        
        if (_WastePumpTimer < rx_get_ticks() + val || !_WastePumpTimer || !RX_StepperStatus.info.vacuum_running)
        {
            _vacuum_on();
            _WastePumpTimer = rx_get_ticks() + val;
        }
        break;

    case CMD_ROB_FILL_CAP:
        strcpy(_CmdName, "CMD_ROB_FILL_CAP");
        if (!_CmdRunning)
        {

            if (!RX_StepperStatus.robinfo.ref_done)
            {
                Error(ERR_CONT, 0, "LBROB: Robot not refenenced, cmd=0x%08x", msgId);
                break;
            }
            _CmdRunning = msgId;
            pos = *((INT32 *)pdata);
            _RobFunction = pos;
            switch (_RobFunction)
            {
            case rob_fct_cap:
                RX_StepperStatus.robinfo.moving = TRUE;
                Fpga.par->output &= ~RO_ALL_OUTPUTS;
                Fpga.par->output |= RO_FLUSH_TO_CAP_LEFT;
                Fpga.par->output |= RO_FLUSH_PUMP;
                _CapFillTime = rx_get_ticks();
                break;

            case rob_fct_purge_all:
                if (RX_StepperCfg.wipe_speed == 0)
                    Error(ERR_CONT, 0, "Wipe-Speed is set 0, please chose another value");

                if (!RX_StepperStatus.info.z_in_ref)
                {
                    if (!RX_StepperStatus.info.moving)
                    {
                        _CmdRunning_Lift = CMD_LIFT_REFERENCE;
                        lb702_handle_ctrl_msg(INVALID_SOCKET, _CmdRunning_Lift, NULL);
                        _NewCmd = msgId;
                        break;
                    }
                }
                _vacuum_on();
                RX_StepperStatus.robinfo.moving = TRUE;
                _CmdRunning = msgId;
                if (RX_StepperCfg.wipe_speed)
                    _ParCable_drive_purge.speed = _micron_2_steps(1000 * RX_StepperCfg.wipe_speed); // multiplied with 1000 to get from mm/s to um/s
                else
                    _ParCable_drive_purge.speed = _micron_2_steps(1000 * 10); // multiplied with 1000 to get from mm/s to um/s
                motors_move_to_step(MOTOR_X_BITS, &_ParCable_drive_purge, _micron_2_steps(CABLE_PURGE_POS_FRONT));
                break;

            default:
                break;
            }
        }
        break;

    case CMD_HEAD_ADJUST:
        RX_StepperStatus.screwerinfo.screwed = FALSE;
        RX_StepperStatus.screwerinfo.screw_out_of_range = FALSE;
        _turn_screw(*(SHeadAdjustment *)pdata);
        break;

    case CMD_HEAD_OUT_OF_RANGE:
        RX_StepperStatus.screwerinfo.screw_out_of_range = TRUE;
        RX_StepperStatus.screw_count++;
        break;

    case CMD_SEARCH_ALL_SCREWS:
        _search_all_screws();
        break;
        
    case CMD_RESET_ALL_SCREWS:
        _set_screws_to_zero();
        break;

    case CMD_CFG_SCREW_POS:
        _set_Screwer_Cfg(*(SRobotOffsets *)pdata);
        break;

    default:
        lb702_handle_ctrl_msg(socket, msgId, pdata);
        break;
    }
}

//--- _cln_move_to ---------------------------------------
static void _cln_move_to(int msgId, ERobotFunctions fct)
{
    int val, pos;
    int wash_pos = (abs(motor_get_step(MOTOR_X_0) - _micron_2_steps(CABLE_PURGE_POS_FRONT)) <= MAX_POS_DIFFERENT);
    if (!_CmdRunning)
    {
        _RobFunction = fct;
        if ((_RobFunction == rob_fct_vacuum || _RobFunction == rob_fct_wash || _RobFunction == rob_fct_wipe) && wash_pos)
        {
            _Old_RobFunction = _RobFunction;
            _RobFunction = rob_fct_move;
            RX_StepperStatus.robinfo.purge_ready = FALSE;
        }

        if (!RX_StepperStatus.screwerinfo.z_in_down)
        {
            _CmdRunning_Robi = CMD_ROBI_MOVE_Z_DOWN;
            _NewCmd = msgId;
            robi_lb702_handle_ctrl_msg(INVALID_SOCKET, _CmdRunning_Robi, NULL);
            return;
        }
        else if (!RX_StepperStatus.info.z_in_ref && !(_RobFunction == rob_fct_move && RX_StepperStatus.info.z_in_wash) && !(wash_pos && (_RobFunction == rob_fct_wash || _RobFunction == rob_fct_wipe)))
        {
            if (!RX_StepperStatus.info.moving)
            {
                _CmdRunning_Lift = CMD_LIFT_REFERENCE;
                lb702_handle_ctrl_msg(INVALID_SOCKET, _CmdRunning_Lift, NULL);
                _NewCmd = msgId;
            }
            return;
        }
        else if (!RX_StepperStatus.robinfo.ref_done || (_RobFunction == rob_fct_cap && (RX_StepperStatus.info.x_in_cap || _HeadScrewPos)) || 
                     (RX_StepperStatus.robinfo.purge_ready && _RobFunction != rob_fct_wash && _RobFunction != rob_fct_vacuum && _RobFunction != rob_fct_wipe) ||
                     (_RobFunction == rob_fct_purge4ever && RX_StepperStatus.info.x_in_cap && !RX_StepperStatus.robinfo.x_in_purge4ever))
        {
            _lbrob_do_reference();
            _CmdRunning_old = msgId;
            return;
        }

        RX_StepperStatus.robinfo.moving = TRUE;
        _CmdRunning = msgId;
        switch (_RobFunction)
        {
        case rob_fct_purge4ever:
        case rob_fct_cap:
            _vacuum_on();
            _lbrob_move_to_pos_slow(_CmdRunning, _micron_2_steps(CABLE_CAP_POS));
            break;

        case rob_fct_purge_all:
            _lbrob_move_to_pos_slow(_CmdRunning, _micron_2_steps(CABLE_PURGE_POS_BACK));
            _vacuum_on();
            break;

        case rob_fct_purge_head0:
        case rob_fct_purge_head1:
        case rob_fct_purge_head2:
        case rob_fct_purge_head3:
        case rob_fct_purge_head4:
        case rob_fct_purge_head5:
        case rob_fct_purge_head6:
        case rob_fct_purge_head7:
            _vacuum_on();
            pos = (CABLE_PURGE_POS_BACK + (((int)_RobFunction - (int)rob_fct_purge_head0) * (CABLE_PURGE_POS_FRONT - CABLE_PURGE_POS_BACK)) / 7);
            _lbrob_move_to_pos_slow(_CmdRunning, _micron_2_steps(pos));
            break;
        case rob_fct_vacuum:
        case rob_fct_wash:
        case rob_fct_wipe:
            _lbrob_move_to_pos_slow(_CmdRunning, _micron_2_steps(CABLE_PURGE_POS_FRONT));
            break;

        case rob_fct_move:
            if (!RX_StepperStatus.info.z_in_wash)
            {
                if (!RX_StepperStatus.info.moving && !RX_StepperStatus.screwerinfo.moving)
                {
                    _CmdRunning = FALSE;
                    RX_StepperStatus.robinfo.moving = FALSE;
                    _CmdRunning_Lift = CMD_LIFT_WASH_POS;
                    _NewCmd = msgId;
                    lb702_handle_ctrl_msg(INVALID_SOCKET, _CmdRunning_Lift, NULL);
                }
                return;
            }
            else if(_Old_RobFunction == rob_fct_wipe && ((_WipeSide >= wipe_right && !RX_StepperStatus.screwerinfo.wipe_right_up) || (_WipeSide == wipe_left && !RX_StepperStatus.screwerinfo.wipe_left_up)))
            {
                if (_WipeSide >= wipe_right)
                {
                    _CmdRunning = FALSE;
                    _CmdRunning_Robi = CMD_ROBI_WIPE_RIGHT;
                    _NewCmd = msgId;
                    pos = 1;
                    robi_lb702_handle_ctrl_msg(INVALID_SOCKET, _CmdRunning_Robi, &pos);
                    break;
                }
                else if (_WipeSide == wipe_left)
                {
                    _CmdRunning = FALSE;
                    _CmdRunning_Robi = CMD_ROBI_WIPE_LEFT;
                    pos = 1;
                    _NewCmd = msgId;
                    robi_lb702_handle_ctrl_msg(INVALID_SOCKET, _CmdRunning_Robi, &pos);
                    break;
                }
            }
            motors_move_to_step(MOTOR_X_BITS, &_ParCable_drive_slow, _micron_2_steps(CABLE_WASH_POS_BACK));
            switch (_Old_RobFunction)
            {
            case rob_fct_wash:
                Fpga.par->output |= RO_FLUSH_WIPE;
                Fpga.par->output |= RO_FLUSH_PUMP;
            case rob_fct_vacuum:
                _vacuum_on();
                break;
            case rob_fct_wipe:
                _vacuum_off();
                if (RX_StepperStatus.screwerinfo.wipe_left_up)
                    Fpga.par->output |= RO_FLUSH_WIPE_LEFT;
                else if (RX_StepperStatus.screwerinfo.wipe_right_up)
                    Fpga.par->output |= RO_FLUSH_WIPE_RIGHT;
                Fpga.par->output |= RO_FLUSH_PUMP;
                break;
            default:
                break;
            }
            break;

        case rob_fct_screw_cluster:
        case rob_fct_screw_head0:
        case rob_fct_screw_head1:
        case rob_fct_screw_head2:
        case rob_fct_screw_head3:
        case rob_fct_screw_head4:
        case rob_fct_screw_head5:
        case rob_fct_screw_head6:
        case rob_fct_screw_head7:
            pos = (CABLE_SCREW_POS_BACK + (((int)_RobFunction - (int)rob_fct_screw_cluster) * (CABLE_SCREW_POS_FRONT - CABLE_SCREW_POS_BACK)) / 8);
            if (RX_StepperStatus.posY[0] < pos)
            {
				_CmdRunning_old = msgId;
                _lbrob_do_reference();
                return;
            }
            _lbrob_move_to_pos(_CmdRunning, _micron_2_steps(pos));
            break;

        default:
            Error(ERR_CONT, 0, "Command %s: Rob-Function %d not implemented", _CmdName, _RobFunction);
        }
    }
}

static void _rob_wipe(int msgId, EWipeSide side)
{
    if ((_WipeSide == wipe_left && side == wipe_right) || (_WipeSide == wipe_right && side == wipe_left))
        side = wipe_all;
    _WipeSide = side;
    if (!_CmdRunning)
    {
        ERobotFunctions robfct = rob_fct_wipe;
        lbrob_handle_ctrl_msg(INVALID_SOCKET, CMD_ROB_MOVE_POS, &robfct);
    }
}

// static void _turn_screw(int screwNr, int screwTurns)
static void _turn_screw(SHeadAdjustment headAdjustment)
{
    int screwNr, screwSteps, pos, difference;
    static int max_Wait_Time = 46000; // ms
    static int max_Wait_Time_Sledge = 100000; // ms
    static int max_Wait_Time_Screw = 180000;  // ms
    static int correction_value;
    static int wait_time = 0;
    int pos_min;

    screwNr = headAdjustment.printbarNo * HEADS_PER_COLOR * SCREWS_PER_HEAD + headAdjustment.headNo * SCREWS_PER_HEAD + headAdjustment.axis + 1;
    screwSteps = headAdjustment.steps;

    if (!RX_StepperStatus.info.moving && !RX_StepperStatus.robinfo.moving && !RX_StepperStatus.screwerinfo.moving)
    {
        if (headAdjustment.headNo < -1 || (headAdjustment.headNo == -1 && headAdjustment.axis == AXE_ANGLE) || headAdjustment.headNo >= HEADS_PER_COLOR)
        {
            Error(ERR_CONT, 0, "Screw does not exist");
            return;
        }
        else if (headAdjustment.headNo == HEADS_PER_COLOR - 1 && headAdjustment.axis == AXE_STITCH)
        {
            Error(ERR_CONT, 0, "Last screw of each color is pointless to turn");
            return;
        }
        else if (headAdjustment.steps == 0)
        {
            Error(LOG, 0, "Screw of Printbar %d, Head %d and Axis %d moves only %d Steps, which will not be made", 
              headAdjustment.printbarNo+1, headAdjustment.headNo+1, headAdjustment.axis, headAdjustment.steps);
            RX_StepperStatus.screwerinfo.screwed = TRUE;
            RX_StepperStatus.screw_count++;
            return;            
        }        

        _HeadAdjustment = headAdjustment;

        if (screwNr >= HEADS_PER_COLOR * 4)
        {
            Error(ERR_CONT, 0, "Screw %d does not exist", screwNr);
            _CmdScrewing = 0;
            return;
        }
        if (_ScrewTime && rx_get_ticks() > _ScrewTime && _CmdScrewing)
        {
            Error(ERR_CONT, 0, "Robot stock in turning screw step %d at printbar %d, head %d, axis %d", _CmdScrewing, _HeadAdjustment.printbarNo, _HeadAdjustment.headNo, _HeadAdjustment.axis);
            _ScrewTime = 0;
            _CmdScrewing = 0;
            return;
        }
        
        if (_CmdScrewing == 0)  correction_value = 0;
        if (_CmdScrewing != 6)  wait_time = 0;

        RX_StepperStatus.screwerinfo.screw_reset = FALSE;

        switch (_CmdScrewing)
        {
        case 0:
            RX_StepperStatus.screwerinfo.screwer_blocked_left = FALSE;
            RX_StepperStatus.screwerinfo.screwer_blocked_right = FALSE;
            RX_StepperStatus.screwerinfo.screwed = FALSE;
            if (_check_in_screw_pos(headAdjustment))
            {
                _CmdScrewing = 4;
                _ScrewTime = rx_get_ticks() + max_Wait_Time;
            }
            else
            {
                pos = headAdjustment.headNo + rob_fct_screw_head0;
                if (_HeadScrewPos != pos)
                    lbrob_handle_ctrl_msg(INVALID_SOCKET, CMD_ROB_MOVE_POS, &pos);
                _CmdScrewing++;
                _ScrewTime = rx_get_ticks() + max_Wait_Time_Sledge;
            }
           
            break;

        case 1:
            if (RX_StepperStatus.robinfo.ref_done && _HeadScrewPos == headAdjustment.headNo + rob_fct_screw_head0)
            {
                if (headAdjustment.headNo == -1)
                {
                    if (RX_StepperCfg.robot[RX_StepperCfg.boardNo].screwclusters[headAdjustment.printbarNo].posY)
                        pos = RX_StepperCfg.robot[RX_StepperCfg.boardNo].screwclusters[headAdjustment.printbarNo].posY + correction_value;
                    else
                        pos = SCREW_Y_FRONT + correction_value;
                }
                else if (RX_StepperCfg.robot[RX_StepperCfg.boardNo].screwpositions[headAdjustment.printbarNo][headAdjustment.headNo][headAdjustment.axis].posY)
                    pos = RX_StepperCfg.robot[RX_StepperCfg.boardNo].screwpositions[headAdjustment.printbarNo][headAdjustment.headNo][headAdjustment.axis].posY + correction_value;
                else if (headAdjustment.axis == AXE_STITCH)
                    pos = SCREW_Y_FRONT + correction_value;
                else
                    pos = SCREW_Y_BACK + correction_value;
                robi_lb702_handle_ctrl_msg(INVALID_SOCKET, CMD_ROBI_MOVE_TO_Y, &pos);
                _CmdScrewing++;
                _ScrewTime = rx_get_ticks() + max_Wait_Time;
            }
            break;
        case 2:
            if (RX_StepperStatus.screwerinfo.y_in_pos)
            {
                if (headAdjustment.headNo == -1)
                {
                    if (RX_StepperCfg.robot[RX_StepperCfg.boardNo].screwclusters[headAdjustment.printbarNo].posX)
                        pos = RX_StepperCfg.robot[RX_StepperCfg.boardNo].screwclusters[headAdjustment.printbarNo].posX;
                    else if (headAdjustment.printbarNo == LEFT)
                        pos = SCREW_X_LEFT;
                    else
                        pos = SCREW_X_RIGHT;
                }
                else if (RX_StepperCfg.robot[RX_StepperCfg.boardNo].screwpositions[headAdjustment.printbarNo][headAdjustment.headNo][headAdjustment.axis].posX)
                    pos = RX_StepperCfg.robot[RX_StepperCfg.boardNo].screwpositions[headAdjustment.printbarNo][headAdjustment.headNo][headAdjustment.axis].posX;
                else if (headAdjustment.printbarNo == LEFT)
                    pos = SCREW_X_LEFT;
                else
                    pos = SCREW_X_RIGHT;
                robi_lb702_handle_ctrl_msg(INVALID_SOCKET, CMD_ROBI_MOVE_TO_X, &pos);
                _CmdScrewing++;
                _ScrewTime = rx_get_ticks() + max_Wait_Time;
            }
            break;

        case 3:
            if (RX_StepperStatus.screwerinfo.x_in_pos)
            {
                 if (!RX_StepperStatus.info.z_in_screw) lb702_handle_ctrl_msg(INVALID_SOCKET, CMD_LIFT_SCREW, NULL);
                _CmdScrewing++;
                _ScrewTime = rx_get_ticks() + max_Wait_Time;
            }
            break;

        case 4:
            if (RX_StepperStatus.info.z_in_screw)
            {
                robi_lb702_handle_ctrl_msg(INVALID_SOCKET, CMD_ROBI_MOVE_Z_UP, NULL);
                _CmdScrewing++;
                _TimeSearchScrew = rx_get_ticks();
                _ScrewTime = rx_get_ticks() + max_Wait_Time;
            }
            break;

        case 5:
            if (RX_StepperStatus.screwerinfo.z_in_up)
            {
                correction_value = 0;
                if (headAdjustment.axis == AXE_STITCH && screwSteps < 0)
                {
                    screwSteps -= 12;
                    screwSteps = abs(screwSteps);
                    _HeadAdjustment.steps = 12;
                    robi_lb702_handle_ctrl_msg(INVALID_SOCKET, CMD_ROBI_SCREW_RIGHT, &screwSteps);
                }
                else if (headAdjustment.axis == AXE_ANGLE == screwSteps > 0)
                {
                    screwSteps += 12;
                    _HeadAdjustment.steps = -12;
                    robi_lb702_handle_ctrl_msg(INVALID_SOCKET, CMD_ROBI_SCREW_LEFT, &screwSteps);
                }
                else
                {
                    RX_StepperStatus.screwerinfo.screw_loosed = TRUE;
                }
                _CmdScrewing++;
                _ScrewTime = rx_get_ticks() + max_Wait_Time_Screw;
            }
            else if (_TimeSearchScrew && rx_get_ticks() > _TimeSearchScrew + SCREW_SEARCHING_TIME)
            {
                _TimeSearchScrew = 0;
                if (abs(correction_value) >= 3000)
                {
                    Error(ERR_CONT, 0, "Screw %d of Head %d of Printhead %d not found", headAdjustment.axis, headAdjustment.headNo, headAdjustment.printbarNo);
                    _CmdScrewing = 0;
                    break;
                }
                else if (correction_value >= 0)
                    correction_value = (correction_value + 500) * (-1);
                else
                    correction_value *= (-1);
                _CmdScrewing = 1;
                _ScrewTime = rx_get_ticks() + max_Wait_Time;
            }
            else if (!RX_StepperStatus.screwerinfo.moving)
            {
                robi_lb702_handle_ctrl_msg(INVALID_SOCKET, CMD_ROBI_MOVE_Z_UP, NULL);
            }
            break;

        case 6:
            if (RX_StepperStatus.screwerinfo.screw_loosed)
            {
                if (!wait_time) wait_time = rx_get_ticks() + 200;
                if (rx_get_ticks() > wait_time)
                {
                    if (screwSteps >= 0)
                        robi_lb702_handle_ctrl_msg(INVALID_SOCKET, CMD_ROBI_SCREW_LEFT, &screwSteps);
                    else
                    {
                        screwSteps = abs(screwSteps);
                        robi_lb702_handle_ctrl_msg(INVALID_SOCKET, CMD_ROBI_SCREW_RIGHT, &screwSteps);
                    }
                    _CmdScrewing++;
                    _ScrewTime = rx_get_ticks() + max_Wait_Time_Screw;
                }
            }
            break;
        case 7:
            if (RX_StepperStatus.screwerinfo.screw_tight)
            {
                int i;
                if (headAdjustment.headNo == -1)
                {
                    RX_StepperStatus.screwclusters[headAdjustment.printbarNo].posX = RX_StepperStatus.screw_posX;
                    RX_StepperStatus.screwclusters[headAdjustment.printbarNo].posY = RX_StepperStatus.screw_posY;
                    for (i = headAdjustment.headNo + 1; i < HEADS_PER_COLOR; i++)
                    {
                        if (RX_StepperCfg.robot[RX_StepperCfg.boardNo].screwclusters[headAdjustment.printbarNo].posY)
                            difference = (RX_StepperStatus.screw_posY - RX_StepperCfg.robot[RX_StepperCfg.boardNo].screwclusters[headAdjustment.printbarNo].posY);
                        else
                            difference = 0;
                        if (RX_StepperStatus.screwpositions[headAdjustment.printbarNo][i][AXE_ANGLE].posY)
                            RX_StepperStatus.screwpositions[headAdjustment.printbarNo][i][AXE_ANGLE].posY += difference;
                        if (RX_StepperStatus.screwpositions[headAdjustment.printbarNo][i][AXE_STITCH].posY)
                            RX_StepperStatus.screwpositions[headAdjustment.printbarNo][i][AXE_STITCH].posY += difference;
                    }
                }
                else
                {
                    RX_StepperStatus.screwpositions[headAdjustment.printbarNo][headAdjustment.headNo][headAdjustment.axis].posX = RX_StepperStatus.screw_posX;
                    RX_StepperStatus.screwpositions[headAdjustment.printbarNo][headAdjustment.headNo][headAdjustment.axis].posY = RX_StepperStatus.screw_posY;
                    
                    if (headAdjustment.axis == AXE_STITCH)
                    {
                        for (i = headAdjustment.headNo + 1; i < HEADS_PER_COLOR; i++)
                        {
                            if (RX_StepperCfg.robot[RX_StepperCfg.boardNo].screwpositions[headAdjustment.printbarNo][headAdjustment.headNo][AXE_STITCH].posY)
                                difference = (RX_StepperStatus.screw_posY - RX_StepperCfg.robot[RX_StepperCfg.boardNo].screwpositions[headAdjustment.printbarNo][headAdjustment.headNo][AXE_STITCH].posY);
                            else
                                difference = 0;
                            if (RX_StepperStatus.screwpositions[headAdjustment.printbarNo][i][AXE_ANGLE].posY)
                                RX_StepperStatus.screwpositions[headAdjustment.printbarNo][i][AXE_ANGLE].posY += difference;
                            if (i != (HEADS_PER_COLOR - 1) && RX_StepperStatus.screwpositions[headAdjustment.printbarNo][i][AXE_STITCH].posY)
                                RX_StepperStatus.screwpositions[headAdjustment.printbarNo][i][AXE_STITCH].posY += difference;
                        }
                    }
                }
                _CmdScrewing++;
                RX_StepperStatus.screwerinfo.screwed = TRUE;
                RX_StepperStatus.screw_count++;
                robi_lb702_handle_ctrl_msg(INVALID_SOCKET, CMD_ROBI_MOVE_Z_DOWN, NULL);
                _ScrewTime = rx_get_ticks() + max_Wait_Time;
            }
            break;

        case 8:
            if (RX_StepperStatus.screwerinfo.z_in_down)
            {
                _CmdScrewing = 0;
                _ScrewTime = 0;
            }
            break;
        }
    }
}

static void _search_all_screws()
{
    // static variablen mit _ anfangen!
    static int correction_value = 0;
    static int max_Wait_Time_Sledge = 100000;    // ms
    static int max_Wait_Time = 40000; // ms
    int test;
    int pos_min;
    int pos;
    
    //--- _SearchScrewNr to ... ---- code wird lesbarer
    // int printbar; // 0,1
    // int head;
    // int axis;

    // Was ist X, was Y?
    // Was ist SCREW_FRONT? angle? wieso nicht SCREW_ANGLE und SCREW_DIST benennen?
    // Wir sollten uns noch einigen: haben wir angle + dist oder angle + stitch (Ist im moment nicht konsequent, speziell auch in der Kamera nicht.)

    if ((!RX_StepperStatus.info.moving && !RX_StepperStatus.robinfo.moving && !RX_StepperStatus.screwerinfo.moving))
    {
        if (_SearchScrewNr >= HEADS_PER_COLOR * COLORS_PER_STEPPER * SCREWS_PER_HEAD)
        {
            Error(ERR_CONT, 0, "Screw %d does not exist", _SearchScrewNr - 1);
            _CmdSearchScrews = 0;
            _SearchScrewNr = 0;
            return;
        }

        int front_screw = (_SearchScrewNr + 1) % SCREWS_PER_HEAD;
        int head_nr = (((_SearchScrewNr % (SCREWS_PER_HEAD * HEADS_PER_COLOR)) + 1) / COLORS_PER_STEPPER) - 1;
        int head_Dist = (abs(CABLE_SCREW_POS_FRONT) - abs(CABLE_SCREW_POS_BACK)) / 8;
        int screw_Dist = 16400; // um

        if (_CmdSearchScrews == 0) correction_value = 0;
        if (_ScrewTime && rx_get_ticks() > _ScrewTime)
        {
            Error(ERR_CONT, 0, "Robot stock in searching screw step %d at screw %d", _CmdSearchScrews, _SearchScrewNr);
            _ScrewTime = 0;
            _CmdSearchScrews = 0;
            return;
        }

        RX_StepperStatus.screwerinfo.screw_reset = FALSE;

        switch (_CmdSearchScrews)
        {
        case 0:
            pos = head_nr + rob_fct_screw_head0;
            if (pos != _HeadScrewPos)
                lbrob_handle_ctrl_msg(INVALID_SOCKET, CMD_ROB_MOVE_POS, &pos);
            _CmdSearchScrews++;
            _ScrewTime = rx_get_ticks() + max_Wait_Time_Sledge;
            break;
        case 1:
            if (RX_StepperStatus.robinfo.ref_done && _HeadScrewPos == head_nr + rob_fct_screw_head0)
            {
                if (_SearchScrewNr == 0)
                    pos = SCREW_Y_FRONT + correction_value;
                else if (_SearchScrewNr == SCREWS_PER_HEAD * HEADS_PER_COLOR)
                    pos = RX_StepperCfg.robot[RX_StepperCfg.boardNo].screwclusters[0].posY + correction_value;
                else if (_SearchScrewNr == 1 || _SearchScrewNr == SCREWS_PER_HEAD * HEADS_PER_COLOR + 1)
                    pos = RX_StepperCfg.robot[RX_StepperCfg.boardNo].screwclusters[_SearchScrewNr / (SCREWS_PER_HEAD * HEADS_PER_COLOR)].posY + SCREW_Y_BACK - SCREW_Y_FRONT + correction_value;
                else
                    pos = _calculate_average_y_pos(_SearchScrewNr) + correction_value;
                // wenn Y-Pos nicht gespeichert: Position aus vorg�nger-Position + ... rechnen?
                _ScrewTime = rx_get_ticks() + max_Wait_Time;
                robi_lb702_handle_ctrl_msg(INVALID_SOCKET, CMD_ROBI_MOVE_TO_Y, &pos);
                _CmdSearchScrews++;
            }
            break;
        case 2:
            if (RX_StepperStatus.screwerinfo.y_in_pos)
            {
                if (_SearchScrewNr == 0)
                    pos = SCREW_X_LEFT;
                else if (_SearchScrewNr == SCREWS_PER_HEAD * HEADS_PER_COLOR)
                    pos = RX_StepperCfg.robot[RX_StepperCfg.boardNo].screwclusters[0].posX - SCREW_X_LEFT + SCREW_X_RIGHT;
                else if (_SearchScrewNr % (SCREWS_PER_HEAD * HEADS_PER_COLOR) == 1)
                    pos = RX_StepperCfg.robot[RX_StepperCfg.boardNo].screwclusters[_SearchScrewNr / (SCREWS_PER_HEAD * HEADS_PER_COLOR)].posX;
                else
                {
                    pos = RX_StepperCfg.robot[RX_StepperCfg.boardNo].screwpositions[_SearchScrewNr / (SCREWS_PER_HEAD * HEADS_PER_COLOR)]
                                  [((_SearchScrewNr - 2) % HEADS_PER_COLOR) / SCREWS_PER_HEAD]
                                  [(_SearchScrewNr - 2) % SCREWS_PER_HEAD].posX -
                          RX_StepperCfg.robot[RX_StepperCfg.boardNo].screwclusters[_SearchScrewNr / (SCREWS_PER_HEAD *HEADS_PER_COLOR)].posX;
                    int y_dist_old = ((((_SearchScrewNr - 1 + 1) / SCREWS_PER_HEAD) % (SCREWS_PER_HEAD * HEADS_PER_COLOR))) * head_Dist - ((_SearchScrewNr - 1) % SCREWS_PER_HEAD) * screw_Dist;

                    int y_dist = ((((_SearchScrewNr + 1) / SCREWS_PER_HEAD) % (SCREWS_PER_HEAD * HEADS_PER_COLOR))) * head_Dist - (_SearchScrewNr % SCREWS_PER_HEAD) * screw_Dist;
                    pos = RX_StepperCfg.robot[RX_StepperCfg.boardNo].screwclusters[_SearchScrewNr / (SCREWS_PER_HEAD * HEADS_PER_COLOR)].posX + pos * y_dist / y_dist_old;
                }
                // wenn X-Pos nicht gespeichert: Position aus vorg�nger-Position + ... rechnen ?
                _ScrewTime = rx_get_ticks() + max_Wait_Time;
                robi_lb702_handle_ctrl_msg(INVALID_SOCKET, CMD_ROBI_MOVE_TO_X, &pos);
                _CmdSearchScrews++;
            }
            break;
        case 3:
            if (RX_StepperStatus.screwerinfo.x_in_pos)
            {
                _ScrewTime = rx_get_ticks() + max_Wait_Time;
                if (!RX_StepperStatus.info.z_in_screw) lb702_handle_ctrl_msg(INVALID_SOCKET, CMD_LIFT_SCREW, NULL);
                _CmdSearchScrews++;
            }
            break;

        case 4:
            if (RX_StepperStatus.info.z_in_screw)
            {
                _ScrewTime = rx_get_ticks() + max_Wait_Time;
                robi_lb702_handle_ctrl_msg(INVALID_SOCKET, CMD_ROBI_MOVE_Z_UP, NULL);
                _CmdSearchScrews++;
                _TimeSearchScrew = rx_get_ticks();
            }
            break;

        case 5:
            if (RX_StepperStatus.screwerinfo.z_in_up)
            {
                if (_SearchScrewNr % (HEADS_PER_COLOR * SCREWS_PER_HEAD) == 0)
                {
                    RX_StepperCfg.robot[RX_StepperCfg.boardNo].screwclusters[_SearchScrewNr / (HEADS_PER_COLOR * SCREWS_PER_HEAD)].posX = RX_StepperStatus.screw_posX;
                    RX_StepperCfg.robot[RX_StepperCfg.boardNo].screwclusters[_SearchScrewNr / (HEADS_PER_COLOR * SCREWS_PER_HEAD)].posY = RX_StepperStatus.screw_posY;
                    RX_StepperStatus.screwclusters[_SearchScrewNr / (HEADS_PER_COLOR * SCREWS_PER_HEAD)] = RX_StepperCfg.robot[RX_StepperCfg.boardNo].screwclusters[_SearchScrewNr / (HEADS_PER_COLOR * SCREWS_PER_HEAD)];
                }
                else
                {
                    RX_StepperCfg.robot[RX_StepperCfg.boardNo].screwpositions[_SearchScrewNr /(SCREWS_PER_HEAD * HEADS_PER_COLOR)]
                                       [((_SearchScrewNr - 1) % (SCREWS_PER_HEAD * HEADS_PER_COLOR)) / SCREWS_PER_HEAD]
                                       [(_SearchScrewNr - 1) % SCREWS_PER_HEAD].posX = RX_StepperStatus.screw_posX;
                    RX_StepperCfg.robot[RX_StepperCfg.boardNo].screwpositions[_SearchScrewNr /(SCREWS_PER_HEAD * HEADS_PER_COLOR)]
                                       [((_SearchScrewNr - 1) % (SCREWS_PER_HEAD * HEADS_PER_COLOR)) / SCREWS_PER_HEAD]
                                       [(_SearchScrewNr - 1) % SCREWS_PER_HEAD].posY = RX_StepperStatus.screw_posY;

                    RX_StepperStatus.screwpositions[_SearchScrewNr / (SCREWS_PER_HEAD * HEADS_PER_COLOR)]
                                       [((_SearchScrewNr - 1) % (SCREWS_PER_HEAD * HEADS_PER_COLOR)) / SCREWS_PER_HEAD][(_SearchScrewNr - 1) % SCREWS_PER_HEAD] =
                        RX_StepperCfg.robot[RX_StepperCfg.boardNo].screwpositions[_SearchScrewNr / (SCREWS_PER_HEAD * HEADS_PER_COLOR)]
                                [((_SearchScrewNr - 1) % (SCREWS_PER_HEAD * HEADS_PER_COLOR)) / SCREWS_PER_HEAD]
                                [(_SearchScrewNr - 1) % SCREWS_PER_HEAD];
                }
                // wenn gefunden: neues Telegram zum speichern der pos ans main (nicht �ber x Bedingungen aus dem Status zur�cklesen)

                _ScrewTime = 0;
                _CmdSearchScrews = 0;
                _SearchScrewNr++;
                _TimeSearchScrew = 0;
                correction_value = 0;
                if (_SearchScrewNr < 4 * HEADS_PER_COLOR)
                {
                    lbrob_handle_ctrl_msg(INVALID_SOCKET, CMD_SEARCH_ALL_SCREWS, NULL);
                }
                else
                {
                    _SearchScrewNr = 0;
                    RX_StepperStatus.screwerinfo.screws_found = TRUE;
                    robi_lb702_handle_ctrl_msg(INVALID_SOCKET, CMD_ROBI_MOVE_Z_DOWN, NULL);
                    Error(LOG, 0, "All Screws found");
                }
            }
            else if (_TimeSearchScrew && rx_get_ticks() > _TimeSearchScrew + SCREW_SEARCHING_TIME)
            {
                _ScrewTime = 0;
                _TimeSearchScrew = 0;
                if (abs(correction_value) >= 5000)
                {
                    Error(ERR_CONT, 0, "Screw %d not found", _SearchScrewNr);
                    _CmdSearchScrews = 0;
                    break;
                }
                else if (correction_value >= 0)
                    correction_value = (correction_value + 1000) * (-1);
                else
                    correction_value *= (-1);
                _CmdSearchScrews = 1;
            }
            break;
        }
    }
}

//---  _set_screws_to_zero
static void _set_screws_to_zero(void)
{
    static int correction_value = 0;
    static int max_Wait_Time = 40000; // ms
    static int max_Wait_Time_Sledge = 100000; // ms
    static int max_Wait_Time_Screw = 180000; // ms
    int pos;
    
    if ((!RX_StepperStatus.info.moving && !RX_StepperStatus.robinfo.moving && !RX_StepperStatus.screwerinfo.moving))
    {
        if (_ResetScrewNr >= HEADS_PER_COLOR * COLORS_PER_STEPPER * SCREWS_PER_HEAD)
        {
            Error(ERR_CONT, 0, "Screw %d does not exist", _ResetScrewNr - 1);
            _CmdResetScrews = 0;
            _ResetScrewNr = 0;
            return;
        }

        int printbar = _ResetScrewNr / (HEADS_PER_COLOR * SCREWS_PER_HEAD);
        int front_screw = (_ResetScrewNr + 1) % SCREWS_PER_HEAD;
        int head_nr = (((_ResetScrewNr % (SCREWS_PER_HEAD * HEADS_PER_COLOR)) + 1) / COLORS_PER_STEPPER) - 1;

        if (_CmdResetScrews == 0) correction_value = 0;
        if (_ScrewTime && rx_get_ticks() > _ScrewTime)
        {
            Error(ERR_CONT, 0, "Robot stock in zeroing screw step %d at screw %d", _CmdResetScrews, _ResetScrewNr);
            _ScrewTime = 0;
            _CmdResetScrews = 0;
            return;
        }

        switch (_CmdResetScrews)
        {
        case 0:
            pos = head_nr + rob_fct_screw_head0;
            if (pos != _HeadScrewPos)
                lbrob_handle_ctrl_msg(INVALID_SOCKET, CMD_ROB_MOVE_POS, &pos);
            _CmdResetScrews++;
            _ScrewTime = rx_get_ticks() + max_Wait_Time_Sledge;
            break;
        case 1:
            if (RX_StepperStatus.robinfo.ref_done && _HeadScrewPos == head_nr + rob_fct_screw_head0)
            {
                if (_ResetScrewNr % (HEADS_PER_COLOR * SCREWS_PER_HEAD) == 0)
                {
                    if (RX_StepperCfg.robot[RX_StepperCfg.boardNo].screwclusters[printbar].posY)
                        pos = RX_StepperCfg.robot[RX_StepperCfg.boardNo].screwclusters[_ResetScrewNr / (HEADS_PER_COLOR * SCREWS_PER_HEAD)].posY + correction_value;
                    else
                        pos = SCREW_Y_FRONT + correction_value;
                }
                else if (RX_StepperCfg.robot[RX_StepperCfg.boardNo].screwpositions[printbar][head_nr][front_screw].posY)
                    pos = RX_StepperCfg.robot[RX_StepperCfg.boardNo].screwpositions[printbar][head_nr][front_screw].posY + correction_value;
                else if (front_screw)
                    pos = SCREW_Y_FRONT + correction_value;
                else
                    pos = SCREW_Y_BACK + correction_value;
                
                _ScrewTime = rx_get_ticks() + max_Wait_Time;
                robi_lb702_handle_ctrl_msg(INVALID_SOCKET, CMD_ROBI_MOVE_TO_Y, &pos);
                _CmdResetScrews++;
            }
            break;
        case 2:
            if (RX_StepperStatus.screwerinfo.y_in_pos)
            {
                if (_ResetScrewNr % (HEADS_PER_COLOR * SCREWS_PER_HEAD) == 0)
                {
                    if (RX_StepperCfg.robot[RX_StepperCfg.boardNo].screwclusters[printbar].posX)
                        pos = RX_StepperCfg.robot[RX_StepperCfg.boardNo].screwclusters[printbar].posX;
                    else if (printbar == LEFT)
                        pos = SCREW_X_LEFT;
                    else
                        pos = SCREW_X_RIGHT;
                }
                else if (RX_StepperCfg.robot[RX_StepperCfg.boardNo].screwpositions[printbar][head_nr][front_screw].posX)
                    pos = RX_StepperCfg.robot[RX_StepperCfg.boardNo].screwpositions[printbar][head_nr][front_screw].posX;
                else if (printbar == LEFT)
                    pos = SCREW_X_LEFT;
                else
                    pos = SCREW_X_RIGHT;
                _ScrewTime = rx_get_ticks() + max_Wait_Time;
                robi_lb702_handle_ctrl_msg(INVALID_SOCKET, CMD_ROBI_MOVE_TO_X, &pos);
                _CmdResetScrews++;
            }
            break;
        case 3:
            if (RX_StepperStatus.screwerinfo.x_in_pos)
            {
                _ScrewTime = rx_get_ticks() + max_Wait_Time;
                if (!RX_StepperStatus.info.z_in_screw) lb702_handle_ctrl_msg(INVALID_SOCKET, CMD_LIFT_SCREW, NULL);
                _CmdResetScrews++;
            }
            break;

        case 4:
            if (RX_StepperStatus.info.z_in_screw)
            {
                _ScrewTime = rx_get_ticks() + max_Wait_Time;
                robi_lb702_handle_ctrl_msg(INVALID_SOCKET, CMD_ROBI_MOVE_Z_UP, NULL);
                _CmdResetScrews++;
                _TimeSearchScrew = rx_get_ticks();
            }
            break;
            
        case 5:
            if (RX_StepperStatus.screwerinfo.z_in_up && !robi_lb702_screw_correction())
            {
                correction_value = 0;
                int screwSteps;
                if (front_screw)
                {
                    screwSteps = 6 * 35;
                    if (_ResetInfinity % 2 == 0)
                        robi_lb702_handle_ctrl_msg(INVALID_SOCKET, CMD_ROBI_SCREW_LEFT, &screwSteps);
                    else
                        robi_lb702_handle_ctrl_msg(INVALID_SOCKET, CMD_ROBI_SCREW_RIGHT, &screwSteps);
                }
                else
                {
                    screwSteps = 6 * 21;
                    if (_ResetInfinity % 2 == 0)
                        robi_lb702_handle_ctrl_msg(INVALID_SOCKET, CMD_ROBI_SCREW_LEFT, &screwSteps);
                    else
                        robi_lb702_handle_ctrl_msg(INVALID_SOCKET, CMD_ROBI_SCREW_RIGHT, &screwSteps);
                }

                _CmdResetScrews++;
                _ScrewTime = rx_get_ticks() + max_Wait_Time_Screw;
            }
            else if (_TimeSearchScrew && rx_get_ticks() > _TimeSearchScrew + SCREW_SEARCHING_TIME)
            {
                _TimeSearchScrew = 0;
                if (abs(correction_value) >= 3000)
                {
                    Error(ERR_CONT, 0, "Screw %d of Head %d of Printhead %d not found", front_screw, head_nr, printbar);
                    _CmdScrewing = 0;
                    break;
                }
                else if (correction_value >= 0)
                    correction_value = (correction_value + 500) * (-1);
                else
                    correction_value *= (-1);
                _CmdResetScrews = 1;
                _ScrewTime = rx_get_ticks() + max_Wait_Time;
            }
            
        case 6:
            if (RX_StepperStatus.screwerinfo.screw_in_0)
            {
                int i, difference;
                if (_ResetScrewNr % (HEADS_PER_COLOR * SCREWS_PER_HEAD) == 0)
                {
                    RX_StepperStatus.screwclusters[printbar].posX = RX_StepperStatus.screw_posX;
                    RX_StepperStatus.screwclusters[printbar].posY = RX_StepperStatus.screw_posY;
                    
                    for (i = head_nr + 1; i < HEADS_PER_COLOR; i++)
                    {
                        if (RX_StepperCfg.robot[RX_StepperCfg.boardNo].screwclusters[printbar].posY)
                            difference = (RX_StepperStatus.screw_posY - RX_StepperCfg.robot[RX_StepperCfg.boardNo].screwclusters[printbar].posY);
                        else
                            difference = 0;
                        if (RX_StepperStatus.screwpositions[printbar][i][AXE_ANGLE].posY)
                            RX_StepperStatus.screwpositions[printbar][i][AXE_ANGLE].posY += difference;
                        if (RX_StepperStatus.screwpositions[printbar][i][AXE_STITCH].posY)
                            RX_StepperStatus.screwpositions[printbar][i][AXE_STITCH].posY += difference;
                    }
                }
                else
                {
                    RX_StepperStatus.screwpositions[printbar][head_nr][front_screw].posX = RX_StepperStatus.screw_posX;
                    RX_StepperStatus.screwpositions[printbar][head_nr][front_screw].posY = RX_StepperStatus.screw_posY;
                    
                    if (front_screw)
                    {
                        for (i = head_nr + 1; i < HEADS_PER_COLOR; i++)
                        {
                            if (RX_StepperCfg.robot[RX_StepperCfg.boardNo].screwpositions[printbar][head_nr][AXE_STITCH].posY)
                                difference = (RX_StepperStatus.screw_posY - RX_StepperCfg.robot[RX_StepperCfg.boardNo].screwpositions[printbar][head_nr][AXE_STITCH].posY);
                            else
                                difference = 0;
                            if (RX_StepperStatus.screwpositions[printbar][i][AXE_ANGLE].posY)
                                RX_StepperStatus.screwpositions[printbar][i][AXE_ANGLE].posY += difference;
                            if (i != (HEADS_PER_COLOR - 1) && RX_StepperStatus.screwpositions[printbar][i][AXE_STITCH].posY)
                                RX_StepperStatus.screwpositions[printbar][i][AXE_STITCH].posY += difference;
                        }
                    }
                }

                RX_StepperStatus.screwNr_reset = _ResetScrewNr;
                RX_StepperStatus.screwerinfo.screw_reset = TRUE;
                _ScrewTime = 0;
                _CmdResetScrews = 0;
                _ResetScrewNr++;
                _TimeSearchScrew = 0;
                correction_value = 0;
                if ((_ResetScrewNr < COLORS_PER_STEPPER * SCREWS_PER_HEAD * HEADS_PER_COLOR && !_ResetScrewsLeft) || (_ResetScrewsLeft && _ResetScrewNr < SCREWS_PER_HEAD * HEADS_PER_COLOR))
                {
                    lbrob_handle_ctrl_msg(INVALID_SOCKET, CMD_RESET_ALL_SCREWS, NULL);
                }
                else if (_ResetInfinity && (_ResetScrewsLeft || !_ResetScrewsRight))
                {
                    _ResetInfinity++;
                    _ResetScrewNr = 0;
                    lbrob_handle_ctrl_msg(INVALID_SOCKET, CMD_RESET_ALL_SCREWS, NULL);
                }
                else if (_ResetInfinity && _ResetScrewsRight)
                {
                    _ResetInfinity++;
                    _ResetScrewNr = SCREWS_PER_HEAD * HEADS_PER_COLOR;
                    lbrob_handle_ctrl_msg(INVALID_SOCKET, CMD_RESET_ALL_SCREWS, NULL);
                }
                else
                {
                    _ResetScrewNr = 0;
                    robi_lb702_handle_ctrl_msg(INVALID_SOCKET, CMD_ROBI_MOVE_Z_DOWN, NULL);
                    Error(LOG, 0, "All Screws set to 0");
                }
            }
            break;
        }
    }
}

//--- _check_in_screw_pos --------------------------------------
static int _check_in_screw_pos(SHeadAdjustment headAdjustment)
{
    if ((abs(RX_StepperStatus.screw_posX - SCREW_X_LEFT) > MAX_VAR_SCREW_POS &&
         abs(RX_StepperStatus.screw_posX - SCREW_X_RIGHT) > MAX_VAR_SCREW_POS) ||
        (abs(RX_StepperStatus.screw_posY - SCREW_Y_FRONT) > MAX_VAR_SCREW_POS &&
         abs(RX_StepperStatus.screw_posX - SCREW_Y_BACK) > MAX_VAR_SCREW_POS))
        return FALSE;

    if (abs(RX_StepperStatus.screw_posX - SCREW_X_LEFT) > MAX_VAR_SCREW_POS &&
        headAdjustment.printbarNo == LEFT)
        return FALSE;

    if (abs(RX_StepperStatus.screw_posX - SCREW_X_RIGHT) > MAX_VAR_SCREW_POS &&
        headAdjustment.printbarNo == RIGHT)
        return FALSE;

    if (abs(RX_StepperStatus.screw_posY - SCREW_Y_BACK) > MAX_VAR_SCREW_POS &&
        headAdjustment.axis == AXE_ANGLE)
        return FALSE;

    if (abs(RX_StepperStatus.screw_posY - SCREW_Y_FRONT) > MAX_VAR_SCREW_POS &&
        headAdjustment.axis == AXE_STITCH)
        return FALSE;

    int headNr;
    int pos = RX_StepperStatus.posY[0];

    for (headNr = 0; headNr < HEADS_PER_COLOR; headNr++)
    {
        if (abs(pos - (CABLE_SCREW_POS_BACK + ((headNr + 1) * (CABLE_SCREW_POS_FRONT - CABLE_SCREW_POS_BACK)) / 8)) < MAX_VAR_SCREW_POS)
        {
            if (headNr == headAdjustment.headNo)
                return TRUE;
            else
                return FALSE;
        }
    }
    return FALSE;
}

static int _calculate_average_y_pos(int screwNr)
{
    int i;
    int printbarNo = 0;
    int headNo = 0;
    int axis = 0;

    int y_combined = 0;

    if (screwNr % (SCREWS_PER_HEAD * HEADS_PER_COLOR) == 2)
        return RX_StepperCfg.robot[RX_StepperCfg.boardNo].screwclusters[screwNr / (SCREWS_PER_HEAD * HEADS_PER_COLOR)].posY;

    if (screwNr % (SCREWS_PER_HEAD * HEADS_PER_COLOR) == 3)
        return RX_StepperCfg.robot[RX_StepperCfg.boardNo].screwpositions[screwNr / (SCREWS_PER_HEAD * HEADS_PER_COLOR)][0][0].posY;

    if (screwNr < SCREWS_PER_HEAD * HEADS_PER_COLOR)
        i = (screwNr % SCREWS_PER_HEAD);
    else
        i = SCREWS_PER_HEAD * HEADS_PER_COLOR + (screwNr % SCREWS_PER_HEAD);

    for (; i < screwNr - 1; i += SCREWS_PER_HEAD)
    {
        if (i == 0 || i == 16)
            y_combined += RX_StepperCfg.robot[RX_StepperCfg.boardNo].screwclusters[i / (SCREWS_PER_HEAD * HEADS_PER_COLOR)].posY;
        else
        {
            printbarNo = i / (SCREWS_PER_HEAD * HEADS_PER_COLOR);
            headNo = ((i - 1) % (SCREWS_PER_HEAD * HEADS_PER_COLOR)) / SCREWS_PER_HEAD;
            axis = (i + 1) % SCREWS_PER_HEAD;
            y_combined += RX_StepperCfg.robot[RX_StepperCfg.boardNo].screwpositions[printbarNo][headNo][axis].posY;
        }
    }
    return y_combined / ((screwNr % (SCREWS_PER_HEAD * HEADS_PER_COLOR)) / SCREWS_PER_HEAD);
}

//--- _set_Screwer_Cfg ---------------------------------------
static void _set_Screwer_Cfg(SRobotOffsets screw_Cfg)
{
    memcpy(&RX_StepperCfg.robot[RX_StepperCfg.boardNo].screwclusters, screw_Cfg.screwclusters, sizeof(screw_Cfg.screwclusters));
    memcpy(&RX_StepperCfg.robot[RX_StepperCfg.boardNo].screwpositions, screw_Cfg.screwpositions, sizeof(screw_Cfg.screwpositions));
}

//--- _handle_waste_pump -----------------------------------------
static void _handle_waste_pump(void)
{
    if (rx_get_ticks() >= _WastePumpTimer && _WastePumpTimer && RX_StepperStatus.info.vacuum_running)
    {
        Fpga.par->output &= ~RO_VACUUM_CLEANER;
        _WastePumpTimer = rx_get_ticks() + WASTE_PUMP_TIME;
    }
    else if (rx_get_ticks() >= _WastePumpTimer && _WastePumpTimer && !RX_StepperStatus.info.vacuum_running)
    {
        Fpga.par->output &= ~RO_WASTE_VAC;
        _WastePumpTimer = 0;
    }
}

//--- _handle_ink_pump_back --------------------------------------------
#define TIMER_INK_PUMP_CHECK    1000
static void _handle_ink_pump_back(void)
{
    int val;
    static int _timer_left = 0;
    static int _timer_right = 0;
    static int _lastTime = 0;
    int _VentSpeed = 3;

    if ((_PumpTimeLeft && rx_get_ticks() >= _PumpTimeLeft) || (Fpga.par->output & RO_INK_VALVE_LEFT && RX_StepperStatus.robinfo.moving))
    {
        Fpga.par->output &= ~RO_INK_VALVE_LEFT;
        _PumpTimeLeft = 0;
    }

    if ((_PumpTimeRight && rx_get_ticks() >= _PumpTimeRight) || (Fpga.par->output & RO_INK_VALVE_RIGHT && RX_StepperStatus.robinfo.moving))
    {
        Fpga.par->output &= ~RO_INK_VALVE_RIGHT;
        _PumpTimeRight = 0;
    }
    

    if ((RX_StepperStatus.robinfo.moving) && (Fpga.par->output & RO_INK_VALVE_LEFT || Fpga.par->output & RO_INK_VALVE_RIGHT))
    {
        val = 0;
        lbrob_handle_ctrl_msg(INVALID_SOCKET, CMD_ROB_EMPTY_WASTE, &val);
    }
    
    if (Fpga.par->output & RO_INK_VALVE_LEFT)
    {
        _set_ink_back_pump_left(_PumpSpeed);
    }
    else if (!(Fpga.par->output & RO_INK_VALVE_LEFT))
    {
        _set_ink_back_pump_left(0);
    }

    if (Fpga.par->output & RO_INK_VALVE_RIGHT)
        {
        _set_ink_back_pump_right(_PumpSpeed);
        }
    else if (!(Fpga.par->output & RO_INK_VALVE_LEFT))
        {
        _set_ink_back_pump_right(0);
        }
    }

static void _set_ink_back_pump_left(int voltage)
    {
    _PumpValueLeft = voltage;
    _pump_main();
    }
    
static void _set_ink_back_pump_right(int voltage)
    {
    _PumpValueRight = voltage;
    _pump_main();
    }

//--- _pump_main -------------------------------------------
static void _pump_main()
    {
    static int _lastTime = 0;
    static int time_left = 0;
    static int time_right = 0;
    int time = rx_get_ticks();
    static int cnt = 0;
    static int _newValueLeft = 0;
    static int _oldValueLeft = 0;
    static int _newValueRight = 0;
    static int _oldValueRight = 0;

    if (RX_StepperStatus.inkinfo.ink_pump_error_left)   _PumpValueLeft = 0;
    if (RX_StepperStatus.inkinfo.ink_pump_error_right)  _PumpValueRight = 0;

    if (_PumpValueLeft)
    {
        _newValueLeft = Fpga.stat->analog_in[AI_INK_PUMP_LEFT];
        if (abs(_newValueLeft - _oldValueLeft) >= 50)
        {
            time_left = rx_get_ticks();
        }
        _oldValueLeft = _newValueLeft;
        if (rx_get_ticks() >= time_left + 1000)
        {
            Error(LOG, 0, "Ink-Pump Left is blocked");
            RX_StepperStatus.inkinfo.ink_pump_error_left = TRUE;
            _PumpValueLeft = 0;
        }
    }
    else
        time_left = rx_get_ticks();

    if (_PumpValueRight)
    {
        _newValueRight = Fpga.stat->analog_in[AI_INK_PUMP_RIGHT];
        if (abs(_newValueRight - _oldValueRight) >= 50)
        {
            time_right = rx_get_ticks();
        }
        _oldValueRight = _newValueRight;
        if (rx_get_ticks() >= time_right + 1000)
        {

            Error(LOG, 0, "Ink-Pump Right is blocked");
            RX_StepperStatus.inkinfo.ink_pump_error_right = TRUE;
            _PumpValueRight = 0;
        }
    }
    else
        time_right = rx_get_ticks();


    if (time - _lastTime > 100)
    {
        if (_PumpValueLeft < 1)
    {
            Fpga.par->output &= ~RO_INK_PUMP_LEFT;
            Fpga.par->pwm_output[PWM_INK_PUMP_LEFT] = 0;
        }
        else if (_PumpValueLeft < 100)  Fpga.par->pwm_output[3] = (0x10000 * _PumpValueLeft) / 100;
        else
        {
            Fpga.par->pwm_output[PWM_INK_PUMP_LEFT] = 0;
            Fpga.par->output |= RO_INK_PUMP_LEFT;
        }

        if (_PumpValueRight < 1)
        {
            Fpga.par->output &= ~RO_INK_PUMP_RIGHT;
            Fpga.par->pwm_output[PWM_INK_PUMP_RIGHT] = 0;
        }
        else if (_PumpValueRight < 100)
            Fpga.par->pwm_output[PWM_INK_PUMP_RIGHT] = (0x10000 * _PumpValueRight) / 100;
        else                             
        {
            Fpga.par->pwm_output[4] = 0;
            Fpga.par->output |= RO_INK_PUMP_RIGHT;
        }
        _lastTime = time;
    }
}

void lbrob_reset_variables(void)
{
    _NewCmd = FALSE;
}

static void _vacuum_on()
{
    Fpga.par->output |= RO_WASTE_VAC;
    Fpga.par->output |= RO_VACUUM_CLEANER;
    _WastePumpTimer = 0;
}

static void _vacuum_off()
{
    if (_WastePumpTimer) _WastePumpTimer = rx_get_ticks();
}

//--- _head_screw_pos ------------------------------------------
int _head_screw_pos(void)
{
    return _HeadScrewPos;
}

//--- _lbrob_motor_test ---------------------------------
static void _lbrob_motor_test(int motorNo, int steps)
{
    if (motorNo <= 1) return;

    int motors = 1 << motorNo;
    SMovePar par;

    memset(&par, 0, sizeof(SMovePar));

    if (motorNo == 4)
    {
        par.speed = 2000;
        par.accel = 4000;
        par.current_acc = 420.0;
        par.current_run = 420.0;
        par.enc_bwd = TRUE;
        par.encCheck = chk_off;

        _CmdRunning = 1; // TEST
        RX_StepperStatus.info.moving = TRUE;

        motors_config(motors, CURRENT_HOLD, X_STEPS_PER_REV, X_INC_PER_REV,
                      STEPS);
        motors_move_by_step(motors, &par, steps, FALSE);
    }
    else if (motorNo >= 2 && motorNo <= 3)
    {
        par.speed = 6000;
        par.accel = 10000;
        par.current_acc = 280.0;
        par.current_run = 280.0;
        par.encCheck = chk_off;

        motors_config(motors, CURRENT_HOLD, 0, 0, STEPS);
        motors_move_by_step(motors, &par, steps, FALSE);
    }
}
