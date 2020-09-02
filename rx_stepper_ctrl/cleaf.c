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
#include "args.h"
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
#define MOTOR_Z_FRONT	0
#define MOTOR_Z_BACK	1
#define MOTOR_Z_BITS	((0x01<<MOTOR_Z_FRONT) | (0x01<<MOTOR_Z_BACK))

#define CURRENT_Z_HOLD	50

#define POS_UP			    10000		// um
#define PRINT_HEIGHT_MIN	1000		// um

#define HEIGHT_INCREASE_AFTER_MATERIAL_DETECTION	1000	//um
#define HEIGHT_INCREASE_TIMEOUT						2000	//ms

#define DIST_Z_REV		2000.0	// moving distance per revolution [�m]

// Laser
#define	LASER_IN				0		// Analog Input 0-3 -> 0 is used for distance sensor before Printheads
//#define LASER_IN_CURRENT		1		// Analog Input 1 is used for distance sensor with current input -> just for tests
#define LASER_VOLT_PER_MM		0.45	// V/mm -> 9V/20mm
//#define LASER_AMPER_PER_MM		1.6		// A/mm -> 16mA/10mm
#define LASER_MM_PER_VOLT		2.22	// mm/V -> 20mm/9V
//#define LASER_MM_PER_AMPER		0.625	// mm/V -> 10mm/16mA
#define LASER_VOLT_OFFSET 		1000	// Laser Value with 20mm material in it (Laser goes from 1-10V for 25-45mm) -> makes an offset of 1000mV
//#define LASER_CURRENT_OFFSET	4000	// Laser Value with 10mm material in it (Laser goes from 4-20mA for 20-30mm) -> makes an offset of 4000uA
#define LASER_VOLT_RANGE		9000	// Laser works from 1-10V which makes a range of 9000mV
//#define LASER_CURRENT_RANGE		16000	// Laser works from 4-20mA mich makes a range of 16000uA
#define LASER_VARIATION			400		// Variation +- in um
#define LASER_TIMEOUT			2000	// ms
#define HEAD_DOWN_EN_DELAY		10
#define LASER_ANALOG_AVR		10.0

#define DRIPPAN_TIMEOUT			500		// ms -> Time to change side on valve. This is needed especially for the first time starting.

#define HEAD_DOWN_EN_DELAY		10
//#define MAX_POS_UM		 		90000 //Resolution 20um, Working range 45...85 mm, temperature drift 18 um/K, Value range 0...10V linear rising, 1V=4mm ?

#define VAL_TO_MV_AI(x) ((x*10000)/0x0fff)
//#define VAL_TO_MA_AI(x) ((x*20500)/0x0fff)

// Digital Inputs (max 12)
#define HEAD_UP_IN_FRONT	0
#define HEAD_UP_IN_BACK		1

// Head supervision with Stepperboard
#define LASER_BEFORE_HEAD_IN		2

// DRIP PAN FUNCTION
#define DRIP_PANS_INFEED_DOWN		5
#define DRIP_PANS_INFEED_UP			6
#define DRIP_PANS_OUTFEED_DOWN		7
#define DRIP_PANS_OUTFEED_UP		8

#define PRINTHEAD_EN		11	// Input from SPS // '1' Allows Head to go down

// Digital Outputs (max 12)
#define LASER_BEFORE_HEAD_OUT	0x001

// DRIP PAN FUNCTION
#define DRIP_PANS_VALVE_ALL		0x300
#define DRIP_PANS_VALVE_DOWN	0x100
#define DRIP_PANS_VALVE_UP		0x200

#define RO_ALL_OUTPUTS			0x0FE //  ALL Outputs instead of Output 0, 8, 9, 10 and 11



#define REF_HEIGHT			RX_StepperCfg.robot[RX_StepperCfg.boardNo].ref_height
#define HEAD_ALIGN			RX_StepperCfg.robot[RX_StepperCfg.boardNo].head_align

#define CAPPING_HEIGHT			35000


//--- global  variables -----------------------------------------------------------------------------------------------------------------
static char	_CmdName[32];
static int  _LastRobPosCmd = 0;
static int	_PrintPos;
static int  _AllowMoveDown = 0;

// Lift
static SMovePar	_Par_Z_ref;
static SMovePar	_ParZ_down;
// static SMovePar	_ParZ_cap;

// Laser
static int	_PrintHeadEn_On  = 0;
static int	_PrintHeadEn_Off = 0;
static int	_LaserAvr = 0;
static int	_LaserCnt = 0;

// Timers
static int	_LaserTimeThin;
static int	_LaserTimeThick;
static int	_PrintHeight_Time;
static int  _DripPanTime;

static UINT32	_cleaf_Error=0;

// Head supervision
static int  _PrintHeightTemp;
static int  _MaterialDetected;
static int  _MaterialDetectedInput;


//--- prototypes --------------------------------------------
static void _cleaf_motor_z_test(int steps);
static void _cleaf_motor_test(int motor, int steps);
static void _cleaf_check_laser(void);
static void _check_material_supervision(void);

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
	_Par_Z_ref.speed			= 16000; // 10000; // 2mm/U // 10mm/s => 5U/s*200steps/U*16=16000  //  20mm/s => 10U/s*200steps/U*16=32000
	_Par_Z_ref.accel			= 32000; // 8000; // 5000;
	_Par_Z_ref.current_acc		= 320; // 250.0;
	_Par_Z_ref.current_run		= 300; // 250.0;
	_Par_Z_ref.stop_mux			= MOTOR_Z_BITS;
	_Par_Z_ref.dis_mux_in		= TRUE;
	_Par_Z_ref.estop_level		= 1;
	_Par_Z_ref.encCheck			= chk_std;

	_Par_Z_ref.estop_in_bit[MOTOR_Z_FRONT]	= (1<<HEAD_UP_IN_FRONT);
	_Par_Z_ref.estop_in_bit[MOTOR_Z_BACK]	= (1<<HEAD_UP_IN_BACK); 
	
    _ParZ_down.speed		= 30000; // 16000; too slow for checks
	_ParZ_down.accel		= 32000; 
	_ParZ_down.current_acc	= 320.0; 	
	_ParZ_down.current_run	= 300.0; 	
	_ParZ_down.stop_mux		= MOTOR_Z_BITS;
	_ParZ_down.dis_mux_in	= FALSE;
	_ParZ_down.encCheck		= chk_std;

	/*
	_ParZ_cap.speed			= 10000; // 1000;
	_ParZ_cap.accel			=  5000; //1000;
	_ParZ_cap.current		= 150.0;
	_ParZ_cap.stop_mux		= FALSE;
	_ParZ_down.dis_mux_in	= FALSE;
	_ParZ_cap.stop_in_bits	= 0;
	_ParZ_cap.stop_level	= 0;
	_ParZ_cap.estop_in      = ESTOP_UNUSED;
	_ParZ_cap.estop_level   = 0;
	_ParZ_cap.checkEncoder  = TRUE;
	*/
		
	//--- Outputs ----------------
	Fpga.par->output &= ~RO_ALL_OUTPUTS; // ALL OUTPUTS
}

//--- cleaf_main ------------------------------------------------------------------
void cleaf_main(int ticks, int menu)
{
	int motor;
	int lift_pos;
	SStepperStat	oldStatus;
		
	SStepperStat oldSatus;
	memcpy(&oldSatus, &RX_StepperStatus, sizeof(RX_StepperStatus));

	if (!motors_init_done())
	{
		motors_config(MOTOR_Z_BITS, CURRENT_Z_HOLD, L5918_STEPS_PER_METER, L5918_INC_PER_METER, STEPS);
	}
	
	if (RX_StepperCfg.printerType != printer_cleaf) return; // printer_cleaf
	motor_main(ticks, menu);
	
	
		
	// --- read Inputs Cap ---
	RX_StepperStatus.info.headUpInput_0 = fpga_input(HEAD_UP_IN_FRONT);
	RX_StepperStatus.info.headUpInput_1 = fpga_input(HEAD_UP_IN_BACK);
	
	RX_StepperStatus.posZ = REF_HEIGHT - _z_steps_2_micron(motor_get_step(0));
		
	// Drip Pans : enabled when the main send a command CMD_ROB_DRIP_PANS
	RX_StepperStatus.info.DripPans_InfeedDOWN	= (RX_StepperCfg.boardNo==0) && fpga_input(DRIP_PANS_INFEED_DOWN);
	RX_StepperStatus.info.DripPans_InfeedUP		= (RX_StepperCfg.boardNo==0) && fpga_input(DRIP_PANS_INFEED_UP);
	RX_StepperStatus.info.DripPans_OutfeedDOWN	= (RX_StepperCfg.boardNo==0) && fpga_input(DRIP_PANS_OUTFEED_DOWN);
	RX_StepperStatus.info.DripPans_OutfeedUP	= (RX_StepperCfg.boardNo==0) && fpga_input(DRIP_PANS_OUTFEED_UP);

	// --- set positions False while moving ---
	RX_StepperStatus.info.moving = (RX_StepperStatus.cmdRunning!=0);

	if (RX_StepperStatus.cmdRunning && RX_StepperStatus.cmdRunning != CMD_ROB_DRIP_PANS)
	{
		RX_StepperStatus.info.z_in_ref   = FALSE;
		RX_StepperStatus.info.z_in_up    = FALSE;
		RX_StepperStatus.info.z_in_print = FALSE;
		RX_StepperStatus.info.z_in_cap   = FALSE;
	}
	
	// Enable supervision for printheads
	if (RX_StepperStatus.info.z_in_print || _MaterialDetected)	_MaterialDetectedInput = fpga_input(LASER_BEFORE_HEAD_IN);
	else														_MaterialDetectedInput = FALSE;


	// --- Executed after each move ---
	if (RX_StepperStatus.cmdRunning && motors_move_done(MOTOR_ALL_BITS))
	{		
		RX_StepperStatus.info.moving = FALSE;
				
		// --- tasks after motor error ---
		if (RX_StepperStatus.cmdRunning != CMD_LIFT_REFERENCE)
		{
			if (motors_error(MOTOR_ALL_BITS, &motor))
			{
				RX_StepperStatus.info.ref_done   = FALSE;
				
				if      (motor==MOTOR_Z_FRONT)	Error(ERR_CONT, 0, "Stepper: Command %s: Motor Front blocked", _CmdName);
				else if (motor==MOTOR_Z_BACK)	Error(ERR_CONT, 0, "Stepper: Command %s: Motor Back blocked", _CmdName);
				else							Error(ERR_CONT, 0, "Stepper: Command %s: Motor[%d] blocked", _CmdName, motor + 1);
				RX_StepperStatus.cmdRunning = FALSE;
				Fpga.par->output &= ~RO_ALL_OUTPUTS; // set all output to off
			}			
		}
		
		if (RX_StepperStatus.cmdRunning == CMD_LIFT_REFERENCE)
		{
			motors_reset(MOTOR_Z_BITS);
			if (!fpga_input(HEAD_UP_IN_FRONT)) Error(LOG, 0, "Stepper: Command REFERENCE: End Sensor Front NOT HIGH");
			if (!fpga_input(HEAD_UP_IN_BACK))  Error(LOG, 0, "Stepper: Command REFERENCE: End Sensor Back NOT HIGH");
			RX_StepperStatus.info.ref_done = (RX_StepperStatus.info.headUpInput_0 && RX_StepperStatus.info.headUpInput_1);
			motors_move_to_step(MOTOR_Z_BITS, &_ParZ_down, 100);			
		}
		
		if (RX_StepperStatus.cmdRunning == CMD_LIFT_UP_POS)
		{
			Fpga.par->output |= LASER_BEFORE_HEAD_OUT;			// Laser for supervision OFF
		}

		if (RX_StepperStatus.cmdRunning == CMD_ROB_DRIP_PANS)
		{
			if (rx_get_ticks() > _DripPanTime + DRIPPAN_TIMEOUT /*&& RX_StepperStatus.info.z_in_ref*/)
			{
				/*
				First it should move for 0.5 seconds to the position it is already and only then it should move the other way, 
				this is to fill the tubes and so it shouldn't move too fast down. The other part is in the cleaf_handle_ctrl_msg().
				Here it moves to the final position 
				*/
				if (fpga_input(DRIP_PANS_INFEED_UP) || fpga_input(DRIP_PANS_OUTFEED_UP))
				{
					// all stepper motors in reference
					Fpga.par->output &= ~DRIP_PANS_VALVE_ALL;
					Fpga.par->output |= DRIP_PANS_VALVE_DOWN;
				}
				else 
				{
					// all stepper motors in reference
					Fpga.par->output &= ~DRIP_PANS_VALVE_ALL;
					Fpga.par->output |= DRIP_PANS_VALVE_UP;
				}
				_DripPanTime = 0;
				RX_StepperStatus.cmdRunning = 0;
			}
		}
		else
		{
			RX_StepperStatus.info.z_in_ref   = (RX_StepperStatus.cmdRunning == CMD_LIFT_REFERENCE);
			RX_StepperStatus.info.z_in_up    = (RX_StepperStatus.cmdRunning == CMD_LIFT_UP_POS);
			RX_StepperStatus.info.z_in_print = (RX_StepperStatus.cmdRunning == CMD_LIFT_PRINT_POS);
			RX_StepperStatus.info.z_in_cap   = (RX_StepperStatus.cmdRunning == CMD_LIFT_CAPPING_POS);
			RX_StepperStatus.cmdRunning = 0;
		}
		
	}	
	
	//--- HEAD DOWN ENABLE --------------------
	{
		if (fpga_input(PRINTHEAD_EN)) 	// --- Printhead down enable is ON---
		{
			_PrintHeadEn_Off = 0;
			if (((++_PrintHeadEn_On) == HEAD_DOWN_EN_DELAY) && (RX_StepperStatus.info.printhead_en == FALSE)) // (_PrintHeadEn_On < HEAD_DOWN_EN_DELAY) && 
			{
			//	Error(LOG, 0, "LIFT: printhead_en enabled");
				if (RX_StepperStatus.info.splicing) Error(LOG, 0, "LIFT: printhead_en enabled, splice ended", RX_StepperStatus.cmdRunning);
				RX_StepperStatus.info.printhead_en = TRUE;
				RX_StepperStatus.info.splicing = FALSE;
			//	ctrl_send_2(REP_STEPPER_STAT, sizeof(RX_StepperStatus), &RX_StepperStatus);
			}
		}
		else // --- Printhead down enable is OFF---
		{
			_PrintHeadEn_On = 0;
			if (((++_PrintHeadEn_Off) == HEAD_DOWN_EN_DELAY) && (RX_StepperStatus.info.printhead_en == TRUE)) // (_PrintHeadEn_Off < HEAD_DOWN_EN_DELAY) && 
			{
			//	Error(LOG, 0, "LIFT: printhead_en disabled", RX_StepperStatus.cmdRunning);
				if (RX_StepperStatus.info.z_in_print)
				{	
					Error(LOG, 0, "LIFT: printhead_en disabled, go up for splice");
					RX_StepperStatus.info.splicing = TRUE;
					cleaf_handle_ctrl_msg(INVALID_SOCKET, CMD_LIFT_UP_POS, NULL);
				//	ctrl_send_2(REP_STEPPER_STAT, sizeof(RX_StepperStatus), &RX_StepperStatus);
				}
				RX_StepperStatus.info.printhead_en = FALSE;  // if 0 then splice is comming and head has to go up, if not in capping
			}
		}	
	}
	
	_cleaf_check_laser();
	_check_material_supervision();

	if (memcmp(&oldSatus.info, &RX_StepperStatus.info, sizeof(RX_StepperStatus.info)))
	{
		ctrl_send_2(REP_STEPPER_STAT, sizeof(RX_StepperStatus), &RX_StepperStatus);		
	}

}

//--- _cleaf_check_laser ------------------------------------------
static void _cleaf_check_laser(void)
{					
	//--- Read LASER value -------------------------------------------------------------------------
	INT32 laser_value = ((LASER_VOLT_RANGE - ((int)VAL_TO_MV_AI(Fpga.stat->analog_in[LASER_IN]) - LASER_VOLT_OFFSET)) * LASER_MM_PER_VOLT); // (mV -mV)*mm/V = um // medium thickness in um
	//INT32 laser_value = ((LASER_CURRENT_RANGE - ((int)VAL_TO_MA_AI(Fpga.stat->analog_in[LASER_IN_CURRENT]) - LASER_CURRENT_OFFSET)) * LASER_MM_PER_AMPER); // (uA -uA)*mm/mA = um // medium thickness in um
	_LaserAvr += laser_value;

	if ((++_LaserCnt) == LASER_ANALOG_AVR)
	{
		RX_StepperStatus.posY = _LaserAvr / LASER_ANALOG_AVR;
		_LaserAvr = 0;
		_LaserCnt = 0;
	}
	
//	return;
	
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
				cleaf_handle_ctrl_msg(INVALID_SOCKET, CMD_LIFT_UP_POS, NULL);			
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

// --- _check_material_supervision --------------------------
static void _check_material_supervision(void)
{
	// Supervision for printhead
	// -> If sensor detects material -> Head moves up a bit, if the sensor still detects material, it sends an Error and moves up otherwise it moves down again after a short waiting time
	if (_MaterialDetectedInput && !_MaterialDetected)
	{
		int pos;
		if (!_PrintHeightTemp) _PrintHeightTemp = RX_StepperStatus.posZ;
		pos = RX_StepperStatus.posZ + HEIGHT_INCREASE_AFTER_MATERIAL_DETECTION;
		_MaterialDetected = TRUE;
		cleaf_handle_ctrl_msg(INVALID_SOCKET, CMD_LIFT_PRINT_POS, &pos);
	}
	else if (_MaterialDetected && _MaterialDetectedInput && RX_StepperStatus.info.z_in_print)
	{
		Error(ERR_ABORT, 0, "Laser detects material in front of Printhead.");
		cleaf_handle_ctrl_msg(INVALID_SOCKET, CMD_LIFT_UP_POS, NULL);
	}
	else if (_MaterialDetected && !_MaterialDetectedInput)
	{
		if (!_PrintHeight_Time) _PrintHeight_Time = rx_get_ticks();
		if (rx_get_ticks() > _PrintHeight_Time + HEIGHT_INCREASE_TIMEOUT && !_MaterialDetectedInput)
		{
			cleaf_handle_ctrl_msg(INVALID_SOCKET, CMD_LIFT_PRINT_POS, &_PrintHeightTemp);
			_MaterialDetected = FALSE;
			_PrintHeight_Time = 0;
			_PrintHeightTemp = 0;
		}
		else if (rx_get_ticks() >= _PrintHeight_Time + HEIGHT_INCREASE_TIMEOUT && _MaterialDetectedInput) _PrintHeight_Time = rx_get_ticks();
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
		term_printf("moving:            %d		cmd: %08x\n", RX_StepperStatus.info.moving, RX_StepperStatus.cmdRunning);	
		term_printf("refheight:         %06d  ", RX_StepperCfg.ref_height);
		term_printf("pos in um:         %06d  \n", RX_StepperStatus.posZ);
		term_printf("LASER in um:       %06d  row=%06d \n", RX_StepperStatus.posY, Fpga.stat->analog_in[LASER_IN]);	
		term_printf("Head UP Sensor:    front: %d  back: %d\n",	fpga_input(HEAD_UP_IN_FRONT), fpga_input(HEAD_UP_IN_BACK));	
		term_printf("reference done:    %d\n", RX_StepperStatus.info.ref_done);
		term_printf("allow move down:   %d \n", _AllowMoveDown);
		term_printf("Material detected: %d \n", _MaterialDetectedInput);
		term_printf("z in ref:%d up:%d print: %d cap:%d\n", RX_StepperStatus.info.z_in_ref, RX_StepperStatus.info.z_in_up, RX_StepperStatus.info.z_in_print, RX_StepperStatus.info.z_in_cap);
		if (RX_StepperCfg.boardNo==0)
		{
			term_printf("drips pans valve: up=%d down=%d\n", (Fpga.par->output&DRIP_PANS_VALVE_UP)!=0, (Fpga.par->output&DRIP_PANS_VALVE_DOWN)!=0);
			term_printf("drips pans UP:   Infeed: %d Outfeed: %d\n", RX_StepperStatus.info.DripPans_InfeedUP,   RX_StepperStatus.info.DripPans_OutfeedUP);
			term_printf("drips pans DOWN: Infeed: %d Outfeed: %d\n", RX_StepperStatus.info.DripPans_InfeedDOWN, RX_StepperStatus.info.DripPans_OutfeedDOWN);			
		}
		term_printf("\n");
	}
}

	//--- cleaf_menu --------------------------------------------------
int cleaf_menu(void)
{
	char str[MAX_PATH];
	int synth = FALSE;
	static int cnt = 0;
	static int _showMenu=FALSE;
	int i, old;
	int pos;

	if (RX_StepperCfg.printerType != printer_cleaf) return TRUE;

	cleaf_display_status();
	
	term_printf("MENU CLEAF ------------------------\n");
	
	if (_showMenu)
	{
		term_printf("s: STOP\n");
		term_printf("r<n>: reset motor<n>\n");	
		term_printf("m<n><steps>: move Motor<n> by <steps>\n");
		term_printf("o: toggle output <no>\n");	
		{
	//		term_printf("MENU LIFT ----------------\n");
			term_printf("R: Reference lift\n");
			term_printf("p<pos>: move lift to print <pos>\n");
			term_printf("c: move lift to cap\n");
			term_printf("u: move lift to UP position\n");
			term_printf("i: reset ADC\n");
			term_printf("d: move Drip Pans");
			term_printf("z: move lift by <steps>\n");
		}
		term_printf("x: exit\n");			
	}
	else term_printf("?: HELP for commands\n");			
	term_printf(">");
	term_flush();
	
	if (term_get_str(str, sizeof(str)))
	{		
		switch (str[0])
		{
		case 'o': Fpga.par->output ^= (1 << atoi(&str[1])); break;
		case 's': cleaf_handle_ctrl_msg(INVALID_SOCKET, CMD_LIFT_STOP, NULL); break;
		case 'r': motor_reset(atoi(&str[1])); break;
		case 'm': _cleaf_motor_test(str[1] - '0', atoi(&str[2])); break;
		
		case 'R': cleaf_handle_ctrl_msg(INVALID_SOCKET, CMD_LIFT_REFERENCE, NULL); break;
		case 'c': cleaf_handle_ctrl_msg(INVALID_SOCKET, CMD_LIFT_CAPPING_POS, NULL); break;
		case 'p': pos=atoi(&str[1]);
				   if (arg_debug && REF_HEIGHT==0) REF_HEIGHT=90000;
				  old =  _AllowMoveDown;
				  _AllowMoveDown=TRUE;
				  cleaf_handle_ctrl_msg(INVALID_SOCKET, CMD_LIFT_PRINT_POS, &pos); 
				  _AllowMoveDown = old;	
				  break;
		case 'u': cleaf_handle_ctrl_msg(INVALID_SOCKET, CMD_LIFT_UP_POS, NULL); break;
		case 'i': Fpga.par->adc_rst = TRUE; break;
		case 'd': cleaf_handle_ctrl_msg(INVALID_SOCKET, CMD_ROB_DRIP_PANS, NULL); break;
		case 'z': _cleaf_motor_z_test(atoi(&str[1])); break;
		case 'x': return FALSE;
		case '?': _showMenu=!_showMenu; return TRUE;
		}
		_showMenu = FALSE;
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
	
	switch (msgId)
	{	
		// ============================================================== Hub ==============================================================

	case CMD_LIFT_STOP:				strcpy(_CmdName, "CMD_LIFT_STOP");
		motors_stop(MOTOR_ALL_BITS); // Stops Hub and Robot
		Fpga.par->output &= ~RO_ALL_OUTPUTS; // set all output to off
		RX_StepperStatus.info.moving = TRUE;
		RX_StepperStatus.info.ref_done = FALSE;
		RX_StepperStatus.cmdRunning = msgId;
		_MaterialDetected = FALSE;
		break;

	case CMD_LIFT_REFERENCE:			strcpy(_CmdName, "CMD_LIFT_REFERENCE");
	//	Error(LOG, 0, "got CMD_LIFT_REFERENCE");
		if (!RX_StepperStatus.cmdRunning)
		{ 
			motors_reset(MOTOR_ALL_BITS); // to recover from move count missalignment
			RX_StepperStatus.info.ref_done		= FALSE;
			Fpga.par->output &= ~RO_ALL_OUTPUTS;
			Fpga.par->output |= LASER_BEFORE_HEAD_OUT;			// Laser for supervision OFF
			RX_StepperStatus.cmdRunning  = CMD_LIFT_REFERENCE;
			RX_StepperStatus.info.moving = TRUE;
			motors_move_by_step(MOTOR_Z_BITS, &_Par_Z_ref, -500000, TRUE);
			_MaterialDetected = FALSE;
		}
		break;

	case CMD_LIFT_PRINT_POS:			strcpy(_CmdName, "CMD_LIFT_PRINT_POS");
		pos = (*((INT32*)pdata));
		if (pos<PRINT_HEIGHT_MIN) 
		{
			pos = PRINT_HEIGHT_MIN;				
			Error(WARN, 0, "PrintHeight set to minimum (%d.%03d mm)", PRINT_HEIGHT_MIN/1000, PRINT_HEIGHT_MIN%1000);
		}
		if (pos >= REF_HEIGHT - 1000) pos = REF_HEIGHT - 1000;
		_PrintPos   = RX_StepperCfg.material_thickness + pos;
//		Error(LOG, 0, "got CMD_LIFT_PRINT_POS %d, RX_StepperStatus.cmdRunning=0x%08x ", _PrintPos, RX_StepperStatus.cmdRunning);
		if(!RX_StepperStatus.cmdRunning && _AllowMoveDown)
		{
			if      (!RX_StepperStatus.info.ref_done)				Error(ERR_CONT, 0, "Reference not done");
			else if (!_AllowMoveDown)								Error(ERR_CONT, 0, "Stepper: Command 0x%08x: printhead_en signal not set", msgId);
// #ifdef debug
			else if (RX_StepperStatus.posY < (RX_StepperCfg.material_thickness - LASER_VARIATION) 
				||   RX_StepperStatus.posY > (RX_StepperCfg.material_thickness + LASER_VARIATION)) Error(ERR_CONT, 0, "WEB: Laser detects material out of range. (measured %d, expected %d)", RX_StepperStatus.posY, RX_StepperCfg.material_thickness);
// #endif
			else
			{
				if (REF_HEIGHT<87000) Error(WARN, 0, "Reference Height is only %d.02d mm", REF_HEIGHT/100, REF_HEIGHT%100);
				RX_StepperStatus.cmdRunning = CMD_LIFT_PRINT_POS;
				motor_move_to_step(MOTOR_Z_FRONT, &_ParZ_down, _z_micron_2_steps(REF_HEIGHT - _PrintPos));
				motor_move_to_step(MOTOR_Z_BACK,  &_ParZ_down, _z_micron_2_steps(REF_HEIGHT - _PrintPos + HEAD_ALIGN));
				motors_start(MOTOR_Z_BITS, TRUE);
				Fpga.par->output &= ~LASER_BEFORE_HEAD_OUT;		// Laser for supervision on
				
			}
		}				
		break;
		
	/*
	case CMD_LIFT_CAPPING_POS:		strcpy(_CmdName, "CMD_LIFT_CAPPING_POS");
		if (!RX_StepperStatus.cmdRunning && RX_StepperStatus.info.ref_done)
		{
			RX_StepperStatus.cmdRunning = msgId;
			motor_move_to_step(MOTOR_Z_FRONT, &_ParZ_cap, _z_micron_2_steps(CAPPING_HEIGHT));		
			motor_move_to_step(MOTOR_Z_BACK,  &_ParZ_cap, _z_micron_2_steps(CAPPING_HEIGHT));		
			motors_start(MOTOR_Z_BITS, FALSE);
		}
		break;
	*/
		
	case CMD_LIFT_UP_POS:			
		strcpy(_CmdName, "CMD_LIFT_UP_POS");
//		Error(LOG, 0, "got CMD_LIFT_UP_POS RX_StepperStatus.cmdRunning=0x%08x", RX_StepperStatus.cmdRunning);
		if (!RX_StepperStatus.cmdRunning && RX_StepperStatus.info.ref_done)
		{
			RX_StepperStatus.cmdRunning = msgId;
			motors_move_to_step(MOTOR_Z_BITS, &_ParZ_down, POS_UP);
			_MaterialDetected = FALSE;
			//Fpga.par->output |= LASER_BEFORE_HEAD_OUT;			// Laser for supervision OFF
		}
		break;
		
	case CMD_ROB_DRIP_PANS:
		if (RX_StepperStatus.info.z_in_ref||RX_StepperStatus.info.z_in_up) 
		{
			/*
			First it should move for 0.5 seconds to the position it is already and only then it should move the other way, 
			this is to fill the tubes and so it shouldn't move too fast down. The other part is in the cleaf_main().
			Here it moves first to the position, it is already.
			*/
			RX_StepperStatus.cmdRunning = msgId;
			_DripPanTime = rx_get_ticks();
			if (fpga_input(DRIP_PANS_INFEED_UP) || fpga_input(DRIP_PANS_OUTFEED_UP))
			{
				Fpga.par->output &= ~DRIP_PANS_VALVE_ALL;
				Fpga.par->output |= DRIP_PANS_VALVE_UP;
			}
			else 
			{
				Fpga.par->output &= ~DRIP_PANS_VALVE_ALL;
				Fpga.par->output |= DRIP_PANS_VALVE_DOWN;
			}
		}
		break;
				
	case CMD_LIFT_ALLOW_MOVE_DOWN:
		_AllowMoveDown = 1;
		break;
		
	case CMD_LIFT_NOT_ALLOW_MOVE_DOWN:
		_AllowMoveDown = 0;
		break;
										
	// ============================================================== Reset ==============================================================

	case CMD_ERROR_RESET:		//strcpy(_CmdName, "CMD_ERROR_RESET");
		fpga_stepper_error_reset();
		_cleaf_Error = 0;
		break;
		
	case CMD_LIFT_VENT:	break;
		
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

	memset(&par, 0, sizeof(par));

	par.speed	= 5000;
	par.accel	= 32000;
	par.current_acc	= 400.0;
	par.current_run	= 400.0;
	mot_steps	= -steps;
	
	par.stop_mux		= 0;
	par.dis_mux_in		= 0;
	par.encCheck		= chk_off;
	par.dis_mux_in		= FALSE;
	RX_StepperStatus.info.moving = TRUE;
	RX_StepperStatus.cmdRunning = 1; // TEST

	motors_move_by_step(motor, &par, mot_steps, FALSE);
}

//--- _cleaf_motor_z_test ----------------------------------------------------------------------
void _cleaf_motor_z_test(int steps)
{	
	RX_StepperStatus.cmdRunning = 1; // TEST
	RX_StepperStatus.info.moving = TRUE;
	motors_move_by_step(MOTOR_Z_BITS, &_ParZ_down, -steps, TRUE);
}
