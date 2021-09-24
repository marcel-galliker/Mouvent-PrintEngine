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
#include "rx_threads.h"
#include "robi_def.h"
#include "lb702.h"
#include "robi_lb702.h"
#include "robot_client.h"

#define MOTOR_SLIDE               4

#define MOTOR_SLIDE_BITS        (0x01<<MOTOR_SLIDE)
#define MOTOR_ALL_BITS          0x13

#define SLIDE_STEPS_PER_REV     3200.0
#define SLIDE_INC_PER_REV       16000.0
#define SLIDE_DIST_PER_REV      54000   // 36000

#define CURRENT_HOLD            200

#ifdef DEBUG
    #define CAP_FILL_TIME           2000       // ms
#else
    #define CAP_FILL_TIME           16000       // ms
#endif

#define VACUUM_PUMP_TIME        5000        // ms
#define WASTE_PUMP_TIME         (VACUUM_PUMP_TIME+30000)  // ms

// Digital Inputs
#define SLIDE_REF               2
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

// Times
#define FOLLOW_UP_TIME_INK_BACK 20000       // ms


// globals
static int _CmdRunning_Lift = 0;
static int _CmdRunning_Robi = 0;

// static
static SMovePar _ParSlide_ref;
static SMovePar _ParSlide_drive;
static SMovePar _ParSlide_drive_slow;
static SMovePar _ParSlide_drive_purge;

static char     *_MotorName[5] = {"BACK", "FRONT", "M2", "M3", "SLIDE"};
static char     _CmdName[32];
static int      _CmdRunning = 0;
static int      _Step = 0;
static int      _NewCmd = 0;

static SClnMovePar _ClnMovePar;
static ERobotFunctions _Old_RobFunction = 0;

// Timers
static int _CapFillTime = 0;
static int _WastePumpTimer = 0;
static int _VacuumTimer = 0;

static int _PumpValueLeft = 0;
static int _PumpValueRight = 0;
static int _PumpTimeLeft = 0;
static int _PumpTimeRight = 0;

static int _PumpSpeed = 100;
static int  _CapIsWet = TRUE;
static UINT32 _RO_Flush = 0x0;

//--- prototypes --------------------------------------------
static void _lbrob_motor_z_test(int steps);
static void _lbrob_motor_test(int motor, int steps);
static int  _micron_2_steps(int micron);
static int  _steps_2_micron(int steps);
static void _cln_move_to(int msgId, ERobotFunctions fct, int position);
static void _handle_flush_pump(void);
static void _handle_ink_pump_back(void);
static void _set_ink_back_pump_left(int voltage);
static void _set_ink_back_pump_right(int voltage);
static void _vacuum_on();
static void _pump_main();

static int  _lbrob_start_sm(ERobotFunctions fct, int pos);
static void _lbrob_sm(void);    // state machine 

//--- lbrob_init --------------------------------------
void lbrob_init(int robotUsed)
{
    /*
#ifdef DEBUG
    _CapIsWet = FALSE;
#endif
*/
    memset(_CmdName, 0, sizeof(_CmdName));
    memset(&_ParSlide_ref, 0, sizeof(SMovePar));
    memset(&_ParSlide_drive, 0, sizeof(SMovePar));
    memset(&_ParSlide_drive_purge, 0, sizeof(SMovePar));
    
    // config for referencing slide motor (motor 4)
    _ParSlide_ref.speed = 1000;
    _ParSlide_ref.accel = 4000;
    _ParSlide_ref.current_acc = 350.0;
    _ParSlide_ref.current_run = 350.0;
    _ParSlide_ref.stop_mux = 0;
    _ParSlide_ref.dis_mux_in = 0;
    _ParSlide_ref.estop_level = TRUE;
    _ParSlide_ref.estop_in_bit[MOTOR_SLIDE] = (1 << SLIDE_REF);
    _ParSlide_ref.enc_bwd = TRUE;
    _ParSlide_ref.encCheck = chk_std;

    // config for moving normal with slide motor (motor 4)
    // This commands that use this config need to start the motor with the
    // special encoder mode
    _ParSlide_drive.speed = 10000;
    _ParSlide_drive.accel = 8000;
    _ParSlide_drive.current_acc = 400.0;        // max 420      --> with 400 it's possible, that the motor turns, but jumps over the belt
    _ParSlide_drive.current_run = 400.0;        // max 420      --> with 400 it's possible, that the motor turns, but jumps over the belt
    _ParSlide_drive.stop_mux = 0;
    _ParSlide_drive.dis_mux_in = 0;
    _ParSlide_drive.estop_level = 0;
    _ParSlide_drive.enc_bwd = TRUE;
    _ParSlide_drive.encCheck = chk_std;

    _ParSlide_drive_slow.speed = 1000;
    _ParSlide_drive_slow.accel = 8000;
    _ParSlide_drive_slow.current_acc = 400.0;   // max 420      --> with 400 it's possible, that the motor turns, but jumps over the belt
    _ParSlide_drive_slow.current_run = 400.0;   // max 420      --> with 400 it's possible, that the motor turns, but jumps over the belt
    _ParSlide_drive_slow.stop_mux = 0;
    _ParSlide_drive_slow.dis_mux_in = 0;
    _ParSlide_drive_slow.estop_level = 0;
    _ParSlide_drive_slow.enc_bwd = TRUE;
    _ParSlide_drive_slow.encCheck = chk_std;

    _ParSlide_drive_purge.speed = _micron_2_steps(1000 * 10); // multiplied with 1000 to get from mm/s to um/s
    _ParSlide_drive_purge.accel = 4000;
    _ParSlide_drive_purge.current_acc = 400.0; // max 420       --> with 400 it's possible, that the motor turns, but jumps over the belt
    _ParSlide_drive_purge.current_run = 400.0; // max 420       --> with 400 it's possible, that the motor turns, but jumps over the belt
    _ParSlide_drive_purge.stop_mux = 0;
    _ParSlide_drive_purge.dis_mux_in = 0;
    _ParSlide_drive_purge.estop_level = 0;
    _ParSlide_drive_purge.enc_bwd = TRUE;
    _ParSlide_drive_purge.encCheck = chk_std;

    motor_config(MOTOR_SLIDE, CURRENT_HOLD, SLIDE_STEPS_PER_REV, SLIDE_INC_PER_REV, STEPS);

    rc_init(robotUsed);
    rc_config(RX_StepperCfg.boardNo);
}

//--- lbrob_main ------------------------------------------------------------------
void lbrob_main(int ticks, int menu)
{
    int motor;
    int pos, val;
    static int _x_in_ref_old;
    static int _startTime=0;

    RX_StepperStatus.posY[0] = _steps_2_micron(motor_get_step(MOTOR_SLIDE));
    RX_StepperStatus.posY[1] = _steps_2_micron(motor_get_step(MOTOR_SLIDE)) - SLIDE_PURGE_POS_BACK;
    RX_StepperStatus.info.x_in_ref = fpga_input(SLIDE_REF);
    RX_StepperStatus.info.x_in_cap = fpga_input(CAPPING_ENDSTOP);

    SStepperStat oldSatus;
    memcpy(&oldSatus, &RX_StepperStatus, sizeof(RX_StepperStatus));

    RX_StepperStatus.info.vacuum_running = (Fpga.par->output & RO_VACUUM_CLEANER) >> 4;

    motor_main(ticks, menu);
    rc_main(ticks, menu);

    _handle_ink_pump_back();
    _handle_flush_pump();

    if (_VacuumTimer && rx_get_ticks() >= _VacuumTimer)
    {
        Fpga.par->output &= ~RO_VACUUM_CLEANER;
        _VacuumTimer = 0;
    }
    if (_WastePumpTimer && rx_get_ticks() >= _WastePumpTimer)
    {
        Fpga.par->output &= ~RO_WASTE_VAC;
        _WastePumpTimer = 0;
    }

    RX_StepperStatus.robinfo.moving = (_CmdRunning != 0);
    if (RX_StepperStatus.robinfo.moving)
    {
        if (!_startTime) _startTime = rx_get_ticks();
        RX_StepperStatus.robinfo.cap_ready      = FALSE;
        RX_StepperStatus.robinfo.rob_in_cap     = FALSE;
        RX_StepperStatus.robinfo.purge_ready    = FALSE;
        RX_StepperStatus.robinfo.wash_done      = FALSE;
        RX_StepperStatus.robinfo.vacuum_done    = FALSE;
        RX_StepperStatus.robinfo.wipe_done      = FALSE;
        RX_StepperStatus.robinfo.x_in_purge4ever= FALSE;
        if (_CapIsWet && _CmdRunning==CMD_ROB_REFERENCE && !fpga_input(SLIDE_REF) && rx_get_ticks()-_startTime>2000)
        {
            _vacuum_on();
        }
    }
    else _startTime=0;

    if (RX_StepperStatus.info.moving)
    {
        RX_StepperStatus.robinfo.cap_ready = FALSE;
    }
    else _x_in_ref_old=RX_StepperStatus.info.x_in_ref;

    if (!RX_StepperStatus.robinfo.ref_done && !fpga_input(SLIDE_REF)) 
        _CapIsWet = TRUE;

    if (_CmdRunning && motors_move_done(MOTOR_SLIDE_BITS))
    {
        if (_CmdRunning == CMD_ROB_REFERENCE && _CapIsWet)
            _CapIsWet = FALSE;
        _x_in_ref_old=RX_StepperStatus.info.x_in_ref;
        RX_StepperStatus.robinfo.moving = FALSE;
    
        if (_CmdRunning == CMD_ROB_REFERENCE)
        {
            if (!RX_StepperStatus.robinfo.ref_done)
            {
                if (motors_error(MOTOR_SLIDE_BITS, &motor))
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
                    motors_reset(MOTOR_SLIDE_BITS);
                    RX_StepperStatus.robinfo.ref_done = TRUE;
                    TrPrintfL(TRUE, "LBROB: Command CMD_ROB_REFERENCE: done");
                    Error(LOG, 0, "LBROB: Command CMD_ROB_REFERENCE: done");
                }
            }
            else
            {
                if (motors_error(MOTOR_SLIDE_BITS, &motor))
                {
                    Error(ERR_CONT, 0, "LIFT: Command %s - 1000 steps: Motor %s blocked",_CmdName, _MotorName[motor]);
                    RX_StepperStatus.robinfo.ref_done = FALSE;
                }
                else 
                {                
                    int time=0;
                    TrPrintfL(TRUE, "LBROB: Command CMD_ROB_REFERENCE: Checking SLIDE_REF, slidePos=%d, x_in_ref=%d, time=%d", RX_StepperStatus.posY[0], RX_StepperStatus.info.x_in_ref, time);
                    for (time=0; time<500; time+=10)
                    {
                        if ( RX_StepperStatus.info.x_in_ref=fpga_input(SLIDE_REF)) break;
                        rx_sleep(10);
                    }
                    TrPrintfL(TRUE, "LBROB: Command CMD_ROB_REFERENCE: Checking SLIDE_REF, slidePos=%d, x_in_ref=%d, time=%d", RX_StepperStatus.posY[0], RX_StepperStatus.info.x_in_ref, time);
                    if (!RX_StepperStatus.info.x_in_ref)
                    {
                        Error(ERR_CONT, 0, "LBROB: Command %s: End Sensor REF NOT HIGH", _CmdName, fpga_input(SLIDE_REF));
                        RX_StepperStatus.robinfo.ref_done = FALSE;
                    }
                }
                _CmdRunning = FALSE;
            }
        }

        if (_CmdRunning != CMD_ROB_REFERENCE)
        {
            if (motors_error(MOTOR_SLIDE_BITS, &motor))
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
            switch (_ClnMovePar.function)
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

            case rob_fct_move_purge:
                _CapIsWet = TRUE;
                _CmdRunning = FALSE;
                RX_StepperStatus.robinfo.cap_ready = TRUE;
                break;

            case rob_fct_move_purge_end:
                _CapIsWet = TRUE;
                _CmdRunning = FALSE;
                RX_StepperStatus.robinfo.cap_ready = TRUE;
                break;

            default:
                Error(ERR_CONT, 0, "Command %s: Robi-Function %d not implemented", _CmdName, _ClnMovePar.function);
                break;
            }
            _Old_RobFunction = _ClnMovePar.function;
        }
        else if (_CmdRunning == CMD_ROB_MOVE_POS)
        {
            switch (_ClnMovePar.function)
            {
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

            case rob_fct_move_to_pos:
                _CmdRunning = 0;
                break;

            case rob_fct_cap:           break;  // handled in _lbrob_sm();
            case rob_fct_move_purge:    break;
            case rob_fct_move_purge_end:break;

            case rob_fct_vacuum:
            case rob_fct_wash:
                _CapIsWet=TRUE;
                _Old_RobFunction = _ClnMovePar.function;
                _ClnMovePar.function = rob_fct_move;
                _CmdRunning = 0;
                lbrob_handle_ctrl_msg(INVALID_SOCKET, CMD_ROB_MOVE_POS, &_ClnMovePar);
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
                _Old_RobFunction = _ClnMovePar.function;
                break;

            default:
                Error(ERR_CONT, 0, "LBROB_MAIN: Rob-Function %d not implemented", _ClnMovePar.function);
                _CmdRunning = 0;
                RX_StepperStatus.robinfo.ref_done = FALSE;
                break;
            }
            if (!_CmdRunning_Lift && !(RX_StepperStatus.info.z_in_wash && _Old_RobFunction == rob_fct_wash) && _ClnMovePar.function != rob_fct_move)
                _Old_RobFunction = _ClnMovePar.function;
        }
        else
        {
            _CmdRunning = FALSE;
        }
    }

    if (_CmdRunning_Lift || _CmdRunning_Robi)
    {
        int loc_new_cmd = 0;
        ERobotFunctions robfunction = 0;
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
            robfunction = _ClnMovePar.function;
        }
        if (loc_new_cmd)
        {
            switch (loc_new_cmd)
            {
            case CMD_ROB_REFERENCE:
                lbrob_handle_ctrl_msg(INVALID_SOCKET, CMD_ROB_REFERENCE, NULL);
                break;
            case CMD_ROB_MOVE_POS:
                lbrob_handle_ctrl_msg(INVALID_SOCKET, CMD_ROB_MOVE_POS, &_ClnMovePar);
                break;
            case CMD_ROB_FILL_CAP:
                lbrob_handle_ctrl_msg(INVALID_SOCKET, CMD_ROB_MOVE_POS, &robfunction);
				break;
            default:
                Error(ERR_CONT, 0, "LBROB_MAIN: Command 0x%08x not implemented", loc_new_cmd);
                break;
            }
        }
        loc_new_cmd = FALSE;
    }

    static int j = 0;

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
   
    _pump_main();
    _lbrob_sm();
}

//--- _lbrob_start_sm -------------------------------------
static int _lbrob_start_sm(ERobotFunctions fct, int pos)
{    
    if (_ClnMovePar.function==0)
    {
        RX_StepperStatus.robinfo.purge_ready = FALSE;
        switch(fct)
        {
        case rob_fct_move_purge:    _ClnMovePar.function = fct;
                                    _ClnMovePar.position = (SLIDE_PURGE_POS_BACK + (pos* (SLIDE_PURGE_POS_FRONT - SLIDE_PURGE_POS_BACK)) / 7); 
                                    TrPrintfL(TRUE, "_lbrob_start_sm rob_fct_move_purge: head=%d, pos=%d", pos, _ClnMovePar.position);
                                    break;

        case rob_fct_move_purge_end:_ClnMovePar.function = fct;
                                    _ClnMovePar.position = pos;
                                    TrPrintfL(TRUE, "_lbrob_start_sm rob_fct_move_purge_end: head=%d, pos=%d", pos, _ClnMovePar.position);
                                    break;

        case rob_fct_cap:           if (!RX_StepperStatus.robinfo.rob_in_cap)
                                    {
                                        _ClnMovePar.function = fct; 
                                        _ClnMovePar.position = SLIDE_CAP_POS;
                                    }
                                    break;

        default:                    _Step=0;
                                    return FALSE;
        }
        _Step=1;
        _lbrob_sm();
        return TRUE;
    }
    return FALSE;
}

//--- _lbrob_sm ----------------------------------------------
static void _lbrob_sm(void)
{
    static int time=0;
    int trace=FALSE;
    if (_Step && rx_get_ticks()>time)
    {
        time=rx_get_ticks()+1000;
        trace=TRUE;
    }
    switch(_Step)
    {
    case 0: break;

    case 1: if (!RX_StepperStatus.screwerinfo.z_in_down)
            {
                rc_move_bottom(_FL_);
            }
            _Step++;
            break;

    case 2: if (!RX_StepperStatus.screwer_used || RX_StepperStatus.screwerinfo.z_in_down)
            {
                TrPrintfL(TRUE, "_lbrob_sm[%s/%d]", RobFunctionStr[_ClnMovePar.function], _Step);
                _CmdRunning = CMD_ROB_MOVE_POS;
                if (RX_StepperStatus.info.z_in_ref)
                    _Step++;
                else if (!RX_StepperStatus.info.moving) 
                {
                    lb702_do_reference();
                    _Step++;
                }
            }
			break;

    case 3: TrPrintfL(trace, "_lbrob_sm[%s/%d]: z_in_ref=%d", RobFunctionStr[_ClnMovePar.function], _Step, RX_StepperStatus.info.z_in_ref);
            if (RX_StepperStatus.info.z_in_ref)
            {
                TrPrintfL(TRUE, "_lbrob_sm[%s/%d]", RobFunctionStr[_ClnMovePar.function], _Step);
                _Step++;
                if (!RX_StepperStatus.robinfo.ref_done && fpga_input(SLIDE_REF)) 
                {
                    _CmdRunning = CMD_ROB_REFERENCE;
                    motors_move_by_step(MOTOR_SLIDE_BITS, &_ParSlide_drive, _micron_2_steps(-5000), TRUE);
                }
            }
            break;

    case 4: TrPrintfL(trace, "_lbrob_sm[%s/%d]: ref_done=%d, moveDone=%d", RobFunctionStr[_ClnMovePar.function], _Step, RX_StepperStatus.robinfo.ref_done, motors_move_done(MOTOR_SLIDE_BITS));
            if (RX_StepperStatus.robinfo.ref_done)
                _Step++;
            else if (motors_move_done(MOTOR_SLIDE_BITS))
            {
                TrPrintfL(TRUE, "_lbrob_sm[%s/%d]", RobFunctionStr[_ClnMovePar.function], _Step);
                _Step++;
                lbrob_reference_slide();
            }
            break;
            
    case 5: TrPrintfL(trace, "_lbrob_sm[%s/%d]: ref_done=%d, z_in_ref=%d, rob.ref_done=%d, moveDone=%d", RobFunctionStr[_ClnMovePar.function], _Step, RX_StepperStatus.robinfo.ref_done, RX_StepperStatus.info.z_in_ref, RX_StepperStatus.robinfo.ref_done, motors_move_done(MOTOR_SLIDE_BITS));
            if (RX_StepperStatus.info.ref_done && RX_StepperStatus.info.z_in_ref && RX_StepperStatus.robinfo.ref_done && motors_move_done(MOTOR_SLIDE_BITS))
            {
                TrPrintfL(TRUE, "_lbrob_sm[%s/%d]", RobFunctionStr[_ClnMovePar.function], _Step);
                lbrob_move_to_pos(0, _micron_2_steps(_ClnMovePar.position), _ClnMovePar.function==rob_fct_move_purge_end);
                _Step++;
            }
            break;

    case 6: if (motors_move_done(MOTOR_SLIDE_BITS)) 
            {
                TrPrintfL(TRUE, "_lbrob_sm[%s/%d] moveDone=%d", RobFunctionStr[_ClnMovePar.function], _Step, motors_move_done(MOTOR_SLIDE_BITS));
                Error(LOG, 0, "SlideMove fct=%s done",  RobFunctionStr[_ClnMovePar.function]);
                switch(_ClnMovePar.function)
                {
                case rob_fct_move_purge: _CapIsWet=TRUE;
                                         RX_StepperStatus.robinfo.purge_ready = TRUE;   
                                         break;
                
                case rob_fct_move_purge_end:
                                         _CapIsWet=TRUE;
                                         RX_StepperStatus.robinfo.purge_ready = TRUE;   
                                         break;

                case rob_fct_cap:       RX_StepperStatus.robinfo.rob_in_cap = fpga_input(CAPPING_ENDSTOP);
                                        if (!RX_StepperStatus.robinfo.rob_in_cap)
                                        {
                                            Error(ERR_CONT, 0, "LBROB: Command %s: End Sensor Capping NOT HIGH", _CmdName);
                                            RX_StepperStatus.robinfo.ref_done = FALSE;
                                        } 
                                        break;
                default: break;
                }
                
                _CmdRunning = 0;
                _Step=0;
                _ClnMovePar.function=0;
            }
            break;            
    }
}

//---_micron_2_steps --------------------------------------------------------------
static int _micron_2_steps(int micron)
{
    return (int)(0.5 + SLIDE_STEPS_PER_REV / SLIDE_DIST_PER_REV * micron);
}

//--- _steps_2_micron -------------------------------------------------------------
static int _steps_2_micron(int steps)
{
    return (int)(0.5 + (double)steps / SLIDE_STEPS_PER_REV * SLIDE_DIST_PER_REV);
}

//--- _lbrob_display_status --------------------------------------------------------
void lbrob_display_status(void)
{
    term_printf("LB Clean ---------------------------------\n");
    if (RX_StepperStatus.cln_used)
    {
        term_printf("moving: \t\t %d \t cmd: %08x\n", RX_StepperStatus.robinfo.moving, _CmdRunning);
        term_printf("reference done: \t %d\n", RX_StepperStatus.robinfo.ref_done);
        term_printf("slide in ref: \t\t %d\n", RX_StepperStatus.info.x_in_ref);
        term_printf("slide in cap: \t\t %d\n", RX_StepperStatus.info.x_in_cap);
 //       term_printf("Cap ready \t\t %d\n", RX_StepperStatus.robinfo.cap_ready);
        term_printf("Purge ready: \t\t %d\n", RX_StepperStatus.robinfo.purge_ready);
        term_printf("Slide pos: \t\t %dum\n", RX_StepperStatus.posY[0]);
//        term_printf("Wipe-Speed: \t\t %d\n", RX_StepperCfg.wipe_speed);
//        term_printf("Vacuum done: \t\t %d\n", RX_StepperStatus.robinfo.vacuum_done);
//        term_printf("Wash done: \t\t %d\n", RX_StepperStatus.robinfo.wash_done);
//        term_printf("adjustDoneCnt: \t\t %d\n", RX_StepperStatus.adjustDoneCnt);
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
        term_printf("b<n>: Set Speed of Waste pump to <n>%, Actual value %d%\n", _PumpSpeed);
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
    SClnMovePar SClnMovePar;
    switch (str[0])
    {
    case 's':
        lbrob_handle_ctrl_msg(INVALID_SOCKET, CMD_ROB_STOP, NULL);
        break;
    case 'o':
        Fpga.par->output ^= (1 << atoi(&str[1]));
        break;
    case 'R':
        lbrob_reference_slide();
        break;
    case 'r':
        motor_reset(atoi(&str[1]));
        break;
    case 'c':   
        SClnMovePar.function = rob_fct_cap; 
        lbrob_handle_ctrl_msg(INVALID_SOCKET, CMD_ROB_MOVE_POS, &SClnMovePar);  
        break;
    case 'M':
        SClnMovePar.function = rob_fct_maintenance;
        lbrob_handle_ctrl_msg(INVALID_SOCKET, CMD_ROB_MOVE_POS, &SClnMovePar);
        break;
    case 'w':
        SClnMovePar.function = rob_fct_wash;
        lbrob_handle_ctrl_msg(INVALID_SOCKET, CMD_ROB_MOVE_POS, &SClnMovePar);
        break;
    case 'q':
        val = atoi(&str[1]);
        lbrob_handle_ctrl_msg(INVALID_SOCKET, CMD_ROB_VACUUM, &val);
        break;
    case 'v':
        SClnMovePar.function = rob_fct_vacuum;
        lbrob_handle_ctrl_msg(INVALID_SOCKET, CMD_ROB_MOVE_POS, &SClnMovePar);
        break;
    case 'g':
        SClnMovePar.function = rob_fct_move_purge;
        SClnMovePar.position = str[1];
        lbrob_handle_ctrl_msg(INVALID_SOCKET, CMD_ROB_MOVE_POS, &SClnMovePar);
        break;
    case 'p':
        pos = rob_fct_move_purge;
        lbrob_handle_ctrl_msg(INVALID_SOCKET, CMD_ROB_FILL_CAP, &pos);
        break;
    case 'm':
        _lbrob_motor_test(str[1] - '0', atoi(&str[2]));
        break;
    case 'b':
        val = atoi(&str[1]);
        if (val < 0) val = 0;
        if (val > 100) val = 100;
        _PumpSpeed = val;
        break;
    }
}

//--- lbrob_stop ----------------------------------------------
void lbrob_stop(void)
{
    motors_stop(MOTOR_SLIDE_BITS);
    Fpga.par->output &= ~RO_ALL_FLUSH_OUTPUTS;
    if (_CmdRunning == CMD_HEAD_ADJUST)
    {
        Error(LOG, 0, "AdjustCnt++");
        RX_StepperStatus.adjustDoneCnt++;
    }

    _CmdRunning_Lift = 0;
    _CmdRunning = 0;
    _RO_Flush = 0;
    RX_StepperStatus.robinfo.ref_done = FALSE;
    rc_stop();
}

//--- lbrob_reference_slide --------------------------
void	 lbrob_reference_slide(void)
{
    Fpga.par->output &= ~RO_ALL_FLUSH_OUTPUTS;
    _NewCmd = 0;
    RX_StepperStatus.robinfo.moving = TRUE;
    _CmdRunning = CMD_ROB_REFERENCE;

    TrPrintfL(TRUE, "lbrob_reference_slide: z_in_down=%d, slide.ref_done=%d, SlideInRef=%d", RX_StepperStatus.screwerinfo.z_in_down, RX_StepperStatus.robinfo.ref_done, fpga_input(SLIDE_REF));

    if (!RX_StepperStatus.screwerinfo.z_in_down)
    {
        rc_move_bottom(_FL_);
    }
    else if (!RX_StepperStatus.robinfo.ref_done)
    {
        if (fpga_input(SLIDE_REF))
        {
            TrPrintfL(TRUE, "lbrob_reference_slide: 1");
            motors_move_by_step(MOTOR_SLIDE_BITS, &_ParSlide_drive, _micron_2_steps(-5000), TRUE);
        }
        else
        {
            TrPrintfL(TRUE, "lbrob_reference_slide: 2");
            motor_reset(MOTOR_SLIDE);
            motor_config(MOTOR_SLIDE, CURRENT_HOLD, SLIDE_STEPS_PER_REV, SLIDE_INC_PER_REV, STEPS);
            RX_StepperStatus.robinfo.ref_done = FALSE;
            motors_move_by_step(MOTOR_SLIDE_BITS, &_ParSlide_ref, 1000000, TRUE);
        }
    }
    else if (_CapIsWet)
    {
        TrPrintfL(TRUE, "lbrob_reference_slide: 3");
        motor_reset(MOTOR_SLIDE);
        motor_config(MOTOR_SLIDE, CURRENT_HOLD, SLIDE_STEPS_PER_REV, SLIDE_INC_PER_REV, STEPS);
        RX_StepperStatus.robinfo.ref_done = FALSE;
        motors_move_by_step(MOTOR_SLIDE_BITS, &_ParSlide_ref, 1000000, TRUE);
    }
	else 
    {   
        TrPrintfL(TRUE, "lbrob_reference_slide: 4");
        lbrob_move_to_pos(CMD_ROB_REFERENCE, _micron_2_steps(500), FALSE);
    }
}

//--- lbrob_move_to_pos ---------------------------------------------------------------
void lbrob_move_to_pos(int cmd, int pos, int wipe_state)
{
    if (cmd) _CmdRunning = cmd;
    RX_StepperStatus.robinfo.moving = TRUE;
    int actual_pos = motor_get_step(MOTOR_SLIDE);

    TrPrintfL(TRUE, "lbrob_move_to_pos cmd=0x%08x, pos=%d, wipe_state=%d", _CmdRunning, pos, wipe_state);

    if (wipe_state)
    {
        if (RX_StepperCfg.wipe_speed)
            _ParSlide_drive_purge.speed = _micron_2_steps(1000 * RX_StepperCfg.wipe_speed); // multiplied with 1000 to get from mm/s to um/s
        else
            _ParSlide_drive_purge.speed = _micron_2_steps(1000 * 10); // multiplied with 1000 to get from mm/s to um/s
        motors_move_to_step(MOTOR_SLIDE_BITS, &_ParSlide_drive_purge, pos);
    }
    else if (_CapIsWet && (pos > actual_pos)) // move backwards
    {
        motors_move_to_step(MOTOR_SLIDE_BITS, &_ParSlide_drive_slow, pos);
    }      
    else
    {
        if (abs(pos-actual_pos)<20)
             motors_move_to_step(MOTOR_SLIDE_BITS, &_ParSlide_drive_slow, pos);
        else        
            motors_move_to_step(MOTOR_SLIDE_BITS, &_ParSlide_drive, pos);
    }
}

//--- lbrob_cln_move_to ------------------------
void lbrob_cln_move_to(int pos)
{
    _cln_move_to(CMD_ROB_MOVE_POS, rob_fct_move_to_pos, pos);
}

//--- _cln_move_to ---------------------------------------
static void _cln_move_to(int msgId, ERobotFunctions fct, int position)
{
    int val, pos, head;
    TrPrintfL(TRUE, "_cln_move_to msgId=0x%08x, fct=%s, _CmdRunning=0x%08x, position=%d, _NewPos=%d", msgId, RobFunctionStr[fct], _CmdRunning, position, _ClnMovePar.position);
    if (!_CmdRunning)
    {        
        if (_ClnMovePar.position==0) _ClnMovePar.position = position;

        if (_lbrob_start_sm(fct, position)) return;

        if (!RX_StepperStatus.screwerinfo.z_in_down)
        {
            Error(ERR_CONT, 0, "_cln_move_to: screwer not down!");
            return;
        }
        else if ((!(RX_StepperStatus.info.z_in_ref || RX_StepperStatus.info.z_in_screw) && !(_ClnMovePar.function == rob_fct_move && RX_StepperStatus.info.z_in_wash)) || !RX_StepperStatus.info.ref_done)
        {
            if (!RX_StepperStatus.info.moving)
            {
                _CmdRunning_Lift = CMD_LIFT_REFERENCE;
                lb702_handle_ctrl_msg(INVALID_SOCKET, _CmdRunning_Lift, NULL, _FL_);
                _NewCmd = msgId;
                _ClnMovePar.position = position;
            }
            else TrPrintfL(TRUE, "Error");
            return;
        }
        /*
        else if (!RX_StepperStatus.robinfo.ref_done || (_CapIsWet && _ClnMovePar.function != rob_fct_vacuum && _ClnMovePar.function != rob_fct_wash && _ClnMovePar.function != rob_fct_move && _ClnMovePar.function != rob_fct_move_purge))
        {
            Error(ERR_CONT, 0, "REFEREMCE SLIDE first msg=0x%08x", msgId);
            _CmdRunning_old = msgId;
            _ClnMovePar.position = position;
            lbrob_reference_slide();
            return;
        }
        */

        RX_StepperStatus.robinfo.moving = TRUE;
        _CmdRunning = msgId;
        switch (_ClnMovePar.function)
        {
        case rob_fct_purge4ever:
            lbrob_move_to_pos(0, _micron_2_steps(SLIDE_CAP_POS), FALSE);
            break;
            
        case rob_fct_maintenance:
            rc_stop();
            lbrob_move_to_pos(0, _micron_2_steps(SLIDE_MAINTENANCE_POS), FALSE);
            break;

        case rob_fct_move_purge:
            pos = (SLIDE_PURGE_POS_BACK + (position * (SLIDE_PURGE_POS_FRONT - SLIDE_PURGE_POS_BACK)) / 7);
            lbrob_move_to_pos(0, _micron_2_steps(pos), FALSE);
            break;
        case rob_fct_vacuum:
            lbrob_move_to_pos(0, _micron_2_steps(SLIDE_PURGE_POS_FRONT), FALSE);
            break;
        case rob_fct_wash:
            lbrob_move_to_pos(0, _micron_2_steps(SLIDE_WASH_POS_BACK), FALSE);
            break;

        case rob_fct_move:
            if (!RX_StepperStatus.info.z_in_wash)
            {
                if (!RX_StepperStatus.info.moving && !RX_StepperStatus.screwerinfo.moving && rc_move_done())
                {
                    _CmdRunning = 0;
                    RX_StepperStatus.robinfo.moving = FALSE;
                    _CmdRunning_Lift = CMD_LIFT_WASH_POS;
                    _NewCmd = msgId;
                    lb702_handle_ctrl_msg(INVALID_SOCKET, _CmdRunning_Lift, NULL, _FL_);
                }
                else TrPrintfL(TRUE, "Error");
                return;
            }
            
            switch (_Old_RobFunction)
            {
            case rob_fct_wash:
                lbrob_move_to_pos(0, _micron_2_steps(SLIDE_PURGE_POS_FRONT), TRUE);
                
                if (_RO_Flush)
                    Fpga.par->output |= _RO_Flush;
                else
                Fpga.par->output |= RO_FLUSH_WIPE;
                
                _RO_Flush &= ~RO_FLUSH_WIPE;
                Fpga.par->output |= RO_FLUSH_PUMP;
                Fpga.par->output &= ~RO_VACUUM_CLEANER;
                break;
                
            case rob_fct_vacuum:
                lbrob_move_to_pos(0, _micron_2_steps(SLIDE_WASH_POS_BACK), TRUE);
                _vacuum_on();
                break;
                
            default:
                break;
            }
            break;

        case rob_fct_move_to_pos:
            if (position == 0 && _ClnMovePar.position!=0) 
            {
                position = _ClnMovePar.position;
                _ClnMovePar.position = 0;
            }
            TrPrintfL(TRUE, "lbrob_move_to_pos 2 cmd=0x%08x, pos=%d", _CmdRunning, pos);
            lbrob_move_to_pos(0, _micron_2_steps(position), FALSE);            
            break;

        default:
            Error(ERR_CONT, 0, "Command %s: Rob-Function %d not implemented", _CmdName, _ClnMovePar.function);
        }
    }
    else TrPrintfL(TRUE, "_cln_move_to: CmdRunning");
}

//--- lbrob_handle_ctrl_msg -----------------------------------
int lbrob_handle_ctrl_msg(RX_SOCKET socket, int msgId, void *pdata)
{
    int val, pos;
    SClnMovePar robmovepos;

    switch (msgId)
    {
    case CMD_ROB_STOP:
        strcpy(_CmdName, "CMD_ROB_STOP");
        break;

    case CMD_ROB_REFERENCE:
        lbrob_reference_slide();
        break;

    case CMD_ROB_MOVE_POS:
        strcpy(_CmdName, "CMD_ROB_MOVE_POS");
        SClnMovePar tmp = (*(SClnMovePar *)pdata);
        _cln_move_to(msgId, tmp.function, tmp.position);
        break;

    case CMD_ROB_SERVICE:
        strcpy(_CmdName, "CMD_ROB_SERVICE");
        robmovepos.function = rob_fct_maintenance;
        lbrob_handle_ctrl_msg(INVALID_SOCKET, CMD_ROB_MOVE_POS, &robmovepos);
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
        }
        else if (val == 1 && !RX_StepperStatus.inkinfo.ink_pump_error_left)
        {
            Fpga.par->output |= RO_INK_VALVE_LEFT;
        }
        else if (val == 2)
        {
            if (Fpga.par->output & RO_INK_VALVE_LEFT)
                _PumpTimeLeft = rx_get_ticks() + FOLLOW_UP_TIME_INK_BACK;
        }
        else if (val == 3 && !RX_StepperStatus.inkinfo.ink_pump_error_right)
        {
            Fpga.par->output |= RO_INK_VALVE_RIGHT;
        }
        else if (val == 4)
        {
            if (Fpga.par->output & RO_INK_VALVE_RIGHT)
                _PumpTimeRight = rx_get_ticks() + FOLLOW_UP_TIME_INK_BACK;
        }
        else if (val == 5)
        {
            Fpga.par->output |= RO_INK_VALVE_BOTH;
        }
        break;

    case CMD_ROB_VACUUM:
        /*
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
        */
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
            SClnMovePar par = (*(SClnMovePar *)pdata);
            switch (par.function)
            {
            case rob_fct_cap:
                _CapIsWet = TRUE;
                _CmdRunning = msgId;
                _ClnMovePar.function = par.function;
                RX_StepperStatus.robinfo.moving = TRUE;
                Fpga.par->output &= ~RO_ALL_FLUSH_OUTPUTS;
                if (RX_StepperCfg.printerType != printer_LB702_UV) Fpga.par->output |= RO_FLUSH_TO_CAP_LEFT;
                Fpga.par->output |= RO_FLUSH_PUMP;
                _CapFillTime = rx_get_ticks();
                break;

            case rob_fct_move_purge_end:
                _cln_move_to(msgId, rob_fct_move_purge_end, SLIDE_PURGE_POS_FRONT);
                break;

            default:
                break;
            }
        }
        break;

    case CMD_ROB_SET_FLUSH_VALVE:
        val = (*(INT32 *)pdata);
        if (val == 0)           _RO_Flush |= RO_FLUSH_WIPE_LEFT;
        else if (val == 1)      _RO_Flush |= RO_FLUSH_WIPE_RIGHT;
        else if (val == 2)      _RO_Flush |= RO_FLUSH_WIPE;
        else                    _RO_Flush &= ~RO_FLUSH_WIPE;
        break;

    default:
        rc_handle_ctrl_msg(socket, msgId, pdata);
        break;
    }
    return REPLY_OK;
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
        _VacuumTimer     = rx_get_ticks()+VACUUM_PUMP_TIME;
        _WastePumpTimer  = rx_get_ticks()+WASTE_PUMP_TIME;
    }
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

        motors_config(motors, CURRENT_HOLD, SLIDE_STEPS_PER_REV, SLIDE_INC_PER_REV, STEPS);
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
