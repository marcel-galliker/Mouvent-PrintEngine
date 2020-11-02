// ****************************************************************************
//
//	DIGITAL PRINTING - tts_ink.c
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
#include "motor.h"
#include "tts_ink.h"
#include "rx_error.h"

#define RO_FLUSH_C1         0x00000001;
#define RO_FLUSH_C2         0x00000002;
#define RO_XL_C1_INPUT      0x00000010;
#define RO_IPA_C1_INPUT     0x00000020;
#define RO_IPA_C1_OUTPUT    0x00000040;
#define RO_XL_C1_OUTPUT     0x00000080;
#define RO_XL_C2_INPUT      0x00000100;
#define RO_IPA_C2_INPUT     0x00000200;
#define RO_IPA_C2_OUTPUT    0x00000400;
#define RO_XL_C2_OUTPUT     0x00000800;

#define RO_C1_ALL 0x0f1;
#define RO_C2_ALL 0xf02;

static int _RO_FLUSH_C1     = RO_FLUSH_C1;
static int _RO_FLUSH_C2     = RO_FLUSH_C2;
static int _RO_XL_C1_IN     = RO_XL_C1_INPUT;
static int _RO_IPA_C1_IN    = RO_IPA_C1_INPUT;
static int _RO_XL_C1_OUT    = RO_XL_C1_OUTPUT;
static int _RO_IPA_C1_OUT   = RO_IPA_C1_OUTPUT;
static int _RO_XL_C2_IN     = RO_XL_C2_INPUT;
static int _RO_IPA_C2_IN    = RO_IPA_C2_INPUT;
static int _RO_XL_C2_OUT    = RO_XL_C2_OUTPUT;
static int _RO_IPA_C2_OUT   = RO_IPA_C2_OUTPUT;

static int _Cluster1_Valve_Input;
static int _Cluster2_Valve_Input;
static void _tts_ink_display_status(void);
static void _tts_ink_motor_test(int motorNo, int steps);

//--- tts_valve_init --------------------------------------
void tts_ink_init(void)
{
}

//--- tts_valve_main ----------------------------------------
void tts_ink_main(int ticks, int menu)
{
    SStepperStat oldSatus;
    memcpy(&oldSatus, &RX_StepperStatus, sizeof(RX_StepperStatus));
    memset(&RX_StepperStatus.inkinfo, 0, sizeof(RX_StepperStatus.inkinfo));

    if (RX_StepperCfg.boardNo == 1)
    {
        if (!(Fpga.par->output & _RO_IPA_C1_OUT || Fpga.par->output & _RO_XL_C1_OUT))
            RX_StepperStatus.inkinfo.waste_valve_0 = TRUE;
        if (!(Fpga.par->output & _RO_IPA_C2_OUT || Fpga.par->output & _RO_XL_C2_OUT))
            RX_StepperStatus.inkinfo.waste_valve_1 = TRUE;
        if (Fpga.par->output & _RO_IPA_C1_IN)
            RX_StepperStatus.inkinfo.ipa_valve_0 = TRUE;
        if (Fpga.par->output & _RO_IPA_C2_IN)
            RX_StepperStatus.inkinfo.ipa_valve_1 = TRUE;
        if (Fpga.par->output & _RO_XL_C1_IN)
            RX_StepperStatus.inkinfo.xl_valve_0 = TRUE;
        if (Fpga.par->output & _RO_XL_C2_IN)
            RX_StepperStatus.inkinfo.xl_valve_1 = TRUE;
        if (Fpga.par->output & _RO_FLUSH_C1)
            RX_StepperStatus.inkinfo.flush_valve_0 = TRUE;
        if (Fpga.par->output & _RO_FLUSH_C2)
            RX_StepperStatus.inkinfo.flush_valve_0 = TRUE;
    }
    else if (RX_StepperCfg.boardNo == 2)
    {
        if (!(Fpga.par->output & _RO_IPA_C1_OUT || Fpga.par->output & _RO_XL_C1_OUT))
            RX_StepperStatus.inkinfo.waste_valve_2 = TRUE;
        if (!(Fpga.par->output & _RO_IPA_C2_OUT || Fpga.par->output & _RO_XL_C2_OUT))
            RX_StepperStatus.inkinfo.waste_valve_3 = TRUE;
        if (Fpga.par->output & _RO_IPA_C1_IN)
            RX_StepperStatus.inkinfo.ipa_valve_2 = TRUE;
        if (Fpga.par->output & _RO_IPA_C2_IN)
            RX_StepperStatus.inkinfo.ipa_valve_3 = TRUE;
        if (Fpga.par->output & _RO_XL_C1_IN)
            RX_StepperStatus.inkinfo.xl_valve_2 = TRUE;
        if (Fpga.par->output & _RO_XL_C2_IN)
            RX_StepperStatus.inkinfo.xl_valve_3 = TRUE;
        if (Fpga.par->output & _RO_FLUSH_C1)
            RX_StepperStatus.inkinfo.flush_valve_2 = TRUE;
        if (Fpga.par->output & _RO_FLUSH_C2)
            RX_StepperStatus.inkinfo.flush_valve_3 = TRUE;
        
    }
    
    if (memcmp(&oldSatus.inkinfo, &RX_StepperStatus.inkinfo, sizeof(RX_StepperStatus.inkinfo)))
    {
        ctrl_send_2(REP_STEPPER_STAT, sizeof(RX_StepperStatus), &RX_StepperStatus);
    }
}

//--- tts_valve_menu ---------------------------------------
int tts_ink_menu(void)
{
    char str[MAX_PATH];
    int synth = FALSE;
    static int cnt = 0;
    int i;
    int pos = 10000;

    _tts_ink_display_status();

    term_printf("MENU TEST TABLE SEON INK -------------------------\n");
    term_printf("o: toggle output <no>\n");
    term_printf("s: STOP\n");
    term_printf("r<n>: reset motor<n>\n");
    term_printf("m<n><steps>: move Motor<n> by <steps>\n");
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
            tts_ink_handle_ctrl_msg(INVALID_SOCKET, CMD_LIFT_STOP, NULL);
            break;
        case 'r':
            if (str[1] == 0)
                for (i = 0; i < MOTOR_CNT; i++) motor_reset(i);
            else
                motor_reset(atoi(&str[1]));
            break;
        case 'm':
            _tts_ink_motor_test(str[1] - '0', atoi(&str[2]));
            break;
        case 'x':
            return FALSE;
        }
    }
    return TRUE;
}

//--- tts_valve_handle_ctrl_msg ------------------------------------------
int tts_ink_handle_ctrl_msg(RX_SOCKET socket, int msgId, void *pdata)
{
    INT32 pos;
    UINT32 test = Fpga.par->output;
    SValue value;

    switch (msgId)
    {
    case CMD_FLUID_TTS:
        value = *((SValue *)pdata);
        switch (value.no)
        {
            case 0:
            case 2:
                switch (value.value)
                {
                case 0:
                    Fpga.par->output &= ~RO_IPA_C1_INPUT;
                    Fpga.par->output &= ~RO_XL_C1_INPUT;
                    Fpga.par->output &= ~RO_IPA_C1_OUTPUT;
                    Fpga.par->output &= ~RO_XL_C1_OUTPUT;
                    break;
                case 1:
                    if (!(Fpga.par->output & _RO_IPA_C1_IN))
                    {
                        Fpga.par->output |= RO_IPA_C1_INPUT;
                        Fpga.par->output &= ~RO_XL_C1_INPUT;
                        Fpga.par->output &= ~RO_IPA_C1_OUTPUT;
                        Fpga.par->output &= ~RO_XL_C1_OUTPUT; 
                    }
                    break;
                case 2:
                    if (!(Fpga.par->output & _RO_XL_C1_IN))
                    {
                        Fpga.par->output &= ~RO_IPA_C1_INPUT;
                        Fpga.par->output |= RO_XL_C1_INPUT;
                        Fpga.par->output &= ~RO_IPA_C1_OUTPUT;
                        Fpga.par->output &= ~RO_XL_C1_OUTPUT;
                    }
                    break;
                case 3:
                    if (Fpga.par->output & _RO_IPA_C1_IN)
                    {
                        Fpga.par->output ^= RO_IPA_C1_OUTPUT;
                        Fpga.par->output &= ~RO_XL_C1_OUTPUT;
                    }
                    else if (Fpga.par->output & _RO_XL_C1_IN)
                    {
                        Fpga.par->output &= ~RO_IPA_C1_OUTPUT;
                        Fpga.par->output ^= RO_XL_C1_OUTPUT;
                    }
                    else
                    {
                        Fpga.par->output &= ~RO_IPA_C1_OUTPUT;
                        Fpga.par->output &= ~RO_XL_C1_OUTPUT;
                    }
                    break;
                default: Error(ERR_CONT, 0, "Unknown command %d for Fluid System", value.value);
                    break;
                }
                break;
            case 1:
            case 3:
                switch (value.value)
                {
                case 0:
                    Fpga.par->output &= ~RO_IPA_C2_INPUT;
                    Fpga.par->output &= ~RO_XL_C2_INPUT;
                    Fpga.par->output &= ~RO_IPA_C2_OUTPUT;
                    Fpga.par->output &= ~RO_XL_C2_OUTPUT;
                    break;
                case 1:
                    if (!(Fpga.par->output & _RO_IPA_C2_IN))
                    {
                        Fpga.par->output |= RO_IPA_C2_INPUT;
                        Fpga.par->output &= ~RO_XL_C2_INPUT;
                        Fpga.par->output &= ~RO_IPA_C2_OUTPUT;
                        Fpga.par->output &= ~RO_XL_C2_OUTPUT;
                    }
                    break;
                case 2:
                    if (!(Fpga.par->output & _RO_XL_C2_IN))
                    {
                        Fpga.par->output &= ~RO_IPA_C2_INPUT;
                        Fpga.par->output |= RO_XL_C2_INPUT;
                        Fpga.par->output &= ~RO_IPA_C2_OUTPUT;
                        Fpga.par->output &= ~RO_XL_C2_OUTPUT;
                    }
                    break;
                case 3:
                    if (Fpga.par->output & _RO_IPA_C2_IN)
                    {
                        Fpga.par->output ^= RO_IPA_C2_OUTPUT;
                        Fpga.par->output &= ~RO_XL_C2_OUTPUT;
                    }
                    else if (Fpga.par->output & _RO_XL_C2_IN)
                    {
                        Fpga.par->output &= ~RO_IPA_C2_OUTPUT;
                        Fpga.par->output ^= RO_XL_C2_OUTPUT;
                    }
                    else
                    {
                        Fpga.par->output &= ~RO_IPA_C2_OUTPUT;
                        Fpga.par->output &= ~RO_XL_C2_OUTPUT;
                    }
                    break;
                default: Error(ERR_CONT, 0, "Unknown command %d for Fluid System", value.value);
                    break;
                }
                break;
            default:
                Error(ERR_CONT, 0, "Unknown number %d for Fluid System", value.no);
        }
        break;
        
    case CMD_FLUID_FLUSH:
        value = *((SValue *)pdata);
        switch (value.value)
        {
        case 0:
            Fpga.par->output &= ~RO_FLUSH_C1;
            Fpga.par->output &= ~RO_FLUSH_C2;
            break;
        case 1:
        case 3:
            if (!(Fpga.par->output & _RO_FLUSH_C1) && ((value.value == 1 && RX_StepperCfg.boardNo == 1) || (value.value == 3 && RX_StepperCfg.boardNo == 2)))
            {
                Fpga.par->output |= RO_FLUSH_C1;
            }
            else
            {
                Fpga.par->output &= ~RO_FLUSH_C1;
            }
            
            break;
            
        case 2:
        case 4:
            if (!(Fpga.par->output & _RO_FLUSH_C2) && ((value.value == 2 && RX_StepperCfg.boardNo == 1) || (value.value == 4 && RX_StepperCfg.boardNo == 2)))
            {
                Fpga.par->output |= RO_FLUSH_C2;
            }
            else
            {
                Fpga.par->output &= ~RO_FLUSH_C2;
            }
            break;
        }
        break;
    }
    return REPLY_OK;
}

//--- _tts_valve_display_status ------------------------------------------
static void _tts_ink_display_status(void)
{
}

//--- _tts_valve_motor_test ------------------------------------------
static void _tts_ink_motor_test(int motorNo, int steps)
{
    int motors = 1 << motorNo;
    SMovePar par;
    int i;

    memset(&par, 0, sizeof(par));
    par.speed = 5000;
    par.accel = 32000;
    par.current_acc = 100.0;
    par.current_run = 100.0;
    par.stop_mux = 0;
    par.dis_mux_in = 0;
    par.encCheck = chk_std;
    RX_StepperStatus.info.moving = TRUE;

    //	motors_config(motors,  CURRENT_HOLD, 0.0, 0.0);
    motors_config(motors, 0, L3518_STEPS_PER_METER, L3518_INC_PER_METER, STEPS);
    motors_move_by_step(motors, &par, steps, FALSE);
}