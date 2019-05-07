#include "rx_def.h"
#include "ps_common.h"

#ifndef _powerstep01_H__
#define __powerstep01_H__

// Application command Addresses ( <<5)
#define POWERSTEP01_NOP					(0x0)
#define POWERSTEP01_ADDR_SET_PARAM		(0x0)
#define POWERSTEP01_ADDR_GET_PARAM		(0x1)
#define POWERSTEP01_ADDR_CONF			(0x2)
#define POWERSTEP01_ADDR_GOTO_ABS		(0x3)
#define POWERSTEP01_ADDR_PERF_MOTION	(0x8)

// Application commands
#define POWERSTEP01_RUN_BW				(0x50)
#define POWERSTEP01_RUN_FW				(0x51)
#define POWERSTEP01_STEP_CLK_BW			(0x58)
#define POWERSTEP01_STEP_CLK_FW			(0x59)
#define POWERSTEP01_MOVE_BW				(0x40)
#define POWERSTEP01_MOVE_FW				(0x41)
#define POWERSTEP01_GOTO_ABS			(0x60)
#define POWERSTEP01_GOTO_ABS_BW			(0x68)
#define POWERSTEP01_GOTO_ABS_FW			(0x69)

// Application commands
#define POWERSTEP01_GO_HOME				(0x70)
#define POWERSTEP01_GO_MARK				(0x78)
#define POWERSTEP01_RESET_POS			(0xD8)
#define POWERSTEP01_RESET_DEV			(0xC0)
#define POWERSTEP01_SOFT_STOP			(0xB0)
#define POWERSTEP01_HARD_STOP			(0xB8)
#define POWERSTEP01_SOFT_HIZ			(0xA0)
#define POWERSTEP01_HARD_HIZ			(0xA8)
#define POWERSTEP01_GET_STATUS			(0xD0)
#define POWERSTEP01_RESERVED_0			(0xEB)
#define POWERSTEP01_RESERVED_1			(0xF8)

// General configuration parameter address
#define POWERSTEP01_PARAM_ABS_POS		(0x1)
#define POWERSTEP01_PARAM_EL_POS		(0x2)
#define POWERSTEP01_PARAM_MARK			(0x3)
#define POWERSTEP01_PARAM_SPEED			(0x4)
#define POWERSTEP01_PARAM_ACC			(0x5)
#define POWERSTEP01_PARAM_DEC			(0x6)
#define POWERSTEP01_PARAM_MAX_SPEED		(0x7)
#define POWERSTEP01_PARAM_MIN_SPEED		(0x8)
#define POWERSTEP01_PARAM_ADC_OUT		(0x12)
#define POWERSTEP01_PARAM_OCD_TH		(0x13)
#define POWERSTEP01_PARAM_FS_SPD		(0x15)
#define POWERSTEP01_PARAM_STEP_MODE		(0x16)
#define POWERSTEP01_PARAM_ALARM_EN		(0x17)
#define POWERSTEP01_PARAM_GATECFG1		(0x18)
#define POWERSTEP01_PARAM_GATECFG2		(0x19)
#define POWERSTEP01_PARAM_STATUS		(0x1B)

#define POWERSTEP01_PARAM_CONFIG		(0x1A)

// Voltage mode configuration parameter address
#define POWERSTEP01_PARAM_KVAL_HOLD		(0x09)
#define POWERSTEP01_PARAM_KVAL_RUN		(0x0A)
#define POWERSTEP01_PARAM_KVAL_ACC		(0x0B)
#define POWERSTEP01_PARAM_KVAL_DEC		(0x0C)
#define POWERSTEP01_PARAM_INT_SPEED		(0x0D)
#define POWERSTEP01_PARAM_ST_SLP		(0x0E)
#define POWERSTEP01_PARAM_FN_SLP_ACC	(0x0F)
#define POWERSTEP01_PARAM_FN_SLP_DEC	(0x10)
#define POWERSTEP01_PARAM_K_THERM		(0x11)
#define POWERSTEP01_PARAM_STALL_TH		(0x14)


// Current mode configuration parameter address
#define POWERSTEP01_PARAM_TVAL_HOLD		(0x09)
#define POWERSTEP01_PARAM_TVAL_RUN		(0x0A)
#define POWERSTEP01_PARAM_TVAL_ACC		(0x0B)
#define POWERSTEP01_PARAM_TVAL_DEC		(0x0C)
#define POWERSTEP01_PARAM_T_FAST		(0x0E)
#define POWERSTEP01_PARAM_TON_MIN		(0x0F)
#define POWERSTEP01_PARAM_TOFF_MIN		(0x10)

#endif /* __powerstep01_H__ */
