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
#include "lbrob.h"
//#include "robi_interface.h"
#include "lb702.h"

#define MOTOR_X_0 4

#define MOTOR_X_BITS 0x10
#define MOTOR_ALL_BITS 0x13

#define X_STEPS_PER_METER 636619.0 // 100000	//636619.0
#define X_INC_PER_METER 316507.0   // 49717.0

//#define CABLE_CAP_POS			-540000	//						steps PROTOTYPE
//#define CABLE_CAP_POS			-436000	//						steps LB702
//#define CABLE_WIPE_POS_FRONT	-410000	//-460000				steps LB702
//#define CABLE_WIPE_POS_BACK	-180000 //						steps LB702

//#define CABLE_CAP_POS			-848231	// PROTOTYP TEST		um PROTOYPE
#define CABLE_CAP_POS -684868 //						um LB702
//#define CABLE_WIPE_POS_FRONT	-800000	//						um PROTOYPE
#define CABLE_WIPE_POS_FRONT -644027 //						um LB702
//#define CABLE_WIPE_POS_BACK		-282000	//						um PROTOYPE
#define CABLE_WIPE_POS_BACK -290000 //-282743	//						um LB702

#define CURRENT_HOLD 200

#define CAP_FILL_TIME 180000      // ms
#define MAX_PUMP_TIME 300000      // ms
#define WASTE_PUMP_TIME 60000     // ms
#define WASTE_VALVE_INTERVAL 1000 // ms

// Digital Inputs
#define CABLE_PULL_REF 2
#define CAPPING_ENDSTOP 3

#define CABLE_PULL_REF_BIT 0x04
#define CAPPING_ENDSTOP_BIT 0x08

// Digital Outputs
#define RO_ALL_OUTPUTS      0x0BF   // All outputs except of outputs 8, 9, 10 and 11
#define RO_FLUSH_TO_CAP     0x001   // o0
#define RO_FLUSH_LEFT       0x002   // o1
#define RO_FLUSH_RIGHT      0x004   // o2
#define RO_SCREW_AIR_VALVE  0x008   // o3
#define RO_VACUUM_VALVE     0x010   // o4
#define RO_FLUSH_PUMP       0x020   // o5
#define RO_WASTE_PUMP       0x040   // o6
#define RO_EN_VACUUM        0x080   // o7
#define RO_CAP_WASTE_LEFT   0x100   // o8
#define RO_CAP_WASTE_RIGHT  0x200   // o9
#define RO_CAP_WASTE_SUMP   0x400   // o10
//#define RO_VACUUM_WASTE_SUMP	0x800		// o11
#define RO_CAP_WASTE_TUB    0x800   // o11

#define RO_CAP_WASTE_ALL    0x700   // o8, o9, o10

#define MAX_POS_DIFFERENT   3000    // steps

// globals

int _CmdRunning_Lift = 0;
int _CmdStep = 0;

// static
static SMovePar _ParCable_ref;
static SMovePar _ParCable_drive;
static SMovePar _ParCable_drive_purge;


static int _CmdRunning = 0;
static char _CmdName[32];

static int _NewCmd = 0;

static int _TimeNow = 0;

static int _WasteValves[] = {RO_CAP_WASTE_LEFT, RO_CAP_WASTE_RIGHT, RO_CAP_WASTE_SUMP};
static int _WasteValveSelect = 0;
static int _PrevWasteValveSelect = 0;
static int _WasteValveSwitched = 0;

static int _PumpState = FALSE;
static int _PumpState_old = FALSE;

static int _PumpStartTime = 0;
static int _CapFillTime = 0;
static int _PumpWasteBackOutTime = 0;

static int _FrontPumpUsed = FALSE;

static ERobotFunctions _RobFunction = 0;
static ERobotFunctions _Old_RobFunction = 0;

//--- prototypes --------------------------------------------
static void _lbrob_motor_z_test(int steps);
static void _lbrob_motor_test(int motor, int steps, int Cmd);
static void _lbrob_do_reference(int step);
static void _lbrob_move_to_pos(int cmd, int pos);
static void _lbrob_motor_enc_reg_test(int steps);
static int _micron_2_steps(int micron);
static void _set_waste_pump(int on);
static void _cln_move_to(int msgId, ERobotFunctions fct);
static void _set_waste_pump(int on);
static void _check_pump(void);

static int _CmdRunning_old = 0;

//--- lbrob_init --------------------------------------
void lbrob_init(void)
{
    memset(_CmdName, 0, sizeof(_CmdName));
    memset(&_ParCable_ref, 0, sizeof(SMovePar));
    memset(&_ParCable_drive, 0, sizeof(SMovePar));
    memset(&_ParCable_drive_purge, 0, sizeof(SMovePar));

    // config for referencing cable pull motor (motor 4)
    _ParCable_ref.speed = 16000; // defined by Peter // 10000
    _ParCable_ref.accel = 32000; // defined by Peter // 10000
    _ParCable_ref.current_acc = 400.0;
    _ParCable_ref.current_run = 400.0;
    _ParCable_ref.stop_mux = 0;
    _ParCable_ref.dis_mux_in = 0;
    _ParCable_ref.estop_level = 0;
    _ParCable_ref.enc_bwd = TRUE;
    _ParCable_ref.encCheck = chk_off;

    // config for moving normal with cable pull motor (motor 4)
    // This commands that use this config need to start the motor with the
    // special encoder mode
    _ParCable_drive.speed = 32000; // defined by Peter // 10000
    _ParCable_drive.accel = 32000; // defined by Peter // 10000
    _ParCable_drive.current_acc = 400.0;
    _ParCable_drive.current_run = 400.0;
    _ParCable_drive.stop_mux = 0;
    _ParCable_drive.dis_mux_in = 0;
    _ParCable_drive.estop_level = 0;
    _ParCable_drive.enc_bwd = TRUE;
    //	_ParCable_drive.encCheck = chk_lbrob;
    _ParCable_drive.encCheck = chk_off;

    _ParCable_drive_purge.speed = RX_StepperCfg.wipe_speed * X_STEPS_PER_METER / 1000; // divided by 1000 to get from steps/m to steps/mm
    _ParCable_drive_purge.accel = 8000;
    _ParCable_drive_purge.current_acc = 400.0;
    _ParCable_drive_purge.current_run = 400.0;
    _ParCable_drive_purge.stop_mux = 0;
    _ParCable_drive_purge.dis_mux_in = 0;
    _ParCable_drive_purge.estop_level = 0;
    _ParCable_drive_purge.enc_bwd = TRUE;
    _ParCable_drive_purge.encCheck = chk_lbrob;

    motor_config(MOTOR_X_0, CURRENT_HOLD, X_STEPS_PER_METER, X_INC_PER_METER, STEPS);
}

//--- _set_waste_pump ------------------------------------------------
static void _set_waste_pump(int on)
{
    int tmp = Fpga.par->output & RO_WASTE_PUMP;
    if (RX_StepperCfg.boardNo == 0)
    {
        if (on)
        {
            Fpga.par->output |= RO_WASTE_PUMP;
        }
        else if (Fpga.par->output & RO_WASTE_PUMP && !_PumpWasteBackOutTime)
            _PumpWasteBackOutTime = rx_get_ticks();
        else if ((rx_get_ticks() >= _PumpWasteBackOutTime + WASTE_PUMP_TIME) && _PumpWasteBackOutTime)
        {
            Fpga.par->output &= ~RO_WASTE_PUMP;
            Fpga.par->output &= ~RO_CAP_WASTE_TUB;
            Fpga.par->output &= ~RO_CAP_WASTE_ALL;
            _PumpWasteBackOutTime = 0;
        }
    }
    else
    {
        if (on) Fpga.par->output |= RO_WASTE_PUMP;
        else    Fpga.par->output &= ~RO_WASTE_PUMP;
    }

    _PumpState_old = _PumpState;
}

//--- _check_pump ---------------------------------------------------------------
static void _check_pump(void)
{
    _TimeNow = rx_get_ticks();
    if (RX_StepperCfg.boardNo == 0)
    {
        if ((_TimeNow >= _PumpWasteBackOutTime + WASTE_PUMP_TIME) && _PumpWasteBackOutTime)
        {
            _set_waste_pump(FALSE);
        }
        if (_PumpWasteBackOutTime)
        {
            Fpga.par->output |= RO_CAP_WASTE_TUB;
            Fpga.par->output &= ~RO_CAP_WASTE_ALL;
        }
        else    Fpga.par->output &= ~RO_CAP_WASTE_TUB;
    }

    if (RX_StepperCfg.boardNo == 0)
    {
        _PumpState = _CmdRunning || _PumpStartTime || _CmdRunning_Lift || RX_StepperStatus.robinfo.purge_ready || _PumpWasteBackOutTime;
        if (_CmdRunning || _PumpStartTime || _CmdRunning_Lift || RX_StepperStatus.robinfo.purge_ready)
            _PumpWasteBackOutTime = 0;
    }
    else
        _PumpState = _CmdRunning || _PumpStartTime || _CmdRunning_Lift || RX_StepperStatus.robinfo.purge_ready;

    if (_PumpState && (!_PumpWasteBackOutTime || RX_StepperCfg.boardNo != 0))
    {
        int t = _TimeNow / WASTE_VALVE_INTERVAL;

        if (t > _WasteValveSwitched)
        {
            _WasteValveSwitched = t;
            if (_FrontPumpUsed)
            {
                Fpga.par->output &= ~RO_CAP_WASTE_ALL;
                Fpga.par->output |= RO_CAP_WASTE_SUMP;
            }
            else
            {
                _PrevWasteValveSelect = _WasteValveSelect++;
                if (_WasteValveSelect >= SIZEOF(_WasteValves))
                    _WasteValveSelect = 0;
                Fpga.par->output |= _WasteValves[_WasteValveSelect];
                Fpga.par->output &= ~_WasteValves[_PrevWasteValveSelect];
            }
        }
    }

    if ((_CmdRunning && _CmdRunning != CMD_ROB_FILL_CAP) || (!_CmdRunning && _RobFunction == rob_fct_cap && !_PumpState))
        _FrontPumpUsed = FALSE;

    if (_TimeNow >= _PumpStartTime + MAX_PUMP_TIME && _PumpStartTime)
    {
        _PumpStartTime = 0;
        if (RX_StepperCfg.boardNo == 0 && !_PumpWasteBackOutTime) _PumpWasteBackOutTime = rx_get_ticks();
    }


    if (_PumpState != _PumpState_old)
    {
        _set_waste_pump(_PumpState);
    }

    if (_TimeNow >= _PumpStartTime + MAX_PUMP_TIME) _PumpStartTime = 0;
}

//--- lbrob_main ------------------------------------------------------------------
void lbrob_main(int ticks, int menu)
{
    int motor;
    int SensorRef, SensorCap;
    RX_StepperStatus.posX = motor_get_step(MOTOR_X_0);
    SStepperStat oldSatus;
    memcpy(&oldSatus, &RX_StepperStatus, sizeof(RX_StepperStatus));

    _check_pump();

    motor_main(ticks, menu);

    RX_StepperStatus.robinfo.moving = (_CmdRunning != 0);
    if (RX_StepperStatus.robinfo.moving)
    {
        RX_StepperStatus.info.x_in_ref = FALSE;
        RX_StepperStatus.info.x_in_cap = FALSE;
        RX_StepperStatus.robinfo.cap_ready = FALSE;
        RX_StepperStatus.robinfo.rob_in_cap = FALSE;
        RX_StepperStatus.robinfo.purge_ready = FALSE;
    }

    SensorRef = fpga_input(CABLE_PULL_REF);
    SensorCap = fpga_input(CAPPING_ENDSTOP);

    if (_CmdRunning && (motors_move_done(MOTOR_X_BITS) || (_CmdRunning == CMD_ROB_REFERENCE && SensorRef) || (_CmdRunning == CMD_ROB_MOVE_POS && SensorCap)))
    {
        RX_StepperStatus.robinfo.moving = FALSE;


        RX_StepperStatus.info.x_in_ref = fpga_input(CABLE_PULL_REF);
        RX_StepperStatus.info.x_in_cap = fpga_input(CAPPING_ENDSTOP);

        if (_CmdRunning == CMD_ROB_REFERENCE)
        {
            if (!_CmdStep)
            {
                _lbrob_do_reference(1);
                Error(LOG, 0, "REF small move done - start REF to sensor");
            }
            else if (motors_error(MOTOR_X_BITS, &motor) || SensorRef)
            {
                motors_reset(MOTOR_X_BITS);
                _CmdStep++;
                if (RX_StepperStatus.info.x_in_ref)
                {
                    // motors_reset(MOTOR_X_BITS);
                    RX_StepperStatus.robinfo.ref_done = TRUE;
                    Error(LOG, 0, "REF to sensor done");
                }
                else
                {
                    Error(ERR_CONT, 0, "LBROB: Command %s: End Sensor REF NOT HIGH", _CmdName);
                    RX_StepperStatus.robinfo.ref_done = FALSE;
                    _CmdRunning_old = FALSE;
                }
            }
        }

        if (_CmdRunning != CMD_ROB_REFERENCE)
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
        }

        if (_CmdRunning == CMD_ROB_FILL_CAP)
        {
            switch (_RobFunction)
            {
            case rob_fct_cap:           if ((rx_get_ticks() >= _CapFillTime + CAP_FILL_TIME) && _CapFillTime)
                                        {
                                            _CmdRunning = FALSE;
                                            Fpga.par->output &= ~RO_ALL_OUTPUTS;
                                            RX_StepperStatus.robinfo.cap_ready = RX_StepperStatus.info.x_in_cap && !RX_StepperStatus.robinfo.moving;
                                            _CapFillTime = 0;
                                            Error(LOG, 0, "_CapFillTime to 0");
                                        }
                                        break;

            case rob_fct_purge_all:     _CmdRunning = FALSE;
                                        RX_StepperStatus.robinfo.purge_ready = TRUE;
                                        break;
            case rob_fct_purge_head0:
            case rob_fct_purge_head1:
            case rob_fct_purge_head2:
            case rob_fct_purge_head3:
            case rob_fct_purge_head4:
            case rob_fct_purge_head5:
            case rob_fct_purge_head6:
            case rob_fct_purge_head7:   _CmdRunning = FALSE;
                                        _PumpStartTime = rx_get_ticks();
                                        break;

            default:                    Error(ERR_CONT, 0, "Command %s: Robi-Function %d not implemented", _CmdName, _RobFunction); break;
            }
            _Old_RobFunction = _RobFunction;
        }
        else if (_CmdRunning == CMD_ROB_MOVE_POS)
        {
            switch (_RobFunction)
            {
            case rob_fct_cap:           RX_StepperStatus.robinfo.rob_in_cap = fpga_input(CAPPING_ENDSTOP);
                                        if (!RX_StepperStatus.robinfo.rob_in_cap && _NewCmd != CMD_ROB_MOVE_POS)
                                        {
                                            Error(ERR_CONT, 0, "LBROB: Command %s: End Sensor Capping NOT HIGH", _CmdName);
                                            RX_StepperStatus.robinfo.ref_done = FALSE;
                                        }
                                        _CmdRunning = FALSE;
                                        break;

            case rob_fct_purge_all:     RX_StepperStatus.robinfo.rob_in_cap = fpga_input(CAPPING_ENDSTOP);
                                        RX_StepperStatus.robinfo.purge_ready = RX_StepperStatus.robinfo.rob_in_cap;
                                        if (!RX_StepperStatus.robinfo.purge_ready && _NewCmd != CMD_ROB_MOVE_POS)
                                        {
                                            Error(ERR_CONT, 0, "LBROB: Command %s: Robot not in correct position", _CmdName);
                                            RX_StepperStatus.robinfo.ref_done = FALSE;
                                        }
                                        _CmdRunning = FALSE;
                                        break;
                                        
            case rob_fct_purge_head0:   RX_StepperStatus.robinfo.purge_ready = (abs(motor_get_step(MOTOR_X_0) - motor_get_end_step(MOTOR_X_0)) <= MAX_POS_DIFFERENT);
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
            case rob_fct_purge_head7:   RX_StepperStatus.robinfo.purge_ready = TRUE;
                                        _CmdRunning = FALSE;
                                        break;
                                        
            default:                    Error(ERR_CONT, 0, "LBROB_MAIN: Rob-Function %d not implemented", _RobFunction);
                                        _CmdRunning = FALSE;
                                        RX_StepperStatus.robinfo.ref_done = FALSE;
                                        break;
            }
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
            case CMD_ROB_MOVE_POS: lbrob_handle_ctrl_msg(INVALID_SOCKET, CMD_ROB_MOVE_POS, &_RobFunction); break;
            default: Error(ERR_CONT, 0, "LBROB_MAIN: Command 0x%08x not implemented", _CmdRunning_old); break;
            }
            loc_new_cmd = FALSE;
        }
        else if (!(_CmdRunning == CMD_ROB_REFERENCE && _CmdStep == 1))
        {
            _CmdRunning = FALSE;
            _CmdRunning_old = FALSE;
        }
    }

    if (_CmdRunning_Lift)
    {
        int loc_new_cmd = 0;
        if (!RX_StepperStatus.robinfo.moving && !RX_StepperStatus.info.moving)
        {
            loc_new_cmd = _NewCmd;
            _NewCmd = FALSE;
            _CmdRunning_Lift = FALSE;
        }
        if (loc_new_cmd)
        {
            switch (loc_new_cmd)
            {
            case CMD_ROB_REFERENCE: lbrob_handle_ctrl_msg(INVALID_SOCKET, CMD_ROB_REFERENCE, NULL); break;
            case CMD_ROB_MOVE_POS: lbrob_handle_ctrl_msg(INVALID_SOCKET, CMD_ROB_MOVE_POS, &_RobFunction); break;
            default: Error(ERR_CONT, 0, "LBROB_MAIN: Command 0x%08x not implemented", loc_new_cmd); break;
            }
        }
        loc_new_cmd = FALSE;
    }

    if (memcmp(&oldSatus.robinfo, &RX_StepperStatus.robinfo, sizeof(RX_StepperStatus.robinfo)))
    {
        ctrl_send_2(REP_STEPPER_STAT, sizeof(RX_StepperStatus), &RX_StepperStatus);
    }
}

//---_micron_2_steps --------------------------------------------------------------
static int _micron_2_steps(int micron)
{
    // return (int)(0.5 + STEPS_REV / DIST_REV*micron);
    return (int)((0.5 + micron * X_STEPS_PER_METER) / 1000000);
}

//--- _lbrob_display_status --------------------------------------------------------
void lbrob_display_status(void)
{
    term_printf("LB ROB ---------------------------------\n");
    if (RX_StepperStatus.robot_used)
    {
        term_printf("moving:         %d		cmd: %08x\n", RX_StepperStatus.robinfo.moving, _CmdRunning);
        term_printf("Ref sensor:     %d\n", fpga_input(CABLE_PULL_REF));
        term_printf("Cap sensor:     %d\n", fpga_input(CAPPING_ENDSTOP));
        term_printf("reference done: %d Step=%d\n", RX_StepperStatus.robinfo.ref_done, _CmdStep);
        term_printf("x in reference: %d\n", RX_StepperStatus.info.x_in_ref);
        term_printf("x in cap:       %d\n", RX_StepperStatus.info.x_in_cap);
        term_printf("Cap ready       %d\n", RX_StepperStatus.robinfo.cap_ready);
        term_printf("Purge ready     %d\n", RX_StepperStatus.robinfo.purge_ready);
        term_printf("actPos Robi:    %d\n", RX_StepperStatus.posX);
        term_printf("Wipe-Speed:     %d\n", RX_StepperCfg.wipe_speed);
        if (_PumpStartTime)
            term_printf("Pump-Time:      %d\n", ((MAX_PUMP_TIME - (rx_get_ticks() - _PumpStartTime)) / 1000));
        else
            term_printf("Pump-Time:      %d\n", _PumpStartTime);
        if (_CapFillTime != 0)
            term_printf("Cap-Fill-Time:  %d\n", ((CAP_FILL_TIME - (rx_get_ticks() - _CapFillTime)) / 1000));
        else
            term_printf("Cap-Fill-Time:  %d\n", _CapFillTime);
        if (_PumpWasteBackOutTime && RX_StepperCfg.boardNo == 0)
            term_printf("Pump-Back-Time: %d\n", ((WASTE_PUMP_TIME - (rx_get_ticks() - _PumpWasteBackOutTime)) / 1000));
        else if (RX_StepperCfg.boardNo == 0)
            term_printf("Pump-Back-Time: %d\n", _PumpWasteBackOutTime);
        term_printf("\n");
    }
    else
    {
        term_printf("No Robot (bridge stepper board not installed)\n");
    }
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
        term_printf("c: move to cap Robi\n");
        term_printf("g<n>: Go to purge position of head 1 -8 or for all (0)\n");
        term_printf("f: Fill Cap\n");
        term_printf("w: Wipe Heads\n");
        term_printf("m<n><steps>: move Motor<n> by <steps>\n");
        term_printf("e<steps>: move Cablepull with encoder regulator by <steps>\n");
        term_printf("p: Start pump for %d seconds\n", MAX_PUMP_TIME / 1000);
        term_printf("w: Move to Purge Front Position\n");
        term_flush();
    }
    else
    {
        term_printf("?: help\n");
        term_printf("1: LB702 menu\n");
    }
}

//--- lbrob_handle_menu -----------------------------------------------
void lbrob_handle_menu(char *str)
{
    int pos = 10000;
    switch (str[0])
    {
    case 's': lbrob_handle_ctrl_msg(INVALID_SOCKET, CMD_ROB_STOP, NULL); break;
    case 'o': Fpga.par->output ^= (1 << atoi(&str[1])); break;
    case 'R': lbrob_handle_ctrl_msg(INVALID_SOCKET, CMD_ROB_REFERENCE, NULL); break;
    case 'r': motor_reset(atoi(&str[1])); break;
    case 'c': pos = rob_fct_cap;
             lbrob_handle_ctrl_msg(INVALID_SOCKET, CMD_ROB_MOVE_POS, &pos); break;
    case 'g': switch (str[1])
            {
            case '0': pos = rob_fct_purge_all; break;
            case '1': pos = rob_fct_purge_head0; break;
            case '2': pos = rob_fct_purge_head1; break;
            case '3': pos = rob_fct_purge_head2; break;
            case '4': pos = rob_fct_purge_head3; break;
            case '5': pos = rob_fct_purge_head4; break;
            case '6': pos = rob_fct_purge_head5; break;
            case '7': pos = rob_fct_purge_head6; break;
            case '8': pos = rob_fct_purge_head7; break;
            }
        lbrob_handle_ctrl_msg(INVALID_SOCKET, CMD_ROB_MOVE_POS, &pos); break;
    case 'f':
        pos = rob_fct_cap;
        lbrob_handle_ctrl_msg(INVALID_SOCKET, CMD_ROB_FILL_CAP, &pos); break;
    case 'w':
        pos = rob_fct_purge_all;
        lbrob_handle_ctrl_msg(INVALID_SOCKET, CMD_ROB_FILL_CAP, &pos); break;
    case 'm': _lbrob_motor_test(str[1] - '0', atoi(&str[2]), 1); break;
    case 'e': _lbrob_motor_enc_reg_test(atoi(&str[1])); break;
    case 'p': _PumpStartTime = rx_get_ticks(); break;
    }
}

//--- _lbrob_move_to_pos ---------------------------------------------------------------
static void _lbrob_move_to_pos(int cmd, int pos)
{
    _CmdRunning = cmd;
    RX_StepperStatus.robinfo.moving = TRUE;
    motors_move_to_step(MOTOR_X_0, &_ParCable_drive, pos);
}

static void _lbrob_do_reference(int step)
{
    motor_reset(MOTOR_X_0);
    Fpga.par->output &= ~RO_ALL_OUTPUTS;
    RX_StepperStatus.robinfo.ref_done = FALSE;
    _NewCmd = 0;
    _CmdRunning = CMD_ROB_REFERENCE;
    RX_StepperStatus.robinfo.moving = TRUE;
    
    if (!step)
    {
        _lbrob_motor_test(MOTOR_X_0, 10000, CMD_ROB_REFERENCE);
    }
    else
    {
        _PumpStartTime = rx_get_ticks();
        motors_move_by_step(1 << MOTOR_X_0, &_ParCable_ref, -CABLE_CAP_POS, TRUE);
    }
    _CmdStep = step;
}

//--- lbrob_handle_ctrl_msg -----------------------------------
int lbrob_handle_ctrl_msg(RX_SOCKET socket, int msgId, void *pdata)
{
    int val, pos;

    switch (msgId)
    {
    case CMD_ROB_STOP:              strcpy(_CmdName, "CMD_ROB_STOP");
        motors_stop(MOTOR_ALL_BITS);
        Fpga.par->output &= ~RO_ALL_OUTPUTS;
        _CmdRunning_Lift = 0;
        _CmdRunning = 0;
        break;

    case CMD_ROB_REFERENCE:         strcpy(_CmdName, "CMD_ROB_REFERENCE");
        if (_CmdRunning){lbrob_handle_ctrl_msg(INVALID_SOCKET, CMD_ROB_STOP, NULL); _NewCmd = CMD_ROB_REFERENCE; break;}
        if (!RX_StepperStatus.info.z_in_ref || RX_StepperStatus.info.z_in_up)
        {
            if (!RX_StepperStatus.info.moving)
            {
                Error(WARN, 0, "lbrob CMD_CLN_REFER");
                _CmdRunning_Lift = CMD_LIFT_REFERENCE;
                lb702_handle_ctrl_msg(INVALID_SOCKET, _CmdRunning_Lift, NULL);
                _NewCmd = msgId;
            }
            // Error(ERR_CONT, 0, "Command %s: Basket is too low", _CmdName);
            break;
        }
        _CmdRunning = msgId;
        RX_StepperStatus.robinfo.purge_ready = FALSE;
        _lbrob_do_reference(0);
        break;

    case CMD_ROB_MOVE_POS:          strcpy(_CmdName, "CMD_ROB_MOVE_POS");
        _cln_move_to(msgId, *(ERobotFunctions *)pdata);
        break;

    case CMD_ROB_FILL_CAP:          strcpy(_CmdName, "CMD_ROB_FILL_CAP");
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
            case rob_fct_cap:       _WasteValveSwitched = 0;
                                    _PrevWasteValveSelect = 0;
                                    RX_StepperStatus.robinfo.moving = TRUE;
                                    Fpga.par->output &= ~RO_ALL_OUTPUTS;
                                    Fpga.par->output |= RO_FLUSH_TO_CAP;
                                    Fpga.par->output |= RO_FLUSH_PUMP;
                                    _CapFillTime = rx_get_ticks();
                                    _FrontPumpUsed = TRUE;
                                    _PumpStartTime = rx_get_ticks();
                                    break;

            case rob_fct_purge_all: /*if (RX_StepperCfg.wipe_speed == 0)
                                    Error(ERR_CONT, 0, "Wipe-Speed is set 0, please chose another value");

                                    if (!RX_StepperStatus.info.z_in_ref  || RX_StepperStatus.info.z_in_up)//!RX_StepperStatus.info.headUpInput_0 || !RX_StepperStatus.info.headUpInput_1)
                                    {
                                        if (!RX_StepperStatus.info.moving)
                                        {
                                            _CmdRunning_Lift = CMD_LIFT_REFERENCE;
                                            lb702_handle_ctrl_msg(INVALID_SOCKET, _CmdRunning_Lift, NULL); _NewCmd = msgId;
                                            break;
                                        }
                                    }
                                    RX_StepperStatus.robinfo.moving = TRUE;
                                    _CmdRunning = msgId;
                                    if (RX_StepperCfg.wipe_speed)
                                        _ParCable_drive_purge.speed = RX_StepperCfg.wipe_speed * X_STEPS_PER_METER / 1000;			// divided by 1000 to get from steps/m to steps/mm
                                    else
                                        _ParCable_drive_purge.speed = 10 * X_STEPS_PER_METER / 1000;                                // divided by 1000 to get from steps/m to steps/mm 
                                    motors_move_to_step(MOTOR_X_BITS, &_ParCable_drive_purge, _micron_2_steps(CABLE_WIPE_POS_FRONT));*/
                _PumpStartTime = rx_get_ticks();
                _FrontPumpUsed = TRUE;
                break;


            case rob_fct_purge_head0:
            case rob_fct_purge_head1:
            case rob_fct_purge_head2:
            case rob_fct_purge_head3:
            case rob_fct_purge_head4:
            case rob_fct_purge_head5:
            case rob_fct_purge_head6:
            case rob_fct_purge_head7:
                break;

            default: break;
            }
        }

    case CMD_ERROR_RESET:           strcpy(_CmdName, "CMD_ERROR_RESET");
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
    int pos;
    if (!_CmdRunning)
    {
        _RobFunction = fct;
        if (!RX_StepperStatus.info.z_in_ref || RX_StepperStatus.info .z_in_up)      // Here this is for purging and not for vacuum
        {
            if (!RX_StepperStatus.info.moving)
            {
                _CmdRunning_Lift = CMD_LIFT_REFERENCE;
                lb702_handle_ctrl_msg(INVALID_SOCKET, _CmdRunning_Lift, NULL);
                _NewCmd = msgId;
            }
            return;
        }
        else if (!RX_StepperStatus.robinfo.ref_done || (_FrontPumpUsed && _Old_RobFunction != rob_fct_cap))
        {
            _lbrob_do_reference(0);
            // Error(ERR_CONT, 0, "LBROB: Robot not refenenced, cmd=0x%08x", msgId);
            _CmdRunning_old = msgId;
            return;
        }
        RX_StepperStatus.robinfo.moving = TRUE;
        _CmdRunning = msgId;
        switch (_RobFunction)
        {
        case rob_fct_cap:           motors_move_to_step(MOTOR_X_BITS, &_ParCable_drive, _micron_2_steps(CABLE_CAP_POS));
                                    break;

        case rob_fct_purge_all:     motors_move_to_step(MOTOR_X_BITS, &_ParCable_drive, _micron_2_steps(CABLE_CAP_POS));
                                    break;

        case rob_fct_purge_head0:
        case rob_fct_purge_head1:
        case rob_fct_purge_head2:
        case rob_fct_purge_head3:
        case rob_fct_purge_head4:
        case rob_fct_purge_head5:
        case rob_fct_purge_head6:
        case rob_fct_purge_head7:   // pos = (CABLE_WIPE_POS_BACK + (((int)_RobFunction - (int)rob_fct_purge_head0) * (CABLE_WIPE_POS_FRONT - CABLE_WIPE_POS_BACK)) / 7);
                                    // motors_move_to_step(MOTOR_X_BITS, &_ParCable_drive, _micron_2_steps(pos));
                                    break;

        default:                    Error(ERR_CONT, 0, "Command %s: Rob-Function %d not implemented", _CmdName, _RobFunction);
        }
    }
}

//--- _lbrob_motor_test ---------------------------------
static void _lbrob_motor_test(int motorNo, int steps, int Cmd)
{
    motorNo = 4;
    int motors = 1 << motorNo;
    SMovePar par;
    int i;

    memset(&par, 0, sizeof(SMovePar));

    par.speed = 16000;
    par.accel = 32000;
    par.current_acc = 400.0;
    par.current_run = 400.0;
    par.enc_bwd = TRUE;

    _CmdRunning = Cmd; // TEST
    RX_StepperStatus.info.moving = TRUE;

    motors_config(motors, CURRENT_HOLD, X_STEPS_PER_METER, X_INC_PER_METER, STEPS);
    motors_move_by_step(motors, &par, steps, FALSE);
}

//--- _lbrob_motor_enc_reg_test -----------------------------------------------
static void _lbrob_motor_enc_reg_test(int steps)
{
    RX_StepperStatus.robinfo.moving = TRUE;
    _CmdRunning = 1;

    motors_move_by_step(MOTOR_X_0, &_ParCable_drive, -steps, FALSE);
}