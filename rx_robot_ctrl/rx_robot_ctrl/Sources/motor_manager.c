#include "motor_manager.h"
#include "rx_robot_def.h"
#include "rx_robot_tcpip.h"

#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "sys/types.h"

#include <ft900.h>

#include "network_manager.h"
#include "status_manager.h"
#include "gpio_manager.h"
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
static uint32_t _encoderTolerance[MOTOR_CNT];
static uint8_t _stopBits[MOTOR_CNT];
static uint8_t _stopBitLevels[MOTOR_CNT];

// Status Flags
static bool _isInitialized = false;

static void set_config(SRobotMotorSetCfgCmd* config);
static void move_motors(SRobotMotorsMoveCmd* moveCommand);
static void stop_motors(SRobotMotorsStopCmd* stopCommand);
static void reset_motors(SRobotMotorsResetCmd* resetCommand);
static void update_motor(uint8_t motor);
static void check_encoder(uint8_t motor);
static void check_stop_bits(uint8_t motor);
static void stop_motor(uint8_t motor);
static void reset_motor(uint8_t motor);
static int32_t spi_read_register(uint8_t reg, uint8_t motor);
static void spi_write_register(uint8_t reg, uint32_t data, uint8_t motor);
static void update_status(uint8_t status, uint8_t motor);

bool motor_manager_start(void)
{
	memset(&_motorConfigs, 0, sizeof(_motorConfigs));
	memset(&_encoderTolerance, 0, sizeof(_encoderTolerance));
	memset(&_stopBits, 0, sizeof(_stopBits));
	memset(&_stopBitLevels, 0, sizeof(_stopBitLevels));

	spi_init(SPIM, spi_dir_master, spi_mode_3, SPI_CLOCK_DIVIDER);
	spi_option(SPIM, spi_option_fifo_size, SPI_FIFO_SIZE);
	spi_option(SPIM, spi_option_fifo, SPI_FIFO_ENABLED);
	spi_option(SPIM, spi_option_fifo_receive_trigger, SPI_FIFO_TRIGGER_LEVEL);

	_isInitialized = true;
	return true;
}

bool motor_manager_is_initalized(void)
{
	return _isInitialized;
}

//--- motor_handle_message ----------------------------------------------------------------------
void motor_handle_message(void* message)
{
	SMsgHdr* hdr = (SMsgHdr*)message;

	if(_isInitialized)
	{
		switch(hdr->msgId)
		{
		case CMD_MOTOR_SET_CONFIG: 	set_config((SRobotMotorSetCfgCmd*)message); break;
		case CMD_MOTORS_MOVE:		move_motors((SRobotMotorsMoveCmd*)message);	break;
		case CMD_MOTORS_STOP:		stop_motors((SRobotMotorsStopCmd*)message);	break;
		case CMD_MOTORS_RESET:		reset_motors((SRobotMotorsResetCmd*)message);	break;
		default:
			break;
		}
	}
}

//--- motor_main ---------------------------------------------------------------
void motor_main(void)
{
	for(uint8_t motor=0; motor < MOTOR_CNT; motor++)
	{
		if(RX_RobotStatus.motor[motor].isConfigured)
		{
			update_motor(motor);
			check_encoder(motor);
			check_stop_bits(motor);
		}
	}
}

static void set_config(SRobotMotorSetCfgCmd* configCommand)
{
	if(configCommand->motor >= MOTOR_CNT)
		return;

	memcpy(&_motorConfigs[configCommand->motor], &configCommand->config, sizeof(SMotorConfig));

	spi_write_register(TMC_GCONF_REG, configCommand->config.gconf, configCommand->motor);
	spi_write_register(TMC_SWMODE_REG, configCommand->config.swmode, configCommand->motor);
	spi_write_register(TMC_IHOLDIRUN_REG, configCommand->config.iholdirun, configCommand->motor);
	spi_write_register(TMC_TPOWERDOWN_REG, configCommand->config.tpowerdown, configCommand->motor);
	spi_write_register(TMC_TPWMTHRS_REG, configCommand->config.tpwmthrs, configCommand->motor);
	spi_write_register(TMC_RAMPMODE_REG, configCommand->config.rampmode, configCommand->motor);
	spi_write_register(TMC_VMAX_REG, configCommand->config.vmax, configCommand->motor);
	spi_write_register(TMC_V1_REG, configCommand->config.v1, configCommand->motor);
	spi_write_register(TMC_AMAX_REG, configCommand->config.amax, configCommand->motor);
	spi_write_register(TMC_DMAX_REG, configCommand->config.dmax, configCommand->motor);
	spi_write_register(TMC_A1_REG, configCommand->config.a1, configCommand->motor);
	spi_write_register(TMC_D1_REG, configCommand->config.d1, configCommand->motor);
	spi_write_register(TMC_VSTART_REG, configCommand->config.vstart, configCommand->motor);
	spi_write_register(TMC_VSTOP_REG, configCommand->config.vstop, configCommand->motor);
	spi_write_register(TMC_TZEROWAIT_REG, configCommand->config.tzerowait, configCommand->motor);
	spi_write_register(TMC_VDCMIN_REG, configCommand->config.vdcmin, configCommand->motor);
	spi_write_register(TMC_CHOPCONF_REG, configCommand->config.chopconf, configCommand->motor);
	spi_write_register(TMC_COOLCONF_REG, configCommand->config.coolconf, configCommand->motor);
	spi_write_register(TMC_PWMCONF_REG, configCommand->config.pwmconf, configCommand->motor);
	spi_write_register(TMC_TCOOLTHRS_REG, configCommand->config.tcoolthrs, configCommand->motor);
	spi_write_register(TMC_THIGH_REG, configCommand->config.thigh, configCommand->motor);
	spi_write_register(TMC_ENCMODE_REG, configCommand->config.encmode, configCommand->motor);
	spi_write_register(TMC_ENCCONST_REG, configCommand->config.encconst, configCommand->motor);
	reset_motor(configCommand->motor);
	RX_RobotStatus.motor[configCommand->motor].isConfigured = true;
}

static void move_motors(SRobotMotorsMoveCmd* cmd)
{
	for(int motor = 0; motor < MOTOR_CNT; motor++)
	{
		if ((cmd->motors & (1<<motor)) && !RX_RobotStatus.motor[motor].isMoving)
		{
			TrPrintf(true, "Move Motor[%d] to %d", motor, cmd->targetPos[motor]);

			_encoderTolerance[motor] = cmd->encoderTol[motor];
			_stopBits[motor]		 = cmd->stopBits[motor];
			_stopBitLevels[motor] 	 = cmd->stopBitLevels[motor];

			gpio_manager_enable_motor(motor);
			gpio_manager_stop_motor(motor);	// Pause to start them in sync later
			spi_write_register(TMC_XTARGET_REG, cmd->targetPos[motor], motor);
		}
	}

	// Start motors sync
	for(int motor = 0; motor < MOTOR_CNT; motor++)
	{
		if ((cmd->motors & (1<<motor)) && !RX_RobotStatus.motor[motor].isMoving)
		{
			gpio_manager_start_motor(motor);
			RX_RobotStatus.motor[motor].moveIdStarted=cmd->moveId[motor];
		}
	}
}

static void stop_motors(SRobotMotorsStopCmd* cmd)
{
	for(int motor = 0; motor < MOTOR_CNT; motor++)
	{
		if(cmd->motors & (1<<motor)) stop_motor(motor);
	}
}

static void reset_motors(SRobotMotorsResetCmd* cmd)
{
	for(int motor = 0; motor < MOTOR_CNT; motor++)
	{
		if(cmd->motors & (1<<motor)) reset_motor(motor);
	}
	status_manager_send_status();
}

static void update_motor(uint8_t motor)
{
	RX_RobotStatus.motor[motor].motorPos  = spi_read_register(TMC_XACTUAL_REG, motor);
	RX_RobotStatus.motor[motor].encPos	  = spi_read_register(TMC_XENC_REG, motor);
	RX_RobotStatus.motor[motor].targetPos = spi_read_register(TMC_XTARGET_REG, motor);
	RX_RobotStatus.motor[motor].speed	  = spi_read_register(TMC_VACTUAL_REG, motor);
}

static void check_encoder(uint8_t motor)
{
	if(_encoderTolerance[motor] && (abs(RX_RobotStatus.motor[motor].motorPos - RX_RobotStatus.motor[motor].encPos) > _encoderTolerance[motor]))
	{
		RX_RobotStatus.motor[motor].isStalled = true;
		stop_motor(motor);
	}
}

static void check_stop_bits(uint8_t motor)
{
	if(RX_RobotStatus.motor[motor].isMoving && _stopBits[motor])
	{
		for(int in = 0; in < INPUT_CNT; in++)
		{
			if (_stopBits[motor] & (1<<in))
			{
				uint8_t isSet = gpio_manager_get_input(in);
				if(isSet == _stopBitLevels[motor])
				{
					_stopBits[motor] &= ~(1<<in); // stop only once
					stop_motor(motor);
					TrPrintf(true, "Motor[%d].in[%d]=%d: STOP", motor, in, isSet);
					return;
				}
			}
		}
	}
}

static void stop_motor(uint8_t motor)
{
	if(motor >= MOTOR_CNT)
		return;

	gpio_manager_stop_motor(motor);

	int32_t motorPosition = spi_read_register(TMC_XACTUAL_REG, motor);

	spi_write_register(TMC_XTARGET_REG, motorPosition, motor);
	spi_write_register(TMC_XACTUAL_REG, motorPosition, motor);

	gpio_manager_start_motor(motor);
}

static void reset_motor(uint8_t motor)
{
	if(motor >= MOTOR_CNT)
			return;

	gpio_manager_stop_motor(motor);

	RX_RobotStatus.motor[motor].targetPos = 0;
	RX_RobotStatus.motor[motor].motorPos  = 0;
	RX_RobotStatus.motor[motor].encPos 	  = 0;

	spi_write_register(TMC_XTARGET_REG, 0, motor);
	spi_write_register(TMC_XACTUAL_REG, 0, motor);
	spi_write_register(TMC_XENC_REG, 0, motor);

	_encoderTolerance[motor] = 0;
	_stopBits[motor] = 0;
	_stopBitLevels[motor] = 0;

	RX_RobotStatus.motor[motor].isMoving = false;
	RX_RobotStatus.motor[motor].isStalled = false;

	gpio_manager_start_motor(motor);
	gpio_manager_disable_motor(motor);
}

static int32_t spi_read_register(uint8_t reg, uint8_t motor)
{
	int32_t data;
	SpiTxDatagram_t txDatagram;
	SpiRxDatagram_t rxDatagram;

	if(_isInitialized == false)
		return 0;

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

	update_status(rxDatagram.status, motor);

	spi_open(SPIM, motor);
	spi_xchangen(SPIM, (uint8_t *)&rxDatagram, (uint8_t *)&txDatagram, sizeof(rxDatagram));
	spi_close(SPIM, motor);

	data = __bswap32(rxDatagram.data);

	update_status(rxDatagram.status, motor);

	return data;
}

static void spi_write_register(uint8_t reg, uint32_t data, uint8_t motor)
{
	SpiTxDatagram_t txDatagram;
	SpiRxDatagram_t rxDatagram;

	if(_isInitialized == false)
		return;

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

	update_status(rxDatagram.status, motor);
}

static void update_status(uint8_t status, uint8_t motor)
{
	RX_RobotStatus.motor[motor].status = status;

	int isMoving = !(RX_RobotStatus.motor[motor].status & TMC_STATUS_STANDSTILL_FLAG);
	if(RX_RobotStatus.motor[motor].isMoving && !isMoving)
	{
		RX_RobotStatus.motor[motor].moveIdDone = RX_RobotStatus.motor[motor].moveIdStarted;
		RX_RobotStatus.motor[motor].isMoving = false;
		_encoderTolerance[motor] = 0;
		_stopBits[motor] = 0;
		_stopBitLevels[motor] = 0;
		gpio_manager_disable_motor(motor);
		status_manager_send_status();
	}
	else if(!RX_RobotStatus.motor[motor].isMoving && isMoving)
	{
		RX_RobotStatus.motor[motor].isMoving = true;
		status_manager_send_status();
	}
}
