// ****************************************************************************
//
//	DIGITAL PRINTING - txrob.c
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
#include "txrob.h"
#include "stepper_ctrl.h"

//--- defines ------------------------------------------------

// General
#define MOTOR_ALL_ROBOT_BITS	0x03
#define MAIN_PER_SEC			930			// Main executions per sec
#define STEPS_REV				(200*16)	// steps per motor revolution * 16 microsteps
#define DIST_REV				1000.0		// moving distance per revolution on wrinkle detection motor [µm]

// Rotation Motor // 200 steps/rev // max 4.2 Amp
// #define CURRENT_HOLD_ROT	200.0
#define CURRENT_HOLD_ROT	50.0	// 2Amp Holding current is too high!

#define CURRENT_HOLD_WD		50.0
#define TX_PRINT_POS_MIN	1200.0
#define TX_REF_HEIGHT_WD	9000

#define MOTOR_ROT			0
#define MOTOR_ROT_BITS		0x01
#define MOTOR_ROT_VAR		40	// max allowed variance 

#define ROT_STEPS_PER_REV	3200.0 // 200 steps * 16
#define ROT_INC_PER_REV		16000	//(4*4000.0) // 3'200 m / 0.8

#define SCREW_MIN_UNIT		4.0 // 360/4.0 = 90° minimal turn

// Shift Motor // 200 steps/rev // 2mm/rev // max 0.67 Amp
#define CURRENT_HOLD_SHIFT  7.0

#define MOTOR_SHIFT			1
#define MOTOR_SHIFT_BITS	0x02
#define MOTOR_SHIFT_VAR		100 // 20 // 10 // max allowed variance 

#define MOTOR_WRINKLE_DETECTION_FRONT		2
#define MOTOR_WRINKLE_DETECTION_FRONT_BITS	0x04

#define MOTOR_WRINKLE_DETECTION_BACK		3
#define MOTOR_WRINKLE_DETECTION_BACK_BITS	0x08

#define MOTOR_WD_BITS					    0x0c
#define MOTOR_WD_CNT						2

#define SHIFT_STEPS_PER_METER	1600000.0 // 200 * 16 * / 2mm * 1000mm
#define SHIFT_INC_PER_METER		2000000.0 // 1600000.0 / 0.8

#define SPEED_SLOW_SHIFT	3200 // in steps/s

//#define ROB_FUNCTION_CAP	0
//#define ROB_FUNCTION_WASH	1
//#define ROB_FUNCTION_VACUUM	2
//#define ROB_FUNCTION_WIPE	3
//#define ROB_FUNCTION_TILT	4
#define ROB_FUNCTION_VACUUM_CHANGE	5



#define ROB_SIDE_MOVINGS	3

// MOTOR_ROT Positions
#define POS_ROT_CNT					6
#define POS_ROT_WIPE				(ROT_STEPS_PER_REV/4.0*0)
#define POS_ROT_VAC					(ROT_STEPS_PER_REV/4.0*1)
#define POS_ROT_WASH				(ROT_STEPS_PER_REV/4.0*2)
#define POS_ROT_CAP					(ROT_STEPS_PER_REV/4.0*3)
//#define POS_ROT_VAC_CHANGE			(ROT_STEPS_PER_REV/4.0*3)
#define POS_ROT_TILT				(POS_ROT_CAP-_TiltSteps)
#define POS_ROT_VAC_1_TO_4_TO_ALL	(ROT_STEPS_PER_REV/4.0*2.5)
#define POS_ROT_VAC_5_TO_8_TO_ALL	(ROT_STEPS_PER_REV/4.0*0.5)


// ENCODER_ROT Positions
#define POS_ENC_WIPE				(ROT_INC_PER_REV/4.0*0)
#define POS_ENC_VAC					(ROT_INC_PER_REV/4.0*1)
#define POS_ENC_WASH				(ROT_INC_PER_REV/4.0*2)
#define POS_ENC_CAP					(ROT_INC_PER_REV/4.0*3)
//#define POS_ENC_VAC_CHANGE			(ROT_INC_PER_REV/4.0*3)
#define POS_ENC_TILT				(POS_ROT_TILT*ROT_INC_PER_REV/ROT_STEPS_PER_REV)
#define POS_ENC_VAC_1_TO_4_TO_ALL	(POS_ROT_VAC_1_TO_4_TO_ALL*ROT_INC_PER_REV/ROT_STEPS_PER_REV)
#define POS_ENC_VAC_5_TO_8_TO_ALL	(POS_ROT_VAC_5_TO_8_TO_ALL*ROT_INC_PER_REV/ROT_STEPS_PER_REV)

// MOTOR_SHIFT Positions
#define POS_CENTER_OFFSET		(0.001+0.002) // 0.001 // in m
#define POS_SHIFT_CNT			7
#define POS_SHIFT_REF			(SHIFT_STEPS_PER_METER*(0/*-POS_CENTER_OFFSET*/))
#define POS_SHIFT_CENTER_801	(SHIFT_STEPS_PER_METER*(0.024-0.002))
#define POS_SHIFT_START			(SHIFT_STEPS_PER_METER*(0.024-0.024)) // (SHIFT_STEPS_PER_METER*(0.024+0.024))
#define POS_SHIFT_END_801		(SHIFT_STEPS_PER_METER*(0.024+0.023))	//(SHIFT_STEPS_PER_METER*(0.024+0.025)) // (SHIFT_STEPS_PER_METER*(0.024-0.024))
#define POS_SHIFT_CENTER_802	(SHIFT_STEPS_PER_METER*(0.024+0.020))
#define POS_SHIFT_END_802		(SHIFT_STEPS_PER_METER*(0.024+0.066))
#define POS_SHIFT_MAX_TURN		(SHIFT_STEPS_PER_METER*(0.024+0.055))
//#define POS_SHIFT_START		(SHIFT_STEPS_PER_METER*(0.024-0.024)) // (SHIFT_STEPS_PER_METER*(0.024+0.024))



#define POS_SHIFT_CENTER	 POS_SHIFT_CENTER_801

// Inputs
#define ROT_STORED_IN		0
#define SHIFT_STORED_IN		1
#define ROT_VAC_OR_WASH_IN	2
#define ROT_WASH_OR_CAP_IN	3
#define WD_FRONT_STORED_IN	4
#define WD_BACK_STORED_IN	5
#define WRINKLE_DETECTED	6

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
#define CAP_POS					0
#define WASH_POS				1
#define	VAC_POS					2
#define WIPE_POS				3
#define VAC_POS_1_TO_4_TO_ALL	4
#define VAC_POS_5_TO_8_TO_ALL	5
#define POS_SHIFT_MAX_TURN_POS	6

static int	POS_ROT[POS_ROT_CNT] = {
	POS_ROT_CAP,	
	POS_ROT_WASH,
	POS_ROT_VAC,
	POS_ROT_WIPE,
	POS_ROT_VAC_1_TO_4_TO_ALL,
	POS_ROT_VAC_5_TO_8_TO_ALL,
};

static int	POS_ENC_ROT[POS_ROT_CNT] = {
	POS_ENC_CAP,	
	POS_ENC_WASH,
	POS_ENC_VAC,
	POS_ENC_WIPE,
	POS_ENC_VAC_1_TO_4_TO_ALL,
	POS_ENC_VAC_5_TO_8_TO_ALL,
};

static int	POS_SHIFT[POS_SHIFT_CNT] = {
	POS_SHIFT_REF,	
	POS_SHIFT_CENTER_801,
	POS_SHIFT_START,
	POS_SHIFT_END_801,
	POS_SHIFT_START,
	POS_SHIFT_START,
	POS_SHIFT_MAX_TURN
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
static int  _first_move = 0;
//static int  _stop_reached = 0;
static int _CapTilt = 0;
static int _MoveLeftPos = 0;

static int _WipeWaiting = FALSE;
static int _VacuumWaiting = FALSE;

static int _PosShiftCenter = 0;
static int _PosShiftEnd = 0;

static int _Reference_Count = 0;
static int _Max_Reference = 3;

// static SMovePar	_ParRotRef;
static SMovePar	_ParRotSensRef;
static SMovePar	_ParRotDrive;

static SMovePar	_ParShiftSensRef;
static SMovePar	_ParShiftDrive;
static SMovePar	_ParShiftWet;
static SMovePar	_ParShiftVac;
static SMovePar	_ParShiftWipe;

static ERobotVacuumState _VacuumState;

static SMovePar _ParWDRef;
static SMovePar _ParWDDrive;

static UINT32	_txrob_Error[5];

// variables for Wrinkle Detection
static int _CmdRunningWD = 0;
static int _PrintPos_New = 0;
static int _PrintPos_Act = 0;
static int _WD_in_print = 0;
static int _WrinkleDetected = 0;


// Times and Parameters
static int	_TimeWastePump = 5; //20; // in s
static int	_TimeFillCap = 14; // in s
static int	_TimeFillCap801 = 14;	// in s
static int	_TimeFillCap802 = 25;	// in s
static int	_SpeedShift = 19000; // in steps/s
static int	_RotRefOffset = 11; // in steps
static int _TiltSteps = 90;     // in steps

// Times for Wrinkle Detection
static int _TimeWrinkleDetected = 2000;	// in ms

//--- prototypes --------------------------------------------
static void _txrob_motor_test(int motor, int steps);
//static void _txrob_turn_screw_to_pos(int screw, int steps);
static void _txrob_error_reset(void);
static void _txrob_send_status(RX_SOCKET);
static void _txrob_control(RX_SOCKET socket, EnFluidCtrlMode ctrlMode);

static int  _rot_rev_2_steps(int rev);
static int  _rot_steps_2_rev(int steps);

static int  _shift_micron_2_steps(int micron);
static int  _shift_steps_2_micron(int steps);

static int  _rot_pos_check(int steps);

static int  _step_rob_in_wipe(void);
static int  _step_rob_in_wash(void);
static int  _step_rob_in_vacuum(void);
static int  _step_rob_in_capping(void);

static int  _wipe_done(void);
static int  _wash_done(void);
static int  _vacuum_done(void);
static int  _vacuum_in_change(void);

static int _robot_left(void);

// Functions for the wrinkle detection
static void _tx801_wd_main(void);
static void _tx801_wrinkledetection_do_reference(void);
static int _micron_2_steps(int micron);
static int _steps_2_micron(int steps);
static void _tx801_wd_move_to_pos(int cmd, int pos);

//--- txrob_init --------------------------------------
void txrob_init(void)
{
	// Init Motors
	motor_config(MOTOR_ROT,   CURRENT_HOLD_ROT,   ROT_STEPS_PER_REV, ROT_INC_PER_REV, MICROSTEPS);
    motor_config(MOTOR_SHIFT, CURRENT_HOLD_SHIFT, SHIFT_STEPS_PER_METER, SHIFT_INC_PER_METER, STEPS);

	//--- movement parameters capping ----------------
	
	_ParRotSensRef.speed = 400; // speed with max tork: 3200/4
	_ParRotSensRef.accel =  400;
	_ParRotSensRef.current_acc = 210.0;	//210.0; // max 420 = 4.2 A
	_ParRotSensRef.current_run = 210.0;	//210.0; // max 420 = 4.2 A
	_ParRotSensRef.stop_mux = 0;
	_ParRotSensRef.dis_mux_in = 0;
	//_ParRotSensRef.estop_in_bit[MOTOR_ROT] = 1<<ROT_STORED_IN; // Check Input 0
	//_ParRotSensRef.estop_level = 1; // stopp when sensor on
	_ParRotSensRef.encCheck		= chk_txrob_ref;

	_ParRotDrive.speed		 = 1000; // speed with max tork:
	_ParRotDrive.accel		 = 1000;
	_ParRotDrive.current_acc = 400.0; // max 420 = 4.2 A
	_ParRotDrive.current_run = 400.0; // max 420 = 4.2 A
	_ParRotDrive.stop_mux	 = 0;
	_ParRotDrive.dis_mux_in  = 0;
	_ParRotDrive.encCheck	 = chk_std;
	
	//--- movement parameters shift motor ----------------
	_ParShiftSensRef.speed		 = 21000; // speed with max tork: 21'333
	_ParShiftSensRef.accel		 = 10000;
	_ParShiftSensRef.current_acc = 40.0; //  max 67 = 0.67 A
	_ParShiftSensRef.current_run = 40.0; //  max 67 = 0.67 A
	_ParShiftSensRef.stop_mux	 = 0;
	_ParShiftSensRef.dis_mux_in  = 0;
	//_ParShiftSensRef.estop_in_bit[MOTOR_SHIFT] = 1<<SHIFT_STORED_IN; // Check Input 1
	//_ParShiftSensRef.estop_level = 1; // stopp when sensor on
	_ParShiftSensRef.encCheck	 = chk_std;

	_ParShiftDrive.speed		= 21000; // speed with max tork: 21'333
	_ParShiftDrive.accel		= 10000;
	_ParShiftDrive.current_acc	= 67.0; //  max 67 = 0.67 A
	_ParShiftDrive.current_run	= 67.0; //  max 67 = 0.67 A
	_ParShiftDrive.stop_mux		= 0;
	_ParShiftDrive.dis_mux_in	= 0;
	_ParShiftDrive.encCheck		= chk_std;
	
	_ParShiftWet.speed			= 21000; // speed with max tork: 21'333
	_ParShiftWet.accel			= 10000;
	_ParShiftWet.current_acc	= 67.0; //  max 67 = 0.67 A
	_ParShiftWet.current_run	= 67.0; //  max 67 = 0.67 A
	_ParShiftWet.stop_mux		= 0;
	_ParShiftWet.dis_mux_in		= 0;
	_ParShiftWet.encCheck		= chk_std;
	
	_ParShiftVac.speed			= 21000; // 21000; // speed with max tork: 21'333
	_ParShiftVac.accel			= 10000;
	_ParShiftVac.current_acc	= 67.0; //  80.0; // max 134 = 1.34 A // Amps/Phase Parallel: 0.67 A
	_ParShiftVac.current_run	= 67.0; //  80.0; // max 134 = 1.34 A // Amps/Phase Parallel: 0.67 A
	_ParShiftVac.stop_mux		= 0;
	_ParShiftVac.dis_mux_in		= 0;
	_ParShiftVac.encCheck		= chk_std;
	
	_ParShiftWipe.speed			= 21000; // 21000; // speed with max tork: 21'333
	_ParShiftWipe.accel			= 10000;
	_ParShiftWipe.current_acc	= 67.0; //  80.0; // max 134 = 1.34 A // Amps/Phase Parallel: 0.67 A
	_ParShiftWipe.current_run	= 67.0; //  80.0; // max 134 = 1.34 A // Amps/Phase Parallel: 0.67 A
	_ParShiftWipe.stop_mux		= 0;
	_ParShiftWipe.dis_mux_in	= 0;
	_ParShiftWipe.encCheck		= chk_std;

    //--- movement parameters Wrinkle Detection motors ----------------

    motors_config(MOTOR_WD_BITS, CURRENT_HOLD_WD, L3518_STEPS_PER_METER, L3518_INC_PER_METER, STEPS);
    
    _ParWDRef.speed = 10000;
    _ParWDRef.accel = 32000;
    _ParWDRef.current_acc = 150;
    _ParWDRef.current_run = 100;
    _ParWDRef.estop_in_bit[0] = (1 << WD_FRONT_STORED_IN);
    _ParWDRef.estop_in_bit[1] = (1 << WD_BACK_STORED_IN);
    _ParWDRef.estop_level = 1;
    _ParWDRef.stop_mux = MOTOR_WD_BITS;
    _ParWDRef.dis_mux_in = TRUE;
    _ParWDRef.encCheck = chk_std;

    _ParWDDrive.speed = 10000;
    _ParWDDrive.accel = 32000;
    _ParWDDrive.current_acc = 200.0;
    _ParWDDrive.current_run = 100.0;
    _ParWDDrive.stop_mux = MOTOR_WD_BITS;
    _ParWDDrive.dis_mux_in = FALSE;
    _ParWDDrive.encCheck = chk_std;
	
	//--- Outputs ----------------
	Fpga.par->output &= ~RO_ALL_OUTPUTS; // ALL OUTPUTS OFF
	
	// Initalize the differences between TX801 and TX802
	if (RX_StepperCfg.printerType == printer_TX801)
	{
		int temp;
		POS_SHIFT[1] = POS_SHIFT_CENTER_801;
		POS_SHIFT[3] = POS_SHIFT_END_801;
		_TimeFillCap = 14;		// seconds

	}
	else if (RX_StepperCfg.printerType == printer_TX802)
	{
		POS_SHIFT[1] = POS_SHIFT_CENTER_802;
		POS_SHIFT[3] = POS_SHIFT_END_802;
		_TimeFillCap = 25;		// seconds
	}
}

//---_rot_pos_check --------------------------------------------------------------
static int  _rot_pos_check_err(int steps)
{
	//if (_stop_reached == 1) return FALSE;
	int i;
	if (abs(POS_ROT[CAP_POS] - steps) <= MOTOR_ROT_VAR)
	{
		if (abs(POS_ENC_ROT[CAP_POS] - Fpga.encoder[MOTOR_ROT].pos) <= MOTOR_SHIFT_VAR)
		{
			_VacuumState = rob_vacuum_5_to_8;
			return FALSE;
		}
		else
		{
            i = Fpga.encoder[MOTOR_ROT].pos;
			Error(ERR_CONT, 0, "Stepper: Command %s: Motor Encoder at invalid position", _CmdName);
		}
	}
	else if (abs(POS_ROT_TILT - steps) <= MOTOR_ROT_VAR)
	{
		if (abs(POS_ENC_TILT - Fpga.encoder[MOTOR_ROT].pos) <= MOTOR_SHIFT_VAR)
		{
			return FALSE;
		}
		else
		{
			Error(ERR_CONT, 0, "Stepper: Command %s: Motor Encoder at invalid position", _CmdName);
		}
	}
	else if (abs(POS_ROT[WASH_POS] - steps) <= MOTOR_ROT_VAR)
	{
		if (abs(POS_ENC_ROT[WASH_POS] - Fpga.encoder[MOTOR_ROT].pos) <= MOTOR_SHIFT_VAR)
		{
			return FALSE;
		}
		else
		{	
			i = Fpga.encoder[MOTOR_ROT].pos;
			Error(ERR_CONT, 0, "Stepper: Command %s: Motor Encoder at invalid position", _CmdName);
		}
	}
	else if (abs(POS_ROT[VAC_POS] - steps) <= MOTOR_ROT_VAR)
	{
		if (abs(POS_ENC_ROT[VAC_POS] - Fpga.encoder[MOTOR_ROT].pos) <= MOTOR_SHIFT_VAR)
		{
			return FALSE;
		}
		else
		{
			i = Fpga.encoder[MOTOR_ROT].pos;
			Error(ERR_CONT, 0, "Stepper: Command %s: Motor Encoder at invalid position", _CmdName);
		}
	}
	else if (abs(POS_ROT[WIPE_POS] - steps) <= MOTOR_ROT_VAR)
	{
		if (abs(POS_ENC_ROT[WIPE_POS] - Fpga.encoder[MOTOR_ROT].pos) <= MOTOR_SHIFT_VAR)
		{
			_VacuumState = rob_vacuum_1_to_4;
			return FALSE;
		}
		else
		{
			Error(ERR_CONT, 0, "Stepper: Command %s: Motor Encoder at invalid position", _CmdName);
		}
	}
	else if (abs(POS_ROT[VAC_POS_1_TO_4_TO_ALL] - steps) <= MOTOR_ROT_VAR)
	{
		if (abs(POS_ENC_ROT[VAC_POS_1_TO_4_TO_ALL] - Fpga.encoder[MOTOR_ROT].pos) <= MOTOR_SHIFT_VAR)
		{
			_VacuumState = rob_vacuum_all;
			return FALSE;
		}
		else
		{
			Error(ERR_CONT, 0, "Stepper: Command %s: Motor Encoder at invalid position", _CmdName);
		}
	}
	else if (abs(POS_ROT[VAC_POS_5_TO_8_TO_ALL] - steps) <= MOTOR_ROT_VAR)
	{
		if (abs(POS_ENC_ROT[VAC_POS_5_TO_8_TO_ALL] - Fpga.encoder[MOTOR_ROT].pos) <= MOTOR_SHIFT_VAR)
		{
			_VacuumState = rob_vacuum_all;
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
		motor_config(MOTOR_ROT, CURRENT_HOLD_ROT, ROT_STEPS_PER_REV, ROT_INC_PER_REV, MICROSTEPS);
        motor_config(MOTOR_SHIFT, CURRENT_HOLD_SHIFT, SHIFT_STEPS_PER_METER, SHIFT_INC_PER_METER, STEPS);
	}

    // --- read Inputs ---
	RX_StepperStatus.robinfo.z_in_ref = fpga_input(ROT_STORED_IN); // Reference Sensor Rotation
	RX_StepperStatus.robinfo.x_in_ref = fpga_input(SHIFT_STORED_IN); // Reference Sensor Shift
	RX_StepperStatus.robinfo.rob_in_wipe = fpga_input(ROT_STORED_IN) && !fpga_input(ROT_VAC_OR_WASH_IN) && !fpga_input(ROT_WASH_OR_CAP_IN);
	RX_StepperStatus.robinfo.rob_in_vac = fpga_input(ROT_VAC_OR_WASH_IN) && !fpga_input(ROT_WASH_OR_CAP_IN) && !fpga_input(ROT_STORED_IN);
	RX_StepperStatus.robinfo.rob_in_wash = fpga_input(ROT_VAC_OR_WASH_IN) && fpga_input(ROT_WASH_OR_CAP_IN) && !fpga_input(ROT_STORED_IN);
	RX_StepperStatus.robinfo.rob_in_cap = fpga_input(ROT_WASH_OR_CAP_IN) && !fpga_input(ROT_VAC_OR_WASH_IN) && !fpga_input(ROT_STORED_IN);
    if (fpga_input(ROT_STORED_IN) && (fpga_input(ROT_VAC_OR_WASH_IN) || fpga_input(ROT_WASH_OR_CAP_IN)))
	{
		Error(ERR_CONT, 0, "CLN Command %s: Invalid position sensors combination", _CmdName);
		RX_StepperStatus.robinfo.ref_done = FALSE; 
		_CmdRunning = FALSE;
		_NewCmd = FALSE;
	}
	
	RX_StepperStatus.posZ = _rot_steps_2_rev(motor_get_step(MOTOR_ROT));
	RX_StepperStatus.posX = _shift_steps_2_micron(motor_get_step(MOTOR_SHIFT));

	// --- set positions False while moving ---
	if (RX_StepperStatus.robinfo.moving) //  && (Fpga.stat->moving != 0))
	{
		RX_StepperStatus.robinfo.z_in_print = FALSE;
		RX_StepperStatus.robinfo.z_in_cap = FALSE;
		RX_StepperStatus.robinfo.move_ok = FALSE;
		RX_StepperStatus.robinfo.wipe_ready = FALSE;
		RX_StepperStatus.robinfo.wipe_done = FALSE;
		RX_StepperStatus.robinfo.vacuum_ready = FALSE;
		RX_StepperStatus.robinfo.vacuum_done = FALSE;
		RX_StepperStatus.robinfo.vacuum_in_change = FALSE;
		RX_StepperStatus.robinfo.vacuum_in_change = FALSE;
		RX_StepperStatus.robinfo.wash_ready = FALSE;
		RX_StepperStatus.robinfo.wash_done = FALSE;
	}
	else
	{
		RX_StepperStatus.robinfo.wipe_ready = _step_rob_in_wipe();
		RX_StepperStatus.robinfo.wipe_done = _wipe_done();
		RX_StepperStatus.robinfo.vacuum_ready = _step_rob_in_vacuum();
		RX_StepperStatus.robinfo.vacuum_done = _vacuum_done();
		RX_StepperStatus.robinfo.vacuum_in_change = _vacuum_in_change();
		RX_StepperStatus.robinfo.wash_ready = _step_rob_in_wash();
		RX_StepperStatus.robinfo.wash_done = _wash_done();
        _ParRotSensRef.encCheck = chk_txrob_ref;
    }

    if (RX_StepperStatus.robinfo.ref_done) _Reference_Count = 0;


    if (_TimeCnt != 0) _TimeCnt--; // waiting time
    // --- Executed after each move ---
    if (_CmdRunning && motors_move_done(MOTOR_ALL_ROBOT_BITS) && (_TimeCnt == 0))
    {
		TrPrintfL(TRUE, "Move done >>%s<<", _CmdName);
        RX_StepperStatus.robinfo.moving = FALSE;
        int loc_move_pos = _NewCmdPos;
        int loc_new_cmd = _NewCmd;

        Fpga.par->output &= ~PUMP_FLUSH_WET; // PUMP_FLUSH_WET OFF
        Fpga.par->output &= ~PUMP_WASTE_VAC; // Waste OFF

        // --- tasks after motor error ---
        if ((_CmdRunning != CMD_CLN_REFERENCE) && (_CmdRunning != CMD_CLN_ROT_REF) && (_CmdRunning != CMD_CLN_ROT_REF2) && (_CmdRunning != FALSE) && (_CmdRunning != CMD_CLN_SHIFT_REF))
        {
            if (motors_error(MOTOR_ALL_ROBOT_BITS, &motor))
            {
                loc_new_cmd = FALSE;
                RX_StepperStatus.robinfo.ref_done = FALSE;
                Error(ERR_CONT, 0, "Stepper: Command %s: Motor[%d] blocked", _CmdName, motor + 1);
                _CmdRunning = FALSE;
                _ResetRetried = 0;
                Fpga.par->output &= ~RO_ALL_OUTPUTS; // set all output to off
            }
            else if (_rot_pos_check_err(motor_get_step(MOTOR_ROT)))
            {
                Error(ERR_CONT, 0, "CLN: Command %s: position corrupt, try referencing", _CmdName);
                RX_StepperStatus.robinfo.ref_done = FALSE;
                _CmdRunning = FALSE;
                loc_new_cmd = FALSE;
            }
        }


        // --- tasks after motor ref ---
        if (_CmdRunning == CMD_CLN_REFERENCE)
        {
            loc_new_cmd = CMD_CLN_SHIFT_REF; // CMD_CLN_SHIFT_REF; // CMD_CLN_ROT_REF;
        }

        if (_CmdRunning == CMD_CLN_SHIFT_REF && _NewCmd == FALSE)
        {
            RX_StepperStatus.robinfo.x_in_ref = fpga_input(SHIFT_STORED_IN); // Reference Sensor Shift
            if (RX_StepperStatus.robinfo.x_in_ref == TRUE)
            {
                loc_new_cmd = CMD_CLN_ROT_REF;
                loc_move_pos = 0; // Ref Pos
                motors_reset(MOTOR_SHIFT_BITS);
                RX_StepperStatus.posX = _shift_steps_2_micron(motor_get_step(MOTOR_SHIFT));
                _RobFunction = rob_fct_tilt; // 5; // not 0, otherwise shift is canceled
            }
            else
            {
                Error(ERR_CONT, 0, "Stepper: Command %s: Shift reference Sensor not deteced", _CmdName);
            }
        }

        if (_CmdRunning == CMD_CLN_ROT_REF && _NewCmd == FALSE)
        {
			RX_StepperStatus.robinfo.z_in_ref = fpga_input(ROT_STORED_IN); // Reference Sensor Rotation
            if (motors_error(MOTOR_ROT_BITS, &motor))
            {
                if (RX_StepperStatus.robinfo.z_in_ref)
                {
                    motors_reset(MOTOR_ROT_BITS);
                    RX_StepperStatus.posZ = _rot_steps_2_rev(motor_get_step(MOTOR_ROT));
                    loc_new_cmd = CMD_CLN_ROT_REF2;
                }
                else
                {
                    if (_Reference_Count >= _Max_Reference)
                    {
                        Error(ERR_CONT, 0, "Stepper: Command %s: No Sensor at reference deteced", _CmdName);
                        _Reference_Count = 0;
                    }
                    else
                    {
                        loc_new_cmd = CMD_CLN_ROT_REF;
                    }
                }
            }
            else
            {
                Error(ERR_CONT, 0, "Stepper: Command %s: No mechanical reference deteced", _CmdName);
            }
        }

        if (_CmdRunning == CMD_CLN_ROT_REF2)
        {
            motors_reset(MOTOR_ROT_BITS);
            RX_StepperStatus.posZ = _rot_steps_2_rev(motor_get_step(MOTOR_ROT));
            RX_StepperStatus.robinfo.ref_done = TRUE;
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
			case rob_fct_cap:	// Cap
				TrPrintfL(TRUE, "ROB_FUNCTION_CAP done");
				loc_new_cmd = CMD_CLN_SHIFT_MOV; 
				loc_move_pos = 1; // center
				break;

			case rob_fct_wash:	// Wash
				TrPrintfL(TRUE, "ROB_FUNCTION_WASH done");
				loc_new_cmd = CMD_CLN_SHIFT_MOV; 
				loc_move_pos = 2; // start
				break;
	
			case rob_fct_vacuum:	// Vacuum
				TrPrintfL(TRUE, "ROB_FUNCTION_VACUUM done");
				loc_new_cmd = CMD_CLN_SHIFT_MOV; 
				loc_move_pos = 2; // start
				_VacuumWaiting = TRUE;
				Fpga.par->output |= VAC_ON; // VACUUM ON
				break;
				
			case rob_fct_wipe: // Wipe
				TrPrintfL(TRUE, "ROB_FUNCTION_WIPE done");
				loc_new_cmd = CMD_CLN_SHIFT_MOV;
				loc_move_pos = 2; // start
				_WipeWaiting = TRUE;
				break;
				
			case rob_fct_tilt: // Tilt
				TrPrintfL(TRUE, "ROB_FUNCTION_TILT done");
				_RobFunction = rob_fct_cap;	//0; // Robot in cap
				RX_StepperStatus.robinfo.cap_ready = TRUE;
				break;
				
			case rob_fct_vacuum_change:
			case rob_fct_vacuum_all:
				TrPrintfL(TRUE, "ROB_FUNCTION_VACUUM_ALL done");
				loc_new_cmd = CMD_CLN_MOVE_POS;
				loc_move_pos = VAC_POS;
				break;
			}
		}
		
		if (_CmdRunning == CMD_CLN_TILT_CAP) 
		{	
			TrPrintfL(TRUE, "CMD_CLN_TILT_CAP done");
			loc_new_cmd = CMD_CLN_WAIT;  // Tilt wait
			loc_move_pos = 2;
		}
		
		if (_CmdRunning == CMD_CLN_WAIT) 
		{
			TrPrintfL(TRUE, "CMD_CLN_WAIT done");
			loc_new_cmd = CMD_CLN_MOVE_POS; 
			loc_move_pos = rob_fct_tilt; // Tilt end, move to Cap
		}
		
		//========================================== shift ==========================================
		if (_CmdRunning == CMD_CLN_SHIFT_MOV) 
		{
			RX_StepperStatus.robinfo.move_ok = TRUE;
		
			TrPrintfL(TRUE, "CMD_CLN_SHIFT_MOV done, _LastRobPosCmd=%d, _RobFunction=%d", _LastRobPosCmd, _RobFunction);

			switch (_LastRobPosCmd)
			{
			case 0:	// Ref
				motors_reset(MOTOR_SHIFT_BITS);
				RX_StepperStatus.posX = _shift_steps_2_micron(motor_get_step(MOTOR_SHIFT));
				RX_StepperStatus.robinfo.ref_done = TRUE;
				break;

			case 1:	// Center
				loc_new_cmd = CMD_CLN_FILL_CAP; 
				break;
	
			case 2:	// Start
				switch (_RobFunction)
				{
				case rob_fct_cap:	// Cap
					break;

				case rob_fct_wash:	// Wash
					Fpga.par->output |= PUMP_FLUSH_WET; // Flush Wet ON
					break;
	
				case rob_fct_vacuum:	// Vacuum
					Fpga.par->output |= PUMP_WASTE_VAC; // Waste ON
					break;
				
				case rob_fct_wipe: // Wipe
					break;
					
				case rob_fct_tilt: // Tilt
					break;
				}
				break;
				
			case 3: // End
				switch (_RobFunction)
				{
				case rob_fct_cap:	// Cap
					break;

				case rob_fct_wash:	// Wash
					break;
	
				case rob_fct_vacuum:	// Vacuum -> drive back
					Fpga.par->output |= PUMP_WASTE_VAC; // Waste ON
					break;
				
				case rob_fct_wipe: // Wipe
					break;
					
				case rob_fct_tilt: // Tilt
					break;
				}
				break;
			case 4: // End for Vacuum
				loc_new_cmd = CMD_CLN_VACUUM;
				break;
			}
		}
		else
		{
			RX_StepperStatus.robinfo.move_ok = FALSE;
		}
		
		//========================================== timers ==========================================
		if (_CmdRunning == CMD_CLN_FILL_CAP) 
		{
			Fpga.par->output &= ~PUMP_FLUSH_CAP; // Flush Cap OFF
			loc_new_cmd = CMD_CLN_TILT_CAP; // empty cap partially by tilting the cap
		}
		
		if (_CmdRunning == CMD_CLN_VACUUM) 
		{
			loc_new_cmd = CMD_CLN_EMPTY_WASTE;
			Fpga.par->output &= ~PUMP_WASTE_BASE; // Waste base OFF
			Fpga.par->output &= ~PUMP_WASTE_VAC; // Waste base OFF
			Fpga.par->output &= ~VAC_ON; // VACUUM OFF
		}
		
		if (_CmdRunning == CMD_CLN_EMPTY_WASTE) 
		{
			Fpga.par->output &= ~PUMP_WASTE_BASE; // Waste base OFF
			Fpga.par->output &= ~PUMP_WASTE_VAC; // Waste base OFF
			Fpga.par->output &= ~VAC_ON; // VACUUM OFF
		}
		if (_CmdRunning == CMD_CLN_SHIFT_LEFT)
		{
			txrob_handle_ctrl_msg(INVALID_SOCKET, CMD_CLN_MOVE_POS, &_MoveLeftPos);
		}
		else
		{
		// --- Reset Commands ---
			_CmdRunning = FALSE;
			_NewCmdPos = 0;
			_NewCmd = FALSE;
		}
		// --- Execute next Commands ---
		switch (loc_new_cmd)
		{
		case CMD_CLN_REFERENCE: txrob_handle_ctrl_msg(INVALID_SOCKET, CMD_CLN_REFERENCE, NULL); break;
		case CMD_CLN_SHIFT_REF: txrob_handle_ctrl_msg(INVALID_SOCKET, CMD_CLN_SHIFT_REF, NULL); break;
		case CMD_CLN_ROT_REF:	txrob_handle_ctrl_msg(INVALID_SOCKET, CMD_CLN_ROT_REF, NULL); break;
		case CMD_CLN_ROT_REF2:	txrob_handle_ctrl_msg(INVALID_SOCKET, CMD_CLN_ROT_REF2, NULL); break;
		case CMD_CLN_MOVE_POS:	txrob_handle_ctrl_msg(INVALID_SOCKET, CMD_CLN_MOVE_POS, &loc_move_pos); break;
		case CMD_CLN_SHIFT_MOV: txrob_handle_ctrl_msg(INVALID_SOCKET, CMD_CLN_SHIFT_MOV, &loc_move_pos); break;
		case CMD_CLN_FILL_CAP :	txrob_handle_ctrl_msg(INVALID_SOCKET, CMD_CLN_FILL_CAP, NULL); break;
		case CMD_CLN_TILT_CAP :	txrob_handle_ctrl_msg(INVALID_SOCKET, CMD_CLN_TILT_CAP, NULL); break;
		case CMD_CLN_EMPTY_WASTE: txrob_handle_ctrl_msg(INVALID_SOCKET, CMD_CLN_EMPTY_WASTE, NULL); break;
		case CMD_CLN_VACUUM:	txrob_handle_ctrl_msg(INVALID_SOCKET, CMD_CLN_VACUUM, NULL); break;
		case CMD_CLN_WAIT:	txrob_handle_ctrl_msg(INVALID_SOCKET, CMD_CLN_WAIT, &loc_move_pos); break;
		case 0: break;
		default:				Error(ERR_CONT, 0, "TXROB_MAIN: Command 0x%08x not implemented", loc_new_cmd); break;
		}
		loc_new_cmd = FALSE;
		loc_move_pos = 0;

	}

    _tx801_wd_main();
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
	
	term_printf("TX80x Robot Advanced mechref ---------------------------------\n");
	term_printf("moving:              %d		cmd: %08x\n", RX_StepperStatus.robinfo.moving, _CmdRunning);
	term_printf("move ok:             %d\n", RX_StepperStatus.robinfo.move_ok);
	term_printf("Sensor Ref Rotation: %d\n", RX_StepperStatus.robinfo.z_in_ref);
	term_printf("Sensor Ref Shift:    %d\n", RX_StepperStatus.robinfo.x_in_ref);
	term_printf("Ref done:            %d\n", RX_StepperStatus.robinfo.ref_done);
	term_printf("Rotation in rev:     %06d\n", RX_StepperStatus.posZ);
	term_printf("Rotation in steps:   %06d\n", motor_get_step(MOTOR_ROT));
	term_printf("Rotation in encsteps:%06d\n", Fpga.encoder[MOTOR_ROT].pos);
	term_printf("Shift in um:         %06d\n", RX_StepperStatus.posX);
	term_printf("Shift in steps:      %06d\n", motor_get_step(MOTOR_SHIFT));
	term_printf("Rotation RefOffset:  %06d\n", _RotRefOffset);
	term_printf("Tilt for Cap:        %06d\n", _TiltSteps);
	term_printf("Time Waste Vac [s]:  %06d\n", _TimeWastePump);
	term_printf("Time fill Cap [s]:   %06d\n", _TimeFillCap);
	term_printf("Speed Shift [mm/s]:  %06d\n", _SpeedShift * 2 / 200 / 16);
	term_printf("Timer Count:         %06d\n", _TimeCnt);
	term_printf("\n");
	term_printf("x position in micro:  %d\n", RX_StepperStatus.posX);	
	term_printf("y position in micro:  %d\n", RX_StepperStatus.posY);	
	term_printf("z position in micro:  %d\n", RX_StepperStatus.posZ);
	term_printf("Vaccum State:         %d\n", _VacuumState);
	term_printf("Vacuum Ready:         %d\n", RX_StepperStatus.robinfo.vacuum_ready);
	term_printf("Roboter in Wipe:      %d\n", RX_StepperStatus.robinfo.rob_in_wipe);
	term_printf("Roboter in Vac:       %d\n", RX_StepperStatus.robinfo.rob_in_vac);
	term_printf("Roboter in Wash:      %d\n", RX_StepperStatus.robinfo.rob_in_wash);
	term_printf("Roboter in Cap:       %d\n", RX_StepperStatus.robinfo.rob_in_cap);
    term_printf("\n");
    term_printf("Reference WD done:     %d\n", RX_StepperStatus.robinfo.ref_done_wd);
    term_printf("WD-Motor Front pos:    %d\n", TX_REF_HEIGHT_WD - _steps_2_micron(motor_get_step(MOTOR_WRINKLE_DETECTION_FRONT)));
    term_printf("WD-Motor Back pos:     %d\n", TX_REF_HEIGHT_WD - _steps_2_micron(motor_get_step(MOTOR_WRINKLE_DETECTION_BACK)));
    term_printf("Wrinkle Detected:      %d\n", RX_StepperStatus.robinfo.wrinkle_detected);
    term_printf("\n");
}

//--- txrob_menu --------------------------------------------------
int txrob_menu(void)
{
	char str[MAX_PATH];
	int synth = FALSE;
	static int cnt = 0;
	int i;
	int pos = 10000;

	_txrob_display_status();
	
	term_printf("MENU FLO-ROBOT -------------------------\n");
	term_printf("s: STOP\n");
	term_printf("R: Reference\n");
	term_printf("c: move to capping\n");
	term_printf("e: move to wash\n");
	term_printf("a: move to vacuum\n");
	term_printf("w: move to wipe\n");
	term_printf("b: shift back\n");
	term_printf("l: shift left\n");
	term_printf("d<steps>: set Rotation RefOffset <steps>\n");
	term_printf("t<steps>: set Tilt for Cap <steps>\n");
	term_printf("p<sec>: set Time Waste Vac <sec>\n");
	term_printf("f<sec>: set Time fill Cap <sec>\n");
	term_printf("v<mm/s>: set Speed Shift for wash <mm/s>\n");
    term_printf("h<um>: set hight for Wrinkle Detection\n");
    term_printf("u: move Wrinkle Detection to up position\n");
	term_printf("x: exit\n");
    term_printf(">");
	term_flush();

	if (term_get_str(str, sizeof(str)))
	{
		switch (str[0])
		{
		case 's': txrob_handle_ctrl_msg(INVALID_SOCKET, CMD_CLN_STOP, NULL);
            txrob_handle_ctrl_msg(INVALID_SOCKET, CMD_CAP_STOP, NULL);
            break;
        case 'r': if (str[1] == 0) for (i = 0; i < MOTOR_CNT; i++) motor_reset(i);
			else           motor_reset(atoi(&str[1]));
			break;
		case 'o' : Fpga.par->output ^= (1 << atoi(&str[1])); break;
		case 'R': txrob_handle_ctrl_msg(INVALID_SOCKET, CMD_CLN_REFERENCE, NULL);
				  txrob_handle_ctrl_msg(INVALID_SOCKET, CMD_CAP_REFERENCE, NULL);
				  break;
        case 'c': pos = 0; txrob_handle_ctrl_msg(INVALID_SOCKET, CMD_CLN_MOVE_POS, &pos); break;
		case 'e': pos = 1; txrob_handle_ctrl_msg(INVALID_SOCKET, CMD_CLN_MOVE_POS, &pos); break;
		case 'a': pos = 2; txrob_handle_ctrl_msg(INVALID_SOCKET, CMD_CLN_MOVE_POS, &pos); break;
		case 'w': pos = 3; txrob_handle_ctrl_msg(INVALID_SOCKET, CMD_CLN_MOVE_POS, &pos); break;
		case 'b': pos = 3; txrob_handle_ctrl_msg(INVALID_SOCKET, CMD_CLN_SHIFT_MOV, &pos); break;
		
		case 'd' : pos = atoi(&str[1]); 
			if (pos < 0) pos = 0;
			if (pos > 800) pos = 800;
			_RotRefOffset = pos;
			break;
		case 't' : pos = atoi(&str[1]); 
			if (pos < 0) pos = 0;
			if (pos > 1000) pos = 1000;
			_TiltSteps = pos;
			break;
		case 'p' : pos = atoi(&str[1]); 
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
        case 'h': pos = atoi(&str[1]);
            txrob_handle_ctrl_msg(INVALID_SOCKET, CMD_CAP_PRINT_POS, &pos);
            break;
        case 'u': txrob_handle_ctrl_msg(INVALID_SOCKET, CMD_CAP_UP_POS, NULL); break;
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
    INT32 pos_wd, steps_wd;
    EnFluidCtrlMode	ctrlMode;
	int firsttime = 0;
	
	switch (msgId)
	{
	case CMD_TT_STATUS:				sok_send_2(&socket, REP_TT_STATUS, sizeof(RX_StepperStatus), &RX_StepperStatus);
									//_txrob_control(socket, ctrlMode);
        break;
	
	case CMD_CLN_STOP:				strcpy(_CmdName, "CMD_CLN_STOP");
		motors_stop(MOTOR_ALL_ROBOT_BITS); // Stops Hub and Robot
		Fpga.par->output &= ~RO_ALL_OUTPUTS; // set all output to off
		RX_StepperStatus.robinfo.moving = TRUE;
		_CmdRunning = msgId;
		// reset var
		_LastRobPosCmd = 0;
		_RobFunction = rob_fct_cap;
		_TimeCnt = 0;
		_NewCmdPos = 0;
		_NewCmd = FALSE;
		_WipeWaiting = FALSE;
		_VacuumWaiting = FALSE;
		break;

	case CMD_CLN_REFERENCE:			strcpy(_CmdName, "CMD_CLN_REFERENCE");
		TrPrintfL(TRUE, "SOCKET[%d]: %s", socket, _CmdName);
		if (_CmdRunning && _CmdRunning != CMD_FLUID_CTRL_MODE)
		{ 
            txrob_handle_ctrl_msg(INVALID_SOCKET, CMD_CLN_STOP, NULL); _NewCmd = CMD_CLN_REFERENCE; break; 
        }
		motor_reset(MOTOR_ROT); // to recover from move count missalignment
		motor_reset(MOTOR_SHIFT); // to recover from move count missalignment
		Fpga.par->output &= ~RO_ALL_OUTPUTS;
		RX_StepperStatus.robinfo.ref_done = FALSE;
		_CmdRunning = msgId;
		// reset var
		_LastRobPosCmd = 0;
		_RobFunction = rob_fct_cap;
		_TimeCnt = 0;
		_NewCmdPos = 0;
		_NewCmd = FALSE;
		_WipeWaiting = FALSE;
		_VacuumWaiting = FALSE;
		_VacuumState = rob_vacuum_1_to_4;		
		break;
		
	case CMD_CLN_SHIFT_REF:			strcpy(_CmdName, "CMD_CLN_SHIFT_REF");
		TrPrintfL(TRUE, "SOCKET[%d]: %s", socket, _CmdName);
		if (_CmdRunning && _CmdRunning != CMD_FLUID_CTRL_MODE){ 
            txrob_handle_ctrl_msg(INVALID_SOCKET, CMD_CLN_STOP, NULL); _NewCmd = CMD_CLN_SHIFT_REF; break; 
            }
		motor_reset(MOTOR_SHIFT); // to recover from move count missalignment
		Fpga.par->output &= ~RO_ALL_OUTPUTS;
		RX_StepperStatus.robinfo.ref_done = FALSE;
		RX_StepperStatus.robinfo.moving = TRUE;
		_CmdRunning = msgId;
		_NewCmd = FALSE;
		if (fpga_input(SHIFT_STORED_IN) == TRUE){ motors_move_by_step(MOTOR_SHIFT_BITS, &_ParShiftDrive, SHIFT_STEPS_PER_METER * 0.01, TRUE); _NewCmd = CMD_CLN_SHIFT_REF; break; }
		motors_move_by_step(MOTOR_SHIFT_BITS, &_ParShiftSensRef, -SHIFT_STEPS_PER_METER * 0.09, TRUE);
		break;
		
	case CMD_CLN_ROT_REF:			strcpy(_CmdName, "CMD_CLN_ROT_REF");
		TrPrintfL(TRUE, "SOCKET[%d]: %s", socket, _CmdName);
		if (_CmdRunning && _CmdRunning != CMD_FLUID_CTRL_MODE){
            txrob_handle_ctrl_msg(INVALID_SOCKET, CMD_CLN_STOP, NULL); _NewCmd = CMD_CLN_ROT_REF; break;
            }
		motor_reset(MOTOR_ROT); // to recover from move count missalignment
		Fpga.par->output &= ~RO_ALL_OUTPUTS;
		RX_StepperStatus.robinfo.ref_done = FALSE;
		RX_StepperStatus.robinfo.moving = TRUE;
		_CmdRunning = msgId;
		_NewCmd = FALSE;
		if (fpga_input(ROT_STORED_IN) == TRUE){	motors_move_by_step(MOTOR_ROT_BITS, &_ParRotDrive, ROT_STEPS_PER_REV * 0.02, TRUE); _NewCmd = CMD_CLN_ROT_REF; break;}
		motors_move_by_step(MOTOR_ROT_BITS, &_ParRotSensRef, -ROT_STEPS_PER_REV, TRUE);
        _Reference_Count++;
        break;
		
	case CMD_CLN_ROT_REF2:	strcpy(_CmdName, "CMD_CLN_ROT_REF2");
		TrPrintfL(TRUE, "SOCKET[%d]: %s", socket, _CmdName);
		if (_CmdRunning && _CmdRunning != CMD_FLUID_CTRL_MODE){ 
            txrob_handle_ctrl_msg(INVALID_SOCKET, CMD_CLN_STOP, NULL); _NewCmd = CMD_CLN_ROT_REF2; break; 
            }
		motor_reset(MOTOR_ROT); // to recover from move count missalignment
		Fpga.par->output &= ~RO_ALL_OUTPUTS;
		RX_StepperStatus.robinfo.ref_done = FALSE;
		RX_StepperStatus.robinfo.moving = TRUE;
		_CmdRunning = msgId;
		_NewCmd = FALSE;
		motors_move_by_step(MOTOR_ROT_BITS, &_ParRotDrive, _RotRefOffset, TRUE);
		break;
		
	case CMD_CLN_MOVE_POS:		strcpy(_CmdName, "CMD_CLN_MOVE_POS");
		TrPrintfL(TRUE, "SOCKET[%d]: %s", socket, _CmdName);
		if (!_CmdRunning || _CmdRunning == CMD_CLN_SHIFT_LEFT)
		{
			if (!RX_StepperStatus.robinfo.ref_done)
			{ 
				Error(ERR_CONT,0, "Robot not referenced, cmd=0x%08x", msgId);
				break;
			}
			pos = *((INT32*)pdata);
			if (pos < rob_fct_cap) {Error(LOG, 0, "CLN: Command %s: negative position not allowed", _CmdName); break;}
			if (pos > rob_fct_vacuum_all) {Error(LOG, 0, "CLN: Command %s: too high pos", _CmdName); break;}

			if (POS_SHIFT[POS_SHIFT_MAX_TURN_POS] < motor_get_step(MOTOR_SHIFT) && RX_StepperStatus.robinfo.moving == FALSE)
			{
				_CmdRunning = 0;
				int left_pos = POS_SHIFT_MAX_TURN_POS;
				txrob_handle_ctrl_msg(INVALID_SOCKET, CMD_CLN_SHIFT_LEFT, &left_pos);
				_MoveLeftPos = pos;
			}
			else if (POS_SHIFT[POS_SHIFT_MAX_TURN_POS] >= motor_get_step(MOTOR_SHIFT))
			{
				_CmdRunning = msgId;
				RX_StepperStatus.robinfo.cap_ready = FALSE;
				RX_StepperStatus.robinfo.moving = TRUE;
				_RobFunction = pos;
				
				if (pos == rob_fct_vacuum_change && (_VacuumState == rob_vacuum_1_to_4 || _VacuumState == rob_vacuum_all))		pos = POS_ROT[CAP_POS];
				else if (pos == rob_fct_vacuum_change && _VacuumState == rob_vacuum_5_to_8)										pos = POS_ROT[WIPE_POS];
				else if (pos == rob_fct_tilt)																					pos = POS_ROT[CAP_POS];
				else if (pos == rob_fct_vacuum_all)
				{
					if (_VacuumState == rob_vacuum_1_to_4)																		pos = POS_ROT[VAC_POS_1_TO_4_TO_ALL];
					else if (_VacuumState == rob_vacuum_5_to_8)																	pos = POS_ROT[VAC_POS_5_TO_8_TO_ALL];
					else if (_VacuumState == rob_vacuum_all)																	pos = POS_ROT[VAC_POS];
				}
				else																											pos = POS_ROT[pos];
				
				_MoveLeftPos = 0;
				Fpga.par->output &= ~RO_ALL_OUTPUTS; // set all output to off
				sok_send_2(&socket, REP_TT_STATUS, sizeof(RX_StepperStatus), &RX_StepperStatus);
				motors_move_to_step(MOTOR_ROT_BITS, &_ParRotDrive, pos);
			}		
		}
		break;
		
	case CMD_CLN_SHIFT_MOV:		strcpy(_CmdName, "CMD_CLN_SHIFT_MOV");
		TrPrintfL(TRUE, "SOCKET[%d]: %s", socket, _CmdName);
		if (!_CmdRunning || _CmdRunning == CMD_FLUID_CTRL_MODE)
		{
			if (!RX_StepperStatus.robinfo.ref_done)
			{ 
				Error(ERR_CONT,0, "Robot not referenced, cmd=0x%08x", msgId);
				break;
			}
			pos = *((INT32*)pdata);
			if (pos < 0) {Error(LOG, 0, "CLN: Command %s: negative position not allowed", _CmdName); break;}
			if (pos >= POS_SHIFT_CNT) {Error(LOG, 0, "CLN: Command %s: too high pos", _CmdName); break;}
			if (_RobFunction == rob_fct_cap && pos != 1) {Error(LOG, 0, "CLN: Command %s: cancle shift, robot in capping", _CmdName); break;}
			_CmdRunning = msgId;
			RX_StepperStatus.robinfo.moving = TRUE;
			_LastRobPosCmd = pos;
			
			if (pos == 3 && (_RobFunction == rob_fct_wash || _RobFunction == rob_fct_wipe))
			{
				pos = POS_SHIFT[pos];
				_ParShiftWet.speed = _SpeedShift;
				motors_move_to_step(MOTOR_SHIFT_BITS, &_ParShiftWet, pos);
				_WipeWaiting = FALSE;
			}
			else if (pos == 3 || pos == 4)
			{
				if (_RobFunction == rob_fct_vacuum || _RobFunction == rob_fct_vacuum_all)
				{
					if (abs(POS_SHIFT[3] - motor_get_step(MOTOR_SHIFT) <= MOTOR_SHIFT_VAR) || abs(POS_SHIFT[6] - motor_get_step(MOTOR_SHIFT) <= MOTOR_SHIFT_VAR))	pos = POS_SHIFT[4];
					else if (abs(POS_SHIFT[4] - motor_get_step(MOTOR_SHIFT) <= MOTOR_SHIFT_VAR))																	pos = POS_SHIFT[3];
					else																																			pos = POS_SHIFT[pos];
	
				}
				else																																				pos = POS_SHIFT[pos];
				
				if (_VacuumState == rob_vacuum_all)	_ParShiftWet.speed = _SpeedShift;
				else								_ParShiftWet.speed = SPEED_SLOW_SHIFT;
				motors_move_to_step(MOTOR_SHIFT_BITS, &_ParShiftWet, pos);
				_VacuumWaiting = FALSE;
			}
			else
			{
				if (_RobFunction == rob_fct_vacuum || _RobFunction == rob_fct_vacuum_all)
				{
					if (abs(POS_SHIFT[3] - motor_get_step(MOTOR_SHIFT) <= MOTOR_SHIFT_VAR)	|| abs(POS_SHIFT[6] - motor_get_step(MOTOR_SHIFT) <= MOTOR_SHIFT_VAR))	pos = POS_SHIFT[3];
					else if (abs(POS_SHIFT[4] - motor_get_step(MOTOR_SHIFT) <= MOTOR_SHIFT_VAR))																	pos = POS_SHIFT[4];
					else																																			pos = POS_SHIFT[pos];
	
				}
				else																																				pos = POS_SHIFT[pos];
                if (abs(pos - motor_get_step(MOTOR_SHIFT)) > MOTOR_SHIFT_VAR)	motors_move_to_step(MOTOR_SHIFT_BITS, &_ParShiftDrive, pos);
			}
		}
		break;
		
	case CMD_CLN_SHIFT_LEFT:	strcpy(_CmdName, "CMD_CLN_SHIFT_LEFT");
		TrPrintfL(TRUE, "SOCKET[%d]: %s", socket, _CmdName);
		if (!_CmdRunning || _CmdRunning == CMD_FLUID_CTRL_MODE)
		{
			if (!RX_StepperStatus.robinfo.ref_done)
			{ 
				Error(ERR_CONT,0, "Robot not referenced, cmd=0x%08x", msgId);
				break;
			}
			pos = *((INT32*)pdata);
			if (pos < 0) {Error(LOG, 0, "CLN: Command %s: negative position not allowed", _CmdName); break;}
			if (pos >= POS_SHIFT_CNT) {Error(LOG, 0, "CLN: Command %s: too high pos", _CmdName); break;}
			if (_RobFunction == rob_fct_cap && pos != 1) {Error(LOG, 0, "CLN: Command %s: cancle shift, robot in capping", _CmdName); break;}
			_CmdRunning = msgId;
			RX_StepperStatus.robinfo.moving = TRUE;
			pos = POS_SHIFT[pos];
			_ParShiftWet.speed = _SpeedShift;
			motors_move_to_step(MOTOR_SHIFT_BITS, &_ParShiftWet, pos);
		}
		
	case CMD_CLN_FILL_CAP :		strcpy(_CmdName, "CMD_CLN_FILL_CAP");
		TrPrintfL(TRUE, "SOCKET[%d]: %s", socket, _CmdName);
		if (!_CmdRunning || _CmdRunning == CMD_FLUID_CTRL_MODE)
		{
			Fpga.par->output |= PUMP_FLUSH_CAP; // Flush Cap ON
			_CmdRunning = msgId;
			RX_StepperStatus.robinfo.moving = TRUE;
			_TimeCnt = _TimeFillCap * MAIN_PER_SEC;
		}
		break;
		
	case CMD_CLN_WAIT :		strcpy(_CmdName, "CMD_CLN_WAIT");
		TrPrintfL(TRUE, "SOCKET[%d]: %s", socket, _CmdName);
		if (!_CmdRunning || _CmdRunning == CMD_FLUID_CTRL_MODE)
		{
			pos = *((INT32*)pdata);
			_CmdRunning = msgId;
			RX_StepperStatus.robinfo.moving = TRUE;
			_TimeCnt = pos * MAIN_PER_SEC;
		}
		break;
		
	case CMD_CLN_TILT_CAP :		strcpy(_CmdName, "CMD_CLN_TILT_CAP");
		TrPrintfL(TRUE, "SOCKET[%d]: %s", socket, _CmdName);
		if (!_CmdRunning || _CmdRunning == CMD_FLUID_CTRL_MODE)
		{
			if (!RX_StepperStatus.robinfo.ref_done)
			{ 
				Error(ERR_CONT,0, "Robot not referenced, cmd=0x%08x", msgId);
				break;
			}
			_CmdRunning = msgId;
			RX_StepperStatus.robinfo.moving = TRUE;
			motors_move_to_step(MOTOR_ROT_BITS, &_ParRotDrive, POS_ROT_TILT);
		}
		
		break;
		
	case CMD_CLN_EMPTY_WASTE :		strcpy(_CmdName, "CMD_CLN_EMPTY_WASTE");
		TrPrintfL(TRUE, "SOCKET[%d]: %s", socket, _CmdName);
		if (!_CmdRunning || _CmdRunning == CMD_FLUID_CTRL_MODE)
		{
			Fpga.par->output |= PUMP_WASTE_BASE; // Waste base ON
			Fpga.par->output |= PUMP_WASTE_VAC; // Waste vac ON
			_CmdRunning = msgId;
			RX_StepperStatus.robinfo.moving = TRUE;
			_TimeCnt = _TimeWastePump * MAIN_PER_SEC;
		}
		break;
		
	case CMD_CLN_VACUUM :		strcpy(_CmdName, "CMD_CLN_VACUUM");
		TrPrintfL(TRUE, "SOCKET[%d]: %s", socket, _CmdName);
		if (!_CmdRunning || _CmdRunning == CMD_FLUID_CTRL_MODE)
		{
			Fpga.par->output |= PUMP_WASTE_VAC; // Waste vac ON
			Fpga.par->output |= VAC_ON; // VACUUM ON
			_CmdRunning = msgId;
			RX_StepperStatus.robinfo.moving = TRUE;
			_TimeCnt = 5 * MAIN_PER_SEC;
		}
		break;

	case CMD_ERROR_RESET:		
		TrPrintfL(TRUE, "SOCKET[%d]: %s", socket, _CmdName);
		fpga_stepper_error_reset();
		_txrob_error_reset();
		break;
        
    case CMD_CAP_STOP:
		TrPrintfL(TRUE, "SOCKET[%d]: %s", socket, _CmdName);
        motors_stop(MOTOR_WD_BITS);
        break;
        
    case CMD_CAP_REFERENCE:				strcpy(_CmdName, "CMD_CAP_REFERENCE");
		TrPrintfL(TRUE, "SOCKET[%d]: %s", socket, _CmdName);
        motors_reset(MOTOR_WD_BITS);
        RX_StepperStatus.robinfo.ref_done_wd = FALSE;
        _tx801_wrinkledetection_do_reference();
        break;

    case CMD_CAP_PRINT_POS:		        strcpy(_CmdName, "CMD_CAP_PRINT_POS");
		TrPrintfL(TRUE, "SOCKET[%d]: %s", socket, _CmdName);
        pos_wd = (*((INT32 *)pdata));
        if (pos_wd < TX_PRINT_POS_MIN)
        {
            pos_wd = TX_PRINT_POS_MIN;
            Error(WARN, 0, "CMD_CAP_PRINT_POS of Wrinkle Detection set to MIN pos=%d.%03d mm", pos_wd / 1000, pos_wd % 1000);
        }
        else if (pos_wd > TX_REF_HEIGHT_WD)
        {
            pos_wd = TX_REF_HEIGHT_WD;
            Error(WARN, 0, "CMD_CAP_PRINT_POS of Wrinkle Detection set to MAX pos=%d.%03d mm", pos_wd / 1000, pos_wd % 1000);
        }
        if (!_CmdRunningWD && (!_WD_in_print || (steps_wd != _PrintPos_Act)))
        {
            _PrintPos_New = _micron_2_steps(TX_REF_HEIGHT_WD + 500 - pos_wd);
            if (RX_StepperStatus.robinfo.ref_done_wd)
                _tx801_wd_move_to_pos(CMD_CAP_PRINT_POS, _PrintPos_New);
            else
                _tx801_wrinkledetection_do_reference();
        }
        break;
        
    case CMD_CAP_UP_POS:
    case CMD_CAP_CAPPING_POS:
    case CMD_CAP_WASH_POS:
    case CMD_CAP_WIPE_POS:
    case CMD_CAP_VACUUM_POS:
        strcpy(_CmdName, "CMD_CAP_UP_POS");
		TrPrintfL(TRUE, "SOCKET[%d]: %s", socket, _CmdName);
        if (!_CmdRunningWD)
        {
            _PrintPos_New = _micron_2_steps(500);
            if (RX_StepperStatus.robinfo.ref_done_wd)
                _tx801_wd_move_to_pos(CMD_CAP_UP_POS, _PrintPos_New);
            else
                _tx801_wrinkledetection_do_reference();
        }
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

	memset(&par, 0, sizeof(par));
	par.stop_mux	= 0;
	par.dis_mux_in	= 0;
	par.encCheck	= chk_off;
	RX_StepperStatus.robinfo.moving = TRUE;
	_CmdRunning = 1;
	
	if (motorNo == MOTOR_ROT)
	{
        // paramaters tested 14-JAN-20
        par.speed		= 1000; // speed with max tork: 21'333
		par.accel		= 10000;
		par.current_acc = 400.0; //  max 67 = 0.67 A
		par.current_run = 300.0; //  max 67 = 0.67 A
		par.stop_mux	= 0;
		par.dis_mux_in	= 0;
		par.encCheck    = chk_std;

        motors_config(motors, 0, L3518_STEPS_PER_METER, L3518_INC_PER_METER, MICROSTEPS);
		motors_move_by_step(motors, &par, steps, FALSE);			
	}
	else
	{
        // paramaters tested 14-JAN-20
        
        par.speed		= 21000; // speed with max tork: 21'333
		par.accel		= 10000;
		par.current_acc = 40.0; //  max 67 = 0.67 A
		par.current_run = 40.0; //  max 67 = 0.67 A
		par.stop_mux	= 0;
		par.dis_mux_in	= 0;
		par.encCheck	= chk_off;
        motors_config(motors, CURRENT_HOLD_SHIFT, 0.0, 0.0, STEPS);
		motors_move_by_step(motors, &par, steps, TRUE);			
	}	
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

static int _step_rob_in_wipe(void)
{
	if (RX_StepperStatus.robinfo.move_ok == TRUE && RX_StepperStatus.robinfo.moving == FALSE && abs(POS_ROT[WIPE_POS] - motor_get_step(MOTOR_ROT)) <= MOTOR_ROT_VAR && RX_StepperStatus.robinfo.rob_in_wipe)
	{
		return TRUE;
	}
	return FALSE;
}

static int _step_rob_in_wash(void)
{
	if (RX_StepperStatus.robinfo.move_ok == TRUE && RX_StepperStatus.robinfo.moving == FALSE && abs(POS_ROT[WASH_POS] - motor_get_step(MOTOR_ROT)) <= MOTOR_ROT_VAR && RX_StepperStatus.robinfo.rob_in_wash)
	{
		return TRUE;
	}
	return FALSE;
}

static int _step_rob_in_vacuum(void)
{
	if (RX_StepperStatus.robinfo.move_ok == TRUE && RX_StepperStatus.robinfo.moving == FALSE && abs(POS_ROT[VAC_POS] - motor_get_step(MOTOR_ROT)) <= MOTOR_ROT_VAR && RX_StepperStatus.robinfo.rob_in_vac)
	{
		return TRUE;
	}
	return FALSE;
}

static int _step_rob_in_capping(void)
{
	if (RX_StepperStatus.robinfo.moving == FALSE && abs(POS_ROT[CAP_POS] - motor_get_step(MOTOR_ROT)) <= MOTOR_ROT_VAR && RX_StepperStatus.robinfo.rob_in_cap)
	{
		return TRUE;
	}
	return FALSE;
}

static int _wipe_done(void)
{
	if (RX_StepperStatus.robinfo.move_ok == TRUE && RX_StepperStatus.robinfo.moving == FALSE && abs(POS_ROT[WIPE_POS] - motor_get_step(MOTOR_ROT)) <= MOTOR_ROT_VAR && _WipeWaiting == FALSE)
	{
		return TRUE;
	}
	return FALSE;
}

static int _wash_done(void)
{
	if (RX_StepperStatus.robinfo.move_ok == TRUE && RX_StepperStatus.robinfo.moving == FALSE && abs(POS_ROT[WASH_POS] - motor_get_step(MOTOR_ROT)) <= MOTOR_ROT_VAR && abs(POS_SHIFT[3] - motor_get_step(MOTOR_SHIFT) <= MOTOR_SHIFT_VAR))
	{
		return TRUE;
	}
	return FALSE;
}

static int _vacuum_done(void)
{
	if ((RX_StepperStatus.robinfo.moving == FALSE || _CmdRunning == CMD_CLN_EMPTY_WASTE) && abs(POS_ROT[VAC_POS] - motor_get_step(MOTOR_ROT)) <= MOTOR_ROT_VAR && _VacuumWaiting == FALSE)
	{
		return TRUE;
	}
	return FALSE;
}

static int _vacuum_in_change(void)
{
	if (RX_StepperStatus.robinfo.moving == FALSE && ((abs(POS_ROT[CAP_POS] - motor_get_step(MOTOR_ROT)) <= MOTOR_ROT_VAR && _VacuumState == rob_vacuum_5_to_8) || (abs(POS_ROT[WASH_POS] - motor_get_step(MOTOR_ROT)) <= MOTOR_ROT_VAR && _VacuumState == rob_vacuum_1_to_4)))
	{
		return TRUE;
	}
}

static int _robot_left(void)
{
	int position = _shift_steps_2_micron(POS_SHIFT_CENTER);
	if (RX_StepperStatus.posX <= position + 10) return TRUE;
	else return FALSE;
}

static void _tx801_wd_main(void)
{
    int motor, ok;

    if (rx_get_ticks() >= _WrinkleDetected + _TimeWrinkleDetected)
        _WrinkleDetected = 0;

    // read Inputs
    RX_StepperStatus.robinfo.wrinkle_detected = fpga_input(WRINKLE_DETECTED) || _WrinkleDetected; // || _WrinkleDetected;
    if (RX_StepperStatus.robinfo.wrinkle_detected && !_WrinkleDetected)
    {
        ctrl_send_2(REP_TT_STATUS, sizeof(RX_StepperStatus), &RX_StepperStatus);
        _WrinkleDetected = rx_get_ticks();
    }

    if (RX_StepperStatus.robinfo.moving_wd)
    {
        _WD_in_print = FALSE;
		_WrinkleDetected = 0;
    }

    RX_StepperStatus.robinfo.wd_front_up = fpga_input(WD_FRONT_STORED_IN);
    RX_StepperStatus.robinfo.wd_back_up = fpga_input(WD_BACK_STORED_IN);
    if (_CmdRunningWD && motors_move_done(MOTOR_WD_BITS))
    {
        RX_StepperStatus.robinfo.moving_wd = FALSE;
        if (_CmdRunningWD == CMD_CAP_REFERENCE)
        {
            for (motor = MOTOR_WRINKLE_DETECTION_FRONT, ok = TRUE; motor < MOTOR_WD_CNT; motor++)
            {
                if ((Fpga.stat->statMot[motor].err_estop & ENC_ESTOP_ENC))
                {
                    if (!fpga_input(motor))
                    {
                        Error(ERR_ABORT, 0, "WRINKLE DETECTION: %s: motor %s blocked", _CmdName, motor+1);
                        ok = FALSE;
                    }
                }
            }
            motors_reset(MOTOR_WD_BITS);
            RX_StepperStatus.robinfo.ref_done_wd = ok && RX_StepperStatus.robinfo.wd_front_up && RX_StepperStatus.robinfo.wd_back_up;
        }
        else
        {
            for (motor = MOTOR_WRINKLE_DETECTION_FRONT, ok = TRUE; motor < MOTOR_WD_CNT; motor++)
            {
                if (motor_error(motor))
                {
                    Error(ERR_ABORT, 0, "WRINKLE DETECTION: %s: motor %s blocked", _CmdName, motor + 1);
                    ok = FALSE;
                }
            }
            if (!ok) RX_StepperStatus.robinfo.ref_done_wd = FALSE;
        }

        _WD_in_print = (_CmdRunning == CMD_CAP_PRINT_POS && RX_StepperStatus.info.ref_done);
        
        if (_CmdRunningWD == CMD_CAP_REFERENCE && _PrintPos_New)
        {
            _tx801_wd_move_to_pos(CMD_CAP_PRINT_POS, _PrintPos_New);
            _PrintPos_Act = _PrintPos_New;
            _PrintPos_New = 0;
        }
        else
            _CmdRunningWD = FALSE;
    }
}

//--- _tx801_wrinkledetection_do_reference ----------------------------------------------------------------
static void _tx801_wrinkledetection_do_reference(void)
{
    motors_stop(MOTOR_WD_BITS);
    //motors_config(MOTOR_WD_BITS, CURRENT_HOLD_WD, L3518_STEPS_PER_METER, L3518_INC_PER_METER, STEPS);

    _CmdRunningWD = CMD_CAP_REFERENCE;
    RX_StepperStatus.robinfo.moving_wd = TRUE;
    motors_move_by_step(MOTOR_WD_BITS, &_ParWDRef, -100000, TRUE);
}
//--- _tx801_wd_move_to_pos -----------------------------------------------------------------

static void _tx801_wd_move_to_pos(int cmd, int pos)
{
    _CmdRunningWD = cmd;
    RX_StepperStatus.info.moving = TRUE;
    motors_move_to_step(MOTOR_WD_BITS, &_ParWDDrive, pos);
}

//---_micron_2_steps --------------------------------------------------------------
static int _micron_2_steps(int micron)
{
    return (int)(0.5 + STEPS_REV / DIST_REV * micron);
}

//---_steps_2_micron --------------------------------------------------------------
static int _steps_2_micron(int steps)
{
    return (int)(DIST_REV * steps / STEPS_REV + 0.5);
}