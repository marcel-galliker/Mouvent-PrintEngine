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

//--- includes ----------------------------------------------------------------

#include "rx_def.h"
#include "rx_error.h"
#include "fpga_stepper.h"
#include "ps_common.h"
#include "power_step.h"


static int	   _voltage_mv;


//--- prototypes ---------------------------------

static UINT16 _ps_get_status(UINT32 *pspi);
static void   _ps_set_byte	(UINT32 *pspi, int par, int val);
static UINT8  _ps_get_byte  (UINT32 *pspi, int par);
static void	  _ps_set_int16	(UINT32 *pspi, int par, int val);
static UINT16 _ps_get_uint16(UINT32 *pspi, int par);

static SPS_MotorCfg cfg_motor = 
{
	//--- Speed Profile -----------
	.acc			= 10,		
	.dec			= 10,		
	.max_peed		= 20,	
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
	.tval_acc		= 50, //328.12,	
	.tval_dec		= 50, //328.12,	
	.tval_run		= 50, //328.12,	
	.tval_hold		= 5, //328.12,	
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
	.ocd_th			= POWERSTEP01_OCD_TH_31_25mV, //POWERSTEP01_OCD_TH_281_25mV,			
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

//--- ps_main ---------------------------
int	 ps_main(void)
{
#define MIN_VOLTAGE 10000
	UINT8 adc_out = _ps_get_byte(&Fpga.qsys->spi_powerstep[0], POWERSTEP01_ADC_OUT);	
	int old=_voltage_mv;
	_voltage_mv = (adc_out*3300)*(56+2)/2.0/32.0;
	if (old>=MIN_VOLTAGE && _voltage_mv<MIN_VOLTAGE) return REPLY_ERROR;
	return REPLY_OK;
}

//--- ps_init ----------------------------------------------
int ps_init(UINT32 *pspi, SPS_MotorCfg *pcfg)
{	
	SPS_MotorCfg *cfg;
	if (pcfg == NULL) cfg = &cfg_motor;
	else cfg = pcfg;
	
	_ps_set_byte (pspi, POWERSTEP01_STEP_MODE,	cfg->step_mode_sync
													  | cfg->step_mode_cm_vm 
													  | cfg->step_mode);

	if (cfg->step_mode_cm_vm==POWERSTEP01_CM_VM_VOLTAGE)
	{
		_ps_set_int16(pspi, POWERSTEP01_CONFIG, cfg->cfg_pwm_div
														| cfg->cfg_pwm_mul
														| cfg->cfg_vccval
														| cfg->cfg_uvloval
														| cfg->cfg_oc_sd
														| cfg->cfg_vscomp
														| cfg->cfg_sw_mode
														| cfg->cfg_osc_clk_sel);
		
		_ps_set_byte (pspi, POWERSTEP01_STALL_TH,	StallTh_to_Par(cfg->stall_th));
		
		_ps_set_byte (pspi, POWERSTEP01_K_THERM,	KTherm_to_Par(cfg->k_therm));

		_ps_set_byte (pspi, POWERSTEP01_KVAL_HOLD,	Kval_Perc_to_Par(cfg->kval_hold));
		_ps_set_byte (pspi, POWERSTEP01_KVAL_RUN,	Kval_Perc_to_Par(cfg->kval_run));
		_ps_set_byte (pspi, POWERSTEP01_KVAL_ACC,	Kval_Perc_to_Par(cfg->kval_acc));
		_ps_set_byte (pspi, POWERSTEP01_KVAL_DEC,	Kval_Perc_to_Par(cfg->kval_dec));

		_ps_set_int16(pspi, POWERSTEP01_INT_SPD,	IntSpd_Steps_to_Par(cfg->int_speed));
		_ps_set_byte (pspi, POWERSTEP01_ST_SLP,		BEMF_Slope_Perc_to_Par(cfg->st_slp));	
		_ps_set_byte (pspi, POWERSTEP01_FN_SLP_ACC,	BEMF_Slope_Perc_to_Par(cfg->fn_slp_acc));	
		_ps_set_byte (pspi, POWERSTEP01_FN_SLP_DEC,	BEMF_Slope_Perc_to_Par(cfg->fn_slp_dec));	
	}
	else
	{
		// Set Config Current Mode
		_ps_set_int16(pspi, POWERSTEP01_CONFIG, cfg->cfg_pred_en
														| cfg->cfg_tsw
														| cfg->cfg_vccval
														| cfg->cfg_uvloval
														| cfg->cfg_oc_sd
														| cfg->cfg_en_tqreg
														| cfg->cfg_sw_mode
														| cfg->cfg_osc_clk_sel);

		_ps_set_byte (pspi, POWERSTEP01_TOFF_MIN,	Tmin_Time_to_Par(cfg->toff_min));
		_ps_set_byte (pspi, POWERSTEP01_TON_MIN,	Tmin_Time_to_Par(cfg->ton_min));
		_ps_set_byte (pspi, POWERSTEP01_T_FAST,		cfg->t_fast_toff_fast
												  | cfg->t_fast_fast_step);

		_ps_set_byte (pspi, POWERSTEP01_TVAL_HOLD,	Tval_Current_to_Par(cfg->tval_hold));
		_ps_set_byte (pspi, POWERSTEP01_TVAL_RUN,	Tval_Current_to_Par(cfg->tval_run));
		_ps_set_byte (pspi, POWERSTEP01_TVAL_ACC,	Tval_Current_to_Par(cfg->tval_acc));
		_ps_set_byte (pspi, POWERSTEP01_TVAL_DEC,	Tval_Current_to_Par(cfg->tval_dec));
	}

	// Set GateCFG1 (Has to be written at start-up)
	_ps_set_int16(pspi, POWERSTEP01_GATECFG1,	cfg->gatecfg1_wd_en
											  | cfg->gatecfg1_tboost
											  | cfg->gatecfg1_igate
											  | cfg->gatecfg1_tcc);
	_ps_set_byte (pspi, POWERSTEP01_GATECFG2,	cfg->gatecfg2_tblank
											  | cfg->gatecfg2_tdt);
	_ps_set_byte (pspi, POWERSTEP01_ALARM_EN,   cfg->alarm_en);
	_ps_set_byte (pspi, POWERSTEP01_OCD_TH,		cfg->ocd_th);

	// Set Speed Profile
	_ps_set_int16(pspi, POWERSTEP01_ACC,		AccDec_Steps_to_Par(cfg->acc));	
	_ps_set_int16(pspi, POWERSTEP01_DEC,		AccDec_Steps_to_Par(cfg->dec));
	_ps_set_int16(pspi, POWERSTEP01_MAX_SPEED,	MaxSpd_Steps_to_Par(cfg->max_peed));
	_ps_set_int16(pspi, POWERSTEP01_MIN_SPEED,	MinSpd_Steps_to_Par(cfg->min_speed));
	_ps_set_int16(pspi, POWERSTEP01_FS_SPD,		cfg->fs_spd_boost
												| FSSpd_Steps_to_Par(cfg->fs_spd));

	// Read config, reset error flags
	UINT16 res=_ps_get_status(pspi);
	return REPLY_OK;
}

//--- ps_set_current --------------------------------------
int	 ps_set_current	 (UINT32 *pspi, double current)
{
	_ps_set_byte (pspi, POWERSTEP01_TVAL_HOLD,	Tval_Current_to_Par(current));
	_ps_set_byte (pspi, POWERSTEP01_TVAL_RUN,	Tval_Current_to_Par(current));
	_ps_set_byte (pspi, POWERSTEP01_TVAL_ACC,	Tval_Current_to_Par(current));
	_ps_set_byte (pspi, POWERSTEP01_TVAL_DEC,	Tval_Current_to_Par(current));			
	return REPLY_OK;
}

//--- ps_set_backwards -------------------------------
int ps_set_backwards(UINT32 *pspi, int backwards)
{
	if (backwards) *pspi = POWERSTEP01_STEP_CLK_BW;
	else           *pspi = POWERSTEP01_STEP_CLK_FW;
	return REPLY_OK;
}

//--- _ps_set_byte ----------------------------------------
static void _ps_set_byte(UINT32 *pspi, int par, int val)
{
	*pspi = POWERSTEP01_SET_PARAM | par;
	*pspi = val;
}

//--- _ps_set_int16 -----------------------------------------
static void _ps_set_int16(UINT32 *pspi, int par, int val)
{
	*pspi = POWERSTEP01_SET_PARAM | par;
	*pspi = (val>>8) & 0xff;
	*pspi = val      & 0xff;
	
	//--- check --- 
	*pspi = POWERSTEP01_GET_PARAM | par;
	BYTE b0 = *pspi;
	BYTE b1 = *pspi;
	b1=b1;
}

//--- _ps_get_byte -----------------------------------------
static UINT8 _ps_get_byte(UINT32 *pspi, int par)
{
	*pspi = POWERSTEP01_GET_PARAM | par;
	return (UINT8)((*pspi) & 0xff);
}

//--- _ps_get_uint16 -----------------------------------------
static UINT16 _ps_get_uint16(UINT32 *pspi, int par)
{
	*pspi = POWERSTEP01_GET_PARAM | par;
	return (UINT16) ((*pspi) & 0xff) << 8 | ((*pspi) & 0xff);
}

//--- ps_get_power -----------------------------------
int ps_get_power(void)
{
	return _voltage_mv;
}
	
//--- _ps_get_status ----------------------
static UINT16 _ps_get_status(UINT32 *pspi)
{
	return _ps_get_uint16(pspi, POWERSTEP01_STATUS);
}


