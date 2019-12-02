// ****************************************************************************
//
//	DIGITAL PRINTING - lbrob_rob.c
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

//--- defines ------------------------------------------------
#define CURRENT_HOLD	   10.0 // 200 steps/rev // 2 mm/rev

#define MOTOR_CAP			0

#define MOTOR_CAP_BITS			0x01
#define MOTOR_ALL_BITS			0x01

#define CAP_STEPS_PER_METER		385852.0 // 200 steps *16 * 40 / U // 1:40
#define CAP_INC_PER_METER		482315.0 // 385852.0 / 0.8
//#define CAP_ENC_RATIO			0.8 //

// Digital Inputs (max 12)
#define RO_STORED_IN		0
//#define RO_CAPPING_IN		1

// Digital Outputs (max 12)
#define RO_CAPP_INFLATE		0x001
#define RO_BLADE_UP_0		0x008
#define RO_BLADE_UP_1		0x010
#define RO_VACUUM_0			0x020
#define RO_VACUUM_1			0x040
#define RO_ALL_OUT			0x07F
// Flush?
//	Fpga.par->output |= Output; // Set Fpga.par->output |= RO_CAPP_INFLATE;
//	Fpga.par->output &= ~Output; // Release

// Analog Inputs (max 4)

#define POS_STORED_UM			0 // Ref
#define POS_PRINTHEADS_UM	RX_StepperCfg.cap_pos // capping position max distance 285'000 steps

static SMovePar	_ParCap_ref;
static SMovePar	_ParCap_drive;
static SMovePar	_ParCap_wipe;

static int		_CmdRunning = 0;
static int		_lastPosCmd = 0;
static int		_reset_retried = 0;
static int		_new_cmd = FALSE;

static UINT32	_lbrob_Error[5];

//--- prototypes --------------------------------------------
static void _lbrob_ro_motor_test(int motor, int steps);
static void _lbrob_ro_error_reset(void);
static void _lbrob_ro_send_status(RX_SOCKET);

//--- lbrob_init --------------------------------------
void lbrob_init(void)
{
	int i;

	motor_config(MOTOR_CAP, CURRENT_HOLD, CAP_STEPS_PER_METER, CAP_INC_PER_METER);

	
	//--- movement parameters capping ---------------- 0.9 Nm with 100 U/min --- 0.8 Nm with 300 U/min -- 300*200*16/60
	
	_ParCap_ref.speed = 8000; // 16000; // speed with max tork: 16'000
	_ParCap_ref.accel =  4000; //8000;
	_ParCap_ref.current_acc = 400.0; // max 424 = 4.24 A
	_ParCap_ref.current_run = 400.0; // max 424 = 4.24 A
	_ParCap_ref.stop_mux = 0;
	_ParCap_ref.dis_mux_in = 0;
	_ParCap_ref.encCheck = chk_off;
		
	_ParCap_drive.speed = 16000; // 32000; // speed with max tork: 16'000
	_ParCap_drive.accel = 8000; // 16000;
	_ParCap_drive.current_acc = 400.0; // max 424 = 4.24 A
	_ParCap_drive.current_run = 400.0; // max 424 = 4.24 A
	_ParCap_ref.stop_mux = 0;
	_ParCap_ref.dis_mux_in = 0;
	_ParCap_drive.encCheck = chk_off; // TRUE;

	_ParCap_wipe.speed = 8000; // 25000; // speed with max tork: 16'000
	_ParCap_wipe.accel = 4000;// 12500;
	_ParCap_wipe.current_acc = 300.0; // max 424 = 4.24 A
	_ParCap_wipe.current_run = 300.0; // max 424 = 4.24 A
	_ParCap_wipe.stop_mux = 0;
	_ParCap_wipe.dis_mux_in = 0;
	_ParCap_wipe.encCheck = chk_off; // TRUE;	
}

//--- lbrob_main ------------------------------------------------------------------
void lbrob_main(int ticks, int menu)
{
	motor_main(ticks, menu);
	
	// --- read Inputs ---
	RX_StepperStatus.info.x_in_ref = fpga_input(RO_STORED_IN); // Reference Sensor
	//RX_StepperStatus.info.cln_in_capping = fpga_input(RO_CAPPING_IN); // Sensor for cleaning station beeing in capping pos
	RX_StepperStatus.posZ = motor_get_step(MOTOR_CAP) * 1000000.0 / CAP_STEPS_PER_METER;
	
	// --- Set Position Flags ---
	RX_StepperStatus.info.z_in_ref   = ((RX_StepperStatus.info.ref_done == 1) && (abs(RX_StepperStatus.posZ - POS_STORED_UM)     <= 10) && (RX_StepperStatus.info.x_in_ref == 1));
	RX_StepperStatus.info.z_in_print = RX_StepperStatus.info.z_in_ref;
	RX_StepperStatus.info.z_in_cap   = ((RX_StepperStatus.info.ref_done == 1) && (abs(RX_StepperStatus.posZ - POS_PRINTHEADS_UM) <= 10) && (RX_StepperStatus.info.x_in_ref == 0));

	// --- set positions False while moving ---
	if (RX_StepperStatus.info.moving) //  && (Fpga.stat->moving != 0))
	{
		RX_StepperStatus.info.z_in_ref = FALSE;
		RX_StepperStatus.info.z_in_print = FALSE;
		RX_StepperStatus.info.z_in_cap = FALSE;
	//	RX_StepperStatus.info.move_ok = FALSE;
	}

	if (_CmdRunning && motors_move_done(MOTOR_ALL_BITS))
	{
		RX_StepperStatus.info.moving = FALSE;

		// --- tasks ater the deceleration period of the Stopp Command ---
		if (_CmdRunning == CMD_CLN_STOP)
		{
			if (motor_error(MOTOR_CAP))
			{
				RX_StepperStatus.info.ref_done = FALSE;
				Error(ERR_CONT, 0, "FLO_RO: Command 0x%08x: triggers motor_error", _CmdRunning);
			}
		}

		// --- tasks ater reference cleaning station ---
		if (_CmdRunning == CMD_CLN_REFERENCE)
		{
			rx_sleep(1000); // wait 1s on transport to stand still
			RX_StepperStatus.info.x_in_ref = fpga_input(RO_STORED_IN); // Reference Sensor

			if (motor_error(MOTOR_CAP))
			{
				RX_StepperStatus.info.ref_done = FALSE;
				Error(ERR_CONT, 0, "FLO_RO: Command 0x%08x: triggers motor_error", _CmdRunning);
			}
			else if (RX_StepperStatus.info.x_in_ref)
			{
				motor_reset(MOTOR_CAP);				// reset position after referencing
				RX_StepperStatus.info.ref_done = TRUE;
				_reset_retried = 0;
				RX_StepperStatus.posZ = motor_get_step(MOTOR_CAP) * 1000000.0 / CAP_STEPS_PER_METER;
			}
			else
			{
				if (_reset_retried < 3)
				{
					_reset_retried++;
					_new_cmd = CMD_CLN_REFERENCE;
					Error(LOG, 0, "FLO_RO: Command 0x%08x: Retry Reference CLN", _CmdRunning);
				}
				else
				{
					Error(ERR_CONT, 0, "FLO_RO: Command 0x%08x: Referencing CLN failed", _CmdRunning);
					_reset_retried = 0;
				}
			}
		}

		// --- tasks after move to pos 0 stored position, check ref sensor ---
		if ((_CmdRunning == CMD_CLN_MOVE_POS) && (_lastPosCmd == 0))  //  && (RX_StepperStatus.posZ == 0)
		{
			if (!RX_StepperStatus.info.x_in_ref)
			{
				_new_cmd = CMD_CLN_REFERENCE;
				Error(LOG, 0, "FLO_RO: Command 0x%08x: triggers Referencing", _CmdRunning); //problem mit toggel move : do not toggle twice!
			}
		}
				
		// --- Reset Commands ---
		_CmdRunning = FALSE;

		// --- Execute next Commands ---
		switch (_new_cmd)
		{
		case CMD_CLN_REFERENCE:	  lbrob_handle_ctrl_msg(INVALID_SOCKET, CMD_CLN_REFERENCE, NULL);			 break;
		case 0: break;
		default:				Error(ERR_CONT, 0, "FLO_RO_MAIN: Command 0x%08x not implemented", _new_cmd); break;
		}
		_new_cmd = FALSE;

		// _cln_send_status(0); // Push news of move end to main

	}
	//// --- Executed after each move ---
	//if (_CmdRunning  == CMD_CLN_STOP && Fpga.stat->moving == 0)
	//{
	////	RX_StepperStatus.info.moving = FALSE;
	////	_CmdRunning = FALSE;
	//	RX_StepperStatus.info.x_in_ref = fpga_input(RO_STORED_IN); 
	//}
}

//--- lbrob_display_status ---------------------------------------------------------
static void _lbrob_display_status(void)
{
	int enc0_mm;
	enc0_mm = Fpga.encoder[MOTOR_CAP].pos * 1000000.0 / CAP_INC_PER_METER;
	int printhead_soll = POS_PRINTHEADS_UM;

	term_printf("LB Robot ---------------------------------\n");
	term_printf("moving:         %d		cmd: %08x\n", RX_StepperStatus.info.moving, _CmdRunning);
	term_printf("Sensor Reference Motor: %d\n", RX_StepperStatus.info.x_in_ref);
	term_printf("z in reference: %d  ", RX_StepperStatus.info.z_in_ref);
	term_printf("z in print:     %d  ", RX_StepperStatus.info.z_in_print);
	term_printf("z in capping:   %d\n", RX_StepperStatus.info.z_in_cap);
	term_printf("Flag reference done: %d  ", RX_StepperStatus.info.ref_done);
//	term_printf("Flag move OK: %d", RX_StepperStatus.info.move_ok);
	term_printf("Last pos cmd in um:   %d\n", _lastPosCmd);
	term_printf("Pos0 in steps:   %d  ", motor_get_step(MOTOR_CAP));
	//term_printf("Pos0 in steps SOLL:   %d\n", printhead_soll);
	term_printf("Pos0 in um:   %d \n", RX_StepperStatus.posZ);
	term_printf("Enc0 in steps:   %d  ", Fpga.encoder[MOTOR_CAP].pos);
	term_printf("Enc0 in um:   %d\n", enc0_mm);
	term_printf("\n");
}

//--- lbrob_menu --------------------------------------------------
int lbrob_menu(void)
{
	char str[MAX_PATH];
	int synth = FALSE;
	static int cnt = 0;
	int i;
	int pos = 0;

	_lbrob_display_status();
	
	term_printf("MENU TX-PROTOTYPE-ROBOT -------------------------\n");
	term_printf("o: toggle output <no>\n");
	term_printf("s: STOP\n");
	term_printf("r<n>: reset motor<n>\n");
	term_printf("R: Reference\n");
	term_printf("p: move to <um>\n");
	term_printf("m<n><steps>: move Motor<n> by <steps>\n");
	term_printf("w: wipe to <um>\n");
	term_printf("x: exit\n");
	term_printf(">");
	term_flush();

	if (term_get_str(str, sizeof(str)))
	{
		switch (str[0])
		{
		case 'o': Fpga.par->output ^= (1 << atoi(&str[1])); break;
		case 's': lbrob_handle_ctrl_msg(INVALID_SOCKET, CMD_CLN_STOP, NULL); break;
		case 'r': if (str[1] == 0) for (i = 0; i < MOTOR_CNT; i++) motor_reset(i);
			else           motor_reset(atoi(&str[1]));
			break;
		case 'R': lbrob_handle_ctrl_msg(INVALID_SOCKET, CMD_CLN_REFERENCE, NULL); break;
		case 'p': pos = atoi(&str[1]); lbrob_handle_ctrl_msg(INVALID_SOCKET, CMD_CLN_MOVE_POS, &pos); break;
		case 'm': _lbrob_ro_motor_test(str[1] - '0', atoi(&str[2])); break;
		case 'x': return FALSE;
		}
	}
	return TRUE;
}


//--- lbrob_handle_ctrl_msg -----------------------------------
int  lbrob_handle_ctrl_msg(RX_SOCKET socket, int msgId, void *pdata)
{	
	int i;
	INT32 pos, steps;
	
	switch (msgId)
	{
	case CMD_CLN_STOP:				motors_stop(MOTOR_ALL_BITS);
		_CmdRunning = msgId;
		Fpga.par->output &= ~RO_ALL_OUT; // set all output to off
		RX_StepperStatus.info.moving = TRUE;
		break;

	case CMD_CLN_REFERENCE:
		if (_CmdRunning){ lbrob_handle_ctrl_msg(INVALID_SOCKET, CMD_CLN_STOP, NULL); _new_cmd = CMD_CLN_REFERENCE; break; }
		motor_reset(MOTOR_CAP); // to recover from move count missalignment
		_CmdRunning = msgId;
		if ((Fpga.par->output & RO_ALL_OUT))
		{
			Fpga.par->output &= ~RO_ALL_OUT; // set all output to off
			rx_sleep(1000); // wait ms
		}
		RX_StepperStatus.info.ref_done = FALSE;
		RX_StepperStatus.info.moving = TRUE;
		_lastPosCmd = 0;
		motors_move_by_step(MOTOR_CAP_BITS, &_ParCap_ref, 1000000, TRUE);
		break;

	case CMD_CLN_MOVE_POS:		if (!_CmdRunning)
		{
			if ((Fpga.par->output & RO_ALL_OUT))
			{
				Fpga.par->output &= ~RO_ALL_OUT; // set all output to off
				rx_sleep(1000); // wait ms
			}
			_CmdRunning = msgId;
			RX_StepperStatus.info.moving = TRUE;
			pos = *((INT32*)pdata);
			_lastPosCmd = pos;
			if (!RX_StepperStatus.info.ref_done) Error(LOG, 0, "CLN: Command 0x%08x: missing ref_done: triggers Referencing", _CmdRunning);
			if (RX_StepperStatus.info.ref_done) motors_move_to_step(MOTOR_CAP_BITS, &_ParCap_drive, -pos*CAP_STEPS_PER_METER / 1000000);
			else								  lbrob_handle_ctrl_msg(INVALID_SOCKET, CMD_CLN_REFERENCE, NULL);
		}
		break;
								
	case CMD_CLN_CAP:		if (!_CmdRunning)
		{
			if (!RX_StepperStatus.info.ref_done)
			{
				Error(LOG, 0, "CLN: Command 0x%08x: missing ref_done: triggers Referencing", _CmdRunning); 
				lbrob_handle_ctrl_msg(INVALID_SOCKET, CMD_CLN_REFERENCE, NULL);
			}
			else if (RX_StepperStatus.info.z_in_cap)
			{
				Fpga.par->output |= RO_CAPP_INFLATE;
			}
			else 
			{
				Error(ERR_CONT, 0, "FLO_RO: Command 0x%08x: not in capping position", _CmdRunning);
			}										
												  
		}
		break;

	case CMD_ERROR_RESET:
		fpga_stepper_error_reset();
		_lbrob_ro_error_reset();
		break;
	
	case CMD_CAP_VENT:	break;
		
	default:						Error(ERR_CONT, 0, "CLN: Command 0x%08x not implemented", msgId); break;
	}
}

//--- _lbrob_ro_motor_test ---------------------------------
static void _lbrob_ro_motor_test(int motorNo, int steps)
{
	int motor = 1 << motorNo;
	SMovePar par;
	int i;

	memset(&par, 0, sizeof(par));
	if (motorNo == MOTOR_CAP)
	{
		par.speed = 8000; // 16000; // 32000;//25000; // 8000;// 4000; // 2000;
		par.accel = 4000; // 8000; //16000;//12500;// 4000;//2000; // 1000;
		par.current_acc = 400.0; // 60.0;  // for Tests: 50
		par.current_run = 400.0; // 60.0;  // for Tests: 50
	}
	else
	{
		par.speed	= 21000;		//21000;					//21000;				//21000;			//21000;			//21000;					
		par.accel	= 10000;		//10000;					//10000;				//10000;			//10000;			//10000;				
		par.current_acc = 80.0;//134.0;	//40.0;	minimum for 0.3 Nm	// 60.0; for 0.4 Nm		// 80.0 for 0.5 Nm	// 100.0 for 0.6 Nm	// 120.0 for 0.7 Nm	// 134.0 for 0.8 Nm	
		par.current_run = 80.0;//134.0;	//40.0;	minimum for 0.3 Nm	// 60.0; for 0.4 Nm		// 80.0 for 0.5 Nm	// 100.0 for 0.6 Nm	// 120.0 for 0.7 Nm	// 134.0 for 0.8 Nm	
	}
	par.stop_mux = 0;
	par.dis_mux_in = 0;
	par.encCheck = chk_off;
	RX_StepperStatus.info.moving = TRUE;
	_CmdRunning = 1; // TEST

	for (i = 0; i < MOTOR_CNT; i++) if (motor & (1 << i)) motor_config(i, CURRENT_HOLD, CAP_STEPS_PER_METER, CAP_INC_PER_METER);
	motors_move_by_step(motor, &par, -steps, FALSE);
}

//--- _lbrob_ro_error_reset ------------------------------------
void	_lbrob_ro_error_reset(void)
{
	memset(_lbrob_Error, 0, sizeof(_lbrob_Error));
	// use: ErrorFlag (ERR_CONT, (UINT32*)&_Error[isNo],  1,  0, "InkSupply[%d] Ink Tank Sensor Error", isNo+1);
}

//--- _lbrob_ro_send_status --------------------------------------------------
static void _lbrob_ro_send_status(RX_SOCKET socket)
{
	static RX_SOCKET _socket = INVALID_SOCKET;
	if (socket != INVALID_SOCKET) _socket = socket;
	if (_socket != INVALID_SOCKET) sok_send_2(&_socket, REP_TT_STATUS, sizeof(RX_StepperStatus), &RX_StepperStatus);
}