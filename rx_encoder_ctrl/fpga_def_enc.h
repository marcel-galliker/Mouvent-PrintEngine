// ********************************************************void ********************
//
//	DIGITAL PRINTING - fpga_def_enc.h
//
//	Communication between ARM processor and FPGA
//
// ****************************************************************************
//
//	Copyright 2015 by Radex AG, Switzerland. All rights reserved.
//	Written by Marcel Galliker
//
// ****************************************************************************

#pragma once

#include "rx_common.h"
#include "rx_def.h"

#define ENCODER_IN_CNT	8
#define ENCODER_OUT_CNT	8

//--- TEST Value Arrays ---------------------------------------
#define TEST_VALUE_CNT			8192		// UINT32
#define TEST_VALUE_ADDR_0		0xc0210000
#define TEST_VALUE_ADDR_1		0xc0230000
#define TEST_VALUE_ADDR_2		0xc0250000
#define TEST_VALUE_ADDR_3		0xc0270000

#define ENC_CORR_VALUES_0		0xc0201000	//	UINT32[0x1000]:	Correction Values of Encoder[0]
#define ENC_CORR_VALUES_1		0xc0202000	//	UINT32[0x1000]:	Correction Values of Encoder[1]

#define SINUS_CORRECTION		0xc0290000	//	UINT32[0x10000]:	SIN (Increments_rev/4)
#define SINUS_IDENTIFICATION	0xc02D0000	//	UINT32[0x10000]:	SIN

//--- SDriveErr ------------------------------------
typedef struct
{
	UINT32	chanel_a;			// 0x0000:
	UINT32	chanel_b;			// 0x0004:
	UINT32  chanel_i;			// 0x0008:
	UINT32  chanel_d;			// 0x000c:
} SDriveErr;

//--- SEncErrStatus -----------------------------------------
typedef struct
{
	UINT32	enc_0;					// 0x0600:
	UINT32	enc_1;					// 0x0604:
	UINT32	rol_0;					// 0x0608;
	UINT32	rol_1;					// 0x060c:
	UINT32	input_0;				// 0x0610;
	UINT32	input_1;				// 0x0614:
	UINT32	r_output_0;				// 0x0618;
	UINT32	r_output_1;				// 0x061c:
	UINT32	r_corr_in_past_cnt_0;	// 0x0620;
	UINT32	r_corr_in_past_cnt_1;	// 0x0624;
} SEncErrStatus;

//--- SEncInStatus ----------------------------------------
typedef struct
{
	UINT32	rev_sum;			// 0x0000: Encoder Steps per revolution (inc_revolution wrap)
	UINT32	i_to_a;				// 0x0004: Time between Index pulse and next edge in signal A
	UINT32	i_to_b;				// 0x0008: Time between Index pulse and next edge in signal B
	UINT32	position;			// 0x000c: Actual position of the encoder (32bit wrap)	
	UINT32	step_time;			// 0x0010: Actual step time
	UINT32	setp_time_min;		// 0x0014: Minimum step time (can be reset)
	UINT32	step_time_max;		// 0x0018: Maximum step time (can be reset)
	UINT32	ident_obs_b1;		// 0x001c
	UINT32	ident_obs_a1;		// 0x0020
	UINT32	enc_diff;			// 0x0024
	UINT32	position_rev;		// 0x0028
	UINT32	enc_diff_min;		// 0x002c
	UINT32	enc_diff_max;		// 0x0030
	UINT32	enc_diff_overflow;	// 0x0034
	UINT32	pg_start_pos;		// 0x0038 in encoder input steps
	UINT32	res_3c;				// 0x003c
 } SEncInStatus;

//--- SEncOutStatus -----------------------------------------
typedef struct
{
	UINT32	position;		// 0x0000:	// 20 Bit!
	UINT32	speed;			// 0x0004:
	UINT32	speed_min;		// 0x0008:	// * 50000000/2^31    OR *23/1000	
	UINT32	speed_max;		// 0x000c:
	UINT32	PG_cnt;			// 0x0010	// 10 bit!
	UINT32	res_14;			// 0x0014
	UINT32	res_18;			// 0x0018
	UINT32	res_1c;			// 0x001c
	UINT32	res_20;			// 0x0020
	UINT32	res_24;			// 0x0024
	UINT32	res_28;			// 0x0028
	UINT32	res_2c;			// 0x002c
	UINT32	res_30;			// 0x0030
	UINT32	res_34;			// 0x0034
	UINT32	res_38;			// 0x0038
	UINT32	res_3c;			// 0x003c
} SEncOutStatus;

//--- SEncStatus ----------------------------------------------
typedef struct
{
	SEncInStatus	encIn[8];					// 0x0000
	SEncOutStatus	encOut[8];					// 0x0200
	UINT32			info;						// 0x0400:
	UINT32			error;						// 0x0404:
	UINT32			statistics[8];				// 0x0408
	UINT32			msg_cnt;					// 0x0428:
	UINT32			msg_cnt_reset;				// 0x042c:
	SVersion		version;					// 0x0430:
    UINT32			mem_pointer[4];				// 0x0440: mem_pointer[0]
//  UINT32			mem_pointer_1;				// 0x0444: mem_pointer[1]
//  UINT32			mem_pointer_2;				// 0x0448: mem_pointer[2]	
//  UINT32			mem_pointer_3;				// 0x044c: mem_pointer[3]	
	UINT32			in_pulse_cnt;				// 0x0450: read after every scan
	UINT32			out_pulse_cnt;				// 0x0454: read after every scan
	UINT32			in_end_pos;					// 0x0458: read after every scan
	UINT32			out_end_pos;				// 0x045C: read after every scan
	UINT32			stroke_enc_pulse_cnt;		// 0x0460: read after every scan
	UINT32			stroke_enc_end_pos;			// 0x0464: read after every scan
	UINT32			stroke_sum_end_pos;			// 0x0468: read after every scan
	UINT32			dir_change_cnt[2];			// 0x046C: read after every scan
//	UINT32			dir_change_cnt_1;			// 0x0470: read after every scan
	UINT32			ab_change_error[4];			// 0x0474: read after every scan
//	UINT32			ab_change_error_1;			// 0x0478: read after every scan
//	UINT32			ab_change_error_2;			// 0x047C: read after every scan
//	UINT32			ab_change_error_3;			// 0x0480: read after every scan
	UINT32			rol_coeff_at_use[4];		// 0x0484: coeff a0 sin rol 0
//	UINT32			rol_coeff_at_use_1;			// 0x0488: coeff a1 cos rol 0
//	UINT32			rol_coeff_at_use_2;			// 0x048C: coeff a0 sin rol 1
//	UINT32			rol_coeff_at_use_3;			// 0x0490: coeff a1 cos rol 1
	SDriveErr 		drive_err[4];				// 0x0494: 
//	SDriveErr 		drive_err_1;				// 0x04a4: 
//	SDriveErr 		drive_err_2;				// 0x04b4: 
//	SDriveErr 		drive_err_3;				// 0x04c4: 
	UINT32 		    dig_pg_window_err[8];		// 0x04d4
//	UINT32 		    dig_pg_window_err_1;		// 0x04d8
//	UINT32 		    dig_pg_window_err_2;		// 0x04dc
//	UINT32 		    dig_pg_window_err_3;		// 0x04e0
//	UINT32 		    dig_pg_window_err_4;		// 0x04e4
//	UINT32 		    dig_pg_window_err_5;		// 0x04e8
//	UINT32 		    dig_pg_window_err_6;		// 0x04ec
//	UINT32 		    dig_pg_window_err_7;		// 0x04f0
	UINT32			drift_w_coeff;				// 0x04f4: 
	UINT32			rol_corr_pos_0;				// 0x04f8: 
	UINT32			rol_corr_pos_1; 			// 0x04fC: 
	UINT32			rol_flags_0;				// 0x0500: 
	UINT32			rol_flags_1;				// 0x0504: 
	UINT32			rev_sums_long_0;			// 0x0508: 
	UINT32			rev_sums_long_1;			// 0x050C: 
	UINT32			ramp_value_0;				// 0x0510: 
	UINT32			ramp_value_1;				// 0x0514:
	UINT32			corr_out_fill_level_0;		// 0x0518: 
	UINT32			corr_out_fill_level_1;		// 0x051C: 
	UINT32			corr_out_delays_busy_0;		// 0x0520: 
	UINT32			corr_out_delays_busy_1;		// 0x0524: 
	UINT32			corr_out_delays_err_0;		// 0x0528: 
	UINT32			corr_out_delays_err_1;		// 0x052C:
	UINT32			curr_ratio_0;				// 0x0530: 
	UINT32			curr_ratio_1;				// 0x0534: 
	UINT32			corr_out_fill_level_0_max;	// 0x0538: 
	UINT32			corr_out_fill_level_0_min;	// 0x053c: 	
	UINT32 		    pg_fifo_empty_err;			// 0x0540
	UINT32 		    ignored_fifo_empty_err;		// 0x0544
	UINT32 		    window_fifo_empty_err;		// 0x0548
	UINT32 		    rolcor_0_err_vec;			// 0x054C // reset_min_max
	//(0)r_corr_frame_err.r_int.r_enc_error_too_slow;
	//(1)r_corr_frame_err.r_int.r_enc_error_too_fast;
	//(2)r_corr_frame_err.r_int.r_enc_error_count_less;
	//(3)r_corr_frame_err.r_int.r_enc_error_count_more;
	//(4)r_corr_frame_err.r_int.r_enc_error_count_back;
	//(10)r_corr_frame_err.r_rol.r_delays_busy;
	//(11)r_corr_frame_err.r_rol.r_delay_busy_err;
	//(12)r_corr_frame_err.r_rol.r_pos_cnt_overflow;
	//(13)r_corr_frame_err.r_rol.r_sin_mul_overflow;
	//(14)r_corr_frame_err.r_rol.r_prevalue_sin_corr_high_overflow;
	//(15)r_corr_frame_err.r_rol.r_prevalue_sin_corr_low_overflow;
	//(16)r_corr_frame_err.r_ident.r_ident_sin_sum_overflow;
	//(17)r_corr_frame_err.r_ident.r_ident_cos_sum_overflow;
	//(18)r_corr_frame_err.r_ident.r_int_diff_add_overflow;
	//(19)r_corr_frame_err.r_ident.r_sin_af_mul_overflow;
	//(20)r_corr_frame_err.r_ident.r_cos_af_mul_overflow;
	//(21)r_corr_frame_err.r_ident.r_ident_sin_add_overflow;
	UINT32 		    rolcor_1_err_vec;			// 0x0550 // reset_min_max
	UINT32 		    rolcor_0_delay_busy_max;	// 0x0554 // reset_min_max
	UINT32 		    str_corr;	// 0x0558 // reset_min_max
	UINT32 		    str_corr_max;	// 0x055C // reset_min_max
	UINT32 		    str_corr_min;	// 0x0560 // reset_min_max
	UINT32 		    max_0_b1_ident_b1;	// 0x0564 // reset_min_max
	UINT32 		    max_0_b1_ident_a1;	// 0x0568 // reset_min_max
	UINT32 		    max_0_a1_ident_b1;	// 0x056C // reset_min_max
	UINT32 		    max_0_a1_ident_a1;	// 0x0570 // reset_min_max
	UINT32 		    max_1_b1_ident_b1;	// 0x0574 // reset_min_max
	UINT32 		    max_1_b1_ident_a1;	// 0x0578 // reset_min_max
	UINT32 		    max_1_a1_ident_b1;	// 0x057C // reset_min_max
	UINT32 		    max_1_a1_ident_a1;	// 0x0580 // reset_min_max
	UINT32 		    dig_in_cnt;			// 0x0584 
	UINT32 		    dig_cnt_stat;		// 0x0588
	UINT32 		    res_058c;			// 0x058c
	UINT32 		    shift_delay_cnt;	// 0x0590 not used, only of first counter for debuging
	UINT32			res[(0x0600 - 0x0594) / 4];	// 0x054c .. 0x0600
	SEncErrStatus		err;					// 0x0600 - 0x0624: 
	UINT32			res2[(0x0800 - 0x0628) / 4];// 0x0628 .. 0x0800
} SEncFpgaStatus;

//--- SEncInCfg ----------------------------------------------------
typedef struct
{
	UINT32	enable;				// 0x0000:
	UINT32	reset_min_max;		// 0x0004:
	UINT32	orientation;		// 0x0008:
	UINT32	index_on_b;			// 0x000c:
	UINT32	inc_per_revolution;	// 0x0010:
	UINT32	correction;			// 0x0014: 0x04: Linear correction (Textile Slide)
			#define CORR_OFF		0x00
			#define CORR_ENCODER	0x01	// 32 values
			#define CORR_ROTATIVE	0x02	// 
			#define CORR_LINEAR		0x04	// using two encoders (scanning)
	UINT32	index_en;			// 0x0018: Index enable
	UINT32	reset_pos;			// 0x001c: position at reset
	UINT32	res_20;				// 0x0020:
	UINT32	res_24;				// 0x0024:
	UINT32	res_28;				// 0x0028:
	UINT32	res_2c;				// 0x002c:
	UINT32	res_30;				// 0x0030:
	UINT32	res_34;				// 0x0034:
	UINT32	res_38;				// 0x0038:
	UINT32	res_3c;				// 0x003c:
} SEncInCfg;

//--- SEncOutCfg -------------------------------------
typedef struct
{
	UINT32	encoder_no;			// 0x0000:
	UINT32	reset_min_max;		// 0x0004:
	UINT32	dist_ratio;			// 0x0008: Ratio of: (stroke step size)/(encoder step size); 31 digits after the binary point
	UINT32	synthetic_freq;		// 0x000c: Defines the frequency of the synthetic encoder.; Min: 1 Hz; Max: 200 KHz
	UINT32	backlash;			// 0x0010:
	UINT32	scanning;			// 0x0014:
	UINT32	res_18;	            // 0x0018:
	UINT32	res_1c;				// 0x001c:
	UINT32	res_20;				// 0x0020:
	UINT32	res_24;				// 0x0024:
	UINT32	res_28;				// 0x0028:
	UINT32	res_2c;				// 0x002c:
	UINT32	res_30;				// 0x0030:
	UINT32	res_34;				// 0x0034:
	UINT32	res_38;				// 0x0038:
	UINT32	res_3c;				// 0x003c:
} SEncOutCfg;

//--- SPrintGoCfg ------------------------------------
typedef struct
{
	UINT32	reset_pos;			// 0x0000:
	UINT32	enc_start_pos_fwd;	// 0x0004:
	UINT32  enc_start_pos_bwd;	// 0x0008:
	UINT32	fifos_ready;		// 0x000c:
	UINT32	pos_pg_fwd;			// 0x0010:
	UINT32  pos_pg_bwd;			// 0x0014:
	UINT32	printgo_n;			// 0x0018:
	UINT32	fifos_used;			// 0x001c: 2 Bit: 0=OFF, 1=Distance, 2=MarkReader
		#define FIFOS_OFF			0
		#define FIFOS_DIST			1
		#define FIFOS_MARKREADER	2
	
	UINT32	dig_in_sel;			// 0x0020: select digital input 2bit
	UINT32	quiet_window;		// 0x0024:
	UINT32	res_28;			    // 0x0028:
	UINT32	res_2c;			    // 0x002c:
	UINT32	res_30;				// 0x0030: select digital input 2bit
	UINT32	res_34;				// 0x0034:
	UINT32	res_38;			    // 0x0038:
	UINT32	res_3c;			    // 0x003c
} SPrintGoCfg;

//--- SGeneralCfg ------------------------------------
typedef struct
{
	UINT32	watchdog_freq;			// 0x0000:
	UINT32	watchdog_cnt;			// 0x0004:
	UINT32	reset_fifos;			// 0x0008:
									// 			--   8 --   16 --   32 --   64 --   128 --   256 --   512 --  1024 --  2048 --   4096 --   8192
	UINT32	avr_coeff_0_unused;		// 0x000c:	-- 0x8 -- 0x10 -- 0x20 -- 0x40 --  0x80 -- 0x100 -- 0x200 -- 0x400 -- 0x800 -- 0x1000 -- 0x2000
	UINT32	avr_coeff_1_unused;		// 0x0010:	-- 0x6 -- 0x08 -- 0x0A -- 0x0C --  0x0E -- 0x010 -- 0x012 -- 0x014 -- 0x016 -- 0x0018 -- 0x001A
	UINT32  avr_coeff_2_unused;		// 0x0014:	-- 0x8 -- 0x10 -- 0x20 -- 0x40 --  0x80 -- 0x100 -- 0x200 -- 0x400 -- 0x800 -- 0x1000 -- 0x2000
	UINT32	ident_rol_en;			// 0x0018: enables coeff. identification logic in roller correction
	UINT32	reset_errors;					// 0x001c:
	UINT32	max_a0_var_high;		// 0x0020: Threshold for the variation value, when to switch off correction
	UINT32	max_a0_var_low;			// 0x0024: Threshold for the variation value, when to switch on correction
	UINT32	rol_0_new_b1;			// 0x0028: a0 sin coeff for roller 0 correction, overwrites internal coeff ! -- s.16.15 -- if single_sin_en=1 => amp -- s.16.15
	UINT32	rol_0_new_a1;			// 0x002c: a1 cos coeff for roller 0 correction, overwrites internal coeff ! -- s.16.15 -- if single_sin_en=1 => phase -- in steps per rev
	UINT32	rol_1_new_b1;			// 0x0030: a0 sin coeff for roller 1 correction, overwrites internal coeff ! -- s.16.15 -- if single_sin_en=1 => amp -- s.16.15
	UINT32	rol_1_new_a1;			// 0x0034: a1 cos coeff for roller 1 correction, overwrites internal coeff ! -- s.16.15 -- if single_sin_en=1 => phase -- sin steps per rev
	UINT32	rol_ident_res_shift;	// 0x0038: "000" for 2'000 steps per rev, "111" for 238'000 steps per rev
	UINT32	res_3c;					// 0x003c: 

	UINT32	single_sin_en;			// 0x0040: flag
	UINT32	use_internal_ident_en;	// 0x0044: flag
	UINT32	clear_max_coeff;		// 0x0048: flag

	UINT32	rol_2_first;			// 0x004C: flag
	UINT32	rol_drift_mu_two;		// 0x0050:

	UINT32	subsample_meas;			// 0x0054:
	
	UINT32  shift_delay;            // 0x0058 in strokes (21um strokes)
	
	UINT32  sel_roller_dia_offset[2]; // 0x005c: 4 bit: 70 + sel_roller_dia_offset_0 = Roller Diameter in mm
} SGeneralCfg;

typedef struct
{
	SEncInCfg	encIn[ENCODER_IN_CNT];		// 0x0000
	SEncOutCfg	encOut[ENCODER_OUT_CNT];	// 0x0200
	SPrintGoCfg	pg[ENCODER_OUT_CNT];		// 0x0400
	SGeneralCfg	general;					// 0x0600	0x0d00
} SEncFpgaConfig;


typedef struct	// 0x3400
{
	UINT32	par;	// linear corr
	UINT32	res_04;
	UINT32	res_08;
	UINT32	res_0c;
	UINT32	res_10;
	UINT32	res_14;
	UINT32	res_18;
	UINT32	res_1c;
	UINT32	res_20;				// 0x0020:
	UINT32	res_24;				// 0x0024:
	UINT32	res_28;				// 0x0028:
	UINT32	res_2c;				// 0x002c:
	UINT32	res_30;				// 0x0030:
	UINT32	res_34;				// 0x0034:
	UINT32	res_38;				// 0x0038:
	UINT32	res_3c;				// 0x003c:
} SEncFpgaCorrPar;

typedef struct	// 0x3000
{
	UINT32 res[0x100];
	SEncFpgaCorrPar par[ENCODER_OUT_CNT];	// 0x3400 
} SEncFpgaCorr;

//--- SEncFpga ---------------------------------------
typedef struct
{
	SEncFpgaStatus	stat; // 0x000
	SEncFpgaConfig	cfg;  // 0x800
} SEncFpga;

typedef struct
{
	UINT32	fill_level;				//  0x00
	UINT32	i_status;				//  0x04
	UINT32	event;					//  0x08
	UINT32	interrupt_enable;		//  0x0c
	UINT32	almost_full;			//  0x10
	UINT32	almost_empty;			//	0x14
	UINT32	res_18;					//  0x18
	UINT32	res_1c;					//  0x1c
} SFifoStats;

//--- SEcoderFpgaMem -------------------------------------------
typedef struct
{
	// base:				0xff20 0000
	UINT32	sysid_qsys;		//	 0x0000
	UINT32	res_04;			//	 0x0004
	UINT32	window_fifo;	//	 0x0008
	UINT32	ignored_fifo;	//	 0x000c
	UINT32	in;				//	 0x0010
	UINT32	res_14;			//	 0x0014
	UINT32	res_18;			//	 0x0018
	UINT32	res_1c;			//	 0x001c	
	UINT32	out;			//	 0x0020
	
	UINT32	res_24_40[(0x40-0x24)/4];

	UINT32	out_status;			//	 0x0040

	UINT32	res_44_60[(0x60-0x44)/4];

	SFifoStats	window_status;	//	0x0060	
	SFifoStats	ignored_status;	//	0x0080
	SFifoStats	printGo_status;	//	0x00a0
	UINT32	printGo_fifo;		//	0x00c0
	UINT32	pio_board_revision;	//	0x00d0 input 4bit
	UINT32	pio_enc_alarm_d_in;	//	0x00e0 input 4bit
	UINT32	pio_enc_alarm_in;	//	0x00f0 input 12bit	
} SEncFpgaQSys;
