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
#include "rx_trace.h"
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
#include "robot_client.h"

#define MOTOR_X_0               4

#define MOTOR_X_BITS            0x10
#define MOTOR_ALL_BITS          0x13

#define X_STEPS_PER_REV         3200.0
#define X_INC_PER_REV           16000.0
#define X_DIST_PER_REV          54000   // 36000

#define CABLE_MAINTENANCE_POS   -770000     //  um LB702
#define CABLE_CAP_POS           -687000     //	um LB702
#define CABLE_WASH_POS_FRONT    -634000     //	um LB702
#define CABLE_WASH_POS_BACK     -221000     //	um LB702
#define CABLE_PURGE_POS_BACK    -311000     //  um LB702
#define VACUUM_POS              -400000     //  um LB702
#define CABLE_PURGE_POS_FRONT   -622000     //  um LB702    CABLE_PURGE_POS_BACK - (7 * HEAD_WIDTH) - 10000 ->  HEAD_WIDTH = 43000
#define CABLE_SCREW_POS_FRONT   -503000     //  um LB702    
#define CABLE_SCREW_POS_BACK    -156208     //  um LB702    CABLE_SCREW_POS_BACK + (8 * HEAD_WIDTH) ->  HEAD_WIDTH = 43349

#define ROB_POSY_FREE           40000   // minimum y-position to allow moving in x

#define CURRENT_HOLD            200

#define CAP_FILL_TIME           16000       // ms
#define SCREW_SEARCHING_TIME    1000        // ms
#define VACUUM_PUMP_TIME        25000       // ms
#define WASTE_PUMP_TIME         30000       // ms

#define DISTANCE_PER_TURN       11          // um
#define STEPS_PER_TURN          6           // steps

#define MAX_WAIT_TIME_SLEDGE    100000  // ms
#define MAX_WAIT_TIME           40000   // ms
#define MAX_WAIT_TIME_SCREW     180000  // ms		// with TCP/IP 100000 is enough

// Digital Inputs
#define CABLE_PULL_REF          2
#define CAPPING_ENDSTOP         3
#define DI_INK_PUMP_REED_LEFT   4
#define DI_INK_PUMP_REED_RIGHT  5

// Digital Outputs
#define RO_ALL_FLUSH_OUTPUTS    0x02F       // All used outputs -> o0, o1, o2, o3, o5
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

#define MAX_STEPS_STITCH    (30 * 6)  // 30 turns with 6 steps each turn
#define MAX_STEPS_ANGLE     (16 * 6)  // 16 turns with 6 steps each turn
#define OVERTURN            (2*6)     // 2 turns
#define LOOSE_TURN          (6/2)     // half turn for reserve, that the mechanical end should never be reached anymore

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
static char     *_AxisName[2]  = {"ANGLE", "STITCH"};
static char     _CmdName[32];
static int      _CmdRunning = 0;
static int      _NewCmd = 0;

static ERobotFunctions _RobFunction = 0;
static ERobotFunctions _Old_RobFunction = 0;
static int _ScrewFunction=0;
static int _ScrewDist;  
static int  _ScrewTurned;
static int _RobStateMachine_Step = 0;
static SHeadAdjustment _ScrewPar;

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
static int  _CapIsWet = TRUE;

static SScrewPositions _ScrewPos;
static int             _ScrewerStalled;

//--- prototypes --------------------------------------------
static void _lbrob_motor_z_test(int steps);
static void _lbrob_motor_test(int motor, int steps);
static void _lbrob_do_reference(void);
static void _lbrob_move_to_pos(int cmd, int pos, int wipe_state);
static int  _micron_2_steps(int micron);
static int  _steps_2_micron(int steps);
static void _cln_move_to(int msgId, ERobotFunctions fct);
static void _rob_state_machine(void);
static void _set_ScrewPos(SScrewPositions *pos);
static void _save_ScrewPos(int dist);
static void _handle_waste_pump(void);
static void _handle_flush_pump(void);
static void _handle_ink_pump_back(void);
static void _set_ink_back_pump_left(int voltage);
static void _set_ink_back_pump_right(int voltage);
static void _vacuum_on();
static void _vacuum_off();
static void _pump_main();


//--- lbrob_init --------------------------------------
void lbrob_init(void)
{
    /*
#ifdef DEBUG
    _CapIsWet = FALSE;
#endif
*/
    memset(_CmdName, 0, sizeof(_CmdName));
    memset(&_ParCable_ref, 0, sizeof(SMovePar));
    memset(&_ParCable_drive, 0, sizeof(SMovePar));
    memset(&_ParCable_drive_purge, 0, sizeof(SMovePar));
    
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
    rc_init();
    rc_config(RX_StepperCfg.boardNo);
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
    rc_main(ticks, menu);

    _handle_ink_pump_back();
    _handle_waste_pump();
    _handle_flush_pump();

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

    if (!RX_StepperStatus.robinfo.ref_done)
    {
        _HeadScrewPos = FALSE;
        _CapIsWet = TRUE;
    }
    

    if (_CmdRunning && motors_move_done(MOTOR_X_BITS))
    {
		if (_CapIsWet)
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
                    _CmdRunning = 0;
                    _CmdRunning_Lift = 0;
                    _NewCmd = 0;
                    Fpga.par->output &= ~RO_ALL_FLUSH_OUTPUTS;
                }
                if (RX_StepperStatus.info.x_in_ref)
                {
                    motors_reset(MOTOR_X_BITS);
                    RX_StepperStatus.robinfo.ref_done = TRUE;
                    _CapIsWet = FALSE;
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
                else
                    _CapIsWet = FALSE;
                RX_StepperStatus.cmdRunning = FALSE;
            }
        }

        if (_CmdRunning != CMD_ROB_REFERENCE)
        {
            if (motors_error(MOTOR_X_BITS, &motor))
            {
                RX_StepperStatus.robinfo.ref_done = FALSE;
                Error(ERR_CONT, 0, "Stepper: Command %s: Motor %s blocked", _CmdName, _MotorName[motor]);
                if (_CmdRunning==CMD_HEAD_ADJUST) RX_StepperStatus.adjustDoneCnt++;
                _CmdRunning = 0;
                _CmdRunning_Lift = 0;
                _NewCmd = 0;
                Fpga.par->output &= ~RO_ALL_FLUSH_OUTPUTS;
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
                    Fpga.par->output &= ~RO_ALL_FLUSH_OUTPUTS;
                    RX_StepperStatus.robinfo.cap_ready = RX_StepperStatus.info.x_in_cap && !RX_StepperStatus.robinfo.moving;
                    _CapFillTime = 0;
                }
                else if (rx_get_ticks() >= _CapFillTime + CAP_FILL_TIME / 2)
                {
                    if (RX_StepperCfg.printerType != printer_LB702_UV)   Fpga.par->output |= RO_FLUSH_TO_CAP_RIGHT;
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
                _CmdRunning = 0;
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
                _CmdRunning = 0;
                break;

            case rob_fct_maintenance:
                _CmdRunning = 0;
                break;

            case rob_fct_purge4ever:
                RX_StepperStatus.robinfo.x_in_purge4ever = fpga_input(CAPPING_ENDSTOP);
                _CapIsWet=TRUE;
                if (!RX_StepperStatus.robinfo.x_in_purge4ever && _NewCmd != CMD_ROB_MOVE_POS)
                {
                    Error(ERR_CONT, 0, "LBROB: Command %s: End Sensor Capping NOT HIGH", _CmdName);
                    RX_StepperStatus.robinfo.ref_done = FALSE;
                }
                _CmdRunning = FALSE;
                break;

            case rob_fct_purge_all:
            case rob_fct_purge_head0:
            case rob_fct_purge_head1:
            case rob_fct_purge_head2:
            case rob_fct_purge_head3:
            case rob_fct_purge_head4:
            case rob_fct_purge_head5:
            case rob_fct_purge_head6:
            case rob_fct_purge_head7:
                _CapIsWet=TRUE;
                RX_StepperStatus.robinfo.purge_ready = TRUE;
                _CmdRunning = 0;
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
                _CmdRunning = 0;
                break;
            case rob_fct_vacuum:
            case rob_fct_wash:
                _CapIsWet=TRUE;
                _Old_RobFunction = _RobFunction;
                _RobFunction = rob_fct_move;
                _CmdRunning = 0;
                _CmdRunning_old = 0;
                if (!RX_StepperStatus.screwerinfo.y_in_ref || !RX_StepperStatus.screwerinfo.robi_in_ref)
                {
                    _CmdRunning_Robi = CMD_ROBI_MOVE_TO_GARAGE;
                    _NewCmd = CMD_ROB_MOVE_POS;
                    robi_lb702_handle_ctrl_msg(INVALID_SOCKET, _CmdRunning_Robi, NULL);
                }
                else
                    lbrob_handle_ctrl_msg(INVALID_SOCKET, CMD_ROB_MOVE_POS, &_RobFunction);
                break;

            case rob_fct_move:
                _CmdRunning = 0;
                Fpga.par->output &= ~RO_ALL_FLUSH_OUTPUTS;
                switch (_Old_RobFunction)
                {
                case rob_fct_wash:
                    Error(LOG, 0, "Wash done");
                    RX_StepperStatus.robinfo.wash_done = TRUE;
                    break;
                case rob_fct_vacuum:
                    RX_StepperStatus.robinfo.vacuum_done = TRUE;
                    break;
                default:
                    break;
                }
                _Old_RobFunction = _RobFunction;
                break;

            default:
                Error(ERR_CONT, 0, "LBROB_MAIN: Rob-Function %d not implemented", _RobFunction);
                _CmdRunning = 0;
                RX_StepperStatus.robinfo.ref_done = FALSE;
                break;
            }
            if (!_CmdRunning_Lift && !(RX_StepperStatus.info.z_in_wash && _Old_RobFunction == rob_fct_wash) && _RobFunction != rob_fct_move)
                _Old_RobFunction = _RobFunction;
        }
        else if (_CmdRunning == CMD_ROB_REFERENCE && _CmdRunning_old)
        {
            int loc_new_cmd = 0;
            if (!RX_StepperStatus.robinfo.moving && !RX_StepperStatus.info.moving)
            {
                loc_new_cmd = _CmdRunning_old;
                _CmdRunning_old = 0;
                _CmdRunning = 0;
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
            if (_CmdRunning_Robi==CMD_ROBI_MOVE_Z_DOWN && !RX_StepperStatus.screwerinfo.z_in_down)
            {
                Error(ERR_CONT, 0, "Screwer blocked in UP position");
                lbrob_handle_menu("z5000");
            }
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
            case CMD_ROB_FILL_CAP:
                lbrob_handle_ctrl_msg(INVALID_SOCKET, CMD_ROB_MOVE_POS, &_RobFunction);
				break;
            default:
                Error(ERR_CONT, 0, "LBROB_MAIN: Command 0x%08x not implemented", loc_new_cmd);
                break;
            }
        }
        loc_new_cmd = FALSE;
    }

    static int j = 0;

    if (!_RobStateMachine_Step && !RX_StepperStatus.info.moving && !RX_StepperStatus.robinfo.moving && !RX_StepperStatus.screwerinfo.moving && 
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

    if (_RobStateMachine_Step) _rob_state_machine();
    else _ScrewTurned = TRUE;
   
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
    term_printf("LB Clean ---------------------------------\n");
    if (RX_StepperStatus.robot_used)
    {
        term_printf("moving: \t\t %d \t cmd: %08x\n", RX_StepperStatus.robinfo.moving, _CmdRunning);
        term_printf("reference done: \t %d\n", RX_StepperStatus.robinfo.ref_done);
        term_printf("x in reference: \t %d\n", RX_StepperStatus.info.x_in_ref);
        term_printf("x in cap: \t\t %d\n", RX_StepperStatus.info.x_in_cap);
        term_printf("Cap ready \t\t %d\n", RX_StepperStatus.robinfo.cap_ready);
        term_printf("Purge ready: \t\t %d\n", RX_StepperStatus.robinfo.purge_ready);
        term_printf("Slide pos: \t\t %dum\n", RX_StepperStatus.posY[0]);
        term_printf("Wipe-Speed: \t\t %d\n", RX_StepperCfg.wipe_speed);
        term_printf("Vacuum done: \t\t %d\n", RX_StepperStatus.robinfo.vacuum_done);
        term_printf("Wash done: \t\t %d\n", RX_StepperStatus.robinfo.wash_done);
        term_printf("adjustDoneCnt: \t\t %d\n", RX_StepperStatus.adjustDoneCnt);
        term_printf("State:\t\t\t %d \tprintbar=%d head=%d screw=%s\n", _RobStateMachine_Step, _ScrewPar.printbar, _ScrewPar.head, _AxisName[_ScrewPar.axis]);
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
        term_printf("M: Go to Maintenance-Pos\n");
        term_printf("w: Wash Heads\n");
        term_printf("q<n>: Stop(n=0)/Start(n=1) Vacuum Pump \n");
        term_printf("v: Vacuum Heads\n");
        term_printf("g<n>: Go to purge position of head 1 -8 or for all (0)\n");
        term_printf("p: Move the Robi for Purging all Heads\n");
        term_printf("m<n><steps>: move Motor<n> by <steps>\n");
        term_printf("f: Find all the screws\n");
        term_printf("a<n>: Go to adjustment position of head 0 - 7\n");
        term_printf("b<n>: Set Speed of Waste pump to <n>%, Actual value %d%\n", _PumpSpeed);
        term_printf("z: Move all Screws to middle position\n");
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
    case 'M':
        pos = rob_fct_maintenance;
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
		default:
			pos = rob_fct_purge_all;
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
        lbrob_handle_ctrl_msg(INVALID_SOCKET, CMD_FIND_ALL_SCREWS, NULL);
        break;
    case 'z':
        if (str[1] == 'r')
            val = 1;
        else
            val = 0;
        lbrob_handle_ctrl_msg(INVALID_SOCKET, CMD_RESET_ALL_SCREWS, &val);
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
    case 'b':
        val = atoi(&str[1]);
        if (val < 0) val = 0;
        if (val > 100) val = 100;
        _PumpSpeed = val;
        break;
    }
}

//--- _lbrob_move_to_pos ---------------------------------------------------------------
static void _lbrob_move_to_pos(int cmd, int pos, int wipe_state)
{
    _CmdRunning = cmd;
    RX_StepperStatus.robinfo.moving = TRUE;
    int actual_pos = motor_get_step(MOTOR_X_0);
    int moving_forward = FALSE;
    if (pos < actual_pos) moving_forward = TRUE;

    TrPrintfL(TRUE, "_lbrob_move_to_pos cmd=0x%08x, pos=%d, wipe_state=%d", cmd, pos, wipe_state);

    if (wipe_state)
    {
        if (RX_StepperCfg.wipe_speed)
            _ParCable_drive_purge.speed = _micron_2_steps(1000 * RX_StepperCfg.wipe_speed); // multiplied with 1000 to get from mm/s to um/s
        else
            _ParCable_drive_purge.speed = _micron_2_steps(1000 * 10); // multiplied with 1000 to get from mm/s to um/s
        motors_move_to_step(MOTOR_X_BITS, &_ParCable_drive_purge, pos);
    }
    else if (_CapIsWet && !moving_forward)
    {
        _vacuum_on();
        motors_move_to_step(MOTOR_X_BITS, &_ParCable_drive_slow, pos);
    }
        
    else
        motors_move_to_step(MOTOR_X_BITS, &_ParCable_drive, pos);
}

//--- _lbrob_do_reference ----------------------------------------------------------------------
static void _lbrob_do_reference()
{
    Fpga.par->output &= ~RO_ALL_FLUSH_OUTPUTS;
    _NewCmd = 0;
    RX_StepperStatus.robinfo.moving = TRUE;
    _CmdRunning = CMD_ROB_REFERENCE;
    if (!RX_StepperStatus.screwerinfo.z_in_down)
    {
        _CmdRunning_Robi = CMD_ROBI_MOVE_Z_DOWN;
        _NewCmd = _CmdRunning;
        robi_lb702_handle_ctrl_msg(INVALID_SOCKET, _CmdRunning_Robi, NULL);
    }
    else if (!RX_StepperStatus.robinfo.ref_done)
    {
        _vacuum_on();
        motor_reset(MOTOR_X_0);
        motor_config(MOTOR_X_0, CURRENT_HOLD, X_STEPS_PER_REV, X_INC_PER_REV, STEPS);
        RX_StepperStatus.robinfo.ref_done = FALSE;
        motors_move_by_step(1 << MOTOR_X_0, &_ParCable_ref, 1000000, TRUE);
    }
	else
        _lbrob_move_to_pos(CMD_ROB_REFERENCE, _micron_2_steps(3000), FALSE);
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
        Fpga.par->output &= ~RO_ALL_FLUSH_OUTPUTS;
        if (_CmdRunning==CMD_HEAD_ADJUST) RX_StepperStatus.adjustDoneCnt++;
        _CmdRunning_Lift = 0;
        _CmdRunning = 0;
        _RobStateMachine_Step = 0;
        _ScrewTime = 0;
        _CapIsWet = TRUE;
        RX_StepperStatus.robinfo.ref_done = FALSE;
        val = 0;
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

    case CMD_ROB_SERVICE:
        strcpy(_CmdName, "CMD_ROB_SERVICE");
        pos = rob_fct_maintenance;
        lbrob_handle_ctrl_msg(INVALID_SOCKET, CMD_ROB_MOVE_POS, &pos);
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
            _CapIsWet = TRUE;
            _CmdRunning = msgId;
            pos = *((INT32 *)pdata);
            _RobFunction = pos;
            switch (_RobFunction)
            {
            case rob_fct_cap:
                RX_StepperStatus.robinfo.moving = TRUE;
                Fpga.par->output &= ~RO_ALL_FLUSH_OUTPUTS;
                if (RX_StepperCfg.printerType != printer_LB702_UV) Fpga.par->output |= RO_FLUSH_TO_CAP_LEFT;
                Fpga.par->output |= RO_FLUSH_PUMP;
                _CapFillTime = rx_get_ticks();
                break;

            case rob_fct_purge_all:
                if (RX_StepperCfg.wipe_speed == 0)
                    Error(ERR_CONT, 0, "Wipe-Speed is set 0, please chose another value");

                if (!RX_StepperStatus.info.z_in_ref && !RX_StepperStatus.info.moving)
                    {
                        _CmdRunning_Lift = CMD_LIFT_REFERENCE;
                        lb702_handle_ctrl_msg(INVALID_SOCKET, _CmdRunning_Lift, NULL);
                        _NewCmd = msgId;
                        break;
                    }
                _vacuum_on();
                RX_StepperStatus.robinfo.moving = TRUE;
                _lbrob_move_to_pos(msgId, _micron_2_steps(CABLE_PURGE_POS_FRONT), TRUE);
                break;

            default:
                break;
            }
        }
        break;

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
                break;
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
        lbrob_to_garage();
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
    if (!_CmdRunning)
    {
        _RobFunction = fct;
        TrPrintfL(TRUE, "_cln_move_to msgId=0x%08x, fct=%d", msgId, fct);

        if (!RX_StepperStatus.screwerinfo.z_in_down)
        {
            _CmdRunning_Robi = CMD_ROBI_MOVE_Z_DOWN;
            _NewCmd = msgId;
            robi_lb702_handle_ctrl_msg(INVALID_SOCKET, _CmdRunning_Robi, NULL);
            return;
        }
        else if ((!(RX_StepperStatus.info.z_in_ref || RX_StepperStatus.info.z_in_screw) && !(_RobFunction == rob_fct_move && RX_StepperStatus.info.z_in_wash)) || !RX_StepperStatus.info.ref_done)
        {
            if (!RX_StepperStatus.info.moving)
            {
                _CmdRunning_Lift = CMD_LIFT_REFERENCE;
                lb702_handle_ctrl_msg(INVALID_SOCKET, _CmdRunning_Lift, NULL);
                _NewCmd = msgId;
            }
            return;
        }
        else if (!RX_StepperStatus.robinfo.ref_done || (_CapIsWet && _RobFunction != rob_fct_vacuum
            && _RobFunction != rob_fct_wash && _RobFunction != rob_fct_move
            && (_RobFunction < rob_fct_purge_all || _RobFunction > rob_fct_purge_head7)))
        {
            _CmdRunning_old = msgId;
            _lbrob_do_reference();
            return;
        }

        RX_StepperStatus.robinfo.moving = TRUE;
        _CmdRunning = msgId;
        switch (_RobFunction)
        {
        case rob_fct_purge4ever:
        case rob_fct_cap:
            _vacuum_on();
            _lbrob_move_to_pos(_CmdRunning, _micron_2_steps(CABLE_CAP_POS), FALSE);
            break;
            
        case rob_fct_maintenance:
            _lbrob_move_to_pos(_CmdRunning, _micron_2_steps(CABLE_MAINTENANCE_POS), FALSE);
            break;
            
        case rob_fct_purge_all:
            _lbrob_move_to_pos(_CmdRunning, _micron_2_steps(CABLE_PURGE_POS_BACK), FALSE);
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
            pos = (CABLE_PURGE_POS_BACK + (((int)_RobFunction - rob_fct_purge_head0) * (CABLE_PURGE_POS_FRONT - CABLE_PURGE_POS_BACK)) / 7);
            _lbrob_move_to_pos(_CmdRunning, _micron_2_steps(pos), FALSE);
            break;
        case rob_fct_vacuum:
            _lbrob_move_to_pos(_CmdRunning, _micron_2_steps(CABLE_PURGE_POS_FRONT), FALSE);
            break;
        case rob_fct_wash:
            _lbrob_move_to_pos(_CmdRunning, _micron_2_steps(CABLE_WASH_POS_BACK), FALSE);
            break;

        case rob_fct_move:
            if (!RX_StepperStatus.info.z_in_wash)
            {
                if (!RX_StepperStatus.info.moving && !RX_StepperStatus.screwerinfo.moving)
                {
                    _CmdRunning = 0;
                    RX_StepperStatus.robinfo.moving = FALSE;
                    _CmdRunning_Lift = CMD_LIFT_WASH_POS;
                    _NewCmd = msgId;
                    lb702_handle_ctrl_msg(INVALID_SOCKET, _CmdRunning_Lift, NULL);
                }
                return;
            }
            switch (_Old_RobFunction)
            {
            case rob_fct_wash:
                _lbrob_move_to_pos(_CmdRunning, _micron_2_steps(CABLE_PURGE_POS_FRONT), TRUE);
                Fpga.par->output |= RO_FLUSH_WIPE;
                Fpga.par->output |= RO_FLUSH_PUMP;
                Fpga.par->output &= ~RO_VACUUM_CLEANER;
                break;
            case rob_fct_vacuum:
                _lbrob_move_to_pos(_CmdRunning, _micron_2_steps(CABLE_WASH_POS_BACK), TRUE);
                _vacuum_on();
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
            pos = (CABLE_SCREW_POS_BACK + (((int)_RobFunction - rob_fct_screw_cluster) * (CABLE_SCREW_POS_FRONT - CABLE_SCREW_POS_BACK)) / 8);
            /* not needed
            if (RX_StepperStatus.posY[0] < pos)
            {
				_CmdRunning_old = msgId;
                _lbrob_do_reference();
                return;
            }
            */
            _lbrob_move_to_pos(_CmdRunning, _micron_2_steps(pos), FALSE);
            break;

        default:
            Error(ERR_CONT, 0, "Command %s: Rob-Function %d not implemented", _CmdName, _RobFunction);
        }
    }
}

//--- _rob_state_machine --------------------------------------------------------------------
static void _rob_state_machine(void)
{
    static SScrewPos _pos;
    static int       _posy;
    static int _correction_step;
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
        /*
        if (_ScrewTime && rx_get_ticks() > _ScrewTime)
        {
            Error(ERR_CONT, 0, "Robot stuck in searching screw step %d at screw (printbar %d, head %d, axis %d)", _RobStateMachine_Step, _ScrewPar.printbar, _ScrewPar.head, _ScrewPar.axis);
            _ScrewTime = 0;
            _RobStateMachine_Step = 0;
            RX_StepperStatus.adjustDoneCnt++;
            return;
        }
        */

        switch (_RobStateMachine_Step)
        {
        case 0: // new screw
            TrPrintfL(TRUE, "_rob_state_machine: State=%d", _RobStateMachine_Step);
            memset(&_pos, 0x00, sizeof(_pos));

            if (_ScrewPar.printbar<2)
            {
                int defaults = (_ScrewFunction==CMD_FIND_ALL_SCREWS);
                defaults = FALSE;
                _ScrewDist=0;
                if (_ScrewPar.head==-1)
                {
                    if (_ScrewPar.axis==AXE_STITCH)
                    {
                        memcpy(&_pos, &_ScrewPos.printbar[_ScrewPar.printbar].stitch, sizeof(_pos));
                        if (defaults || _pos.x==0)
                        {
							if (_ScrewPar.printbar == 0)
								_pos.x = SCREW_X_LEFT;
							else if (_ScrewPos.printbar[0].stitch.x == 0)
								_pos.x = SCREW_X_RIGHT;
                            else 
                                _pos.x = SCREW_X_RIGHT + _ScrewPos.printbar[0].stitch.x - SCREW_X_LEFT;
                            TrPrintfL(TRUE, "Default _pos.x=%d", _pos.x);
                        }
                        if (defaults || _pos.y<MIN_Y_POS)
                        {
							if (_ScrewPar.printbar == 0 || _ScrewPos.printbar[0].stitch.y < MIN_Y_POS) _pos.y = SCREW_Y_STITCH;
                            else _pos.y = _ScrewPos.printbar[0].stitch.y;

                            /*
                            if (_correction_step==0) 
                            {
                                Error(WARN, 0, "Test for searching screws");
                                _pos.y -= 5000;
                            }
                            */

                            TrPrintfL(TRUE, "Default _pos.y=%d", _pos.y);
                        }
                    }
                }
                else 
                {                
                    memcpy(&_pos, &_ScrewPos.printbar[_ScrewPar.printbar].head[_ScrewPar.head][_ScrewPar.axis], sizeof(_pos));
                    if (defaults || _pos.x==0)
                    {
                        if (_ScrewPar.axis==AXE_ANGLE)
                        {
                            if (_ScrewPar.head==0) _pos.x = _ScrewPos.printbar[_ScrewPar.printbar].stitch.x;
                            else                   _pos.x = _ScrewPos.printbar[_ScrewPar.printbar].head[_ScrewPar.head-1][AXE_ANGLE].x;
                        }
                        else _pos.x = _ScrewPos.printbar[_ScrewPar.printbar].head[_ScrewPar.head][AXE_ANGLE].x;
                        TrPrintfL(TRUE, "Default _pos.x=%d", _pos.x);
                    }
                    if (defaults || _pos.y < MIN_Y_POS)                
                    {                   
                        if (_ScrewPar.axis==AXE_ANGLE)
                        {
							if ((_ScrewPar.head == 0 && _ScrewPos.printbar[_ScrewPar.printbar].stitch.y < MIN_Y_POS) 
							        || (_ScrewPos.printbar[_ScrewPar.printbar].head[_ScrewPar.head - 1][AXE_ANGLE].y < MIN_Y_POS && _ScrewPar.head != 0))
								_pos.y = SCREW_Y_ANGLE;
							else if (_ScrewPar.head==0)                                                             
								_pos.y = _ScrewPos.printbar[_ScrewPar.printbar].stitch.y - SCREW_Y_STITCH + SCREW_Y_ANGLE;
                            else                                            
								_pos.y = _ScrewPos.printbar[_ScrewPar.printbar].head[_ScrewPar.head-1][AXE_ANGLE].y;
                        }
                        else _pos.y = _ScrewPos.printbar[_ScrewPar.printbar].head[_ScrewPar.head][AXE_ANGLE].y - SCREW_Y_ANGLE + SCREW_Y_STITCH;
                        TrPrintfL(TRUE, "Default PosY=%d", _pos.y);
                    }
                }
            }

            Error(LOG, 0, "Move to x=%d, y=%d", _pos.x, _pos.y);

            if (_pos.x)
            {
                _RobStateMachine_Step++;
                _correction_step = 0;
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
            
            // no break here!

       case 1:  // move slide 
            if (!rc_move_done()) break;
            TrPrintfL(TRUE, "_rob_state_machine: State=%d", _RobStateMachine_Step);
            pos = _ScrewPar.head + rob_fct_screw_head0;
            if (pos != _HeadScrewPos)
                lbrob_handle_ctrl_msg(INVALID_SOCKET, CMD_ROB_MOVE_POS, &pos);
            if (_ScrewTurned) rc_screwer_to_ref();
            _ScrewTurned = FALSE;
            _correction_step=0;
            _ScrewTime = rx_get_ticks() + MAX_WAIT_TIME_SLEDGE;
            _RobStateMachine_Step++;
            // no break here!

        case 2: // move rob to y (move robi and slide simultanious)
            TrPrintfL(TRUE, "_rob_state_machine: State=%d, head=%d/%d", _RobStateMachine_Step, _ScrewPar.printbar, _ScrewPar.head);
            if (rc_move_xy_error())
            {
                Error(ERR_CONT, 0, "Motor stalled");
                _RobStateMachine_Step=0;
            }
            else if (RX_StepperStatus.robinfo.ref_done && (RX_StepperStatus.info.z_in_screw || RX_StepperStatus.info.z_in_ref || RX_StepperStatus.info.z_in_wash || RX_StepperStatus.info.z_in_cap) && RX_StepperStatus.info.ref_done)
            {
                _posy = (_pos.y<ROB_POSY_FREE && abs(_pos.x-RX_StepperStatus.screw_posX)>1000) ? ROB_POSY_FREE : _pos.y;
                TrPrintfL(TRUE, "diff.x=%d", abs(_pos.x-RX_StepperStatus.screw_posX));
                TrPrintfL(TRUE, "CMD_ROBI_MOVE_TO_Y(%d)", _posy);
                Error(LOG, 0, "CMD_ROBI_MOVE_TO_Y(%d)", _posy);
                robi_lb702_handle_ctrl_msg(INVALID_SOCKET, CMD_ROBI_MOVE_TO_Y, &_posy);
                _ScrewTime = rx_get_ticks() + MAX_WAIT_TIME;
                _RobStateMachine_Step++;
            }
            break;

        case 3: // move rob to x
            if (!rc_move_done()) break;
            TrPrintfL(trace, "_rob_state_machine: State=%d, screw_posY=%d, _posy=%d, diff=%d", _RobStateMachine_Step, RX_StepperStatus.screw_posY, _posy, abs(RX_StepperStatus.screw_posY-_posy));
            if (abs(RX_StepperStatus.screw_posY-_posy)>1000) break;

            TrPrintfL(trace, "_rob_state_machine: State=%d, moveDone=%d, y_in_pos=%d, _HeadScrewPos=%d, head=%d/%d-%d, add=%d", _RobStateMachine_Step, rc_move_done(), RX_StepperStatus.screwerinfo.y_in_pos, _HeadScrewPos, _ScrewPar.printbar, _ScrewPar.head, _ScrewPar.axis, rob_fct_screw_head0);
            if (rc_move_xy_error())
            {
                Error(ERR_CONT, 0, "Motor stalled");
                _RobStateMachine_Step=0;
            }
            else if (rc_move_done() && _HeadScrewPos == _ScrewPar.head + rob_fct_screw_head0)
            {
                TrPrintfL(TRUE, "_rob_state_machine: State=%d, y_in_pos=%d, _HeadScrewPos=%d, head=%d/%d-%d, add=%d", _RobStateMachine_Step, RX_StepperStatus.screwerinfo.y_in_pos, _HeadScrewPos, _ScrewPar.printbar, _ScrewPar.head, _ScrewPar.axis, rob_fct_screw_head0);
                _ScrewTime = rx_get_ticks() + MAX_WAIT_TIME;
                TrPrintfL(TRUE, "CMD_ROBI_MOVE_TO_X(%d), y=%d", _pos.x, RX_StepperStatus.screw_posY);
            //  robi_lb702_handle_ctrl_msg(INVALID_SOCKET, CMD_ROBI_MOVE_TO_X, &_pos.x);
                rc_moveto_x(_pos.x, _FL_);
                _RobStateMachine_Step++;
            }
            break;

        case 4: // move rob to y (move robi and slide simultanious)
            if (!rc_move_done()) break;
            TrPrintfL(TRUE, "_rob_state_machine: State=%d, head=%d/%d", _RobStateMachine_Step, _ScrewPar.printbar, _ScrewPar.head);
            if (rc_move_xy_error())
            {
                Error(ERR_CONT, 0, "Motor stalled");
                _RobStateMachine_Step=0;
            }
            else// if (_pos.y != RX_StepperStatus.screw_posY)
            {
                TrPrintfL(TRUE, "CMD_ROBI_MOVE_TO_Y(%d)", _pos.y);
                Error(LOG, 0, "CMD_ROBI_MOVE_TO_Y(%d)", _pos.y);
                robi_lb702_handle_ctrl_msg(INVALID_SOCKET, CMD_ROBI_MOVE_TO_Y, &_pos.y);
                _ScrewTime = rx_get_ticks() + MAX_WAIT_TIME;
                _RobStateMachine_Step++;
            }
            /*
            else
            {
                _RobStateMachine_Step++;
                _rob_state_machine();
            }
            */
            break;

        case 5:
            TrPrintfL(trace, "_rob_state_machine: State=%d, x_in_pos=%d, movedone=%d", _RobStateMachine_Step, RX_StepperStatus.screwerinfo.x_in_pos, rc_move_done());
       //     if (RX_StepperStatus.screwerinfo.x_in_pos)
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
                    TrPrintfL(TRUE, "_rob_state_machine: State=%d, x_in_pos", _RobStateMachine_Step);
                    _ScrewTime = rx_get_ticks() + MAX_WAIT_TIME;
                    if (!RX_StepperStatus.info.z_in_screw) lb702_handle_ctrl_msg(INVALID_SOCKET, CMD_LIFT_SCREW, NULL);
                    _RobStateMachine_Step++;
                }
            }
            break;

        case 6:
            TrPrintfL(TRUE, "_rob_state_machine: State=%d", _RobStateMachine_Step);
            if (RX_StepperStatus.info.z_in_screw)
            {
                TrPrintfL(TRUE, "CMD_ROBI_MOVE_Z_UP");
                _ScrewTime = rx_get_ticks() + MAX_WAIT_TIME;
                rc_move_top(_FL_);
                _RobStateMachine_Step++;
                _TimeSearchScrew = rx_get_ticks();
            }
            break;

        case 7:
            if (RX_StepperStatus.screwerinfo.z_in_up)
            {
                TrPrintfL(TRUE, "_rob_state_machine: State=%d, zpos=%d, z_in_up=%d", _RobStateMachine_Step, RX_StepperStatus.motor[3].motor_pos, RX_StepperStatus.screwerinfo.z_in_up);
                _RobStateMachine_Step = 12;
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
      
        case 8:
            if (rc_move_done())
            {
                rc_find_screw(-1);
                _RobStateMachine_Step++;
            }
            break;

        case 9:
            if (rc_move_done())
            {
                _ScrewTurned = TRUE;
                if (RX_StepperStatus.screwerinfo.z_in_up)
                {
                    TrPrintfL(TRUE, "_rob_state_machine: State=%d, z_in_up", _RobStateMachine_Step);
                    _RobStateMachine_Step = 11;
                }
                else
                {                
                    TrPrintfL(TRUE, "_rob_state_machine: State=%d, move done", _RobStateMachine_Step);             
                    rc_move_bottom(_FL_);
                    _RobStateMachine_Step++;
                }
            }
            break;

        case 10:
            if (RX_StepperStatus.screwerinfo.z_in_down)
            {
                TrPrintfL(TRUE, "_rob_state_machine: State=%d, screwer move done, posz=%d", _RobStateMachine_Step, RX_StepperStatus.motor[3].motor_pos);
                if (_ScrewFunction==CMD_FIND_ALL_SCREWS || _ScrewFunction==CMD_RESET_ALL_SCREWS)
                {
                    if (_ScrewPar.head<0) ErrorEx(dev_head, _ScrewPar.printbar*RX_StepperCfg.headsPerColor, LOG, 0, "PrintBar %s: screw not found, trying again",  _AxisName[_ScrewPar.axis]);
                    else                  ErrorEx(dev_head, _ScrewPar.printbar*RX_StepperCfg.headsPerColor, LOG, 0, "Head %d %s: screw not found (y=%d), trying again", _ScrewPar.head+1, _AxisName[_ScrewPar.axis], RX_StepperStatus.screw_posY);
                }

                TrPrintfL(TRUE, "z_in_up=%d, screw_posY=%d", RX_StepperStatus.screwerinfo.z_in_up, RX_StepperStatus.screw_posY);
                _ScrewTime = 0;
                _TimeSearchScrew = 0;
                _correction_step++;
                if (_correction_step > 8)
                {
                    robi_lb702_handle_ctrl_msg(INVALID_SOCKET, CMD_ROBI_MOVE_Z_DOWN, NULL);
                    Error(ERR_CONT, 0, "Screw (printbar %d, head %d, axis %d) not found", _ScrewPar.printbar, _ScrewPar.head, _ScrewPar.axis);
                    _RobStateMachine_Step = 0;
                    RX_StepperStatus.adjustDoneCnt++;
                    break;
                }
                int dist = 500*((_correction_step+1)/2);
                if (_correction_step&1) dist=-dist;

                TrPrintfL(TRUE, "_correction_step=%d, dist=%d", _correction_step, dist);
                pos = _pos.y+dist;
                TrPrintfL(TRUE, "CMD_ROBI_MOVE_TO_Y(%d)", pos);
                Error(LOG, 0, "CMD_ROBI_MOVE_TO_Y(%d)", pos);
                rc_moveto_y(pos, _FL_);        
                _TimeSearchScrew = rx_get_ticks();
                _RobStateMachine_Step++;
            }
            break;

        case 11:
            if (rc_move_done())
            {
               TrPrintfL(TRUE, "_rob_state_machine: State=%d, Y move done", _RobStateMachine_Step);
               rc_move_top(_FL_);
               _RobStateMachine_Step=7;
            }
            break;

        case 12:    // screw found 
            TrPrintfL(TRUE, "_rob_state_machine: State=%d", _RobStateMachine_Step);

            TrPrintfL(TRUE, "z_in_up=%d, screw_posY=%d", RX_StepperStatus.screwerinfo.z_in_up, RX_StepperStatus.screw_posY);
            if (_ScrewPar.head<0)
            {
                if (TRUE || _ScrewFunction==CMD_FIND_ALL_SCREWS) ErrorEx(dev_head, _ScrewPar.printbar*RX_StepperCfg.headsPerColor, LOG, 0, "PrintBar %s: screw found",  _AxisName[_ScrewPar.axis]);
                _ScrewPos.printbar[_ScrewPar.printbar].stitch.x = RX_StepperStatus.screw_posX;
                _ScrewPos.printbar[_ScrewPar.printbar].stitch.y = RX_StepperStatus.screw_posY;
            }
            else
            {
                if (TRUE || _ScrewFunction==CMD_FIND_ALL_SCREWS) ErrorEx(dev_head, _ScrewPar.printbar*RX_StepperCfg.headsPerColor, LOG, 0, "Head %d %s: screw found (y=%d)", _ScrewPar.head+1, _AxisName[_ScrewPar.axis], RX_StepperStatus.screw_posY);
                _ScrewPos.printbar[_ScrewPar.printbar].head[_ScrewPar.head][_ScrewPar.axis].x = RX_StepperStatus.screw_posX;
                _ScrewPos.printbar[_ScrewPar.printbar].head[_ScrewPar.head][_ScrewPar.axis].y = RX_StepperStatus.screw_posY;
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

            robi_lb702_handle_ctrl_msg(INVALID_SOCKET, CMD_ROBI_SCREW_TIGHT, &pos);
            _ScrewTime = rx_get_ticks() + MAX_WAIT_TIME_SCREW;
            _RobStateMachine_Step++;
            break;

        case 101:
            TrPrintfL(TRUE, "_rob_state_machine: State=%d, screwerPos=%d", _RobStateMachine_Step, rc_get_screwer_pos());
        //  Error(LOG, 0, "Turn right to stop done, steps=%d", (robi_lb702_screw_edgeCnt() * DISTANCE_PER_TURN) / STEPS_PER_TURN);
            TrPrintfL(TRUE, "Turn right to stop done, steps=%d", (robi_lb702_screw_edgeCnt() * DISTANCE_PER_TURN) / STEPS_PER_TURN);
            _ScrewDist += (robi_lb702_screw_edgeCnt() * DISTANCE_PER_TURN) / STEPS_PER_TURN;

            // turn left to middle + 2 turns
            if (_ScrewPar.axis==AXE_ANGLE) pos = MAX_STEPS_ANGLE/2  + OVERTURN + LOOSE_TURN;
            else                           pos = MAX_STEPS_STITCH/2 + OVERTURN + LOOSE_TURN;
            TrPrintfL(TRUE, "Motor[2]: (PB=%d H=%d, A=%d) rc_turn_ticks_right(%d)", _ScrewPar.printbar, _ScrewPar.head, _ScrewPar.axis, pos);
            robi_lb702_handle_ctrl_msg(INVALID_SOCKET, CMD_ROBI_SCREW_LOOSE, &pos);
            _ScrewTime = rx_get_ticks() + MAX_WAIT_TIME_SCREW;
            _RobStateMachine_Step++;
            break;

        case 102:
            TrPrintfL(TRUE, "_rob_state_machine: State=%d, screwerPos=%d", _RobStateMachine_Step, rc_get_screwer_pos());
        //  Error(LOG, 0, "Turn left to center, steps=%d", (robi_lb702_screw_edgeCnt() * DISTANCE_PER_TURN) / STEPS_PER_TURN);
            TrPrintfL(TRUE, "Turn left to center, steps=%d", (robi_lb702_screw_edgeCnt() * DISTANCE_PER_TURN) / STEPS_PER_TURN);
            _ScrewDist += (robi_lb702_screw_edgeCnt() * DISTANCE_PER_TURN) / STEPS_PER_TURN;

            _ScrewerStalled = rc_screwer_stalled();
            // turn right 2 turns
            pos = OVERTURN;
            TrPrintfL(TRUE, "Motor[2]: (PB=%d H=%d, A=%d) rc_turn_ticks_left(%d)", _ScrewPar.printbar, _ScrewPar.head, _ScrewPar.axis, pos);
            robi_lb702_handle_ctrl_msg(INVALID_SOCKET, CMD_ROBI_SCREW_TIGHT, &pos);
            _ScrewTime = rx_get_ticks() + MAX_WAIT_TIME_SCREW;
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
        //  Error(LOG, 0, "Turn right 2 turns, steps=%d", (robi_lb702_screw_edgeCnt() * DISTANCE_PER_TURN) / STEPS_PER_TURN);
            TrPrintfL(TRUE, "Turn right 2 turns, steps=%d", (robi_lb702_screw_edgeCnt() * DISTANCE_PER_TURN) / STEPS_PER_TURN);
            _ScrewDist += (robi_lb702_screw_edgeCnt() * DISTANCE_PER_TURN) / STEPS_PER_TURN;

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
                robi_lb702_handle_ctrl_msg(INVALID_SOCKET, CMD_ROBI_SCREW_LOOSE, &pos);
            }
            _ScrewTime = rx_get_ticks() + MAX_WAIT_TIME_SCREW;
            _RobStateMachine_Step++;
            break;

        case 201:
            TrPrintfL(TRUE, "_rob_state_machine: State=%d, blockedLeft=%d, blockedRight=%d", _RobStateMachine_Step, RX_StepperStatus.screwerinfo.screwer_blocked_left, RX_StepperStatus.screwerinfo.screwer_blocked_right);
            _ScrewDist += (robi_lb702_screw_edgeCnt() * DISTANCE_PER_TURN) / STEPS_PER_TURN;
            if (_ScrewPar.steps < 0) pos = 2*6;
            else                     pos = _ScrewPar.steps;
            robi_lb702_handle_ctrl_msg(INVALID_SOCKET, CMD_ROBI_SCREW_TIGHT, &pos);
            _RobStateMachine_Step++;
            _ScrewTime = rx_get_ticks() + MAX_WAIT_TIME_SCREW;
            break;
            
        case 202:
            TrPrintfL(TRUE, "_rob_state_machine: State=%d, blockedLeft=%d, blockedRight=%d", _RobStateMachine_Step, RX_StepperStatus.screwerinfo.screwer_blocked_left, RX_StepperStatus.screwerinfo.screwer_blocked_right);

            RX_StepperStatus.screwerinfo.screwed_OK = !RX_StepperStatus.screwerinfo.screwer_blocked_left && !RX_StepperStatus.screwerinfo.screwer_blocked_right;
            _ScrewDist += (robi_lb702_screw_edgeCnt() * DISTANCE_PER_TURN) / STEPS_PER_TURN;
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
            _save_ScrewPos(_ScrewDist);
            
            if (!RX_StepperStatus.screwerinfo.screwer_blocked_left && !RX_StepperStatus.screwerinfo.screwer_blocked_right)
            {
                _ScrewTime = 0;
                _RobStateMachine_Step++;
            }
            else
            {
                if (RX_StepperStatus.screwerinfo.screwer_blocked_left)
                {
                    pos = LOOSE_TURN;
                    robi_lb702_handle_ctrl_msg(INVALID_SOCKET, CMD_ROBI_SCREW_LOOSE, &pos);
                }
                else
                {
                    pos = OVERTURN + LOOSE_TURN;
                    robi_lb702_handle_ctrl_msg(INVALID_SOCKET, CMD_ROBI_SCREW_TIGHT, &pos);
                }
                _RobStateMachine_Step++;    
            }
            break;
            
        case 203:
            TrPrintfL(TRUE, "_rob_state_machine: State=%d", _RobStateMachine_Step);
            rc_move_bottom(_FL_);
             _RobStateMachine_Step++;
            break;

        case 204:
            TrPrintfL(TRUE, "_rob_state_machine: State=%d", _RobStateMachine_Step);
            _ScrewTime = 0;
            _RobStateMachine_Step = 0; // wait for next command
            RX_StepperStatus.adjustDoneCnt++;
            break;

        //--------------- next screw ---------------
        case 900:
            TrPrintfL(TRUE, "_rob_state_machine: State=%d", _RobStateMachine_Step);
            _save_ScrewPos(_ScrewDist);
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
                _ScrewTime = 0;
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
                    rc_reference();
                    _RobStateMachine_Step=0;
                }
            }
            break;

        case 1001: 
            if (rc_move_done())
            {
                TrPrintfL(TRUE, "_rob_state_machine: State=%d", _RobStateMachine_Step);
                TrPrintfL(TRUE, "CMD_ROBI_MOVE_TO_X(%d)", 0);
                _lbrob_move_to_pos(_CmdRunning, _micron_2_steps(3000), FALSE);
                rc_moveto_x(0, _FL_);
                _RobStateMachine_Step++;
            }
            break;

        case 1002: // move to garage ---
            if (rc_move_done())
            {
                TrPrintfL(TRUE, "_rob_state_machine: State=%d", _RobStateMachine_Step);
                rc_moveto_y(0, _FL_);
                _RobStateMachine_Step++;
            }
            break;
        
        case 1003:
            if (rc_move_done())
            {
                TrPrintfL(TRUE, "_rob_state_machine: State=%d", _RobStateMachine_Step);
                _RobStateMachine_Step=0;
                RX_StepperStatus.adjustDoneCnt++;
            }
            break;
        }
    }
    if (_RobStateMachine_Step==0) rx_enable_stall_error(TRUE);
}

//--- lbrob_to_garage --------------------------------
void lbrob_to_garage(void)
{
    if (_RobStateMachine_Step==0)
    {
        if (!rc_in_garage())
        {
            if (!RX_StepperStatus.screwerinfo.ref_done) 
            {
                rc_reference();
            }
            else 
            {
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

//--- _save_ScrewPos ---------------------------------------
static void _save_ScrewPos(int dist)
{
    if (_ScrewPar.axis == AXE_STITCH)
    {
        // int distance = (robi_lb702_screw_edgeCnt() * DISTANCE_PER_TURN) / STEPS_PER_TURN;
    //  Error(LOG, 0, "_save_ScrewPos: dist=%d", dist);

        if (abs(dist)>10)
        {
            int i;
            for (i = _ScrewPar.head + 1; i < RX_StepperCfg.headsPerColor; i++)
            {
                Error(LOG, 0, "Recalc Head[%d]", i);
                _ScrewPos.printbar[_ScrewPar.printbar].head[i][AXE_ANGLE].y += dist;
                if (i != RX_StepperCfg.headsPerColor - 1)
                    _ScrewPos.printbar[_ScrewPar.printbar].head[i][AXE_STITCH].y += dist;
            }
        }
    }
    ctrl_send_2(REP_SET_SCREW_POS, sizeof(_ScrewPos), &_ScrewPos);
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

//--- _handle_flush_pump -------------------------------------------------------------------------
static void _handle_flush_pump(void)
{
    if (!(Fpga.par->output & RO_FLUSH_WIPE) && !(Fpga.par->output & RO_FLUSH_TO_CAP) && (Fpga.par->output & RO_FLUSH_PUMP))
        Fpga.par->output &= ~RO_FLUSH_PUMP;
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

//--- _set_ink_back_pump_left -------------------------------------
static void _set_ink_back_pump_left(int voltage)
{
    _PumpValueLeft = voltage;
    _pump_main();
}

//--- _set_ink_bak_pump_right --------------------------------------------------
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

//--- lbrob_reset_variables ----------------------------------------
void lbrob_reset_variables(void)
{
    _NewCmd = FALSE;
}

//--- _vacuum_on -----------------------------------------------
static void _vacuum_on()
{
    if (_CapIsWet)
    {
        Fpga.par->output |= RO_WASTE_VAC;
        Fpga.par->output |= RO_VACUUM_CLEANER;
        _WastePumpTimer = 0;
    }
}

//--- _vacuum_off -----------------------------------------------
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

        motors_config(motors, CURRENT_HOLD, X_STEPS_PER_REV, X_INC_PER_REV, STEPS);
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
