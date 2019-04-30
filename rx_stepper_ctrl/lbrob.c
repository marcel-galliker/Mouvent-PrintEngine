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
#define CURRENT_HOLD_SCREWS 7.0 // 200 steps/rev

#define MOTOR_SCREW_CNT		2
#define MOTOR_CAP			0
#define MOTOR_SCREW_0		1
#define MOTOR_SCREW_1		2

#define MOTOR_CAP_BITS			0x01
#define MOTOR_SCREW_ALL_BITS	0x06
#define MOTOR_SCREW_0_BITS		0x02
#define MOTOR_SCREW_1_BITS		0x04
#define MOTOR_ALL_BITS			0x07

#define CAP_STEPS_PER_METER		385852.0 // 200 steps *16 * 40 / U // 1:40
#define CAP_INC_PER_METER		482315.0 // 385852.0 / 0.8
//#define CAP_ENC_RATIO			0.8 //

#define SCREW_STEPS_PER_REV		 80000.0 // 200 * 16 * 25  // 1:25
#define SCREW_INC_PER_REV		200000.0 //
#define SCREW_ENC_RATIO			0.4
#define SCREW_PLAY_REV			2 // 1 // 0.500 // 1=1rev // no negative value allowed!
#define SCREW_DETACH_REV		0.002 // less than 1 deg

//#define SCREW_INC_PER_M		SCREW_INC_PER_REV*SCREW_REV_PER_M 

// Digital Inputs (max 12)
#define RO_STORED_IN		0
//#define RO_CAPPING_IN		1
#define RO_SCREW_DOWN_0		1
#define RO_SCREW_DETECT_0	2
#define RO_SCREW_DOWN_1		3
#define RO_SCREW_DETECT_1	4

// Digital Outputs (max 12)
#define RO_CAPP_INFLATE		0x001
#define RO_SCREW_UP_0		0x002
#define RO_SCREW_UP_1		0x004
#define RO_SCREW_UP_ALL		0x006
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
static SMovePar	_ParCap_detect[MOTOR_SCREW_CNT];

static SMovePar	_ParScrew_ref[MOTOR_SCREW_CNT];
static SMovePar	_ParScrew_turn;

static int		_CmdRunning = 0;
static int		_lastPosCmd = 0;
static int		_reset_retried = 0;
static int		_new_cmd = FALSE;
static int		_rescrew_pos = 0;

static UINT32	_lbrob_Error[5];

//--- prototypes --------------------------------------------
static void _lbrob_ro_motor_test(int motor, int steps);
static void _lbrob_ro_detect_test(int motor, int pos);
static void _lbrob_ro_turn_screw_to_pos(int screw, int steps);
static void _lbrob_ro_error_reset(void);
static void _lbrob_ro_send_status(RX_SOCKET);

//--- lbrob_init --------------------------------------
void lbrob_init(void)
{
	int i;

	motor_config(MOTOR_CAP, CURRENT_HOLD, CAP_STEPS_PER_METER, CAP_INC_PER_METER);
	motors_config(MOTOR_SCREW_ALL_BITS, CURRENT_HOLD_SCREWS, CAP_STEPS_PER_METER, CAP_INC_PER_METER);

	//--- movement parameters screws ---------------- 0.1 Nm with 400 U/min -- 400*200*16/60
	for (i = 0; i < MOTOR_SCREW_CNT; i++) 
	{	
		_ParScrew_ref[i].speed = 4000; // speed with max tork: 21'333
		_ParScrew_ref[i].accel = 1000;
		_ParScrew_ref[i].current = 60.0; // max 67 = 0.67 A
		_ParScrew_ref[i].stop_mux = 0;
		_ParScrew_ref[i].dis_mux_in = 0;
		_ParScrew_ref[i].stop_in = ESTOP_UNUSED;
		_ParScrew_ref[i].stop_level = 0; // stopp when sensor off
		_ParScrew_ref[i].estop_in = RO_SCREW_DOWN_0 + i;
		_ParScrew_ref[i].estop_level = 0; // stopp when sensor off
		_ParScrew_ref[i].checkEncoder = FALSE; // TRUE;
	}
	
		_ParScrew_turn.speed = 21000; // speed with max tork: 21'333
		_ParScrew_turn.accel = 10000;
		_ParScrew_turn.current = 80.0;  // max 95 = 0.95 A // Amps/Phase Parallel: 0.67 A // approx 0.9N; max 1.4 N
		_ParScrew_turn.stop_mux = 0;
		_ParScrew_turn.dis_mux_in = 0;
		_ParScrew_turn.stop_in = ESTOP_UNUSED;
		_ParScrew_turn.stop_level = 0;
		_ParScrew_turn.estop_in = ESTOP_UNUSED;
		_ParScrew_turn.estop_level = 0;
	_ParScrew_turn.checkEncoder = FALSE; // TRUE;
	
	//--- movement parameters capping ---------------- 0.9 Nm with 100 U/min --- 0.8 Nm with 300 U/min -- 300*200*16/60
	
	_ParCap_ref.speed = 8000; // 16000; // speed with max tork: 16'000
	_ParCap_ref.accel =  4000; //8000;
	_ParCap_ref.current = 400.0; // max 424 = 4.24 A
	_ParCap_ref.stop_mux = 0;
	_ParCap_ref.dis_mux_in = 0;
	_ParCap_ref.stop_in		= ESTOP_UNUSED; // Check Input 0
	_ParCap_ref.stop_level = 0; // stopp when sensor on
	_ParCap_ref.estop_in = RO_STORED_IN; // Check Input 0
	_ParCap_ref.estop_level = 1; // stopp when sensor on
	_ParCap_ref.checkEncoder = FALSE; // TRUE;

	_ParCap_drive.speed = 16000; // 32000; // speed with max tork: 16'000
	_ParCap_drive.accel = 8000; // 16000;
	_ParCap_drive.current = 400.0; // max 424 = 4.24 A
	_ParCap_drive.stop_mux = 0;
	_ParCap_drive.dis_mux_in = 0;
	_ParCap_drive.stop_in	= ESTOP_UNUSED;
	_ParCap_drive.stop_level = 0;
	_ParCap_drive.estop_in = ESTOP_UNUSED;
	_ParCap_drive.estop_level = 1;
	_ParCap_drive.checkEncoder = FALSE; // TRUE;

	_ParCap_wipe.speed = 8000; // 25000; // speed with max tork: 16'000
	_ParCap_wipe.accel = 4000;// 12500;
	_ParCap_wipe.current = 300.0; // max 424 = 4.24 A
	_ParCap_wipe.stop_mux = 0;
	_ParCap_wipe.dis_mux_in = 0;
	_ParCap_wipe.stop_in = ESTOP_UNUSED;
	_ParCap_wipe.stop_level = 0;
	_ParCap_wipe.estop_in = ESTOP_UNUSED;
	_ParCap_wipe.estop_level = 1;
	_ParCap_wipe.checkEncoder = FALSE; // TRUE;

	for (i = 0; i < MOTOR_SCREW_CNT; i++) 
	{
		_ParCap_detect[i].speed = 16000; // 32000; // speed with max tork: 16'000
		_ParCap_detect[i].accel = 8000; // 16000;
		_ParCap_detect[i].current = 400.0; // max 424 = 4.24 A
		_ParCap_detect[i].stop_mux = 0;
		_ParCap_detect[i].dis_mux_in = 0;
		_ParCap_detect[i].stop_in = ESTOP_UNUSED;
		_ParCap_detect[i].stop_level = 1;
		_ParCap_detect[i].estop_in = RO_SCREW_DETECT_0 + i;
		_ParCap_detect[i].estop_level = 1;
		_ParCap_detect[i].checkEncoder = FALSE; // TRUE;
	}
	
}

//--- lbrob_main ------------------------------------------------------------------
void lbrob_main(int ticks, int menu)
{
	motor_main(ticks, menu);
	
	// --- read Inputs ---
	RX_TestTableStatus.info.x_in_ref = fpga_input(RO_STORED_IN); // Reference Sensor
	RX_TestTableStatus.info.cln_screw_0 = fpga_input(RO_SCREW_DOWN_0); // Screwhead 0 is pressed down
	RX_TestTableStatus.info.cln_screw_1 = fpga_input(RO_SCREW_DOWN_1); // Screwhead 1 is pressed down
	RX_TestTableStatus.info.cln_screw_2 = fpga_input(RO_SCREW_DETECT_0); // Screwhead 0 is detected
	RX_TestTableStatus.info.cln_screw_3 = fpga_input(RO_SCREW_DETECT_1); // Screwhead 1 is detected
	//RX_TestTableStatus.info.cln_in_capping = fpga_input(RO_CAPPING_IN); // Sensor for cleaning station beeing in capping pos
	RX_TestTableStatus.posZ = motor_get_step(MOTOR_CAP) * 1000000.0 / CAP_STEPS_PER_METER;
	
	// --- Set Position Flags ---
	RX_TestTableStatus.info.z_in_ref = ((RX_TestTableStatus.info.ref_done == 1)
		&& (abs(RX_TestTableStatus.posZ - POS_STORED_UM) <= 10)
		&& (RX_TestTableStatus.info.x_in_ref == 1));
	RX_TestTableStatus.info.z_in_print = RX_TestTableStatus.info.z_in_ref;
	RX_TestTableStatus.info.z_in_cap = ((RX_TestTableStatus.info.ref_done == 1)
		&& (abs(RX_TestTableStatus.posZ - POS_PRINTHEADS_UM) <= 10)
		&& (RX_TestTableStatus.info.x_in_ref == 0));

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
			RX_TestTableStatus.info.x_in_ref = fpga_input(RO_STORED_IN); // Reference Sensor

			if (motor_error(MOTOR_CAP))
			{
				RX_TestTableStatus.info.ref_done = FALSE;
				Error(ERR_CONT, 0, "FLO_RO: Command 0x%08x: triggers motor_error", _CmdRunning);
			}
			else if (RX_TestTableStatus.info.x_in_ref)
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

		// --- tasks after move to pos 0 stored position, check ref sensor ---
		if ((_CmdRunning == CMD_CLN_MOVE_POS) && (_lastPosCmd == 0))  //  && (RX_TestTableStatus.posZ == 0)
		{
			if (!RX_TestTableStatus.info.x_in_ref)
			{
				_new_cmd = CMD_CLN_REFERENCE;
				Error(LOG, 0, "FLO_RO: Command 0x%08x: triggers Referencing", _CmdRunning); //problem mit toggel move : do not toggle twice!
			}
		}
		
		// --- tasks after wipe ---
		if ((_CmdRunning == CMD_CLN_WIPE))  //  && (RX_TestTableStatus.posZ == 0)
		{
			Fpga.par->output &= ~RO_BLADE_UP_0;
			Fpga.par->output &= ~RO_VACUUM_0;
			Fpga.par->output &= ~RO_BLADE_UP_1;
			Fpga.par->output &= ~RO_VACUUM_1;
			if (motor_error(MOTOR_CAP))
			{
				RX_TestTableStatus.info.ref_done = FALSE;
				Error(ERR_CONT, 0, "FLO_RO: Command 0x%08x: triggers motor_error", _CmdRunning);
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
		case CMD_CLN_SCREW_0_POS: lbrob_handle_ctrl_msg(INVALID_SOCKET, CMD_CLN_SCREW_0_POS, &_rescrew_pos); break;
		case CMD_CLN_REFERENCE:	  lbrob_handle_ctrl_msg(INVALID_SOCKET, CMD_CLN_REFERENCE, NULL);			 break;
		case 0: break;
		default:				Error(ERR_CONT, 0, "FLO_RO_MAIN: Command 0x%08x not implemented", _new_cmd); break;
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
	//	RX_TestTableStatus.info.x_in_ref = fpga_input(RO_STORED_IN); 
	//}
}

//--- lbrob_display_status ---------------------------------------------------------
static void _lbrob_display_status(void)
{
	int enc0_mm;
	int screwpos0_mm;
	int screwenc0_mm;
	enc0_mm = Fpga.encoder[MOTOR_CAP].pos * 1000000.0 / CAP_INC_PER_METER;
	screwpos0_mm = motor_get_step(MOTOR_SCREW_0) * 1000000.0 / SCREW_STEPS_PER_REV;
	screwenc0_mm = Fpga.encoder[MOTOR_SCREW_0].pos * 1000000.0 / SCREW_STEPS_PER_REV;
	int printhead_soll = POS_PRINTHEADS_UM;

	term_printf("TX Robot ---------------------------------\n");
	term_printf("moving:         %d		cmd: %08x\n", RX_TestTableStatus.info.moving, _CmdRunning);
	term_printf("Sensor Reference Motor: %d\n", RX_TestTableStatus.info.x_in_ref);
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
	term_printf("b: detach all screws\n");
	term_printf("c: reference all screws\n");
	term_printf("d<scr><um>: detect screw <scr> move to <um>\n");
	term_printf("t: turn screw 0 for 1/1000000 rev\n");
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
		case 'b': lbrob_handle_ctrl_msg(INVALID_SOCKET, CMD_CLN_SCREW_DETACH, NULL); break;
		case 'c': pos = atoi(&str[1]); lbrob_handle_ctrl_msg(INVALID_SOCKET, CMD_CLN_SCREW_REF, &pos); break;
		case 'd': _lbrob_ro_detect_test(str[1] - '0', atoi(&str[2])); break;
		case 't' : pos = atoi(&str[1]); lbrob_handle_ctrl_msg(INVALID_SOCKET, CMD_CLN_SCREW_0_POS, &pos); break;
		case 'p': pos = atoi(&str[1]); lbrob_handle_ctrl_msg(INVALID_SOCKET, CMD_CLN_MOVE_POS, &pos); break;
		case 'm': _lbrob_ro_motor_test(str[1] - '0', atoi(&str[2])); break;
		case 'w' : pos = atoi(&str[1]); lbrob_handle_ctrl_msg(INVALID_SOCKET, CMD_CLN_WIPE, &pos); break;
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
	case CMD_TT_STATUS:				sok_send_2(&socket, INADDR_ANY, REP_TT_STATUS, sizeof(RX_TestTableStatus), &RX_TestTableStatus);	
		break;

	case CMD_CLN_STOP:				motors_stop(MOTOR_ALL_BITS);
		_CmdRunning = msgId;
		Fpga.par->output &= ~RO_ALL_OUT; // set all output to off
		RX_TestTableStatus.info.moving = TRUE;
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
		RX_TestTableStatus.info.ref_done = FALSE;
		RX_TestTableStatus.info.moving = TRUE;
		_lastPosCmd = 0;
		motors_move_by_step(MOTOR_CAP_BITS, &_ParCap_ref, 1000000, TRUE);
		break;

	case CMD_CLN_SCREW_REF:
		if (_CmdRunning){ lbrob_handle_ctrl_msg(INVALID_SOCKET, CMD_CLN_STOP, NULL); break; }
		pos = *((INT32*)pdata);
		
		if (pos == 1)
		{
			_CmdRunning = msgId;
			Fpga.par->output |= RO_SCREW_UP_0;
			RX_TestTableStatus.info.moving = TRUE;
			motors_move_by_step(MOTOR_SCREW_0_BITS, &_ParScrew_ref[0], -500000, TRUE);
		}
		if (pos == 2)
		{
			_CmdRunning = msgId;
			Fpga.par->output |= RO_SCREW_UP_1;
			RX_TestTableStatus.info.moving = TRUE;
			motors_move_by_step(MOTOR_SCREW_1_BITS, &_ParScrew_ref[1], -500000, TRUE);
		}
		if (pos == 3)
		{
			_CmdRunning = msgId;
			Fpga.par->output |= RO_SCREW_UP_ALL;
			RX_TestTableStatus.info.moving = TRUE;
			for (i = 0; i < 2; i++) motor_move_by_step(i + 1, &_ParScrew_ref[i], -500000);											
			motors_start(MOTOR_SCREW_ALL_BITS, TRUE);
		}
		break;

	case CMD_CLN_MOVE_POS:		if (!_CmdRunning)
		{
			if ((Fpga.par->output & RO_ALL_OUT))
			{
				Fpga.par->output &= ~RO_ALL_OUT; // set all output to off
				rx_sleep(1000); // wait ms
			}
			_CmdRunning = msgId;
			RX_TestTableStatus.info.moving = TRUE;
			pos = *((INT32*)pdata);
			_lastPosCmd = pos;
			if (!RX_TestTableStatus.info.ref_done) Error(LOG, 0, "CLN: Command 0x%08x: missing ref_done: triggers Referencing", _CmdRunning);
			if (RX_TestTableStatus.info.ref_done) motors_move_to_step(MOTOR_CAP_BITS, &_ParCap_drive, -pos*CAP_STEPS_PER_METER / 1000000);
			else								  lbrob_handle_ctrl_msg(INVALID_SOCKET, CMD_CLN_REFERENCE, NULL);
		}
		break;

	case CMD_CLN_WIPE:		if (!_CmdRunning)
		{
			Fpga.par->output |= RO_BLADE_UP_0;
			Fpga.par->output |= RO_VACUUM_0;
			Fpga.par->output |= RO_BLADE_UP_1;
			Fpga.par->output |= RO_VACUUM_1;
			_CmdRunning = msgId;
			RX_TestTableStatus.info.moving = TRUE;
			pos = *((INT32*)pdata);
			_lastPosCmd = pos;
			if (!RX_TestTableStatus.info.ref_done) Error(LOG, 0, "CLN: Command 0x%08x: missing ref_done: triggers Referencing", _CmdRunning);
			if (RX_TestTableStatus.info.ref_done) motors_move_to_step(MOTOR_CAP_BITS, &_ParCap_wipe, -pos*CAP_STEPS_PER_METER / 1000000);
			else								  lbrob_handle_ctrl_msg(INVALID_SOCKET, CMD_CLN_REFERENCE, NULL);
		}
		break;

	case CMD_CLN_SCREW_0_POS:		if (!_CmdRunning)
		{
			_CmdRunning = msgId;
			Fpga.par->output |= RO_SCREW_UP_0;
			RX_TestTableStatus.info.moving = TRUE;
			pos = *((INT32*)pdata);
			if (pos < 0)
			{
				pos = pos - SCREW_PLAY_REV * 1000000;    // turn more than the play down in addition to the turn change
				_rescrew_pos = SCREW_PLAY_REV * 1000000; // turn later more than the play up
			}
			motors_move_by_step(MOTOR_SCREW_0_BITS, &_ParScrew_turn, pos*SCREW_STEPS_PER_REV / 1000000.0, FALSE);
		}
		break;
						
	case CMD_CLN_SCREW_1_POS:		if (!_CmdRunning)
		{
			_CmdRunning = msgId;
			Fpga.par->output |= RO_SCREW_UP_1;
			RX_TestTableStatus.info.moving = TRUE;
			pos = *((INT32*)pdata);
			if (pos < 0)
			{
				pos = pos - SCREW_PLAY_REV * 1000000;    // turn more than the play down in addition to the turn change
				_rescrew_pos = SCREW_PLAY_REV * 1000000; // turn later more than the play up
			}
			motors_move_by_step(MOTOR_SCREW_1_BITS, &_ParScrew_turn, pos*SCREW_STEPS_PER_REV / 1000000.0, FALSE);
		}
		break;
									
	case CMD_CLN_DETECT_SCREW_0:		if (!_CmdRunning)
		{
			_CmdRunning = msgId;
			if ((Fpga.par->output & RO_ALL_OUT))
			{
				Fpga.par->output &= ~RO_ALL_OUT; // set all output to off
				rx_sleep(1000); // wait ms
			}
			RX_TestTableStatus.info.moving = TRUE;
			pos = *((INT32*)pdata);
			_lastPosCmd = pos;
			if (!RX_TestTableStatus.info.ref_done) Error(LOG, 0, "CLN: Command 0x%08x: missing ref_done: triggers Referencing", _CmdRunning);
			if (RX_TestTableStatus.info.ref_done) motors_move_to_step(MOTOR_CAP_BITS, &_ParCap_detect[0], -pos*CAP_STEPS_PER_METER / 1000000);
			else								  lbrob_handle_ctrl_msg(INVALID_SOCKET, CMD_CLN_REFERENCE, NULL);
		}
		break;
								
	case CMD_CLN_DETECT_SCREW_1:		if (!_CmdRunning)
		{
			_CmdRunning = msgId;
			if ((Fpga.par->output & RO_ALL_OUT))
			{
				Fpga.par->output &= ~RO_ALL_OUT; // set all output to off
				rx_sleep(1000); // wait ms
			}									RX_TestTableStatus.info.moving = TRUE;
			pos = *((INT32*)pdata);
			_lastPosCmd = pos;
			if (!RX_TestTableStatus.info.ref_done) Error(LOG, 0, "CLN: Command 0x%08x: missing ref_done: triggers Referencing", _CmdRunning);
			if (RX_TestTableStatus.info.ref_done) motors_move_to_step(MOTOR_CAP_BITS, &_ParCap_detect[1], -pos*CAP_STEPS_PER_METER / 1000000);
			else								  lbrob_handle_ctrl_msg(INVALID_SOCKET, CMD_CLN_REFERENCE, NULL);
		}
		break;
								
	case CMD_CLN_SCREW_DETACH:		if (!_CmdRunning)
		{
			_CmdRunning = msgId;
			Fpga.par->output &= ~RO_SCREW_UP_ALL; // turn off screws
			RX_TestTableStatus.info.moving = TRUE;
			motors_move_by_step(MOTOR_SCREW_ALL_BITS, &_ParScrew_turn, -SCREW_DETACH_REV*SCREW_STEPS_PER_REV, FALSE);
		}
		break;
								
	case CMD_CLN_CAP:		if (!_CmdRunning)
		{
			if (!RX_TestTableStatus.info.ref_done)
			{
				Error(LOG, 0, "CLN: Command 0x%08x: missing ref_done: triggers Referencing", _CmdRunning); 
				lbrob_handle_ctrl_msg(INVALID_SOCKET, CMD_CLN_REFERENCE, NULL);
			}
			else if (RX_TestTableStatus.info.z_in_cap)
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
		_lbrob_ro_error_reset();
		fpga_stepper_error_reset();
		break;

	default:						Error(ERR_CONT, 0, "CLN: Command 0x%08x not implemented", msgId); break;
	}
}

//--- _lbrob_ro_detect_test ---------------------------------
static void _lbrob_ro_detect_test(int motorNo, int pos)
{
	switch (motorNo)	
	{
	case 0:	lbrob_handle_ctrl_msg(INVALID_SOCKET, CMD_CLN_DETECT_SCREW_0, &pos); break;
	case 1:	lbrob_handle_ctrl_msg(INVALID_SOCKET, CMD_CLN_DETECT_SCREW_1, &pos); break;
	}
}

//--- _lbrob_ro_motor_test ---------------------------------
static void _lbrob_ro_motor_test(int motorNo, int steps)
{
	int motor = 1 << motorNo;
	SMovePar par;
	int i;

	if (motorNo == MOTOR_CAP)
	{
		par.speed = 8000; // 16000; // 32000;//25000; // 8000;// 4000; // 2000;
		par.accel = 4000; // 8000; //16000;//12500;// 4000;//2000; // 1000;
		par.current = 400.0; // 60.0;  // for Tests: 50
	}
	else
	{
		par.speed	= 21000;		//21000;					//21000;				//21000;			//21000;			//21000;					
		par.accel	= 10000;		//10000;					//10000;				//10000;			//10000;			//10000;				
		par.current = 80.0;//134.0;	//40.0;	minimum for 0.3 Nm	// 60.0; for 0.4 Nm		// 80.0 for 0.5 Nm	// 100.0 for 0.6 Nm	// 120.0 for 0.7 Nm	// 134.0 for 0.8 Nm	
	}
	par.stop_mux = 0;
	par.dis_mux_in = 0;
	par.stop_in		= ESTOP_UNUSED;
	par.stop_level	= 0;
	par.estop_in = ESTOP_UNUSED;
	par.estop_level = 0;
	par.checkEncoder = FALSE;
	RX_TestTableStatus.info.moving = TRUE;
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
	if (_socket != INVALID_SOCKET) sok_send_2(&_socket, INADDR_ANY, REP_TT_STATUS, sizeof(RX_TestTableStatus), &RX_TestTableStatus);
}