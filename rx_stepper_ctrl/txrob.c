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
#include "tx80x_wd.h"
#include "txrob.h"

//--- defines ------------------------------------------------

// General
#define MOTOR_ALL_ROBOT_BITS	0x03
#define MAIN_PER_SEC		930			// Main executions per sec
#define STEPS_REV			(200*16)	// steps per motor revolution * 16 microsteps

// Rotation Motor // 200 steps/rev // max 4.2 Amp
#define CURRENT_HOLD_ROT	50.0	// 2Amp Holding current is too high!

#define MOTOR_ROT			0
#define MOTOR_ROT_BITS		0x01
#define MOTOR_ROT_VAR		40	// max allowed variance 

#define ROT_STEPS_PER_REV	3200.0 // 200 steps * 16
#define ROT_INC_PER_REV		16000	//(4*4000.0) // 3'200 m / 0.8

#define SCREW_MIN_UNIT		4.0 // 360/4.0 = 90� minimal turn

// Shift Motor // 200 steps/rev // 2mm/rev // max 0.67 Amp
#define CURRENT_HOLD_SHIFT  7.0

#define MOTOR_SHIFT			1
#define MOTOR_SHIFT_BITS	0x02
#define MOTOR_SHIFT_VAR		100 // 20 // 10 // max allowed variance 

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
#define POS_ROT_CNT			6
#define POS_ROT_WIPE		(ROT_STEPS_PER_REV/4.0*0)
#define POS_ROT_VAC			(ROT_STEPS_PER_REV/4.0*1)
#define POS_ROT_WASH		(ROT_STEPS_PER_REV/4.0*2)
#define POS_ROT_CAP			(ROT_STEPS_PER_REV/4.0*3)
//#define POS_ROT_VAC_CHANGE			(ROT_STEPS_PER_REV/4.0*3)
#define POS_ROT_TILT		(POS_ROT_CAP-_TiltSteps)
#define POS_ROT_VAC_1_TO_4_TO_ALL	(ROT_STEPS_PER_REV/4.0*2.5)
#define POS_ROT_VAC_5_TO_8_TO_ALL	(ROT_STEPS_PER_REV/4.0*0.5)

// ENCODER_ROT Positions
#define POS_ENC_WIPE		(ROT_INC_PER_REV/4.0*0)
#define POS_ENC_VAC			(ROT_INC_PER_REV/4.0*1)
#define POS_ENC_WASH				(ROT_INC_PER_REV/4.0*2)
#define POS_ENC_CAP			(ROT_INC_PER_REV/4.0*3)
//#define POS_ENC_VAC_CHANGE			(ROT_INC_PER_REV/4.0*3)
#define POS_ENC_TILT		(POS_ROT_TILT*ROT_INC_PER_REV/ROT_STEPS_PER_REV)
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

// Outputs
#define VAC_ON				0x001 // 0
#define PUMP_FLUSH_CAP		0x010 // 4 Flush 2
#define PUMP_WASTE_VAC		0x020 // 5 Waste 1
#define PUMP_FLUSH_WET		0x040 // 6 Flush 1
#define PUMP_WASTE_BASE		0x080 // 7 Waste 2

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
static char	_CmdName[32];
static int  _LastRobPosCmd = 0;
static int  _RobFunction = 0;
static int	_NewCmd = 0;
static int	_NewCmdPos = 0;
static int	_TimeCnt = 0;
static int _MoveLeftPos = 0;

static int _WipeWaiting = FALSE;
static int _VacuumWaiting = FALSE;

static int _PosShiftCenter = 0;
static int _PosShiftEnd = 0;

static int _Reference_Count = 0;
static int _Max_Reference = 3;

static int _Help = FALSE;
static int _Menu = 1;

// static SMovePar	_ParRotRef;
static SMovePar	_ParRotSensRef;
static SMovePar	_ParRotDrive;

static SMovePar	_ParShiftSensRef;
static SMovePar	_ParShiftDrive;
static SMovePar	_ParShiftWet;
static SMovePar	_ParShiftVac;
static SMovePar	_ParShiftWipe;

static ERobotVacuumState _VacuumState;

static UINT32	_txrob_Error[5];


// Times and Parameters
static int	_TimeWastePump = 10000; //20; // in ms
static int	_TimeFillCap = 14; // in s
static int	_TimeFillCap801 = 14;	// in s
static int	_TimeFillCap802 = 25;	// in s
static int	_SpeedShift = 19000; // in steps/s
static int	_RotRefOffset = 11; // in steps
static int	_TiltSteps = 90; // in steps

static int _TimeWastePumpStart = 0;

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

	_ParRotDrive.speed		 = 800; // speed with max tork:
	_ParRotDrive.accel		 = 800;
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
            if (!RX_StepperStatus.robinfo.rob_in_cap)
                Error(ERR_CONT, 0, "Stepper: Command %s: Motor seems to have slippage, please check connection motor to clutch.", _CmdName);
            else
            {
                _VacuumState = rob_vacuum_5_to_8;
			return FALSE;
		}
            
		}
		else
			Error(ERR_CONT, 0, "Stepper: Command %s: Motor Encoder at invalid position", _CmdName);
		}
    
	else if (abs(POS_ROT_TILT - steps) <= MOTOR_ROT_VAR)
	{
		if (abs(POS_ENC_TILT - Fpga.encoder[MOTOR_ROT].pos) <= MOTOR_SHIFT_VAR)
			return FALSE;
		else
			Error(ERR_CONT, 0, "Stepper: Command %s: Motor Encoder at invalid position", _CmdName);
		}
    
	else if (abs(POS_ROT[WASH_POS] - steps) <= MOTOR_ROT_VAR)
	{
		if (abs(POS_ENC_ROT[WASH_POS] - Fpga.encoder[MOTOR_ROT].pos) <= MOTOR_SHIFT_VAR)
		{
            if (!RX_StepperStatus.robinfo.rob_in_wash)
                Error(ERR_CONT, 0, "Stepper: Command %s: Motor seems to have slippage, please check connection motor to clutch.", _CmdName);
            else
			return FALSE;
		}
		else
			Error(ERR_CONT, 0, "Stepper: Command %s: Motor Encoder at invalid position", _CmdName);
		}
    
	else if (abs(POS_ROT[VAC_POS] - steps) <= MOTOR_ROT_VAR)
	{
		if (abs(POS_ENC_ROT[VAC_POS] - Fpga.encoder[MOTOR_ROT].pos) <= MOTOR_SHIFT_VAR)
		{
            if (!RX_StepperStatus.robinfo.rob_in_vac)
                Error(ERR_CONT, 0, "Stepper: Command %s: Motor seems to have slippage, please check connection motor to clutch.", _CmdName);
            else
			return FALSE;
		}
		else
			Error(ERR_CONT, 0, "Stepper: Command %s: Motor Encoder at invalid position", _CmdName);
		}
    
	else if (abs(POS_ROT[WIPE_POS] - steps) <= MOTOR_ROT_VAR)
	{
		if (abs(POS_ENC_ROT[WIPE_POS] - Fpga.encoder[MOTOR_ROT].pos) <= MOTOR_SHIFT_VAR)
	{
            if (!RX_StepperStatus.robinfo.rob_in_wipe)
                Error(ERR_CONT, 0, "Stepper: Command %s: Motor seems to have slippage, please check connection motor to clutch.", _CmdName);
            else
		{
                _VacuumState = rob_vacuum_1_to_4;
			return FALSE;
		}
            
		}
		else
			Error(ERR_CONT, 0, "Stepper: Command %s: Motor Encoder at invalid position", _CmdName);
		}
    
	else if (abs(POS_ROT[VAC_POS_1_TO_4_TO_ALL] - steps) <= MOTOR_ROT_VAR)
	{
		if (abs(POS_ENC_ROT[VAC_POS_1_TO_4_TO_ALL] - Fpga.encoder[MOTOR_ROT].pos) <= MOTOR_SHIFT_VAR)
		{
			_VacuumState = rob_vacuum_all;
			return FALSE;
		}
		else
			Error(ERR_CONT, 0, "Stepper: Command %s: Motor Encoder at invalid position", _CmdName);
		}
    
	else if (abs(POS_ROT[VAC_POS_5_TO_8_TO_ALL] - steps) <= MOTOR_ROT_VAR)
	{
		if (abs(POS_ENC_ROT[VAC_POS_5_TO_8_TO_ALL] - Fpga.encoder[MOTOR_ROT].pos) <= MOTOR_SHIFT_VAR)
		{
			_VacuumState = rob_vacuum_all;
			return FALSE;
		}
		else
			Error(ERR_CONT, 0, "Stepper: Command %s: Motor Encoder at invalid position", _CmdName);
		}
    
	else
		Error(ERR_CONT, 0, "Stepper: Command %s: Motor Steps at invalid position", _CmdName);
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
	RX_StepperStatus.robinfo.z_in_ref		= fpga_input(ROT_STORED_IN); // Reference Sensor Rotation
	RX_StepperStatus.robinfo.x_in_ref		= fpga_input(SHIFT_STORED_IN); // Reference Sensor Shift
	RX_StepperStatus.robinfo.rob_in_wipe	= fpga_input(ROT_STORED_IN) && !fpga_input(ROT_VAC_OR_WASH_IN) && !fpga_input(ROT_WASH_OR_CAP_IN);
	RX_StepperStatus.robinfo.rob_in_vac		= fpga_input(ROT_VAC_OR_WASH_IN) && !fpga_input(ROT_WASH_OR_CAP_IN) && !fpga_input(ROT_STORED_IN);
	RX_StepperStatus.robinfo.rob_in_wash	= fpga_input(ROT_VAC_OR_WASH_IN) && fpga_input(ROT_WASH_OR_CAP_IN) && !fpga_input(ROT_STORED_IN);
	RX_StepperStatus.robinfo.rob_in_cap		= fpga_input(ROT_WASH_OR_CAP_IN) && !fpga_input(ROT_VAC_OR_WASH_IN) && !fpga_input(ROT_STORED_IN);
	
	if (RX_StepperStatus.robinfo.rob_in_cap) _ParRotDrive.speed = 600;
	else if (RX_StepperStatus.robinfo.rob_in_vac || RX_StepperStatus.robinfo.rob_in_wash || RX_StepperStatus.robinfo.rob_in_wipe) _ParRotDrive.speed = 800;

    if (fpga_input(ROT_STORED_IN) && (fpga_input(ROT_VAC_OR_WASH_IN) || fpga_input(ROT_WASH_OR_CAP_IN)))
	{
		ErrorFlag(ERR_CONT, &_txrob_Error, 0x01, 0, "CLN Command %s: Invalid position sensors combination", _CmdName);
		RX_StepperStatus.robinfo.ref_done = FALSE; 
		RX_StepperStatus.cmdRunning = FALSE;
		_NewCmd = FALSE;
	}
	
	RX_StepperStatus.posZ = _rot_steps_2_rev(motor_get_step(MOTOR_ROT));
	RX_StepperStatus.posX = _shift_steps_2_micron(motor_get_step(MOTOR_SHIFT));

	// --- set positions False while moving ---
	if (RX_StepperStatus.robinfo.moving) //  && (Fpga.stat->moving != 0))
	{
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
        _TimeWastePumpStart = 0;
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
        if (!_TimeWastePumpStart) _TimeWastePumpStart = rx_get_ticks();
    }

    if (_TimeWastePumpStart && _TimeWastePumpStart > rx_get_ticks() - _TimeWastePump)
    {
        Fpga.par->output |= PUMP_WASTE_BASE;
        Fpga.par->output |= PUMP_WASTE_VAC;
    }
    else
    {
        Fpga.par->output &= ~PUMP_WASTE_BASE;
        Fpga.par->output &= ~PUMP_WASTE_VAC;
	}

    if (RX_StepperStatus.robinfo.ref_done) _Reference_Count = 0;


	if (_TimeCnt != 0) _TimeCnt--; // waiting time
	// --- Executed after each move ---
    if (RX_StepperStatus.cmdRunning && motors_move_done(MOTOR_ALL_ROBOT_BITS) && (_TimeCnt == 0))
	{
		TrPrintfL(TRUE, "Move done >>%s<<", _CmdName);
		RX_StepperStatus.robinfo.moving = FALSE;
		int loc_move_pos = _NewCmdPos;
		int loc_new_cmd = _NewCmd;

		Fpga.par->output &= ~PUMP_FLUSH_WET; // PUMP_FLUSH_WET OFF

		// --- tasks after motor error ---
        if ((RX_StepperStatus.cmdRunning != CMD_ROB_REFERENCE) && (RX_StepperStatus.cmdRunning != CMD_ROB_ROT_REF) && (RX_StepperStatus.cmdRunning != CMD_ROB_ROT_REF2) && (RX_StepperStatus.cmdRunning != FALSE) && (RX_StepperStatus.cmdRunning != CMD_ROB_SHIFT_REF))
        {
            if (motors_error(MOTOR_ALL_ROBOT_BITS, &motor))
			{
				loc_new_cmd = FALSE;
				RX_StepperStatus.robinfo.ref_done = FALSE;
				Error(ERR_CONT, 0, "Stepper: Command %s: Motor[%d] blocked", _CmdName, motor + 1);
                RX_StepperStatus.cmdRunning = FALSE;
				Fpga.par->output &= ~RO_ALL_OUTPUTS; // set all output to off
			}
            else if (_rot_pos_check_err(motor_get_step(MOTOR_ROT)))
			{
                Error(ERR_CONT, 0, "CLN: Command %s: position corrupt, try referencing", _CmdName);
                RX_StepperStatus.robinfo.ref_done = FALSE;
                RX_StepperStatus.cmdRunning = FALSE;
				loc_new_cmd = FALSE;
			}
		}


		// --- tasks after motor ref ---
        if (RX_StepperStatus.cmdRunning == CMD_ROB_REFERENCE)
        {
            loc_new_cmd = CMD_ROB_SHIFT_REF; // CMD_ROB_SHIFT_REF; // CMD_ROB_ROT_REF;
		}

        if (RX_StepperStatus.cmdRunning == CMD_ROB_SHIFT_REF && _NewCmd == FALSE)
		{
            RX_StepperStatus.robinfo.x_in_ref = fpga_input(SHIFT_STORED_IN); // Reference Sensor Shift
			if (RX_StepperStatus.robinfo.x_in_ref == TRUE)
			{
                loc_new_cmd = CMD_ROB_ROT_REF;
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

        if (RX_StepperStatus.cmdRunning == CMD_ROB_ROT_REF && _NewCmd == FALSE)
		{
			RX_StepperStatus.robinfo.z_in_ref = fpga_input(ROT_STORED_IN); // Reference Sensor Rotation
			if (motors_error(MOTOR_ROT_BITS, &motor))
			{
				if (RX_StepperStatus.robinfo.z_in_ref)
				{
					motors_reset(MOTOR_ROT_BITS);
					RX_StepperStatus.posZ = _rot_steps_2_rev(motor_get_step(MOTOR_ROT));
                    loc_new_cmd = CMD_ROB_ROT_REF2;
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
                        loc_new_cmd = CMD_ROB_ROT_REF;
                    }
				}
			}
			else
			{
				Error(ERR_CONT, 0, "Stepper: Command %s: No mechanical reference deteced", _CmdName);
			}
		}

        if (RX_StepperStatus.cmdRunning == CMD_ROB_ROT_REF2)
		{
			motors_reset(MOTOR_ROT_BITS);
			RX_StepperStatus.posZ = _rot_steps_2_rev(motor_get_step(MOTOR_ROT));
            RX_StepperStatus.robinfo.ref_done = TRUE;
		}
		
		//========================================== rot ==========================================
		if (RX_StepperStatus.cmdRunning == CMD_ROB_MOVE_POS) 
		{
			switch (_RobFunction)
			{
			case rob_fct_cap:	// Cap
				TrPrintfL(TRUE, "ROB_FUNCTION_CAP done");
				loc_new_cmd = CMD_ROB_SHIFT_MOV; 
				loc_move_pos = 1; // center
				break;

			case rob_fct_wash:	// Wash
				TrPrintfL(TRUE, "ROB_FUNCTION_WASH done");
				loc_new_cmd = CMD_ROB_SHIFT_MOV; 
				loc_move_pos = 2; // start
				break;
	
			case rob_fct_vacuum:	// Vacuum
				TrPrintfL(TRUE, "ROB_FUNCTION_VACUUM done");
				loc_new_cmd = CMD_ROB_SHIFT_MOV; 
				loc_move_pos = 2; // start
				_VacuumWaiting = TRUE;
				Fpga.par->output |= VAC_ON; // VACUUM ON
				break;
				
			case rob_fct_wipe: // Wipe
				TrPrintfL(TRUE, "ROB_FUNCTION_WIPE done");
				loc_new_cmd = CMD_ROB_SHIFT_MOV;
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
				loc_new_cmd = CMD_ROB_MOVE_POS;
				loc_move_pos = VAC_POS;
				break;
			}
		}
		
		if (RX_StepperStatus.cmdRunning == CMD_ROB_TILT_CAP) 
		{	
			TrPrintfL(TRUE, "CMD_ROB_TILT_CAP done");
			loc_new_cmd = CMD_ROB_WAIT;  // Tilt wait
			loc_move_pos = 2;
		}
		
		if (RX_StepperStatus.cmdRunning == CMD_ROB_WAIT) 
		{
			TrPrintfL(TRUE, "CMD_ROB_WAIT done");
			loc_new_cmd = CMD_ROB_MOVE_POS; 
			loc_move_pos = rob_fct_tilt; // Tilt end, move to Cap
		}
		
		//========================================== shift ==========================================
		if (RX_StepperStatus.cmdRunning == CMD_ROB_SHIFT_MOV) 
		{
			RX_StepperStatus.robinfo.move_ok = TRUE;
		
			TrPrintfL(TRUE, "CMD_ROB_SHIFT_MOV done, _LastRobPosCmd=%d, _RobFunction=%d", _LastRobPosCmd, _RobFunction);

			switch (_LastRobPosCmd)
			{
			case 0:	// Ref
				motors_reset(MOTOR_SHIFT_BITS);
				RX_StepperStatus.posX = _shift_steps_2_micron(motor_get_step(MOTOR_SHIFT));
				RX_StepperStatus.robinfo.ref_done = TRUE;
				break;

			case 1:	// Center
				loc_new_cmd = CMD_ROB_FILL_CAP; 
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
                    Fpga.par->output &= ~VAC_ON; // Vacuum OFF
					break;
				
				case rob_fct_wipe: // Wipe
					break;
					
				case rob_fct_tilt: // Tilt
					break;
				}
				break;
			}
		}
		else
		{
			RX_StepperStatus.robinfo.move_ok = FALSE;
		}
		
		//========================================== timers ==========================================
		if (RX_StepperStatus.cmdRunning == CMD_ROB_FILL_CAP) 
		{
			Fpga.par->output &= ~PUMP_FLUSH_CAP; // Flush Cap OFF
			loc_new_cmd = CMD_ROB_TILT_CAP; // empty cap partially by tilting the cap
		}
		
		if (RX_StepperStatus.cmdRunning == CMD_ROB_SHIFT_LEFT)
		{
			txrob_handle_ctrl_msg(INVALID_SOCKET, CMD_ROB_MOVE_POS, &_MoveLeftPos);
		}
		else
		{
		// --- Reset Commands ---
			RX_StepperStatus.cmdRunning = FALSE;
			_NewCmdPos = 0;
			_NewCmd = FALSE;
		}
		// --- Execute next Commands ---
		switch (loc_new_cmd)
		{
		case CMD_ROB_REFERENCE:		txrob_handle_ctrl_msg(INVALID_SOCKET, CMD_ROB_REFERENCE, NULL); break;
		case CMD_ROB_SHIFT_REF:		txrob_handle_ctrl_msg(INVALID_SOCKET, CMD_ROB_SHIFT_REF, NULL); break;
		case CMD_ROB_ROT_REF:		txrob_handle_ctrl_msg(INVALID_SOCKET, CMD_ROB_ROT_REF, NULL); break;
		case CMD_ROB_ROT_REF2:		txrob_handle_ctrl_msg(INVALID_SOCKET, CMD_ROB_ROT_REF2, NULL); break;
		case CMD_ROB_MOVE_POS:		txrob_handle_ctrl_msg(INVALID_SOCKET, CMD_ROB_MOVE_POS, &loc_move_pos); break;
		case CMD_ROB_SHIFT_MOV:		txrob_handle_ctrl_msg(INVALID_SOCKET, CMD_ROB_SHIFT_MOV, &loc_move_pos); break;
		case CMD_ROB_FILL_CAP :		txrob_handle_ctrl_msg(INVALID_SOCKET, CMD_ROB_FILL_CAP, NULL); break;
		case CMD_ROB_TILT_CAP :		txrob_handle_ctrl_msg(INVALID_SOCKET, CMD_ROB_TILT_CAP, NULL); break;
		case CMD_ROB_WAIT:			txrob_handle_ctrl_msg(INVALID_SOCKET, CMD_ROB_WAIT, &loc_move_pos); break;
		case 0: break;
		default:				Error(ERR_CONT, 0, "TXROB_MAIN: Command 0x%08x not implemented", loc_new_cmd); break;
		}
		loc_new_cmd = FALSE;
		loc_move_pos = 0;

	}
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
	term_printf("moving:              %d		cmd: %08x\n", RX_StepperStatus.robinfo.moving, RX_StepperStatus.cmdRunning);
	term_printf("move ok:             %d\n", RX_StepperStatus.robinfo.move_ok);
	term_printf("Sensor Ref Rotation: %d\n", RX_StepperStatus.robinfo.z_in_ref);
	term_printf("Sensor Ref Shift:    %d\n", RX_StepperStatus.robinfo.x_in_ref);
	term_printf("Ref done:            %d\n", RX_StepperStatus.robinfo.ref_done);
	term_printf("Rotation in rev:     %06d\n", RX_StepperStatus.posZ);
	term_printf("Shift in um:         %06d\n", RX_StepperStatus.posX);
	term_printf("Tilt for Cap:        %06d\n", _TiltSteps);
	term_printf("Time Waste Vac [s]:  %06d\n", _TimeWastePump);
	term_printf("Time fill Cap [s]:   %06d\n", _TimeFillCap);
	term_printf("Speed Shift [mm/s]:  %06d\n", _SpeedShift * 2 / 200 / 16);
	term_printf("Timer Count:         %06d\n", _TimeCnt);
	term_printf("\n");
	term_printf("Vaccum State:         %d\n", _VacuumState);
	term_printf("Vacuum Ready:         %d\n", RX_StepperStatus.robinfo.vacuum_ready);
	term_printf("Roboter in Wipe:      %d\n", RX_StepperStatus.robinfo.rob_in_wipe);
	term_printf("Roboter in Vac:       %d\n", RX_StepperStatus.robinfo.rob_in_vac);
	term_printf("Roboter in Wash:      %d\n", RX_StepperStatus.robinfo.rob_in_wash);
	term_printf("Roboter in Cap:       %d\n", RX_StepperStatus.robinfo.rob_in_cap);
    if ((_TimeWastePumpStart + _TimeWastePump - rx_get_ticks()) / 1000 > 0)
        term_printf("Time for waste pumps  %d\n", _TimeWastePumpStart / 1000 + _TimeWastePump / 1000 - rx_get_ticks() / 1000);
    else
        term_printf("Time for waste pumps  %d\n", 0);
    term_printf("\n");
}

static void _txrob_handle_menu(char *str)
{
    int i, pos;
    
    if (str)
    {
        switch (str[0])
        {
        case 's':	txrob_handle_ctrl_msg(INVALID_SOCKET, CMD_ROB_STOP, NULL); break;
        case 'r':   if (str[1] == 0)	for (i = 0; i < MOTOR_CNT; i++) motor_reset(i);
					else				motor_reset(atoi(&str[1]));
					break;
        case 'o':   Fpga.par->output ^= (1 << atoi(&str[1])); break;
        case 'R':	txrob_handle_ctrl_msg(INVALID_SOCKET, CMD_ROB_REFERENCE, NULL);	break;
        case 'c':	pos = 0;
					txrob_handle_ctrl_msg(INVALID_SOCKET, CMD_ROB_MOVE_POS, &pos);
					break;
        case 'e':	pos = 1;
					txrob_handle_ctrl_msg(INVALID_SOCKET, CMD_ROB_MOVE_POS, &pos);
					break;
        case 'a':	pos = 2;
					txrob_handle_ctrl_msg(INVALID_SOCKET, CMD_ROB_MOVE_POS, &pos);
					break;
        case 'w':	pos = 3;
					txrob_handle_ctrl_msg(INVALID_SOCKET, CMD_ROB_MOVE_POS, &pos);
					break;
        case 'b':	pos = 3;
					txrob_handle_ctrl_msg(INVALID_SOCKET, CMD_ROB_SHIFT_MOV, &pos);
					break;
        case 'd':	pos = atoi(&str[1]);
					if (pos < 0) pos = 0;
					if (pos > 800) pos = 800;
					_RotRefOffset = pos;
					break;
        case 't':	pos = atoi(&str[1]);
					if (pos < 0) pos = 0;
					if (pos > 1000) pos = 1000;
					_TiltSteps = pos;
					break;
        case 'p':	pos = atoi(&str[1]);
					if (pos < 0) pos = 0;
					if (pos > 300) pos = 300;
					_TimeWastePump = pos;
					break;
        case 'f':	pos = atoi(&str[1]);
					if (pos < 0) pos = 0;
					if (pos > 300) pos = 300;
					_TimeFillCap = pos;
					break;
        case 'v':	pos = (atoi(&str[1])) * 200 * 16 / 2;
					if (pos < 500) pos = 500;
					if (pos > 21333) pos = 21333;
					_SpeedShift = pos;
					break;
        case 'm':	_txrob_motor_test(str[1] - '0', atoi(&str[2])); break;
        }
    }
}

//--- txrob_menu --------------------------------------------------
int txrob_menu(void)
{
	char str[MAX_PATH];
	int synth = FALSE;
	static int cnt = 0;

	_txrob_display_status();
    tx80x_wd_display_status();
    if (_Menu == 1)	term_printf("TX ROB MENU --------------------------\n");
    else			term_printf("TX WRINKLE DETECTION MENU ------------\n");

    if (_Menu == 1)
    {
        if (_Help)
        {
	term_printf("s: STOP\n");
            term_printf("r<n>: reset motor<n>\n");
            term_printf("o: toggle output <no>\n");
	term_printf("R: Reference\n");
	term_printf("c: move to capping\n");
            term_printf("e: move to wash\n");
	term_printf("a: move to vacuum\n");
	term_printf("w: move to wipe\n");
	term_printf("b: shift back\n");
	term_printf("d<steps>: set Rotation RefOffset <steps>\n");
	term_printf("t<steps>: set Tilt for Cap <steps>\n");
	term_printf("p<sec>: set Time Waste Vac <sec>\n");
	term_printf("f<sec>: set Time fill Cap <sec>\n");
            term_printf("v<mm/s>: set Speed Shift for wash <mm/s>\n");
            term_printf("m<n><steps>: move Motor<n> by <steps>\n");
	term_printf("x: exit\n");
            
        }
        else
        {
            term_printf("?: help\n");
            term_printf("2: WRINKLE DETECTION menu\n");
        }
    }
    else tx80x_wd_menu(_Help);

	term_printf(">");
	term_flush();

	if (term_get_str(str, sizeof(str)))
	{
        _Help = FALSE;
		switch (str[0])
		{
        case '?': _Help = TRUE; break;
        case '1': _Menu = 1; break;
        case '2': _Menu = 2; break;
		case 'x': return FALSE;
        default:
            if (_Menu == 1) _txrob_handle_menu(str);
            else			tx80x_wd_handle_menu(str);
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
	int firsttime = 0;
	
	switch (msgId)
	{	
	case CMD_ROB_STOP:				strcpy(_CmdName, "CMD_ROB_STOP");
		motors_stop(MOTOR_ALL_ROBOT_BITS); // Stops Hub and Robot
		Fpga.par->output &= ~RO_ALL_OUTPUTS; // set all output to off
		RX_StepperStatus.robinfo.moving = TRUE;
		RX_StepperStatus.cmdRunning = msgId;
		// reset var
		_LastRobPosCmd = 0;
		_RobFunction = rob_fct_cap;
		_TimeCnt = 0;
		_NewCmdPos = 0;
		_NewCmd = FALSE;
		_WipeWaiting = FALSE;
		_VacuumWaiting = FALSE;
		break;

	case CMD_ROB_REFERENCE:
        if (RX_StepperStatus.cmdRunning == CMD_ROB_REFERENCE ||
            RX_StepperStatus.cmdRunning == CMD_ROB_SHIFT_REF ||
            RX_StepperStatus.cmdRunning == CMD_ROB_ROT_REF ||
            RX_StepperStatus.cmdRunning == CMD_ROB_ROT_REF2)
            break;
        strcpy(_CmdName, "CMD_ROB_REFERENCE");
		TrPrintfL(TRUE, "SOCKET[%d]: %s", socket, _CmdName);
		if (RX_StepperStatus.cmdRunning && RX_StepperStatus.cmdRunning != CMD_FLUID_CTRL_MODE)
		{ 
            txrob_handle_ctrl_msg(INVALID_SOCKET, CMD_ROB_STOP, NULL); _NewCmd = CMD_ROB_REFERENCE; break; 
        }
		motor_reset(MOTOR_ROT); // to recover from move count missalignment
		motor_reset(MOTOR_SHIFT); // to recover from move count missalignment
		Fpga.par->output &= ~RO_ALL_OUTPUTS;
		RX_StepperStatus.robinfo.ref_done = FALSE;
		RX_StepperStatus.cmdRunning = msgId;
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
		
	case CMD_ROB_SHIFT_REF:			strcpy(_CmdName, "CMD_ROB_SHIFT_REF");
		TrPrintfL(TRUE, "SOCKET[%d]: %s", socket, _CmdName);
		if (RX_StepperStatus.cmdRunning && RX_StepperStatus.cmdRunning != CMD_FLUID_CTRL_MODE){ 
            txrob_handle_ctrl_msg(INVALID_SOCKET, CMD_ROB_STOP, NULL); _NewCmd = CMD_ROB_SHIFT_REF; break; 
            }
		motor_reset(MOTOR_SHIFT); // to recover from move count missalignment
		Fpga.par->output &= ~RO_ALL_OUTPUTS;
		RX_StepperStatus.robinfo.ref_done = FALSE;
		RX_StepperStatus.robinfo.moving = TRUE;
		RX_StepperStatus.cmdRunning = msgId;
		_NewCmd = FALSE;
		if (fpga_input(SHIFT_STORED_IN) == TRUE){ motors_move_by_step(MOTOR_SHIFT_BITS, &_ParShiftDrive, SHIFT_STEPS_PER_METER * 0.01, TRUE); _NewCmd = CMD_ROB_SHIFT_REF; break; }
		motors_move_by_step(MOTOR_SHIFT_BITS, &_ParShiftSensRef, -SHIFT_STEPS_PER_METER * 0.09, TRUE);
		break;
		
	case CMD_ROB_ROT_REF:			strcpy(_CmdName, "CMD_ROB_ROT_REF");
		TrPrintfL(TRUE, "SOCKET[%d]: %s", socket, _CmdName);
		if (RX_StepperStatus.cmdRunning && RX_StepperStatus.cmdRunning != CMD_FLUID_CTRL_MODE){
            txrob_handle_ctrl_msg(INVALID_SOCKET, CMD_ROB_STOP, NULL); _NewCmd = CMD_ROB_ROT_REF; break;
            }
		motor_reset(MOTOR_ROT); // to recover from move count missalignment
		Fpga.par->output &= ~RO_ALL_OUTPUTS;
		RX_StepperStatus.robinfo.ref_done = FALSE;
		RX_StepperStatus.robinfo.moving = TRUE;
		RX_StepperStatus.cmdRunning = msgId;
		_NewCmd = FALSE;
		if (fpga_input(ROT_STORED_IN) == TRUE){	motors_move_by_step(MOTOR_ROT_BITS, &_ParRotDrive, ROT_STEPS_PER_REV * 0.02, TRUE); _NewCmd = CMD_ROB_ROT_REF; break;}
		motors_move_by_step(MOTOR_ROT_BITS, &_ParRotSensRef, -ROT_STEPS_PER_REV, TRUE);
        _Reference_Count++;
		break;
		
	case CMD_ROB_ROT_REF2:	strcpy(_CmdName, "CMD_ROB_ROT_REF2");
		TrPrintfL(TRUE, "SOCKET[%d]: %s", socket, _CmdName);
		if (RX_StepperStatus.cmdRunning && RX_StepperStatus.cmdRunning != CMD_FLUID_CTRL_MODE){ 
            txrob_handle_ctrl_msg(INVALID_SOCKET, CMD_ROB_STOP, NULL); _NewCmd = CMD_ROB_ROT_REF2; break; 
            }
		motor_reset(MOTOR_ROT); // to recover from move count missalignment
		Fpga.par->output &= ~RO_ALL_OUTPUTS;
		RX_StepperStatus.robinfo.ref_done = FALSE;
		RX_StepperStatus.robinfo.moving = TRUE;
		RX_StepperStatus.cmdRunning = msgId;
		_NewCmd = FALSE;
		motors_move_by_step(MOTOR_ROT_BITS, &_ParRotDrive, _RotRefOffset, TRUE);
		break;
		
	case CMD_ROB_MOVE_POS:		strcpy(_CmdName, "CMD_ROB_MOVE_POS");
		pos = *((INT32*)pdata);
		TrPrintfL(TRUE, "SOCKET[%d]: %s rob_function=%d", socket, _CmdName, pos);
		// Error(LOG, 0, "%s rob_function=%d: wipe_ready=%d, vacuum_ready=%d", _CmdName, pos, RX_StepperStatus.robinfo.wipe_ready, RX_StepperStatus.robinfo.vacuum_ready);

		if (!RX_StepperStatus.cmdRunning || RX_StepperStatus.cmdRunning == CMD_ROB_SHIFT_LEFT)
		{
			if (!RX_StepperStatus.robinfo.ref_done)
			{ 
				Error(ERR_CONT,0, "Robot not referenced, cmd=0x%08x", msgId);
				break;
			}

			if (pos < rob_fct_cap) {Error(LOG, 0, "CLN: Command %s: negative position not allowed", _CmdName); break;}
			if (pos > rob_fct_vacuum_all) {Error(LOG, 0, "CLN: Command %s: too high pos", _CmdName); break;}

			if (POS_SHIFT[POS_SHIFT_MAX_TURN_POS] + MOTOR_SHIFT_VAR < motor_get_step(MOTOR_SHIFT) && RX_StepperStatus.robinfo.moving == FALSE)
			{
				RX_StepperStatus.cmdRunning = 0;
				int left_pos = POS_SHIFT_MAX_TURN_POS;
				txrob_handle_ctrl_msg(INVALID_SOCKET, CMD_ROB_SHIFT_LEFT, &left_pos);
				_MoveLeftPos = pos;
			}
			else if (POS_SHIFT[POS_SHIFT_MAX_TURN_POS] + MOTOR_SHIFT_VAR >= motor_get_step(MOTOR_SHIFT))
			{
				RX_StepperStatus.cmdRunning = msgId;
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
				
				sok_send_2(&socket, REP_STEPPER_STAT, sizeof(RX_StepperStatus), &RX_StepperStatus);
				motors_move_to_step(MOTOR_ROT_BITS, &_ParRotDrive, pos);
			}		
		}
		// else Error(WARN, 0, "Command refused (cmdRunning=0x%08x)", RX_StepperStatus.cmdRunning);
		break;
		
	case CMD_ROB_SHIFT_MOV:		strcpy(_CmdName, "CMD_ROB_SHIFT_MOV");
		TrPrintfL(TRUE, "SOCKET[%d]: %s", socket, _CmdName);
		if (!RX_StepperStatus.cmdRunning || RX_StepperStatus.cmdRunning == CMD_FLUID_CTRL_MODE)
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
			RX_StepperStatus.cmdRunning = msgId;
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
                if (RX_StepperStatus.robinfo.rob_in_vac) Fpga.par->output |= VAC_ON;
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
		
	case CMD_ROB_SHIFT_LEFT:	strcpy(_CmdName, "CMD_ROB_SHIFT_LEFT");
		TrPrintfL(TRUE, "SOCKET[%d]: %s", socket, _CmdName);
		if (!RX_StepperStatus.cmdRunning || RX_StepperStatus.cmdRunning == CMD_FLUID_CTRL_MODE)
		{
			if (!RX_StepperStatus.robinfo.ref_done)
			{ 
				Error(ERR_CONT,0, "Robot not referenced, cmd=0x%08x", msgId);
				break;
			}
			pos = *((INT32*)pdata);
			if (pos < 0) {Error(LOG, 0, "CLN: Command %s: negative position not allowed", _CmdName); break;}
			if (pos >= POS_SHIFT_CNT) {Error(LOG, 0, "CLN: Command %s: too high pos", _CmdName); break;}
			//if (_RobFunction == rob_fct_cap && pos != 1) {Error(LOG, 0, "CLN: Command %s: cancle shift, robot in capping", _CmdName); break;}
			RX_StepperStatus.cmdRunning = msgId;
			RX_StepperStatus.robinfo.moving = TRUE;
			pos = POS_SHIFT[pos];
			_ParShiftWet.speed = _SpeedShift;
			motors_move_to_step(MOTOR_SHIFT_BITS, &_ParShiftWet, pos);
		}
		
	case CMD_ROB_FILL_CAP:		strcpy(_CmdName, "CMD_ROB_FILL_CAP");
		TrPrintfL(TRUE, "SOCKET[%d]: %s", socket, _CmdName);
		if (!RX_StepperStatus.cmdRunning || RX_StepperStatus.cmdRunning == CMD_FLUID_CTRL_MODE)
		{
			Fpga.par->output |= PUMP_FLUSH_CAP; // Flush Cap ON
			RX_StepperStatus.cmdRunning = msgId;
			RX_StepperStatus.robinfo.moving = TRUE;
			_TimeCnt = _TimeFillCap * MAIN_PER_SEC;
		}
		break;
		
	case CMD_ROB_WAIT:		strcpy(_CmdName, "CMD_ROB_WAIT");
		TrPrintfL(TRUE, "SOCKET[%d]: %s", socket, _CmdName);
		if (!RX_StepperStatus.cmdRunning || RX_StepperStatus.cmdRunning == CMD_FLUID_CTRL_MODE)
		{
			pos = *((INT32*)pdata);
			RX_StepperStatus.cmdRunning = msgId;
			RX_StepperStatus.robinfo.moving = TRUE;
			_TimeCnt = pos * MAIN_PER_SEC;
		}
		break;
		
	case CMD_ROB_TILT_CAP:		strcpy(_CmdName, "CMD_ROB_TILT_CAP");
		TrPrintfL(TRUE, "SOCKET[%d]: %s", socket, _CmdName);
		if (!RX_StepperStatus.cmdRunning || RX_StepperStatus.cmdRunning == CMD_FLUID_CTRL_MODE)
		{
			if (!RX_StepperStatus.robinfo.ref_done)
			{ 
				Error(ERR_CONT,0, "Robot not referenced, cmd=0x%08x", msgId);
				break;
			}
			RX_StepperStatus.cmdRunning = msgId;
			RX_StepperStatus.robinfo.moving = TRUE;
			motors_move_to_step(MOTOR_ROT_BITS, &_ParRotDrive, POS_ROT_TILT);
		}
		
		break;

	case CMD_ERROR_RESET:		
		TrPrintfL(TRUE, "SOCKET[%d]: %s", socket, _CmdName);
		fpga_stepper_error_reset();
		_txrob_error_reset();
		break;
        
    case CMD_LIFT_STOP:
    case CMD_LIFT_REFERENCE:
    case CMD_LIFT_PRINT_POS:
    case CMD_LIFT_UP_POS:
    case CMD_LIFT_CAPPING_POS:
    case CMD_LIFT_WASH_POS:
    case CMD_LIFT_WIPE_POS:
    case CMD_LIFT_VACUUM_POS: break;

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
	RX_StepperStatus.cmdRunning = 1;
	
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
	else if (motorNo == MOTOR_SHIFT)
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
    else if (motorNo == 2 || motorNo == 3)
    {
		//---> CALL function from wd
        // paramaters tested 14-JAN-20
        par.speed = 10000; // speed with max tork: 21'333
        par.accel = 32000;
        par.current_acc = 400.0;
        par.current_run = 300.0;
        par.stop_mux = 0;
        par.dis_mux_in = 0;
    //  par.encCheck = chk_std;
        par.encCheck = chk_off;
        motors_config(motors, 50.0, L3518_STEPS_PER_METER, L3518_INC_PER_METER, STEPS);
        motors_move_by_step(motors, &par, steps, FALSE);
    }
    else
    {
        par.speed = 1000;
        par.accel = 1000;
        par.current_acc = 250.0;
        par.current_run = 250.0;
        par.stop_mux = 0;
        par.dis_mux_in = 0;
        par.encCheck = chk_off;
        motors_config(motors, 50, L3518_STEPS_PER_METER, L3518_INC_PER_METER, STEPS);
        motors_move_by_step(motors, &par, steps, FALSE);
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
	if (_socket != INVALID_SOCKET) sok_send_2(&_socket, REP_STEPPER_STAT, sizeof(RX_StepperStatus), &RX_StepperStatus);
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
	if ((RX_StepperStatus.robinfo.moving == FALSE) && abs(POS_ROT[VAC_POS] - motor_get_step(MOTOR_ROT)) <= MOTOR_ROT_VAR && _VacuumWaiting == FALSE)
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




