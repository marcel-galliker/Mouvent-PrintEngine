// ****************************************************************************
//
//	DIGITAL PRINTING - web.c
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
#include "rx_threads.h"
#include "tcp_ip.h"
#include "fpga_stepper.h"
#include "power_step.h"
#include "motor_cfg.h"
#include "motor.h"
#include "cln.h"

// SStepperCfg
//EPrinterType	printerType;
//INT32			boardNo;
//#define		step_lift	0
//#define		step_clean	1
//INT32			print_height;
//INT32			wipe_height;
//INT32			capping_pos;

//--- defines ------------------------------------------------
#define CURRENT_HOLD	   10.0
#define CURRENT_HOLD_SCREWS 7.0

#define MOTOR_SCREW_CNT		4
#define MOTOR_CAP			0
#define MOTOR_SCREW_0		1
#define MOTOR_SCREW_1		2
#define MOTOR_SCREW_2		3
#define MOTOR_SCREW_3		4

#define MOTOR_CAP_BITS			0x01
#define MOTOR_SCREW_ALL_BITS	0x1e
#define MOTOR_SCREW_0_BITS		0x02
#define MOTOR_SCREW_1_BITS		0x04
#define MOTOR_SCREW_2_BITS		0x08
#define MOTOR_SCREW_3_BITS		0x10
#define MOTOR_ALL_BITS			0x1f

#define CAP_STEPS_PER_METER		138203.0 // calculated 133149.2 // measured 139581.3 // new measure 138203 (70069 steps)
#define CAP_INC_PER_METER		172754.0 // 174476.6 // calculated 166436.5 // measured 174476.6 // new measure 172754
//#define CAP_ENC_RATIO			0.8 // 40000.0/50000.0 // Encoder_steps * ENC_RATIO = Motor_steps

#define SCREW_STEPS_PER_REV		95616.0 // vollschrittwinkel 15 grad // 360/15*249=5976vollschritte // 5976*16=95616 steps  // rounded: 120*249/28761*100000=103891.0 // measured approx 72708.5
#define SCREW_INC_PER_REV		 29880.0 //  120steps/rev*249untersetzung=29880inc/rev
#define SCREW_ENC_RATIO			0.3125

#define SCREW_INC_PER_M		SCREW_INC_PER_REV*SCREW_REV_PER_M // for TEST !!!
#define CLN_STORED_IN		0
#define CLN_CAPPING_IN		1
#define CLN_SCREW_DOWN_0	2
#define CLN_SCREW_DOWN_1	3
#define CLN_SCREW_DOWN_2	4
#define CLN_SCREW_DOWN_3	5

#define POS_STORED_UM			0 // Ref
#define POS_PRINTHEADS_UM	RX_StepperCfg.cap_pos // capping position

static SMovePar	_ParCap_ref;
static SMovePar	_ParCap_drive;
static SMovePar	_ParCap_wipe;

static SMovePar	_ParScrew_ref[MOTOR_SCREW_CNT];
static SMovePar	_ParScrew_turn;

static int		_CmdRunning = 0;
static int		_lastPosCmd = 0;
static int		_reset_retried = 0;
static int		_new_cmd = FALSE;

static UINT32	_Cln_Error[5];

//--- prototypes --------------------------------------------
static void _cln_motor_test(int motor, int steps);
static void _cln_turn_screw_to_pos(int screw, int steps);
static void _cln_error_reset(void);
static void _cln_send_status(RX_SOCKET);

//--- cln_init --------------------------------------
void cln_init(void)
{
	int i;
	
	motor_config(MOTOR_CAP, CURRENT_HOLD, CAP_STEPS_PER_METER, CAP_INC_PER_METER);
	motors_config(MOTOR_SCREW_ALL_BITS, CURRENT_HOLD_SCREWS, CAP_STEPS_PER_METER, CAP_INC_PER_METER);
	
	//--- movement parameters screws ----------------
	
	for (i = 0; i < MOTOR_SCREW_CNT; i++) 
	{		
		_ParScrew_ref[i].speed			= 10000;
		_ParScrew_ref[i].accel			= 10000;
		_ParScrew_ref[i].current		= 7.0;
		_ParScrew_ref[i].stop_mux		= 0;
		_ParScrew_ref[i].estop_in		= 2 + i;	// Check Inputs 2..5
		_ParScrew_ref[i].estop_level	= 0; // stopp when sensor off
		_ParScrew_ref[i].checkEncoder	= FALSE; //TRUE;
	}
	
	_ParScrew_turn.speed		= 10000;
	_ParScrew_turn.accel		= 5000;
	_ParScrew_turn.current		= 7.0; // approx 0.9N; max 1.4 N
	_ParScrew_turn.stop_mux		= 0;
	_ParScrew_turn.estop_in		= ESTOP_UNUSED;
	_ParScrew_turn.estop_level	= 0;
	_ParScrew_turn.checkEncoder	= FALSE; //TRUE;
	
	//--- movement parameters capping ----------------
	
	_ParCap_ref.speed		 = 2000; // 1000
	_ParCap_ref.accel		 = 500;
	_ParCap_ref.current		 = 300.0; // 400.0 = 4A // Amps/Phase Parallel: 4.24A // fot Tests: 50
	_ParCap_ref.stop_mux	 = 0;
	_ParCap_ref.estop_in     = CLN_STORED_IN; // Check Input 0
	_ParCap_ref.estop_level  = 1; // stopp when sensor on
	_ParCap_ref.checkEncoder = TRUE;
	
	_ParCap_drive.speed			= 10000;
	_ParCap_drive.accel			= 3000;
	_ParCap_drive.current		= 400.0; // fot Tests: 50
	_ParCap_drive.stop_mux		= 0;
	_ParCap_drive.estop_in		= ESTOP_UNUSED;
	_ParCap_drive.estop_level	= 1;
	_ParCap_drive.checkEncoder	= TRUE;
	
	_ParCap_wipe.speed			= 4000;
	_ParCap_wipe.accel			= 2000;
	_ParCap_wipe.current		= 300.0; // fot Tests: 50
	_ParCap_wipe.stop_mux		= 0;
	_ParCap_wipe.estop_in		= ESTOP_UNUSED;
	_ParCap_wipe.estop_level	= 1;
	_ParCap_wipe.checkEncoder	= TRUE;
	
}

//--- cln_main ------------------------------------------------------------------
void cln_main(int ticks, int menu)
{
	motor_main(ticks, menu);
	
	// --- read Inputs ---
	RX_TestTableStatus.info.cln_in_stored = fpga_input(CLN_STORED_IN); // Reference Sensor
	RX_TestTableStatus.info.cln_screw_0 = fpga_input(CLN_SCREW_DOWN_0); // Screwhead mot0 is pressed down
	RX_TestTableStatus.info.cln_screw_1 = fpga_input(CLN_SCREW_DOWN_1); // Screwhead mot1 is pressed down
	RX_TestTableStatus.info.cln_screw_2 = fpga_input(CLN_SCREW_DOWN_2); // Screwhead mot2 is pressed down
	RX_TestTableStatus.info.cln_screw_3 = fpga_input(CLN_SCREW_DOWN_3); // Screwhead mot3 is pressed down
	RX_TestTableStatus.info.cln_in_capping = fpga_input(CLN_CAPPING_IN); // Sensor for cleaning station beeing in capping pos
	RX_TestTableStatus.posZ = motor_get_step(MOTOR_CAP) * 1000000.0 / CAP_STEPS_PER_METER;
	
	
	// --- Set Position Flags ---
	RX_TestTableStatus.info.z_in_ref  = ((RX_TestTableStatus.info.ref_done == 1)
											&& (abs(RX_TestTableStatus.posZ - POS_STORED_UM) <= 10) 
											&& (RX_TestTableStatus.info.cln_in_stored == 1) 
											&& (RX_TestTableStatus.info.cln_in_capping == 0));
	RX_TestTableStatus.info.z_in_print  = RX_TestTableStatus.info.z_in_ref;
	RX_TestTableStatus.info.z_in_cap    = ((RX_TestTableStatus.info.ref_done == 1)
											&& (abs(RX_TestTableStatus.posZ - POS_PRINTHEADS_UM) <= 10) 
											&& (RX_TestTableStatus.info.cln_in_capping == 1) 
											&& (RX_TestTableStatus.info.cln_in_stored == 0));
	
	// --- set positions False while moving ---
	if (RX_TestTableStatus.info.moving) //  && (Fpga.stat->moving != 0))
	{
		RX_TestTableStatus.info.z_in_ref   = FALSE;
		RX_TestTableStatus.info.z_in_print = FALSE;
		RX_TestTableStatus.info.z_in_cap   = FALSE;	
		RX_TestTableStatus.info.move_ok    = FALSE;
		RX_TestTableStatus.info.screw_in_ref = FALSE;
		RX_TestTableStatus.info.screw_done = FALSE;
		if (_CmdRunning == CMD_CLN_SCREW_0_POS)			RX_TestTableStatus.adjustmentProgress = (int) (motor_get_step(MOTOR_SCREW_0) * 100.0 / (motor_get_end_step(0))); 
		else if (_CmdRunning == CMD_CLN_SCREW_1_POS)	RX_TestTableStatus.adjustmentProgress = (int) (motor_get_step(MOTOR_SCREW_1) * 100.0 / (motor_get_end_step(1))); 
		else if (_CmdRunning == CMD_CLN_SCREW_2_POS)	RX_TestTableStatus.adjustmentProgress = (int) (motor_get_step(MOTOR_SCREW_2) * 100.0 / (motor_get_end_step(2))); 
		else if (_CmdRunning == CMD_CLN_SCREW_3_POS)	RX_TestTableStatus.adjustmentProgress = (int) (motor_get_step(MOTOR_SCREW_3) * 100.0 / (motor_get_end_step(3))); 
		else											RX_TestTableStatus.adjustmentProgress = 0;
	}
	
	// --- Executed after each move ---
	if (_CmdRunning && motors_move_done(MOTOR_ALL_BITS))
	{
		RX_TestTableStatus.info.moving = FALSE;
		
		// --- tasks ater the deceleration period of the Stopp Command ---
		if (_CmdRunning == CMD_CLN_STOP) 
		{
			if (motor_error(MOTOR_CAP))
			{
				RX_TestTableStatus.info.ref_done = FALSE;
				Error(ERR_CONT, 0, "CLN: Command 0x%08x: triggers motor_error", _CmdRunning);
			}
		}
		
		// --- tasks ater reference cleaning station ---
		if (_CmdRunning == CMD_CLN_REFERENCE) 
		{
			rx_sleep(1000); // wait 1s on transport to stand still
			RX_TestTableStatus.info.cln_in_stored = fpga_input(CLN_STORED_IN); // Reference Sensor

			if (motor_error(MOTOR_CAP))
			{
				RX_TestTableStatus.info.ref_done = FALSE;
				Error(ERR_CONT, 0, "CLN: Command 0x%08x: triggers motor_error", _CmdRunning);
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
					_reset_retried ++;
					_new_cmd = CMD_CLN_REFERENCE;
					Error(LOG, 0, "CLN: Command 0x%08x: Retry Reference CLN", _CmdRunning);
				}
				else
				{
					Error(ERR_CONT, 0, "CLN: Command 0x%08x: Referencing CLN failed", _CmdRunning);
					_reset_retried = 0;
				}
			}
		}
		
		// --- tasks ater reference screws ---
		if (_CmdRunning == CMD_CLN_SCREW_REF) 
		{
			if (motor_error(MOTOR_SCREW_ALL_BITS))
			{
				Error(ERR_CONT, 0, "CLN: Command 0x%08x: triggers motor_error", _CmdRunning);
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
				Error(ERR_CONT, 0, "CLN: Command 0x%08x: Referencing Screws failed", _CmdRunning);
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
				Error(ERR_CONT, 0, "CLN: Command 0x%08x: triggers motor_error", _CmdRunning);
			}
			else
			{
				RX_TestTableStatus.info.screw_done = TRUE;
				RX_TestTableStatus.adjustmentProgress = (int) 100;
			}
		}
		
		// --- tasks ater move to pos 0 stored position, check ref sensor ---
		if ((_CmdRunning == CMD_CLN_MOVE_POS) && (_lastPosCmd == 0))  //  && (RX_TestTableStatus.posZ == 0)
		{
			if (!RX_TestTableStatus.info.cln_in_stored)
			{
				_new_cmd = CMD_CLN_REFERENCE;
				Error(LOG, 0, "CLN: Command 0x%08x: triggers Referencing", _CmdRunning); //problem mit toggel move : do not toggle twice!
			}
		}
		
		// --- Set Position Flags after Commands ---
		RX_TestTableStatus.info.move_ok = ((_CmdRunning == CMD_CLN_REFERENCE || _CmdRunning == CMD_CLN_MOVE_POS || _CmdRunning == CMD_CLN_WIPE) 
												&& RX_TestTableStatus.info.ref_done
												&& abs(RX_TestTableStatus.posZ - _lastPosCmd) <= 10);
		
		if ((_new_cmd == FALSE) && (_CmdRunning != CMD_CAP_STOP)) 
		{
			RX_TestTableStatus.info.move_tgl = !RX_TestTableStatus.info.move_tgl;
		}
		
		// --- Reset Commands ---
		_CmdRunning = FALSE;
		
		// --- Execute next Commands ---
		switch (_new_cmd)
		{
		case CMD_CLN_REFERENCE:	cln_handle_ctrl_msg(INVALID_SOCKET, CMD_CLN_REFERENCE, NULL);				break;
		case 0: break;
		default:				Error(ERR_CONT, 0, "CLN_MAIN: Command 0x%08x not implemented", _new_cmd);	break;
		}
		_new_cmd = FALSE;
		
		// _cln_send_status(0); // Push news of move end to main
		
	}	
}

//--- cln_display_status ---------------------------------------------------------
void cln_display_status(void)
{
	int enc0_mm;
	int screwpos0_mm;
	int screwenc0_mm;
	enc0_mm = Fpga.encoder[MOTOR_CAP].pos * 1000000.0 / CAP_INC_PER_METER;
	screwpos0_mm = motor_get_step(MOTOR_SCREW_0) * 1000000.0 / SCREW_STEPS_PER_REV;
	screwenc0_mm = Fpga.encoder[MOTOR_SCREW_0].pos * 1000000.0 / SCREW_STEPS_PER_REV;
	int printhead_soll = POS_PRINTHEADS_UM;
	
	term_printf("CLEANING Unit ---------------------------------\n");
	term_printf("moving:         %d		cmd: %08x\n", RX_TestTableStatus.info.moving, _CmdRunning);	
	term_printf("Sensor Reference Motor: %d\n", RX_TestTableStatus.info.cln_in_stored);
	term_printf("Sensor Capping: %d\n", RX_TestTableStatus.info.cln_in_capping);
	term_printf("Sensor screw_down 0 to 3: %d  ", RX_TestTableStatus.info.cln_screw_0);
	term_printf(" %d  ", RX_TestTableStatus.info.cln_screw_1);
	term_printf(" %d  ", RX_TestTableStatus.info.cln_screw_2);
	term_printf(" %d\n", RX_TestTableStatus.info.cln_screw_3);
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

//--- cln_menu --------------------------------------------------
int cln_menu(void)
{
	char str[MAX_PATH];
	int synth = FALSE;
	static int cnt = 0;
	int i;
	int pos = 0;

	cln_display_status();
	
	term_printf("MENU Cleaning -------------------------\n");
	term_printf("o: toggle output <no>\n");
	term_printf("s: STOP\n");
	term_printf("r<n>: reset motor<n>\n");	
	term_printf("R: Reference\n");
	term_printf("c: reference all screws\n");
	term_printf("t: turn screw <screw_nr> for <um>\n");
	term_printf("p: move to <um>\n");
	term_printf("m<n><steps>: move Motor<n> by <steps>\n");	
	term_printf("x: exit\n");
	term_printf(">");
	term_flush();

	if (term_get_str(str, sizeof(str)))
	{
		switch (str[0])
		{
		case 'o': Fpga.par->output ^= (1 << atoi(&str[1])); break;
		case 's': cln_handle_ctrl_msg(INVALID_SOCKET, CMD_CLN_STOP, NULL); break;
		case 'r': if (str[1] == 0) for (i = 0; i < MOTOR_CNT; i++) motor_reset(i);
			else           motor_reset(atoi(&str[1])); 
			break;
		case 'R': cln_handle_ctrl_msg(INVALID_SOCKET, CMD_CLN_REFERENCE, NULL); break;
		case 'c': cln_handle_ctrl_msg(INVALID_SOCKET, CMD_CLN_SCREW_REF, NULL); break;
		case 't': _cln_turn_screw_to_pos(str[1] - '0', atoi(&str[2])); break;
		//case 't': pos = atoi(&str[1]); cln_handle_ctrl_msg(INVALID_SOCKET, CMD_CLN_SCREW_2_POS, &pos); break;
		case 'p': pos = atoi(&str[1]); cln_handle_ctrl_msg(INVALID_SOCKET, CMD_CLN_MOVE_POS, &pos); break;
		case 'm': _cln_motor_test(str[1] - '0', atoi(&str[2])); break;			
		case 'x': return FALSE;
		}
	}
	return TRUE;
}

//--- cln_handle_ctrl_msg -----------------------------------
int  cln_handle_ctrl_msg(RX_SOCKET socket, int msgId, void *pdata)
{	
	int i;
	int pos;
	int enc_pos;
	switch (msgId)
	{
	case CMD_TT_STATUS:				_cln_send_status(socket); // sok_send_2(&socket, REP_TT_STATUS, sizeof(RX_TestTableStatus), &RX_TestTableStatus);	
		break;
		
	case CMD_CLN_STOP:				motors_stop(MOTOR_ALL_BITS);
		_CmdRunning = msgId;
		RX_TestTableStatus.info.moving = TRUE;
		break;	
		
	case CMD_CLN_REFERENCE:
		if (_CmdRunning){cln_handle_ctrl_msg(INVALID_SOCKET, CMD_CLN_STOP, NULL); _new_cmd = CMD_CLN_REFERENCE; break; }
		motor_reset(MOTOR_CAP); // to recover from move count missalignment
		_CmdRunning  = msgId;
		RX_TestTableStatus.info.ref_done = FALSE;
		RX_TestTableStatus.info.moving = TRUE;
		_lastPosCmd = 0;
		motors_move_by_step(MOTOR_CAP_BITS, &_ParCap_ref, -100000, TRUE);										
		break;

	case CMD_CLN_SCREW_REF:
		if (_CmdRunning){cln_handle_ctrl_msg(INVALID_SOCKET, CMD_CLN_STOP, NULL); _new_cmd = CMD_CLN_SCREW_REF; break;}
		_CmdRunning  = msgId;
		RX_TestTableStatus.info.moving = TRUE;
		//motors_quad_move_by_step(MOTOR_SCREW_ALL_BITS, &_ParScrew_ref, -500000, TRUE);
		for (i = 0; i < 4; i++) motor_move_by_step(i + 1, &_ParScrew_ref[i], -500000);											
		motors_start(MOTOR_SCREW_ALL_BITS, TRUE);
		break;
		
	case CMD_CLN_MOVE_POS :		if (!_CmdRunning)
		{
			_CmdRunning  = msgId;
			RX_TestTableStatus.info.moving = TRUE;
			pos = *((INT32*)pdata);
			_lastPosCmd = pos;
			if (!RX_TestTableStatus.info.ref_done) Error(LOG, 0, "CLN: Command 0x%08x: missing ref_done: triggers Referencing", _CmdRunning);
			if (RX_TestTableStatus.info.ref_done) motors_move_to_step(MOTOR_CAP_BITS, &_ParCap_drive, pos*CAP_STEPS_PER_METER / 1000000);
			else								  cln_handle_ctrl_msg(INVALID_SOCKET, CMD_CLN_REFERENCE, NULL);
		}
		break;
		
	case CMD_CLN_WIPE :		if (!_CmdRunning)
		{
			_CmdRunning  = msgId;
			RX_TestTableStatus.info.moving = TRUE;
			pos = *((INT32*)pdata);
			_lastPosCmd = pos;
			if (!RX_TestTableStatus.info.ref_done) Error(LOG, 0, "CLN: Command 0x%08x: missing ref_done: triggers Referencing", _CmdRunning);
			if (RX_TestTableStatus.info.ref_done) motors_move_to_step(MOTOR_CAP_BITS, &_ParCap_wipe, pos*CAP_STEPS_PER_METER / 1000000);
			else								  cln_handle_ctrl_msg(INVALID_SOCKET, CMD_CLN_REFERENCE, NULL);
		}
		break;
		
	case CMD_CLN_SCREW_0_POS :		if (!_CmdRunning)
		{
			_CmdRunning  = msgId;
			RX_TestTableStatus.info.moving = TRUE;
			pos = *((INT32*)pdata);
			motors_move_by_step(MOTOR_SCREW_0_BITS, &_ParScrew_turn, pos*SCREW_STEPS_PER_REV / 1000000.0, FALSE);
		}
		break;

	case CMD_CLN_SCREW_1_POS :		if (!_CmdRunning)
		{
			_CmdRunning  = msgId;
			RX_TestTableStatus.info.moving = TRUE;
			pos = *((INT32*)pdata);
			motors_move_by_step(MOTOR_SCREW_1_BITS, &_ParScrew_turn, pos*SCREW_STEPS_PER_REV / 1000000.0, FALSE);
		}
		break;
		
	case CMD_CLN_SCREW_2_POS :		if (!_CmdRunning)
		{
			_CmdRunning  = msgId;
			RX_TestTableStatus.info.moving = TRUE;
			pos = *((INT32*)pdata);
			motors_move_by_step(MOTOR_SCREW_2_BITS, &_ParScrew_turn, pos*SCREW_STEPS_PER_REV / 1000000.0, FALSE);
		}
		break;
		
	case CMD_CLN_SCREW_3_POS :		if (!_CmdRunning)
		{
			_CmdRunning  = msgId;
			RX_TestTableStatus.info.moving = TRUE;
			pos = *((INT32*)pdata);
			motors_move_by_step(MOTOR_SCREW_3_BITS, &_ParScrew_turn, pos*SCREW_STEPS_PER_REV / 1000000.0, FALSE);
		}
		break;
		
	case CMD_ERROR_RESET:		_cln_error_reset();
		fpga_stepper_error_reset();
		break;

	default:						Error(ERR_CONT, 0, "CLN: Command 0x%08x not implemented", msgId); break;
	}
}

//--- _web_move_to_pos -------------------------------------------------
static void _cln_turn_screw_to_pos(int screw, int pos)
{
	int position = pos;
	if (!_CmdRunning)
	{
		switch (screw)
		{
		case 0 :
			_CmdRunning  = CMD_CLN_SCREW_0_POS;
			RX_TestTableStatus.info.moving = TRUE;
			motors_move_by_step(MOTOR_SCREW_0_BITS, &_ParScrew_turn, pos*SCREW_STEPS_PER_REV / 1000000.0, FALSE);
			break;
		case 1 :
			_CmdRunning  = CMD_CLN_SCREW_1_POS;
			RX_TestTableStatus.info.moving = TRUE;
			motors_move_by_step(MOTOR_SCREW_1_BITS, &_ParScrew_turn, pos*SCREW_STEPS_PER_REV / 1000000.0, FALSE);
			break;
		case 2 :
			_CmdRunning  = CMD_CLN_SCREW_2_POS;
			RX_TestTableStatus.info.moving = TRUE;
			motors_move_by_step(MOTOR_SCREW_2_BITS, &_ParScrew_turn, pos*SCREW_STEPS_PER_REV / 1000000.0, FALSE);
			break;
		case 3 :
			_CmdRunning  = CMD_CLN_SCREW_3_POS;
			RX_TestTableStatus.info.moving = TRUE;
			motors_move_by_step(MOTOR_SCREW_3_BITS, &_ParScrew_turn, pos*SCREW_STEPS_PER_REV / 1000000.0, FALSE);
			break;
		default:						Error(ERR_CONT, 0, "Screw Nr %d not implemented", screw); break;
		}
	}
}

//--- _cln_motor_test ---------------------------------
static void _cln_motor_test(int motorNo, int steps)
{
	int motor = 1 << motorNo;
	SMovePar par;
	int i;

	if (motorNo == MOTOR_CAP)
	{
		par.speed		= 10000;
		par.accel		= 3000;
		par.current		= 300.0;  // fot Tests: 50
	}
	else
	{
		par.speed		= 10000; // 40000; 
		par.accel		= 5000;  // 30000; 
		par.current		= 15.0;	// 7.0 // 5.0	 // 300				
	}	
	par.stop_mux	= 0;
	par.estop_in    = ESTOP_UNUSED;
	par.estop_level = 0;
	par.checkEncoder = FALSE;
	RX_TestTableStatus.info.moving = TRUE;
	_CmdRunning = 1; // TEST
	
	for (i = 0; i < MOTOR_CNT; i++) if (motor & (1 << i)) motor_config(i, CURRENT_HOLD, CAP_STEPS_PER_METER, CAP_INC_PER_METER);
	motors_move_by_step(motor, &par, steps, FALSE);			
}

//--- cln_error_reset ------------------------------------
void	_cln_error_reset(void)
{
	memset(_Cln_Error, 0, sizeof(_Cln_Error));
	// use: ErrorFlag (ERR_CONT, (UINT32*)&_Error[isNo],  1,  0, "InkSupply[%d] Ink Tank Sensor Error", isNo+1);
}

//--- _cln_send_status --------------------------------------------------
static void _cln_send_status(RX_SOCKET socket)
{
	static RX_SOCKET _socket = INVALID_SOCKET;
	if (socket != INVALID_SOCKET) _socket = socket;
	if (_socket != INVALID_SOCKET) sok_send_2(&_socket, INADDR_ANY, REP_TT_STATUS, sizeof(RX_TestTableStatus), &RX_TestTableStatus);	
}