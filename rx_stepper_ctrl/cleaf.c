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
#include "cleaf.h"

//--- defines -----------------------------------------------------------------------------------------------------------------------------

// General
#define MAIN_PER_SEC	930			// Main executions per sec
#define STEPS_REV		(200*16)	// steps per motor revolution * 16 microsteps
#define MOTOR_ALL_BITS	0x0F 

#define SHOW_LIFT	1
#define SHOW_ROB	2
#define SHOW_PUMP	3

// Lift Z Axis
#define MOTOR_Z_CNT		2
#define MOTOR_Z_0		0
#define MOTOR_Z_1		1
#define MOTOR_Z_BITS	((0x01<<MOTOR_Z_0) | (0x01<<MOTOR_Z_1))

#define CURRENT_Z_HOLD	50

#define POS_UP				1000
#define PRINT_HEIGHT_MIN	1000

#define DIST_Z_REV		2000.0	// moving distance per revolution [µm]

// Cap X Axis
#define MOTOR_ROB_CNT	1
#define MOTOR_ROB_0		2

#define MOTOR_ROB_BITS		(0x01<<MOTOR_ROB_0)

#define CURRENT_X_HOLD	    7.0

#define ROB_STEPS_PER_METER		909456.8
#define ROB_INC_PER_METER		1136821.0	// 909456.8 / 0.8

// Pump
#define MOTOR_WASTE_PUMP	4
#define WASTE_PUMP_STEPS		500000 // steps per trigger period

#define CURRENT_PUMP_HOLD	    7.0

#define PUMP_0		3 // for pwm -> outputs at output 11
#define PUMP_THRESH_LVL		150
#define PUMP_0_FILL_CAP		5000 // 16340 // 65535 / 100 * 25 = 16340
#define PUMP_0_WET_WIPE		10000 // 30000 // 65535 /100 * 75 = 50000 
#define PUMP_1_WASTE		100
//
#define MOTOR_WASTE_PUMP_BITS	0x10

// Laser
#define	LASER_IN				0		// Analog Input 0-3
#define LASER_VOLT_PER_MM		0.25 // V/mm
#define LASER_MM_PER_VOLT		4    // mm/V
#define LASER_VOLT_OFFSET 		5900 // Laser Value without medium in mV
#define LASER_VARIATION			400 // Variation +- in um
#define LASER_TIMEOUT			2000 // ms
#define HEAD_DOWN_EN_DELAY		10
#define LASER_ANALOG_AVR		10.0
//#define MAX_POS_UM		 		90000 //Resolution 20um, Working range 45...85 mm, temperature drift 18 um/K, Value range 0...10V linear rising, 1V=4mm ?

#define VAL_TO_MV_AI(x) ((x*10000)/0x0fff)

// Digital Inputs (max 12)
#define HEAD_UP_IN_0		0
#define HEAD_UP_IN_1		1
#define RO_STORED_IN_0		2

// DRIP PAN FUNCTION
#define DRIP_PANS_INFEED_DOWN		5
#define DRIP_PANS_INFEED_UP			6
#define DRIP_PANS_OUTFEED_DOWN		7
#define DRIP_PANS_OUTFEED_UP		8

#define PRINTHEAD_EN		11	// Input from SPS // '1' Allows Head to go down

// Digital Outputs (max 12)
#define RO_FLUSH_CAR_0			0x001 //  0 // Y1 Flush Servicecar 0
#define RO_BLADE_UP_0			0x008 //  3 // SY13 Wipe-Blade 0
#define RO_VACUUM_BLADE			0x020 //  5 // Y8 Vakuum Blade
#define RO_VACUUM				0x040 //  6 // SY10 Vakuum
#define RO_DRAIN_WASTE			0x080 // 7 // Y6 & Y7 // 1 == Waste Beh. Absaugen // 0 == Cable Chain absaugen 
//#define RO_FLUSH_VOLT			0x100
#define RO_ALL_OUT_MECH			0x00E // ALL robot (blades and screws)
#define RO_ALL_OUTPUTS			0x2FF //  ALL Outputs

// DRIP PAN FUNCTION
#define DRIP_PANS_VALVE_ALL		0x300
#define DRIP_PANS_VALVE_UP		0x100
#define DRIP_PANS_VALVE_DOWN	0x200

// Vectors
static int	HEAD_UP_IN[MOTOR_Z_CNT] = {
	HEAD_UP_IN_0,			
	HEAD_UP_IN_1
};

static int	RO_STORED_IN[MOTOR_ROB_CNT] = {
	RO_STORED_IN_0
};

#define REF_HEIGHT			RX_StepperCfg.robot[RX_StepperCfg.boardNo].ref_height
#define HEAD_ALIGN			RX_StepperCfg.robot[RX_StepperCfg.boardNo].head_align
#define ROBOT_HEIGHT		RX_StepperCfg.robot[RX_StepperCfg.boardNo].robot_height
#define ROBOT_ALIGN			RX_StepperCfg.robot[RX_StepperCfg.boardNo].robot_align

#define CAPPING_HEIGHT		35000

#define SLIDE_CAPPING_POS		630000

#define LIFT_RESET				1000 // Reset height
#define POS_STORED				REF_HEIGHT-LIFT_RESET // ROBOT_HEIGHT


//--- global  variables -----------------------------------------------------------------------------------------------------------------
static int	_CmdRunning = 0;
static char	_CmdName[32];
static int	_ResetRetried = 0;
static int  _LastRobPosCmd = 0;
static int  _ShowStatus = 0;
static int	_NewCmd = 0;
static int	_NewCmdPos = 0;
static int	_TimeCnt = 0;
static int	_Step;
static int	_PrintPos;
static int	_DripPans_Connected = FALSE;
static int  _AllowMoveDown = 0;
static int  _DripPansPosition = 2;	// 2 = position unknown after start

// Lift
static SMovePar	_Par_Z_Ref[MOTOR_Z_CNT];
static SMovePar	_ParZ_down;
static SMovePar	_ParZ_cap;

// Rob
static SMovePar	_ParRob_ref[MOTOR_ROB_CNT];
static SMovePar	_ParRob_drive;
static SMovePar	_ParRob_wipe;
static SMovePar	_ParRob_wipe_vac;

// Laser
static int	_PrintHeadEn_On  = 0;
static int	_PrintHeadEn_Off = 0;
static int	_LaserAvr = 0;
static int	_LaserCnt = 0;

// Pump
static SMovePar	_ParPump_waste;
static int	_WastePumpOn = FALSE;

// Times
static int	_TimeWastePump = 20; // in s
static int	_TimeFillCap = 14; // in s
static int	_TimeEmptyCap = 30; // in s

static int	_LaserTimeThin;
static int	_LaserTimeThick;


static UINT32	_cleaf_Error[5];

//--- prototypes --------------------------------------------
static void _cleaf_motor_z_test(int steps);
static void _cleaf_motor_x_test(int steps);
static void _cleaf_motor_test(int motor, int steps);
static void _cleaf_error_reset(void);
static void _cleaf_send_status(RX_SOCKET);
static int _cleaf_retry_ref(int cmd);
static void _cleaf_motors_by_step(int motor_bits, int steps, SMovePar par[2]);
static void _cleaf_check_laser(void);

// Lift
static int  _z_micron_2_steps(int micron);
static int  _z_steps_2_micron(int steps);
static void _lift_do_reference(void);

// Robot
static int  _rob_micron_2_steps(int micron);
static int  _rob_steps_2_micron(int steps);

// Pump
static void _rob_set_threshold(int analog_in_nr, int threshold);
static void _rob_waste_pump(int steps);

// PWM
static void _rob_set_pwm(int pwm_nr, int pulse_width);

//--- cleaf_init --------------------------------------
void cleaf_init(void)
{
	int i;

	memset(_CmdName, 0, sizeof(_CmdName));

	Error(LOG, 0, "cleaf_init: material_thickness=%d", RX_StepperCfg.material_thickness);
	
	//--- movment parameters Lift ----------------
	for (i = 0; i < MOTOR_Z_CNT; i++) 
	{
		_Par_Z_Ref[i].speed			= 16000; // 10000; // 2mm/U // 10mm/s => 5U/s*200steps/U*16=16000  //  20mm/s => 10U/s*200steps/U*16=32000
		_Par_Z_Ref[i].accel			=  8000; // 5000;
		_Par_Z_Ref[i].current		= 250.0;
//		_Par_Z_Ref[i].stop_mux		= MOTOR_Z_BITS;
		_Par_Z_Ref[i].stop_mux		= 0;
		_Par_Z_Ref[i].dis_mux_in	= TRUE;
		_Par_Z_Ref[i].stop_in		= ESTOP_UNUSED;
		_Par_Z_Ref[i].stop_level	= 0;
		_Par_Z_Ref[i].estop_in		= HEAD_UP_IN[i]; // Check Ref Input
		_Par_Z_Ref[i].estop_level	= 1;
		_Par_Z_Ref[i].checkEncoder	= TRUE;
	}
	
	_ParZ_down.speed		= 30000; 
	_ParZ_down.accel		= 15000; 
//	_ParZ_down.current		= 300.0; 	
	_ParZ_down.current		= 350.0; 	
	_ParZ_down.stop_mux		= MOTOR_Z_BITS;
	_ParZ_down.dis_mux_in	= 0;
	_ParZ_down.estop_in     = ESTOP_UNUSED;
	_ParZ_down.estop_level  = 0;
	_ParZ_down.checkEncoder = TRUE;
	_ParZ_down.stop_in		= ESTOP_UNUSED;
	_ParZ_down.stop_level	= 0;
	_ParZ_down.dis_mux_in	= FALSE;

	_ParZ_cap.speed			= 10000; // 1000;
	_ParZ_cap.accel			=  5000; //1000;
	_ParZ_cap.current		= 150.0;
	_ParZ_cap.stop_mux		= FALSE;
	_ParZ_cap.dis_mux_in	= 0;
	_ParZ_cap.stop_in		= ESTOP_UNUSED;
	_ParZ_cap.stop_level	= 0;
	_ParZ_cap.estop_in      = ESTOP_UNUSED;
	_ParZ_cap.estop_level   = 0;
	_ParZ_cap.checkEncoder  = TRUE;
	_ParZ_cap.dis_mux_in	= FALSE;
	
	//--- movement parameters Roboter ---------------- 0.9 Nm with 100 U/min --- 0.8 Nm with 300 U/min -- 300*200*16/60
	for (i = 0; i < MOTOR_ROB_CNT; i++) 
	{
		_ParRob_ref[i].speed			= 20000; // 10000; // 4000; // speed with max tork: 21'333 // 8000; // 16000; // speed with max tork: 16'000
		_ParRob_ref[i].accel			= 10000; //  5000; // 2000; // 4000; //8000;
		_ParRob_ref[i].current			= 60.0; // max 67 = 0.67 A // 400.0; // max 424 = 4.24 A
		_ParRob_ref[i].stop_mux			= MOTOR_ROB_BITS;
		_ParRob_ref[i].dis_mux_in		= 0;
		_ParRob_ref[i].estop_level		= 1; // stopp when sensor on
		_ParRob_ref[i].checkEncoder		= TRUE;
		_ParRob_ref[i].stop_in			= ESTOP_UNUSED;
		_ParRob_ref[i].stop_level		= 0;
		_ParRob_ref[i].dis_mux_in		= TRUE;
		_ParRob_ref[i].estop_in			= RO_STORED_IN[i]; // Check Ref Input
	}
	
	_ParRob_drive.speed				= 21000; // 8000; // speed with max tork: 21'333 // 16000; // 32000; // speed with max tork: 16'000
	_ParRob_drive.accel				= 10000; // 2000; // 8000; // 16000;
	_ParRob_drive.current			= 60.0; // max 67 = 0.67 A // 400.0; // max 424 = 4.24 A
	_ParRob_drive.stop_mux			= MOTOR_ROB_BITS;
	_ParRob_drive.dis_mux_in		= 0;
	_ParRob_drive.estop_in			= ESTOP_UNUSED;
	_ParRob_drive.estop_level		= 1;
	_ParRob_drive.checkEncoder		= TRUE;
	_ParRob_drive.stop_in			= ESTOP_UNUSED;
	_ParRob_drive.stop_level		= 0;
	_ParRob_drive.dis_mux_in		= FALSE;

	_ParRob_wipe.speed				= 20000; // 4000; // speed with max tork: 21'333 // 8000; // 25000; // speed with max tork: 16'000
	_ParRob_wipe.accel				= 10000; // 2000; // 4000;// 12500;
	_ParRob_wipe.current			= 60.0; // 40.0; // max 67 = 0.67 A // 300.0; // max 424 = 4.24 A
	_ParRob_wipe.stop_mux			= MOTOR_ROB_BITS;
	_ParRob_wipe.dis_mux_in			= 0;
	_ParRob_wipe.estop_in			= ESTOP_UNUSED;
	_ParRob_wipe.estop_level		= 1;
	_ParRob_wipe.checkEncoder		= TRUE;
	_ParRob_wipe.stop_in			= ESTOP_UNUSED;
	_ParRob_wipe.stop_level			= 0;
	_ParRob_wipe.dis_mux_in			= FALSE;
	
	_ParRob_wipe_vac.speed			= 10000; // 4000; // speed with max tork: 21'333 // 8000; // 25000; // speed with max tork: 16'000
	_ParRob_wipe_vac.accel			=  5000; // 2000; // 4000;// 12500;
	_ParRob_wipe_vac.current		= 60.0; // 40.0; // max 67 = 0.67 A // 300.0; // max 424 = 4.24 A
	_ParRob_wipe_vac.stop_mux		= MOTOR_ROB_BITS;
	_ParRob_wipe_vac.dis_mux_in		= 0;
	_ParRob_wipe_vac.estop_in		= ESTOP_UNUSED;
	_ParRob_wipe_vac.estop_level	= 1;
	_ParRob_wipe_vac.checkEncoder	= TRUE;
	_ParRob_wipe_vac.stop_in		= ESTOP_UNUSED;
	_ParRob_wipe_vac.stop_level		= 0;
	_ParRob_wipe_vac.dis_mux_in		= FALSE;
	
	//--- movment parameters Pump stepper motor ----------------
	_ParPump_waste.speed			= 20000; // 32000; // speed with max tork: 2k pulses per sec * 16 // 500u/min * 200 * 16 /60 = 26000
	_ParPump_waste.accel			= 10000; // 16000; // ;
	_ParPump_waste.current			= 80.0; // max 86 = 0.86 A
	_ParPump_waste.stop_mux			= 0;
	_ParPump_waste.dis_mux_in		= 0;
	_ParPump_waste.estop_in			= ESTOP_UNUSED;
	_ParPump_waste.estop_level		= 1;
	_ParPump_waste.checkEncoder		= FALSE;
	_ParPump_waste.stop_in			= ESTOP_UNUSED;
	_ParPump_waste.stop_level		= 0;
	_ParPump_waste.dis_mux_in		= FALSE;
	
	//--- Outputs ----------------
	Fpga.par->output &= ~RO_ALL_OUTPUTS; // ALL OUTPUTS
	//Fpga.par->output |= RO_FLUSH_VOLT;
	
	Fpga.par->adc_thresh[PUMP_0]    = PUMP_THRESH_LVL;
}

//--- cleaf_main ------------------------------------------------------------------
void cleaf_main(int ticks, int menu)
{
	int motor;
	int lift_pos;
	
	if (!motors_init_done())
	{
		motors_config(MOTOR_Z_BITS, CURRENT_Z_HOLD, 0.0, 0.0);
		motors_config(MOTOR_ROB_BITS, CURRENT_X_HOLD, ROB_STEPS_PER_METER, ROB_INC_PER_METER);
		motor_config(MOTOR_WASTE_PUMP, CURRENT_PUMP_HOLD, ROB_STEPS_PER_METER, ROB_INC_PER_METER);
	}
	
	if (RX_StepperCfg.printerType != printer_cleaf) return; // printer_cleaf
	motor_main(ticks, menu);
	
	RX_StepperStatus.info.x_in_cap = TRUE; // enable capping button
	
	// --- read Inputs Cap ---
	RX_StepperStatus.info.headUpInput_0 = fpga_input(HEAD_UP_IN_0);
	RX_StepperStatus.info.headUpInput_1 = fpga_input(HEAD_UP_IN_1);
	RX_StepperStatus.posZ = REF_HEIGHT - _z_steps_2_micron(motor_get_step(0));
	
	// --- read Inputs Rob ---
	RX_StepperStatus.info.x_in_ref	= TRUE; // fpga_input(RO_STORED_IN_0); // Reference Sensor
	RX_StepperStatus.posX			= -_rob_steps_2_micron(motor_get_step(MOTOR_ROB_0));
	
	// Drip Pans : enabled when the main send a command CMD_CLN_DRIP_PANS
	if (_DripPans_Connected)
	{
		RX_StepperStatus.info.DripPans_InfeedDOWN	= fpga_input(DRIP_PANS_INFEED_DOWN);
		RX_StepperStatus.info.DripPans_InfeedUP		= fpga_input(DRIP_PANS_INFEED_UP);
		RX_StepperStatus.info.DripPans_OutfeedDOWN	= fpga_input(DRIP_PANS_OUTFEED_DOWN);
		RX_StepperStatus.info.DripPans_OutfeedUP	= fpga_input(DRIP_PANS_OUTFEED_UP);
	}

	// --- set positions False while moving ---
	if (RX_StepperStatus.info.moving)
	{
		// RX_StepperStatus.info.z_in_ref		= (RX_StepperStatus.info.ref_done && (abs(RX_StepperStatus.posZ - REF_HEIGHT) <= 10 + LIFT_RESET));
		// RX_StepperStatus.info.x_in_ref		= (RX_StepperStatus.info.ref_done && (RX_StepperStatus.info.x_in_ref == 1));
		RX_StepperStatus.info.move_ok = FALSE;
	}
	if (_CmdRunning == 0) RX_StepperStatus.info.moving = FALSE;
	if (_TimeCnt != 0) _TimeCnt--; // waiting time
	// --- Executed after each move ---
	if (_CmdRunning && motors_move_done(MOTOR_ALL_BITS) && (_TimeCnt == 0))
	{		
		RX_StepperStatus.info.moving = FALSE;
		int loc_move_pos = _NewCmdPos;
		int loc_new_cmd = _NewCmd;
				
		// --- tasks after motor error ---
		if ((_CmdRunning != CMD_CAP_REFERENCE) && (_CmdRunning != CMD_CLN_REFERENCE))
		{
			if (motors_error(MOTOR_ALL_BITS, &motor))
			{
				loc_new_cmd = FALSE;
				RX_StepperStatus.info.ref_done = FALSE;
				RX_StepperStatus.info.z_ref_done = FALSE;
				Error(ERR_CONT, 0, "Stepper: Command %s: Motor[%d] blocked", _CmdName, motor + 1);
				_CmdRunning = FALSE;
				_ResetRetried = 0;
				Fpga.par->output &= ~RO_ALL_OUTPUTS; // set all output to off
			}			
		}
		
		if(_CmdRunning == CMD_CAP_REFERENCE)									RX_StepperStatus.info.z_in_ref = TRUE;
		if (motor_get_step(MOTOR_Z_0)>2000 || motor_get_step(MOTOR_Z_1)>2000)	RX_StepperStatus.info.z_in_ref = FALSE;

		// --- tasks after motor stop ---
		if (_CmdRunning == CMD_CAP_STOP)
		{
			_ResetRetried = 0;
			RX_StepperStatus.set_io_cnt = 0;
			RX_StepperStatus.adjustmentProgress = (int)100;
		}
		
		//========================================== cap ==========================================
		if (_CmdRunning == CMD_CAP_REFERENCE) 
		{
			if (!RX_StepperStatus.info.headUpInput_0) Error(LOG, 0, "LB702: Command REFERENCE: End Sensor 1 NOT HIGH");
			if (!RX_StepperStatus.info.headUpInput_1) Error(LOG, 0, "LB702: Command REFERENCE: End Sensor 2 NOT HIGH");
			RX_StepperStatus.info.z_ref_done =  RX_StepperStatus.info.headUpInput_0 && RX_StepperStatus.info.headUpInput_1;
			motors_reset(MOTOR_Z_BITS);
			RX_StepperStatus.posZ = REF_HEIGHT - _z_steps_2_micron(motor_get_step(0));
			if ((!RX_StepperStatus.info.headUpInput_0) || (!RX_StepperStatus.info.headUpInput_1))
			{
				loc_new_cmd = FALSE;
			}
			else
			{
				loc_new_cmd = CMD_CLN_REFERENCE;
				_ResetRetried = 0;
			}
		}

		//--- CMD_CAP_PRINT_POS ----------------------------------------------
		if (_CmdRunning ==  CMD_CAP_PRINT_POS)
		{
			loc_new_cmd = CMD_CAP_PRINT_POS;
			switch(++_Step)
			{
			case 1:	// Error(LOG, 0, "CMD_CAP_PRINT_POS: Start");
					if (RX_StepperStatus.posX>POS_UP) motors_move_to_step(MOTOR_Z_BITS, &_ParZ_down, POS_UP);
					break;

			case 2:	// Error(LOG, 0, "CMD_CAP_PRINT_POS: Step 1 done");
					motors_move_to_step(MOTOR_ROB_BITS, &_ParRob_drive, 0);
					break;
	
			case 3:	// Error(LOG, 0, "CMD_CAP_PRINT_POS: Step 1 done");
					motor_move_to_step(MOTOR_Z_0, &_ParZ_down, _z_micron_2_steps(REF_HEIGHT - _PrintPos));
					motor_move_to_step(MOTOR_Z_1, &_ParZ_down, _z_micron_2_steps(REF_HEIGHT - _PrintPos + HEAD_ALIGN));
					motors_start(MOTOR_Z_BITS, TRUE);
					break;
				
			case 4: // Error(LOG, 0, "CMD_CAP_PRINT_POS: done");
					loc_new_cmd=0;
					RX_StepperStatus.info.z_in_print = TRUE;
					break;
			}
		}

		//--- CMD_CAP_UP_POS ----------------------------------------------
		if (_CmdRunning ==  CMD_CAP_UP_POS)
		{
			loc_new_cmd = CMD_CAP_UP_POS;
			switch(++_Step)
			{
			case 1:	// Error(LOG, 0, "CMD_CAP_UP_POS: Start");
					motors_move_to_step(MOTOR_Z_BITS, &_ParZ_down, POS_UP);
					break;
				
			case 2:	// Error(LOG, 0, "CMD_CAP_UP_POS: Step 1 done");
					motors_move_to_step(MOTOR_ROB_BITS, &_ParRob_drive, 0);	
					break;
								
			case 3: loc_new_cmd=0;	
				//	Error(LOG, 0, "CMD_CAP_UP_POS: done");
					RX_StepperStatus.info.z_in_ref = TRUE;										
					break;
			}			
		}
		
		//--- CMD_CAP_CAPPING_POS ----------------------------------------------
		if (_CmdRunning ==  CMD_CAP_CAPPING_POS)
		{
			loc_new_cmd = CMD_CAP_CAPPING_POS;
			switch(++_Step)
			{
			case 1:	// Error(LOG, 0, "CMD_CAP_CAPPING_POS: Start");
					motors_move_to_step(MOTOR_Z_BITS, &_ParZ_down, POS_UP);											
					break;
				
			case 2:	Error(LOG, 0, "CMD_CAP_CAPPING_POS: Step 1 done");
					motors_move_to_step(MOTOR_ROB_BITS, &_ParRob_drive, -_rob_micron_2_steps(SLIDE_CAPPING_POS));	
					break;
				
			case 3:	// Error(LOG, 0, "CMD_CAP_CAPPING_POS: Step 2 done");
					RX_StepperStatus.info.x_in_cap = TRUE;					
					//motor_move_to_step(MOTOR_Z_0, &_ParZ_cap, _z_micron_2_steps(ROBOT_HEIGHT + 0           + 100));
					//motor_move_to_step(MOTOR_Z_1, &_ParZ_cap, _z_micron_2_steps(ROBOT_HEIGHT + ROBOT_ALIGN + 100));			
					motor_move_to_step(MOTOR_Z_0, &_ParZ_cap, _z_micron_2_steps(CAPPING_HEIGHT));		
					motor_move_to_step(MOTOR_Z_1, &_ParZ_cap, _z_micron_2_steps(CAPPING_HEIGHT));		
					motors_start(MOTOR_Z_BITS, FALSE);
					break;
				
			case 4: loc_new_cmd=0;	
					// Error(LOG, 0, "CMD_CAP_CAPPING_POS: done");
					RX_StepperStatus.info.z_in_cap = TRUE;										
					break;
			}			
		}
		
		// --- tasks after fill cap ---
		if (_CmdRunning == CMD_CAP_FILL_CAP)
		{
			_rob_set_pwm(PUMP_0, 0);
			Fpga.par->output &= ~RO_VACUUM; // o2
		}
		
		// --- tasks after empty cap ---
		if (_CmdRunning == CMD_CAP_EMPTY_CAP)
		{
			Fpga.par->output &= ~RO_VACUUM; // o2
		}
		
		//========================================== rob ==========================================
		// --- tasks ater reference cleaning station ---
		if (_CmdRunning == CMD_CLN_REFERENCE)
		{
		//	rx_sleep(1000); // wait 1s on transport to stand still
		//	RX_StepperStatus.info.x_in_ref = fpga_input(RO_STORED_IN_0); // Reference Sensor
			RX_StepperStatus.info.x_in_ref = TRUE;
			if (RX_StepperStatus.info.x_in_ref)
			{
				motors_reset(MOTOR_ROB_BITS);				// reset position after referencing
				RX_StepperStatus.info.ref_done = TRUE;
				_ResetRetried = 0;
				RX_StepperStatus.posX = -_rob_steps_2_micron(motor_get_step(MOTOR_ROB_0));
				loc_move_pos = FALSE;
			}
			else
			{
				loc_new_cmd = _cleaf_retry_ref(CMD_CLN_REFERENCE);
			}
		}
		
		// --- tasks after move ---
		if (_CmdRunning == CMD_CLN_MOVE_POS) 
		{
			// --- check ref sensor ---
			if ((_LastRobPosCmd == 0) && (!RX_StepperStatus.info.x_in_ref))  //  && (RX_StepperStatus.posX == 0)
			{
				loc_new_cmd = CMD_CLN_REFERENCE;
				Error(LOG, 0, "Stepper: Command %s: triggers Referencing", _CmdName); 
			}	
		}
		
		// --- tasks after wipe ---
		if (_CmdRunning == CMD_CLN_WIPE)
		{
			Fpga.par->output &= ~RO_BLADE_UP_0;
		}
				

		//========================================== waste pump ==========================================
		// --- tasks after drain waste ---
		if (_CmdRunning == CMD_CLN_DRAIN_WASTE)  //  && (RX_StepperStatus.posX == 0)
		{
			_rob_waste_pump(0);
			_WastePumpOn = FALSE;
			Fpga.par->output &= ~RO_DRAIN_WASTE; // o10
		}
		
		//==========================================
		
		
		// --- Set Position Flags after Commands ---
		//RX_StepperStatus.info.move_ok = (RX_StepperStatus.info.ref_done && (abs(RX_StepperStatus.posZ - _LastRobPosCmd) <= 10));
		// RX_StepperStatus.info.x_in_ref	= (RX_StepperStatus.info.ref_done && (RX_StepperStatus.info.x_in_ref == 1));
		RX_StepperStatus.info.x_in_ref	= RX_StepperStatus.info.ref_done;
		
		// --- Toggle to mark end of move ---
		if ((loc_new_cmd == FALSE) && (_CmdRunning != CMD_CAP_STOP) && (_CmdRunning != CMD_CLN_SET_WASTE_PUMP)) 
		{
			RX_StepperStatus.info.move_tgl = !RX_StepperStatus.info.move_tgl;
			RX_StepperStatus.info.move_ok = (RX_StepperStatus.info.ref_done
				&& abs(RX_StepperStatus.posX - _LastRobPosCmd) <= 10);
		}

		// --- Reset Commands ---
		_CmdRunning = FALSE;
		_NewCmdPos = 0;
		_NewCmd = FALSE;

		// --- Execute next Commands ---
		switch (loc_new_cmd)
		{
		case CMD_CAP_REFERENCE: 	cleaf_handle_ctrl_msg(INVALID_SOCKET, CMD_CAP_REFERENCE, NULL); break;
		case CMD_CLN_MOVE_POS:		cleaf_handle_ctrl_msg(INVALID_SOCKET, CMD_CLN_MOVE_POS, &loc_move_pos); break;
		case CMD_CLN_REFERENCE:		cleaf_handle_ctrl_msg(INVALID_SOCKET, CMD_CLN_REFERENCE, NULL); break;
		case CMD_CAP_UP_POS:  		_CmdRunning = loc_new_cmd;	break;
		case CMD_CAP_PRINT_POS:		_CmdRunning = loc_new_cmd;	break;
		case CMD_CAP_CAPPING_POS:	_CmdRunning = loc_new_cmd;	break;
		case 0: break;
		default:				Error(ERR_CONT, 0, "LBROB_MAIN: Command 0x%08x not implemented", loc_new_cmd); break;
		}
		loc_new_cmd = FALSE;
		loc_move_pos = 0;
		
	}	
	
	//--- HEAD DOWN ENABLE --------------------
	{
		if (fpga_input(PRINTHEAD_EN)) 	// --- Printhead down enable is ON---
		{
			_PrintHeadEn_Off = 0;
			if (((++_PrintHeadEn_On) == HEAD_DOWN_EN_DELAY) && (RX_StepperStatus.info.printhead_en == FALSE)) // (_PrintHeadEn_On < HEAD_DOWN_EN_DELAY) && 
			{
			//	Error(LOG, 0, "LIFT: printhead_en enabled");
				if (RX_StepperStatus.info.splicing) Error(LOG, 0, "LIFT: printhead_en enabled, splice ended", _CmdRunning);
				RX_StepperStatus.info.printhead_en = TRUE;
				RX_StepperStatus.info.splicing = FALSE;
				_cleaf_send_status(0);
			}
		}
		else // --- Printhead down enable is OFF---
		{
			_PrintHeadEn_On = 0;
			if (((++_PrintHeadEn_Off) == HEAD_DOWN_EN_DELAY) && (RX_StepperStatus.info.printhead_en == TRUE)) // (_PrintHeadEn_Off < HEAD_DOWN_EN_DELAY) && 
			{
			//	Error(LOG, 0, "LIFT: printhead_en disabled", _CmdRunning);
				if (RX_StepperStatus.info.z_in_print)					
				{	
					Error(LOG, 0, "LIFT: printhead_en disabled, go up for splice");
					RX_StepperStatus.info.splicing = TRUE;
					cleaf_handle_ctrl_msg(INVALID_SOCKET, CMD_CAP_UP_POS, NULL);
				}
				RX_StepperStatus.info.printhead_en = FALSE;  // if 0 then splice is comming and head has to go up, if not in capping
				_cleaf_send_status(0);				
			}
		}	
	}
	_cleaf_check_laser();
}

//--- _cleaf_check_laser ------------------------------------------
static void _cleaf_check_laser(void)
{					
	//--- Read LASER value -------------------------------------------------------------------------
	INT32 laser_value = ((LASER_VOLT_OFFSET - (int)VAL_TO_MV_AI(Fpga.stat->analog_in[LASER_IN])) * LASER_MM_PER_VOLT); // (mV -mV)*mm/V = um // medium thickness in um
	_LaserAvr += laser_value;

	if ((++_LaserCnt) == LASER_ANALOG_AVR)
	{
		RX_StepperStatus.posY = _LaserAvr / LASER_ANALOG_AVR;
		_LaserAvr = 0;
		_LaserCnt = 0;
	}
	
	return;
	
	//--- checks -----------------------------------------------------------------------------------
	if(RX_StepperStatus.info.printhead_en)
	{
		if(RX_StepperStatus.info.z_in_print)
		{
			if(RX_StepperStatus.posY >= (RX_StepperCfg.material_thickness - LASER_VARIATION) && RX_StepperStatus.posY <= (RX_StepperCfg.material_thickness + LASER_VARIATION))
			{
				_LaserTimeThin  = 0;
				_LaserTimeThick = 0;				
			}
			else
			{
				if(RX_StepperStatus.posY < RX_StepperCfg.material_thickness - LASER_VARIATION)
				{
					_LaserTimeThick = 0;
					if (!_LaserTimeThin) _LaserTimeThin  = rx_get_ticks();
				}
				if(RX_StepperStatus.posY > RX_StepperCfg.material_thickness + LASER_VARIATION)
				{
					_LaserTimeThin = 0;
					if (!_LaserTimeThick) _LaserTimeThick  = rx_get_ticks();
				}
			}
	
			if (_LaserTimeThick && (rx_get_ticks()-_LaserTimeThick) > LASER_TIMEOUT)
			{
			//	if (ErrorFlag(ERR_ABORT, &_cleaf_Error[0], 0x01, 0, "WEB: Laser detects too thick material. Moving head up. (measured %d, expected %d)", RX_StepperStatus.posY, RX_StepperCfg.material_thickness))
			//		cleaf_handle_ctrl_msg(INVALID_SOCKET, CMD_CAP_UP_POS, NULL);
				Error(WARN, 0, "WEB: Laser detects too thick material. Moving head up. (measured %d, expected %d)", RX_StepperStatus.posY, RX_StepperCfg.material_thickness);
				_LaserTimeThick = 0; // start next check
			}
			
			if (_LaserTimeThin && (rx_get_ticks()-_LaserTimeThin) > LASER_TIMEOUT)
			{
			//	if (ErrorFlag(ERR_ABORT, &_cleaf_Error[0], 0x02, 0, "WEB: Laser detects too thin material. Moving head up. (measured %d, expected %d)", RX_StepperStatus.posY, RX_StepperCfg.material_thickness))
			//		cleaf_handle_ctrl_msg(INVALID_SOCKET, CMD_CAP_UP_POS, NULL);								
				Error(WARN, 0, "WEB: Laser detects too thin material. Moving head up. (measured %d, expected %d)", RX_StepperStatus.posY, RX_StepperCfg.material_thickness);
				_LaserTimeThin = 0;
			}
		}
		else
		{
			_LaserTimeThin  = 0;
			_LaserTimeThick = 0;
		}
	}
	else
	{
		_LaserTimeThin  = 0;
		_LaserTimeThick = 0;
		_cleaf_Error[0] &= ~0x03;
	}
}

//--- value_str3 ---------------------------------------------
static char *_value_str3(int val)
{
	static char str[32];
	if (val==INVALID_VALUE) strcpy(str, "-----");
	else					sprintf(str, "%d.%03d", val/1000, abs(val)%1000);
	return str;
}

//--- cleaf_display_status ---------------------------------------------------------
void cleaf_display_status(void)
{
	term_printf("Cleaf Stepper ---------------------------------\n");
	term_printf("Pos: mm         "); 
		term_printf("%6s    ", _value_str3(_z_steps_2_micron(motor_get_step(MOTOR_Z_0))));
		term_printf("%6s    ", _value_str3(_z_steps_2_micron(motor_get_step(MOTOR_Z_1))));
		term_printf("%6s    ", _value_str3(-_rob_steps_2_micron(motor_get_step(MOTOR_ROB_0))));
		term_printf("\n");
	
	term_printf("moving:         %d		cmd: %08x\n", RX_StepperStatus.info.moving, _CmdRunning);	
	
	if (_ShowStatus == SHOW_LIFT)
	{
		term_printf("Cleaf Stepper LIFT ---------------------------------\n");
		term_printf("moving:         %d		cmd: %08x\n", RX_StepperStatus.info.moving, _CmdRunning);	
		term_printf("refheight:      %06d  ", RX_StepperCfg.ref_height);
		term_printf("pos in um:      %06d  \n", RX_StepperStatus.posZ);
		term_printf("LASER in um:      %06d  row=%06d \n", RX_StepperStatus.posY, Fpga.stat->analog_in[LASER_IN]);	
		//term_printf("Head UP Sensor: %d  %d\n",	fpga_input(HEAD_UP_IN_0), fpga_input(HEAD_UP_IN_1));	
		term_printf("reference done: %d\n", RX_StepperStatus.info.ref_done);
		term_printf("z reference done: %d\n", RX_StepperStatus.info.z_ref_done);
		term_printf("z in reference: %d ", RX_StepperStatus.info.z_in_ref);
		term_printf("z in print:     %d ", RX_StepperStatus.info.z_in_print);
		term_printf("z in capping:   %d\n", RX_StepperStatus.info.z_in_cap);
		term_printf("move tgl bit: %d \n", RX_StepperStatus.info.move_tgl);
		term_printf("allow move down: %d \n", _AllowMoveDown);
		term_printf("drips pans connected: %d \n", _DripPans_Connected);
		term_printf("drips pans valve: %d \n", _DripPansPosition);
		term_printf("drips pans DOWN (Infeed Outfeed): %d %d \n", RX_StepperStatus.info.DripPans_InfeedDOWN, RX_StepperStatus.info.DripPans_OutfeedDOWN);
		term_printf("drips pans UP (Infeed Outfeed): %d %d \n", RX_StepperStatus.info.DripPans_InfeedUP, RX_StepperStatus.info.DripPans_OutfeedUP);
		term_printf("\n");
	}
	
	if (_ShowStatus == SHOW_ROB)
	{
		int enc0_mm = Fpga.encoder[MOTOR_ROB_0].pos * 1000000.0 / ROB_INC_PER_METER;
		term_printf("Cleaf Stepper ROB ---------------------------------\n");
		term_printf("moving:         %d		cmd: %08x\n", RX_StepperStatus.info.moving, _CmdRunning);
		term_printf("Pos0 in steps:   %d  ", motor_get_step(MOTOR_ROB_0));
		term_printf("Pos0 in um:   %d \n", RX_StepperStatus.posX);
		//term_printf("Last pos cmd in um:   %d\n", _LastRobPosCmd);
		//term_printf("Pos0 in steps SOLL:   %d\n", POS_PRINTHEADS_UM);
		term_printf("Enc0 in steps:   %d  ", Fpga.encoder[MOTOR_ROB_0].pos);
		term_printf("Enc0 in um:   %d\n", enc0_mm);
		term_printf("reference done: %d \n", RX_StepperStatus.info.ref_done);
		term_printf("x in reference: %d \n", RX_StepperStatus.info.x_in_ref);
		//term_printf("x in capping:   %d \n", RX_StepperStatus.info.x_in_cap);
		// term_printf("Flag move OK: %d ", RX_StepperStatus.info.move_ok);
		term_printf("move tgl bit: %d \n", RX_StepperStatus.info.move_tgl);	
		term_printf("\n");
	}
	
	if (_ShowStatus == SHOW_PUMP)
	{
		term_printf("Cleaf Stepper PUMP ---------------------------------\n");
		term_printf("moving:         %d		cmd: %08x\n", RX_StepperStatus.info.moving, _CmdRunning);	
		term_printf("Waste Pump on:   %d\n", _WastePumpOn);
		term_printf("Time Empty Cap:   %d\n", _TimeEmptyCap);
		term_printf("Time Fill Cap:   %d\n", _TimeFillCap);
		term_printf("Time Wate Pump:   %d\n", _TimeWastePump);
		term_printf("Time Empty Cap:   %d\n", _TimeEmptyCap);
	}
}

	//--- cleaf_menu --------------------------------------------------
int cleaf_menu(void)
{
	char str[MAX_PATH];
	int synth = FALSE;
	static int cnt = 0;
	int i;
	int pos;

	if (RX_StepperCfg.printerType != printer_cleaf) return TRUE;

	cleaf_display_status();
	
	term_printf("MENU CLEAF ------------------------\n");
	term_printf("s: STOP\n");
	term_printf("r<n>: reset motor<n>\n");	
	term_printf("m<n><steps>: move Motor<n> by <steps>\n");
	term_printf("o: toggle output <no>\n");
	term_printf("l: show lift interface\n");
	term_printf("a: show rob interface\n");
	term_printf("b: show pump interface\n");
	
	if (_ShowStatus == SHOW_LIFT)
	{
		term_printf("MENU LIFT ----------------\n");
		term_printf("R: Reference lift\n");
		term_printf("p: move lift to print\n");
		term_printf("c: move lift to cap\n");
		term_printf("u: move lift to UP position\n");
		term_printf("z: move lift by <steps>\n");
	}
	
	if (_ShowStatus == SHOW_ROB)
	{
		term_printf("MENU ROB ----------------\n");
		term_printf("R: Reference Roboter\n");
		term_printf("p: move cap to <um>\n");
		term_printf("w: wipe to <um>\n");
		term_printf("z<steps>: move cap Motor0/1 by <steps>\n");
	}
	
	if (_ShowStatus == SHOW_PUMP)
	{
		term_printf("MENU PUMP ----------------\n");
		term_printf("Drain Pump\n");
		term_printf("d: drain waste\n");
		term_printf("e<time>: set empty time to <time> in s\n");
		term_printf("f<time>: set fill time to <time> in s\n");
		term_printf("g<time>: set waste time to <time> in s\n");
		term_printf("v<n><x>: analog in <n> set threshold <x>\n");
		term_printf("w<n><x>: pwm out <n> set pulse width <x>\n");
	}
	
	term_printf("x: exit\n");
	term_printf(">");
	term_flush();
	
	if (term_get_str(str, sizeof(str)))
	{
		switch (str[0])
		{
		case 'o': Fpga.par->output ^= (1 << atoi(&str[1])); break;
		case 's': cleaf_handle_ctrl_msg(INVALID_SOCKET, CMD_CAP_STOP, NULL); break;
		case 'r': motor_reset(atoi(&str[1])); break;
		case 'm': _cleaf_motor_test(str[1] - '0', atoi(&str[2])); break;
		case 'l': _ShowStatus = (_ShowStatus == SHOW_LIFT)? 0 : SHOW_LIFT; break;
		case 'a': _ShowStatus = (_ShowStatus == SHOW_ROB) ? 0 : SHOW_ROB;  break;
		case 'b': _ShowStatus = (_ShowStatus == SHOW_PUMP)? 0 : SHOW_PUMP; break;
		}
		
		if (_ShowStatus == SHOW_LIFT)
		{
			switch (str[0])
			{
			case 'R': cleaf_handle_ctrl_msg(INVALID_SOCKET, CMD_CAP_REFERENCE, NULL); break;
			case 'c': cleaf_handle_ctrl_msg(INVALID_SOCKET, CMD_CAP_CAPPING_POS, NULL); break;
			case 'p': cleaf_handle_ctrl_msg(INVALID_SOCKET, CMD_CAP_PRINT_POS, &pos); break;
			case 'u': cleaf_handle_ctrl_msg(INVALID_SOCKET, CMD_CAP_UP_POS, NULL); break;
			case 'z': _cleaf_motor_z_test(atoi(&str[1])); break;
			case 'x': return FALSE;
			}
		}
		if (_ShowStatus == SHOW_ROB)
		{
			switch (str[0])
			{
			case 'R': cleaf_handle_ctrl_msg(INVALID_SOCKET, CMD_CLN_REFERENCE, NULL); break;
			case 'p': pos = atoi(&str[1]); cleaf_handle_ctrl_msg(INVALID_SOCKET, CMD_CLN_MOVE_POS, &pos); break;
			case 'n': pos = atoi(&str[1]); cleaf_handle_ctrl_msg(INVALID_SOCKET, CMD_CLN_WIPE, &pos); break;
			case 'z': _cleaf_motor_x_test(atoi(&str[1])); break;
			}
		}
		if (_ShowStatus == SHOW_PUMP)
		{
			switch (str[0])
			{
			case 'd' : cleaf_handle_ctrl_msg(INVALID_SOCKET, CMD_CLN_DRAIN_WASTE, NULL); break;
			case 'e' : _TimeEmptyCap = atoi(&str[1]); break;
			case 'f' : _TimeFillCap = atoi(&str[1]); break;
			case 'g' : _TimeWastePump = atoi(&str[1]); break;
			case 'v': _rob_set_threshold(str[1] - '0', atoi(&str[2])); break;
			case 'w': _rob_set_pwm(str[1] - '0', atoi(&str[2])); break;	
			}
		}
		switch (str[0])
		{
		case 'x': return FALSE;
		}
	}
	return TRUE;
}
	
//---_z_micron_2_steps --------------------------------------------------------------
static int  _z_micron_2_steps(int micron)
{
	return (int)(0.5 + STEPS_REV / DIST_Z_REV*micron);
}

//---_z_steps_2_micron --------------------------------------------------------------
static int  _z_steps_2_micron(int steps)
{
	return (int)(steps * DIST_Z_REV / STEPS_REV + 0.5);
}
	
//---_rob_micron_2_steps --------------------------------------------------------------
static int  _rob_micron_2_steps(int micron)
{
	return (int)(0.5 + micron * (ROB_STEPS_PER_METER / 1000000.0));
}

//---_rob_steps_2_micron --------------------------------------------------------------
static int  _rob_steps_2_micron(int steps)
{
	return (int)(steps * (1000000.0 / ROB_STEPS_PER_METER) + 0.5);
}

//--- _lift_do_reference ----------------------------------------------------------------
static void _lift_do_reference(void)
{
	int i;
	//motors_stop	(MOTOR_Z_BITS);
	_CmdRunning  = CMD_CAP_REFERENCE;
	RX_StepperStatus.info.moving = TRUE;
	_cleaf_motors_by_step(MOTOR_Z_BITS, -500000, _Par_Z_Ref);
}
	
//--- cleaf_handle_ctrl_msg -----------------------------------
int  cleaf_handle_ctrl_msg(RX_SOCKET socket, int msgId, void *pdata)
{	
	int i;
	int pos, steps;
	
	switch (msgId)
	{
	case CMD_TT_STATUS:				sok_send_2(&socket, REP_TT_STATUS, sizeof(RX_StepperStatus), &RX_StepperStatus);	
		break;
		
		// ============================================================== Hub ==============================================================

	case CMD_CAP_STOP:				strcpy(_CmdName, "CMD_CAP_STOP");
		motors_stop(MOTOR_ALL_BITS); // Stops Hub and Robot
		Fpga.par->output &= ~RO_ALL_OUTPUTS; // set all output to off
		RX_StepperStatus.info.moving = TRUE;
		_CmdRunning = msgId;
		// reset var
		_NewCmdPos = 0;
		_NewCmd = FALSE;
		_TimeCnt = 0;
		_rob_set_pwm(PUMP_0, 0);
		_rob_waste_pump(0);
		_WastePumpOn = FALSE;
		break;

	case CMD_CAP_REFERENCE:			strcpy(_CmdName, "CMD_CAP_REFERENCE");
	//	Error(LOG, 0, "got CMD_CAP_REFERENCE");
		if (_CmdRunning){ cleaf_handle_ctrl_msg(INVALID_SOCKET, CMD_CAP_STOP, NULL); _NewCmd = CMD_CAP_REFERENCE; break; }
		motors_reset(MOTOR_ALL_BITS); // to recover from move count missalignment
		motor_reset(MOTOR_WASTE_PUMP); // to recover from move count missalignment
		RX_StepperStatus.set_io_cnt = 0;
		RX_StepperStatus.info.ref_done = FALSE;
		RX_StepperStatus.info.z_ref_done = FALSE;
		RX_StepperStatus.info.z_in_ref	= FALSE;
		RX_StepperStatus.info.z_in_print	= FALSE;
		RX_StepperStatus.info.z_in_cap	= FALSE;
		_rob_set_pwm(PUMP_0, 0);
		Fpga.par->output &= ~RO_ALL_OUTPUTS;
		_lift_do_reference();	
		break;

	case CMD_CAP_PRINT_POS:			strcpy(_CmdName, "CMD_CAP_PRINT_POS");
		pos = (*((INT32*)pdata));
		if (pos<PRINT_HEIGHT_MIN) 
		{
			pos = PRINT_HEIGHT_MIN;				
			Error(WARN, 0, "PrintHeight set to minimum (%d.%03 mm)", PRINT_HEIGHT_MIN/1000, PRINT_HEIGHT_MIN%1000);
		}
		_PrintPos   = RX_StepperCfg.material_thickness + pos;
//		Error(LOG, 0, "got CMD_CAP_PRINT_POS %d, _CmdRunning=0x%08x ", _PrintPos, _CmdRunning);
		RX_StepperStatus.info.z_in_ref	= FALSE;
		RX_StepperStatus.info.z_in_print	= FALSE;
		RX_StepperStatus.info.z_in_cap	= FALSE;
		if(!_CmdRunning && _AllowMoveDown)
		{
			if(RX_StepperStatus.info.ref_done)
			{
				if(RX_StepperStatus.info.printhead_en) 
				{
				//	if(RX_StepperStatus.posY >= (RX_StepperCfg.material_thickness - LASER_VARIATION) && RX_StepperStatus.posY <= (RX_StepperCfg.material_thickness + LASER_VARIATION))
					{
						_CmdRunning = CMD_CAP_PRINT_POS;
						_Step = 0;															
					}
				//	else Error(ERR_CONT, 0, "WEB: Laser detects material out of range. (measured %d, expected %d)", RX_StepperStatus.posY, RX_StepperCfg.material_thickness);				
				}
				else Error(ERR_CONT, 0, "Stepper: Command 0x%08x: printhead_en signal not set", _CmdRunning);
			}
			else Error(ERR_CONT, 0, "Reference not done");
		}				
		break;

	case CMD_CAP_ROB_Z_POS:			strcpy(_CmdName, "CMD_CAP_ROB_Z_POS");
		pos   = (*((INT32*)pdata));
		Error(LOG, 0, "got CMD_CAP_ROB_Z_POS %d", pos);		
		if (!_CmdRunning && RX_StepperStatus.info.z_ref_done)
		{
			RX_StepperStatus.info.moving = TRUE;
			motor_move_to_step(MOTOR_Z_0, &_ParZ_down, _z_micron_2_steps(ROBOT_HEIGHT - pos));
			motor_move_to_step(MOTOR_Z_1, &_ParZ_down, _z_micron_2_steps(ROBOT_HEIGHT - pos + ROBOT_ALIGN));
			motors_start(MOTOR_Z_BITS, TRUE);
		}
		break;
		
	case CMD_CAP_CAPPING_POS:		strcpy(_CmdName, "CMD_CAP_CAPPING_POS");
		RX_StepperStatus.info.z_in_ref	= FALSE;
		RX_StepperStatus.info.z_in_print	= FALSE;
		RX_StepperStatus.info.z_in_cap	= FALSE;		
		if (!_CmdRunning && RX_StepperStatus.info.z_ref_done)
		{
			_CmdRunning = msgId;
			_Step = 0;
		}
		break;

	case CMD_CAP_UP_POS:			
		strcpy(_CmdName, "CMD_CAP_UP_POS");
//		Error(LOG, 0, "got CMD_CAP_UP_POS _CmdRunning=0x%08x", _CmdRunning);
		RX_StepperStatus.info.z_in_ref	= FALSE;
		RX_StepperStatus.info.z_in_print	= FALSE;
		RX_StepperStatus.info.z_in_cap	= FALSE;		
	//	if (!_CmdRunning && RX_StepperStatus.info.z_ref_done)
		if (!_CmdRunning)
		{
			_CmdRunning = msgId;
			_Step = 0;
		}
		break;
		
	case CMD_CLN_DRIP_PANS:
		_DripPans_Connected = TRUE;
		if (RX_StepperStatus.info.z_in_ref) 
		{			
			if (!_DripPansPosition)
			{
				// all stepper motors in reference
				Fpga.par->output &= ~DRIP_PANS_VALVE_ALL;
				Fpga.par->output |= DRIP_PANS_VALVE_UP;	
				_DripPansPosition = 1;
			}
			else
			{
				// all stepper motors in reference
				Fpga.par->output &= ~DRIP_PANS_VALVE_ALL;
				Fpga.par->output |= DRIP_PANS_VALVE_DOWN;	
				_DripPansPosition = 0;
			}
		}
		break;
		
	case CMD_CLN_DRIP_PANS_EN:
		_DripPans_Connected = TRUE;
		if (fpga_input(DRIP_PANS_INFEED_UP) && fpga_input(DRIP_PANS_OUTFEED_UP)) _DripPansPosition = 1;
		else _DripPansPosition = 0;
		
		break;
		
	case CMD_CAP_ALLOW_MOVE_DOWN:
		_AllowMoveDown = 1;
		break;
		
	case CMD_CAP_NOT_ALLOW_MOVE_DOWN:
		_AllowMoveDown = 0;
		break;
				
		// ============================================================== Robot ==============================================================
		
	case CMD_CLN_REFERENCE:			strcpy(_CmdName, "CMD_CLN_REFERENCE");
		if (_CmdRunning){ cleaf_handle_ctrl_msg(INVALID_SOCKET, CMD_CAP_STOP, NULL); _NewCmd = CMD_CLN_REFERENCE; break; }
		motor_reset(MOTOR_ROB_0); // to recover from move count missalignment
		RX_StepperStatus.info.ref_done = FALSE;
		_CmdRunning = msgId;
		if ((Fpga.par->output & RO_ALL_OUT_MECH))
		{
			Fpga.par->output &= ~RO_ALL_OUT_MECH; // set all output to off
			rx_sleep(1000); // wait ms
		}
		if (RX_StepperStatus.info.z_ref_done != TRUE) Error(ERR_CONT, 0, "CLN: Command %s: canceled reference ROBOT, reference HUB first", _CmdName);
		if (RX_StepperStatus.info.z_ref_done != TRUE) break;
		RX_StepperStatus.set_io_cnt = 0;
		_WastePumpOn = FALSE;
		_rob_waste_pump(0);
		RX_StepperStatus.info.moving = TRUE;
		Fpga.par->output &= ~RO_DRAIN_WASTE; // 11 und 10 // 0 enables drain chain
		_LastRobPosCmd = 0;
		_cleaf_motors_by_step(MOTOR_ROB_BITS, 1000000, _ParRob_ref);
		break;
		
	case CMD_CLN_MOVE_POS:		strcpy(_CmdName, "CMD_CLN_MOVE_POS");
		if (!_CmdRunning)
		{
			_CmdRunning = msgId;
			if ((Fpga.par->output & RO_ALL_OUT_MECH))
			{
				Fpga.par->output &= ~RO_ALL_OUT_MECH; // set all output to off
				rx_sleep(1000); // wait ms
			}
			if (RX_StepperStatus.info.z_ref_done != TRUE) break;
			RX_StepperStatus.info.moving = TRUE;
			pos = *((INT32*)pdata);
			_LastRobPosCmd = pos;
			if (!RX_StepperStatus.info.ref_done) Error(LOG, 0, "CLN: Command %s: missing ref_done: triggers Referencing", _CmdName);
			if (pos < 0) {Error(LOG, 0, "CLN: Command %s: negative position not allowed", _CmdName); break;}
			if (RX_StepperStatus.info.ref_done) motors_move_to_step(MOTOR_ROB_BITS, &_ParRob_drive, -_rob_micron_2_steps(pos));
		}
		break;
		
	case CMD_CLN_MOVE_POS_REL:		//strcpy(_CmdName, "CMD_CLN_MOVE_POS_REL");		
		pos = *((INT32*)pdata);
		if (RX_StepperStatus.info.ref_done) motors_move_to_step(MOTOR_ROB_BITS, &_ParRob_drive, -_rob_micron_2_steps(pos));
		break;
						
	case CMD_CAP_SET_PUMP:			//strcpy(_CmdName, "CMD_SET_PUMP");
		pos = *((INT32*)pdata);
		if (pos == 0)
		{
			//Fpga.par->output &= ~LB_PUMP_FLUSH;
			_rob_set_pwm(PUMP_0, 0);
			//rx_sleep(1000);
			//Fpga.par->output &= ~LB_FLUSH_CAR_0;
			//Fpga.par->output &= ~LB_FLUSH_CAR_1;
		}
		if (pos == 1)
		{
			//Fpga.par->output |= LB_FLUSH_CAR_0;
			//Fpga.par->output |= LB_FLUSH_CAR_1;
			//rx_sleep(500);
			//Fpga.par->output |= LB_PUMP_FLUSH;
			_rob_set_pwm(PUMP_0, PUMP_0_WET_WIPE);
		}
		RX_StepperStatus.set_io_cnt++;
		//Error(LOG, 0, "Stepper: Command 0x%08x: CMD_CAP_SET_PUMP cnt set_io", _CmdRunning);
		break;
		
		// ============================================================== Vacuum ==============================================================
		
	case CMD_CAP_SET_VAC:			//strcpy(_CmdName, "CMD_CAP_SET_VAC");
		pos = *((INT32*)pdata);
		if (pos == 0)
		{
			Fpga.par->output &= ~RO_VACUUM;
			//_rob_set_pwm(PUMP_1, 0);
		}
		if (pos == 1)
		{
			Fpga.par->output |= RO_VACUUM;
			//_rob_set_pwm(PUMP_1, PUMP_1_WASTE);
		}
		RX_StepperStatus.set_io_cnt++;
		//Error(LOG, 0, "Stepper: Command 0x%08x: CMD_CAP_SET_VAC cnt set_io", _CmdRunning);
		break;
		
		// ============================================================== Waste Pump ==============================================================
		
	case CMD_CLN_DRAIN_WASTE:		strcpy(_CmdName, "CMD_CLN_DRAIN_WASTE");
		if (!_CmdRunning)
		{
			_CmdRunning = msgId;
			RX_StepperStatus.info.moving = TRUE;
			Fpga.par->output |= RO_DRAIN_WASTE; // o10
			_TimeCnt = _TimeWastePump * MAIN_PER_SEC;
			_rob_waste_pump(WASTE_PUMP_STEPS);
			_WastePumpOn = TRUE;
		}
		break;
		
	case CMD_CLN_SET_WASTE_PUMP:			//strcpy(_CmdName, "CMD_SET_PUMP");
		pos = *((INT32*)pdata);
		if (pos == 0)
		{
			_WastePumpOn = FALSE;
			_rob_waste_pump(0);
		}
		if (pos == 1)
		{
			_rob_waste_pump(WASTE_PUMP_STEPS);
			_WastePumpOn = TRUE;
		}
		RX_StepperStatus.set_io_cnt++;
		//Error(LOG, 0, "Stepper: Command 0x%08x: CMD_CLN_SET_WASTE_PUMP cnt set_io", _CmdRunning);
		break;
		
		// ============================================================== Reset ==============================================================

	case CMD_ERROR_RESET:		//strcpy(_CmdName, "CMD_ERROR_RESET");		
		_cleaf_error_reset();
		fpga_stepper_error_reset();
		break;
		
	case CMD_CAP_VENT:	break;
		
	default:	Error(WARN, 0, "LIFT: Command 0x%08x not implemented", msgId); break;
	}
	
	return TRUE;
}

//--- _cleaf_retry_ref ----------------------------------------------------------------------
int _cleaf_retry_ref(int cmd)
{
	if (FALSE && _ResetRetried < 3) // Retry Reference 3 times
	{
		_ResetRetried++;
		Error(LOG, 0, "Stepper: Command %s: Retry Referencing", _CmdName);
		return cmd;
	}
	else
	{
		RX_StepperStatus.info.ref_done = FALSE;
		//RX_StepperStatus.info.z_ref_done = FALSE;
		Error(ERR_CONT, 0, "Stepper: Command %s: Referencing failed", _CmdName);
		_ResetRetried = 0;
		return FALSE;
	}
}

//--- cleaf_error_reset ------------------------------------
static void	_cleaf_error_reset(void)
{
	memset(_cleaf_Error, 0, sizeof(_cleaf_Error));
	// use: ErrorFlag (ERR_CONT, (UINT32*)&_Error[isNo],  1,  0, "InkSupply[%d] Ink Tank Sensor Error", isNo+1);
}

//--- _cleaf_send_status --------------------------------------------------
static void _cleaf_send_status(RX_SOCKET socket)
{
	static RX_SOCKET _socket = INVALID_SOCKET;
	if (socket != 0 && socket != INVALID_SOCKET) _socket = socket;
	if (_socket != INVALID_SOCKET) sok_send_2(&_socket, REP_TT_STATUS, sizeof(RX_StepperStatus), &RX_StepperStatus);	
}
	

//--- _cleaf_motor_test ---------------------------------
static void _cleaf_motor_test(int motorNo, int steps)
{
	int motor = 1 << motorNo;
	SMovePar par;
	int i;
	int mot_steps = steps;

	if (motorNo == MOTOR_ROB_0) 
	{
		par.speed	= 21000; // 8000; // speed with max tork: 21'333 //// 16000; // 32000;//25000; // 8000;// 4000; // 2000;
		par.accel	= 10000; // 2000; // 8000; //16000;//12500;// 4000;//2000; // 1000;
		par.current = 60.0; // max 67 = 0.67 A //// 60.0;  // for Tests: 50
		mot_steps	= -steps;
	}
	else if ((motorNo == MOTOR_Z_0) || (motorNo == MOTOR_Z_1)) {
		par.speed	= 5000;
		par.accel	= 2500;
		par.current	= 320.0;
		mot_steps	= -steps;
	}
		
	else if (motorNo == MOTOR_WASTE_PUMP) {
		par.speed	= 20000; // 375 U / min // speed with max tork: 2k pulses per sec * 16 // 500u/min * 200 * 16 /60 = 26000
		par.accel	= 10000;
		par.current	= 80.0;
		mot_steps	= -steps;
	}
	else
	{
		par.speed	= 0;					
		par.accel	= 0;			
		par.current = 0.0;
	}
	
	par.stop_mux		= 0;
	par.dis_mux_in		= 0;
	par.estop_in		= ESTOP_UNUSED;
	par.estop_level		= 0;
	par.checkEncoder	= FALSE;
	par.stop_in			= ESTOP_UNUSED;
	par.stop_level		= 0;
	par.dis_mux_in		= FALSE;
	RX_StepperStatus.info.moving = TRUE;
	_CmdRunning = 1; // TEST

	motors_move_by_step(motor, &par, mot_steps, FALSE);
}

//--- _cleaf_motor_x_test ----------------------------------------------------------------------
void _cleaf_motor_x_test(int steps)
{
	SMovePar par;

	par.speed		= 21000;
	par.accel		= 10000;
	par.current		= 60.0; // 60.0; // max 80.0 // 350.0; // 300.0; // for Tests:30
	par.stop_mux	= MOTOR_ROB_BITS;
	par.dis_mux_in  = 0;
	par.estop_in    = ESTOP_UNUSED;
	par.estop_level = 0;
	par.checkEncoder= FALSE;
	par.stop_in		= ESTOP_UNUSED;
	par.stop_level	= 0;
	par.dis_mux_in	= FALSE;
	RX_StepperStatus.info.moving = TRUE;
	_CmdRunning = 1; // TEST
	
	int i;
	motors_move_by_step(MOTOR_ROB_BITS, &par, -steps, FALSE);	
}

//--- _cleaf_motor_z_test ----------------------------------------------------------------------
void _cleaf_motor_z_test(int steps)
{	
	int i;
	
	_CmdRunning = 1; // TEST
	RX_StepperStatus.info.moving = TRUE;
	motors_move_by_step(MOTOR_Z_BITS, &_ParZ_down, -steps, TRUE);
}

//--- _rob_waste_pump ----------------------------------------------------------------------
void _rob_waste_pump(int steps)
{
	if (steps == 0)
	{
		motors_stop(MOTOR_WASTE_PUMP_BITS);
		motor_set_hold_current(MOTOR_WASTE_PUMP);
	}
	if ((Fpga.stat->moving & (0x01 << 4)) == 0)
	{
		if (steps != 0)
		{
			int i;
			motors_move_by_step(MOTOR_WASTE_PUMP_BITS, &_ParPump_waste, -steps, FALSE);
		}
	}
}

//--- _cleaf_motors_by_step ---------------------------------
static void _cleaf_motors_by_step(int motor_bits, int steps, SMovePar par[2])
{
	// wieso nicht motors_move_by_step ?
	int i = 0;
	int i_par = 0;
	for (i = 0; i < MOTOR_CNT; i++)
	{
		if (motor_bits & (1 << i)) 
		{
			motor_move_by_step(i, &par[i_par], steps);	
			i_par++;
		}
	}
	motors_start(motor_bits, TRUE);
}

//--- _rob_set_pwm ---------------------------------
static void _rob_set_pwm(int pwm_nr, int pulse_width)
{
	Fpga.par->pwm_output[pwm_nr]    = pulse_width;
}

//--- _rob_set_threshold ---------------------------------
static void _rob_set_threshold(int analog_in_nr, int threshold)
{
	Fpga.par->adc_thresh[analog_in_nr]    = threshold;
}
