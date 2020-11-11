// ****************************************************************************
//
//	plc_ctrl.cpp
//
// ****************************************************************************
//
//	Copyright 2014 by Radex AG, Switzerland. All rights reserved.
//	Written by Marcel Galliker
//
// ****************************************************************************

//--- includes ----------------------------------------------------------------
#ifdef linux
#include <termios.h>
#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "rx_def.h"
#include "rx_threads.h"
#include "args.h"
#include "rx_error.h"
#include "rx_trace.h"
#include "tcp_ip.h"
#include "enc_ctrl.h"
#include "step_tts.h"
#include "plc_ctrl.h"
#include "rx_setup_file.h"
#include "drive_ctrl.h"


#define DEVICE_NAME "ttyUSB0"

static const char *_convert = "0123456789ABCDEF";

#define READ_REGISTER 0x03
#define WRITE_REGISTER 0x06

#define TIMEOUT 2

// Settings to do, before the movement
#define CONTROL_WORD "o1100."
#define TARGETPOSITION "o1901."
#define SPEED "o1902."
#define MODE "o1905."
#define ACCEL "o1906."
#define DECEL "o1907."
#define JERK "o1908."

#define STATE_WORD  "o1000.3"

// State Word Signals
#define REF_DONE            0x1000
#define AXIS_CURRENT        0x0800
#define AXIS_CURRENTLESS    0x0400
#undef  NO_ERROR
#define NO_ERROR            0x0100

// differnet Modes to Drive with the motors
#define MOVE_ABS 1
#define MOVE_REL 2
#define GEARING 3
#define REG_SEARCH 4
#define REG_MOVE 5
#define VELOCITY 6

#define ACTIVATE 1
#define HOMING          0x6003
#define START_PRINT     0x6103
#define START_START     0x6203
#define START_WASTE     0x6303
#define START_JET       0x6403
#define START_BACK      0x6503
#define START_TABLE     0x6603
#define STOP            0x0003
#define ERROR_RESET     0x4003
#define JOG_FWD         0x4007
#define JOG_BWD         0x400b
#define JOG_STOP        0x4003

// different Rows in the Drive Table
#define PRINTING    1
#define MOVE_START  2
#define MOVE_WASTE  3
#define MOVE_JET    4
#define MOVE_BACK   5
#define MOVE_TABLE  6



// Control Word
#undef CTRL
#define CTRL 3

typedef enum
{
    move_undef,
    move_standstill,
    move_homing,
    move_start,
    move_print,
    move_waste,
    move_jet,
    move_back,
    move_table,
    move_run,
    move_wait,
    reset_error,
    jog_fwd,
    jog_bwd,
} Movement_State;


static void *_drive_thread(void *lpParameter);
static void _set_baud_7e1(int baud);
static void _send_data(char* data, int len);
static int _read_data(int function, float *data);
static int _set_setting(int mode, char* setting, int val);
static void _drive_load_material(char* material);

static int _m_min_to_mm_sec(int speed_m_min);

static float _get_setting(char *object);

static int _Init_Start = 0;
static Movement_State _MoveCmd = move_undef;
static Movement_State _New_MoveCmd = move_undef;
static Movement_State _Ref_MoveCmd = move_undef;

//--- global variables ------------------------------------------
static SDriveStat _DriveStatus;

static int _DriveThreadRunning;
static int _Timeout;
static int _Handle = 0;
static int _In_Ref = FALSE;
static int _Axis_Stillstand = FALSE;
static int _Ref_Done = FALSE;
static int _Error = FALSE;
static int _Socket = 0;

static int _Actual_Speed = 0;                       // m/min
static int _Actual_TargetPosition = 0;
static int _Actual_Mode = 0;
static int _Actual_Accel = 0;                       // m/s^2
static int _Actual_Decel = 0;                       // 1/s^2
static int _Actual_Jerk = 0;                        // 1/s^3

static int _Move_Start_Speed = 490;                 // mm/s
static int _Move_Start_TargetPosition = 0;          // mm
static int _Move_Start_Acceleration = 2600;         // mm/s^2
static int _Move_Start_Jerk = 7200;                 // mm/s^3
static int _Move_Table_TargetPosition = 400;         // mm
static int _Move_Waste_TargetPosition = 649;        // mm
static int _Move_Jet_TargetPosition = 588;          // mm
static int _Move_Back_TargetPosition = 750;          // mm


//--- drive_init ------------------------------------------------
int drive_init(void)
{
    
    drive_error_reset();
    _drive_load_material(RX_Config.material);
    _Socket = INVALID_SOCKET;

    if (!_DriveThreadRunning)
    {
        _DriveThreadRunning = TRUE;
#ifdef linux
        rx_thread_start(_drive_thread, NULL, 0, "_drive_thread");
#endif // linux
    }


    if (arg_simuDrive)
    {
        _DriveStatus.enabled = FALSE;
        return REPLY_OK;
    }
    else
    {
        _DriveStatus.enabled = (RX_Config.printer.type == printer_test_table_seon);
        _DriveStatus.acceleration = 3600;
        _DriveStatus.decelerlation = 3600;
        _DriveStatus.jerk = 96000;

#ifndef linux
        if (_DriveStatus.enabled) Error(ERR_CONT, 0, "Parker Drive is not implemented in Windows version");
        #endif // !linux
    }

    
    return REPLY_OK;
}

//--- drive_error_reset ----------------------------------------------
void drive_error_reset(void)
{
#ifdef linux
    if (rx_def_is_tts(printer_test_table_seon) && _Error)
    {
        _MoveCmd = reset_error;
    }
#endif // linux
}

//--- _drive_thread -------------------------------------------------
#ifdef linux
static void *_drive_thread(void *lpParameter)
{
    char buf[256];
    int ret;
    int err = FALSE;
    while (_DriveThreadRunning)
    {
        if (_DriveStatus.enabled)
        {
            ret = REPLY_ERROR;
            if (_Handle <= 0)
            {
                _Handle = open("/dev/" DEVICE_NAME, O_RDWR);
                if (_Handle > 0)
                {
                    _set_baud_7e1(B115200); // prov value -> Needs to be
                                           // discussed with Parker first
                    _Timeout = TIMEOUT;
                }
                else if (!err)
                {
                    err = TRUE;
                    err_system_error(errno, buf, sizeof(buf));
                    TrPrintfL(TRUE, "Parker Drive: Error %d: %s\n", errno, buf);
                }
            }

            if (_Handle >= 0)
            {
                if (_DriveStatus.speed != _Actual_Speed)
                {
                    ret = _set_setting(PRINTING, SPEED, _m_min_to_mm_sec(_DriveStatus.speed));
                    if (ret == REPLY_OK) _Actual_Speed = _DriveStatus.speed;
                }
                if (_DriveStatus.targetposition != _Actual_TargetPosition)
                {
                    ret = _set_setting(PRINTING, TARGETPOSITION,
                                       /*_DriveStatus.targetposition*/
                                       _Move_Back_TargetPosition);
                    if (ret == REPLY_OK)
                        _Actual_TargetPosition = _DriveStatus.targetposition;
                }
                if (_Actual_Mode != MOVE_ABS)
                {
                    ret = _set_setting(PRINTING, MODE, MOVE_ABS);
                    if (ret == REPLY_OK) _Actual_Mode = MOVE_ABS;
                }
                if (_DriveStatus.acceleration != _Actual_Accel)
                {
                    ret = _set_setting(PRINTING, ACCEL,
                                       _DriveStatus.acceleration);
                    if (ret == REPLY_OK)
                        _Actual_Accel = _DriveStatus.acceleration;
                }
                if (_DriveStatus.decelerlation != _Actual_Decel)
                {
                    ret = _set_setting(PRINTING, DECEL,
                                       _DriveStatus.decelerlation);
                    if (ret == REPLY_OK)
                        _Actual_Decel = _DriveStatus.decelerlation;
                }
                if (_DriveStatus.jerk != _Actual_Jerk)
                {
                    ret = _set_setting(PRINTING, JERK, _DriveStatus.jerk);
                    if (ret == REPLY_OK) _Actual_Jerk = _DriveStatus.jerk;
                }
                if (_Init_Start == 0)
                {
                    ret = _set_setting(MOVE_START, SPEED, _Move_Start_Speed);
                    ret += _set_setting(MOVE_START, TARGETPOSITION,
                                       _Move_Start_TargetPosition);
                    ret += _set_setting(MOVE_START, MODE, MOVE_ABS);
                    ret += _set_setting(MOVE_START, ACCEL,
                                        _Move_Start_Acceleration);
                    ret += _set_setting(MOVE_START, DECEL,
                                        _Move_Start_Acceleration);
                    ret += _set_setting(MOVE_START, JERK, _Move_Start_Jerk);
                    if (ret == REPLY_OK)
                        _Init_Start++;
                    else
                        Error(ERR_CONT, 0,
                              "Parker Init Error in State %d with %d Errors",
                              _Init_Start, ret);
                }
                if (_Init_Start == 1)
                {
                    ret = _set_setting(MOVE_WASTE, SPEED, _Move_Start_Speed);
                    ret += _set_setting(MOVE_WASTE, TARGETPOSITION,
                                        _Move_Waste_TargetPosition);
                    ret += _set_setting(MOVE_WASTE, MODE, MOVE_ABS);
                    ret += _set_setting(MOVE_WASTE, ACCEL,
                                        _Move_Start_Acceleration);
                    ret += _set_setting(MOVE_WASTE, DECEL,
                                        _Move_Start_Acceleration);
                    ret += _set_setting(MOVE_WASTE, JERK,
                                        _Move_Start_Jerk);
                    if (ret == REPLY_OK) _Init_Start++;
                    else
                        Error(ERR_CONT, 0, "Parker Init Error in State %d with %d Errors", _Init_Start, ret);
                }
                if (_Init_Start == 2)
                {
                    ret = _set_setting(MOVE_JET, SPEED, _Move_Start_Speed);
                    ret += _set_setting(MOVE_JET, TARGETPOSITION, _Move_Jet_TargetPosition);
                    ret += _set_setting(MOVE_JET, MODE, MOVE_ABS);
                    ret += _set_setting(MOVE_JET, ACCEL, _Move_Start_Acceleration);
                    ret += _set_setting(MOVE_JET, DECEL, _Move_Start_Acceleration);
                    ret += _set_setting(MOVE_JET, JERK, _Move_Start_Jerk);
                    if (ret == REPLY_OK)
                        _Init_Start++;
                    else
                        Error(ERR_CONT, 0, "Parker Init Error in State %d with %d Errors", _Init_Start, ret);
                }
                if (_Init_Start == 3)
                {
                    ret = _set_setting(MOVE_BACK, SPEED, _Move_Start_Speed);
                    ret += _set_setting(MOVE_BACK, TARGETPOSITION, _Move_Back_TargetPosition);
                    ret += _set_setting(MOVE_BACK, MODE, MOVE_ABS);
                    ret += _set_setting(MOVE_BACK, ACCEL, _Move_Start_Acceleration);
                    ret += _set_setting(MOVE_BACK, DECEL, _Move_Start_Acceleration);
                    ret += _set_setting(MOVE_BACK, JERK, _Move_Start_Jerk);
                    if (ret == REPLY_OK)
                        _Init_Start++;
                    else
                        Error(ERR_CONT, 0, "Parker Init Error in State %d with %d Errors", _Init_Start, ret);
                }
                if (_Init_Start == 4)
                {
                    ret = _set_setting(MOVE_TABLE, SPEED, _Move_Start_Speed);
                    ret += _set_setting(MOVE_TABLE, TARGETPOSITION, _Move_Table_TargetPosition);
                    ret += _set_setting(MOVE_TABLE, MODE, MOVE_ABS);
                    ret += _set_setting(MOVE_TABLE, ACCEL, _Move_Start_Acceleration);
                    ret += _set_setting(MOVE_TABLE, DECEL, _Move_Start_Acceleration);
                    ret += _set_setting(MOVE_TABLE, JERK, _Move_Start_Jerk);
                    if (ret == REPLY_OK)
                        _Init_Start++;
                    else
                        Error(ERR_CONT, 0, "Parker Init Error in State %d with %d Errors", _Init_Start, ret);
                }

                if (_Init_Start == 5)
                {
                    ret = (int)_get_setting(STATE_WORD);
                    _Init_Start++;
                }

                if (_Init_Start == 6)
                {

                    ret = (int)_get_setting(STATE_WORD);
                    _Error = !(ret & NO_ERROR);
                    _Axis_Stillstand = ret & AXIS_CURRENT && ret & NO_ERROR;
                    if (ret & REF_DONE) _In_Ref = FALSE;
                    _Ref_Done =
                        ((ret & REF_DONE) || (_In_Ref && _Axis_Stillstand)) && !_Error;

                    if ((_MoveCmd == move_start || _MoveCmd == move_print ||
                        _MoveCmd == move_waste || _MoveCmd == move_waste ||
                        _MoveCmd == move_jet || _MoveCmd == move_back || _MoveCmd == move_table) && !_Ref_Done)
                    {
                        _Ref_MoveCmd = _MoveCmd;
                        _MoveCmd = move_homing;
                    }

                    //Error(LOG, 0, "Move-Cmd: %d", _MoveCmd);
                    switch (_MoveCmd)
                    {
                    case move_undef:
                        ret = _set_setting(CTRL, CONTROL_WORD, ACTIVATE);
                        if (ret == REPLY_OK) _MoveCmd = move_standstill;
                        break;
                    case move_homing:
                        if (RX_StepperStatus.info.z_in_ref ||
                            RX_StepperStatus.info.z_in_up ||
                            RX_StepperStatus.info.z_in_print)
                        {
                            _set_setting(CTRL, CONTROL_WORD, STOP);
                            ret = _set_setting(CTRL, CONTROL_WORD, HOMING);
                            if (ret == REPLY_OK) _MoveCmd = move_run;
                            _In_Ref = TRUE;
                        }
                        break;
                    case move_start:
                        if (_Ref_Done)
                        {
                            _set_setting(CTRL, CONTROL_WORD, STOP);
                            _set_setting(CTRL, CONTROL_WORD, START_START);
                            _MoveCmd = move_run;
                            _Ref_MoveCmd = move_undef;
                        }
                        break;

                    case move_standstill:
                        if (_Ref_MoveCmd != move_undef)
                        {
                            _MoveCmd = _Ref_MoveCmd;
                        }
                        break;
                    case move_print:
                        if (RX_StepperStatus.info.z_in_print)
                        {
                            
                            ret = drive_get_scanner_pos();
                            if (abs(ret - _Move_Start_TargetPosition) <= 2 &&
                                _Axis_Stillstand)
                            {
                                _set_setting(CTRL, CONTROL_WORD, STOP);
                                _set_setting(CTRL, CONTROL_WORD, START_PRINT);
                                _MoveCmd = move_run;
                                _Ref_MoveCmd = move_undef;
                            }
                            else if (_Axis_Stillstand)
                            {
                                enc_set_config(FALSE);
                                _set_setting(CTRL, CONTROL_WORD, STOP);
                                _set_setting(CTRL, CONTROL_WORD, START_START);
                            } 
                        }
                        
                        break;
                    case move_waste:
                        _set_setting(CTRL, CONTROL_WORD, STOP);
                        _set_setting(CTRL, CONTROL_WORD, START_WASTE);
                        _MoveCmd = move_run;
                        _Ref_MoveCmd = move_undef;
                        break;
                    case move_back:
                        _set_setting(CTRL, CONTROL_WORD, STOP);
                        _set_setting(CTRL, CONTROL_WORD, START_BACK);
                        _MoveCmd = move_run;
                        _Ref_MoveCmd = move_undef;
                        break;
                    case move_jet:
                        _set_setting(CTRL, CONTROL_WORD, STOP);
                        _set_setting(CTRL, CONTROL_WORD, START_JET);
                        _MoveCmd = move_run;
                        _Ref_MoveCmd = move_undef;
                        break;
                    case move_table:
                        _set_setting(CTRL, CONTROL_WORD, STOP);
                        _set_setting(CTRL, CONTROL_WORD, START_TABLE);
                        _MoveCmd = move_run;
                        _Ref_MoveCmd = move_undef;
                        break;
                    case move_run:
                        if (_Axis_Stillstand) _MoveCmd = move_standstill;
                        break;
                    case reset_error:
                        _set_setting(CTRL, CONTROL_WORD, 0);
                        _set_setting(CTRL, CONTROL_WORD, ERROR_RESET);
                        _MoveCmd = move_standstill;
                        break;
                    case jog_fwd:
                    case jog_bwd:
                        break;
                    case move_wait:
                        switch (_New_MoveCmd)
                        {
                        case move_waste:
                            _New_MoveCmd = move_undef;
                            drive_move_waste();
                            break;
                            
                        case move_jet:
                            _New_MoveCmd = move_undef;
                            drive_move_jet();
                            break;
                            
                        case move_start:
                            _New_MoveCmd = move_undef;
                            drive_move_start();
                            break;
                            
                        case move_back:
                            _New_MoveCmd = move_undef;
                            drive_move_back();
                            break;
                        default: break;
                        }
                        break;
                    default:
                        Error(ERR_CONT, 0, "Unknown command %d for drive",
                              _MoveCmd);
                        break;
                    }
                }
                rx_sleep(1000);
            }
        }
    }
}
#endif // linux

//--- drive_set_prinbar ------------------------------------------
int drive_set_printbar(SPrintQueueItem *pItem)
{
    TrPrintfL(TRUE, "drive_set_printbar");
    enc_start_printing(pItem, FALSE);
    _DriveStatus.speed = pItem->speed;
    _DriveStatus.targetposition = _Move_Back_TargetPosition;    //pItem->scanLength;
    steptts_to_print_pos();
    _MoveCmd = move_print;
    return REPLY_OK;
}


//--- drive_get_scanner_pos ------------------------------------------
UINT32 drive_get_scanner_pos(void)
{
    float pos;
    pos = _get_setting("o680.5");
    return (UINT32 )pos;
}

//--- drive_start_printing ------------------------------------------
int drive_start_printing(void)
{
    return TRUE;
}

//--- drive_stop_printing ------------------------------------------
void drive_stop_printing(void)
{
    _set_setting(CTRL, CONTROL_WORD, STOP);
    _In_Ref = FALSE;
    _MoveCmd = move_standstill;
    _New_MoveCmd = move_undef;
}

//--- drive_abort_printing ------------------------------------------
void drive_abort_printing(void)
{
    _set_setting(CTRL, CONTROL_WORD, STOP);
}

void drive_move_start(void)
{
    if (RX_StepperStatus.info.z_in_ref || RX_StepperStatus.info.z_in_up || RX_StepperStatus.info.z_in_print)
    {
        if (_MoveCmd == move_standstill || _MoveCmd == move_wait)
            _MoveCmd = move_start;
    }
    else
    {
        if (!RX_StepperStatus.info.moving)
            steptts_handle_gui_msg(INVALID_SOCKET, CMD_LIFT_UP_POS, NULL, 0);
        _New_MoveCmd = move_start;
        _MoveCmd = move_wait;
    }
}

void drive_move_homing(void)
{
    if (_MoveCmd == move_standstill)
        _MoveCmd = move_homing;
}

void drive_jog_cmd(int cmd)
{
    switch (cmd)
    {
    case CMD_TTS_JOG_FWD:
        if (_MoveCmd == move_standstill && (RX_StepperStatus.info.z_in_ref || RX_StepperStatus.info.z_in_up || RX_StepperStatus.info.z_in_print))
        {
            _MoveCmd = jog_fwd;
            _set_setting(CTRL, CONTROL_WORD, JOG_FWD);
        }
        else if (_MoveCmd != move_standstill) Error(WARN, 0, "Scanner not in stillstand mode");
        else if (!RX_StepperStatus.info.ref_done) Error(WARN, 0, "Stepper motors not referenced");
        else if (!RX_StepperStatus.info.z_in_ref && !RX_StepperStatus.info.z_in_up && !RX_StepperStatus.info.z_in_print)
            Error(WARN, 0, "Print heads in wrong hight for jogging");
        else Error(WARN, 0, "Jogging not possible");
        break;

    case CMD_TTS_JOG_BWD:
        if (_MoveCmd == move_standstill && (RX_StepperStatus.info.z_in_ref || RX_StepperStatus.info.z_in_up || RX_StepperStatus.info.z_in_print))
        {
            _MoveCmd = jog_bwd;
            _set_setting(CTRL, CONTROL_WORD, JOG_BWD);
        }
        else if (_MoveCmd != move_standstill) Error(WARN, 0, "Scanner not in stillstand mode");
        else if (!RX_StepperStatus.info.ref_done) Error(WARN, 0, "Stepper motors not referenced");
        else if (!RX_StepperStatus.info.z_in_ref && !RX_StepperStatus.info.z_in_up && !RX_StepperStatus.info.z_in_print)
            Error(WARN, 0, "Print heads in wrong hight for jogging");
        else Error(WARN, 0, "Jogging not possible");
        break;

    case CMD_TTS_JOG_STOP:
        _set_setting(CTRL, CONTROL_WORD, JOG_STOP);
        if (_MoveCmd == jog_fwd || _MoveCmd == jog_bwd)
            _MoveCmd = move_standstill;
        break;
        
    default:
        Error(ERR_CONT, 0, "Command 0x%8x not implemented", cmd);
        break;
    }
}

//--- drive_move_waste ------------------------------------------------
void drive_move_waste(void)
{
    if (RX_StepperStatus.info.z_in_up || RX_StepperStatus.info.z_in_print ||
        RX_StepperStatus.info.z_in_ref)
    {
        if (_MoveCmd == move_standstill || _MoveCmd == move_wait)
        {
            _MoveCmd = move_waste;
        }
    }
    else
    {
        if (!RX_StepperStatus.info.moving)
            steptts_handle_gui_msg(INVALID_SOCKET, CMD_LIFT_UP_POS, NULL, 0);
        _New_MoveCmd = move_waste;
        _MoveCmd = move_wait;
    }
}

//--- drive_move_jet ------------------------------------------------
void drive_move_jet(void)
{
    if (RX_StepperStatus.info.z_in_up || RX_StepperStatus.info.z_in_print ||
        RX_StepperStatus.info.z_in_ref)
    {
        if (_MoveCmd == move_standstill || _MoveCmd == move_wait)
        {
            _MoveCmd = move_jet;
        }
    }
    else
    {
        if (!RX_StepperStatus.info.moving)
            steptts_handle_gui_msg(INVALID_SOCKET, CMD_LIFT_UP_POS, NULL, 0);
        _New_MoveCmd = move_jet;
        _MoveCmd = move_wait;
    }
}

void drive_move_back(void)
{
    if (RX_StepperStatus.info.z_in_up || RX_StepperStatus.info.z_in_print ||
        RX_StepperStatus.info.z_in_ref)
    {
        if (_MoveCmd == move_standstill || _MoveCmd == move_wait)
            _MoveCmd = move_back;
    }
    else
    {
        if (!RX_StepperStatus.info.moving) steptts_handle_gui_msg(INVALID_SOCKET, CMD_LIFT_UP_POS, NULL, 0);
        _New_MoveCmd = move_back;
        _MoveCmd = move_wait;
    }
}

void drive_move_table(void)
{
    if (RX_StepperStatus.info.z_in_up || RX_StepperStatus.info.z_in_print ||
        RX_StepperStatus.info.z_in_ref)
    {
        if (_MoveCmd == move_standstill || _MoveCmd == move_wait)
            _MoveCmd = move_table;
    }
    else
    {
        if (!RX_StepperStatus.info.moving) steptts_handle_gui_msg(INVALID_SOCKET, CMD_LIFT_UP_POS, NULL, 0);
        _New_MoveCmd = move_table;
        _MoveCmd = move_wait;
    }
}

//--- drive_in_waste --------------------------------
int drive_in_waste(void)
{
    int pos;
    if (_MoveCmd != move_standstill)
        return FALSE;
    else
        pos = drive_get_scanner_pos();

    if (abs(pos - _Move_Waste_TargetPosition) <= 1) return TRUE;
    return FALSE;
}

//--- _set_setting -------------------------------------------------------------------
static int _set_setting(int mode, char* setting, int val)
{
    if (_DriveStatus.enabled)
    {
        char buf[256] = "";
        int i;
        float reply;

        for (i = 0; i < (int)strlen(setting); i++)
        {
            buf[i] = setting[i];
        }
        sprintf(buf, "%s%d", buf, mode);
        buf[strlen(setting) + 1] = '=';
        sprintf(buf, "%s%d", buf, val);
        _send_data(buf, (int)strlen(buf));
        if (_read_data(WRITE_REGISTER, &reply) == REPLY_OK)
            return REPLY_OK;
        return REPLY_ERROR;
    }
    else
    {
        Error(WARN, 0, "_DriveStatus still disabled.");
        return REPLY_ERROR;
    }
}

static float _get_setting(char *object)
{
    if (_DriveStatus.enabled)
    {
        char buf[256] = "";
        int i;
        float reply;

        for (i = 0; i < (int)strlen(object); i++)
        {
            buf[i] = object[i];
        }
        _send_data(buf, (int)strlen(buf));
        if (_read_data(READ_REGISTER, &reply) == REPLY_OK)
            return reply;
       return REPLY_ERROR;
    }
    else
    {
        Error(WARN, 0, "_DriveStatus still disabled.");
        return REPLY_ERROR;
    }
}


static void _send_data(char* data, int len)
{
    char buf[256];
    int i;

    memset(buf, 0, sizeof(buf));
    buf[0] = '2';                              // Slave Addr .x
    for (i = 0; i < len; i++)
    {
        buf[1 + i] = data[i];
    }

    len += 1;

    buf[len++] = '\r';                        // CR
    buf[len++] = '\n';                        // LF
    write(_Handle, buf, len);
}


//--- _read_data ----------------------------------
static int _read_data(int function, float *data)
{
    int idx = 0;
    int l;
    int len = 32;
    char buf[256] = "";
    char *confirm = "*>\n";
    char *error = "*!";
    
    struct timeval timeout;
    fd_set readSet;

    int tio = 1000;
    timeout.tv_sec = tio / 1000;
    timeout.tv_usec = 1000 * (tio % 1000);

    FD_ZERO(&readSet);
    FD_SET(_Handle, &readSet);
    if (select(_Handle + 1, &readSet, NULL, NULL, &timeout) == 0)
        return REPLY_ERROR;
    l = read(_Handle, &buf[idx], len - idx);
    if (l < 0) return REPLY_ERROR;

    for (len = 0; len < sizeof(buf) && !(buf[len] == '\n'); len++)
    {
        
    }
    buf[len] = 0;
    if (function == WRITE_REGISTER)
    {
        //if (l == 2 && buf[0] == '*' && buf[1] == '>')
        if (!strncmp(&confirm[0], buf, 2))
        {
            return REPLY_OK;
        }
        else if (!strncmp(&error[0], buf, 2))
        {
            Error(ERR_CONT, 0, "Error message %s from Parker Drive", &buf[2]);
            return REPLY_ERROR;
        }
    }
    else if (function == READ_REGISTER)
    {
        *data = (float)atof(&buf[1]);
        return REPLY_OK;
    }
    return REPLY_OK;
}

//--- _set_baud_7e1 --------------------
static void _set_baud_7e1(int baud)
{
#ifdef linux
    struct termios options;

    tcgetattr(_Handle, &options);
    cfsetispeed(&options, baud);
    cfsetospeed(&options, baud);

    // mode: 7e1
    options.c_cflag &= ~PARENB; // Disable parity generation on output and pairty
                                // checking for input
    options.c_cflag &= ~CSTOPB; // If set, Set two stop bits, rather than one
    options.c_cflag &= ~CSIZE;  // This is a mask for the number of bits ber
                                // character -> here is reset
    options.c_cflag |= CS8;     // This specifies eight bits per byte
    options.c_cflag |= CREAD;   // Enable receiver
    options.c_cflag |= baud;    // set baud rate

    options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);

    tcsetattr(_Handle, TCSANOW, &options);
#endif // linux
}

static int _m_min_to_mm_sec(int speed_m_min)
{
    if (speed_m_min > 45) speed_m_min = 45;
    int speed_mm_sec = (speed_m_min * 1000 )/ 60;
    return speed_mm_sec;
}

static void _drive_load_material(char* material)
{
    if (RX_Config.printer.type == printer_test_table_seon) plc_load_material(material);	
}
/*
void drive_ctrl_save_material(char *varList)
{
    HANDLE attribute = NULL;
    char *str = varList;
    char *end;
    char *val;
    char var[128];

    //--- find material ---
    if ((end = strchr(str, '\n')))
    {
        end++;
    }
    str = end;

    while ((end = strchr(str, '\n')))
    {
        *end = 0;
        val = strchr(str, '=');
        *val = 0;
        strcpy(var, str);
        *val++ = '=';
        printf(">>%s<< = >>%s<<\n", var, val);
        if (!strcmp(var, "XML_MATERIAL"))
            strncpy(_Status.material, val, sizeof(_Status.material) - 1);
        if (!strcmp(var, "XML_HEAD_HEIGHT"))
            _Status.head_height = (INT32)(atof(val) * 1000);
        if (!strcmp(var, "XML_MATERIAL_THICKNESS"))
            _Status.thickness = (INT32)(atof(val) * 1000);
        if (!strcmp(var, "XML_ENC_OFFSET")) _Status.encoder_adj = atoi(val);
        *end++ = '\n';
        str = end;
    }
    _drive_ctrl_tick();
}*/