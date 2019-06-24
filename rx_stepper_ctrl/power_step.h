// ****************************************************************************
//
//	DIGITAL PRINTING - power_step.h
//
//
// ****************************************************************************
//
//	Copyright 2016 by Radex AG, Switzerland. All rights reserved.
//	Written by Marcel Galliker
//
// ****************************************************************************

#pragma once

#include "ps_common.h"
#include "rx_def.h"

typedef struct
{
	//--- Speed Profile -----------
	double	acc;		  // Register ACC:		Acceleration rate in step/s^2, range 14.55 to 59590 steps/s^2
	double	dec;		  // Register DEC:		Deceleration rate in step/s^2, range 14.55 to 59590 steps/s^2
	double  max_peed;	  // Register MAX_SPEED:	Maximum speed in step/s, range 15.25 to 15610 steps/s
	double	min_speed;	  // Register MIN_SPEED:	Minimum speed in step/s, range 0 to 976.3 steps/s
	double  fs_spd;		  // Register FS_SPD:		Full step speed in step/s, range 7.63 to 15625 steps/s
	UINT16	fs_spd_boost; // Register FS_SPD.BOOST_MODE: Boost of the amplitude square wave, enum powerstep01_BoostMode_t

	//--- Voltage mode parameters ---------------------------
	double	kval_acc;	// Register KVAL_ACC: Acceleration duty cycle (torque) in %, range 0 to 99.6%
	double	kval_dec;	// Register KVAL_DEC: Deceleration duty cycle (torque) in %, range 0 to 99.6%
	double	kval_run;	// Register KVAL_RUN: Run duty cycle (torque) in %, range 0 to 99.6%
	double	kval_hold;	// Register KVAL_HOLD: Hold duty cycle (torque) in %, range 0 to 99.6%
	UINT8	cfg_vscomp;	// Register CONFIG.EN_VSCOMP: Motor Supply Voltage Compensation enabling , enum powerstep01_ConfigEnVscomp_t
	UINT8	lspd_opt;	// Register MIN_SPEED.LSPD_OPT: Low speed optimization bit, enum powerstep01_LspdOpt_t
	double	k_therm;	// Register K_THERM:	Thermal compensation param, range 1 to 1.46875  
	double	int_speed;	// Register INT_SPEED:	Intersect speed settings for BEMF compensation in steps/s, range 0 to 3906 steps/s 
	double	st_slp;		// Register ST_SLP:		BEMF start slope settings for BEMF compensation in % step/s, range 0 to 0.4% s/step
	double	fn_slp_acc;	// Register FN_SLP_ACC:	BEMF final acc slope settings for BEMF compensation in % step/s, range 0 to 0.4% s/step
	double	fn_slp_dec;	// Register FN_SLP_DEC: BEMF final dec slope settings for BEMF compensation in % step/s, range 0 to 0.4% s/step
	UINT16	cfg_pwm_div;// Register CONFIG.F_PWM_INT: PWM Frequency Integer division, enum powerstep01_ConfigFPwmInt_t 
	UINT16	cfg_pwm_mul;// Register CONFIG.F_PWM_DEC: PWM Frequency Integer Multiplier, enum powerstep01_ConfigFPwmDec_t

	//--- Advance current control parameters ----------------------------------------
	double	tval_acc;	// Register TVAL_ACC: Acceleration torque in mV, range from 7.8mV to 1000 mV 
	double	tval_dec;	// Register TVAL_DEC: Deceleration torque in mV, range from 7.8mV to 1000 mV
	double	tval_run;	// Register TVAL_RUN: Running torque in mV, range from 7.8mV to 1000 mV
	double	tval_hold;	// Register TVAL_HOLD: Holding torque in mV, range from 7.8mV to 1000 mV 
	UINT16	cfg_en_tqreg;// Register CONFIG.EN_TQREG: External torque regulation enabling , enum powerstep01_ConfigEnTqReg_t 
	UINT16	cfg_pred_en; // Register CONFIG.PRED_EN: Predictive current enabling , enum powerstep01_ConfigPredEn_t
	double	ton_min;	// Register TON_MIN: Minimum on-time in us, range 0.5us to 64us 
	double	toff_min;	// Register TOFF_MIN:  Minimum off-time in us, range 0.5us to 64us
	UINT8	t_fast_toff_fast;   // Register T_FAST.TOFF_FAST: Maximum fast decay time , enum powerstep01_ToffFast_t
	UINT8	t_fast_fast_step;	// Register : T_FAST.FAST_STEP: Maximum fall step time , enum powerstep01_FastStep_t
	UINT16	cfg_tsw;			// Register : CONFIG.TSW: Switching period, enum powerstep01_ConfigTsw_t 

	//--- Gate Driving ---------------------------------------------------
	UINT16	gatecfg1_igate;	// Register GATECFG1.IGATE: Gate sink/source current via enum powerstep01_Igate_t 
	UINT16	cfg_vccval;		// Register CONFIG.VCCVAL: VCC Val, enum powerstep01_ConfigVccVal_t 
	UINT16	cfg_uvloval;	// Register CONFIG.UVLOVAL: UVLO Threshold via powerstep01_ConfigUvLoVal_t 
	UINT16	gatecfg1_tboost;// Register GATECFG1.TBOOST:  Duration of the overboost phase during gate turn-off via enum powerstep01_Tboost_t 
	UINT16	gatecfg1_tcc;	// Register GATECFG1.TCC: Controlled current time via enum powerstep01_Tcc_t
	UINT8	gatecfg2_tblank;// Register GATECFG2.TBLANK: Controlled current time via enum powerstep01_Tcc_t
	UINT8	gatecfg2_tdt;	// Register GATECFG2.TDT: Duration of the blanking time via enum powerstep01_TBlank_t

	//--- others -----------------------
	UINT8	ocd_th;			// Register OCD_TH: Overcurrent threshold settings via enum powerstep01_OcdTh_t / set overcurrent limit to 3.4375 A
	UINT16	cfg_oc_sd;		// Register CONFIG.OC_SD: Over current shutwdown enabling, enum powerstep01_ConfigOcSd_t
	double	stall_th;		// Register STALL_TH: Stall threshold settings in mV, range 31.25mV to 1000mV
	UINT8	alarm_en;		// Register ALARM_EN: Alarm settings via bitmap enum powerstep01_AlarmEn_t     
	UINT16	cfg_sw_mode;	// Register CONFIG.SW_MODE: External switch hard stop interrupt mode, enum powerstep01_ConfigSwMode_t 
	UINT8	step_mode;		// Register STEP_MODE.STEP_MODE: Step mode settings via enum motorStepMode_t
	UINT8	step_mode_cm_vm;// Register STEP_MODE.CM_VM: Current mode or Voltage mode via enum powerstep01_CmVm_t 
	UINT8	step_mode_sync;	// Register STEP_MODE.SYNC_MODE/SYNC_EN: Synch. Mode settings via enum powerstep01_SyncSel_t  
	UINT16	cfg_osc_clk_sel;// Register CONFIG.OSC_CLK_SEL: Clock setting , enum powerstep01_ConfigOscMgmt_t 
	UINT16	gatecfg1_wd_en; // Register GATECFG1.WD_EN: External clock watchdog, enum powerstep01_WdEn_t
} SPS_MotorCfg;

int  ps_init		 (UINT32 *pspi, SPS_MotorCfg *cfg);
int	 ps_main		 (void);
int	 ps_init_done	 (void);
int  ps_set_backwards(UINT32 *pspi, int backwards);
int	 ps_set_current	 (UINT32 *pspi, double current);
int	 ps_get_power    (void);