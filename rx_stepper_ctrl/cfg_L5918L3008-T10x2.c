// ****************************************************************************
//
//	DIGITAL PRINTING - motor.c
//
//
// ****************************************************************************
//
//	Copyright 2016 by Radex AG, Switzerland. All rights reserved.
//	Written by Marcel Galliker
//
// ****************************************************************************

#include "ps_common.h"
#include "power_step.h"

SPS_MotorCfg cfg_L5918L3008_T10x2 = 
{
	//--- Speed Profile -----------
	.acc			= 50,		
	.dec			= 50,		
	.max_peed		= 70,	
	.min_speed		= 0,	
	.fs_spd			= 244.16,		
	.fs_spd_boost	= POWERSTEP01_BOOST_MODE_OFF, 
	.kval_acc		= 16.02,	
	.kval_dec		= 16.02,	
	.kval_run		= 16.02,	
	.kval_hold		= 16.02,	
	.cfg_vscomp		= POWERSTEP01_CONFIG_VS_COMP_DISABLE,	
	.lspd_opt		= POWERSTEP01_LSPD_OPT_OFF,			
	.k_therm		= 1,									
	.int_speed		= 61.512,							
	.st_slp			= 0.03815,							
	.fn_slp_acc		= 0.06256,						
	.fn_slp_dec		= 0.06256,						
	.cfg_pwm_div	= POWERSTEP01_CONFIG_PWM_DIV_2,	
	.cfg_pwm_mul	= POWERSTEP01_CONFIG_PWM_MUL_1,	
	
	//--- Advance current control parameters ----------------------------------------
	.tval_acc		= 328.12,	
	.tval_dec		= 328.12,	
	.tval_run		= 328.12,	
	.tval_hold		= 100.0, //328.12,	
	.cfg_en_tqreg	= POWERSTEP01_CONFIG_TQ_REG_TVAL_USED,
	.cfg_pred_en	= POWERSTEP01_CONFIG_PRED_DISABLE,
	.ton_min		= 3.0,	
	.toff_min		= 21.0,	
	.t_fast_toff_fast=POWERSTEP01_TOFF_FAST_8us,  
	.t_fast_fast_step=POWERSTEP01_FAST_STEP_12us,
	.cfg_tsw		= POWERSTEP01_CONFIG_TSW_048us,		

	//--- Gate Driving ---------------------------------------------------
	.gatecfg1_igate	= POWERSTEP01_IGATE_64mA,		
	.cfg_vccval		= POWERSTEP01_CONFIG_VCCVAL_15V,		
	.cfg_uvloval	= POWERSTEP01_CONFIG_UVLOVAL_LOW,	
	.gatecfg1_tboost= POWERSTEP01_TBOOST_0ns, 
	.gatecfg1_tcc	= POWERSTEP01_TCC_500ns,	
	.gatecfg2_tblank= POWERSTEP01_TBLANK_375ns,
	.gatecfg2_tdt	= POWERSTEP01_TDT_125ns,	

	//--- others -----------------------
	.ocd_th			= POWERSTEP01_OCD_TH_343_75mV,			
	.cfg_oc_sd		= POWERSTEP01_CONFIG_OC_SD_DISABLE,	
	.stall_th		= 531.25,		
	.alarm_en		= POWERSTEP01_ALARM_EN_OVERCURRENT | 
					  POWERSTEP01_ALARM_EN_THERMAL_SHUTDOWN | 
					  POWERSTEP01_ALARM_EN_THERMAL_WARNING | 
					  POWERSTEP01_ALARM_EN_UVLO | 
					  POWERSTEP01_ALARM_EN_STALL_DETECTION |
					  POWERSTEP01_ALARM_EN_SW_TURN_ON |
					  POWERSTEP01_ALARM_EN_WRONG_NPERF_CMD,		
	.cfg_sw_mode	= POWERSTEP01_CONFIG_SW_HARD_STOP,	
	.step_mode		= STEP_MODE_1_16,
	.step_mode_cm_vm= POWERSTEP01_CM_VM_CURRENT,
	.step_mode_sync	= POWERSTEP01_SYNC_SEL_8,
	.cfg_osc_clk_sel= POWERSTEP01_CONFIG_INT_16MHZ,
	.gatecfg1_wd_en	= POWERSTEP01_WD_EN_DISABLE,
};