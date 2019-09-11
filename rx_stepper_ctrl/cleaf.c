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

// Lift Z Axis
#define MOTOR_Z_CNT		2
#define MOTOR_Z_FRONT		0
#define MOTOR_Z_BACK		1
#define MOTOR_Z_BITS	((0x01<<MOTOR_Z_FRONT) | (0x01<<MOTOR_Z_BACK))

#define CURRENT_Z_HOLD	50

#define POS_UP				1000
#define PRINT_HEIGHT_MIN	1000

#define DIST_Z_REV		2000.0	// moving distance per revolution [µm]

// Laser
#define	LASER_IN				0		// Analog Input 0-3
#define LASER_VOLT_PER_MM		0.25 // V/mm
#define LASER_MM_PER_VOLT		4    // mm/V
#define LASER_VOLT_OFFSET 		5900 // Laser Value without medium in mV
#define LASER_VARIATION			400 // Variation +- in um
#define LASER_TIMEOUT			2000 // ms
#define LASER_ANALOG_AVR		10.0

#define HEAD_DOWN_EN_DELAY		10
//#define MAX_POS_UM		 		90000 //Resolution 20um, Working range 45...85 mm, temperature drift 18 um/K, Value range 0...10V linear rising, 1V=4mm ?

#define VAL_TO_MV_AI(x) ((x*10000)/0x0fff)

// Digital Inputs (max 12)
#define HEAD_UP_IN_FRONT	0
#define HEAD_UP_IN_BACK		1

// DRIP PAN FUNCTION
#define DRIP_PANS_INFEED_DOWN		5
#define DRIP_PANS_INFEED_UP			6
#define DRIP_PANS_OUTFEED_DOWN		7
#define DRIP_PANS_OUTFEED_UP		8

#define PRINTHEAD_EN		11	// Input from SPS // '1' Allows Head to go down

// Digital Outputs (max 12)
// #define RO_FLUSH_CAR_0		0x001 //  0 // Y1 Flush Servicecar 0
// #define RO_BLADE_UP_0		0x008 //  3 // SY13 Wipe-Blade 0
// #define RO_VACUUM_BLADE		0x020 //  5 // Y8 Vakuum Blade
// #define RO_VACUUM			0x040 //  6 // SY10 Vakuum
// #define RO_DRAIN_WASTE		0x080 // 7 // Y6 & Y7 // 1 == Waste Beh. Absaugen // 0 == Cable Chain absaugen 
// #define RO_FLUSH_VOLT		0x100
// #define RO_ALL_OUT_MECH			0x00E // ALL robot (blades and screws)
#define RO_ALL_OUTPUTS			0x2FF //  ALL Outputs

// DRIP PAN FUNCTION
#define DRIP_PANS_VALVE_ALL		0x300
#define DRIP_PANS_VALVE_UP		0x100
#define DRIP_PANS_VALVE_DOWN	0x200

#define REF_HEIGHT			RX_StepperCfg.robot[RX_StepperCfg.boardNo].ref_height
#define HEAD_ALIGN			RX_StepperCfg.robot[RX_StepperCfg.boardNo].head_align

#define CAPPING_HEIGHT			35000


//--- global  variables -----------------------------------------------------------------------------------------------------------------
static RX_SOCKET _MainSocket= INVALID_SOCKET;
static int	_CmdRunning = 0;
static char	_CmdName[32];
static int  _LastRobPosCmd = 0;
static int	_PrintPos;
static int	_DripPans_Connected = FALSE;
static int  _AllowMoveDown = 0;
static int  _DripPansPosition = 2;	// 2 = position unknown after start

// Lift
static SMovePar	_Par_Z_ref[MOTOR_Z_CNT];
static SMovePar	_ParZ_down;
static SMovePar	_ParZ_cap;

// Laser
static int	_PrintHeadEn_On  = 0;
static int	_PrintHeadEn_Off = 0;
static int	_LaserAvr = 0;
static int	_LaserCnt = 0;

// Timers
static int	_LaserTimeThin;
static int	_LaserTimeThick;

static UINT32	_cleaf_Error=0;

//--- prototypes --------------------------------------------
static void _cleaf_motor_z_test(int steps);
static void _cleaf_motor_test(int motor, int steps);
static void _cleaf_check_laser(void);

// Lift
static int  _z_micron_2_steps(int micron);
static int  _z_steps_2_micron(int steps);

//--- cleaf_init --------------------------------------
void cleaf_init(void)
{
	int i;

	memset(_CmdName, 0, sizeof(_CmdName));

	Error(LOG, 0, "cleaf_init: material_thickness=%d", RX_StepperCfg.material_thickness);
	
	//--- movment parameters Lift ----------------
	for (i = 0; i < MOTOR_Z_CNT; i++) 
	{
		_Par_Z_ref[i].speed			= 16000; // 10000; // 2mm/U // 10mm/s => 5U/s*200steps/U*16=16000  //  20mm/s => 10U/s*200steps/U*16=32000
		_Par_Z_ref[i].accel			=  8000; // 5000;
		_Par_Z_ref[i].current		= 300; // 250.0;
//		_Par_Z_ref[i].stop_mux		= MOTOR_Z_BITS;
		_Par_Z_ref[i].stop_mux		= 0;
		_Par_Z_ref[i].dis_mux_in	= TRUE;
		_Par_Z_ref[i].stop_in		= ESTOP_UNUSED;
		_Par_Z_ref[i].stop_level	= 0;
		_Par_Z_ref[i].estop_level	= 1;
		_Par_Z_ref[i].checkEncoder	= TRUE;
	}

	_Par_Z_ref[MOTOR_Z_FRONT].estop_in	= HEAD_UP_IN_FRONT; 
	_Par_Z_ref[MOTOR_Z_BACK].estop_in	= HEAD_UP_IN_BACK; 
	
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
				
	//--- Outputs ----------------
	Fpga.par->output &= ~RO_ALL_OUTPUTS; // ALL OUTPUTS
}

//--- cleaf_main ------------------------------------------------------------------
void cleaf_main(int ticks, int menu)
{
	int motor;
	int lift_pos;
	SStepperStat	oldStatus;
		
	if (!motors_init_done())
	{
		motors_config(MOTOR_Z_BITS, CURRENT_Z_HOLD, 0.0, 0.0);
	}
	
	if (RX_StepperCfg.printerType != printer_cleaf) return; // printer_cleaf
	motor_main(ticks, menu);
		
	// --- read Inputs Cap ---
	RX_StepperStatus.info.headUpInput_0 = fpga_input(HEAD_UP_IN_FRONT);
	RX_StepperStatus.info.headUpInput_1 = fpga_input(HEAD_UP_IN_BACK);
	RX_StepperStatus.posZ = REF_HEIGHT - _z_steps_2_micron(motor_get_step(0));
		
	// Drip Pans : enabled when the main send a command CMD_CLN_DRIP_PANS
	RX_StepperStatus.info.DripPans_InfeedDOWN	= _DripPans_Connected && fpga_input(DRIP_PANS_INFEED_DOWN);
	RX_StepperStatus.info.DripPans_InfeedUP		= _DripPans_Connected && fpga_input(DRIP_PANS_INFEED_UP);
	RX_StepperStatus.info.DripPans_OutfeedDOWN	= _DripPans_Connected && fpga_input(DRIP_PANS_OUTFEED_DOWN);
	RX_StepperStatus.info.DripPans_OutfeedUP	= _DripPans_Connected && fpga_input(DRIP_PANS_OUTFEED_UP);

	// --- set positions False while moving ---
	RX_StepperStatus.info.moving = (_CmdRunning!=0);

	if (_CmdRunning)
	{
		RX_StepperStatus.info.z_in_ref   = FALSE;
		RX_StepperStatus.info.z_in_print = FALSE;
		RX_StepperStatus.info.z_in_cap   = FALSE;
	}

	// --- Executed after each move ---
	if (_CmdRunning && motors_move_done(MOTOR_ALL_BITS))
	{		
		RX_StepperStatus.info.moving = FALSE;
				
		// --- tasks after motor error ---
		if (_CmdRunning != CMD_CAP_REFERENCE)
		{
			if (motors_error(MOTOR_ALL_BITS, &motor))
			{
				RX_StepperStatus.info.ref_done   = FALSE;
				
				if      (motor==MOTOR_Z_FRONT)	Error(ERR_CONT, 0, "Stepper: Command %s: Motor Front blocked", _CmdName);
				else if (motor==MOTOR_Z_BACK)	Error(ERR_CONT, 0, "Stepper: Command %s: Motor Back blocked", _CmdName);
				else							Error(ERR_CONT, 0, "Stepper: Command %s: Motor[%d] blocked", _CmdName, motor + 1);
				_CmdRunning = FALSE;
				Fpga.par->output &= ~RO_ALL_OUTPUTS; // set all output to off
			}			
		}
		
		if (_CmdRunning == CMD_CAP_REFERENCE)
		{
			motors_reset(MOTOR_Z_BITS);
			if (!fpga_input(HEAD_UP_IN_FRONT)) Error(LOG, 0, "Stepper: Command REFERENCE: End Sensor Front NOT HIGH");
			if (!fpga_input(HEAD_UP_IN_BACK))  Error(LOG, 0, "Stepper: Command REFERENCE: End Sensor Back NOT HIGH");
			RX_StepperStatus.info.ref_done = (RX_StepperStatus.info.headUpInput_0 && RX_StepperStatus.info.headUpInput_1);
			motors_move_to_step(MOTOR_Z_BITS, &_ParZ_down, POS_UP);			
		}

		RX_StepperStatus.info.z_in_ref   = (_CmdRunning==CMD_CAP_REFERENCE || _CmdRunning==CMD_CAP_UP_POS);
		RX_StepperStatus.info.z_in_print = (_CmdRunning==CMD_CAP_PRINT_POS);
		RX_StepperStatus.info.z_in_cap   = (_CmdRunning==CMD_CAP_CAPPING_POS);
		
		_CmdRunning = 0;
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
				sok_send_2(&_MainSocket, REP_TT_STATUS, sizeof(RX_StepperStatus), &RX_StepperStatus);
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
					sok_send_2(&_MainSocket, REP_TT_STATUS, sizeof(RX_StepperStatus), &RX_StepperStatus);
				}
				RX_StepperStatus.info.printhead_en = FALSE;  // if 0 then splice is comming and head has to go up, if not in capping
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
				Error(WARN, 0, "WEB: Laser detects too thick material. Moving head up. (measured %d, expected %d)", RX_StepperStatus.posY, RX_StepperCfg.material_thickness);
				_LaserTimeThick = 0; // start next check
			}
			
			if (_LaserTimeThin && (rx_get_ticks()-_LaserTimeThin) > LASER_TIMEOUT)
			{
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
		term_printf("%6s    ", _value_str3(_z_steps_2_micron(motor_get_step(MOTOR_Z_FRONT))));
		term_printf("%6s    ", _value_str3(_z_steps_2_micron(motor_get_step(MOTOR_Z_BACK))));
		term_printf("\n");
	
	{
	//	term_printf("Cleaf Stepper LIFT ---------------------------------\n");
		term_printf("moving:         %d		cmd: %08x\n", RX_StepperStatus.info.moving, _CmdRunning);	
		term_printf("refheight:      %06d  ", RX_StepperCfg.ref_height);
		term_printf("pos in um:      %06d  \n", RX_StepperStatus.posZ);
		term_printf("LASER in um:      %06d  row=%06d \n", RX_StepperStatus.posY, Fpga.stat->analog_in[LASER_IN]);	
		term_printf("Head UP Sensor: front: %d  back: %d\n",	fpga_input(HEAD_UP_IN_FRONT), fpga_input(HEAD_UP_IN_BACK));	
		term_printf("reference done: %d\n", RX_StepperStatus.info.ref_done);
		term_printf("z in ref:%d print:%d cap:%d\n", RX_StepperStatus.info.z_in_ref, RX_StepperStatus.info.z_in_print, RX_StepperStatus.info.z_in_cap);
//		term_printf("move tgl bit: %d \n", RX_StepperStatus.info.move_tgl);
		term_printf("allow move down: %d \n", _AllowMoveDown);
		term_printf("drips pans connected: %d \n", _DripPans_Connected);
		term_printf("drips pans valve: %d \n", _DripPansPosition);
		term_printf("drips pans UP:   Infeed: %d Outfeed: %d\n", RX_StepperStatus.info.DripPans_InfeedUP,   RX_StepperStatus.info.DripPans_OutfeedUP);
		term_printf("drips pans DOWN: Infeed: %d Outfeed: %d\n", RX_StepperStatus.info.DripPans_InfeedDOWN, RX_StepperStatus.info.DripPans_OutfeedDOWN);
		term_printf("\n");
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
	
	{
//		term_printf("MENU LIFT ----------------\n");
		term_printf("R: Reference lift\n");
		term_printf("p: move lift to print\n");
		term_printf("c: move lift to cap\n");
		term_printf("u: move lift to UP position\n");
		term_printf("z: move lift by <steps>\n");
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
		
		case 'R': cleaf_handle_ctrl_msg(INVALID_SOCKET, CMD_CAP_REFERENCE, NULL); break;
		case 'c': cleaf_handle_ctrl_msg(INVALID_SOCKET, CMD_CAP_CAPPING_POS, NULL); break;
		case 'p': cleaf_handle_ctrl_msg(INVALID_SOCKET, CMD_CAP_PRINT_POS, &pos); break;
		case 'u': cleaf_handle_ctrl_msg(INVALID_SOCKET, CMD_CAP_UP_POS, NULL); break;
		case 'z': _cleaf_motor_z_test(atoi(&str[1])); break;
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
	
//--- cleaf_handle_ctrl_msg -----------------------------------
int  cleaf_handle_ctrl_msg(RX_SOCKET socket, int msgId, void *pdata)
{	
	int i;
	int pos, steps;
	
	_MainSocket = socket;
	switch (msgId)
	{
	case CMD_TT_STATUS:			
		sok_send_2(&socket, REP_TT_STATUS, sizeof(RX_StepperStatus), &RX_StepperStatus);				
		break;
		
		// ============================================================== Hub ==============================================================

	case CMD_CAP_STOP:				strcpy(_CmdName, "CMD_CAP_STOP");
		motors_stop(MOTOR_ALL_BITS); // Stops Hub and Robot
		Fpga.par->output &= ~RO_ALL_OUTPUTS; // set all output to off
		RX_StepperStatus.info.moving = TRUE;
		_CmdRunning = msgId;
		break;

	case CMD_CAP_REFERENCE:			strcpy(_CmdName, "CMD_CAP_REFERENCE");
	//	Error(LOG, 0, "got CMD_CAP_REFERENCE");
		if (!_CmdRunning)
		{ 
			motors_reset(MOTOR_ALL_BITS); // to recover from move count missalignment
			RX_StepperStatus.info.ref_done		= FALSE;
			Fpga.par->output &= ~RO_ALL_OUTPUTS;
			_CmdRunning  = CMD_CAP_REFERENCE;
			RX_StepperStatus.info.moving = TRUE;
			motor_move_by_step(MOTOR_Z_FRONT, &_Par_Z_ref[0], -500000);
			motor_move_by_step(MOTOR_Z_BACK,  &_Par_Z_ref[1], -500000);
			motors_start(MOTOR_Z_BITS, TRUE);
		}
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
		if(!_CmdRunning && _AllowMoveDown)
		{
			if      (!RX_StepperStatus.info.ref_done)				Error(ERR_CONT, 0, "Reference not done");
			else if (!_AllowMoveDown)								Error(ERR_CONT, 0, "Stepper: Command 0x%08x: printhead_en signal not set", msgId);
		//	else if (!RX_StepperStatus.info.DripPans_InfeedDOWN)	Error(ERR_CONT, 0, "Stepper: Command 0x%08x: Drip pan Infeed is not DOWN", msgId);
		//	else if (!RX_StepperStatus.info.DripPans_OutfeedDOWN)	Error(ERR_CONT, 0, "Stepper: Command 0x%08x: Drip pan Outfeed is not DOWN", msgId);
		//	else if (RX_StepperStatus.posY < (RX_StepperCfg.material_thickness - LASER_VARIATION) 
		//		||   RX_StepperStatus.posY > (RX_StepperCfg.material_thickness + LASER_VARIATION)) Error(ERR_CONT, 0, "WEB: Laser detects material out of range. (measured %d, expected %d)", RX_StepperStatus.posY, RX_StepperCfg.material_thickness);
			else
			{
				_CmdRunning = CMD_CAP_PRINT_POS;
				motor_move_to_step(MOTOR_Z_FRONT, &_ParZ_down, _z_micron_2_steps(REF_HEIGHT - _PrintPos));
				motor_move_to_step(MOTOR_Z_BACK,  &_ParZ_down, _z_micron_2_steps(REF_HEIGHT - _PrintPos + HEAD_ALIGN));
				motors_start(MOTOR_Z_BITS, TRUE);
			}
		}				
		break;
		
	case CMD_CAP_CAPPING_POS:		strcpy(_CmdName, "CMD_CAP_CAPPING_POS");
		if (!_CmdRunning && RX_StepperStatus.info.ref_done)
		{
			_CmdRunning = msgId;
			motor_move_to_step(MOTOR_Z_FRONT, &_ParZ_cap, _z_micron_2_steps(CAPPING_HEIGHT));		
			motor_move_to_step(MOTOR_Z_BACK,  &_ParZ_cap, _z_micron_2_steps(CAPPING_HEIGHT));		
			motors_start(MOTOR_Z_BITS, FALSE);
		}
		break;

	case CMD_CAP_UP_POS:			
		strcpy(_CmdName, "CMD_CAP_UP_POS");
//		Error(LOG, 0, "got CMD_CAP_UP_POS _CmdRunning=0x%08x", _CmdRunning);
		if (!_CmdRunning && RX_StepperStatus.info.ref_done)
		{
			_CmdRunning = msgId;
			motors_move_to_step(MOTOR_Z_BITS, &_ParZ_down, POS_UP);
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
										
	// ============================================================== Reset ==============================================================

	case CMD_ERROR_RESET:		//strcpy(_CmdName, "CMD_ERROR_RESET");		
		_cleaf_Error = 0;
		fpga_stepper_error_reset();
		break;
		
	case CMD_CAP_VENT:	break;
		
	default:	Error(WARN, 0, "LIFT: Command 0x%08x not implemented", msgId); break;
	}
	
	return TRUE;
}

//--- _cleaf_motor_test ---------------------------------
static void _cleaf_motor_test(int motorNo, int steps)
{
	int motor = 1 << motorNo;
	SMovePar par;
	int i;
	int mot_steps = steps;

	if ((motorNo == MOTOR_Z_FRONT) || (motorNo == MOTOR_Z_BACK)) {
		par.speed	= 5000;
		par.accel	= 2500;
		par.current	= 320.0;
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

//--- _cleaf_motor_z_test ----------------------------------------------------------------------
void _cleaf_motor_z_test(int steps)
{	
	_CmdRunning = 1; // TEST
	RX_StepperStatus.info.moving = TRUE;
	motors_move_by_step(MOTOR_Z_BITS, &_ParZ_down, -steps, TRUE);
}
