#include <atmel_start.h>
#include <type_defs.h>
#include "rx_trace.h"
#include "power_step.h"
#include "power_step_def.h"
#include "ps_common.h"
#include "type_defs.h"
#include "spi.h"
#include "communication.h"
#include "string.h"
#include "stdlib.h"

#define MIN_VOLTAGE 10000

static int	_init_done=0;

static SMotorStatus			_motorStatus;
static SMotorReference		_motorReference;
static SetupAssist_MoveCmd	_moveCmd;
static int					_moveTries=0;

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
	//.cfg_sw_mode	= POWERSTEP01_CONFIG_SW_USER,
	.step_mode		= STEP_MODE_1_16,
	.step_mode_cm_vm= POWERSTEP01_CM_VM_CURRENT,
	.step_mode_sync	= POWERSTEP01_SYNC_SEL_8,
	.cfg_osc_clk_sel= POWERSTEP01_CONFIG_INT_16MHZ,
	.gatecfg1_wd_en	= POWERSTEP01_WD_EN_DISABLE,
};


//--- ps_main ---------------------------
int	 ps_main(void) 
{	
	INT32 pos = _ps_spi_transfer(SPI_READ, POWERSTEP01_GET_PARAM | POWERSTEP01_ABS_POS, 3,0);
	_motorStatus.position = (-pos) & 0xfffff;

	if(ps_init_done() == 2)
	{
		_motorStatus.powerStepStatus = _ps_spi_transfer(SPI_READ, POWERSTEP01_GET_PARAM | POWERSTEP01_STATUS, 2,0);
		
		if(_motorStatus.moving && ((_motorStatus.powerStepStatus & POWERSTEP01_STATUS_MOT_STATUS) == 0))
		{
			if (_moveCmd.hdr.msgId==0  || _motorStatus.position==_moveCmd.steps)
			{
				TrPrintf(true, "Move[%d]done:", _motorStatus.moveCnt);
				_motorStatus.moving = false;
				_motorStatus.moveCnt++;
				memset(&_moveCmd, 0, sizeof(_moveCmd));
				ps_set_current(_motorStatus.holdCurrent);
				comm_send_status();				
			}
			else if (_moveTries>0)
			{
				TrPrintf(true, "Move[%d]done: Error pos=%d, expected=%d", _motorStatus.moveCnt, _motorStatus.position, _moveCmd.steps);
				move_absolute(&_moveCmd, --_moveTries);
			}
		}
	}
	
	volatile uint32_t res = _ps_spi_transfer(SPI_READ, POWERSTEP01_GET_PARAM |POWERSTEP01_ADC_OUT, 1,0);
	
	uint32_t old = _motorStatus.voltage;
	_motorStatus.voltage = (res * 3300) * (56 + 2) / 2.0 / 32.0;

	if (old >= MIN_VOLTAGE && _motorStatus.voltage < MIN_VOLTAGE) 
		return REPLY_ERROR;
	
	return REPLY_OK;
}

//--- ps_init ----------------------------------------------
int ps_init()
{	
	volatile SPS_MotorCfg *cfg;
	
	memset(&_motorStatus, 0, sizeof(_motorStatus));
	memset(&_motorReference, 0, sizeof(_motorReference));
	memset(&_moveCmd, 0, sizeof(_moveCmd));
	
	cfg = &cfg_motor;

	_ps_spi_transfer(SPI_WRITE, POWERSTEP01_SET_PARAM |POWERSTEP01_STEP_MODE, 1,	  cfg->step_mode_sync
																					| cfg->step_mode_cm_vm
																					| cfg->step_mode);

	if (cfg->step_mode_cm_vm==POWERSTEP01_CM_VM_VOLTAGE)
	{
		_ps_spi_transfer(SPI_WRITE, POWERSTEP01_SET_PARAM | POWERSTEP01_CONFIG, 2,	cfg->cfg_pwm_div
																					| cfg->cfg_pwm_mul
																					| cfg->cfg_vccval
																					| cfg->cfg_uvloval
																					| cfg->cfg_oc_sd
																					| cfg->cfg_vscomp
																					| cfg->cfg_sw_mode
																					| cfg->cfg_osc_clk_sel);
		
		_ps_spi_transfer(SPI_WRITE, POWERSTEP01_SET_PARAM | POWERSTEP01_STALL_TH,	1,	StallTh_to_Par(cfg->stall_th));
		
		_ps_spi_transfer(SPI_WRITE, POWERSTEP01_SET_PARAM | POWERSTEP01_K_THERM,	1,	KTherm_to_Par(cfg->k_therm));

		_ps_spi_transfer(SPI_WRITE, POWERSTEP01_SET_PARAM | POWERSTEP01_KVAL_HOLD,	1,	Kval_Perc_to_Par(cfg->kval_hold));
		_ps_spi_transfer(SPI_WRITE, POWERSTEP01_SET_PARAM | POWERSTEP01_KVAL_RUN,	1,	Kval_Perc_to_Par(cfg->kval_run));
		_ps_spi_transfer(SPI_WRITE, POWERSTEP01_SET_PARAM | POWERSTEP01_KVAL_ACC,	1,	Kval_Perc_to_Par(cfg->kval_acc));
		_ps_spi_transfer(SPI_WRITE, POWERSTEP01_SET_PARAM | POWERSTEP01_KVAL_DEC,	1,	Kval_Perc_to_Par(cfg->kval_dec));

		_ps_spi_transfer(SPI_WRITE, POWERSTEP01_SET_PARAM | POWERSTEP01_INT_SPD, 2,	IntSpd_Steps_to_Par(cfg->int_speed));
		_ps_spi_transfer(SPI_WRITE, POWERSTEP01_SET_PARAM | POWERSTEP01_ST_SLP,		1,	BEMF_Slope_Perc_to_Par(cfg->st_slp));	
		_ps_spi_transfer(SPI_WRITE, POWERSTEP01_SET_PARAM | POWERSTEP01_FN_SLP_ACC,	1,	BEMF_Slope_Perc_to_Par(cfg->fn_slp_acc));	
		_ps_spi_transfer(SPI_WRITE, POWERSTEP01_SET_PARAM | POWERSTEP01_FN_SLP_DEC,	1,	BEMF_Slope_Perc_to_Par(cfg->fn_slp_dec));	
	}
	else
	{
		// Set Config Current Mode
		_ps_spi_transfer(SPI_WRITE, POWERSTEP01_SET_PARAM | POWERSTEP01_CONFIG, 2,	cfg->cfg_pred_en
																					| cfg->cfg_tsw
																					| cfg->cfg_vccval
																					| cfg->cfg_uvloval
																					| cfg->cfg_oc_sd
																					| cfg->cfg_en_tqreg
																					| cfg->cfg_sw_mode
																					| cfg->cfg_osc_clk_sel);

		_ps_spi_transfer(SPI_WRITE, POWERSTEP01_SET_PARAM | POWERSTEP01_TOFF_MIN,	1,	Tmin_Time_to_Par(cfg->toff_min));
		_ps_spi_transfer(SPI_WRITE, POWERSTEP01_SET_PARAM | POWERSTEP01_TON_MIN,	1,	Tmin_Time_to_Par(cfg->ton_min));
		_ps_spi_transfer(SPI_WRITE, POWERSTEP01_SET_PARAM | POWERSTEP01_T_FAST,		1,	cfg->t_fast_toff_fast
																| cfg->t_fast_fast_step);

		_ps_spi_transfer(SPI_WRITE, POWERSTEP01_SET_PARAM | POWERSTEP01_TVAL_HOLD,	1,	Tval_Current_to_Par(cfg->tval_hold));
		_ps_spi_transfer(SPI_WRITE, POWERSTEP01_SET_PARAM | POWERSTEP01_TVAL_RUN,	1,	Tval_Current_to_Par(cfg->tval_run));
		_ps_spi_transfer(SPI_WRITE, POWERSTEP01_SET_PARAM | POWERSTEP01_TVAL_ACC,	1,	Tval_Current_to_Par(cfg->tval_acc));
		_ps_spi_transfer(SPI_WRITE, POWERSTEP01_SET_PARAM | POWERSTEP01_TVAL_DEC,	1,	Tval_Current_to_Par(cfg->tval_dec));
	}

	// Set GateCFG1 (Has to be written at start-up)
	_ps_spi_transfer(SPI_WRITE, POWERSTEP01_SET_PARAM | POWERSTEP01_GATECFG1, 2,	cfg->gatecfg1_wd_en
																					| cfg->gatecfg1_tboost
																					| cfg->gatecfg1_igate
																					| cfg->gatecfg1_tcc);
	_ps_spi_transfer(SPI_WRITE, POWERSTEP01_SET_PARAM | POWERSTEP01_GATECFG2,	1,	cfg->gatecfg2_tblank
																					| cfg->gatecfg2_tdt);
	_ps_spi_transfer(SPI_WRITE, POWERSTEP01_SET_PARAM | POWERSTEP01_ALARM_EN,   1,	cfg->alarm_en);
	_ps_spi_transfer(SPI_WRITE, POWERSTEP01_SET_PARAM | POWERSTEP01_OCD_TH,		1,	cfg->ocd_th);

	// Set Speed Profile
	_ps_spi_transfer(SPI_WRITE, POWERSTEP01_SET_PARAM | POWERSTEP01_ACC,		2,	AccDec_Steps_to_Par(cfg->acc));	
	_ps_spi_transfer(SPI_WRITE, POWERSTEP01_SET_PARAM | POWERSTEP01_DEC,		2,	AccDec_Steps_to_Par(cfg->dec));
	_ps_spi_transfer(SPI_WRITE, POWERSTEP01_SET_PARAM | POWERSTEP01_MAX_SPEED,	2,	MaxSpd_Steps_to_Par(cfg->max_peed));
	_ps_spi_transfer(SPI_WRITE, POWERSTEP01_SET_PARAM | POWERSTEP01_MIN_SPEED,	2,	MinSpd_Steps_to_Par(cfg->min_speed));
	_ps_spi_transfer(SPI_WRITE, POWERSTEP01_SET_PARAM | POWERSTEP01_FS_SPD,		2,	cfg->fs_spd_boost
																					| FSSpd_Steps_to_Par(cfg->fs_spd));										
	// Read config, reset error flags
	
	_motorStatus.powerStepStatus = _ps_spi_transfer(SPI_READ, POWERSTEP01_GET_PARAM | POWERSTEP01_STATUS, 2,0);
	
	_init_done = 2;
			
	return REPLY_OK;
}

//--- ps_set_current --------------------------------------
int	 ps_set_current	 (double current)
{
	_motorStatus.moveCurrent = current;
	_ps_spi_transfer(SPI_WRITE, POWERSTEP01_SET_PARAM | POWERSTEP01_TVAL_HOLD,	1,	Tval_Current_to_Par(current));
	_ps_spi_transfer(SPI_WRITE, POWERSTEP01_SET_PARAM | POWERSTEP01_TVAL_RUN,	1,	Tval_Current_to_Par(current));
	_ps_spi_transfer(SPI_WRITE, POWERSTEP01_SET_PARAM | POWERSTEP01_TVAL_ACC,	1,	Tval_Current_to_Par(current));
	_ps_spi_transfer(SPI_WRITE, POWERSTEP01_SET_PARAM | POWERSTEP01_TVAL_DEC,	1,	Tval_Current_to_Par(current));			
	return REPLY_OK;
}


//--- ps_get_power -----------------------------------
int ps_get_power(void)
{
	return _motorStatus.voltage;
}
int ps_get_status(void)
{
	return _motorStatus.powerStepStatus;
}
int ps_get_abs_pos(void)
{
	return _motorStatus.position;
}

SMotorStatus* ps_get_motor_status(void)
{
	return &_motorStatus;
}

SMotorReference* ps_get_motor_reference(void)
{
	return &_motorReference;		
}	

//--- motor_config ---------------------
void motor_config(int currentHold)
{
	if (ps_get_power() < MIN_VOLTAGE) return;
	if (_init_done) return;
		_init_done = 1;

	ps_init();

	_motorStatus.holdCurrent = currentHold;
	ps_set_current(0);	
	
}
//--- motors_init_done -----------------------
int	ps_init_done(void)
{
	return _init_done;			
}

void stop_motor(void)
{
	_motorStatus.stopPos = _motorStatus.position;
	memset(&_moveCmd, 0, sizeof(_moveCmd));
	_ps_spi_transfer(SPI_WRITE, POWERSTEP01_HARD_STOP, 0, 0);
	TrPrintf(true, "CMD_MOTOR_STOP at %d", _motorStatus.stopPos);
}

void move_reference(void)
{
	_motorStatus.refDone		= false;
	_motorReference.refSearch	= true;

	memset(&_moveCmd, 0, sizeof(_moveCmd));

	if(gpio_get_pin_level(Input2) == 0)	// not on ref switch
	{
		move(POWERSTEP01_MOVE_FW,
			100000,
			100,
			100,
			300);		
	}
	else
	{	
		move(POWERSTEP01_MOVE_BW,
			1600,
			50,
			100,
			300);
	}
}

void move_absolute(SetupAssist_MoveCmd* motorMoveCommand, int tries)
{
	if (tries<=0) return;
	_moveTries = tries;
	memcpy(&_moveCmd, motorMoveCommand, sizeof(_moveCmd));
	int steps = motorMoveCommand->steps - _motorStatus.position;
	TrPrintf(true, "move_absolute[%d] from=%d to=%d steps=%d", _motorStatus.moveCnt, _motorStatus.position, _moveCmd.steps, steps);
	if(_motorStatus.refDone)
	{
		if (steps==0)
		{
			_motorStatus.moveCnt++;
			comm_send_status();				
		}
		else if (steps > 0)
		{
			if(gpio_get_pin_level(Input1)==0)
			{
				move(POWERSTEP01_MOVE_BW,
				steps,
				motorMoveCommand->speed,
				motorMoveCommand->acc,
				motorMoveCommand->current);
			}
		}
		else
		{
			if(gpio_get_pin_level(Input2)==0)
			{
				move(POWERSTEP01_MOVE_FW,
				-steps,
				motorMoveCommand->speed,
				motorMoveCommand->acc,
				motorMoveCommand->current);
			}
		}
	}
	else TrPrintf(true, "Reference NOT done");
}

void move_relative(SetupAssist_MoveCmd* motorMoveCommand)
{
	if(_motorStatus.refDone)
	{
		TrPrintf(true, "move_relative[%d].steps=%d", _motorStatus.moveCnt, motorMoveCommand->steps);

		if(motorMoveCommand->steps > 0)
		{
			if(gpio_get_pin_level(Input1)==0)
			{
				move(POWERSTEP01_MOVE_BW,
					motorMoveCommand->steps,
					motorMoveCommand->speed,
					motorMoveCommand->acc,
					motorMoveCommand->current);
			}
		}
		else
		{
			if(gpio_get_pin_level(Input2)==0)
			{
					move(POWERSTEP01_MOVE_FW,
					-motorMoveCommand->steps,
					motorMoveCommand->speed,
					motorMoveCommand->acc,
					motorMoveCommand->current);
			}
		}
	}
}

void move(uint8_t moveType, INT32 steps, UINT32 speed, UINT32 acc, UINT32 current)
{
	_ps_spi_transfer(SPI_WRITE, POWERSTEP01_SET_PARAM | POWERSTEP01_MAX_SPEED,	2,	MaxSpd_Steps_to_Par(speed));
	_ps_spi_transfer(SPI_WRITE, POWERSTEP01_SET_PARAM | POWERSTEP01_ACC,		2,	AccDec_Steps_to_Par(acc));
	_ps_spi_transfer(SPI_WRITE, POWERSTEP01_SET_PARAM | POWERSTEP01_DEC,		2,	AccDec_Steps_to_Par(acc));
	ps_set_current(current);
	_ps_spi_transfer(SPI_WRITE, moveType, 3, steps);
	_motorStatus.stopPos = -1;
	_motorStatus.moving = true;
}

void reset_pos(void)
{
	_ps_spi_transfer(SPI_WRITE, POWERSTEP01_RESET_POS, 0, 0);
}

void ps_timer_cb(const struct timer_task *const timer_task)
{
	ps_main();
	
	if(!ps_init_done())
	{
		motor_config(CURRENT_Z_HOLD);
	}
	else
	{
		/*
		switch(Command)
		{
			case 1:
			{
				if((ps_get_status() & POWERSTEP01_STATUS_BUSY))
				{
					_motorStatus.moving = true;
					_ps_spi_transfer(SPI_WRITE, POWERSTEP01_SET_PARAM | POWERSTEP01_MAX_SPEED,	2,	MaxSpd_Steps_to_Par(500));
					_ps_spi_transfer(SPI_WRITE, POWERSTEP01_SET_PARAM | POWERSTEP01_ACC,		2,	AccDec_Steps_to_Par(500));
					_ps_spi_transfer(SPI_WRITE, POWERSTEP01_SET_PARAM | POWERSTEP01_DEC,		2,	AccDec_Steps_to_Par(500));
					ps_set_current(300.0);
					_ps_spi_transfer(SPI_WRITE, POWERSTEP01_MOVE_FW, 3,0xffff);
					
					Command = 2;
				}
			}
			break;
			case 2:
			{
				if((ps_get_status() & POWERSTEP01_STATUS_BUSY))
				{
					_motorStatus.moving = false;
					ps_set_current(0.0);
					gpio_set_pin_level(SM_EXT_INPUT,true);

					Command = 0;
				}
			}
			break;
			default:
			{
				
			}
			break;
		}
		*/
	}
}

