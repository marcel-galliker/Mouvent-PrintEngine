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
//#include "robi_interface.h"
#include "lb702.h"
#include "robi.h"

#define MOTOR_X_0 4

#define MOTOR_X_BITS 0x10
#define MOTOR_ALL_BITS 0x13

#define X_STEPS_PER_REV 3200.0 // 3214.0  //3200.0
#define X_INC_PER_REV 16000.0
#define X_DIST_PER_REV 36000

#define CABLE_CAP_POS -626000        //	um LB702
#define CABLE_WASH_POS_FRONT -573000 //	um LB702
#define CABLE_WASH_POS_BACK -160000  //	um LB702
#define CABLE_PURGE_POS_BACK -250000 //  um LB702
#define CABLE_PURGE_POS_FRONT -561000 //  um LB702        CABLE_PURGE_POS_BACK - (7 * HEAD_WIDTH) - 10000  -> HEAD_WIDTH = 43000
#define CABLE_SCREW_POS_FRONT -452000 //  um LB702
#define CABLE_SCREW_POS_BACK  -105208   //-148557 //  um LB702        CABLE_SCREW_POS_BACK + (7 * HEAD_WIDTH) -> //  HEAD_WIDTH = 43349

#define CURRENT_HOLD 200

#define CAP_FILL_TIME 10000        // ms
#define WASTE_PUMP_TIME 60000      // ms
#define SCREW_SEARCHING_TIME 21000 // ms

// Digital Inputs
#define CABLE_PULL_REF 2
#define CAPPING_ENDSTOP 3

// Digital Outputs
#define RO_ALL_OUTPUTS 0x06F       // All used outputs -> o0, o1, o2, o3, o5, o6
#define RO_FLUSH_TO_CAP_LEFT 0x001 // o0
#define RO_FLUSH_TO_CAP_RIGHT 0x002 // o1
#define RO_FLUSH_TO_CAP 0x003       // o0 + o1
#define RO_FLUSH_WIPE_LEFT 0x004    // o2
#define RO_FLUSH_WIPE_RIGHT 0x008   // o3
#define RO_FLUSH_WIPE 0x00c         // o2 + o3
#define RO_FLUSH_PUMP 0x020         // o5
#define RO_WASTE 0x040              // o6

#define MAX_POS_DIFFERENT 4000 // steps

#define HEAD_WIDTH 43000

#define HEADS_PER_COLOR 8
#define COLORS_PER_STEPPER 2
#define SCREWS_PER_HEAD 2

#define MAX_VAR_SCREW_POS 2000 // um
/*
typedef struct E_ScrewPositions
{
    int posX;
    int posY;
} E_ScrewPositions;
*/
// globals

int _CmdRunning_Lift = 0;
int _CmdRunning_Robi = 0;


// static
static SMovePar _ParCable_ref;
static SMovePar _ParCable_drive;
static SMovePar _ParCable_drive_slow;
static SMovePar _ParCable_drive_purge;

static char *_MotorName[5] = {"BACK", "FRONT", "NONE", "NON", "SLEDGE"};
static char _CmdName[32];
static int _CmdRunning = 0;
static int _NewCmd = 0;

static int _TimeNow = 0;

static int _CapFillTime = 0;
static int _PumpWasteTime = 0;

static ERobotFunctions _RobFunction = 0;
static ERobotFunctions _Old_RobFunction = 0;

// static int _ScrewNr = 0;
// static int _ScrewTurns = 0;
static SHeadAdjustment _HeadAdjustment = {0, 0, 0, 0};
static int _CmdScrewing = 0;
static int _CmdSearchScrews = 0;
static int _SearchScrewNr = 0;
static int _Turns = 0;
static int _TimeSearchScrew = 0;
static int _HeadPos = 0;

// E_ScrewPositions _ScrewPositions[4 * HEADS_PER_COLOR];


//--- prototypes --------------------------------------------
static void _lbrob_motor_z_test(int steps);
static void _lbrob_motor_test(int motor, int steps);
static void _lbrob_do_reference(void);
static void _lbrob_move_to_pos(int cmd, int pos);
static void _lbrob_motor_enc_reg_test(int steps);
static int _micron_2_steps(int micron);
static int _steps_2_micron(int steps);
static void _cln_move_to(int msgId, ERobotFunctions fct);
static void _check_pump(void);
static void _turn_screw(SHeadAdjustment headAdjustment);
static int _check_in_screw_pos(SHeadAdjustment headAdjustment);
static void _search_all_screws();
static int _calculate_average_y_pos(int screwNr);

static int _CmdRunning_old = 0;

//--- lbrob_init --------------------------------------
void lbrob_init(void)
{
    memset(_CmdName, 0, sizeof(_CmdName));
    memset(&_ParCable_ref, 0, sizeof(SMovePar));
    memset(&_ParCable_drive, 0, sizeof(SMovePar));
    memset(&_ParCable_drive_purge, 0, sizeof(SMovePar));

    // config for referencing cable pull motor (motor 4)
    _ParCable_ref.speed = 2000;
    _ParCable_ref.accel = 4000;
    _ParCable_ref.current_acc = 120.0;
    _ParCable_ref.current_run = 120.0;
    _ParCable_ref.stop_mux = 0;
    _ParCable_ref.dis_mux_in = 0;
    _ParCable_ref.estop_level = TRUE;
    _ParCable_ref.estop_in_bit[MOTOR_X_0] = (1 << CABLE_PULL_REF);
    _ParCable_ref.enc_bwd = TRUE;
    _ParCable_ref.encCheck = chk_txrob_ref;

    // config for moving normal with cable pull motor (motor 4)
    // This commands that use this config need to start the motor with the
    // special encoder mode
    _ParCable_drive.speed = 5000;
    _ParCable_drive.accel = 8000;
    _ParCable_drive.current_acc = 420.0;
    _ParCable_drive.current_run = 420.0;
    _ParCable_drive.stop_mux = 0;
    _ParCable_drive.dis_mux_in = 0;
    _ParCable_drive.estop_level = 0;
    _ParCable_drive.enc_bwd = TRUE;
    _ParCable_drive.encCheck = chk_std;

    _ParCable_drive_slow.speed = 1000;
    _ParCable_drive_slow.accel = 8000;
    _ParCable_drive_slow.current_acc = 420.0;
    _ParCable_drive_slow.current_run = 420.0;
    _ParCable_drive_slow.stop_mux = 0;
    _ParCable_drive_slow.dis_mux_in = 0;
    _ParCable_drive_slow.estop_level = 0;
    _ParCable_drive_slow.enc_bwd = TRUE;
    _ParCable_drive_slow.encCheck = chk_std;

    _ParCable_drive_purge.speed = _micron_2_steps(
        1000 * 10); // multiplied with 1000 to get from mm/s to um/s
    _ParCable_drive_purge.accel = 4000;
    _ParCable_drive_purge.current_acc = 420.0;
    _ParCable_drive_purge.current_run = 420.0;
    _ParCable_drive_purge.stop_mux = 0;
    _ParCable_drive_purge.dis_mux_in = 0;
    _ParCable_drive_purge.estop_level = 0;
    _ParCable_drive_purge.enc_bwd = TRUE;
    _ParCable_drive_purge.encCheck = chk_std;

    motor_config(MOTOR_X_0, CURRENT_HOLD, X_STEPS_PER_REV, X_INC_PER_REV,
                 STEPS);

    robi_init();
}

//--- _check_pump
//---------------------------------------------------------------
static void _check_pump(void)
{
    if (!(Fpga.par->output & RO_FLUSH_TO_CAP_LEFT) &&
        !(Fpga.par->output & RO_FLUSH_TO_CAP_RIGHT) &&
        !(Fpga.par->output & RO_FLUSH_WIPE_LEFT) &&
        !(Fpga.par->output & RO_FLUSH_WIPE_RIGHT))
    {
        Fpga.par->output &= ~RO_FLUSH_PUMP;
    }
    _TimeNow = rx_get_ticks();

    if (_PumpWasteTime && _TimeNow <= _PumpWasteTime + WASTE_PUMP_TIME)
        Fpga.par->output |= RO_WASTE;
    else if (_PumpWasteTime)
    {
        _PumpWasteTime = FALSE;
        Fpga.par->output &= ~RO_WASTE;
    }
}

//--- lbrob_main
//------------------------------------------------------------------
void lbrob_main(int ticks, int menu)
{
    int motor;
    int pos;

    static ERobotFunctions new_RobFunction = 0;
    RX_StepperStatus.posY[0] = _steps_2_micron(motor_get_step(MOTOR_X_0));
    RX_StepperStatus.posY[1] = _steps_2_micron(motor_get_step(MOTOR_X_0)) -
                               CABLE_PURGE_POS_BACK;
    SStepperStat oldSatus;
    memcpy(&oldSatus, &RX_StepperStatus, sizeof(RX_StepperStatus));

    _check_pump();


    motor_main(ticks, menu);
    robi_main(ticks, menu);

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
        _HeadPos = FALSE;
    }

    if (!RX_StepperStatus.robinfo.ref_done)
        RX_StepperStatus.screwerinfo.screws_found = FALSE;

    if (_CmdRunning && motors_move_done(MOTOR_X_BITS))
    {
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
                    Error(ERR_CONT, 0, "Stepper: Command %s: Motor[%d] blocked",
                          _CmdName, motor + 1);
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
                    Error(ERR_CONT, 0,
                          "LBROB: Command %s: End Sensor REF NOT HIGH",
                          _CmdName);
                    RX_StepperStatus.robinfo.ref_done = FALSE;
                    _CmdRunning_old = FALSE;
                }
            }
            else
            {
                if (motors_error(MOTOR_X_BITS, &motor))
                {
                    Error(ERR_CONT, 0,
                          "LIFT: Command %s - 1000 steps: Motor %s blocked",
                          _CmdName, _MotorName[motor]);
                    RX_StepperStatus.robinfo.ref_done = FALSE;
                }
                else if (!RX_StepperStatus.info.x_in_ref)
                {
                    Error(ERR_CONT, 0,
                          "LBROB: Command %s: End Sensor REF NOT HIGH",
                          _CmdName);
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
                Error(ERR_CONT, 0, "Stepper: Command %s: Motor %s blocked",
                      _CmdName, _MotorName[motor]);
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
                _PumpWasteTime = rx_get_ticks();
                break;

            default:
                Error(ERR_CONT, 0,
                      "Command %s: Robi-Function %d not implemented", _CmdName,
                      _RobFunction);
                break;
            }
            _Old_RobFunction = _RobFunction;
        }
        else if (_CmdRunning == CMD_ROB_MOVE_POS)
        {
            switch (_RobFunction)
            {
            case rob_fct_cap:
                RX_StepperStatus.robinfo.rob_in_cap =
                    fpga_input(CAPPING_ENDSTOP);
                if (!RX_StepperStatus.robinfo.rob_in_cap &&
                    _NewCmd != CMD_ROB_MOVE_POS)
                {
                    Error(ERR_CONT, 0,
                          "LBROB: Command %s: End Sensor Capping NOT HIGH",
                          _CmdName);
                    RX_StepperStatus.robinfo.ref_done = FALSE;
                }
                _CmdRunning = FALSE;
                break;

            case rob_fct_purge_all:
                pos = abs(motor_get_step(MOTOR_X_0));
                RX_StepperStatus.robinfo.purge_ready =
                    (abs(motor_get_step(MOTOR_X_0) -
                         _micron_2_steps(CABLE_PURGE_POS_BACK)) <=
                     MAX_POS_DIFFERENT);
                if (!RX_StepperStatus.robinfo.purge_ready &&
                    _NewCmd != CMD_ROB_MOVE_POS)
                {
                    Error(ERR_CONT, 0,
                          "LBROB: Command %s: Robot not in correct position",
                          _CmdName);
                    RX_StepperStatus.robinfo.ref_done = FALSE;
                }
                _CmdRunning = FALSE;
                break;

                break;
            case rob_fct_purge_head0:
                RX_StepperStatus.robinfo.purge_ready =
                    (abs(motor_get_step(MOTOR_X_0) -
                         motor_get_end_step(MOTOR_X_0)) <= MAX_POS_DIFFERENT);
                if (!RX_StepperStatus.robinfo.purge_ready &&
                    _NewCmd != CMD_ROB_MOVE_POS)
                {
                    Error(ERR_CONT, 0,
                          "LBROB: Command %s: Robot not in correct position",
                          _CmdName);
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
                _HeadPos = _RobFunction;
                _CmdRunning = FALSE;
                break;
            case rob_fct_vacuum:
            case rob_fct_wash:
                new_RobFunction = rob_fct_move;
                _Old_RobFunction = _RobFunction;
                _CmdRunning = FALSE;
                _CmdRunning_old = FALSE;
                lbrob_handle_ctrl_msg(INVALID_SOCKET, CMD_ROB_MOVE_POS,
                                      &new_RobFunction);
                break;

            case rob_fct_move:
                _CmdRunning = FALSE;
                Fpga.par->output &= ~RO_FLUSH_WIPE;
                switch (_Old_RobFunction)
                {
                case rob_fct_wash:
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
                Error(ERR_CONT, 0,
                      "LBROB_MAIN: Rob-Function %d not implemented",
                      _RobFunction);
                _CmdRunning = FALSE;
                RX_StepperStatus.robinfo.ref_done = FALSE;
                break;
            }
            if (!_CmdRunning_Lift) _Old_RobFunction = _RobFunction;
        }
        else if (_CmdRunning == CMD_ROB_REFERENCE && _CmdRunning_old)
        {
            int loc_new_cmd = 0;
            if (!RX_StepperStatus.robinfo.moving &&
                !RX_StepperStatus.info.moving)
            {
                loc_new_cmd = _CmdRunning_old;
                _CmdRunning_old = FALSE;
                _CmdRunning = FALSE;
            }

            switch (loc_new_cmd)
            {
            case CMD_ROB_MOVE_POS:
                lbrob_handle_ctrl_msg(INVALID_SOCKET, CMD_ROB_MOVE_POS,
                                      &_RobFunction);
                break;
            default:
                Error(ERR_CONT, 0, "LBROB_MAIN: Command 0x%08x not implemented",
                      _CmdRunning_old);
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
        if (!RX_StepperStatus.robinfo.moving && !RX_StepperStatus.info.moving &&
            !RX_StepperStatus.screwerinfo.moving)
        {
            loc_new_cmd = _NewCmd;
            _NewCmd = FALSE;
            _CmdRunning_Lift = FALSE;
        }
        if (loc_new_cmd)
        {
            switch (loc_new_cmd)
            {
            case CMD_ROB_REFERENCE:
                lbrob_handle_ctrl_msg(INVALID_SOCKET, CMD_ROB_REFERENCE, NULL);
                break;
            case CMD_ROB_MOVE_POS:
                lbrob_handle_ctrl_msg(INVALID_SOCKET, CMD_ROB_MOVE_POS,
                                      &_RobFunction);
                break;
            default:
                Error(ERR_CONT, 0, "LBROB_MAIN: Command 0x%08x not implemented",
                      loc_new_cmd);
                break;
            }
        }
        loc_new_cmd = FALSE;
    }

    static int j = 0;

    if (memcmp(&oldSatus.robinfo, &RX_StepperStatus.robinfo,
               sizeof(RX_StepperStatus.robinfo)))
    {
        ctrl_send_2(REP_STEPPER_STAT, sizeof(RX_StepperStatus),
                    &RX_StepperStatus);
    }
    else if (_CmdRunning == CMD_ROB_FILL_CAP)
    {
        if (RX_StepperStatus.posY[1] > j * HEAD_WIDTH)
        {
            j++;
            ctrl_send_2(REP_STEPPER_STAT, sizeof(RX_StepperStatus),
                        &RX_StepperStatus);
        }
    }
    else
    {
        j = 0;
    }

    if (_CmdSearchScrews)
    {
        _CmdScrewing = 0;
        _search_all_screws();
    }
    else
    {
        _SearchScrewNr = 0;
    }

    if (_CmdScrewing)
    {
        _turn_screw(_HeadAdjustment);
        //_turn_screw(_ScrewNr, _ScrewTurns);
    }
    else
    {
        memset(&_HeadAdjustment, 0, sizeof(_HeadAdjustment));
        //_ScrewNr = 0;
        //_ScrewTurns = 0;
    }
}

//---_micron_2_steps
//--------------------------------------------------------------
static int _micron_2_steps(int micron)
{
    return (int)(0.5 + X_STEPS_PER_REV / X_DIST_PER_REV * micron);
}

//--- _steps_2_micron
//-------------------------------------------------------------
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
        term_printf("moving:          %d		cmd: %08x\n",
                    RX_StepperStatus.robinfo.moving, _CmdRunning);
        term_printf("Screwing-Step    %d\n", _CmdScrewing);
        term_printf("reference done: %d\n", RX_StepperStatus.robinfo.ref_done);
        term_printf("x in reference: %d\n", RX_StepperStatus.info.x_in_ref);
        term_printf("x in cap: %d\n", RX_StepperStatus.info.x_in_cap);
        term_printf("Cap ready       %d\n", RX_StepperStatus.robinfo.cap_ready);
        term_printf("Purge ready     %d\n",
                    RX_StepperStatus.robinfo.purge_ready);
        term_printf("actPos Robi:     %dum\n", RX_StepperStatus.posY[0]);
        term_printf("Wipe-Speed:     %d\n", RX_StepperCfg.wipe_speed);
        term_printf("Vacuum done:     %d\n",
                    RX_StepperStatus.robinfo.vacuum_done);
        if (_PumpWasteTime)
        {
            term_printf("Waste-Pump-Time: %d\n",
                        (WASTE_PUMP_TIME - (rx_get_ticks() - _PumpWasteTime)) /
                            1000);
        }
        else
        {
            term_printf("Waste-Pump-Time: 0\n");
        }
        term_printf("\n");
    }
    else
    {
        term_printf("No Robot (bridge stepper board not installed)\n");
    }
}

//--- lbrob_menu
//--------------------------------------------------------------------
void lbrob_menu(int help)
{
    if (help)
    {
        term_printf("LB rob MENU ------------------------- (2)\n");
        term_printf("s: STOP\n");
        term_printf("o: toggle output <no>\n");
        term_printf("R: Reference\n");
        term_printf("r<n>: reset motor<n>\n");
        term_printf("c: Cap the heads\n");
        term_printf("w: Wash Heads\n");
        term_printf("v: Vacuum Heads\n");
        term_printf("g<n>: Go to purge position of head 1 -8 or for all (0)\n");
        term_printf("p: Move the Robi for Purging all Heads\n");
        term_printf("m<n><steps>: move Motor<n> by <steps>\n");
        term_printf(
            "e<steps>: move Cablepull with encoder regulator by <steps>\n");
        term_printf("S<speed>: Set Speed for Wash and Vacuum in <speed>mm/s\n");
        term_printf("f: Find all the screws\n");
        term_printf("a<n>: Go to adjustment position of head 0 - 7\n");
        term_printf("t<screw_nr>: Turn <screw_nr> n/6 Turn (n can be choosen "
                    "by command \"T\")\n");
        term_printf("T<n>: Make <n>/6 turns with the command t\n");


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
    case 'w':
        pos = rob_fct_wash;
        lbrob_handle_ctrl_msg(INVALID_SOCKET, CMD_ROB_MOVE_POS, &pos);
        break;
    case 'q':
        _PumpWasteTime = rx_get_ticks();
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
    case 'e':
        _lbrob_motor_enc_reg_test(atoi(&str[1]));
        break;
    case 'S':
        pos = atoi(&str[1]);
        if (pos <= 8) pos = 8;
        _ParCable_drive_slow.speed = _micron_2_steps(1000 * pos);
        break;
    case 'f':
        lbrob_handle_ctrl_msg(INVALID_SOCKET, CMD_SEARCH_ALL_SCREWS, NULL);
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
        screw_head.printbarNo =
            atoi(&str[1]) / (SCREWS_PER_HEAD * HEADS_PER_COLOR);
        screw_head.headNo =
            (atoi(&str[1]) % (SCREWS_PER_HEAD * HEADS_PER_COLOR)) /
            SCREWS_PER_HEAD;
        screw_head.axis = atoi(&str[1]) % SCREWS_PER_HEAD;
        screw_head.steps = _Turns;
        lbrob_handle_ctrl_msg(INVALID_SOCKET, CMD_HEAD_ADJUST, &screw_head);
        break;
    case 'T':
        _Turns = atoi(&str[1]);
        break;
    }
}

//--- _lbrob_move_to_pos
//---------------------------------------------------------------
static void _lbrob_move_to_pos(int cmd, int pos)
{
    _CmdRunning = cmd;
    RX_StepperStatus.robinfo.moving = TRUE;
    motors_move_to_step(MOTOR_X_BITS, &_ParCable_drive, pos);
}


static void _lbrob_do_reference()
{
    _PumpWasteTime = rx_get_ticks();
    Fpga.par->output &= ~RO_ALL_OUTPUTS;
    _NewCmd = 0;
    RX_StepperStatus.robinfo.moving = TRUE;
    _CmdRunning = CMD_ROB_REFERENCE;
    if (!RX_StepperStatus.robinfo.ref_done)
    {
        motor_reset(MOTOR_X_0);
        RX_StepperStatus.robinfo.ref_done = FALSE;
        motors_move_by_step(1 << MOTOR_X_0, &_ParCable_ref, 1000000, TRUE);
    }
    else
    {
        motors_move_to_step(MOTOR_X_BITS, &_ParCable_drive,
                            _micron_2_steps(3000));
    }
}

//--- lbrob_handle_ctrl_msg -----------------------------------
int lbrob_handle_ctrl_msg(RX_SOCKET socket, int msgId, void *pdata)
{
    int val, pos;
    int screw_nr, screw_turn;
    if (socket != -1) _CmdScrewing = 0;

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
        if (!robi_disabled())
            robi_handle_ctrl_msg(INVALID_SOCKET, CMD_ROBI_STOP, NULL);
        break;

    case CMD_ROB_REFERENCE:
        strcpy(_CmdName, "CMD_ROB_REFERENCE");
        if (_CmdRunning)
        {
            lbrob_handle_ctrl_msg(INVALID_SOCKET, CMD_ROB_STOP, NULL);
            _NewCmd = CMD_ROB_REFERENCE;
            break;
        }
        if (!RX_StepperStatus.info.z_in_ref &&
            !RX_StepperStatus.info.z_in_screw)
        {
            if (!RX_StepperStatus.info.moving)
            {
                _CmdRunning_Lift = CMD_LIFT_REFERENCE;
                lb702_handle_ctrl_msg(INVALID_SOCKET, _CmdRunning_Lift, NULL);
                _NewCmd = msgId;
            }
            break;
        }
        else if (!RX_StepperStatus.screwerinfo.z_in_down && !robi_disabled())
        {
            _CmdRunning_Robi = CMD_ROBI_MOVE_Z_DOWN;
            _NewCmd = msgId;
            robi_handle_ctrl_msg(INVALID_SOCKET, _CmdRunning_Robi, NULL);
            
            break;
        }
        _CmdRunning = msgId;
        _lbrob_do_reference();
        break;

    case CMD_ROB_MOVE_POS:
        strcpy(_CmdName, "CMD_ROB_MOVE_POS");
        _cln_move_to(msgId, *(ERobotFunctions *)pdata);
        break;

    case CMD_ROB_FILL_CAP:
        strcpy(_CmdName, "CMD_ROB_FILL_CAP");
        if (!_CmdRunning)
        {

            if (!RX_StepperStatus.robinfo.ref_done)
            {
                Error(ERR_CONT, 0, "LBROB: Robot not refenenced, cmd=0x%08x",
                      msgId);
                break;
            }
            _CmdRunning = msgId;
            pos = *((INT32 *)pdata);
            _RobFunction = pos;
            switch (_RobFunction)
            {
            case rob_fct_cap:
                _PumpWasteTime = 0;
                RX_StepperStatus.robinfo.moving = TRUE;
                Fpga.par->output &= ~RO_ALL_OUTPUTS;
                Fpga.par->output |= RO_FLUSH_TO_CAP_LEFT;
                Fpga.par->output |= RO_FLUSH_PUMP;
                _CapFillTime = rx_get_ticks();
                break;

            case rob_fct_purge_all:
                if (RX_StepperCfg.wipe_speed == 0) Error(ERR_CONT, 0, "Wipe-Speed is set 0, please chose another value");

                if (!RX_StepperStatus.info.z_in_ref)
                {
                    if (!RX_StepperStatus.info.moving)
                    {
                        _CmdRunning_Lift = CMD_LIFT_REFERENCE; lb702_handle_ctrl_msg(INVALID_SOCKET, _CmdRunning_Lift, NULL);
                        _NewCmd = msgId;
                        break;
                    }
                }
                RX_StepperStatus.robinfo.moving = TRUE;
                _CmdRunning = msgId;
                if (RX_StepperCfg.wipe_speed)
                    _ParCable_drive_purge.speed = _micron_2_steps(1000 * RX_StepperCfg.wipe_speed); // multiplied with 1000 to get from mm/s to um/s
                else
                    _ParCable_drive_purge.speed = _micron_2_steps(1000 * 10); // multiplied with 1000 to get from mm/s to um/s
                _PumpWasteTime = rx_get_ticks();
                motors_move_to_step(MOTOR_X_BITS, &_ParCable_drive_purge, _micron_2_steps(CABLE_PURGE_POS_FRONT));
                break;

            default:
                break;
            }
        }
        break;
    case CMD_HEAD_ADJUST:
        _turn_screw(*(SHeadAdjustment *)pdata);

        break;

    case CMD_SEARCH_ALL_SCREWS:
        _search_all_screws();
        break;


    case CMD_ERROR_RESET:
        strcpy(_CmdName, "CMD_ERROR_RESET");
        fpga_stepper_error_reset();
        break;

    default:
        lb702_handle_ctrl_msg(socket, msgId, pdata);
        break;
    }
}

//--- _cln_move_to ---------------------------------------
static void _cln_move_to(int msgId, ERobotFunctions fct)
{
    if (!_CmdRunning)
    {
        int pos;
        _RobFunction = fct;
        if (!RX_StepperStatus.screwerinfo.z_in_down && !robi_disabled())
        {
            _CmdRunning_Robi = CMD_ROBI_MOVE_Z_DOWN;
            _NewCmd = msgId;
            robi_handle_ctrl_msg(INVALID_SOCKET, _CmdRunning_Robi, NULL);
            
            return;
        }
        else if (!RX_StepperStatus.info.z_in_ref && !((_RobFunction == rob_fct_move) && RX_StepperStatus.info.z_in_wash) &&
                 !(RX_StepperStatus.info.z_in_screw && _RobFunction >= rob_fct_screw_head0 && _RobFunction <= rob_fct_screw_head7)) // Here this is for purging and not for vacuum
        {
            if (!RX_StepperStatus.info.moving)
            {
                _CmdRunning_Lift = CMD_LIFT_REFERENCE;
                lb702_handle_ctrl_msg(INVALID_SOCKET, _CmdRunning_Lift, NULL);
                _NewCmd = msgId;
            }
            return;
        }
        else if (!RX_StepperStatus.robinfo.ref_done)
        {
            _lbrob_do_reference();
            _CmdRunning_old = msgId;
            return;
        }
        RX_StepperStatus.robinfo.moving = TRUE;
        _CmdRunning = msgId;
        switch (_RobFunction)
        {
        case rob_fct_cap:
            _lbrob_move_to_pos(_CmdRunning, _micron_2_steps(CABLE_CAP_POS));
            // motors_move_to_step(MOTOR_X_BITS, &_ParCable_drive,
            // _micron_2_steps(CABLE_CAP_POS));
            break;

        case rob_fct_purge_all:
            _lbrob_move_to_pos(_CmdRunning,
                               _micron_2_steps(CABLE_PURGE_POS_BACK));
            // motors_move_to_step(MOTOR_X_BITS, &_ParCable_drive,
            // _micron_2_steps(CABLE_PURGE_POS_BACK));
            _PumpWasteTime = rx_get_ticks();
            break;

        case rob_fct_purge_head0:
        case rob_fct_purge_head1:
        case rob_fct_purge_head2:
        case rob_fct_purge_head3:
        case rob_fct_purge_head4:
        case rob_fct_purge_head5:
        case rob_fct_purge_head6:
        case rob_fct_purge_head7:
            pos = (CABLE_PURGE_POS_BACK + (((int)_RobFunction - (int)rob_fct_purge_head0) * (CABLE_PURGE_POS_FRONT - CABLE_PURGE_POS_BACK)) / 7);
            _lbrob_move_to_pos(_CmdRunning, _micron_2_steps(pos));
            break;
        case rob_fct_vacuum:
        case rob_fct_wash:
            _lbrob_move_to_pos(_CmdRunning, _micron_2_steps(CABLE_PURGE_POS_FRONT));
            break;

        case rob_fct_move:
            if (!RX_StepperStatus.info.z_in_wash)
            {
                if (!RX_StepperStatus.info.moving)
                {
                    _CmdRunning = FALSE;
                    _CmdRunning_Lift = CMD_LIFT_WASH_POS;
                    _NewCmd = msgId;
                    lb702_handle_ctrl_msg(INVALID_SOCKET, _CmdRunning_Lift,
                                          NULL);
                }
                return;
            }
            motors_move_to_step(MOTOR_X_BITS, &_ParCable_drive_slow,
                                _micron_2_steps(CABLE_WASH_POS_BACK));
            switch (_Old_RobFunction)
            {
            case rob_fct_wash:
                Fpga.par->output |= RO_FLUSH_WIPE;
                Fpga.par->output |= RO_FLUSH_PUMP;
            case rob_fct_vacuum:
                Fpga.par->output |= RO_WASTE;
                break;
            default:
                break;
            }
            _PumpWasteTime = rx_get_ticks();
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

            _PumpWasteTime = 0;
            Fpga.par->output &= ~RO_WASTE;
            pos = (CABLE_SCREW_POS_BACK +
                   (((int)_RobFunction - (int)rob_fct_screw_cluster) *
                    (CABLE_SCREW_POS_FRONT - CABLE_SCREW_POS_BACK)) /
                       8);
            if (RX_StepperStatus.posY[0] < pos)
            {
                _lbrob_do_reference();
                _CmdRunning_old = msgId;
                return;
            }
            _lbrob_move_to_pos(_CmdRunning, _micron_2_steps(pos));
            break;

        default:
            Error(ERR_CONT, 0, "Command %s: Rob-Function %d not implemented",
                  _CmdName, _RobFunction);
        }
    }
}

// static void _turn_screw(int screwNr, int screwTurns)
static void _turn_screw(SHeadAdjustment headAdjustment)
{
    int screwNr;
    int screwSteps;
    screwNr = headAdjustment.printbarNo * HEADS_PER_COLOR * SCREWS_PER_HEAD +
              headAdjustment.headNo * SCREWS_PER_HEAD + headAdjustment.axis + 1;
    screwSteps = headAdjustment.steps;
    if (!RX_StepperStatus.info.moving && !RX_StepperStatus.robinfo.moving &&
        !RX_StepperStatus.screwerinfo.moving)
    {
        if (headAdjustment.headNo < -1 || (headAdjustment.headNo == -1 && headAdjustment.axis == AXE_ANGLE) || headAdjustment.headNo >= HEADS_PER_COLOR)
        {
            Error(ERR_CONT, 0, "Screw does not exist");
            return;
        }
        else if (headAdjustment.headNo == HEADS_PER_COLOR - 1 && headAdjustment.axis == AXE_DIST)
        {
            Error(ERR_CONT, 0, "Last Screw of each color is pointless to turn");
            return;
        }
        static int correction_value;
        static int wait_time = 0;
        int pos_min;
        
        _HeadAdjustment = headAdjustment;

        if (screwNr >= HEADS_PER_COLOR * 4)
        {
            Error(ERR_CONT, 0, "Screw %d does not exist", screwNr);
            _CmdScrewing = 0;
            return;
        }
        if (!_CmdScrewing && _check_in_screw_pos(headAdjustment))
        {
            _CmdScrewing = 4;
        }

        if (_CmdScrewing == 0) correction_value = 0;
        
        int pos;
        if (_CmdScrewing != 6) wait_time = 0;
        switch (_CmdScrewing)
        {
        case 0:
            RX_StepperStatus.screwerinfo.screwed = FALSE;
            pos = headAdjustment.headNo + rob_fct_screw_head0;
            if (_HeadPos != pos)
                lbrob_handle_ctrl_msg(INVALID_SOCKET, CMD_ROB_MOVE_POS, &pos);
            _CmdScrewing++;
            break;

        case 1:
            if (RX_StepperStatus.robinfo.ref_done &&
                _HeadPos == headAdjustment.headNo + rob_fct_screw_head0)
            {
                if (headAdjustment.headNo == -1)
                {
                    if (RX_StepperCfg.screwclusters[headAdjustment.printbarNo][0][0].posY)
                        pos = RX_StepperCfg.screwclusters[headAdjustment.printbarNo][0][0].posY + correction_value;
                    else
                        pos = SCREW_Y_FRONT + correction_value;
                }
                else if (RX_StepperCfg.screwpositions[headAdjustment.printbarNo][headAdjustment.headNo][headAdjustment.axis].posY)
                    pos = RX_StepperCfg.screwpositions[headAdjustment.printbarNo][headAdjustment.headNo][headAdjustment.axis].posY + correction_value;
                else if (headAdjustment.axis == AXE_DIST)
                    pos = SCREW_Y_FRONT + correction_value;
                else
                    pos = SCREW_Y_BACK + correction_value;
                robi_handle_ctrl_msg(INVALID_SOCKET, CMD_ROBI_MOVE_TO_Y, &pos);
                _CmdScrewing++;
            }
            break;
        case 2:
            if (RX_StepperStatus.screwerinfo.y_in_pos)
            {
                if (headAdjustment.headNo == -1)
                {
                    if (RX_StepperCfg.screwclusters[headAdjustment.printbarNo][0][0].posX)
                        pos = RX_StepperCfg.screwclusters[headAdjustment.printbarNo][0][0].posX + correction_value;
                    else if (headAdjustment.printbarNo == LEFT)
                        pos = SCREW_X_LEFT;
                    else
                        pos = SCREW_X_RIGHT;
                }
                else if (RX_StepperCfg.screwpositions[headAdjustment.printbarNo][headAdjustment.headNo][headAdjustment.axis].posX)
                    pos = RX_StepperCfg.screwpositions[headAdjustment.printbarNo][headAdjustment.headNo][headAdjustment.axis].posX;
                else if (headAdjustment.printbarNo == LEFT)
                    pos = SCREW_X_LEFT;
                else
                    pos = SCREW_X_RIGHT;
                robi_handle_ctrl_msg(INVALID_SOCKET, CMD_ROBI_MOVE_TO_X, &pos);
                _CmdScrewing++;
            }
            break;

        case 3:
            if (RX_StepperStatus.screwerinfo.x_in_pos)
            {
                lb702_handle_ctrl_msg(INVALID_SOCKET, CMD_LIFT_SCREW, NULL);
                _CmdScrewing++;
            }
            break;

        case 4:
            if (RX_StepperStatus.info.z_in_screw)
            {
                robi_handle_ctrl_msg(INVALID_SOCKET, CMD_ROBI_MOVE_Z_UP, NULL);
                _CmdScrewing++;
                _TimeSearchScrew = rx_get_ticks();
            }
            break;

        case 5:
            if (RX_StepperStatus.screwerinfo.z_in_up)
            {
                correction_value = 0;
                if (headAdjustment.axis == AXE_DIST && screwSteps < 0)
                {
                    screwSteps -= 12;
                    screwSteps = abs(screwSteps);
                    _HeadAdjustment.steps = 12;
                    //_ScrewTurns = 12;
                    robi_handle_ctrl_msg(INVALID_SOCKET, CMD_ROBI_SCREW_RIGHT,
                                         &screwSteps);
                }
                else if (headAdjustment.axis == AXE_ANGLE == screwSteps > 0)
                {
                    screwSteps += 12;
                    //_ScrewTurns = -12;
                    _HeadAdjustment.steps = -12;
                    robi_handle_ctrl_msg(INVALID_SOCKET, CMD_ROBI_SCREW_LEFT,
                                         &screwSteps);
                }
                else
                {
                    RX_StepperStatus.screwerinfo.screw_loosed = TRUE;
                }
                _CmdScrewing++;
            }
            else if (_TimeSearchScrew &&
                     rx_get_ticks() > _TimeSearchScrew + SCREW_SEARCHING_TIME)
            {
                _TimeSearchScrew = 0;
                if (abs(correction_value) >= 5000)
                {
                    Error(ERR_CONT, 0, "Screw %d of Head %d of Printhead %d not found", headAdjustment.axis, headAdjustment.headNo, headAdjustment.printbarNo);
                    _CmdScrewing = 0;
                    break;
                }
                else if (correction_value >= 0)
                    correction_value = (correction_value + 1000) * (-1);
                else
                    correction_value *= (-1);
                _CmdScrewing = 1;
            }
            else if (!RX_StepperStatus.screwerinfo.moving)
            {
                robi_handle_ctrl_msg(INVALID_SOCKET, CMD_ROBI_MOVE_Z_UP, NULL);
            }
            break;

        case 6:
            if (RX_StepperStatus.screwerinfo.screw_loosed)
            {
                if (!wait_time) wait_time = rx_get_ticks();
                if (rx_get_ticks() > wait_time + 200)
                {
                    if (screwSteps >= 0)
                        robi_handle_ctrl_msg(INVALID_SOCKET, CMD_ROBI_SCREW_LEFT, &screwSteps);
                    else
                    {
                        screwSteps = abs(screwSteps);
                        robi_handle_ctrl_msg(INVALID_SOCKET, CMD_ROBI_SCREW_RIGHT, &screwSteps);
                    }
                    _CmdScrewing++;
                }
            }
            break;
        case 7:
            if (RX_StepperStatus.screwerinfo.screw_tight)
            {
                RX_StepperCfg.screwpositions[headAdjustment.printbarNo][headAdjustment.headNo][headAdjustment.axis].posX = RX_StepperStatus.screw_posX;
                RX_StepperCfg.screwpositions[headAdjustment.printbarNo][headAdjustment.headNo][headAdjustment.axis].posY = RX_StepperStatus.screw_posY;
                _CmdScrewing++;
                robi_handle_ctrl_msg(INVALID_SOCKET, CMD_ROBI_MOVE_Z_DOWN, NULL);
            }
            break;

        case 8:
            if (RX_StepperStatus.screwerinfo.z_in_down)
            {
                RX_StepperStatus.screwerinfo.screwed = TRUE;
                _CmdScrewing = 0;
            }
            break;
        }
    }
}

static void _search_all_screws()
{
    static int correction_value = 0;
    static int cmd_Time;
    static int max_Wait_Time = 25000; // ms
    static int repeat;
    int test;
    int pos_min;
    int pos;
    if (!RX_StepperStatus.info.moving && !RX_StepperStatus.robinfo.moving &&
        !RX_StepperStatus.screwerinfo.moving)
    {
        if (_SearchScrewNr >= HEADS_PER_COLOR * COLORS_PER_STEPPER * SCREWS_PER_HEAD)
        {
            Error(ERR_CONT, 0, "Screw %d does not exist", _SearchScrewNr - 1);
            _CmdSearchScrews = 0;
            _SearchScrewNr = 0;
            return;
        }

        int front_screw = (_SearchScrewNr+1) % SCREWS_PER_HEAD;
        int head_nr = (((_SearchScrewNr % (SCREWS_PER_HEAD * HEADS_PER_COLOR)) + 1) / COLORS_PER_STEPPER)-1;
        int head_Dist = (abs(CABLE_SCREW_POS_FRONT) - abs(CABLE_SCREW_POS_BACK)) / 7;
        int screw_Dist = 16400; // um

        if (_CmdSearchScrews == 0) correction_value = 0;
        if (cmd_Time && rx_get_ticks() > cmd_Time + max_Wait_Time && !repeat)
        {
            _CmdSearchScrews--;
            repeat = 1;
            cmd_Time = rx_get_ticks();
        }
        else if (cmd_Time && rx_get_ticks() > cmd_Time + max_Wait_Time && repeat)
        {
            Error(ERR_CONT, 0, "Robot stock in searching screw step %d at screw %d", _CmdSearchScrews, _SearchScrewNr);
            cmd_Time = 0;
            _CmdSearchScrews = 0;
            repeat = 0;
            return;
        }
        if (cmd_Time == 0) repeat = 0;

        switch (_CmdSearchScrews)
        {
        case 0:
            pos = head_nr + rob_fct_screw_head0;
            if (pos != _HeadPos)
                lbrob_handle_ctrl_msg(INVALID_SOCKET, CMD_ROB_MOVE_POS, &pos);
            _CmdSearchScrews++;
            cmd_Time = rx_get_ticks();
            break;
        case 1:
            if (RX_StepperStatus.robinfo.ref_done &&
                _HeadPos == head_nr + rob_fct_screw_head0)
            {
                if (_SearchScrewNr == 0)
                    pos = SCREW_Y_FRONT + correction_value;
                else if (_SearchScrewNr == SCREWS_PER_HEAD * HEADS_PER_COLOR)
                    pos = RX_StepperCfg.screwclusters[0][0][0].posY + correction_value;
                else if (_SearchScrewNr == 1 || _SearchScrewNr == SCREWS_PER_HEAD * HEADS_PER_COLOR + 1)
                    pos = RX_StepperCfg.screwclusters[_SearchScrewNr / (SCREWS_PER_HEAD * HEADS_PER_COLOR)][0][0].posY + SCREW_Y_BACK - SCREW_Y_FRONT + correction_value;
                else
                    pos = _calculate_average_y_pos(_SearchScrewNr);
                cmd_Time = rx_get_ticks();
                robi_handle_ctrl_msg(INVALID_SOCKET, CMD_ROBI_MOVE_TO_Y, &pos);
                _CmdSearchScrews++;
            }
            break;
        case 2:
            if (RX_StepperStatus.screwerinfo.y_in_pos)
            {
                if (_SearchScrewNr == 0)
                    pos = SCREW_X_LEFT;
                else if (_SearchScrewNr == SCREWS_PER_HEAD * HEADS_PER_COLOR)
                    pos = RX_StepperCfg.screwclusters[0][0][0].posX -
                          SCREW_X_LEFT + SCREW_X_RIGHT;
                else if (_SearchScrewNr % (SCREWS_PER_HEAD * HEADS_PER_COLOR) == 1)
                    pos = RX_StepperCfg.screwclusters[_SearchScrewNr / (SCREWS_PER_HEAD * HEADS_PER_COLOR)][0][0].posX;
                else 
                {
                    pos = RX_StepperCfg.screwpositions[_SearchScrewNr / (SCREWS_PER_HEAD * HEADS_PER_COLOR)][((_SearchScrewNr - 2) % HEADS_PER_COLOR) / SCREWS_PER_HEAD][(_SearchScrewNr - 2) % SCREWS_PER_HEAD].posX -
                          RX_StepperCfg.screwclusters[_SearchScrewNr / (SCREWS_PER_HEAD * HEADS_PER_COLOR)][0][0].posX;
                    /*int y_dist_old = ((((_SearchScrewNr-1) % (SCREWS_PER_HEAD * HEADS_PER_COLOR)) / SCREWS_PER_HEAD)+1) * head_Dist -
                                     ((((_SearchScrewNr-1) % (SCREWS_PER_HEAD * HEADS_PER_COLOR))) % SCREWS_PER_HEAD) * screw_Dist;*/
                    int y_dist_old = ((((_SearchScrewNr - 1 + 1) / SCREWS_PER_HEAD) % (SCREWS_PER_HEAD * HEADS_PER_COLOR))) * head_Dist - 
                                     ((_SearchScrewNr - 1) % SCREWS_PER_HEAD) * screw_Dist;

                    //int y_dist = (_SearchScrewNr % (SCREWS_PER_HEAD * HEADS_PER_COLOR)) / SCREWS_PER_HEAD * head_Dist +
                    //    ((_SearchScrewNr % (SCREWS_PER_HEAD * HEADS_PER_COLOR)) % SCREWS_PER_HEAD) * screw_Dist;
                    int y_dist = ((((_SearchScrewNr + 1) / SCREWS_PER_HEAD) % (SCREWS_PER_HEAD * HEADS_PER_COLOR))) * head_Dist -
                                     (_SearchScrewNr % SCREWS_PER_HEAD) * screw_Dist;
                    pos = RX_StepperCfg.screwclusters[_SearchScrewNr / (SCREWS_PER_HEAD * HEADS_PER_COLOR)][0][0].posX + pos * y_dist / y_dist_old;
                }
                cmd_Time = rx_get_ticks();
                robi_handle_ctrl_msg(INVALID_SOCKET, CMD_ROBI_MOVE_TO_X, &pos);
                _CmdSearchScrews++;
            }
            break;
        case 3:
            if (RX_StepperStatus.screwerinfo.x_in_pos)
            {
                cmd_Time = rx_get_ticks();
                lb702_handle_ctrl_msg(INVALID_SOCKET, CMD_LIFT_SCREW, NULL);
                _CmdSearchScrews++;
            }
            break;

        case 4:
            if (RX_StepperStatus.info.z_in_screw)
            {
                cmd_Time = rx_get_ticks();
                robi_handle_ctrl_msg(INVALID_SOCKET, CMD_ROBI_MOVE_Z_UP, NULL);
                _CmdSearchScrews++;
                _TimeSearchScrew = rx_get_ticks();
            }
            break;

        case 5:
            if (RX_StepperStatus.screwerinfo.z_in_up)
            {
                if (_SearchScrewNr % (HEADS_PER_COLOR * SCREWS_PER_HEAD) == 0)
                {
                    RX_StepperCfg.screwclusters[_SearchScrewNr / (HEADS_PER_COLOR * SCREWS_PER_HEAD)][0][0].posX = RX_StepperStatus.screw_posX;
                    RX_StepperCfg.screwclusters[_SearchScrewNr / (HEADS_PER_COLOR * SCREWS_PER_HEAD)][0][0].posY = RX_StepperStatus.screw_posY;
                }
                else
                {
                    RX_StepperCfg.screwpositions[_SearchScrewNr / (SCREWS_PER_HEAD * HEADS_PER_COLOR)]
                                   [((_SearchScrewNr - 1) % (SCREWS_PER_HEAD * HEADS_PER_COLOR)) / SCREWS_PER_HEAD]
                                   [(_SearchScrewNr - 1) % SCREWS_PER_HEAD].posX = RX_StepperStatus.screw_posX;
                    RX_StepperCfg.screwpositions[_SearchScrewNr / (SCREWS_PER_HEAD * HEADS_PER_COLOR)]
                                   [((_SearchScrewNr - 1) % (SCREWS_PER_HEAD * HEADS_PER_COLOR)) /SCREWS_PER_HEAD]
                                   [(_SearchScrewNr - 1) % SCREWS_PER_HEAD].posY = RX_StepperStatus.screw_posY;
                }
                
                cmd_Time = 0;
                _CmdSearchScrews = 0;
                _SearchScrewNr++;
                _TimeSearchScrew = 0;
                correction_value = 0;
                if (_SearchScrewNr < 4 * HEADS_PER_COLOR)
                {
                    lbrob_handle_ctrl_msg(INVALID_SOCKET, CMD_SEARCH_ALL_SCREWS,
                                          NULL);
                }
                else
                {
                    _SearchScrewNr = 0;
                    RX_StepperStatus.screwerinfo.screws_found = TRUE;
                    robi_handle_ctrl_msg(INVALID_SOCKET, CMD_ROBI_MOVE_Z_DOWN,
                                         NULL);
                }
            }
            else if (_TimeSearchScrew &&
                     rx_get_ticks() > _TimeSearchScrew + SCREW_SEARCHING_TIME)
            {
                cmd_Time = 0;
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

//--- _lbrob_motor_test ---------------------------------
static void _lbrob_motor_test(int motorNo, int steps)
{
    motorNo = 4;
    int motors = 1 << motorNo;
    SMovePar par;
    int i;

    memset(&par, 0, sizeof(SMovePar));

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

//--- _lbrob_motor_enc_reg_test -----------------------------------------------
static void _lbrob_motor_enc_reg_test(int steps)
{
    RX_StepperStatus.robinfo.moving = TRUE;
    _CmdRunning = 1;

    motors_move_by_step(MOTOR_X_0, &_ParCable_drive, -steps, FALSE);
}

//--- _check_in_screw_pos --------------------------------------
// static int _check_in_screw_pos(int screwNr)
static int _check_in_screw_pos(SHeadAdjustment headAdjustment)
{
    if (!RX_StepperStatus.screwerinfo.z_in_up) return FALSE;

    if ((abs(RX_StepperStatus.screw_posX - SCREW_X_LEFT) > MAX_VAR_SCREW_POS &&
         abs(RX_StepperStatus.screw_posX - SCREW_X_RIGHT) >
             MAX_VAR_SCREW_POS) ||
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
        headAdjustment.axis == AXE_DIST)
        return FALSE;

    int headNr;
    int pos = RX_StepperStatus.posY[0];

    for (headNr = 0; headNr < HEADS_PER_COLOR; headNr++)
    {
        if (pos - (CABLE_SCREW_POS_BACK +
                   (headNr * (CABLE_SCREW_POS_FRONT - CABLE_SCREW_POS_BACK)) /
                       7) <
            MAX_VAR_SCREW_POS)
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
        return RX_StepperCfg.screwclusters[screwNr / (SCREWS_PER_HEAD * HEADS_PER_COLOR)][0][0].posY;
    
    if (screwNr % (SCREWS_PER_HEAD * HEADS_PER_COLOR) == 3)
        return RX_StepperCfg.screwpositions[screwNr / (SCREWS_PER_HEAD * HEADS_PER_COLOR)][0][0].posY;
    
    if (screwNr < SCREWS_PER_HEAD * HEADS_PER_COLOR)
    {
        i = (screwNr % SCREWS_PER_HEAD);
        //if (screwNr % 2) y_combined += RX_StepperCfg.screwclusters[0][0][0].posY;
    }
    else
    {
        i = SCREWS_PER_HEAD * HEADS_PER_COLOR + (screwNr % SCREWS_PER_HEAD);
        //if (screwNr % 2) y_combined += RX_StepperCfg.screwclusters[1][0][0].posY;
    }
    
    for (; i < screwNr - 1; i += SCREWS_PER_HEAD)
    {
        if (i == 0 || i == 16)
            y_combined += RX_StepperCfg.screwclusters[i / (SCREWS_PER_HEAD * HEADS_PER_COLOR)][0][0].posY;
        else
        {
            printbarNo = i / (SCREWS_PER_HEAD * HEADS_PER_COLOR);
            headNo = ((i - 1) % (SCREWS_PER_HEAD * HEADS_PER_COLOR)) /
                     SCREWS_PER_HEAD;
            axis = (i + 1) % SCREWS_PER_HEAD;
            y_combined +=
                RX_StepperCfg.screwpositions[printbarNo][headNo][axis].posY;    
            }
            
    }
    return y_combined / ((screwNr % (SCREWS_PER_HEAD * HEADS_PER_COLOR)) / SCREWS_PER_HEAD);
}

void lbrob_reset_variables(void)
{
    _NewCmd = FALSE;
}