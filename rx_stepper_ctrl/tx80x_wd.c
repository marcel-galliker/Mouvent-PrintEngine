// ****************************************************************************
//
//	DIGITAL PRINTING - tx80x_wd.c
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
#include "rx_def.h"
#include "fpga_stepper.h"
#include "stepper_ctrl.h"
#include "motor.h"
#include "rx_error.h"
#include "rx_term.h"
#include "rx_trace.h"
#include "tx801.h"
#include "tx80x_wd.h"

// General
#define STEPS_REV           (200 * 16)      // steps per motor revolution * 16 microsteps
#define DIST_REV            1000.0          // moving distance per revolution on wrinkle detection motor [µm]

#define MOTOR_WD_FRONT      2               
#define MOTOR_WD_BACK       3               

#define MOTOR_WD_CNT        2               

#define MOTOR_WD_FRONT_BITS 0x04
#define MOTOR_WD_BACK_BITS  0x08
#define MOTOR_WD_BITS       0x0c

#define CURRENT_HOLD_WD     50.0            // 50 -> 0.5A

#define TX_REF_HEIGHT_WD    9000    //10200           // in um

// Inputs
#define WD_FRONT_STORED_IN  4
#define WD_BACK_STORED_IN   5
#define WRINKLE_DETECTED    6

// Functions
static void _tx80x_wd_move_to_pos(int cmd, int pos);
static int  _steps_2_micron(int steps);
static int  _micron_2_steps(int micron);
static void _tx80x_wd_motor_z_test(int steps);
static void _tx80x_wd_motor_test(int motorNo, int steps);

// global Variables
static int  _CmdRunning = 0;
static char _CmdName[32];
static int  _PrintPos_New = 0;              // in um
static int  _PrintPos_Act = 0;              // in um
static int  _WD_in_print = 0;
static int  _WrinkleDetected = 0;

// Motor movement Parameters
static SMovePar _ParRef;
static SMovePar _ParDrive;

// Times and Parameters
static int _TimeWrinkleDetected = 2000; // in ms

//--- tx80x_wd_init -------------------------------
void tx80x_wd_init(void)
{
    motors_config(MOTOR_WD_BITS, CURRENT_HOLD_WD, L3518_STEPS_PER_METER, L3518_INC_PER_METER, STEPS);

    _ParRef.speed = 10000;
    _ParRef.accel = 32000;
    _ParRef.current_acc = 150;
    _ParRef.current_run = 100;
    _ParRef.estop_in_bit[0] = (1 << WD_FRONT_STORED_IN);
    _ParRef.estop_in_bit[1] = (1 << WD_BACK_STORED_IN);
    _ParRef.estop_level = 1;
    _ParRef.stop_mux = MOTOR_WD_BITS;
    _ParRef.dis_mux_in = TRUE;
    _ParRef.encCheck = chk_std;

    _ParDrive.speed = 10000;
    _ParDrive.accel = 32000;
    _ParDrive.current_acc = 200.0;
    _ParDrive.current_run = 100.0;
    _ParDrive.stop_mux = MOTOR_WD_BITS;
    _ParDrive.dis_mux_in = FALSE;
    _ParDrive.encCheck = chk_std;
}

//--- tx80x_wd_main ----------------------------------------------
void tx80x_wd_main(void)
{
    int motor, ok;

    if (rx_get_ticks() >= _WrinkleDetected + _TimeWrinkleDetected)
        _WrinkleDetected = 0;

    // read Inputs
    RX_StepperStatus.robinfo.wrinkle_detected =
        fpga_input(WRINKLE_DETECTED) ||
        _WrinkleDetected; // || _WrinkleDetected;
    if (RX_StepperStatus.robinfo.wrinkle_detected && !_WrinkleDetected)
    {
        ctrl_send_2(REP_TT_STATUS, sizeof(RX_StepperStatus), &RX_StepperStatus);
        _WrinkleDetected = rx_get_ticks();
    }

    if (RX_StepperStatus.robinfo.moving_wd)
    {
        _WD_in_print = FALSE;
        _WrinkleDetected = 0;
    }

    RX_StepperStatus.robinfo.wd_front_up = fpga_input(WD_FRONT_STORED_IN);
    RX_StepperStatus.robinfo.wd_back_up = fpga_input(WD_BACK_STORED_IN);
    if (_CmdRunning && motors_move_done(MOTOR_WD_BITS))
    {
        RX_StepperStatus.robinfo.moving_wd = FALSE;
        if (_CmdRunning == CMD_CAP_REFERENCE)
        {
            for (motor = MOTOR_WD_FRONT, ok = TRUE; motor < MOTOR_WD_CNT; motor++)
            {
                if ((Fpga.stat->statMot[motor].err_estop & ENC_ESTOP_ENC))
                {
                    if (!fpga_input(motor))
                    {
                        Error(ERR_ABORT, 0, "WRINKLE DETECTION: %s: motor %s blocked", _CmdName, motor + 1);
                        ok = FALSE;
                    }
                }
            }
            motors_reset(MOTOR_WD_BITS);
            RX_StepperStatus.robinfo.ref_done_wd = ok && RX_StepperStatus.robinfo.wd_front_up && RX_StepperStatus.robinfo.wd_back_up;
        }
        else
        {
            for (motor = MOTOR_WD_FRONT, ok = TRUE;
                 motor < MOTOR_WD_CNT; motor++)
            {
                if (motor_error(motor))
                {
                    Error(ERR_ABORT, 0,
                          "WRINKLE DETECTION: %s: motor %s blocked", _CmdName,
                          motor + 1);
                    ok = FALSE;
                }
            }
            if (!ok) RX_StepperStatus.robinfo.ref_done_wd = FALSE;
        }

        _WD_in_print = (_CmdRunning == CMD_CAP_PRINT_POS && RX_StepperStatus.info.ref_done);

        if (_CmdRunning == CMD_CAP_REFERENCE && _PrintPos_New)
        {
            _tx80x_wd_move_to_pos(CMD_CAP_PRINT_POS, _PrintPos_New);
            _PrintPos_Act = _PrintPos_New;
            _PrintPos_New = 0;
        }
        else
            _CmdRunning = FALSE;
    }
}

//--- tx80x_wd_handle_menu ---------------------------------------
void tx80x_wd_handle_menu(char *str)
{
    int i, pos;

    if (str)
    {
        switch (str[0])
        {
        case 's':   tx80x_wd_handle_ctrl_msg(INVALID_SOCKET, CMD_CAP_STOP, NULL); break;
        case 'r':   if (str[1] == 0)    for (i = 0; i < MOTOR_CNT; i++) motor_reset(i);
                    else                motor_reset(atoi(&str[1]));
                    break;
        case 'o':   Fpga.par->output ^= (1 << atoi(&str[1])); break;
        case 'R':   tx80x_wd_handle_ctrl_msg(INVALID_SOCKET, CMD_CAP_REFERENCE, NULL); break;
        case 'p':   pos = atoi(&str[1]);
                    tx80x_wd_handle_ctrl_msg(INVALID_SOCKET, CMD_CAP_PRINT_POS, &pos);
                    break;
        case 'u':   tx80x_wd_handle_ctrl_msg(INVALID_SOCKET, CMD_CAP_UP_POS, NULL); break;
        case 'a':   tx80x_wd_handle_ctrl_msg(INVALID_SOCKET, CMD_CAP_CALIBRATE, NULL); break;
        case 'z':   _tx80x_wd_motor_z_test(atoi(&str[1])); break;
        case 'm':   _tx80x_wd_motor_test(str[1] - '0', atoi(&str[2])); break;
        }
     
    }
    
   
}

//--- tx80x_wd_menu ----------------------------------------------
void tx80x_wd_menu(int help)
{
    if (help)
    {
        term_printf("s: STOP\n");
        term_printf("r<n>: reset motor<n>\n");
        term_printf("o: toggle output <no>\n");
        term_printf("R: Reference\n");
        term_printf("p<um>: set hight for Wrinkle Detection\n");
        term_printf("u: move Wrinkle Detection to up position\n");
        term_printf("a: move down to adjust wrinkle detection\n");
        term_printf("z<steps>: move WD by <steps>\n");
        term_printf("m<n><steps>: move Motor<n> by <steps>\n");
        term_printf("x: exit\n");
        
    }
    else
    {
        term_printf("?: help\n");
        term_printf("1: ROBOTER menu\n");
    }
}

//--- tx80x_wd_display_status -------------------------------------------------------------
void tx80x_wd_display_status(void)
{
    term_printf("Sensor WD-Front:       %d\n", RX_StepperStatus.robinfo.wd_front_up);
    term_printf("Sensor WD-Back:        %d\n", RX_StepperStatus.robinfo.wd_back_up);
    term_printf("Reference WD done:     %d\n", RX_StepperStatus.robinfo.ref_done_wd);
    term_printf("WD-Motor Front pos:    %d\n", TX_REF_HEIGHT_WD - _steps_2_micron(motor_get_step(MOTOR_WD_FRONT)));
    term_printf("WD-Motor Back pos:     %d\n", TX_REF_HEIGHT_WD - _steps_2_micron(motor_get_step(MOTOR_WD_BACK)));
    term_printf("Wrinkle Detected:      %d\n", RX_StepperStatus.robinfo.wrinkle_detected);
    term_printf("\n");
}

//--- _tx80x_wd_do_reference ----------------------------------------------------------------
static void _tx80x_wd_do_reference(void)
{
    motors_stop(MOTOR_WD_BITS);

    _CmdRunning = CMD_CAP_REFERENCE;
    RX_StepperStatus.robinfo.moving_wd = TRUE;
    motors_move_by_step(MOTOR_WD_BITS, &_ParRef, -100000, TRUE);
}

//--- tx80x_wd_handle_ctrl_msg -----------------------------------
int tx80x_wd_handle_ctrl_msg(RX_SOCKET socket, int msgId, void *pdata)
{
    INT32 pos, steps;
    switch (msgId)
    {
    case CMD_CAP_STOP:
        TrPrintfL(TRUE, "SOCKET[%d]: %s", socket, _CmdName);
        motors_stop(MOTOR_WD_BITS);
        break;

    case CMD_CAP_REFERENCE:
        _PrintPos_New = 0;
        strcpy(_CmdName, "CMD_CAP_REFERENCE");
        TrPrintfL(TRUE, "SOCKET[%d]: %s", socket, _CmdName);
        motors_reset(MOTOR_WD_BITS);
        RX_StepperStatus.robinfo.ref_done_wd = FALSE;
        _tx80x_wd_do_reference();
        break;

    case CMD_CAP_PRINT_POS:
        strcpy(_CmdName, "CMD_CAP_PRINT_POS");
        TrPrintfL(TRUE, "SOCKET[%d]: %s", socket, _CmdName);
        pos = (*((INT32 *)pdata));
        if (pos < TX_PRINT_POS_MIN)
        {
            pos = TX_PRINT_POS_MIN;
            Error(WARN, 0, "CMD_CAP_PRINT_POS of Wrinkle Detection set to MIN pos=%d.%03d mm", pos / 1000, pos % 1000);
        }
        else if (pos > TX_REF_HEIGHT_WD)
        {
            pos = TX_REF_HEIGHT_WD;
            Error(WARN, 0, "CMD_CAP_PRINT_POS of Wrinkle Detection set to MAX pos=%d.%03d mm", pos / 1000, pos % 1000);
        }
        if (!_CmdRunning && (!_WD_in_print || (steps != _PrintPos_Act)))
        {
            _PrintPos_New = _micron_2_steps(TX_REF_HEIGHT_WD + 500 - pos);
            if (RX_StepperStatus.robinfo.ref_done_wd)
                _tx80x_wd_move_to_pos(CMD_CAP_PRINT_POS, _PrintPos_New);
            else
                _tx80x_wd_do_reference();
        }
        break;

    case CMD_CAP_UP_POS:
    case CMD_CAP_CAPPING_POS:
    case CMD_CAP_WASH_POS:
    case CMD_CAP_WIPE_POS:
    case CMD_CAP_VACUUM_POS:
        strcpy(_CmdName, "CMD_CAP_UP_POS");
        TrPrintfL(TRUE, "SOCKET[%d]: %s", socket, _CmdName);
        if (!_CmdRunning)
        {
            _PrintPos_New = _micron_2_steps(500);
            if (RX_StepperStatus.robinfo.ref_done_wd)
                _tx80x_wd_move_to_pos(CMD_CAP_UP_POS, _PrintPos_New);
            else
                _tx80x_wd_do_reference();
        }
        break;
        
    case CMD_CAP_CALIBRATE:
        strcpy(_CmdName, "CMD_CAP_CALIBRATE");
        TrPrintfL(TRUE, "SOCKET[%d]: %s", socket, _CmdName);
        if (!_CmdRunning)
        {
            motors_reset(MOTOR_WD_BITS);
            RX_StepperStatus.robinfo.ref_done_wd = FALSE;
            motors_stop(MOTOR_WD_BITS);
            _CmdRunning = CMD_CAP_CALIBRATE;
            RX_StepperStatus.robinfo.moving_wd = TRUE;
            motors_move_by_step(MOTOR_WD_BITS, &_ParRef, 100000, TRUE);
        }
        break;

    case CMD_ERROR_RESET:
        strcpy(_CmdName, "CMD_ERROR_RESET");
        TrPrintfL(TRUE, "SOCKET[%d]: %s", socket, _CmdName);
        fpga_stepper_error_reset();
        break;

    case CMD_TT_STATUS:
    case CMD_CLN_STOP:
    case CMD_CLN_REFERENCE:
    case CMD_CLN_SHIFT_REF:
    case CMD_CLN_ROT_REF:
    case CMD_CLN_ROT_REF2:
    case CMD_CLN_MOVE_POS:
    case CMD_CLN_SHIFT_MOV:
    case CMD_CLN_SHIFT_LEFT:
    case CMD_CLN_FILL_CAP:
    case CMD_CLN_WAIT:
    case CMD_CLN_TILT_CAP:
    case CMD_CLN_EMPTY_WASTE:
    case CMD_CLN_VACUUM: break;

    default:
        Error(ERR_CONT, 0, "CLN: Command 0x%08x not implemented", msgId);
        break;
    }
    return TRUE;

}

//--- _tx80x_wd_move_to_pos -----------------------------------------------------------------
static void _tx80x_wd_move_to_pos(int cmd, int pos)
{
    _CmdRunning = cmd;
    RX_StepperStatus.robinfo.moving_wd = TRUE;
    motors_move_to_step(MOTOR_WD_BITS, &_ParDrive, pos);
}

//---_steps_2_micron -------------------------------------------------------------
static int _steps_2_micron(int steps)
{
    return (int)(DIST_REV * steps / STEPS_REV + 0.5);
}

//---_micron_2_steps --------------------------------------------------------------
static int _micron_2_steps(int micron)
{
    return (int)(0.5 + STEPS_REV / DIST_REV * micron);
}

//--- _tx80x_wd_motor_z_test ----------------------------------------------------------------------
static void _tx80x_wd_motor_z_test(int steps)
{
    _CmdRunning = 1; // TEST
    RX_StepperStatus.robinfo.moving_wd = TRUE;
    motors_move_by_step(MOTOR_WD_BITS, &_ParDrive, steps, TRUE);
}

//--- _txrob_motor_test ---------------------------------
static void _tx80x_wd_motor_test(int motorNo, int steps)
{
    int motors = 1 << motorNo;
    SMovePar par;
    int i;

    memset(&par, 0, sizeof(par));
    par.stop_mux = 0;
    par.dis_mux_in = 0;
    par.encCheck = chk_off;
    RX_StepperStatus.robinfo.moving_wd = TRUE;
    _CmdRunning = 1;

    if (motorNo == 0)
    {
        // paramaters tested 14-JAN-20
        par.speed = 1000; // speed with max tork: 21'333
        par.accel = 10000;
        par.current_acc = 400.0; //  max 67 = 0.67 A
        par.current_run = 300.0; //  max 67 = 0.67 A
        par.stop_mux = 0;
        par.dis_mux_in = 0;
        par.encCheck = chk_std;
        motors_config(motors, 0, L3518_STEPS_PER_METER, L3518_INC_PER_METER, MICROSTEPS);
        motors_move_by_step(motors, &par, steps, FALSE);
    }
    else if (motorNo == 1)
    {
        // paramaters tested 14-JAN-20

        par.speed = 21000; // speed with max tork: 21'333
        par.accel = 10000;
        par.current_acc = 40.0; //  max 67 = 0.67 A
        par.current_run = 40.0; //  max 67 = 0.67 A
        par.stop_mux = 0;
        par.dis_mux_in = 0;
        par.encCheck = chk_off;
        motors_config(motors, 7.0, 0.0, 0.0, STEPS);
        motors_move_by_step(motors, &par, steps, TRUE);
    }
    else if (motorNo == MOTOR_WD_BACK || motorNo == MOTOR_WD_FRONT)
    {
        // paramaters tested 14-JAN-20
        par.speed = 10000; // speed with max tork: 21'333
        par.accel = 32000;
        par.current_acc = 400.0;
        par.current_run = 300.0;
        par.stop_mux = 0;
        par.dis_mux_in = 0;
        par.encCheck = chk_std;
        motors_config(motors, CURRENT_HOLD_WD, L3518_STEPS_PER_METER, L3518_INC_PER_METER, STEPS);
        motors_move_by_step(motors, &par, steps, FALSE);
    }
    else
    {
        par.speed = 1000;
        par.accel = 1000;
        par.current_acc = 250.0;
        par.current_run = 250.0;
        par.stop_mux = 0;
        par.dis_mux_in = 0;
        par.encCheck = chk_off;
        motors_config(motors, 50, L3518_STEPS_PER_METER, L3518_INC_PER_METER, STEPS);
        motors_move_by_step(motors, &par, steps, FALSE); 
    }
}
