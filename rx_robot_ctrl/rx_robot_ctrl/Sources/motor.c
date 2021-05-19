#include "rx_robot_def.h"
#include "rx_robot_tcpip.h"

#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "sys/types.h"

#include <ft900.h>
#include <gpio.h>
#include <motor.h>
#include <ctrl.h>

#include "network.h"
#include "rx_trace.h"

#define TMC_RAMPSTAT_POS_REACHED_FLAG	0x80
#define TMC_RAMPSTAT_STOP_SG_FLAG		0x40

#define TMC_STATUS_RESET_FLAG				0X01
#define TMC_STATUS_DRIVER_ERROR_FLAG		0x02
#define TMC_STATUS_SG_FLAG					0x04
#define TMC_STATUS_STANDSTILL_FLAG			0x08
#define TMC_STATUS_VELOCITY_REACHED_FLAG	0x10
#define TMC_STATUS_POSITION_REACHED_FLAG	0x20

#define TMC_GCONF_REG			0x00
#define TMC_GSTAT_REG			0x01
#define TMC_IFCNT_REG			0x02
#define TMC_SLAVECONF_REG		0x03
#define TMC_IOIN_REG			0x04
#define TMC_OUTPUT_REG			0x04
#define TMC_XCOMPARE_REG		0x05
#define TMC_IHOLDIRUN_REG		0x10
#define TMC_TPOWERDOWN_REG		0x11
#define TMC_TSTEP_REG			0x12
#define TMC_TPWMTHRS_REG		0x13
#define TMC_TCOOLTHRS_REG		0x14
#define TMC_THIGH_REG			0x15
#define TMC_RAMPMODE_REG		0x20
#define TMC_XACTUAL_REG			0x21
#define TMC_VACTUAL_REG			0x22
#define TMC_VSTART_REG			0x23
#define TMC_A1_REG				0x24
#define TMC_V1_REG				0x25
#define TMC_AMAX_REG			0x26
#define TMC_VMAX_REG			0x27
#define TMC_DMAX_REG			0x28
#define TMC_D1_REG				0x2A
#define TMC_VSTOP_REG			0x2B
#define TMC_TZEROWAIT_REG		0x2C
#define TMC_XTARGET_REG			0x2D
#define TMC_VDCMIN_REG			0x33
#define TMC_SWMODE_REG			0x34
#define TMC_RAMPSTAT_REG		0x35
#define TMC_XLATCH_REG			0x36
#define TMC_ENCMODE_REG			0x38
#define TMC_XENC_REG			0x39
#define TMC_ENCCONST_REG		0x3A
#define TMC_ENCSTATUS_REG		0x3B
#define TMC_ENCLATCH_REG		0x3C
#define TMC_MSLUT0_REG			0x60
#define TMC_MSLUT1_REG			0x61
#define TMC_MSLUT2_REG			0x62
#define TMC_MSLUT3_REG			0x63
#define TMC_MSLUT4_REG			0x64
#define TMC_MSLUT5_REG			0x65
#define TMC_MSLUT6_REG			0x66
#define TMC_MSLUT7_REG			0x67
#define TMC_MSLUTSEL_REG		0x68
#define TMC_MSLUTSTART_REG		0x69
#define TMC_MSCNT_REG			0x6A
#define TMC_MSCURACT_REG		0x6B
#define TMC_CHOPCONF_REG		0x6C
#define TMC_COOLCONF_REG		0x6D
#define TMC_DCCTRL_REG			0x6E
#define TMC_DRVSTATUS_REG		0x6F
#define TMC_PWMCONF_REG			0x70
#define TMC_PWMSCALE_REG		0x71
#define TMC_ENCMCTRL_REG		0x72
#define TMC_LOSTTEPS_REG		0x73

// Motor settings
#define MOTOR_UPDATE_INTERVAL	(10)

// Task settings
#define TASK_MOTOR_STACK_SIZE	(500)
#define TASK_MOTOR_PRIORITY		(9)

#define SPI_CLOCK_DIVIDER		(64)
#define SPI_FIFO_SIZE			(64)
#define SPI_FIFO_ENABLED		(true)
#define SPI_FIFO_TRIGGER_LEVEL	(1)

#pragma pack(1)
typedef struct SpiTxDatagram {
	uint8_t addr;		// 7 bit address + 1bit RW
	uint32_t data;		// 32 bit data
}SpiTxDatagram_t;

typedef struct SpiRxDatagram {
	uint8_t status;		// 8 bit status
	uint32_t data;		// 32 bit data
} SpiRxDatagram_t;
#pragma pack()

static SMotorConfig _motorConfigs[MOTOR_CNT];
static uint8_t  _motors[MOTOR_CNT];
static uint8_t	_encoderCheck[MOTOR_CNT];
static uint32_t _encoderTolerance[MOTOR_CNT];
static int8_t	_edgeCheckIn[MOTOR_CNT];
static uint8_t	_edgeCheckLevel[MOTOR_CNT];
static int32_t	_edgeCheckPos[MOTOR_CNT];
static uint8_t  _stopBits[MOTOR_CNT];
static uint8_t  _stopBitLevels[MOTOR_CNT];
static uint8_t	_stoppedByInput[MOTOR_CNT];

// Status Flags
static void _set_config(SRobotMotorSetCfgCmd* config);
static void _move_motors(SRobotMotorsMoveCmd* moveCommand);
static void _stop_motors(SRobotMotorsStopCmd* stopCommand);
static void _reset_motors(SRobotMotorsResetCmd* resetCommand);
static void _update_motor(uint8_t motor);
static void _check_encoder(uint8_t motor);
static void _check_stop_bits(uint8_t motor);
static void _stop_motor(uint8_t motor);
static void _reset_motor(uint8_t motor);
static int32_t _spi_read_register(uint8_t reg, uint8_t motor);
static void _spi_write_register(uint8_t reg, uint32_t data, uint8_t motor);
static void _update_status(uint8_t status, uint8_t motor);

void motor_init(void)
{
	memset(_motorConfigs, 0, sizeof(_motorConfigs));
	memset(_encoderCheck, 0, sizeof(_encoderCheck));
	memset(_encoderTolerance, 0, sizeof(_encoderTolerance));
	memset(_edgeCheckIn, 0xff, sizeof(_edgeCheckIn));
	memset(_stopBits, 0, sizeof(_stopBits));
	memset(_stopBitLevels, 0, sizeof(_stopBitLevels));

	spi_init(SPIM, spi_dir_master, spi_mode_3, SPI_CLOCK_DIVIDER);
	spi_option(SPIM, spi_option_fifo_size, SPI_FIFO_SIZE);
	spi_option(SPIM, spi_option_fifo, SPI_FIFO_ENABLED);
	spi_option(SPIM, spi_option_fifo_receive_trigger, SPI_FIFO_TRIGGER_LEVEL);
}

//--- motor_handle_msg ----------------------------------------------------------------------
bool motor_handle_msg(void* message)
{
	SMsgHdr* hdr = (SMsgHdr*)message;

	switch(hdr->msgId)
	{
	case CMD_MOTOR_SET_CONFIG: 	_set_config((SRobotMotorSetCfgCmd*)message); 	return TRUE;
	case CMD_MOTORS_MOVE:		_move_motors((SRobotMotorsMoveCmd*)message);	return TRUE;
	case CMD_MOTORS_STOP:		_stop_motors((SRobotMotorsStopCmd*)message);	return TRUE;
	case CMD_MOTORS_RESET:		_reset_motors((SRobotMotorsResetCmd*)message);	return TRUE;
	default:
		break;
	}
	return FALSE;
}

//--- motor_tick ---------------------------------------------------------------
void motor_tick(int tick)
{
	for(uint8_t motor=0; motor < MOTOR_CNT; motor++)
	{
		if(RX_RobotStatus.motor[motor].isConfigured)
		{
			_update_motor(motor);
			_check_encoder(motor);
			_check_stop_bits(motor);
		}
	}
}

//--- _set_config -----------------------------------------------------------
static void _set_config(SRobotMotorSetCfgCmd* configCommand)
{
	if(configCommand->motor >= MOTOR_CNT)
		return;
	int motor=configCommand->motor;
	SMotorConfig *pact = &_motorConfigs[motor];
	SMotorConfig *pcfg = &configCommand->config;

	if (pact->gconf 	!= pcfg->gconf) 	_spi_write_register(TMC_GCONF_REG, 		pcfg->gconf, 	 motor);
	if (pact->swmode 	!= pcfg->swmode) 	_spi_write_register(TMC_SWMODE_REG,		pcfg->swmode, 	 motor);
	if (pact->iholdirun != pcfg->iholdirun) _spi_write_register(TMC_IHOLDIRUN_REG, 	pcfg->iholdirun, motor);
	if (pact->tpowerdown!= pcfg->tpowerdown)_spi_write_register(TMC_TPOWERDOWN_REG, 	pcfg->tpowerdown,motor);
	if (pact->tpwmthrs 	!= pcfg->tpwmthrs) 	_spi_write_register(TMC_TPWMTHRS_REG, 	pcfg->tpwmthrs,  motor);
	if (pact->rampmode 	!= pcfg->rampmode) 	_spi_write_register(TMC_RAMPMODE_REG, 	pcfg->rampmode,  motor);
	if (pact->vmax 		!= pcfg->vmax) 		_spi_write_register(TMC_VMAX_REG, 		pcfg->vmax, 	 motor);
	if (pact->v1 		!= pcfg->v1) 		_spi_write_register(TMC_V1_REG, 			pcfg->v1, 		 motor);
	if (pact->amax 		!= pcfg->amax) 		_spi_write_register(TMC_AMAX_REG, 		pcfg->amax, 	 motor);
	if (pact->dmax 		!= pcfg->dmax) 		_spi_write_register(TMC_DMAX_REG, 		pcfg->dmax, 	 motor);
	if (pact->a1 		!= pcfg->a1) 		_spi_write_register(TMC_A1_REG, 			pcfg->a1, 		 motor);
	if (pact->d1 		!= pcfg->d1) 		_spi_write_register(TMC_D1_REG, 			pcfg->d1, 		 motor);
	if (pact->vstart 	!= pcfg->vstart) 	_spi_write_register(TMC_VSTART_REG, 		pcfg->vstart, 	 motor);
	if (pact->vstop 	!= pcfg->vstop) 	_spi_write_register(TMC_VSTOP_REG, 		pcfg->vstop, 	 motor);
	if (pact->tzerowait != pcfg->tzerowait) _spi_write_register(TMC_TZEROWAIT_REG, 	pcfg->tzerowait, motor);
	if (pact->vdcmin 	!= pcfg->vdcmin) 	_spi_write_register(TMC_VDCMIN_REG, 		pcfg->vdcmin, 	 motor);
	if (pact->chopconf 	!= pcfg->chopconf) 	_spi_write_register(TMC_CHOPCONF_REG, 	pcfg->chopconf,  motor);
	if (pact->coolconf 	!= pcfg->coolconf) 	_spi_write_register(TMC_COOLCONF_REG, 	pcfg->coolconf,  motor);
	if (pact->pwmconf 	!= pcfg->pwmconf) 	_spi_write_register(TMC_PWMCONF_REG, 	pcfg->pwmconf, 	 motor);
	if (pact->tcoolthrs != pcfg->tcoolthrs) _spi_write_register(TMC_TCOOLTHRS_REG, 	pcfg->tcoolthrs, motor);
	if (pact->thigh 	!= pcfg->thigh) 	_spi_write_register(TMC_THIGH_REG, 		pcfg->thigh, 	 motor);
	if (pact->encmode 	!= pcfg->encmode) 	_spi_write_register(TMC_ENCMODE_REG, 	pcfg->encmode, 	 motor);
	if (pact->encconst 	!= pcfg->encconst) 	_spi_write_register(TMC_ENCCONST_REG, 	pcfg->encconst,  motor);

	memcpy(&_motorConfigs[motor], &configCommand->config, sizeof(SMotorConfig));

	_reset_motor(motor);
	RX_RobotStatus.motor[motor].isConfigured = true;
}

static void _move_motors(SRobotMotorsMoveCmd* cmd)
{
	for(int motor = 0; motor < MOTOR_CNT; motor++)
	{
		if (cmd->motors & (1<<motor))
		{
			_motors[motor]		 	 = cmd->motors;
			_encoderCheck[motor] 	 = cmd->encoderCheck[motor];
			_encoderTolerance[motor] = cmd->encoderTol[motor];
			_edgeCheckIn[motor]		 = cmd->edgeCheckIn[motor];
			_stopBits[motor]		 = cmd->stopBits[motor];
			_stopBitLevels[motor] 	 = cmd->stopBitLevels[motor];
			_stoppedByInput[motor]	 = FALSE;

			gpio_enable_motor(motor, TRUE);
			gpio_stop_motor(motor);	// Pause to start them in sync later
			int pos = _spi_read_register(TMC_XENC_REG, motor);
			switch (_encoderCheck[motor]) // reset encoder check
			{
			case ENC_CHECK_ENC: _spi_write_register(TMC_XACTUAL_REG, pos, motor); break;
			case ENC_CHECK_IN:	_edgeCheckPos[motor] 	= pos;
								_edgeCheckLevel[motor]  = gpio_get_input(_edgeCheckIn[motor]);
								break;
			default: break;
			}
			_spi_write_register(TMC_XTARGET_REG, cmd->targetPos[motor], motor);

			_update_motor(motor);
			TrPrintf(true, "Move Motor[%d].id=%d to %d (pos=%d enc=%d diff=%d)", motor, cmd->moveId[motor], cmd->targetPos[motor],
					RX_RobotStatus.motor[motor].motorPos, RX_RobotStatus.motor[motor].encPos,
					RX_RobotStatus.motor[motor].motorPos-RX_RobotStatus.motor[motor].encPos);
		}
	}

	int send=FALSE;
	// Start motors sync
	for(int motor = 0; motor < MOTOR_CNT; motor++)
	{
		if (cmd->motors & (1<<motor))
		{
			gpio_start_motor(motor);
			RX_RobotStatus.motor[motor].moveIdStarted=cmd->moveId[motor];
			RX_RobotStatus.motor[motor].isMoving = true;
			send=TRUE;
		}
	}

	if (send) ctrl_send_status();
}

static void _stop_motors(SRobotMotorsStopCmd* cmd)
{
	for(int motor = 0; motor < MOTOR_CNT; motor++)
	{
		if(cmd->motors & (1<<motor)) _stop_motor(motor);
	}
}

static void _reset_motors(SRobotMotorsResetCmd* cmd)
{
	for(int motor = 0; motor < MOTOR_CNT; motor++)
	{
		if(cmd->motors & (1<<motor)) _reset_motor(motor);
	}
	ctrl_send_status();
}

static void _update_motor(uint8_t motor)
{
	RX_RobotStatus.motor[motor].motorPos  = _spi_read_register(TMC_XACTUAL_REG, motor);
	RX_RobotStatus.motor[motor].encPos	  = _spi_read_register(TMC_XENC_REG, motor);
	RX_RobotStatus.motor[motor].targetPos = _spi_read_register(TMC_XTARGET_REG, motor);
	RX_RobotStatus.motor[motor].speed	  = _spi_read_register(TMC_VACTUAL_REG, motor);
}

static void _check_encoder(uint8_t motor)
{
	if(RX_RobotStatus.motor[motor].moveIdStarted!=RX_RobotStatus.motor[motor].moveIdDone
	&& _encoderCheck[motor]
	&& !RX_RobotStatus.motor[motor].isStalled
	)
	{
		int stalled=false;
		uint8_t isSet;
		switch(_encoderCheck[motor])
		{
		case ENC_CHECK_ENC:	stalled = (abs(RX_RobotStatus.motor[motor].motorPos - RX_RobotStatus.motor[motor].encPos) > _encoderTolerance[motor]);
							if (stalled)
							{
								TrPrintf(TRUE, "Motor[%d].MoveId=%d: stalled (pos=%d enc=%d diff=%d)", motor, RX_RobotStatus.motor[motor].moveIdStarted,
									RX_RobotStatus.motor[motor].motorPos, RX_RobotStatus.motor[motor].encPos,
									RX_RobotStatus.motor[motor].motorPos-RX_RobotStatus.motor[motor].encPos);
							}
							break;
		case ENC_CHECK_IN:	isSet = gpio_get_input(_edgeCheckIn[motor]);
							if (isSet && !_edgeCheckLevel[motor])
							{
				//				TrPrintf(true, "EdgeCheck[%d] pos=%d, diff=%d", motor, RX_RobotStatus.motor[motor].motorPos, abs(RX_RobotStatus.motor[motor].motorPos - _edgeCheckPos[motor]));
								_edgeCheckPos[motor] = RX_RobotStatus.motor[motor].motorPos;
							}
							_edgeCheckLevel[motor] = isSet;
							stalled = (abs(RX_RobotStatus.motor[motor].motorPos - _edgeCheckPos[motor]) > _encoderTolerance[motor]);
							if (stalled)
							{
								TrPrintf(TRUE, "Motor[%d].MoveId=%d: stalled (pos=%d diff=%d)", motor, RX_RobotStatus.motor[motor].moveIdStarted,
									RX_RobotStatus.motor[motor].motorPos,
									abs(RX_RobotStatus.motor[motor].motorPos - _edgeCheckPos[motor]));
							}
							break;
		default: break;
		}
		if (stalled)
		{
			for(int m=0; m<MOTOR_CNT; m++)
			{
				if(_motors[motor] & (1<<m))
				{
					RX_RobotStatus.motor[m].isStalled = true;
					_stop_motor(m);
				}
			}
		}
	}
}

static void _check_stop_bits(uint8_t motor)
{
	if(RX_RobotStatus.motor[motor].isMoving && _stopBits[motor])
	{
		for(int in = 0; in < INPUT_CNT; in++)
		{
			if (_stopBits[motor] & (1<<in))
			{
				uint8_t isSet = gpio_get_input(in);
				if(isSet == _stopBitLevels[motor])
				{
					_stopBits[motor] &= ~(1<<in); // stop only once
					_stoppedByInput[motor]=TRUE;
					_stop_motor(motor);
					TrPrintf(true, "Motor[%d]. moveId=%d: STOP by in[%d]=%d", motor, RX_RobotStatus.motor[motor].moveIdStarted, in, isSet);
					return;
				}
			}
		}
	}
}

static void _stop_motor(uint8_t motor)
{
	if(motor >= MOTOR_CNT)
		return;

	gpio_stop_motor(motor);

	int32_t motorPosition = _spi_read_register(TMC_XACTUAL_REG, motor);

	_spi_write_register(TMC_XTARGET_REG, motorPosition, motor);
	_spi_write_register(TMC_XACTUAL_REG, motorPosition, motor);

	gpio_start_motor(motor);
}

static void _reset_motor(uint8_t motor)
{
	if(motor >= MOTOR_CNT)
			return;

	gpio_stop_motor(motor);

	RX_RobotStatus.motor[motor].targetPos = 0;
	RX_RobotStatus.motor[motor].motorPos  = 0;
	RX_RobotStatus.motor[motor].encPos 	  = 0;

	_spi_write_register(TMC_XTARGET_REG, 0, motor);
	_spi_write_register(TMC_XACTUAL_REG, 0, motor);
	_spi_write_register(TMC_XENC_REG, 0, motor);

	_encoderCheck[motor]	 = 0;
	_encoderTolerance[motor] = 0;
	_stopBits[motor] 		 = 0;
	_stopBitLevels[motor]    = 0;

	RX_RobotStatus.motor[motor].isMoving = false;
	RX_RobotStatus.motor[motor].isStalled = false;

	gpio_start_motor(motor);
	gpio_enable_motor(motor, FALSE);
}

static int32_t _spi_read_register(uint8_t reg, uint8_t motor)
{
	int32_t data;
	SpiTxDatagram_t txDatagram;
	SpiRxDatagram_t rxDatagram;

	if(motor >= MOTOR_CNT)
		return 0;

	// Zero structs
	memset(&txDatagram, 0, sizeof(txDatagram));
	memset(&rxDatagram, 0, sizeof(rxDatagram));

	// Set Address and read bit
	txDatagram.addr = (reg & 0x7F);

	// Send reg to read from and read return value
	spi_open(SPIM, motor);
	spi_xchangen(SPIM, (uint8_t *)&rxDatagram, (uint8_t *)&txDatagram, sizeof(rxDatagram));
	spi_close(SPIM, motor);

	_update_status(rxDatagram.status, motor);

	spi_open(SPIM, motor);
	spi_xchangen(SPIM, (uint8_t *)&rxDatagram, (uint8_t *)&txDatagram, sizeof(rxDatagram));
	spi_close(SPIM, motor);

	data = __bswap32(rxDatagram.data);

	_update_status(rxDatagram.status, motor);

	return data;
}

static void _spi_write_register(uint8_t reg, uint32_t data, uint8_t motor)
{
	SpiTxDatagram_t txDatagram;
	SpiRxDatagram_t rxDatagram;

	if(motor >= MOTOR_CNT)
		return;

	// Zero structs
	memset(&txDatagram, 0, sizeof(txDatagram));

	// Set Address and write bit
	txDatagram.addr = (reg | 0x80);

	// Set data
	txDatagram.data = __bswap32(data);

	spi_open(SPIM, motor);
	spi_xchangen(SPIM, (uint8_t *)&rxDatagram, (uint8_t *)&txDatagram, sizeof(rxDatagram));
	spi_close(SPIM, motor);

	_update_status(rxDatagram.status, motor);
}

static void _update_status(uint8_t status, uint8_t motor)
{
	RX_RobotStatus.motor[motor].status = status;

	int isMoving = !(RX_RobotStatus.motor[motor].status & TMC_STATUS_STANDSTILL_FLAG);
	if(RX_RobotStatus.motor[motor].isMoving
	&& !isMoving
	&& (RX_RobotStatus.motor[motor].motorPos==RX_RobotStatus.motor[motor].targetPos || _stoppedByInput[motor]))
	{
		RX_RobotStatus.motor[motor].moveIdDone = RX_RobotStatus.motor[motor].moveIdStarted;
		RX_RobotStatus.motor[motor].isMoving = false;
		_encoderCheck[motor]= 0;
		_encoderTolerance[motor] = 0;
		_stopBits[motor] = 0;
		_stopBitLevels[motor] = 0;
		gpio_enable_motor(motor, FALSE);
		ctrl_send_status();
	}
}
