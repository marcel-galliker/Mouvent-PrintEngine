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

#define MOTOR_Z_CNT		4
#define MOTOR_Z_BITS	0x0f
#define CURRENT_HOLD	30 // 30 // 50 // for Tests:15

#define PRINTHEAD_EN		0
#define HEAD_UP_IN_0		1
#define HEAD_UP_IN_1		2
#define HEAD_UP_IN_2		3
#define HEAD_UP_IN_3		4
#define CLN_CAPPING_IN		5

static int	HEAD_UP_IN[4] = {
	HEAD_UP_IN_0,			
	HEAD_UP_IN_1,			
	HEAD_UP_IN_2,
	HEAD_UP_IN_3
};

#define MOT_HEAD_0		0
#define MOT_HEAD_1		1
#define MOT_HEAD_2		2
#define MOT_HEAD_3		3

#define CLEAF_STEPS_PER_METER	1600000.0 // 200 steps/rev * 16 // 16000*0.2=3200 steps/rev // 2mm hub pro revolution // 3200/0.002=1600000 steps/m
#define CLEAF_INC_PER_METER		8000000.0 // 16000 steps/rev // 16000/0.002=8000000 steps/m
// #define WEB_ENC_RATIO			0.2

#define MOT_0_SENS2MECH_UM		1900 // Position delta in steps, positive means downwards. at least one motor must move more than 15um, all in the same direction, all should move down
#define MOT_1_SENS2MECH_UM		 400
#define MOT_2_SENS2MECH_UM		1100
#define MOT_3_SENS2MECH_UM		1800

#define POS_PRINTING_UM		RX_StepperCfg.print_height // (RX_StepperCfg.ref_height-RX_StepperCfg.print_height)
#define POS_START_CLEAN_UM	RX_StepperCfg.wipe_height
#define POS_CAPPING_UM		(RX_StepperCfg.ref_height-RX_StepperCfg.cap_height)
#define POS_REF_UM			RX_StepperCfg.ref_height

#define POS_STORED				POS_REF_UM // 0 // Ref
// #define LIFT_SAFE_POS			POS_PRINTING_UM+25000 // save position for drive to if in printing position
#define LIFT_SAFE_POS			POS_STORED // save position for drive to if in printing position

#define LASER_EN				TRUE
#define LASER_VOLT_PER_MM		0.25 // V/mm
#define LASER_MM_PER_VOLT		4    // mm/V
#define LASER_VOLT_OFFSET 		5900 // Laser Value without medium in mV
#define LASER_VARIATION			400 // Variation +- in um
//#define MAX_POS_UM		 		90000 //Resolution 20um, Working range 45...85 mm, temperature drift 18 um/K, Value range 0...10V linear rising, 1V=4mm ?

#define VAL_TO_MV(x) ((x*5000)/0x0fff)

static SMovePar	_ParRef[MOTOR_Z_CNT];
static SMovePar	_ParRefz[MOTOR_Z_CNT];
static SMovePar	_ParZ_up;
static SMovePar	_ParZ_adj;
static SMovePar	_ParZ_down;
static int		_CmdRunning=0;
static char		_CmdName[32];
static int		_lastPosCmd = 0;
static int		_enable_on  = 0;
static int		_enable_off = 0;
//static int		_laser_err_cnt = 0;
static int		_laser_high_cnt = 0;
static int		_laser_low_cnt = 0;
static int		_laser_high_value = 0;
static int		_laser_low_value = 0;
static int		_reset_retried = FALSE;
static int		_new_cmd = FALSE;
static UINT32	_cleaf_Error[5];

//--- prototypes --------------------------------------------
static void _cleaf_motor_z_test(int steps);
static void _cleaf_motor_test  (int motor, int steps);
static void _cleaf_error_reset(void);
static void _cleaf_send_status(RX_SOCKET);
static int  _micron_2_steps(int micron);
static int  _steps_2_micron(int steps);

//--- cleaf_init --------------------------------------
void cleaf_init(void)
{
	int i;
	
	memset(_CmdName, 0, sizeof(_CmdName));
	motors_config(MOTOR_Z_BITS, CURRENT_HOLD, CLEAF_STEPS_PER_METER, CLEAF_INC_PER_METER);
	
	for (i=0; i<MOTOR_Z_CNT; i++) 
	{		
		_ParRef[i].speed		= 2000;
		_ParRef[i].accel		= 1000;
		_ParRef[i].current		= 420.0;// 600.0; // for Tests:30
		_ParRef[i].stop_mux		= 0;
		_ParRef[i].estop_in		= HEAD_UP_IN[i];	// inputs 1..4
		_ParRef[i].estop_level	= 1;
		_ParRef[i].checkEncoder	= TRUE;
	}
	for (i = 0; i < MOTOR_Z_CNT; i++) 
	{		
		_ParRefz[i].speed			= 2000;
		_ParRefz[i].accel			= 1000;
		_ParRefz[i].current			= 420.0;// 600.0; // for Tests:30
		_ParRefz[i].stop_mux		= MOTOR_Z_BITS;
		_ParRefz[i].estop_in		= HEAD_UP_IN[i];	// inputs 1..4
		_ParRefz[i].estop_level		= 1;
		_ParRefz[i].checkEncoder	= TRUE;
	}
	
	//--- movment parameters ----------------
	
	_ParZ_down.speed		= 40000; // 40'000 -> 60000 -> 160000 should be possible? -> 60'000 zu schnell
	_ParZ_down.accel		= 60000; // 30'000 // 60'000/3200*0.002=0.0375 m/s/s       // 0.015625 m/s/s
	_ParZ_down.current		= 420.0; // for Tests:30
	_ParZ_down.stop_mux		= MOTOR_Z_BITS;
	_ParZ_down.estop_in     = ESTOP_UNUSED;
	_ParZ_down.estop_level  = 0;
	_ParZ_down.checkEncoder = TRUE;
	
	_ParZ_up.speed			= 40000; //  -> 60'000 zu schnell
	_ParZ_up.accel			= 60000; // 25000 --> 100'000 ist zu viel // 75'000 ist zu viel //
	_ParZ_up.current		= 420.0;// 600.0; // for Tests:30
	_ParZ_up.stop_mux		= MOTOR_Z_BITS;
	_ParZ_up.estop_in		= ESTOP_UNUSED; // Check Input 1
	_ParZ_up.estop_level	= 1;
	_ParZ_up.checkEncoder	= TRUE;
	
	_ParZ_adj.speed			= 1000; // 40000 * 3.2 ??? 128'000
	_ParZ_adj.accel			= 1000; // 25000 * 64 ???  1'600'000
	_ParZ_adj.current		= 420.0;// 600.0; // for Tests:30
	_ParZ_adj.stop_mux		= 0;
	_ParZ_adj.estop_in		= ESTOP_UNUSED;
	_ParZ_adj.estop_level	= 1;
	_ParZ_adj.checkEncoder	= TRUE;
}

//--- cleaf_main ------------------------------------------------------------------
void cleaf_main(int ticks, int menu)
{
	int motor;
	
	if (RX_StepperCfg.printerType != printer_cleaf) return; // printer_cleaf
	motor_main(ticks, menu);
	
	// --- read Inputs ---
	RX_TestTableStatus.info.headUpInput_0 = fpga_input(HEAD_UP_IN_0);
	RX_TestTableStatus.info.headUpInput_1 = fpga_input(HEAD_UP_IN_1);
	RX_TestTableStatus.info.headUpInput_2 = fpga_input(HEAD_UP_IN_2);
	RX_TestTableStatus.info.headUpInput_3 = fpga_input(HEAD_UP_IN_3);
	RX_TestTableStatus.info.cln_in_capping = fpga_input(CLN_CAPPING_IN); // Sensor for cleaning station beeing in capping pos
	RX_TestTableStatus.posZ = POS_REF_UM+_steps_2_micron(motor_get_step(0));
	
	// --- Set Position Flags ---
	RX_TestTableStatus.info.x_in_cap    =  TRUE;
	RX_TestTableStatus.info.z_in_cap    = (RX_TestTableStatus.info.ref_done && (abs(RX_TestTableStatus.posZ - POS_CAPPING_UM) <= 10));
	RX_TestTableStatus.info.z_in_ref    = (RX_TestTableStatus.info.ref_done
												&& (abs(RX_TestTableStatus.posZ - POS_STORED) <= 10) 
												&& ((RX_TestTableStatus.info.headUpInput_0 == 1)
												|| (RX_TestTableStatus.info.headUpInput_1 == 1)
												|| (RX_TestTableStatus.info.headUpInput_2 == 1)
												|| (RX_TestTableStatus.info.headUpInput_3 == 1)));
	RX_TestTableStatus.info.z_in_print  = (RX_TestTableStatus.info.ref_done && (abs(RX_TestTableStatus.posZ - POS_PRINTING_UM) <= 10));
	
	// --- set positions False while moving ---
	if (RX_TestTableStatus.info.moving)
	{
		RX_TestTableStatus.info.z_in_ref	= FALSE;
		RX_TestTableStatus.info.z_in_print	= FALSE;
		RX_TestTableStatus.info.z_in_cap	= FALSE;	
		RX_TestTableStatus.info.move_ok		= FALSE;
		RX_TestTableStatus.info.head_in_safety = FALSE;
	}
	
	// --- Executed after each move ---
	if (_CmdRunning && motors_move_done(MOTOR_Z_BITS))
	{
		RX_TestTableStatus.info.moving = FALSE;
		
		// --- tasks ater the deceleration period of the Stopp Command ---
		if (_CmdRunning == CMD_CAP_STOP) 
		{
			if (motor_error(MOTOR_Z_BITS))
			{
				RX_TestTableStatus.info.ref_done = FALSE;
				Error(ERR_CONT, 0, "LIFT: Command %s: triggers motor_error", _CmdName);
			}
		}
		
		// --- End of Reference ---
		if (_CmdRunning == CMD_CAP_REFERENCE_2) 
		{
			rx_sleep(500);
			RX_TestTableStatus.info.headUpInput_0 = fpga_input(HEAD_UP_IN_0);
			RX_TestTableStatus.info.headUpInput_1 = fpga_input(HEAD_UP_IN_1);
			RX_TestTableStatus.info.headUpInput_2 = fpga_input(HEAD_UP_IN_2);
			RX_TestTableStatus.info.headUpInput_3 = fpga_input(HEAD_UP_IN_3);
			if (motors_error(MOTOR_Z_BITS, &motor)) // encoder error
			{
				RX_TestTableStatus.info.ref_done = FALSE;
				Error(ERR_CONT, 0, "LIFT: Command %s: Motor[%d] blocked", _CmdName, motor+1);
			}
			else
			{
				motors_reset(MOTOR_Z_BITS);				// reset position after referencing
				RX_TestTableStatus.posZ = POS_REF_UM + _steps_2_micron(motor_get_step(0));
				if ((RX_TestTableStatus.info.headUpInput_0 == 1) 
					&& (RX_TestTableStatus.info.headUpInput_1 == 1) 
					&& (RX_TestTableStatus.info.headUpInput_2 == 1) 
					&& (RX_TestTableStatus.info.headUpInput_3 == 1))
				{
					RX_TestTableStatus.info.ref_done = TRUE;
					_reset_retried = FALSE;
				}
				else
				{
					// Retry Reference once
					if (_reset_retried < 3)
					{
						_reset_retried++;
						_new_cmd = CMD_CAP_REFERENCE_2;
						Error(LOG, 0, "LIFT: Command %s: Retry Reference LIFT", _CmdName);
					}
					else
					{
						Error(ERR_CONT, 0, "LIFT: Command %s: Referencing LIFT failed", _CmdName);
						_reset_retried = 0;
					}
				}
			}
		}	
		
		// --- Next Reference Phase ---
		if (_CmdRunning == CMD_CAP_REFERENCE) 
		{
			rx_sleep(500);
			RX_TestTableStatus.info.headUpInput_0 = fpga_input(HEAD_UP_IN_0);
			RX_TestTableStatus.info.headUpInput_1 = fpga_input(HEAD_UP_IN_1);
			RX_TestTableStatus.info.headUpInput_2 = fpga_input(HEAD_UP_IN_2);
			RX_TestTableStatus.info.headUpInput_3 = fpga_input(HEAD_UP_IN_3);
			if (motors_error(MOTOR_Z_BITS, &motor)) // encoder error
			{
				RX_TestTableStatus.info.ref_done = FALSE;
				Error(ERR_CONT, 0, "LIFT: Command %s: Motor[%d] blocked", _CmdName, motor+1);
			}
			else
			{
				// ------------ ------------ ------------
				if ((RX_TestTableStatus.info.headUpInput_0 == 1)
					|| (RX_TestTableStatus.info.headUpInput_1 == 1)
					|| (RX_TestTableStatus.info.headUpInput_2 == 1)
					|| (RX_TestTableStatus.info.headUpInput_3 == 1))
				{
				_new_cmd = CMD_CAP_REFERENCE_2;
					_reset_retried = FALSE;
				}
				else
				{
					// Retry Reference
					if (_reset_retried < 3)
					{
						_reset_retried++;
						_new_cmd = CMD_CAP_REFERENCE;
						Error(LOG, 0, "WEB: Command 0x%08x: Retry Global Reference Lift", _CmdRunning);
					}
					else
					{
						Error(ERR_CONT, 0, "WEB: Command 0x%08x: Global Referencing Lift failed: Lift Motor blocked", _CmdRunning);
						_reset_retried = 0;
					}
				}
				// ------------ ------------ ------------
			}
		}
		
		// --- Reset all motors after global ref ---
		if (_CmdRunning == CMD_CAP_GLOBAL_REF) 
		{
			if (motors_error(MOTOR_Z_BITS, &motor)) // encoder error
			{
				RX_TestTableStatus.info.ref_done = FALSE;
				Error(ERR_CONT, 0, "LIFT: Command %s: Motor[%d] blocked", _CmdName, motor+1);
			}
			else
			{
				motors_reset(MOTOR_Z_BITS);				// reset position after referencing
				RX_TestTableStatus.posZ = POS_REF_UM + _steps_2_micron(motor_get_step(0));
				// Test Sensors
			}
		}
		
		// --- tasks ater move to pos 0 stored position, check ref sensor ---
		if (((_CmdRunning == CMD_CAP_PRINT_POS) || (_CmdRunning == CMD_CAP_UP_POS)) && (_lastPosCmd == POS_STORED)) 
		{
			if (motors_error(MOTOR_Z_BITS, &motor)) // encoder error
			{
				RX_TestTableStatus.info.ref_done = FALSE;	
				Error(ERR_CONT, 0, "LIFT: Command %s: Motor[%d] blocked", _CmdName, motor+1);						
			}
			else
			{
				if ((RX_TestTableStatus.info.headUpInput_0 == 0) 
					&& (RX_TestTableStatus.info.headUpInput_1 == 0) 
					&& (RX_TestTableStatus.info.headUpInput_2 == 0) 
					&& (RX_TestTableStatus.info.headUpInput_3 == 0))
				{
					_new_cmd = CMD_CAP_GLOBAL_REF;
					Error(LOG, 0, "LIFT: Command 0x%08x: triggers global referencing", _CmdRunning);						
				}
			}
		}
		
		// --- Reset Position after mech horizont ---
		if (_CmdRunning == CMD_CAP_MECH_ADJUST) 
		{
			if (motors_error(MOTOR_Z_BITS, &motor)) // encoder error
			{
				RX_TestTableStatus.info.ref_done = FALSE;
				Error(ERR_CONT, 0, "LIFT: Command %s: Motor[%d] blocked", _CmdName, motor+1);
			}
			else
			{
				motors_reset(MOTOR_Z_BITS);
				RX_TestTableStatus.posZ = POS_REF_UM + _steps_2_micron(motor_get_step(0));
				_new_cmd = CMD_CAP_GLOBAL_REF;
			}
		}
		
		// --- Set Position Flags after Commands ---
		RX_TestTableStatus.info.move_ok = (RX_TestTableStatus.info.ref_done && (abs(RX_TestTableStatus.posZ - _lastPosCmd) <= 10));
		RX_TestTableStatus.info.head_in_safety = ((RX_TestTableStatus.info.ref_done == TRUE) 
													&& (abs(RX_TestTableStatus.posZ - LIFT_SAFE_POS) <= 10) 
													&& (_CmdRunning == CMD_CAP_UP_POS));
		
		if ((_new_cmd == FALSE) && (_CmdRunning != CMD_CAP_STOP)) 
		{
			RX_TestTableStatus.info.move_tgl = !RX_TestTableStatus.info.move_tgl;
		}
		
		// --- Reset Commands ---
		_CmdRunning = FALSE;
		
		// --- Execute next Commands ---
		if (_new_cmd) cleaf_handle_ctrl_msg(INVALID_SOCKET, _new_cmd, NULL);
		/*		
		switch (_new_cmd)
		{
		case CMD_CAP_REFERENCE: 	cleaf_handle_ctrl_msg(INVALID_SOCKET, CMD_CAP_REFERENCE, NULL);		break;
		case CMD_CAP_REFERENCE_2:	cleaf_handle_ctrl_msg(INVALID_SOCKET, CMD_CAP_REFERENCE_2, NULL);	break;
		case CMD_CAP_GLOBAL_REF:	cleaf_handle_ctrl_msg(INVALID_SOCKET, CMD_CAP_GLOBAL_REF, NULL);	break;
		case CMD_CAP_UP_POS:		cleaf_handle_ctrl_msg(INVALID_SOCKET, CMD_CAP_UP_POS, NULL);		break;
		case 0: break;
		default:					Error(ERR_CONT, 0, "CLEAF_MAIN: Command 0x%08x not implemented", _new_cmd);	break;
		}
		*/
		_new_cmd = FALSE;
		
		// _cleaf_send_status(0); // push news of move end to main
	}	

	//--- HEAD DOWN ENABLE --------------------
	{
		#define DELAY 10
		if (fpga_input(PRINTHEAD_EN)) 	// --- Printhead down enable is ON---
		{
			_enable_off=0;
			if ((_enable_on<DELAY) && ((++_enable_on)==DELAY) && (RX_TestTableStatus.info.printhead_en==FALSE))
			{
				if (RX_TestTableStatus.info.splicing) Error(LOG, 0, "LIFT: printhead_en enabled, go down after splice", _CmdRunning);
				RX_TestTableStatus.info.printhead_en = TRUE;
				RX_TestTableStatus.info.splicing = FALSE;
				_cleaf_send_status(0);
			}
		}
		else // --- Printhead down enable is OFF---
		{
			_enable_on=0;
			if ((_enable_off<DELAY) && ((++_enable_off)==DELAY) && (RX_TestTableStatus.info.printhead_en==TRUE))
			{
				if( ((_CmdRunning == 0)
					|| (RX_TestTableStatus.info.moving == FALSE)
					|| (_CmdRunning == CMD_CAP_PRINT_POS && _lastPosCmd != POS_STORED))
					&& (RX_TestTableStatus.info.z_in_ref == FALSE)
					&& (RX_TestTableStatus.info.ref_done == TRUE)
					&& (RX_TestTableStatus.info.cln_in_capping == FALSE)
					&& (LIFT_SAFE_POS-10  > RX_TestTableStatus.posZ)
					&& (POS_STORED - 10  > RX_TestTableStatus.posZ))
				{
					Error(LOG, 0, "LIFT: printhead_en dissabled, go up for splice", _CmdRunning);
					RX_TestTableStatus.info.splicing = TRUE;
					cleaf_handle_ctrl_msg(INVALID_SOCKET, CMD_CAP_UP_POS, NULL); // go to pos 0
				}
				RX_TestTableStatus.info.printhead_en = FALSE;  // if 0 then splice is comming and head has to go up, if not in capping
				_cleaf_send_status(0);				
			}
		}
	}		
	
	// Read LASER value
	INT32 laser_value = ((LASER_VOLT_OFFSET - ((int)(VAL_TO_MV(Fpga.stat->analog_in[3]) * 2))) * LASER_MM_PER_VOLT); // (mV -mV)*mm/V = um // medium thickness in um
	if (laser_value >= RX_TestTableStatus.posX)
	{
		_laser_high_cnt++;
		_laser_low_cnt = 0;
		_laser_high_value += laser_value;
		_laser_low_value = 0;
	}
	else
	{
		_laser_high_cnt = 0;
		_laser_low_cnt++;
		_laser_high_value = 0;
		_laser_low_value += laser_value;
	}
	if ((_laser_high_cnt = 10) || (_laser_low_cnt = 10))
	{
		//RX_TestTableStatus.posX = (_laser_high_value + _laser_low_value)/ 10.0 ;
		//RX_TestTableStatus.posX = laser_value;
		RX_TestTableStatus.posX = (_laser_high_value + _laser_low_value);
		_laser_high_cnt = 0;
		_laser_low_cnt = 0;
		_laser_high_value = 0;
		_laser_low_value = 0;
	}
	
	// --- LASER check thickness ---
	if ((RX_TestTableStatus.posZ < RX_TestTableStatus.posX - LASER_VARIATION) && (RX_TestTableStatus.info.printhead_en == TRUE) && (RX_TestTableStatus.info.moving == FALSE) && LASER_EN)
	{
		//if (_laser_err_cnt == 10) // cleaf_main gets executed every 1ms
		//{
				// Notstopp
				// Error, emergency up
			Error(ERR_ABORT, 0, "WEB: LASER detecs thick medium %d um - emergency up", RX_TestTableStatus.posX);	
			if (((_CmdRunning == 0)
				|| (RX_TestTableStatus.info.moving == FALSE)
				|| (_CmdRunning == CMD_CAP_PRINT_POS && _lastPosCmd != POS_STORED))
				&& (RX_TestTableStatus.info.z_in_ref == FALSE)
				&& (RX_TestTableStatus.info.ref_done == TRUE)
				&& (RX_TestTableStatus.info.cln_in_capping == FALSE)
				&& (LIFT_SAFE_POS - 10  > RX_TestTableStatus.posZ)
				&& (POS_STORED - 10  > RX_TestTableStatus.posZ))
			{				
				cleaf_handle_ctrl_msg(INVALID_SOCKET, CMD_CAP_UP_POS, NULL); // go to pos 0
			}
		//	_laser_err_cnt = 0;
			_cleaf_send_status(0);
		//}
		//else
		//{
		//	_laser_err_cnt++;
		//}
	}
	//else
	//{
	//	_laser_err_cnt = 0;
	//}	
}

//--- cleaf_display_status ---------------------------------------------------------
void cleaf_display_status(void)
{
	int enc0_um = Fpga.encoder[0].pos * 1000000.0 / CLEAF_INC_PER_METER;
	term_printf("CAPPING Unit ---------------------------------\n");
	term_printf("moving:         %d		cmd: %08x\n",	RX_TestTableStatus.info.moving, _CmdRunning);	
	term_printf("Sensor Reference Motor 0 to 3: %d ", RX_TestTableStatus.info.headUpInput_0);	
	term_printf("%d ", RX_TestTableStatus.info.headUpInput_1);
	term_printf("%d ", RX_TestTableStatus.info.headUpInput_2);
	term_printf("%d\n", RX_TestTableStatus.info.headUpInput_3);
	term_printf("Sensor Capping: %d ", RX_TestTableStatus.info.cln_in_capping);	
//	term_printf("M 0: %d\n", motor_move_done(0));	
//	term_printf("M 1: %d\n", motor_move_done(1));
//	term_printf("M 2: %d\n", motor_move_done(2));
//	term_printf("M 3: %d\n", motor_move_done(3));
	term_printf("Sensor printhead_en: %d\n", RX_TestTableStatus.info.printhead_en);
	term_printf("move tgl bit: %d \n", RX_TestTableStatus.info.move_tgl);
	term_printf("z in reference: %d ", RX_TestTableStatus.info.z_in_ref);
	term_printf("z in print:     %d ", RX_TestTableStatus.info.z_in_print);
	term_printf("z in safety:    %d\n", RX_TestTableStatus.info.head_in_safety);
	term_printf("z in capping:   %d ", RX_TestTableStatus.info.z_in_cap);
	term_printf("x in capping:   %d\n", RX_TestTableStatus.info.x_in_cap);
	term_printf("Flag reference done: %d\n",	RX_TestTableStatus.info.ref_done);
	term_printf("Flag move OK: %d ", RX_TestTableStatus.info.move_ok);
	term_printf("Last pos cmd in um:   %d\n", _lastPosCmd);
	term_printf("LASER in mV:   %d  ", (int)(VAL_TO_MV(Fpga.stat->analog_in[3])*2));
	term_printf("LASER thickness in um:   %d\n", RX_TestTableStatus.posX);
	term_printf("Pos0 in steps:   %d  ", (POS_REF_UM+_steps_2_micron(motor_get_step(0))));
	term_printf("Pos0 in um:   %d\n", RX_TestTableStatus.posZ);
	term_printf("Enc0 in steps:   %d  ", Fpga.encoder[0].pos);
	term_printf("ENC0 in um:   %d\n", enc0_um);
	term_printf("\n");
}

//--- cleaf_menu --------------------------------------------------
int cleaf_menu(void)
{
	char str[MAX_PATH];
	int synth=FALSE;
	static int cnt=0;
	int i;
	int pos;

	if (RX_StepperCfg.printerType!=printer_cleaf) return TRUE;

	cleaf_display_status();
	
	term_printf("MENU CLEAF 3 ------------------------\n");
	//motor_trace_move(0);
	term_printf("s: STOP\n");
	term_printf("r<n>: reset motor<n>\n");	
	term_printf("R: Reference\n");
	term_printf("a: adjust to mech horizont\n");
	term_printf("g: global reference\n");
	term_printf("p: move to <um> ABOVE REF\n");
	term_printf("u: move up emergency\n");
	term_printf("z: move by (sign!) <steps>\n");
	term_printf("m<n><steps>: move Motor<n> by <steps>\n");	
	term_printf("x: exit\n");
	term_printf(">");
	term_flush();
	
	if (term_get_str(str, sizeof(str)))
	{
		switch (str[0])
		{
		case 's': cleaf_handle_ctrl_msg(INVALID_SOCKET, CMD_CAP_STOP,			NULL); break;
		case 'R': cleaf_handle_ctrl_msg(INVALID_SOCKET, CMD_CAP_REFERENCE,	NULL); break;
		case 'r': if (str[1]==0) for (i=0; i<MOTOR_Z_CNT; i++) motor_reset(i);
				  else           motor_reset(atoi(&str[1])); 
				  break;
		case 'a': cleaf_handle_ctrl_msg(INVALID_SOCKET, CMD_CAP_MECH_ADJUST, NULL); break;
		case 'g': cleaf_handle_ctrl_msg(INVALID_SOCKET, CMD_CAP_GLOBAL_REF, NULL); break;
		case 'p': pos = atoi(&str[1]); cleaf_handle_ctrl_msg(INVALID_SOCKET, CMD_CAP_PRINT_POS,	&pos); break;
		case 'u': cleaf_handle_ctrl_msg(INVALID_SOCKET, CMD_CAP_UP_POS, NULL); break;
		case 'z': _cleaf_motor_z_test(atoi(&str[1]));break;
		case 'm': _cleaf_motor_test(str[1]-'0', atoi(&str[2]));break;			
		case 'x': return FALSE;
		}
	}
	return TRUE;
}

//---_micron_2_steps --------------------------------------------------------------
static int  _micron_2_steps(int micron)
{
	return (int)(0.5 + micron * CLEAF_STEPS_PER_METER / 1000000);
}

//---_steps_2_micron --------------------------------------------------------------
static int  _steps_2_micron(int steps)
{
	return (int)(steps * 1000000.0 / CLEAF_STEPS_PER_METER + 0.5);
}

//--- cleaf_handle_ctrl_msg -----------------------------------
int  cleaf_handle_ctrl_msg(RX_SOCKET socket, int msgId, void *pdata)
{	
	int i;
	int pos;
	switch(msgId)
	{
	case CMD_TT_STATUS:				_cleaf_send_status(socket);
									break;

	case CMD_CAP_STOP:				motors_stop(MOTOR_Z_BITS);
									_CmdRunning = msgId;
									strcpy(_CmdName, "CMD_CAP_STOP");
									RX_TestTableStatus.info.moving = TRUE;
									break;	

	case CMD_CAP_REFERENCE:			if (_CmdRunning){cleaf_handle_ctrl_msg(INVALID_SOCKET, CMD_CAP_STOP, NULL); _new_cmd = CMD_CAP_REFERENCE; break; }
									Error(LOG, 0, "CMD_CAP_REFERENCE");
									motors_reset(MOTOR_Z_BITS);  // to recover from move count missalignment
									if ((RX_TestTableStatus.info.headUpInput_0 == 1) 
										|| (RX_TestTableStatus.info.headUpInput_1 == 1) 
										|| (RX_TestTableStatus.info.headUpInput_2 == 1) 
										|| (RX_TestTableStatus.info.headUpInput_3 == 1))
									{
										_cleaf_motor_z_test(-1500); 
										_new_cmd = CMD_CAP_REFERENCE; 
										break; 
									}
									_CmdRunning  = msgId;
									strcpy(_CmdName, "CMD_CAP_REFERENCE");
									RX_TestTableStatus.info.ref_done = FALSE;
									RX_TestTableStatus.info.moving = TRUE;
		_lastPosCmd = POS_STORED;
									//motors_quad_move_by_step(MOTOR_Z_BITS, &_ParRefz, 500000, TRUE);
									for (i = 0; i < MOTOR_Z_CNT; i++) motor_move_by_step(i, &_ParRefz[i], 500000);
									motors_start(MOTOR_Z_BITS, TRUE);										
									break;
		
	case CMD_CAP_REFERENCE_2:		if (_CmdRunning){cleaf_handle_ctrl_msg(INVALID_SOCKET, CMD_CAP_STOP, NULL); _new_cmd = CMD_CAP_REFERENCE_2; break;}
									Error(LOG, 0, "CMD_CAP_REFERENCE_2");
									_CmdRunning  = msgId;
									strcpy(_CmdName, "CMD_CAP_REFERENCE_2");
									RX_TestTableStatus.info.moving = TRUE;
		_lastPosCmd = POS_STORED;
									//motors_quad_move_by_step(MOTOR_Z_BITS, &_ParRef, 500000, TRUE);
									for (i = 0; i < MOTOR_Z_CNT; i++) motor_move_by_step(i, &_ParRef[i], 500000);											
									motors_start(MOTOR_Z_BITS, TRUE);										
									break;
		
	case CMD_CAP_GLOBAL_REF:		if (_CmdRunning){cleaf_handle_ctrl_msg(INVALID_SOCKET, CMD_CAP_STOP, NULL); _new_cmd = CMD_CAP_GLOBAL_REF; break;}
									Error(LOG, 0, "CMD_CAP_GLOBAL_REF");
									_CmdRunning  = msgId;
									strcpy(_CmdName, "CMD_CAP_GLOBAL_REF");
									RX_TestTableStatus.info.moving = TRUE;
		_lastPosCmd = POS_STORED;
									//motors_quad_move_by_step(MOTOR_Z_BITS, &_ParRefz, 500000, TRUE);
									for (i = 0; i < MOTOR_Z_CNT; i++) motor_move_by_step(i, &_ParRefz[i], 500000);											
									motors_start(MOTOR_Z_BITS, TRUE);										
									break;

	case CMD_CAP_PRINT_POS:			if (!_CmdRunning)
									{
										Error(LOG, 0, "CMD_CAP_PRINT_POS");
										pos = (*((INT32*)pdata));
										if ((pos < (RX_TestTableStatus.posX - LASER_VARIATION)) && LASER_EN) 
										{
											Error(ERR_CONT, 0, "LIFT: Command %s: LASER detects too thick medium %d um", _CmdName, RX_TestTableStatus.posX); 
											cleaf_handle_ctrl_msg(INVALID_SOCKET, CMD_CAP_UP_POS, NULL);
											break;
										}
										_CmdRunning  = msgId;
										strcpy(_CmdName, "CMD_CAP_PRINT_POS");
										RX_TestTableStatus.info.moving = TRUE;
										_lastPosCmd = pos;
										//Error(LOG, 0, "Command %s: move to pos %d um", _CmdName, (POS_REF_UM - pos));
										if (!RX_TestTableStatus.info.ref_done) Error(LOG, 0, "LIFT: Command %s: missing ref_done: triggers Referencing", _CmdName);
										if (RX_TestTableStatus.info.ref_done) motors_dual_move_to_step(MOTOR_Z_BITS, &_ParZ_up, &_ParZ_down, -_micron_2_steps((POS_REF_UM - pos)));
										else								  cleaf_handle_ctrl_msg(INVALID_SOCKET, CMD_CAP_REFERENCE, NULL);
									}
									break;
		
	case CMD_CAP_UP_POS:	// 	
		if (_CmdRunning){cleaf_handle_ctrl_msg(INVALID_SOCKET, CMD_CAP_STOP, NULL);  _new_cmd = CMD_CAP_UP_POS; break;}
		Error(LOG, 0, "CMD_CAP_UP_POS");
		if ((LIFT_SAFE_POS > 0) && (LIFT_SAFE_POS < POS_STORED) && (LIFT_SAFE_POS-10  > RX_TestTableStatus.posZ)) // move up to safety
		{
			Error(LOG, 0, "LIFT: Command %s: head goes up to LIFT_SAFE_POS", _CmdName);	
			_CmdRunning  = msgId;
			strcpy(_CmdName, "CMD_CAP_UP_POS");
			RX_TestTableStatus.info.moving = TRUE;
			pos = LIFT_SAFE_POS;
			_lastPosCmd = pos;
			//motors_move_to_step(MOTOR_Z_BITS, &_ParZ_up_f, -pos);
			motors_move_to_step(MOTOR_Z_BITS, &_ParZ_up, -_micron_2_steps((POS_REF_UM - pos)));
		}
		else // Move up to zero
		{
			Error(LOG, 0, "LIFT: Command %s: head goes up to Zero", _CmdName);	
			_CmdRunning  = msgId;
			strcpy(_CmdName, "CMD_CAP_UP_POS");
			RX_TestTableStatus.info.moving = TRUE;
			pos = POS_STORED;
			_lastPosCmd = pos;
			//motors_move_to_step(MOTOR_Z_BITS, &_ParZ_up_f, -pos);
			motors_move_to_step(MOTOR_Z_BITS, &_ParZ_up, -_micron_2_steps((POS_REF_UM - pos)));
		}
		break;
		
	case CMD_CAP_MECH_ADJUST:		if ((!_CmdRunning))
		{
			Error(LOG, 0, "CMD_CAP_MECH_ADJUST");
			_CmdRunning  = msgId;
			strcpy(_CmdName, "CMD_CAP_MECH_ADJUST");
			RX_TestTableStatus.info.moving = TRUE;
			_new_cmd = CMD_CAP_GLOBAL_REF;
			motor_move_by_step(MOT_HEAD_0, &_ParZ_adj, -_micron_2_steps(MOT_0_SENS2MECH_UM));
			motor_move_by_step(MOT_HEAD_1, &_ParZ_adj, -_micron_2_steps(MOT_1_SENS2MECH_UM));
			motor_move_by_step(MOT_HEAD_2, &_ParZ_adj, -_micron_2_steps(MOT_2_SENS2MECH_UM));
			motor_move_by_step(MOT_HEAD_3, &_ParZ_adj, -_micron_2_steps(MOT_3_SENS2MECH_UM));
			motors_start(MOTOR_Z_BITS, TRUE);
		}
		break;
		
	case CMD_ERROR_RESET:		
		_cleaf_error_reset(); 
		fpga_stepper_error_reset();
		break;
	
	default:						Error(ERR_CONT, 0, "LIFT: Command 0x%08x not implemented", msgId); break;
	}
}

//--- _cleaf_motor_z_test ----------------------------------------------------------------------
void _cleaf_motor_z_test(int steps)
{
	SMovePar par;

	par.speed		= 2000;
	par.accel		= 1000;
	par.current		= 300.0; // 350.0; // 300.0; // for Tests:30
	par.stop_mux	= MOTOR_Z_BITS;
	par.estop_in    = ESTOP_UNUSED;
	par.estop_level = 0;
	par.checkEncoder= TRUE;
	
	_CmdRunning = 1; // TEST
	RX_TestTableStatus.info.moving = TRUE;
	
	motors_move_by_step(MOTOR_Z_BITS, &par, steps, TRUE);	
}

//--- _cleaf_motor_test ---------------------------------
static void _cleaf_motor_test(int motorNo, int steps)
{
	int motor = 1<<motorNo;
	SMovePar par;
	int i;

	par.speed		= 2000;
	par.accel		= 1000;
	par.current		= 420.0;// 50.0; // for Tests:30
	par.stop_mux	= 0;
	par.estop_in    = ESTOP_UNUSED;
	par.estop_level = 0;
	par.checkEncoder= FALSE;
	
	_CmdRunning = 1; // TEST
	RX_TestTableStatus.info.moving = TRUE;
	
	for (i = 0; i < MOTOR_CNT; i++) if (motor & (1 << i)) motor_config(i, CURRENT_HOLD, CLEAF_STEPS_PER_METER, CLEAF_INC_PER_METER);
	motors_move_by_step(motor, &par, steps, FALSE);			
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
	static RX_SOCKET _socket=INVALID_SOCKET;
	if (socket!=0 && socket!=INVALID_SOCKET) _socket=socket;
	if (_socket!=INVALID_SOCKET) sok_send_2(&_socket, INADDR_ANY, REP_TT_STATUS, sizeof(RX_TestTableStatus), &RX_TestTableStatus);	
}
