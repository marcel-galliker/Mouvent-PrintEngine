// ****************************************************************************
//
//	step_tts.c	Test Table Seon Stepper Control		
//
// ****************************************************************************
//
//	Copyright 2016 by Radex AG, Switzerland. All rights reserved.
//	Written by Marcel Galliker 
//
// ****************************************************************************

#include "rx_sok.h"
#include "rx_def.h"
#include "tcp_ip.h"
#include "rx_error.h"
#include "gui_svr.h"
#include "drive_ctrl.h"
#include "step_tts.h"
#include "ctrl_svr.h"

static void _do_fluid_tts(RX_SOCKET socket, SValue* msg);

#define STEPPER_CNT     3
#define JETTING_TIME    (8 * 60 * 60 * 1000) // ms
#define CHECKING_TIME   2000            // ms

static RX_SOCKET		_step_socket[STEPPER_CNT] = { 0 };
static SStepperStat		_status[STEPPER_CNT];
static int              _Jetting;

static int              _JettingTimer;

static void _control(void);

//--- steptest_init ---------------------------------------------------
void steptts_init(int no, RX_SOCKET psocket)
{
    if (no >= 0 && no < STEPPER_CNT)
    {
        _step_socket[no] = psocket;
    }
    memset(_status, 0, sizeof(_status));
}

//--- steptts_handle_status -----------------------------------
int steptts_handle_status(int no, SStepperStat* pStatus)
{
    ETestTableInfo info;
    EInkPumpInfo inkinfo;
    memcpy(&_status[no], pStatus, sizeof(_status[no]));

    inkinfo.waste_valve_0 = _status[1].inkinfo.waste_valve_0;
    inkinfo.waste_valve_1 = _status[1].inkinfo.waste_valve_1;
    inkinfo.waste_valve_2 = _status[2].inkinfo.waste_valve_2;
    inkinfo.waste_valve_3 = _status[2].inkinfo.waste_valve_3;
    inkinfo.ipa_valve_0 = _status[1].inkinfo.ipa_valve_0;
    inkinfo.ipa_valve_1 = _status[1].inkinfo.ipa_valve_1;
    inkinfo.ipa_valve_2 = _status[2].inkinfo.ipa_valve_2;
    inkinfo.ipa_valve_3 = _status[2].inkinfo.ipa_valve_3;
    inkinfo.xl_valve_0 = _status[1].inkinfo.xl_valve_0;
    inkinfo.xl_valve_1 = _status[1].inkinfo.xl_valve_1;
    inkinfo.xl_valve_2 = _status[2].inkinfo.xl_valve_2;
    inkinfo.xl_valve_3 = _status[2].inkinfo.xl_valve_3;
    inkinfo.flush_valve_0 = _status[1].inkinfo.flush_valve_0;
    inkinfo.flush_valve_1 = _status[1].inkinfo.flush_valve_1;
    inkinfo.flush_valve_2 = _status[2].inkinfo.flush_valve_2;
    inkinfo.flush_valve_3 = _status[2].inkinfo.flush_valve_3;


    memset(&info, 0, sizeof(info));
    info.ref_done = TRUE;
    info.z_in_ref = TRUE;
    info.z_in_print = TRUE;
    info.z_in_ref = _status[0].info.z_in_ref;
    info.z_in_print = _status[0].info.z_in_print;
    info.z_in_up = _status[0].info.z_in_up;
    info.z_in_cap = _status[0].info.z_in_cap;
    info.z_in_jet = _status[0].info.z_in_jet;
    info.ref_done = _status[0].info.ref_done;
    info.moving = _status[0].info.moving;
    RX_StepperStatus.posX = _status[0].posX;
    RX_StepperStatus.posY = _status[0].posY;
    RX_StepperStatus.posZ = _status[0].posZ;

    memcpy(&RX_StepperStatus.info, &info, sizeof(RX_StepperStatus.info));
    memcpy(&RX_StepperStatus.inkinfo, &inkinfo, sizeof(RX_StepperStatus.inkinfo));

    gui_send_msg_2(INVALID_SOCKET, REP_STEPPER_STAT, sizeof(RX_StepperStatus), &RX_StepperStatus);

    if (_Jetting)
    {
        _control();
    }

    return REPLY_OK;
}

//--- steptts_handle_gui_msg ------------------------------
int steptts_handle_gui_msg(RX_SOCKET socket, UINT32 cmd, void* data, int dataLen)
{
    int no;
    int pos;
    SValue value;
    for (no = 0; no < SIZEOF(_step_socket); no++)
    {
        if (_step_socket[no] != INVALID_SOCKET)
        {
            switch (cmd)
            {
            case CMD_TT_STOP:           sok_send_2(&_step_socket[no], cmd, 0, NULL);
                drive_stop_printing();
                break;
            case CMD_LIFT_UP_POS:
            case CMD_TT_VACUUM:         if (no == 0)    sok_send_2(&_step_socket[no], cmd, 0, NULL); break;
            case CMD_LIFT_PRINT_POS:    if (no == 0)    sok_send_2(&_step_socket[no], cmd, sizeof(RX_Config.stepper.print_height), &RX_Config.stepper.print_height); break;
            case CMD_TT_SCAN_RIGHT:
                drive_move_start();
                break;
            case CMD_TT_SCAN_LEFT:
                drive_move_back();
                break;
            case CMD_LIFT_REFERENCE:
                sok_send_2(&_step_socket[no], cmd, 0, NULL);
                drive_move_homing();
                break;
            case CMD_TT_SCAN_TRAY:
                drive_move_jet();
                break;

            case CMD_TT_SCAN_WASTE:
                drive_move_waste();
                break;

            case CMD_TT_MOVE_PURGE:
                drive_move_purge();
                break;

            case CMD_TT_MOVE_ADJUST:
                drive_move_table();
                break;

            case CMD_TTS_PUMP_PURGE:
                pos = *((INT32*)data);
                sok_send_2(&_step_socket[0], cmd, sizeof(pos), &pos);
                break;
            case CMD_FLUID_TTS:			_do_fluid_tts(socket, (SValue*)data);						break;
            case CMD_FLUID_FLUSH:
                sok_send_2(&_step_socket[0], cmd, sizeof(pos), &pos);
                value = *((SValue*)data);
                if (value.value == 0 ||
                    (value.value == 1 && RX_StepperStatus.inkinfo.flush_valve_0) ||
                    (value.value == 2 && RX_StepperStatus.inkinfo.flush_valve_1) ||
                    (value.value == 3 && RX_StepperStatus.inkinfo.flush_valve_2) ||
                    (value.value == 4 && RX_StepperStatus.inkinfo.flush_valve_3))
                    pos = FALSE;
                else
                    pos = TRUE;
                sok_send_2(&_step_socket[0], CMD_TTS_PUMP_PURGE, sizeof(pos), &pos);

                if (no == 1 || no == 2)
                    sok_send_2(&_step_socket[no], cmd, sizeof(value), &value);
                break;


            case CMD_TTS_JOG_FWD:
            case CMD_TTS_JOG_BWD:
            case CMD_TTS_JOG_STOP:
                if (no == 0) drive_jog_cmd(cmd);
                break;
            }
        }
    }

    return REPLY_OK;
}

//--- steptts_to_print_pos ---------------------------------
void steptts_to_print_pos(void)
{
    sok_send_2(&_step_socket[0], CMD_LIFT_PRINT_POS, sizeof(RX_Config.stepper.print_height), &RX_Config.stepper.print_height);
}

//--- steptts_to_top_pos --------------------------------------
void steptts_to_top_pos(void)
{
    sok_send_2(&_step_socket[0], CMD_LIFT_UP_POS, 0, NULL);
}

//--- steptts_in_top_pos ---------------------------------------
int steptts_in_top_pos(void)
{
    return RX_StepperStatus.info.z_in_ref || RX_StepperStatus.info.z_in_up;
}

//--- steptts_to_jet_pos -------------------------------------------
void steptts_to_jet_pos(void)
{
    sok_send_2(&_step_socket[0], CMD_LIFT_JET_TRAY, 0, NULL);
}

//--- steptts_in_jet_pos --------------------------------------------
int steptts_in_jet_pos(void)
{
    return RX_StepperStatus.info.z_in_jet;
}

//--- _do_fluid_tts -------------------------------------------
static void _do_fluid_tts(RX_SOCKET socket, SValue* msg)
{
    if (msg->no <= 1)
        sok_send_2(&_step_socket[1], CMD_FLUID_TTS, sizeof(*msg), msg);
    else
        sok_send_2(&_step_socket[2], CMD_FLUID_TTS, sizeof(*msg), msg);
}

//--- steptts_rob_stop ---------------------------------------
void steptts_rob_stop(void)
{
    int pos = FALSE;
    steptts_handle_gui_msg(INVALID_SOCKET, CMD_TTS_PUMP_PURGE, &pos, sizeof (pos));
}

//--- tts_error_reset ---------------------------------------------------------
void tts_error_reset(void)
{
    sok_send_2(&_step_socket[0], CMD_ERROR_RESET, 0, NULL);
}

//--- step_tts_abort_printing --------------------------------------------------
void step_tts_abort_printing(void)
{
    if (_Jetting)
    {
        ctrl_abort_printing();
        int tmp = FALSE;
        sok_send_2(&_step_socket[0], CMD_TTS_PUMP_JET_TRAY, sizeof(tmp), &tmp);
        steptts_to_top_pos();
    }
    _Jetting = 0;
}

//--- _control --------------------------------------------
static void _control(void)
{
    SValue value;
    static INT64 dropCnt_old[INK_SUPPLY_CNT][MAX_HEADS_BOARD] = { 0 };
    INT64 dropCnt_new[INK_SUPPLY_CNT][MAX_HEADS_BOARD] = { 0 };
    switch (_Jetting)
    {
    case 1:
        steptts_to_top_pos();
        _Jetting++;
        break;

    case 2:
        if (steptts_in_top_pos())
        {
            drive_move_jet();
            _Jetting++;
        }

        break;

    case 3:
        if (drive_in_jet())
        {
            steptts_to_jet_pos();
            int tmp = TRUE;
            sok_send_2(&_step_socket[0], CMD_TTS_PUMP_JET_TRAY, sizeof(tmp), &tmp);
            value.value = 2;
            if (!RX_StepperStatus.inkinfo.xl_valve_0)
            {
                value.no = 0;
                _do_fluid_tts(INVALID_SOCKET, &value);
            }
            if (!RX_StepperStatus.inkinfo.xl_valve_1)
            {
                value.no = 1;
                _do_fluid_tts(INVALID_SOCKET, &value);
            }
            if (!RX_StepperStatus.inkinfo.xl_valve_2)
            {
                value.no = 2;
                _do_fluid_tts(INVALID_SOCKET, &value);
            }
            if (!RX_StepperStatus.inkinfo.xl_valve_3)
            {
                value.no = 3;
                _do_fluid_tts(INVALID_SOCKET, &value);
            }
            _Jetting++;
        }
        break;

    case 4:
        if (RX_StepperStatus.inkinfo.xl_valve_0 && RX_StepperStatus.inkinfo.xl_valve_1 &&
            RX_StepperStatus.inkinfo.xl_valve_2 && RX_StepperStatus.inkinfo.xl_valve_3)
        {
            value.value = 3;
            if (RX_StepperStatus.inkinfo.waste_valve_0)
            {
                value.no = 0;
                _do_fluid_tts(INVALID_SOCKET, &value);
            }
            if (RX_StepperStatus.inkinfo.waste_valve_1)
            {
                value.no = 1;
                _do_fluid_tts(INVALID_SOCKET, &value);
            }
            if (RX_StepperStatus.inkinfo.waste_valve_2)
            {
                value.no = 2;
                _do_fluid_tts(INVALID_SOCKET, &value);
            }
            if (RX_StepperStatus.inkinfo.waste_valve_3)
            {
                value.no = 3;
                _do_fluid_tts(INVALID_SOCKET, &value);
            }
            _Jetting++;
            _JettingTimer = 0;
        }
        break;

    case 5:
        if (steptts_in_jet_pos() && !RX_StepperStatus.inkinfo.waste_valve_0 && !RX_StepperStatus.inkinfo.waste_valve_1
            && !RX_StepperStatus.inkinfo.waste_valve_2 && !RX_StepperStatus.inkinfo.waste_valve_3)
        {
            ctrl_set_jetting();
            _Jetting++;
            _JettingTimer = rx_get_ticks() + JETTING_TIME;
        }
        break;

    case 6:
        if (rx_get_ticks() >= _JettingTimer && _JettingTimer)
        {
            ctrl_abort_printing();
            _Jetting++;
            _JettingTimer = 0;
        }
        break;

    case 7:
        for (int i = 0; i < INK_SUPPLY_CNT; i++)
        {
            for (int j = 0; j < MAX_HEADS_BOARD; j++)
            {
                if (!_JettingTimer)
                {
                    dropCnt_old[i][j] += RX_HBStatus[i].head[j].dotCnt;
                    _JettingTimer = rx_get_ticks() + CHECKING_TIME;
                }
                else
                    dropCnt_new[i][j] += RX_HBStatus[i].head[j].dotCnt;

                if (dropCnt_new[i][j] != dropCnt_old[i][j])
                {
                    _JettingTimer = rx_get_ticks() + CHECKING_TIME;
                    dropCnt_old[i][j] = dropCnt_new[i][j];
                }
            }
        }

        if (rx_get_ticks() >= _JettingTimer && _JettingTimer)
        {
            steptts_to_top_pos();
            int tmp = FALSE;
            sok_send_2(&_step_socket[0], CMD_TTS_PUMP_JET_TRAY, sizeof(tmp), &tmp);
            _Jetting = 0;
        }
        break;
    }
}

//--- step_tts_jetting_start --------------------------------
void step_tts_jetting_start(void)
{
    if (!_Jetting) _Jetting = 1;
}