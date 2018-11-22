// ****************************************************************************
//
//	DIGITAL PRINTING - flo_rob.c
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
#include "txrob.h"

//--- defines ------------------------------------------------
#define CURRENT_HOLD	   10.0 // 200 steps/rev // 2 mm/rev
#define CURRENT_HOLD_SCREWS 7.0 // 200 steps/rev

#define MOTOR_SCREW_CNT		1
#define MOTOR_CAP			0
#define MOTOR_SCREW_0		1

#define MOTOR_CAP_BITS			0x01
#define MOTOR_SCREW_ALL_BITS	0x02
#define MOTOR_SCREW_0_BITS		0x02
#define MOTOR_ALL_BITS			0x03

#define CAP_STEPS_PER_METER		1600000.0 // 200 steps *16 / 2mm * 1000mm = 1'600'000 steps/m // 1000 / (0.01 mm/step) *16 = 1'600'000
#define CAP_INC_PER_METER		2000000.0 // 100'000 m / 0.8
//#define CAP_ENC_RATIO			0.8 //

#define SCREW_STEPS_PER_REV		 48000.0 // 200 * 16 * 15
#define SCREW_INC_PER_REV		120000.0 //
#define SCREW_ENC_RATIO			0.4
#define SCREW_PLAY_REV			1 // 0.500 // 1=1rev // no negative value allowed!

#define SCREW_INC_PER_M		SCREW_INC_PER_REV*SCREW_REV_PER_M 
#define RO_STORED_IN		0
//#define RO_CAPPING_IN		1
#define RO_SCREW_DOWN_0	1

#define POS_STORED_UM			0 // Ref
#define POS_PRINTHEADS_UM	RX_StepperCfg.cap_pos // capping position

static SMovePar	_ParCap_ref;
static SMovePar	_ParCap_drive;
static SMovePar	_ParCap_wipe;

static SMovePar	_ParScrew_ref;
static SMovePar	_ParScrew_turn;

static int		_CmdRunning = 0;
static int		_lastPosCmd = 0;
static int		_reset_retried = 0;
static int		_new_cmd = FALSE;
static int		_rescrew_pos = 0;

static UINT32	_txrob_Error[5];

//--- prototypes --------------------------------------------
static void _txrob_motor_test(int motor, int steps);
static void _txrob_turn_screw_to_pos(int screw, int steps);
static void _txrob_error_reset(void);
static void _txrob_send_status(RX_SOCKET);

//--- txrob_init --------------------------------------
void txrob_init(void)
{
	int i;

	motor_config(MOTOR_CAP, CURRENT_HOLD, CAP_STEPS_PER_METER, CAP_INC_PER_METER);
	motors_config(MOTOR_SCREW_ALL_BITS, CURRENT_HOLD_SCREWS, CAP_STEPS_PER_METER, CAP_INC_PER_METER);

	//--- movement parameters screws ----------------

	_ParScrew_ref.speed = 4000; // 21000; // speed with max tork: 21'333
	_ParScrew_ref.accel = 1000;
	_ParScrew_ref.current = 60.0; //  80.0; // max 134 = 1.34 A // Amps/Phase Parallel: 0.67 A
	_ParScrew_ref.stop_mux = 0;
	_ParScrew_ref.estop_in = RO_SCREW_DOWN_0;
	_ParScrew_ref.estop_level = 0; // stopp when sensor off
	_ParScrew_ref.checkEncoder = TRUE;

	_ParScrew_turn.speed = 21000; // speed with max tork: 21'333
	_ParScrew_turn.accel = 10000;
	_ParScrew_turn.current = 80.0;  // max 134 = 1.34 A // Amps/Phase Parallel: 0.67 A // approx 0.9N; max 1.4 N
	_ParScrew_turn.stop_mux = 0;
	_ParScrew_turn.estop_in = ESTOP_UNUSED;
	_ParScrew_turn.estop_level = 0;
	_ParScrew_turn.checkEncoder = TRUE;

	//--- movement parameters capping ----------------
	
	_ParCap_ref.speed = 16000; // speed with max tork: 16'000
	_ParCap_ref.accel =  8000;
	_ParCap_ref.current = 60.0; // max 134 = 1.34 A // Amps/Phase Parallel: 0.67 A
	_ParCap_ref.stop_mux = 0;
	_ParCap_ref.estop_in = RO_STORED_IN; // Check Input 0
	_ParCap_ref.estop_level = 1; // stopp when sensor on
	_ParCap_ref.checkEncoder = TRUE;

	_ParCap_drive.speed = 32000; // speed with max tork: 16'000
	_ParCap_drive.accel = 16000;
	_ParCap_drive.current = 60.0; // max 134 = 1.34 A // Amps/Phase Parallel: 0.67 A
	_ParCap_drive.stop_mux = 0;
	_ParCap_drive.estop_in = ESTOP_UNUSED;
	_ParCap_drive.estop_level = 1;
	_ParCap_drive.checkEncoder = TRUE;

	_ParCap_wipe.speed = 25000; // speed with max tork: 16'000
	_ParCap_wipe.accel = 12500;
	_ParCap_wipe.current = 60.0; // max 134 = 1.34 A // Amps/Phase Parallel: 0.67 A
	_ParCap_wipe.stop_mux = 0;
	_ParCap_wipe.estop_in = ESTOP_UNUSED;
	_ParCap_wipe.estop_level = 1;
	_ParCap_wipe.checkEncoder = TRUE;

}

//--- txrob_main ------------------------------------------------------------------
void txrob_main(int ticks, int menu)
{
	motor_main(ticks, menu);
	
	// --- read Inputs ---
	RX_TestTableStatus.info.cln_in_stored = fpga_input(RO_STORED_IN); // Reference Sensor
	RX_TestTableStatus.info.cln_screw_0 = fpga_input(RO_SCREW_DOWN_0); // Screwhead mot0 is pressed down
	//RX_TestTableStatus.info.cln_in_capping = fpga_input(RO_CAPPING_IN); // Sensor for cleaning station beeing in capping pos
	RX_TestTableStatus.posZ = motor_get_step(MOTOR_CAP) * 1000000.0 / CAP_STEPS_PER_METER;
	
	// --- Set Position Flags ---
	RX_TestTableStatus.info.z_in_ref = ((RX_TestTableStatus.info.ref_done == 1)
		&& (abs(RX_TestTableStatus.posZ - POS_STORED_UM) <= 10)
		&& (RX_TestTableStatus.info.cln_in_stored == 1));
	RX_TestTableStatus.info.z_in_print = RX_TestTableStatus.info.z_in_ref;
	RX_TestTableStatus.info.z_in_cap = ((RX_TestTableStatus.info.ref_done == 1)
		&& (abs(RX_TestTableStatus.posZ - POS_PRINTHEADS_UM) <= 10)
		&& (RX_TestTableStatus.info.cln_in_stored == 0));

	// --- set positions False while moving ---
	if (RX_TestTableStatus.info.moving) //  && (Fpga.stat->moving != 0))
	{
		RX_TestTableStatus.info.z_in_ref = FALSE;
		RX_TestTableStatus.info.z_in_print = FALSE;
		RX_TestTableStatus.info.z_in_cap = FALSE;
		RX_TestTableStatus.info.move_ok = FALSE;
		RX_TestTableStatus.info.screw_in_ref = FALSE;
		RX_TestTableStatus.info.screw_done = FALSE;
		if (_CmdRunning == CMD_CLN_SCREW_0_POS)			RX_TestTableStatus.adjustmentProgress = (int)(motor_get_step(MOTOR_SCREW_0) * 100.0 / (motor_get_end_step(0)));
		//else											RX_TestTableStatus.adjustmentProgress = 0;
	}

	if (_CmdRunning && motors_move_done(MOTOR_ALL_BITS))
	{
		RX_TestTableStatus.info.moving = FALSE;

		// --- tasks ater the deceleration period of the Stopp Command ---
		if (_CmdRunning == CMD_CLN_STOP)
		{
			if (motor_error(MOTOR_CAP))
			{
				RX_TestTableStatus.info.ref_done = FALSE;
				Error(ERR_CONT, 0, "FLO_RO: Command 0x%08x: triggers motor_error", _CmdRunning);
			}
		}

		// --- tasks ater reference cleaning station ---
		if (_CmdRunning == CMD_CLN_REFERENCE)
		{
			rx_sleep(1000); // wait 1s on transport to stand still
			RX_TestTableStatus.info.cln_in_stored = fpga_input(RO_STORED_IN); // Reference Sensor

			if (motor_error(MOTOR_CAP))
			{
				RX_TestTableStatus.info.ref_done = FALSE;
				Error(ERR_CONT, 0, "FLO_RO: Command 0x%08x: triggers motor_error", _CmdRunning);
			}
			else if (RX_TestTableStatus.info.cln_in_stored)
			{
				motor_reset(MOTOR_CAP);				// reset position after referencing
				RX_TestTableStatus.info.ref_done = TRUE;
				_reset_retried = 0;
				RX_TestTableStatus.posZ = motor_get_step(MOTOR_CAP) * 1000000.0 / CAP_STEPS_PER_METER;
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

		// --- tasks ater reference screws ---
		if (_CmdRunning == CMD_CLN_SCREW_REF)
		{
			if (motor_error(MOTOR_SCREW_ALL_BITS))
			{
				Error(ERR_CONT, 0, "FLO_RO: Command 0x%08x: triggers motor_error", _CmdRunning);
			}
			else if (RX_TestTableStatus.info.cln_screw_0 == 0
				&& RX_TestTableStatus.info.cln_screw_1 == 0
				&& RX_TestTableStatus.info.cln_screw_2 == 0
				&& RX_TestTableStatus.info.cln_screw_3 == 0)
			{
				motor_reset(MOTOR_SCREW_ALL_BITS);				// reset position after referencing
				RX_TestTableStatus.info.screw_in_ref = TRUE;
			}
			else
			{
				Error(ERR_CONT, 0, "FLO_RO: Command 0x%08x: Referencing Screws failed", _CmdRunning);
			}
		}

		// --- tasks ater turning screws ---
		if ((_CmdRunning == CMD_CLN_SCREW_0_POS)
			|| (_CmdRunning == CMD_CLN_SCREW_1_POS)
			|| (_CmdRunning == CMD_CLN_SCREW_2_POS)
			|| (_CmdRunning == CMD_CLN_SCREW_3_POS))
		{
			if (motor_error(MOTOR_SCREW_ALL_BITS))
			{
				Error(ERR_CONT, 0, "FLO_RO: Command 0x%08x: triggers motor_error", _CmdRunning);
			}
			else
			{
				RX_TestTableStatus.info.screw_done = TRUE;
				RX_TestTableStatus.adjustmentProgress = (int)100;
				if (_rescrew_pos != 0)
				{
					_new_cmd = CMD_CLN_SCREW_0_POS;
				}
			}
		}

		// --- tasks ater move to pos 0 stored position, check ref sensor ---
		if ((_CmdRunning == CMD_CLN_MOVE_POS) && (_lastPosCmd == 0))  //  && (RX_TestTableStatus.posZ == 0)
		{
			if (!RX_TestTableStatus.info.cln_in_stored)
			{
				_new_cmd = CMD_CLN_REFERENCE;
				Error(LOG, 0, "FLO_RO: Command 0x%08x: triggers Referencing", _CmdRunning); //problem mit toggel move : do not toggle twice!
			}
		}

		// --- Set Position Flags after Commands ---
		RX_TestTableStatus.info.move_ok = ((_CmdRunning == CMD_CLN_REFERENCE || _CmdRunning == CMD_CLN_MOVE_POS || _CmdRunning == CMD_CLN_WIPE)
			&& RX_TestTableStatus.info.ref_done
			&& abs(RX_TestTableStatus.posZ - _lastPosCmd) <= 10);

		if (_new_cmd == FALSE)
		{
			RX_TestTableStatus.info.move_tgl = !RX_TestTableStatus.info.move_tgl;
		}

		// --- Reset Commands ---
		_CmdRunning = FALSE;

		// --- Execute next Commands ---
		switch (_new_cmd)
		{
		case CMD_CLN_SCREW_0_POS: txrob_handle_ctrl_msg(INVALID_SOCKET, CMD_CLN_SCREW_0_POS, &_rescrew_pos); break;
		case CMD_CLN_REFERENCE:	txrob_handle_ctrl_msg(INVALID_SOCKET, CMD_CLN_REFERENCE, NULL); break;
		case 0: break;
		default:				Error(ERR_CONT, 0, "FLO_RO_MAIN: Command 0x%08x not implemented", _new_cmd);	break;
		}
		_new_cmd = FALSE;
		_rescrew_pos = 0;

		// _cln_send_status(0); // Push news of move end to main

	}
	//// --- Executed after each move ---
	//if (_CmdRunning  == CMD_CLN_STOP && Fpga.stat->moving == 0)
	//{
	////	RX_TestTableStatus.info.moving = FALSE;
	////	_CmdRunning = FALSE;
	//	RX_TestTableStatus.info.cln_in_stored = fpga_input(RO_STORED_IN); 
	//}
}

//--- txrob_display_status ---------------------------------------------------------
static void _txrob_display_status(void)
{
	int enc0_mm;
	int screwpos0_mm;
	int screwenc0_mm;
	enc0_mm = Fpga.encoder[MOTOR_CAP].pos * 1000000.0 / CAP_INC_PER_METER;
	screwpos0_mm = motor_get_step(MOTOR_SCREW_0) * 1000000.0 / SCREW_STEPS_PER_REV;
	screwenc0_mm = Fpga.encoder[MOTOR_SCREW_0].pos * 1000000.0 / SCREW_STEPS_PER_REV;
	int printhead_soll = POS_PRINTHEADS_UM;

	term_printf("FLO Robot ---------------------------------\n");
	term_printf("moving:         %d		cmd: %08x\n", RX_TestTableStatus.info.moving, _CmdRunning);
	term_printf("Sensor Reference Motor: %d\n", RX_TestTableStatus.info.cln_in_stored);
	term_printf("Sensor screw_down 0 to 3: %d\n", RX_TestTableStatus.info.cln_screw_0);
	term_printf("z in reference: %d  ", RX_TestTableStatus.info.z_in_ref);
	term_printf("z in print:     %d  ", RX_TestTableStatus.info.z_in_print);
	term_printf("z in capping:   %d\n", RX_TestTableStatus.info.z_in_cap);
	term_printf("Flag reference done: %d  ", RX_TestTableStatus.info.ref_done);
	term_printf("Flag move OK: %d", RX_TestTableStatus.info.move_ok);
	term_printf("Last pos cmd in um:   %d\n", _lastPosCmd);
	term_printf("Pos0 in steps:   %d  ", motor_get_step(MOTOR_CAP));
	//term_printf("Pos0 in steps SOLL:   %d\n", printhead_soll);
	term_printf("Pos0 in um:   %d \n", RX_TestTableStatus.posZ);
	term_printf("Enc0 in steps:   %d  ", Fpga.encoder[MOTOR_CAP].pos);
	term_printf("Enc0 in um:   %d\n", enc0_mm);
	term_printf("ScrewPos0 in steps:   %d  ", motor_get_step(MOTOR_SCREW_0));
	term_printf("ScrewPos0 in mU:   %d\n", screwpos0_mm);
	term_printf("ScrewEnc0 in steps:   %d  ", Fpga.encoder[MOTOR_SCREW_0].pos);
	term_printf("ScrewEnc0 in mU:   %d\n", screwenc0_mm);
	term_printf("\n");
}

//--- txrob_menu --------------------------------------------------
int txrob_menu(void)
{
	char str[MAX_PATH];
	int synth=FALSE;
	static int cnt=0;
	int i;
	int pos=10000;

	_txrob_display_status();
	
	term_printf("MENU FLO-ROBOT -------------------------\n");
	term_printf("s: STOP\n");
	term_printf("r<n>: reset motor<n>\n");
	term_printf("R: Reference\n");
	term_printf("c: reference all screws\n");
	term_printf("t: turn screw 0 for 1/1000000 rev\n");
	term_printf("p: move to <rev>\n");
	term_printf("m<n><steps>: move Motor<n> by <steps>\n");
	term_printf("x: exit\n");
	term_printf(">");
	term_flush();

	if (term_get_str(str, sizeof(str)))
	{
		switch (str[0])
		{
		case 's': txrob_handle_ctrl_msg(INVALID_SOCKET, CMD_CLN_STOP, NULL); break;
		case 'r': if (str[1] == 0) for (i = 0; i < MOTOR_CNT; i++) motor_reset(i);
				  else           motor_reset(atoi(&str[1]));
				  break;
		case 'R': txrob_handle_ctrl_msg(INVALID_SOCKET, CMD_CLN_REFERENCE, NULL); break;
		case 'c': txrob_handle_ctrl_msg(INVALID_SOCKET, CMD_CLN_SCREW_REF, NULL); break;
		case 't' : pos = atoi(&str[1]); txrob_handle_ctrl_msg(INVALID_SOCKET, CMD_CLN_SCREW_0_POS, &pos); break;
		case 'p': pos = atoi(&str[1]); txrob_handle_ctrl_msg(INVALID_SOCKET, CMD_CLN_MOVE_POS, &pos); break;
		case 'm': _txrob_motor_test(str[1] - '0', atoi(&str[2])); break;
		case 'x': return FALSE;
		}
	}
	return TRUE;
}


//--- txrob_handle_ctrl_msg -----------------------------------
int  txrob_handle_ctrl_msg(RX_SOCKET socket, int msgId, void *pdata)
{	
	int i;
	INT32 pos, steps;
	
	switch(msgId)
	{
	case CMD_TT_STATUS:				sok_send_2(&socket, INADDR_ANY, REP_TT_STATUS, sizeof(RX_TestTableStatus), &RX_TestTableStatus);	
									break;

	case CMD_CLN_STOP:				motors_stop(MOTOR_ALL_BITS);
		_CmdRunning = msgId;
		RX_TestTableStatus.info.moving = TRUE;
		break;

	case CMD_CLN_REFERENCE:
		if (_CmdRunning){ txrob_handle_ctrl_msg(INVALID_SOCKET, CMD_CLN_STOP, NULL); _new_cmd = CMD_CLN_REFERENCE; break; }
		motor_reset(MOTOR_CAP); // to recover from move count missalignment
		_CmdRunning = msgId;
		RX_TestTableStatus.info.ref_done = FALSE;
		RX_TestTableStatus.info.moving = TRUE;
		_lastPosCmd = 0;
		motors_move_by_step(MOTOR_CAP_BITS, &_ParCap_ref, -1000000, TRUE);
		break;

	case CMD_CLN_SCREW_REF:
		if (_CmdRunning){ txrob_handle_ctrl_msg(INVALID_SOCKET, CMD_CLN_STOP, NULL); break; }
		_CmdRunning = msgId;
		RX_TestTableStatus.info.moving = TRUE;
		motors_move_by_step(MOTOR_SCREW_0_BITS, &_ParScrew_ref, -500000, TRUE);
		break;

	case CMD_CLN_MOVE_POS:		if (!_CmdRunning)
	{
									_CmdRunning = msgId;
									RX_TestTableStatus.info.moving = TRUE;
									pos = *((INT32*)pdata);
									_lastPosCmd = pos;
									if (!RX_TestTableStatus.info.ref_done) Error(LOG, 0, "CLN: Command 0x%08x: missing ref_done: triggers Referencing", _CmdRunning);
									if (RX_TestTableStatus.info.ref_done) motors_move_to_step(MOTOR_CAP_BITS, &_ParCap_drive, pos*CAP_STEPS_PER_METER / 1000000);
		else								  txrob_handle_ctrl_msg(INVALID_SOCKET, CMD_CLN_REFERENCE, NULL);
	}
								break;

	case CMD_CLN_WIPE:		if (!_CmdRunning)
	{
								_CmdRunning = msgId;
								RX_TestTableStatus.info.moving = TRUE;
								pos = *((INT32*)pdata);
								_lastPosCmd = pos;
								if (!RX_TestTableStatus.info.ref_done) Error(LOG, 0, "CLN: Command 0x%08x: missing ref_done: triggers Referencing", _CmdRunning);
								if (RX_TestTableStatus.info.ref_done) motors_move_to_step(MOTOR_CAP_BITS, &_ParCap_wipe, pos*CAP_STEPS_PER_METER / 1000000);
		else								  txrob_handle_ctrl_msg(INVALID_SOCKET, CMD_CLN_REFERENCE, NULL);
	}
							break;

	case CMD_CLN_SCREW_0_POS:		if (!_CmdRunning)
	{
										_CmdRunning = msgId;
										RX_TestTableStatus.info.moving = TRUE;
										pos = *((INT32*)pdata);
										if (pos < 0)
										{
											pos = pos - SCREW_PLAY_REV*1000000;    // turn more than the play down in addition to the turn change
											_rescrew_pos = SCREW_PLAY_REV*1000000; // turn later more than the play up
										}
										motors_move_by_step(MOTOR_SCREW_0_BITS, &_ParScrew_turn, pos*SCREW_STEPS_PER_REV / 1000000.0, FALSE);
	}
									break;

	case CMD_ERROR_RESET:		_txrob_error_reset();
		fpga_stepper_error_reset();
		break;

	default:						Error(ERR_CONT, 0, "CLN: Command 0x%08x not implemented", msgId); break;
	}
}

//--- _txrob_motor_test ---------------------------------
static void _txrob_motor_test(int motorNo, int steps)
{
	int motor = 1 << motorNo;
	SMovePar par;
	int i;

	if (motorNo == MOTOR_CAP)
	{
		par.speed = 10000; // 32000;//25000; // 8000;// 4000; // 2000;
		par.accel = 5000; //16000;//12500;// 4000;//2000; // 1000;
		par.current = 150.0; // 60.0;  // for Tests: 50
	}
	else
	{
		par.speed	= 21000;		//21000;					//21000;				//21000;			//21000;			//21000;					
		par.accel	= 10000;		//10000;					//10000;				//10000;			//10000;			//10000;				
		par.current = 80.0;//134.0;	//40.0;	minimum for 0.3 Nm	// 60.0; for 0.4 Nm		// 80.0 for 0.5 Nm	// 100.0 for 0.6 Nm	// 120.0 for 0.7 Nm	// 134.0 for 0.8 Nm	
	}
	par.stop_mux = 0;
	par.estop_in = ESTOP_UNUSED;
	par.estop_level = 0;
	par.checkEncoder = FALSE;
	RX_TestTableStatus.info.moving = TRUE;
	_CmdRunning = 1; // TEST

	for (i = 0; i < MOTOR_CNT; i++) if (motor & (1 << i)) motor_config(i, CURRENT_HOLD, CAP_STEPS_PER_METER, CAP_INC_PER_METER);
	motors_move_by_step(motor, &par, steps, FALSE);
}

//--- _txrob_error_reset ------------------------------------
void	_txrob_error_reset(void)
{
	memset(_txrob_Error, 0, sizeof(_txrob_Error));
	// use: ErrorFlag (ERR_CONT, (UINT32*)&_Error[isNo],  1,  0, "InkSupply[%d] Ink Tank Sensor Error", isNo+1);
}

//--- _txrob_send_status --------------------------------------------------
static void _txrob_send_status(RX_SOCKET socket)
{
	static RX_SOCKET _socket = INVALID_SOCKET;
	if (socket != INVALID_SOCKET) _socket = socket;
	if (_socket != INVALID_SOCKET) sok_send_2(&_socket, INADDR_ANY, REP_TT_STATUS, sizeof(RX_TestTableStatus), &RX_TestTableStatus);
}