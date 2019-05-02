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

// General
#define MOTOR_ALL_BITS		0x03
#define MAIN_PER_SEC		930			// Main executions per sec
#define STEPS_REV			(200*16)	// steps per motor revolution * 16 microsteps

// Rotation Motor // 200 steps/rev // max 4.2 Amp
#define CURRENT_HOLD_ROT	200.0

#define MOTOR_ROT			0
#define MOTOR_ROT_BITS		0x01

#define ROT_STEPS_PER_REV	3200.0 // 200 steps * 16
#define ROT_INC_PER_REV		4000.0 // 3'200 m / 0.8

#define SCREW_MIN_UNIT		4.0 // 360/4.0 = 90° minimal turn

// Shift Motor // 200 steps/rev // 2mm/rev // max 0.67 Amp
#define CURRENT_HOLD_SHIFT  7.0

#define MOTOR_SHIFT			1
#define MOTOR_SHIFT_BITS	0x02

#define SHIFT_STEPS_PER_METER	1600000.0 // 200 * 16 * / 2mm * 1000mm
#define SHIFT_INC_PER_METER		2000000.0 // 1600000.0 / 0.8

// MOTOR_ROT Positions
#define POS_ROT_CNT			4
#define POS_ROT_WET			(ROT_STEPS_PER_REV/4.0*0)
#define POS_ROT_VAC			(ROT_STEPS_PER_REV/4.0*1)
#define POS_ROT_WIPE		(ROT_STEPS_PER_REV/4.0*2)
#define POS_ROT_CAP			(ROT_STEPS_PER_REV/4.0*3)

// ENCODER_ROT Positions
#define POS_ENC_WET			(ROT_INC_PER_REV/4.0*0)
#define POS_ENC_VAC			(ROT_INC_PER_REV/4.0*1)
#define POS_ENC_WIPE		(ROT_INC_PER_REV/4.0*2)
#define POS_ENC_CAP			(ROT_INC_PER_REV/4.0*3)

// MOTOR_SHIFT Positions
#define POS_SHIFT_CNT		4
#define POS_SHIFT_REF		(SHIFT_STEPS_PER_METER*(0-0.001))
#define POS_SHIFT_CENTER	(SHIFT_STEPS_PER_METER*(0.024+0))
#define POS_SHIFT_START		(SHIFT_STEPS_PER_METER*(0.024+0.024))
#define POS_SHIFT_END		(SHIFT_STEPS_PER_METER*(0.024-0.024))

// Inputs
#define ROT_STORED_IN		0
#define SHIFT_STORED_IN		1

// Outputs
#define VAC_ON				0x001 // 0
#define PUMP_FLUSH_CAP		0x010 // 3 Flush 2
#define PUMP_WASTE_VAC		0x020 // 4 Waste 1
#define PUMP_FLUSH_WET		0x040 // 5 Flush 1
#define PUMP_WASTE_BASE		0x080 // 6 Waste 2
//#define PUMP_FLUSH_CAP		3
//#define PUMP_WASTE_VAC		4
//#define PUMP_FLUSH_WET		5
//#define PUMP_WASTE_BASE		6
#define RO_ALL_OUTPUTS		0x1FF //  ALL Outputs

// Vectors
static int	POS_ROT[POS_ROT_CNT] = {
	POS_ROT_CAP,	
	POS_ROT_WET,
	POS_ROT_VAC,
	POS_ROT_WIPE
};

static int	POS_ENC_ROT[POS_ROT_CNT] = {
	POS_ENC_CAP,	
	POS_ENC_WET,
	POS_ENC_VAC,
	POS_ENC_WIPE
};

static int	POS_SHIFT[POS_SHIFT_CNT] = {
	POS_SHIFT_REF,	
	POS_SHIFT_CENTER,
	POS_SHIFT_START,
	POS_SHIFT_END
};

//--- global  variables -----------------------------------------------------------------------------------------------------------------
static int	_CmdRunning = 0; 
static char	_CmdName[32];
static int	_ResetRetried = 0;
static int  _LastRobPosCmd = 0;
static int  _RobFunction = 0;
static int  _ShowStatus = 0;
static int	_NewCmd = 0;
static int	_NewCmdPos = 0;
static int	_TimeCnt = 0;
//static int	_Step;
//static int	_PrintPos;

static SMovePar	_ParRotRef;
static SMovePar	_ParRotSensRef;
static SMovePar	_ParRotDrive;

static SMovePar	_ParShiftSensRef;
static SMovePar	_ParShiftDrive;
static SMovePar	_ParShiftWet;
static SMovePar	_ParShiftVac;
static SMovePar	_ParShiftWipe;

static UINT32	_txrob_Error[5];

// Times
static int	_RotRefOffset = 0; // in steps
static int	_TimeWastePump = 5; //20; // in s
static int	_TimeFillCap = 14; // in s
static int	_SpeedShift = 21000; // in steps/s

//--- prototypes --------------------------------------------
static void _txrob_motor_test(int motor, int steps);
//static void _txrob_turn_screw_to_pos(int screw, int steps);
static void _txrob_error_reset(void);
static void _txrob_send_status(RX_SOCKET);

static int  _rot_rev_2_steps(int rev);
static int  _rot_steps_2_rev(int steps);

static int  _shift_micron_2_steps(int micron);
static int  _shift_steps_2_micron(int steps);

static int  _rot_pos_check(int steps);

//--- txrob_init --------------------------------------
void txrob_init(void)
{
	// Init Motors
	motor_config(MOTOR_ROT, CURRENT_HOLD_ROT, ROT_STEPS_PER_REV, ROT_INC_PER_REV);
	motor_config(MOTOR_SHIFT, CURRENT_HOLD_SHIFT, SHIFT_STEPS_PER_METER, SHIFT_INC_PER_METER);

	//--- movement parameters capping ----------------
	
	_ParRotRef.speed = 2; // 10; //90; // speed with max tork: 
	_ParRotRef.accel =  1; // 2; // 10;
	_ParRotRef.current = 150.0; // 200; // max 420 = 4.2 A
	_ParRotRef.stop_mux = 0;
	_ParRotRef.dis_mux_in = 1;
	_ParRotRef.stop_in = ESTOP_UNUSED;
	_ParRotRef.stop_level = 0;
	_ParRotRef.estop_in = ROT_STORED_IN; // Check Input 0
	_ParRotRef.estop_level = 1; // stopp when sensor on
	_ParRotRef.sensRef = TRUE;
	_ParRotRef.checkEncoder = TRUE;
	
	_ParRotSensRef.speed = 500; // speed with max tork: 3200/4
	_ParRotSensRef.accel =  500;
	_ParRotSensRef.current = 200.0; // max 420 = 4.2 A
	_ParRotSensRef.stop_mux = 0;
	_ParRotSensRef.dis_mux_in = 1;
	_ParRotSensRef.stop_in = ESTOP_UNUSED;
	_ParRotSensRef.stop_level = 0;
	_ParRotSensRef.estop_in = ROT_STORED_IN; // Check Input 0
	_ParRotSensRef.estop_level = 1; // stopp when sensor on
	_ParRotSensRef.sensRef = TRUE;
	_ParRotSensRef.checkEncoder = TRUE;

	_ParRotDrive.speed = 500; // speed with max tork:
	_ParRotDrive.accel = 500;
	_ParRotDrive.current = 300.0; // max 420 = 4.2 A
	_ParRotDrive.stop_mux = 0;
	_ParRotDrive.dis_mux_in = 0;
	_ParRotDrive.stop_in = ESTOP_UNUSED;
	_ParRotDrive.stop_level = 0;
	_ParRotDrive.estop_in = ESTOP_UNUSED;
	_ParRotDrive.estop_level = 1;
	_ParRotDrive.sensRef = FALSE;
	_ParRotDrive.checkEncoder = TRUE;
	
	//--- movement parameters screws ----------------
	
	_ParShiftSensRef.speed = 21000; // speed with max tork: 21'333
	_ParShiftSensRef.accel = 10000;
	_ParShiftSensRef.current = 67.0; //  max 67 = 0.67 A
	_ParShiftSensRef.stop_mux = 0;
	_ParShiftSensRef.dis_mux_in = 0;
	_ParShiftSensRef.stop_in = ESTOP_UNUSED;
	_ParShiftSensRef.stop_level = 0;
	_ParShiftSensRef.estop_in = SHIFT_STORED_IN;
	_ParShiftSensRef.estop_level = 1; // stopp when sensor on
	_ParShiftSensRef.checkEncoder = TRUE;

	_ParShiftDrive.speed = 21000; // speed with max tork: 21'333
	_ParShiftDrive.accel = 10000;
	_ParShiftDrive.current = 67.0; //  max 67 = 0.67 A
	_ParShiftDrive.stop_mux = 0;
	_ParShiftDrive.dis_mux_in = 0;
	_ParShiftDrive.stop_in = ESTOP_UNUSED;
	_ParShiftDrive.stop_level = 0;
	_ParShiftDrive.estop_in = ESTOP_UNUSED;
	_ParShiftDrive.estop_level = 0; 
	_ParShiftDrive.checkEncoder = TRUE;
	
	_ParShiftWet.speed = 21000; // speed with max tork: 21'333
	_ParShiftWet.accel = 10000;
	_ParShiftWet.current = 67.0; //  max 67 = 0.67 A
	_ParShiftWet.stop_mux = 0;
	_ParShiftWet.dis_mux_in = 0;
	_ParShiftWet.stop_in = ESTOP_UNUSED;
	_ParShiftWet.stop_level = 0;
	_ParShiftWet.estop_in = ESTOP_UNUSED;
	_ParShiftWet.estop_level = 0; // stopp when sensor on
	_ParShiftWet.checkEncoder = TRUE;
	
	_ParShiftVac.speed = 21000; // 21000; // speed with max tork: 21'333
	_ParShiftVac.accel = 10000;
	_ParShiftVac.current = 67.0; //  80.0; // max 134 = 1.34 A // Amps/Phase Parallel: 0.67 A
	_ParShiftVac.stop_mux = 0;
	_ParShiftVac.dis_mux_in = 0;
	_ParShiftVac.stop_in = ESTOP_UNUSED;
	_ParShiftVac.stop_level = 0;
	_ParShiftVac.estop_in = ESTOP_UNUSED;
	_ParShiftVac.estop_level = 0; // stopp when sensor on
	_ParShiftVac.checkEncoder = TRUE;
	
	_ParShiftWipe.speed = 21000; // 21000; // speed with max tork: 21'333
	_ParShiftWipe.accel = 10000;
	_ParShiftWipe.current = 67.0; //  80.0; // max 134 = 1.34 A // Amps/Phase Parallel: 0.67 A
	_ParShiftWipe.stop_mux = 0;
	_ParShiftWipe.dis_mux_in = 0;
	_ParShiftWipe.stop_in = ESTOP_UNUSED;
	_ParShiftWipe.stop_level = 0;
	_ParShiftWipe.estop_in = ESTOP_UNUSED;
	_ParShiftWipe.estop_level = 0; // stopp when sensor on
	_ParShiftWipe.checkEncoder = TRUE;
	
	//--- Outputs ----------------
	Fpga.par->output &= ~RO_ALL_OUTPUTS; // ALL OUTPUTS OFF
}

//---_rot_pos_check --------------------------------------------------------------
static int  _rot_pos_check_err(int steps)
{
	if (abs(POS_ROT[0] - steps) <= 1)
	{
		if (abs(POS_ENC_ROT[0] - Fpga.encoder[MOTOR_ROT].pos) <= 10)
		{
			return FALSE;
		}
		else
		{
			Error(ERR_CONT, 0, "Stepper: Command %s: Motor Encoder at invalid position", _CmdName);
		}
	}
	else if (abs(POS_ROT[1] - steps) <= 1)
	{
		if (abs(POS_ENC_ROT[1] - Fpga.encoder[MOTOR_ROT].pos) <= 10)
		{
			return FALSE;
		}
		else
		{
			Error(ERR_CONT, 0, "Stepper: Command %s: Motor Encoder at invalid position", _CmdName);
		}
	}
	else if (abs(POS_ROT[2] - steps) <= 1)
	{
		if (abs(POS_ENC_ROT[2] - Fpga.encoder[MOTOR_ROT].pos) <= 10)
		{
			return FALSE;
		}
		else
		{
			Error(ERR_CONT, 0, "Stepper: Command %s: Motor Encoder at invalid position", _CmdName);
		}
	}
	else if (abs(POS_ROT[3] - steps) <= 1)
	{
		if (abs(POS_ENC_ROT[3] - Fpga.encoder[MOTOR_ROT].pos) <= 10)
		{
			return FALSE;
		}
		else
		{
			Error(ERR_CONT, 0, "Stepper: Command %s: Motor Encoder at invalid position", _CmdName);
		}
	}
	else
	{
		Error(ERR_CONT, 0, "Stepper: Command %s: Motor Steps at invalid position", _CmdName);
	}
	return TRUE;
}

//--- txrob_main ------------------------------------------------------------------
void txrob_main(int ticks, int menu)
{
	int motor;
	
	motor_main(ticks, menu);
	
	if (!motors_init_done())
	{
		motor_config(MOTOR_ROT, CURRENT_HOLD_ROT, ROT_STEPS_PER_REV, ROT_INC_PER_REV);
		motor_config(MOTOR_SHIFT, CURRENT_HOLD_SHIFT, SHIFT_STEPS_PER_METER, SHIFT_INC_PER_METER);
	}
	
	// --- read Inputs ---
	RX_StepperStatus.info.z_in_ref = fpga_input(ROT_STORED_IN); // Reference Sensor Rotation
	RX_StepperStatus.info.x_in_ref = fpga_input(SHIFT_STORED_IN); // Reference Sensor Shift
	
	RX_StepperStatus.posZ = _rot_steps_2_rev(motor_get_step(MOTOR_ROT));
	RX_StepperStatus.posX = _shift_steps_2_micron(motor_get_step(MOTOR_SHIFT));
	
	// --- Set Position Flags ---
//	RX_StepperStatus.info.z_in_ref = ((RX_StepperStatus.info.ref_done == 1)
//		&& (abs(RX_StepperStatus.posZ - POS_STORED_UM) <= 10)
//		&& (RX_StepperStatus.info.x_in_ref == 1));
//	RX_StepperStatus.info.z_in_print = RX_StepperStatus.info.z_in_ref;
//	RX_StepperStatus.info.z_in_cap = ((RX_StepperStatus.info.ref_done == 1)
//		&& (abs(RX_StepperStatus.posZ - POS_PRINTHEADS_UM) <= 10)
//		&& (RX_StepperStatus.info.x_in_ref == 0));

	// --- set positions False while moving ---
	if (RX_StepperStatus.info.moving) //  && (Fpga.stat->moving != 0))
	{
		//RX_StepperStatus.info.z_in_ref = FALSE;
		RX_StepperStatus.info.z_in_print = FALSE;
		RX_StepperStatus.info.z_in_cap = FALSE;
		RX_StepperStatus.info.move_ok = FALSE;
	}
	if (_TimeCnt != 0) _TimeCnt--; // waiting time
	// --- Executed after each move ---
	if (_CmdRunning && motors_move_done(MOTOR_ALL_BITS) && (_TimeCnt == 0))
	{
		RX_StepperStatus.info.moving = FALSE;
		int loc_move_pos = _NewCmdPos;
		int loc_new_cmd = _NewCmd;
		
		Fpga.par->output &= ~VAC_ON; // VACUUM OFF
		Fpga.par->output &= ~PUMP_FLUSH_WET; // PUMP_FLUSH_WET OFF
		Fpga.par->output &= ~PUMP_WASTE_VAC; // Waste OFF

		// --- tasks after motor error ---
		if ((_CmdRunning != CMD_CLN_REFERENCE) && (_CmdRunning != CMD_CLN_ROT_REF))
		{		
			if (motors_error(MOTOR_ALL_BITS, &motor))
			{
				loc_new_cmd = FALSE;
				RX_StepperStatus.info.ref_done = FALSE;
				Error(ERR_CONT, 0, "Stepper: Command %s: Motor[%d] blocked", _CmdName, motor + 1);
				_CmdRunning = FALSE;
				_ResetRetried = 0;
				Fpga.par->output &= ~RO_ALL_OUTPUTS; // set all output to off
			}
			else if (_rot_pos_check_err(motor_get_step(MOTOR_ROT))) 
			{
				Error(ERR_CONT, 0, "CLN: Command %s: position corrupt, try referencing", _CmdName); 
				RX_StepperStatus.info.ref_done = FALSE; 
				_CmdRunning = FALSE; 
				loc_new_cmd = CMD_CLN_REFERENCE;
			}
		}
		
		// --- tasks after motor ref ---
		if (_CmdRunning == CMD_CLN_REFERENCE)
		{
			if (motors_error(MOTOR_ROT_BITS, &motor))
			//if(TRUE)
			{
				if (RX_StepperStatus.info.z_in_ref)
				{
					motors_reset(MOTOR_ROT_BITS);
					RX_StepperStatus.posZ = _rot_steps_2_rev(motor_get_step(MOTOR_ROT));
					loc_new_cmd = CMD_CLN_ROT_REF; //CMD_CLN_SHIFT_REF; // CMD_CLN_ROT_REF;
				}
				else
				{
					Error(ERR_CONT, 0, "Stepper: Command %s: No Sensor at reference deteced", _CmdName);
				}
			}
			else
			{
				Error(ERR_CONT, 0, "Stepper: Command %s: No mechanical reference deteced", _CmdName);
			}
		}
		
		if (_CmdRunning == CMD_CLN_ROT_REF)
			{
				//if (motors_error(MOTOR_ROT_BITS, &motor))
				if(TRUE)
				{
					motors_reset(MOTOR_ROT_BITS);
					RX_StepperStatus.posZ = _rot_steps_2_rev(motor_get_step(MOTOR_ROT));
					loc_new_cmd = CMD_CLN_SHIFT_REF; 
				}
				else
				{
					Error(ERR_CONT, 0, "Stepper: Command %s: No mechanical reference deteced", _CmdName);
				}
			}
		
		if (_CmdRunning == CMD_CLN_SHIFT_REF && _NewCmd == FALSE)
		{
			if (RX_StepperStatus.info.x_in_ref == TRUE)
			{
				loc_new_cmd = CMD_CLN_SHIFT_MOV; 
				loc_move_pos = 0; // Ref Pos
				motors_reset(MOTOR_SHIFT_BITS);
				RX_StepperStatus.posX = _shift_steps_2_micron(motor_get_step(MOTOR_SHIFT));
				RX_StepperStatus.info.ref_done = TRUE;
				_RobFunction = 1;
			}
			else
			{
				Error(ERR_CONT, 0, "Stepper: Command %s: Shift reference Sensor not deteced", _CmdName);
			}
		}

		// --- tasks after motor stop ---
		if (_CmdRunning == CMD_CLN_STOP)
		{
			_ResetRetried = 0;
		}
		
		//========================================== rot ==========================================
		if (_CmdRunning == CMD_CLN_MOVE_POS) 
		{
			switch (_RobFunction)
			{
			case 0:	// Cap
				loc_new_cmd = CMD_CLN_SHIFT_MOV; 
				loc_move_pos = 1; // center
				break;

			case 1:	// Wet wipe
				loc_new_cmd = CMD_CLN_SHIFT_MOV; 
				loc_move_pos = 2; // start
				break;
	
			case 2:	// Vacuum
				loc_new_cmd = CMD_CLN_SHIFT_MOV; 
				loc_move_pos = 2; // start
				break;
				
			case 3: // Wipe
				loc_new_cmd = CMD_CLN_SHIFT_MOV; 
				loc_move_pos = 2; // start
				break;
			}
		}
		
		//========================================== shift ==========================================
		if (_CmdRunning == CMD_CLN_SHIFT_MOV) 
		{
			switch (_LastRobPosCmd)
			{
			case 0:	// Ref
				motors_reset(MOTOR_SHIFT_BITS);
				RX_StepperStatus.posX = _shift_steps_2_micron(motor_get_step(MOTOR_SHIFT));
				break;

			case 1:	// Center
				loc_new_cmd = CMD_CLN_FILL_CAP; 
				break;
	
			case 2:	// Start
				switch (_RobFunction)
				{
				case 0:	// Cap
					break;

				case 1:	// Wet wipe
					Fpga.par->output |= PUMP_FLUSH_WET; // Flush Wet ON
					break;
	
				case 2:	// Vacuum
					Fpga.par->output |= VAC_ON; // VACUUM ON
					Fpga.par->output |= PUMP_WASTE_VAC; // Waste ON
					break;
				
				case 3: // Wipe
					break;
				}
				break;
				
			case 3: // End
				switch (_RobFunction)
				{
				case 0:	// Cap
					break;

				case 1:	// Wet wipe
					//loc_new_cmd = CMD_CLN_EMPTY_WASTE; 
					break;
	
				case 2:	// Vacuum
					loc_new_cmd = CMD_CLN_EMPTY_WASTE; 
					break;
				
				case 3: // Wipe
					break;
				}
				break;
			}
		}
		
		//========================================== timers ==========================================
		if (_CmdRunning == CMD_CLN_FILL_CAP) 
		{
			Fpga.par->output &= ~PUMP_FLUSH_CAP; // Flush Cap OFF
		}
		
		if (_CmdRunning == CMD_CLN_EMPTY_WASTE) 
		{
			Fpga.par->output &= ~PUMP_WASTE_BASE; // Waste base OFF
			Fpga.par->output &= ~PUMP_WASTE_VAC; // Waste base OFF
		}
		
		// --- Set Position Flags after Commands ---
		//RX_StepperStatus.info.move_ok = (RX_StepperStatus.info.ref_done && (abs(RX_StepperStatus.posZ - _LastRobPosCmd) <= 10));
		//RX_StepperStatus.info.x_in_ref		= (RX_StepperStatus.info.ref_done && (RX_StepperStatus.info.x_in_ref == 1));
		
		// --- Toggle to mark end of move ---
		if ((loc_new_cmd == FALSE) && (_CmdRunning != CMD_CLN_STOP) && (_CmdRunning != CMD_CLN_SET_WASTE_PUMP)) 
		{
			RX_StepperStatus.info.move_tgl = !RX_StepperStatus.info.move_tgl;
			//RX_StepperStatus.info.move_ok = (RX_StepperStatus.info.ref_done
			//	&& abs(RX_StepperStatus.posX - _LastRobPosCmd) <= 10);
		}

		// --- Reset Commands ---
		_CmdRunning = FALSE;
		_NewCmdPos = 0;
		_NewCmd = FALSE;

		// --- Execute next Commands ---
		switch (loc_new_cmd)
		{
		case CMD_CLN_REFERENCE: txrob_handle_ctrl_msg(INVALID_SOCKET, CMD_CLN_REFERENCE, NULL); break;
		case CMD_CLN_ROT_REF:	txrob_handle_ctrl_msg(INVALID_SOCKET, CMD_CLN_ROT_REF, NULL); break;
		case CMD_CLN_SHIFT_REF: txrob_handle_ctrl_msg(INVALID_SOCKET, CMD_CLN_SHIFT_REF, NULL); break;
		case CMD_CLN_MOVE_POS:	txrob_handle_ctrl_msg(INVALID_SOCKET, CMD_CLN_MOVE_POS, &loc_move_pos); break;
		case CMD_CLN_SHIFT_MOV: txrob_handle_ctrl_msg(INVALID_SOCKET, CMD_CLN_SHIFT_MOV, &loc_move_pos); break;
		case CMD_CLN_FILL_CAP :	txrob_handle_ctrl_msg(INVALID_SOCKET, CMD_CLN_FILL_CAP, NULL); break;
		case CMD_CLN_EMPTY_WASTE: txrob_handle_ctrl_msg(INVALID_SOCKET, CMD_CLN_EMPTY_WASTE, NULL); break;
		case 0: break;
		default:				Error(ERR_CONT, 0, "LBROB_MAIN: Command 0x%08x not implemented", loc_new_cmd); break;
		}
		loc_new_cmd = FALSE;
		loc_move_pos = 0;

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

//--- txrob_display_status ---------------------------------------------------------
static void _txrob_display_status(void)
{
	int i;
	
	term_printf("Inputs:       ");
	for (i = 0; i < INPUT_CNT; i++)
	{
		if (Fpga.stat->input & (1 << i))	term_printf("*");
		else								term_printf("_");
		if (i % 4 == 3)			   			term_printf("   ");
	}
	term_printf("\n");
	term_printf("Outputs:      ");
	for (i = 0; i < OUTPUT_CNT; i++)
	{
		if (Fpga.par->output & (1 << i))	term_printf("*");
		else								term_printf("_");
		if (i % 4 == 3)						term_printf("   ");
	}
	term_printf("\n");
	term_printf("\n");
	
	term_printf("FLO Robot Advanced mechref ---------------------------------\n");
	term_printf("moving:              %d		cmd: %08x\n", RX_StepperStatus.info.moving, _CmdRunning);
	term_printf("Sensor Ref Rotation: %d\n", RX_StepperStatus.info.z_in_ref);
	term_printf("Sensor Ref Shift:    %d\n", RX_StepperStatus.info.x_in_ref);
	term_printf("Ref done:            %d\n", RX_StepperStatus.info.ref_done);
	term_printf("Rotation in rev:     %06d  ", RX_StepperStatus.posZ);
	term_printf("Rotation in steps:   %06d\n", motor_get_step(MOTOR_ROT));
	term_printf("Rotation in encsteps:%06d\n", Fpga.encoder[MOTOR_ROT].pos);
	term_printf("Shift in um:         %06d  ", RX_StepperStatus.posX);
	term_printf("Shift in steps:      %06d\n", motor_get_step(MOTOR_SHIFT));
	term_printf("Rotation RefOffset:  %06d\n", _RotRefOffset);
	term_printf("Time Waste Vac [s]:  %06d\n", _TimeWastePump);
	term_printf("Time fill Cap [s]:   %06d\n", _TimeFillCap);
	term_printf("Speed Shift [mm/s]:  %06d\n", _SpeedShift * 2 / 200 / 16);
	term_printf("Timer Count:         %06d\n", _TimeCnt);
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
	
	term_printf("MENU FLO-ROBOT Advanced mechref -------------------------\n");
	term_printf("s: STOP\n");
	//term_printf("r<n>: reset motor<n>\n");
	term_printf("R: Reference\n");
	//term_printf("y: move rot\n");
	//term_printf("z: move shift\n");
	term_printf("c: move to capping\n");
	term_printf("e: move to wet wipe\n");
	term_printf("u: move to vacuum\n");
	term_printf("w: move to wipe\n");
	term_printf("b: shift back\n");
	term_printf("a<steps>: set Rotation RefOffset <steps>\n");
	term_printf("t<sec>: set Time Waste Vac <sec>\n");
	term_printf("f<sec>: set Time fill Cap <sec>\n");
	term_printf("v<mm/s>: set Speed Shift <mm/s>\n");
	//term_printf("m<n><steps>: move Motor<n> by <steps>\n");
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
		case 'o' : Fpga.par->output ^= (1 << atoi(&str[1])); break;
		case 'R': txrob_handle_ctrl_msg(INVALID_SOCKET, CMD_CLN_REFERENCE, NULL); break;
		//case 'y': pos = atoi(&str[1]); txrob_handle_ctrl_msg(INVALID_SOCKET, CMD_CLN_MOVE_POS, &pos); break;
		//case 'z': pos = atoi(&str[1]); txrob_handle_ctrl_msg(INVALID_SOCKET, CMD_CLN_SHIFT_MOV, &pos); break;
		case 'c': pos = 0; txrob_handle_ctrl_msg(INVALID_SOCKET, CMD_CLN_MOVE_POS, &pos); break;
		case 'e': pos = 1; txrob_handle_ctrl_msg(INVALID_SOCKET, CMD_CLN_MOVE_POS, &pos); break;
		case 'u': pos = 2; txrob_handle_ctrl_msg(INVALID_SOCKET, CMD_CLN_MOVE_POS, &pos); break;
		case 'w': pos = 3; txrob_handle_ctrl_msg(INVALID_SOCKET, CMD_CLN_MOVE_POS, &pos); break;
		case 'b': pos = 3; txrob_handle_ctrl_msg(INVALID_SOCKET, CMD_CLN_SHIFT_MOV, &pos); break;
		case 'a' : pos = atoi(&str[1]); 
			if (pos < 0) pos = 0;
			if (pos > 800) pos = 800;
			_RotRefOffset = pos;
			break;
		case 't' : pos = atoi(&str[1]); 
			if (pos < 0) pos = 0;
			if (pos > 300) pos = 300;
			_TimeWastePump = pos;
			break;
		case 'f' : pos = atoi(&str[1]); 
			if (pos < 0) pos = 0;
			if (pos > 300) pos = 300;
			_TimeFillCap = pos;
			break;
		case 'v' : pos  = (atoi(&str[1])) * 200 * 16 / 2; 
			if (pos < 500) pos = 500;
			if (pos > 21333) pos = 21333;
			_SpeedShift = pos;
			break;
		case 'm': _txrob_motor_test(str[1] - '0', atoi(&str[2])); break;
		case 'x': return FALSE;
		}
	}
	return TRUE;
}
	
//---_shift_micron_2_steps --------------------------------------------------------------
static int  _shift_micron_2_steps(int micron)
{
	return (int)(0.5 + micron * (SHIFT_STEPS_PER_METER / 1000000.0));
}

//---_shift_steps_2_micron --------------------------------------------------------------
static int  _shift_steps_2_micron(int steps)
{
	return (int)(steps * (1000000.0 / SHIFT_STEPS_PER_METER) + 0.5);
}

//---_rot_rev_2_steps --------------------------------------------------------------
static int  _rot_rev_2_steps(int rev)
{
	INT32 round = rev*SCREW_MIN_UNIT / 1000000.0;
	return (int)(0.5 + round / SCREW_MIN_UNIT * ROT_STEPS_PER_REV);
}

//---_rot_steps_2_rev --------------------------------------------------------------
static int  _rot_steps_2_rev(int steps)
{
	return (int)(steps * (1000000.0 / ROT_STEPS_PER_REV) + 0.5);
}

//--- txrob_handle_ctrl_msg -----------------------------------
int  txrob_handle_ctrl_msg(RX_SOCKET socket, int msgId, void *pdata)
{	
	int i;
	INT32 pos, steps;
	EnFluidCtrlMode	ctrlMode;
	
	switch(msgId)
	{
	case CMD_TT_STATUS:				sok_send_2(&socket, REP_TT_STATUS, sizeof(RX_StepperStatus), &RX_StepperStatus);	
									break;
		
	case CMD_CLN_STOP:				strcpy(_CmdName, "CMD_CLN_STOP");
		motors_stop(MOTOR_ALL_BITS); // Stops Hub and Robot
		Fpga.par->output &= ~RO_ALL_OUTPUTS; // set all output to off
		RX_StepperStatus.info.moving = TRUE;
		_CmdRunning = msgId;
		// reset var
		_TimeCnt = 0;
		_NewCmdPos = 0;
		_NewCmd = FALSE;
		break;

	case CMD_CLN_REFERENCE:			strcpy(_CmdName, "CMD_CLN_REFERENCE");
		if (_CmdRunning){ txrob_handle_ctrl_msg(INVALID_SOCKET, CMD_CLN_STOP, NULL); _NewCmd = CMD_CLN_REFERENCE; break; }
		motor_reset(MOTOR_ROT); // to recover from move count missalignment
		motor_reset(MOTOR_SHIFT); // to recover from move count missalignment
		Fpga.par->output &= ~RO_ALL_OUTPUTS;
		RX_StepperStatus.info.ref_done = FALSE;
		RX_StepperStatus.info.moving = TRUE;
		_CmdRunning = msgId;
		// reset var
		_TimeCnt = 0;
		_NewCmdPos = 0;
		_NewCmd = FALSE;
		//Fpga.par->pwm_output[0]    = 0;
		//Fpga.par->pwm_output[1]    = 0;
		//Fpga.par->pwm_output[2]    = 0;
		//Fpga.par->pwm_output[3]    = 0;
		motors_move_by_step(MOTOR_ROT_BITS, &_ParRotSensRef, -ROT_STEPS_PER_REV, TRUE);
		break;
		
	case CMD_CLN_ROT_REF:			strcpy(_CmdName, "CMD_CLN_ROT_REF");
		if (_CmdRunning){ txrob_handle_ctrl_msg(INVALID_SOCKET, CMD_CLN_STOP, NULL); _NewCmd = CMD_CLN_ROT_REF; break; }
		motor_reset(MOTOR_ROT); // to recover from move count missalignment
		Fpga.par->output &= ~RO_ALL_OUTPUTS;
		RX_StepperStatus.info.ref_done = FALSE;
		RX_StepperStatus.info.moving = TRUE;
		_CmdRunning = msgId;
		// reset var
		_TimeCnt = 0;
		_NewCmdPos = 0;
		_NewCmd = FALSE;
		//motors_move_by_step(MOTOR_ROT_BITS, &_ParRotRef, -ROT_STEPS_PER_REV, TRUE);
		motors_move_by_step(MOTOR_ROT_BITS, &_ParRotDrive, _RotRefOffset, TRUE);
		break;
		
	case CMD_CLN_SHIFT_REF:			strcpy(_CmdName, "CMD_CLN_SHIFT_REF");
		if (_CmdRunning){ txrob_handle_ctrl_msg(INVALID_SOCKET, CMD_CLN_STOP, NULL); _NewCmd = CMD_CLN_SHIFT_REF; break; }
		motor_reset(MOTOR_SHIFT); // to recover from move count missalignment
		Fpga.par->output &= ~RO_ALL_OUTPUTS;
		RX_StepperStatus.info.ref_done = FALSE;
		RX_StepperStatus.info.moving = TRUE;
		_CmdRunning = msgId;
		_NewCmd = FALSE;
		if (fpga_input(SHIFT_STORED_IN) == TRUE){ motors_move_by_step(MOTOR_SHIFT_BITS, &_ParShiftDrive, SHIFT_STEPS_PER_METER * 0.01, TRUE); _NewCmd = CMD_CLN_SHIFT_REF; break; }
		motors_move_by_step(MOTOR_SHIFT_BITS,&_ParShiftSensRef,-SHIFT_STEPS_PER_METER*0.06, TRUE);
		break;
		
	case CMD_CLN_MOVE_POS:		strcpy(_CmdName, "CMD_CLN_MOVE_POS");
		if (!_CmdRunning)
		{
			_CmdRunning = msgId;
			RX_StepperStatus.info.moving = TRUE;
			if (!RX_StepperStatus.info.ref_done){ Error(LOG, 0, "CLN: Command %s: missing ref_done", _CmdName); break;}
			pos = *((INT32*)pdata);
			if (pos < 0) {Error(LOG, 0, "CLN: Command %s: negative position not allowed", _CmdName); break;}
			if (pos >= POS_ROT_CNT) {Error(LOG, 0, "CLN: Command %s: too high pos", _CmdName); break;}
			if (_rot_pos_check_err(motor_get_step(MOTOR_ROT))) {Error(ERR_CONT, 0, "CLN: Command %s: position corrupt, cancel move", _CmdName); RX_StepperStatus.info.ref_done = FALSE; break;}
			_RobFunction = pos;
			pos = POS_ROT[pos];
			Fpga.par->output &= ~VAC_ON; // VACUUM OFF
			Fpga.par->output &= ~PUMP_FLUSH_WET; // PUMP_FLUSH_WET OFF
			Fpga.par->output &= ~PUMP_WASTE_VAC; // Waste OFF
			motors_move_to_step(MOTOR_ROT_BITS, &_ParRotDrive, pos);
		}
		break;
		
	case CMD_CLN_SHIFT_MOV:		strcpy(_CmdName, "CMD_CLN_SHIFT_MOV");
		if (!_CmdRunning)
		{
			_CmdRunning = msgId;
			RX_StepperStatus.info.moving = TRUE;
			if (!RX_StepperStatus.info.ref_done){ Error(LOG, 0, "CLN: Command %s: missing ref_done", _CmdName); break;}
			pos = *((INT32*)pdata);
			if (pos < 0) {Error(LOG, 0, "CLN: Command %s: negative position not allowed", _CmdName); break;}
			if (pos >= POS_ROT_CNT) {Error(LOG, 0, "CLN: Command %s: too high pos", _CmdName); break;}
			if (_RobFunction == 0 && pos != 1) {Error(LOG, 0, "CLN: Command %s: cancle shift, robot in capping", _CmdName); break;}
			_LastRobPosCmd = pos;
			if (pos == 3)
			{
				pos = POS_SHIFT[pos];
				_ParShiftWet.speed = _SpeedShift;
				motors_move_to_step(MOTOR_SHIFT_BITS, &_ParShiftWet, pos);
			}
			else
			{
				pos = POS_SHIFT[pos];
				motors_move_to_step(MOTOR_SHIFT_BITS, &_ParShiftDrive, pos);
			}
		}
		break;
		
		
	case CMD_CLN_FILL_CAP :		strcpy(_CmdName, "CMD_CLN_FILL_CAP");
		if (!_CmdRunning)
		{
			Fpga.par->output |= PUMP_FLUSH_CAP; // Flush Cap ON
			_CmdRunning = msgId;
			RX_StepperStatus.info.moving = TRUE;
			_TimeCnt = _TimeFillCap * MAIN_PER_SEC;
		}
		break;
		
	case CMD_CLN_EMPTY_WASTE :		strcpy(_CmdName, "CMD_CLN_EMPTY_WASTE");
		if (!_CmdRunning)
		{
			Fpga.par->output |= PUMP_WASTE_BASE; // Waste base ON
			Fpga.par->output |= PUMP_WASTE_VAC; // Waste base ON
			_CmdRunning = msgId;
			RX_StepperStatus.info.moving = TRUE;
			_TimeCnt = _TimeWastePump * MAIN_PER_SEC;
		}
		break;

	case CMD_ERROR_RESET:		_txrob_error_reset();
		fpga_stepper_error_reset();
		break;
		
	default:						Error(ERR_CONT, 0, "CLN: Command 0x%08x not implemented", msgId); break;
	}
	
	return TRUE;
}

//--- _txrob_motor_test ---------------------------------
static void _txrob_motor_test(int motorNo, int steps)
{
	int motors = 1 << motorNo;
	SMovePar par;
	int i;

	if (motorNo == 0)
	{
		par.speed		= 1000;
		par.accel		= 1000;
		par.current		= 300.0;
		motors_config(motors, CURRENT_HOLD_ROT, 0.0, 0.0);
	}
	else
	{
		par.speed	= 21000;		//21000;					//21000;				//21000;			//21000;			//21000;					
		par.accel	= 10000;		//10000;					//10000;				//10000;			//10000;			//10000;				
		par.current = 67.0;//134.0;	//40.0;	minimum for 0.3 Nm	// 60.0; for 0.4 Nm		// 80.0 for 0.5 Nm	// 100.0 for 0.6 Nm	// 120.0 for 0.7 Nm	// 134.0 for 0.8 Nm	
		motors_config(motors, CURRENT_HOLD_SHIFT, 0.0, 0.0);
	}
	
	par.stop_mux	= 0;
	par.dis_mux_in	= 0;
	par.stop_in		= ESTOP_UNUSED;
	par.stop_level	= 0;
	par.estop_in    = ESTOP_UNUSED;
	par.estop_level = 0;
	par.checkEncoder = FALSE;
	RX_StepperStatus.info.moving = TRUE;
	_CmdRunning = 1;
	
	motors_move_by_step(motors, &par, steps, FALSE);			
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
	if (_socket != INVALID_SOCKET) sok_send_2(&_socket, REP_TT_STATUS, sizeof(RX_StepperStatus), &RX_StepperStatus);
}