// ****************************************************************************
//
//	DIGITAL PRINTING - fpga.c
//
//	Communication between ARM processor and FPGA
//
// ****************************************************************************
//
//	Copyright 2015 by radex AG, Switzerland. All rights reserved.
//	Written by Marcel Galliker
//
// ****************************************************************************

//--- includes ----------------------------------------------------------------
#ifdef linux
	#include <sys/types.h>
	#include <sys/stat.h>
	#include <sys/mman.h>
	#include <unistd.h>
#endif
#include <fcntl.h>
#include <errno.h>
#include "rx_common.h"
#include "rx_error.h"
#include "rx_file.h"
#include "rx_fpga.h"
#include "rx_mac_address.h"
#include "rx_term.h"
#include "rx_threads.h"
#include "rx_trace.h"
#include "motor.h"
#include "fpga_def_stepper.h"
#include "fpga_stepper.h"

//---- COMPILER OPTIONS ------------------------------------------------------



//--- defines -----------------------------------------------------------------

#define OUT_GUI_START	0x0001

#define VAL_TO_MV(x) ((x*5000)/0x0fff)
#define VAL_TO_MV_AI(x) ((x*10000)/0x0fff)
#define VAL_TO_TEMP(x)	 ((int)(25+(x*2-820)/2.654))

//--- structures ----------------------------


//--- globals -----------------------------------------------------------------

SStepperFpga	Fpga;

//--- module globals ---------------------------------------
static int		_MemId=0;
static int		_Init=FALSE;
static HANDLE	_FpgaThread=NULL;
static int		_PWM_Speed[6];
static int		_GuiTimer=0;
static int		_ErrorCheckTime=0;
	
#define WATCHDOG_CNT	0x7fffffff

//--- Errors ----------------------------------------

static INT32 _ErrorFlags;
static int	 _ErrorCnt[32];

//--- prototypes -------------------------------------------
static void  _check_errors(void);

//*** functions ********************************************

//--- fpga_init ----------------
void fpga_init()
{
	_Init = FALSE;
	_ErrorFlags = 0;
	memset(_ErrorCnt, 0, sizeof(_ErrorCnt));
	if (rx_fpga_load (PATH_BIN_STEPPER FIELNAME_STEPPER_RBF)!=REPLY_OK) 
	{
		//--- simulation in windows ----------------
		Fpga.qsys   = (SFpgaQSys*)		rx_malloc(sizeof(SFpgaQSys));
		Fpga.stat   = (SFpgaStat*)		rx_malloc(sizeof(SFpgaStat));
		Fpga.par    = (SFpgaPar*)		rx_malloc(sizeof(SFpgaPar));
		Fpga.move   = (SMove*)			rx_malloc(sizeof(SMove)*MOTOR_CNT*MOVE_CNT);
		Fpga.encoder= (SFpgaEncoder*)	rx_malloc(sizeof(SFpgaEncoder)*ENCODER_CNT);
		#ifndef  soc
		_Init = TRUE;
		#endif
		return;
	}

#ifdef soc
	//--- map the meory ------------------
	_MemId = open("/dev/mem", O_RDWR | O_SYNC);
	if (_MemId == -1) Error(ERR_CONT, 0, "Could not open memory handle.");

	Fpga.qsys    = (SFpgaQSys*)		rx_fpga_map_page(_MemId, ADDR_FPGA_QSYS,	sizeof(SFpgaQSys),	0x00b4);
	Fpga.stat    = (SFpgaStat*)		rx_fpga_map_page(_MemId, ADDR_FPGA_STAT,	sizeof(SFpgaStat),	0x0200);
	Fpga.par     = (SFpgaPar*)		rx_fpga_map_page(_MemId, ADDR_FPGA_PAR,		sizeof(SFpgaPar),	0x0258);
	Fpga.move    = (SMove*)			rx_fpga_map_page(_MemId, ADDR_FPGA_MOVES,   sizeof(SMove)*MOTOR_CNT*MOVE_CNT, 16*MOTOR_CNT*MOVE_CNT);
	
	if (Fpga.stat->version.build<540) Error(ERR_ABORT, 0, "TOO OLD rx_stepper_rbf v=%d, min=540", Fpga.stat->version.build);
	
	Fpga.encoder = (SFpgaEncoder*)	rx_fpga_map_page(_MemId, ADDR_FPGA_ENCODER, sizeof(SFpgaEncoder)*ENCODER_CNT, 0x0030*ENCODER_CNT);		
#endif

	printf("Version: %lu.%lu.%lu.%lu\n", Fpga.stat->version.major, Fpga.stat->version.minor, Fpga.stat->version.revision, Fpga.stat->version.build);

	RX_StepperStatus.fpgaVersion.major	   = Fpga.stat->version.major;
	RX_StepperStatus.fpgaVersion.minor	   = Fpga.stat->version.minor;
	RX_StepperStatus.fpgaVersion.revision  = Fpga.stat->version.revision;
	RX_StepperStatus.fpgaVersion.build	   = Fpga.stat->version.build;

	Fpga.par->watchdog_freq = 250000;
	Fpga.par->watchdog_cnt  = WATCHDOG_CNT;
	Fpga.par->v24_enable    = TRUE;
	Fpga.par->cmd_reset_pos|= 0xffff; //0x0001<<motor;
	Fpga.par->adc_rst = TRUE; // restart adc
	Fpga.par->min_in_pulse_width = 50000;	// in 20 ns!  

	memset(_PWM_Speed, 0, sizeof(_PWM_Speed));
	
	_GuiTimer = rx_get_ticks()+1000;
	Fpga.par->output |= OUT_GUI_START;
	_Init = TRUE;
}

// --- fpga_end -------------------
void fpga_end()
{
#ifdef linux
	_Init = FALSE;

	if (munmap(Fpga.qsys,    sizeof(SFpgaQSys)))				Error(ERR_CONT, 0, "Could not UNMAP Fpga.stat");
	if (munmap(Fpga.stat,    sizeof(SFpgaStat)))				Error(ERR_CONT, 0, "Could not UNMAP Fpga.stat");
	if (munmap(Fpga.par,     sizeof(SFpgaPar)))					Error(ERR_CONT, 0, "Could not UNMAP Fpga.par");
	if (munmap(Fpga.move,    sizeof(SMove)*MOTOR_CNT*MOVE_CNT))	Error(ERR_CONT, 0, "Could not UNMAP Fpga.move");
	if (munmap(Fpga.encoder, sizeof(SFpgaEncoder)*ENCODER_CNT)) Error(ERR_CONT, 0, "Could not UNMAP Fpga.encoder");

	close (_MemId);
	_MemId = 0;
#endif
}

//--- fpga_connected -------------------------
void  fpga_connected(void)
{
	_ErrorCheckTime	= RX_StepperStatus.alive[0]+2;	
}

//--- fpga_deconnected --------------------------
void  fpga_deconnected(void)
{
	_ErrorCheckTime=0;		
}

//--- fpga_is_init -----------------------------
int  fpga_is_init(void)
{
	return _Init;
}

//--- fpga_input -----------------------------------
int	fpga_input(int no)
{
	return (Fpga.stat->input & (1<<no)) != 0;		
}

//--- fpga_display_status -----------------------------------------------
#define PRINTF(n)  for(i=0; i<n; i++) term_printf

void _fpga_display_status(void)
{
	int i, v;
	static UINT32 _lastPulseCnt[6];
	       UINT32 cnt[6];

	extern BYTE	_motor_start_cnt[MOTOR_CNT];

	term_printf("\n");
	term_printf("--- FPGA Status -----------------------------------\n");
	term_printf("Version: %d.%d.%d.%d\n",	Fpga.stat->version.major, Fpga.stat->version.minor,Fpga.stat->version.revision,Fpga.stat->version.build);
	term_printf("Temp:    %dC\n",			VAL_TO_TEMP(Fpga.stat->temp));
	v = VAL_TO_MV(Fpga.stat->voltage_3v3);
	term_printf("Power Supply: % 3d.%03dV(3.3)\n", v/1000, v%1000);
	v = 11*VAL_TO_MV(Fpga.stat->voltage_24v);
	term_printf("Power Supply: % 3d.%03dV(24)\n", v/1000, v%1000);
	v = VAL_TO_MV(Fpga.stat->current_24v) * 5600.0 / 680.0;
	term_printf("Power Supply: % 3d.%03dA\n", v / 1000, v % 1000);
	v = ps_get_power();
	term_printf("Power Motors: % 3d.%03dV(24)\n", v / 1000, v % 1000);
	
	term_printf("Inputs:       ");
	for (i=0; i<INPUT_CNT; i++)
	{
		if (Fpga.stat->input & (1<<i)) term_printf("*");
		else                           term_printf("_");
		if (i%4==3)			   		   term_printf("   ");
	}
	term_printf("Analog [mv]:  ");
	for (i=0; i<SIZEOF(Fpga.stat->analog_in); i++)
	{
		term_printf("%03d    ", VAL_TO_MV_AI(Fpga.stat->analog_in[i]));
	}
	term_printf("\n");
	term_printf("Outputs:      ");
	for (i=0; i<OUTPUT_CNT; i++)
	{
		if (Fpga.par->output & (1<<i))  term_printf("*");
		else                            term_printf("_");
		if (i%4==3)						term_printf("   ");
	}
	term_printf("\n");

	term_printf("PWM [%]:   ");
	for (i=0; i<SIZEOF(Fpga.par->pwm_output); i++)
	term_printf("   %03d ", (100*Fpga.par->pwm_output[i]+0x8000)/0x10000);
	term_printf("\n");
	
	term_printf("Pulse Cnt: ");
	for (i=0; i<SIZEOF(Fpga.par->pwm_output); i++)
	term_printf("%06d ", Fpga.stat->input_pulse_cnt[i]);
	term_printf("\n");
	
	term_printf("Pulse Spd: ");
	memcpy(cnt, Fpga.stat->input_pulse_cnt, sizeof(cnt));	
	for (i=0; i<SIZEOF(cnt); i++)
		term_printf("%06d ", fpga_pwm_speed(i));
	term_printf("\n");
	memcpy(_lastPulseCnt, cnt, sizeof(_lastPulseCnt));	
	
	term_printf("\n");
	term_printf("Motor:           "); PRINTF(MOTOR_CNT)("---%d---   ", i); term_printf("\n");
if (RX_StepperCfg.printerType==printer_TX801) 
	term_printf("Place:           --LF---   --LB---   --RB---   --RF---   -------\n");
	term_printf("Moving:          "); PRINTF(MOTOR_CNT)("   %d      ", ((Fpga.stat->moving & (0x01<<i))!=0));	term_printf("\n"); 
	term_printf("moveIdx:         "); PRINTF(MOTOR_CNT)("%06d    ", Fpga.stat->statMot[i].moveIdx);				term_printf("\n"); 
	term_printf("moving_cnt:      "); PRINTF(MOTOR_CNT)("%06d    ", Fpga.stat->statMot[i].moving_cnt);			term_printf("\n"); 
	term_printf("motor_start_cnt: "); PRINTF(MOTOR_CNT)("%06d    ", _motor_start_cnt[i]); 
	term_printf("\n"); 
	term_printf("stop_mux:       "); PRINTF(MOTOR_CNT)("0x%04x    ", Fpga.par->cfg[i].stop_mux);				term_printf("\n"); 
	term_printf("Pos:            "); PRINTF(MOTOR_CNT)("%06d    ", Fpga.stat->statMot[i].position);				term_printf("\n"); 
	term_printf("Pos rising:     "); PRINTF(MOTOR_CNT)("%06d    ", Fpga.stat->statMot[i].pos_rising); term_printf("\n"); 
	term_printf("Pos falling:    "); PRINTF(MOTOR_CNT)("%06d    ", Fpga.stat->statMot[i].pos_falling); term_printf("\n"); 
	term_printf("Pos end:        "); PRINTF(MOTOR_CNT)("%06d    ", motor_get_end_step(i));						term_printf("\n"); 
	term_printf("Speed [Hz]:     "); PRINTF(MOTOR_CNT)("%06d    ", Fpga.stat->statMot[i].speed/0x10000);		term_printf("\n"); 
//	term_printf("vEdge:          "); PRINTF(MOTOR_CNT)("%06d    ", Fpga.stat->statMot[i].v_edge);				term_printf("\n"); 
	term_printf("stopIn_Pos:     "); PRINTF(MOTOR_CNT)("%06d    ", Fpga.stat->statMot[i].stopIn_Pos);			term_printf("\n"); 
	term_printf("too_fast:       "); PRINTF(MOTOR_CNT)("%06d    ", Fpga.stat->statMot[i].err_too_fast);			term_printf("\n"); 
	term_printf("err_stop:       "); PRINTF(MOTOR_CNT)("%08x  ",   Fpga.stat->statMot[i].err_stop);				term_printf("\n"); 
	term_printf("err_estop:      "); PRINTF(MOTOR_CNT)("%08x  ",   Fpga.stat->statMot[i].err_estop);			term_printf("\n"); 
	term_printf("  ERR.ENC:      "); PRINTF(MOTOR_CNT)("%08x  ",   Fpga.stat->statMot[i].err_estop & ENC_ESTOP_ENC);	term_printf("\n"); 
	term_printf("  ERR.EMUX:     "); PRINTF(MOTOR_CNT)("%08x  ",   Fpga.stat->statMot[i].err_estop & ENC_ESTOP_EMUX);	term_printf("\n"); 
	term_printf("  ERR.MUX:      "); PRINTF(MOTOR_CNT)("%08x  ",   Fpga.stat->statMot[i].err_estop & ENC_ESTOP_MUX);	term_printf("\n"); 
	term_printf("  ERR.IN0:      "); PRINTF(MOTOR_CNT)("%08x  ",   Fpga.stat->statMot[i].err_estop & ENC_ESTOP_IN00);	term_printf("\n"); 
	term_printf("  ERR.IN1:      "); PRINTF(MOTOR_CNT)("%08x  ",   Fpga.stat->statMot[i].err_estop & ENC_ESTOP_IN01);	term_printf("\n"); 
	term_printf("  ERR.IN2:      "); PRINTF(MOTOR_CNT)("%08x  ",   Fpga.stat->statMot[i].err_estop & ENC_ESTOP_IN02);	term_printf("\n"); 
	term_printf("  ERR.IN3:      "); PRINTF(MOTOR_CNT)("%08x  ",   Fpga.stat->statMot[i].err_estop & ENC_ESTOP_IN03);	term_printf("\n"); 
		
	term_printf("Encoder:        "); PRINTF(MOTOR_CNT)("---%d---   ", i);										term_printf("\n");
	term_printf("Pos: inc        "); PRINTF(MOTOR_CNT)("%06d    ", Fpga.encoder[i].pos);						term_printf("\n"); 
//	term_printf("Inc/Rev:        "); PRINTF(MOTOR_CNT)("%06d    ", Fpga.encoder[i].incPerRev);					term_printf("\n"); 
//	term_printf("IdxCnt:         "); PRINTF(MOTOR_CNT)("%06d    ", Fpga.encoder[i].revCnt);						term_printf("\n"); 
	
	/*
	term_printf("pwm pulsewidth: "); PRINTF(4)("%06d    ", (UINT16)Fpga.par->pwm_output[i]); 					term_printf("\n"); 
	
	term_printf("analog thresh:  "); PRINTF(8)("%06d    ", (UINT16)Fpga.par->adc_thresh[i]); 					term_printf("\n"); 
	term_printf("analog PulseCnt:"); PRINTF(8)("%06d    ", (UINT32)Fpga.stat->adc_cnt[i]); 						term_printf("\n");
	*/
	
	term_printf("\n");
	term_flush();
}

//--- fpga_display_error --------------------------------------------------
void fpga_display_error(void)
{
	if (_Init)
	{
	}
}


//--- fpga_main ------------------------------------------
void  fpga_main(int ticks, int menu)
{
	static int led=0;
	static int _lastTicks=0;
	static int _lastCnt[6]={0};
	int i;

	if (!_Init) return;

	Fpga.par->watchdog_cnt =  WATCHDOG_CNT;
	
	//--- fill status ----------------------
	/*
	{
		int i;
		RX_StepperStatus.inputs = Fpga.stat->input;
		for (i=0; i<MAX_STEPPER_MOTORS; i++)
		{
			RX_StepperStatus.motor[i].motor_pos	= Fpga.stat->statMot[i].position;
			RX_StepperStatus.motor[i].encoder_pos = Fpga.encoder[i].pos;
			if (Fpga.stat->moving & (0x01<<i))
			{
				if(Fpga.par->mot_bwd & (0x01<<i)) RX_StepperStatus.motor[i].state = MOTOR_STATE_MOVING_FWD;
				else							  RX_StepperStatus.motor[i].state = MOTOR_STATE_MOVING_BWD;
			}
			else if (Fpga.stat->statMot[i].err_estop & ENC_ESTOP_ENC) RX_StepperStatus.motor[i].state = MOTOR_STATE_BLOCKED;
			else RX_StepperStatus.motor[i].state = MOTOR_STATE_IDLE;
		}		
	}
	*/
	
	if (ticks-_lastTicks>100)
	{
		for (i=0; i<SIZEOF(Fpga.stat->input_pulse_cnt); i++)
		{
			int cnt=Fpga.stat->input_pulse_cnt[i];
			_PWM_Speed[i] = cnt-_lastCnt[i];
			_lastCnt[i]   = cnt;
		}
		_lastTicks = ticks;
	}
	
	if (_GuiTimer && ticks>_GuiTimer) 
	{
		_GuiTimer = 0;
		Fpga.par->output  &= ~OUT_GUI_START;
	}
	
	if (menu)
	{
		//	FpgaCfg.head[0]->cmd_led = led;
		led = !led;
		RX_StepperStatus.alive[0]++;
		
		if (_ErrorCheckTime && RX_StepperStatus.alive[0]>_ErrorCheckTime)
			_check_errors();

		//--- user interface ------------------------------				
		{
//			_fpga_display_error();
			_fpga_display_status();
		}
	}
}

//--- fpga_pwm_speed --------------------------------------
int	  fpga_pwm_speed(int no)
{
	return _PWM_Speed[no];	
}

//--- fpga_output_toggle --------------------------------
void fpga_output_toggle(int no)
{
	Fpga.par->output ^= (1<<no);
}

//--- _check_errors ---------------------------------------------------------------------
static void  _check_errors(void)
{
	if (Fpga.stat->voltage_3v3!= INVALID_VALUE && _ErrorFlags==0)
	{
		if (VAL_TO_MV(Fpga.stat->voltage_3v3) < 3000)						_ErrorCnt[0]++;
		if ((VAL_TO_MV(Fpga.stat->current_24v) * 5600.0 / 680.0) > 15000)	_ErrorCnt[1]++;
		if ((VAL_TO_MV(Fpga.stat->current_24v) * 5600.0 / 680.0) > 15000)	_ErrorCnt[2]++;

		int v24=11*VAL_TO_MV(Fpga.stat->voltage_24v) / 1000;		
		if (v24 < 20) _ErrorCnt[3]++;
		else if (v24 > 22) _ErrorCnt[4]++;
		int rowTemp=Fpga.stat->temp;
		int temp=VAL_TO_TEMP(rowTemp);
		if (rowTemp!=0x0fff && temp > 70) _ErrorCnt[5]++;
	
		if (_ErrorCnt[0]==5) _ErrorFlags=Error(ERR_CONT, 0, "Voltage Error: 3.3V Level is %d V", VAL_TO_MV(Fpga.stat->voltage_3v3));
		if (_ErrorCnt[1]==5) _ErrorFlags=Error(ERR_CONT, 0, "Current Error: Current of 24V Supply is %d A", VAL_TO_MV(Fpga.stat->current_24v) * 5600.0 / 680.0);
		if (_ErrorCnt[2]==5) _ErrorFlags=Error(ERR_CONT, 0, "Current Error: Current of 24V Supply is %d A", VAL_TO_MV(Fpga.stat->current_24v) * 5600.0 / 680.0);
		if (_ErrorCnt[3]==5) _ErrorFlags=Error(WARN,     0, "Voltage Error: 24V Level is %d V", v24);
		if (_ErrorCnt[4]==5) _ErrorFlags=Error(LOG,      0, "Voltage OK: 24V Level is %d V", v24);
		if (_ErrorCnt[5]==5) _ErrorFlags=Error(ERR_CONT, 0, "Temperature Error: Temp Level is %d °C, row=%d", temp, rowTemp);	
	}	
}

//--- fpga_stepper_error_reset ----------------------------------------------------
void fpga_stepper_error_reset(void)
{
	if (_ErrorFlags)
	{
		Fpga.par->adc_rst = TRUE;
		_ErrorCheckTime	= RX_StepperStatus.alive[0]+2;		
	}

	memset(_ErrorCnt, 0, sizeof(_ErrorCnt));
	_ErrorFlags = 0;	
}