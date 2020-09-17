// ****************************************************************************
//
//	DIGITAL PRINTING - tts_lift.c
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
#include "rx_term.h"
#include "fpga_stepper.h"
#include "rx_sok.h"
#include "motor.h"
#include "rx_error.h"
#include "stepper_ctrl.h"
#include "tts_lift.h"

// motors
#define MOTOR_Z_0       0
#define MOTOR_Z_1       1
#define MOTOR_Z_CNT     2
#define MOTOR_Z_BITS    0x03

#define CURRENT_HOLD    50              // *0.01A

// digital Inputs
#define HEAD_UP_IN_0    0
#define HEAD_UP_IN_1    1
#define SCANNER_REF     4

// digital Outputs
#define RO_VACUUM_MOTOR         0x001;       // output 0
#define RO_PURGE_TUBE_EMPTY      0x010;       // output 4
#define RO_JET_TRAY_EMPTY        0x020;       // output 5
#define RO_JET_TRAY_VALVE        0x040;       // output 6
#define RO_BREAK_FRONT          0x080;       // output 7
#define RO_BREAK_BACK           0x100;       // output 8
#define RO_SCANNER_END_FRONT    0x200;       // output 9
//#define RO_SCANNER_END_RIGHT    0x400;       // output 10
#define RO_CLEAR_ERROR          0x400;       // output 10

#define TTS_REF_HEIGHT  12800            // um

#define STEPS_REV       (200*16)        // microsteps
#define DIST_REV        5000            // um/rev

#define TTS_PRINT_POS_MIN   1200        // um

#define POS_UP			11800            // um
#define POS_CAP         7000            // um
#define POS_JET_TRAY    -2300            // um

#define CLEAR_ERROR_TIME    1000        // ms
#define PURGE_PUMP_TIME     10000       // ms

#define INCS_REV         20000        // cpr
//#define STEPS_REV       3200
#define DIST_REV        5000

static SMovePar _ParRef;
static SMovePar _ParZ_down;
static SMovePar _ParZ_cap;
static int      _PrintPos_New = 0;
static int      _PrintPos_Act = 0;
static int      _CapHight = 2000;
static int      _Vacuum_State = FALSE;
static int      _Vacuum_State_old = FALSE;
static int      _Clear_Error = FALSE;
static int      _Clear_Error_Time = 0;
static int      _NewCmd = 0;
static int      _PurgePump_Time = 0;


static void _tts_lift_display_status(void);
static void _tts_lift_motor_test(int motorNo, int steps);
static int  _steps_2_micron(int steps);
static int  _micron_2_steps(int micron);
static char *_motor_name(int no);
static void _tts_lift_move_to_pos(int cmd, int pos);
static void _tts_lift_do_reference(void);

//--- tts_lift_init --------------------------------------
void tts_lift_init(void)
{
    motors_config(MOTOR_Z_BITS, CURRENT_HOLD, STEPS_REV, INCS_REV, STEPS);

    _ParRef.speed           = 1000;
    _ParRef.accel           = 1000;
    _ParRef.current_acc     = 200.0;
    _ParRef.current_run     = 150.0;
    _ParRef.estop_in_bit[0] = (1 << HEAD_UP_IN_0);
    _ParRef.estop_in_bit[1] = (1 << HEAD_UP_IN_1);
    _ParRef.estop_level     = 1;
    _ParRef.stop_mux        = MOTOR_Z_BITS;
    _ParRef.dis_mux_in      = TRUE;
    _ParRef.encCheck        = chk_tts;
    
    _ParZ_down.speed		= 2000;
	_ParZ_down.accel		= 1000;
	_ParZ_down.current_acc	= 420.0;
	_ParZ_down.current_run	= 420.0;
	_ParZ_down.stop_mux		= MOTOR_Z_BITS;
	_ParZ_down.dis_mux_in	= FALSE;
	_ParZ_down.encCheck		= chk_std;
    
    _ParZ_cap.speed			= 1000;
	_ParZ_cap.accel			= 1000;
	_ParZ_cap.current_acc	= 420.0;
	_ParZ_cap.current_run	= 420.0;
	_ParZ_cap.stop_mux		= FALSE;
	_ParZ_cap.dis_mux_in	= FALSE;
	_ParZ_cap.encCheck		= chk_std;
}

//--- tts_lift_main ----------------------------------------
void tts_lift_main(int ticks, int menu)
{
    
    int motor, ok;
    motor_main(ticks, menu);

    if (fpga_input(SCANNER_REF))
    {
        Fpga.par->output |= RO_SCANNER_END_FRONT;
    }
    else
    {
        Fpga.par->output &= ~RO_SCANNER_END_FRONT;
    }


    if (_Clear_Error && _Clear_Error_Time + CLEAR_ERROR_TIME > rx_get_ticks())
    {
        Fpga.par->output |= RO_CLEAR_ERROR;
        _Clear_Error = FALSE;
    }
    else if (_Clear_Error_Time && (rx_get_ticks() > _Clear_Error_Time + CLEAR_ERROR_TIME))
    {
        Fpga.par->output &= ~RO_CLEAR_ERROR;
        _Clear_Error_Time = 0;
        motor_errors_reset();
    }

    if (_PurgePump_Time && (rx_get_ticks() > _PurgePump_Time + PURGE_PUMP_TIME))
    {
        Fpga.par->output &= ~RO_PURGE_TUBE_EMPTY;
        _PurgePump_Time = 0;
    }

    RX_StepperStatus.info.headUpInput_0 = fpga_input(HEAD_UP_IN_0);
    RX_StepperStatus.info.headUpInput_1 = fpga_input(HEAD_UP_IN_1);
    RX_StepperStatus.posZ               = TTS_REF_HEIGHT - _steps_2_micron(motor_get_step(MOTOR_Z_0));
    RX_StepperStatus.info.scannerEnable = RX_StepperStatus.info.ref_done && RX_StepperStatus.posZ >= TTS_PRINT_POS_MIN;

    
    if (RX_StepperStatus.info.moving)
    {
        RX_StepperStatus.info.z_in_ref      = FALSE;
        RX_StepperStatus.info.z_in_up       = FALSE;
        RX_StepperStatus.info.z_in_print    = FALSE;
        RX_StepperStatus.info.z_in_cap      = FALSE;
        RX_StepperStatus.info.z_in_jet      = FALSE;
    }

    if (_Vacuum_State != _Vacuum_State_old && _Vacuum_State)
    {
        Fpga.par->output |= RO_VACUUM_MOTOR;
        _Vacuum_State_old = _Vacuum_State;
    }
    else if (_Vacuum_State_old != _Vacuum_State && _Vacuum_State_old)
    {
       Fpga.par->output &= ~RO_VACUUM_MOTOR;
       _Vacuum_State_old = _Vacuum_State;
    }


    if (RX_StepperStatus.cmdRunning && motors_move_done(MOTOR_Z_BITS))
    {
        RX_StepperStatus.info.moving = FALSE;
        if (RX_StepperStatus.cmdRunning == CMD_LIFT_REFERENCE)
        {
            for (motor = 0, ok = TRUE; motor < MOTOR_Z_CNT; motor++)
            {
                if ((Fpga.stat->statMot[motor].err_estop & ENC_ESTOP_ENC))
                {
                    if (!fpga_input(motor))
                    {
                        Error(ERR_CONT, 0, "LIFT: %s: motor %s blocked", ctrl_cmd_name(RX_StepperStatus.cmdRunning), _motor_name(motor));
                        ok = FALSE;
                        _NewCmd = FALSE;
                    }
                }
            }
            motors_reset(MOTOR_Z_BITS);
            RX_StepperStatus.info.ref_done = ok;
        }
        else
        {
            for (motor = 0, ok = TRUE; motor < MOTOR_Z_CNT; motor++)
            {
                if (motor_error(motor))
                {
                    Error(ERR_CONT, 0, "tts_lift: %s: motor %s blocked", ctrl_cmd_name(RX_StepperStatus.cmdRunning), _motor_name(motor));
                    ok = FALSE;
                    _NewCmd = FALSE;
                }
            }
            if (!ok) RX_StepperStatus.info.ref_done = FALSE;
        }

        RX_StepperStatus.info.z_in_ref =
            (RX_StepperStatus.cmdRunning == CMD_LIFT_REFERENCE &&
             RX_StepperStatus.info.ref_done);
        RX_StepperStatus.info.z_in_up =
            (RX_StepperStatus.cmdRunning == CMD_LIFT_UP_POS &&
             RX_StepperStatus.info.ref_done);
        RX_StepperStatus.info.z_in_print =
            (RX_StepperStatus.cmdRunning == CMD_LIFT_PRINT_POS &&
             RX_StepperStatus.info.ref_done);
        RX_StepperStatus.info.z_in_cap =
            (RX_StepperStatus.cmdRunning == CMD_LIFT_CAPPING_POS &&
             RX_StepperStatus.info.ref_done);
        RX_StepperStatus.info.z_in_jet =
            (RX_StepperStatus.cmdRunning == CMD_LIFT_JET_TRAY &&
             RX_StepperStatus.info.ref_done);

        if (RX_StepperStatus.cmdRunning == CMD_LIFT_REFERENCE &&
            _PrintPos_New && RX_StepperStatus.info.ref_done)
        {
            _tts_lift_move_to_pos(CMD_LIFT_PRINT_POS, _PrintPos_New);
            _PrintPos_Act = _PrintPos_New;
            _PrintPos_New = 0;
        }
        else if (_NewCmd)
        {
            int local_new_cmd;
            local_new_cmd = _NewCmd;
            _NewCmd = 0;
            switch (local_new_cmd)
            {
            case CMD_LIFT_JET_TRAY:
                RX_StepperStatus.cmdRunning = FALSE;
                tts_lift_handle_ctrl_msg(INVALID_SOCKET, CMD_LIFT_JET_TRAY, NULL);
                break;
            default:    Error(ERR_CONT, 0, "Command %08x not implemented", local_new_cmd);
                break;
            }
        }
        else
        {
            RX_StepperStatus.cmdRunning = FALSE;
        }
        _NewCmd = FALSE;
    }
}

//--- tts_lift_menu ---------------------------------------
int tts_lift_menu(void)
{
    char str[MAX_PATH];
    int synth = FALSE;
    static int cnt = 0;
    int i;
    int pos = 10000;

    _tts_lift_display_status();

    term_printf("MENU TEST TABLE SEON LIFT -------------------------\n");
    term_printf("o: toggle output <no>\n");
    term_printf("s: STOP\n");
    term_printf("r<n>: reset motor<n>\n");
    term_printf("R: Reference\n");
    term_printf("u: move to UP position\n");
    term_printf("p<n>: move to print Height <n>um\n");
    term_printf("m<n><steps>: move Motor<n> by <steps>\n");
    term_printf("j: move to jet tray hight\n");
    term_printf("x: exit\n");
    term_printf(">");
    term_flush();

    if (term_get_str(str, sizeof(str)))
    {
        switch (str[0])
        {
        case 'o':
            Fpga.par->output ^= (1 << atoi(&str[1]));
            break;
        case 's':
            tts_lift_handle_ctrl_msg(INVALID_SOCKET, CMD_LIFT_STOP, NULL);
            break;
        case 'r':
            if (str[1] == 0)
                for (i = 0; i < MOTOR_CNT; i++) motor_reset(i);
            else
                motor_reset(atoi(&str[1]));
            break;
        case 'm':
            _tts_lift_motor_test(str[1] - '0', atoi(&str[2]));
            break;
        case 'R':
            tts_lift_handle_ctrl_msg(INVALID_SOCKET, CMD_LIFT_REFERENCE, NULL);
            break;
        case 'u':
            tts_lift_handle_ctrl_msg(INVALID_SOCKET, CMD_LIFT_UP_POS, NULL);
            break;
        case 'p':
            pos = atoi(&str[1]);
            tts_lift_handle_ctrl_msg(INVALID_SOCKET, CMD_LIFT_PRINT_POS, &pos);
            break;
        case'j':
            tts_lift_handle_ctrl_msg(INVALID_SOCKET, CMD_LIFT_JET_TRAY, NULL);
            break;
        case 'x':
            return FALSE;
        }
    }
    return TRUE;
}

//--- tts_lift_handle_ctrl_msg ------------------------------------------
int tts_lift_handle_ctrl_msg(RX_SOCKET socket, int msgId, void *pdata)
{
    INT32 pos, steps;
    
    switch (msgId)
    {
    case CMD_LIFT_STOP: 
        motors_stop(MOTOR_Z_BITS);
        RX_StepperStatus.cmdRunning = 0;
        Fpga.par->output &= ~RO_BREAK_FRONT;
        Fpga.par->output &= ~RO_BREAK_BACK;
        break;
        
    case CMD_LIFT_REFERENCE:
        _PrintPos_New = 0;
        motors_reset(MOTOR_Z_BITS);
        _tts_lift_do_reference();
        break;
        
    case CMD_LIFT_PRINT_POS:
        pos = *((INT32*)pdata);
        if (pos < TTS_PRINT_POS_MIN)
        {
            pos = TTS_PRINT_POS_MIN;
            Error(WARN, 0, "%s: set to MIN pos %d.%03d mm",
                  ctrl_cmd_name(msgId), pos / 1000,
                  pos % 1000);
        }
        else if (pos > POS_UP)
        {
            pos = POS_UP;
            Error(WARN, 0, "%s: set to MAX pos %d.%03d mm",
                  ctrl_cmd_name(msgId), pos / 1000,
                  pos % 1000);
        }

        if (!RX_StepperStatus.cmdRunning &&
            (!RX_StepperStatus.info.z_in_print || STEPS != _PrintPos_Act))
        {
            _PrintPos_New = _micron_2_steps(TTS_REF_HEIGHT - pos);
            if (RX_StepperStatus.info.ref_done)
                _tts_lift_move_to_pos(msgId, _PrintPos_New);
            else
                _tts_lift_do_reference();
        }
        break;
        
    case CMD_LIFT_UP_POS:
        if (!RX_StepperStatus.cmdRunning)
        {
            if (RX_StepperStatus.info.ref_done)
            {
                if (!RX_StepperStatus.info.z_in_up)
                {
                    pos = _micron_2_steps(TTS_REF_HEIGHT - POS_UP);
                    _tts_lift_move_to_pos(msgId, pos);
                }
                
            }
            else
                _tts_lift_do_reference();
        }
        break;
        
    case CMD_LIFT_CAPPING_POS:
        if (!RX_StepperStatus.cmdRunning)
		{
            if (RX_StepperStatus.info.ref_done)
                _tts_lift_move_to_pos(msgId, POS_CAP);
            else
                _tts_lift_do_reference();
		}
		break;
        
    case CMD_LIFT_JET_TRAY:
        if (!RX_StepperStatus.cmdRunning)
        {
            if (RX_StepperStatus.info.ref_done)
            {
                steps = _micron_2_steps(TTS_REF_HEIGHT - POS_JET_TRAY);
                _tts_lift_move_to_pos(msgId, steps);
            }
            else
            {
                _NewCmd = msgId;
                _tts_lift_do_reference();
            }
            
        }
        break;

    case CMD_TT_VACUUM:
        _Vacuum_State = !_Vacuum_State;
        break;
        
    case CMD_TTS_PUMP_JET_TRAY:
        pos = (*(INT32 *)pdata);
        if (pos == TRUE)
        {
            Fpga.par->output |= RO_JET_TRAY_VALVE;
            Fpga.par->output |= RO_JET_TRAY_EMPTY;
        }
        else
        {
            Fpga.par->output &= ~RO_JET_TRAY_VALVE;
            Fpga.par->output &= ~RO_JET_TRAY_EMPTY;
        }
        break;
        
    case CMD_TTS_PUMP_PURGE:
        pos = (*(INT32 *)pdata);
        if (pos == TRUE)
        {
            _PurgePump_Time = 0;
            Fpga.par->output |= RO_PURGE_TUBE_EMPTY;
        }
        else
        {
            _PurgePump_Time = rx_get_ticks();
        }
        break;

    case CMD_ERROR_RESET:
        _Clear_Error = TRUE;
        _Clear_Error_Time = rx_get_ticks();
        //motor_errors_reset();

    default:
        break;
    }
    return REPLY_OK;
}

//--- _tts_lift_display_status ------------------------------------------
static void _tts_lift_display_status(void)
{
    term_printf("TTS ---------------------------------------------");
    term_printf("moving:         %d		cmd: %08x\n",	RX_StepperStatus.info.moving, RX_StepperStatus.cmdRunning);
    term_printf("Head UP Sensor: %d%d\n",	fpga_input(HEAD_UP_IN_0), fpga_input(HEAD_UP_IN_1));
    term_printf("reference done: %d\n",	RX_StepperStatus.info.ref_done);
    term_printf("z in ref      : %d\n", RX_StepperStatus.info.z_in_ref);
    term_printf("z in up       : %d\n", RX_StepperStatus.info.z_in_up);
    term_printf("z in print    : %d\n", RX_StepperStatus.info.z_in_print);
    term_printf("z in cap      : %d\n", RX_StepperStatus.info.z_in_cap);
    term_printf("z in jet      : %d\n", RX_StepperStatus.info.z_in_jet);
    term_printf("z position in micron:   %d\n", RX_StepperStatus.posZ);
    term_printf("\n");
}

//--- _tts_lift_motor_test ------------------------------------------
static void _tts_lift_motor_test(int motorNo, int steps)
{
    int motors = 1 << motorNo;
    SMovePar par;
    int i;

    memset(&par, 0, sizeof(par));
    par.speed = 5000;
    par.accel = 32000;
    par.current_acc = 420.0;
    par.current_run = 420.0;
    par.stop_mux = 0;
    par.dis_mux_in = 0;
    par.encCheck = chk_off;
    RX_StepperStatus.info.moving = TRUE;

    //	motors_config(motors,  CURRENT_HOLD, 0.0, 0.0);
    motors_config(MOTOR_Z_BITS, CURRENT_HOLD, STEPS_REV, INCS_REV, STEPS);
    motors_move_by_step(motors, &par, steps, FALSE);
    
}

static int _steps_2_micron(int steps)
{
    return (int)(DIST_REV * steps / STEPS_REV + 0.5);
}

//---_micron_2_steps --------------------------------------------------------------
static int  _micron_2_steps(int micron)
{
	return (int)(0.5+(micron *STEPS_REV)/DIST_REV);
}

//--- _motor_name ------------------------------
static char *_motor_name(int no)
{
    switch (no)
    {
    case 0:
        return "Right";
    case 1:
        return "Left";
    default:
        return "";
    }
}

//--- _tts_lift_move_to_pos ---------------------------------------------------------------
static void _tts_lift_move_to_pos(int cmd, int pos)
{
	RX_StepperStatus.cmdRunning  = cmd;
	RX_StepperStatus.info.moving = TRUE;
    Fpga.par->output |= RO_BREAK_FRONT;
    Fpga.par->output |= RO_BREAK_BACK;
	motors_move_to_step(MOTOR_Z_BITS, &_ParZ_down, pos);
}



//--- _tts_lift_do_reference ----------------------------------------------------------------
static void _tts_lift_do_reference(void)
{
	motors_stop	(MOTOR_Z_BITS);
    motors_config(MOTOR_Z_BITS, CURRENT_HOLD, STEPS_REV, INCS_REV, STEPS);

	RX_StepperStatus.cmdRunning  = CMD_LIFT_REFERENCE;
	RX_StepperStatus.info.moving = TRUE;
    Fpga.par->output |= RO_BREAK_FRONT;
    Fpga.par->output |= RO_BREAK_BACK;
	motors_move_by_step	(MOTOR_Z_BITS,  &_ParRef, -100000, TRUE);
}

//--- _incs_2_micron ----------------------------
static int _incs_2_micron(int incs)
{
    return (int)(DIST_REV * incs / INCS_REV + 0.5);
}
