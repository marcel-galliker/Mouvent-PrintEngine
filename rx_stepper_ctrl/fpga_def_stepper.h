// ********************************************************void ********************
//
//	DIGITAL PRINTING - fpga_def_stepper.h
//
//	Communication between ARM processor and FPGA
//
// ****************************************************************************
//
//	Copyright 2016 by Radex AG, Switzerland. All rights reserved.
//	Written by Marcel Galliker
//
// ****************************************************************************

#pragma once

#include "rx_common.h"

//--- defines ---------------------------------------------

#define	MOTOR_CNT		5
#define MOVE_CNT		64
#define	ENCODER_CNT		5

#define SYSTEM_CLOCK	50000000 // 50 MHz
#define CYCLE_TIME		20		 // ns

#define INPUT_CNT		12
#define OUTPUT_CNT		12
#define ANALON_IN		4

//--- adddress mapping -----------------------------------------
#define ADDR_FPGA_QSYS		0xff200000

#define ADDR_FPGA_BASE		0xff210000
#define ADDR_FPGA_STAT		(ADDR_FPGA_BASE+0x0000)
#define ADDR_FPGA_PAR		(ADDR_FPGA_BASE+0x1000)
#define ADDR_FPGA_MOVES		(ADDR_FPGA_BASE+0x2000)
#define ADDR_FPGA_ENCODER	(ADDR_FPGA_BASE+0x4000)

typedef struct
{
	// ADDR_FPGA_QSYS	0xff20	0000				
	UINT32	sysID;		//		0000:
	UINT32	res_04;
	UINT32	res_08;
	UINT32	res_0c;

	UINT32	pio_sm_in;	//		0010:
	UINT32	res_14;
	UINT32	res_18;
	UINT32	res_1c;

	UINT32	pio_sm_out;	//		0020:
	UINT32	res_24;
	UINT32	res_28;
	UINT32	res_2c;
	
	UINT32	res_30;		//		0030:
	UINT32	res_34;
	UINT32	res_38;
	UINT32	res_3c;

	UINT32	pio_alarm;	//		0040:
	UINT32	res_44;
	UINT32	res_48;
	UINT32	res_4c;

	UINT32	pio_st;		//		0050:
	UINT32	res_54;
	UINT32	res_58;
	UINT32	res_5c;

	UINT32	spi_powerstep[MOTOR_CNT];	//	0060: 
//	UINT32	spi_powerstep[1];			// 0x0064
//	UINT32	spi_powerstep[2];			// 0x0068
//	UINT32	spi_powerstep[3];			// 0x006c
	
	UINT32	res_70;	//	0070: 
	UINT32	res_74;
	UINT32	res_78;
	UINT32	res_7c;

	UINT32	res_80;	//	0080: 
	UINT32	res_84;
	UINT32	res_88;
	UINT32	res_8c;
	
	UINT32	res_90;	//	0090: 
	UINT32	res_94;
	UINT32	res_98;
	UINT32	res_9c;
	
	UINT32	pio_board_revision;	//	00a0: input 4 bit
	UINT32	res_a4;
	UINT32	res_a8;
	UINT32	res_ac;
	
} SFpgaQSys;

//--- SMotStat -----------------------------------------
typedef struct
{
								// ADDR_FPGA_STAT
								// FF21 
	UINT32	moveIdx;			//      0040: [8 Bit] moveIdx/3-1 = running movement
	UINT32	position;			//      0044: [32 Bit] 
	UINT32	speed;				//      0048: [16 Bit] system clock count between steps (0==no move)
	UINT32	stopIn_Pos;			//      004C: [32 Bit] step in whitch the input got high
	UINT32	err_too_fast;		//      0050: [unused] -> ENC_STOP_OVERSPEED
	UINT32	err_stop;			//      0054: [32 Bit]
		#define ENC_STOP_BREAK	0x00000001
		#define ENC_STOP_IN00	0x00000002
		#define ENC_STOP_IN01	0x00000004
		#define ENC_STOP_IN02	0x00000008
		#define ENC_STOP_IN03	0x00000010
		#define ENC_STOP_IN04	0x00000020
		#define ENC_STOP_IN05	0x00000040
		#define ENC_STOP_IN06	0x00000080
		#define ENC_STOP_IN07	0x00000100
		#define ENC_STOP_IN08	0x00000200
		#define ENC_STOP_IN09	0x00000400
		#define ENC_STOP_IN10	0x00000800
		#define ENC_STOP_IN11	0x00001000
		#define ENC_STOP_AI00	0x00002000
		#define ENC_STOP_AI01	0x00004000
		#define ENC_STOP_AI02	0x00008000
		#define ENC_STOP_AI03	0x00010000	

	UINT32	err_estop;			//      0058: [32 Bit] 
		#define ENC_ESTOP_BREAK	 0x00000001
		#define ENC_ESTOP_ENC	 0x00000002	// encoder or watchdog
		#define ENC_ESTOP_EMUX	 0x00000004	// stopped by other motor estop
		#define ENC_ESTOP_MUX	 0x00000008	// stopped by other motor stop
		#define ENC_ESTOP_IN00	 0x00000010	// 
		#define ENC_ESTOP_IN01	 0x00000020	// 
		#define ENC_ESTOP_IN02	 0x00000040	// 
		#define ENC_ESTOP_IN03	 0x00000080	// 
		#define ENC_ESTOP_IN04	 0x00000100	// 
		#define ENC_ESTOP_IN05	 0x00000200	//
		#define ENC_ESTOP_IN06	 0x00000400	// 
		#define ENC_ESTOP_IN07	 0x00000800	// 
		#define ENC_ESTOP_IN08	 0x00001000	// 
		#define ENC_ESTOP_IN09	 0x00002000	// 
		#define ENC_ESTOP_IN10	 0x00004000	// 
		#define ENC_ESTOP_IN11	 0x00008000	// 
		#define ENC_ESTOP_AI00	 0x00010000	// 
		#define ENC_ESTOP_AI01	 0x00020000	// 
		#define ENC_ESTOP_AI02	 0x00040000	// 
		#define ENC_ESTOP_AI03	 0x00080000	// 

		#define ENC_ESTOP_OVERSPEED	0x20000000	// moving too fast

		#define ENC_ESTOP_SPEEDx 0x40000000	// move[x].speed=0
		#define ENC_ESTOP_SPEED0 0x80000000	// move[0].speed=0

	UINT32	v_edge;				//      005C: [16 Bit]
	BYTE	moving_cnt;			//      0060: [8 Bit] moving done count 
	BYTE	res_61;
	BYTE	res_62;
	BYTE	res_63;
	INT32	pos_rising;			//      0064:
	INT32	pos_falling;		//      0068:
	UINT32	pos_ctrl_out;		//      006c 				
} SMotStat;

//--- SFpgaStatus -------------------------------------
typedef struct
{
	//--- general ---------------------------
							// ADDR_FPGA_STAT
	SVersion version;
	//	UINT32	major;		// FF21 0000: [16 Bit] 
	//	UINT32	minor;		//      0004: [16 Bit] 
	//	UINT32	revision;	//      0008: [16 Bit]
	//	UINT32	build;		//      000c: [16 Bit]
	UINT32	input;			//      0010: [12 Bit]
	UINT32	analog_in[4];	//      0014: [16 Bit]
	UINT32	voltage_3v3;	//      0024: [16 Bit]	3.3V 
	UINT32	voltage_24v;	//      0028: [16 Bit]	24.0V
	UINT32	current_24v;	//      002C: [16 Bit]  24.0V Current
	UINT32	temp;			//      0030: [16 Bit]
	UINT32	moving;			//      0034: [ 1 Bit] 
	UINT32	watchdog_err;	//      0038 
	UINT32	res_3C;			//      003C
	
	//--- motor ----------------------------------
	SMotStat	statMot[MOTOR_CNT]; //	0040 // size 0x30*5=F0

	//--- analog pulse counter -----------------------------
	UINT32	adc_cnt[8];		//       130 // size 0x20
	UINT32	input_pulse_cnt[6];		// 150 // size 0x18 // Input 6 7 8 9 10 11
	UINT32	res_168;
	UINT32	res_16c;

	//--- reserved ------------------
	UINT32	res_170_200[(0x200 - 0x170) / 4];	// FF202130 - FF2021ff
} SFpgaStat;

//--- SMove -----------------------------------------
typedef struct
{
							// FF21
	UINT32	cc0_256;		//      01C0: [32 Bit] [steps/s mit 16bit after binarypoint] Clock Counter 0: number of system clocks between two steps at start of move (*256)
	UINT32	a_256;			//      01C4: [32 Bit] [steps/s/5us mit 16bit after binarypoint] Acceleration per system clock (*256)
	UINT32	steps;			//      01C8: [32 Bit] [steps per move] number of steps withthis speed
	UINT32  amp;            //      01CC: [8 Bit] [ampere reg value]
	
	//------------- pseudo code ----------------------
	//	cc_256 = cc0_256;
	//  cnt    = cc_256/256;
	//	foreach(SYSTEM_CLOCK)
	//	{ 
	//		if (steps==0) return;
	//		if (--cnt==0) 
	//		{
	//			clock_out;
	//			steps--;
	//			cnt = cc_256/256;
	//		}
	//		cc_256 += a_256;		
	//	}
} SMove;

//--- SMotCfg -----------------------------------------
typedef struct
{
	//--- stop ------------------------------------
								// FF21
	UINT32	unused_stopIn;				//       0120: [4 Bit] if (input[stopIn]==stopLevel) the motor stops with stopAcc_256 untill cc_256>stopCC_256
	UINT32	unused_stopLevel;			//       0124: [1 Bit]
	UINT32	stopAcc_256;		//       0128: [unused]	??
	UINT32	stopCC_256;			//       012c: [unused] ??

	//--- E-Stop ----------------------------------
	UINT32	estopIn;			//       0130: [4 Bit] if (input[estopIn]==estopLevel) -> E-Stop
	UINT32	estopLevel;			//       0134: [1 Bit] // !!! INVERTED !!!
	UINT32	amp_idle;				//       0138: [8 Bit] [ampere reg value]
	UINT32	amp_stop;				//       013c: [8 Bit] [ampere reg value]

	//---  encoder -------------------------------
	UINT32	enc_bwd;			//       0140: [1 Bit]
	UINT32	v_min_speed;        //       0144: [32 Bit]
	UINT32	res_148;		    //       0148: [32 Bit]

	UINT32	enc_stop_index;		//       014c: [1 Bit] aborts motor on next index

	UINT32  enc_mot_ratio;		//	     0150: [unused] ratio = motor per rev. / encoder per rev.
	UINT32  enc_rel_steps0;		//	     0154: [12 Bit] 

	//--- relative stall detection -------------------------
	UINT32  enc_rel_steps;		//	     0158: [12 Bit] Motor steps			(0=funciton OFF)
	UINT32  enc_rel_incs;		//	     015c: [12 Bit] Encoder-Increments	(0=function OFF)
								//						function: check every enc_rel_steps that encoder-inrements >= enc_rel_incs
	UINT32  reset_pos_val_mot;	//	     0160: [32 Bit] Motor Pos
	UINT32  reset_pos_val_enc;	//	     0164: [32 Bit] Encoder Pos
	UINT32  res_168;			//	     0168:
	UINT32  res_16c;			//	     016c:

	//--- moves ---------------------------------------
	UINT32	moveCnt;			//	     0170: [8 Bit]
	UINT32  stop_mux;			//	     0174: [5 Bit] choose other motors to stop also
	//--- Disable mux for sensor = 1
	UINT32  disable_mux_in;		//	     0178: [1 Bit]
	UINT32  res_17c;			//	     017c:
} SMotCfg;

//--- SFpgaPar ---------------------------------------
typedef struct
{
	//--- general ---------------------------
							// FF21
	UINT32	cmd_start;		//	    1000: [ 5 Bit] to start 5 motors
	UINT32	cmd_stop;		//       004: [ 5 Bit] to stop 5 motors
	UINT32	cmd_estop;		//       008: [ 5 Bit] to abort 5 motors
	UINT32	cmd_reset_pos;	//       00C: [ 5 Bit] reset the potition (1 bit per motor) Motor Pos und Encoder Pos
	UINT32	output;			//       010: [12 Bit]
	UINT32	watchdog_freq;	//       014: [32 Bit] Half a period lenght of the watchdog frequency in clks (50 MHz) 
	UINT32	watchdog_cnt;	//       018: [32 Bit] WRITE Clocks (50 MHz) to count down until stop of watchdog	
	UINT32	v24_enable;		//       01C: [ 1 Bit]
	UINT32	v24_standby;	//       020: [ 1 Bit]
	UINT32	mot_bwd;		//       024: [ 5 Bit] Direction of Motor movement, 5 bit for 5 motors
	UINT32	adc_rst;		//       028: resets adc
	UINT32	reset_err;		//       02C: 
	UINT32	reset_cnt;		//       030: 
	UINT32	min_in_pulse_width;	//   034: 16 bit entprellung, in multiples of 20ns
	UINT32	cmd_start_encmot;		//       038:  [ 5 Bit] to start 5 motors based on the encoder
	UINT32	res_11C;		//       03C:	
	
	//--- motor -----------------------------
	SMotCfg	cfg		[MOTOR_CNT];	// 040 + 0x0060*5 // size 0x01E0

	//--- analog pulse counter -----------------------------
	UINT32	adc_thresh[8];		//       220 // size 0x20
	UINT32	pwm_output[6];		//       240 // size 0x20 // pwm exits the highest 6 outputs  6 7 8 9 10 11
} SFpgaPar;

//--- SFpgaEncoder ---------------------------------------------
typedef struct
{
								// FF21 3000 
	UINT32	pos;				//	     000: [32 Bit] (R/W)	actual position
	UINT32	incPerRev;			//       004: [16 Bit] increments  per revolution
	UINT32  revCnt;				//       008: [ 8 Bit] (R/W)	number of revolutions
	UINT32	_pos_motor;			//       00c: [32 Bit]
	UINT32	res_10; 			//       010:		
	UINT32	res_14; 			//       014:		
	UINT32	res_18; 			//       018:		
	UINT32	res_1c; 			//       01c:		
	UINT32	res_20; 			//       020:		
	UINT32	res_24; 			//       024:		
	UINT32	res_28; 			//       028:		
	UINT32	res_2c; 			//       02c:		
} SFpgaEncoder;

//--- SFpga ----------------------------------------------
typedef struct 
{
	SFpgaQSys		*qsys;
	SFpgaStat		*stat; // 0x200
	SFpgaPar		*par;  // 0x220
	SMove			*move;		// [MOTOR_CNT][MOVE_CNT];
	SFpgaEncoder	*encoder;	// [ENCODER_CNT]
} SStepperFpga;

