#include "motor_manager.h"
#include "motor_manager_def.h"

#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "sys/types.h"

#include <ft900.h>

#include "tinyprintf.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "communication_def.h"
#include "network_manager.h"
#include "status_manager.h"
#include "gpio_manager.h"

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

static MotorStatus_t _motorStatus[MOTOR_COUNT];
static MotorConfig_t _motorConfigs[MOTOR_COUNT];
static uint32_t _encoderTolerance[MOTOR_COUNT];
static uint8_t _stopBits[MOTOR_COUNT];
static uint8_t _stopBitLevels[MOTOR_COUNT];

// Task handles
static TaskHandle_t _motorManagerTask;

// Status Flags
static bool _isInitialized = false;

static QueueHandle_t _motorMessageQueue;

static void motor_manager_task(void *pvParameters);
static void handle_motor_message(void* message);
static void set_config(RobotMotorSetConfigCommand_t* config);
static void move_motors(RobotMotorsMoveCommand_t* moveCommand);
static void stop_motors(RobotMotorsStopCommand_t* stopCommand);
static void reset_motors(RobotMotorsResetCommand_t* resetCommand);
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
	memset(&_motorStatus, 0, sizeof(_motorStatus));
	memset(&_motorConfigs, 0, sizeof(_motorConfigs));
	memset(&_encoderTolerance, 0, sizeof(_encoderTolerance));
	memset(&_stopBits, 0, sizeof(_stopBits));
	memset(&_stopBitLevels, 0, sizeof(_stopBitLevels));

	_motorMessageQueue = network_manager_get_motor_message_queue();
	if(_motorMessageQueue == NULL)
		chip_reboot();

	spi_init(SPIM, spi_dir_master, spi_mode_3, SPI_CLOCK_DIVIDER);
	spi_option(SPIM, spi_option_fifo_size, SPI_FIFO_SIZE);
	spi_option(SPIM, spi_option_fifo, SPI_FIFO_ENABLED);
	spi_option(SPIM, spi_option_fifo_receive_trigger, SPI_FIFO_TRIGGER_LEVEL);

	xTaskCreate(motor_manager_task,
				"Motors",
				TASK_MOTOR_STACK_SIZE,
				NULL,
				TASK_MOTOR_PRIORITY,
				&_motorManagerTask);

	return true;
}

bool motor_manager_is_initalized(void)
{
	return _isInitialized;
}

MotorStatus_t* motor_manager_get_status(void)
{
	return _motorStatus;
}

static void motor_manager_task(void *pvParameters)
{
	static TickType_t lastWakeTime;
	static const TickType_t frequency = MOTOR_UPDATE_INTERVAL / portTICK_PERIOD_MS;

	while(network_manager_is_initialized() == false){
		vTaskDelay(100 / portTICK_PERIOD_MS);
	}

	_isInitialized = true;

	lastWakeTime = xTaskGetTickCount();

	while(true)
	{
		if(_motorMessageQueue != NULL)
		{
			void* message = NULL;
			if(xQueueReceive(_motorMessageQueue, &message, 0) == pdPASS)
			{
				handle_motor_message(message);
				vPortFree(message);

				status_manager_send_status();
			}
		}

		for(uint8_t motorCount = 0; motorCount < MOTOR_COUNT; motorCount++)
		{
			if(_motorStatus[motorCount].isConfigured != true)
				continue;

			update_motor(motorCount);
			check_encoder(motorCount);
			check_stop_bits(motorCount);
		}

		vTaskDelayUntil(&lastWakeTime, frequency);
	}
}

static void handle_motor_message(void* message)
{
	RobotMessageHeader_t* header = (RobotMessageHeader_t*)message;

	if(_isInitialized == false)
		return;

	switch(header->messageId)
	{
	case CMD_MOTOR_SET_CONFIG:
		set_config((RobotMotorSetConfigCommand_t*)message);
		break;

	case CMD_MOTORS_MOVE:
		move_motors((RobotMotorsMoveCommand_t*)message);
		break;

	case CMD_MOTORS_STOP:
		stop_motors((RobotMotorsStopCommand_t*)message);
		break;

	case CMD_MOTORS_RESET:
		reset_motors((RobotMotorsResetCommand_t*)message);
		break;
	default:
		break;
	}
}

static void set_config(RobotMotorSetConfigCommand_t* configCommand)
{
	if(configCommand->motor >= MOTOR_COUNT)
		return;

	memcpy(&_motorConfigs[configCommand->motor], &configCommand->config, sizeof(MotorConfig_t));

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
	_motorStatus[configCommand->motor].isConfigured = true;
}

static void move_motors(RobotMotorsMoveCommand_t* moveCommand)
{
	for(int motorCount = 0; motorCount < MOTOR_COUNT; motorCount++)
	{
		uint8_t motorSet = (moveCommand->motors >> motorCount) & 1U;

		if(motorSet == false)
			continue;

		if(_motorStatus[motorCount].isMoving == true)
			continue;

		_encoderTolerance[motorCount] = moveCommand->encoderTolerance[motorCount];
		_stopBits[motorCount] = moveCommand->stopBits[motorCount];
		_stopBitLevels[motorCount] = moveCommand->stopBitLevels[motorCount];

		gpio_manager_enable_motor(motorCount);
		gpio_manager_stop_motor(motorCount);	// Pause to start them in sync later
		spi_write_register(TMC_XTARGET_REG, moveCommand->targetPosition[motorCount], motorCount);
	}

	// Start motors sync
	for(int motorCount = 0; motorCount < MOTOR_COUNT; motorCount++)
	{
		uint8_t motorSet = (moveCommand->motors >> motorCount) & 1U;

		if(motorSet == true)
		{
			gpio_manager_start_motor(motorCount);
			_motorStatus[motorCount].moveStartCnt++;
		}
	}
}

static void stop_motors(RobotMotorsStopCommand_t* stopCommand)
{
	for(int motorCount = 0; motorCount < MOTOR_COUNT; motorCount++)
	{
		uint8_t motorSet = (stopCommand->motors >> motorCount) & 1U;

		if(motorSet == true)
		{
			stop_motor(motorCount);
		}
	}
}

static void reset_motors(RobotMotorsResetCommand_t* resetCommand)
{
	for(int motorCount = 0; motorCount < MOTOR_COUNT; motorCount++)
	{
		uint8_t motorSet = (resetCommand->motors >> motorCount) & 1U;

		if(motorSet == true)
		{
			reset_motor(motorCount);
		}
	}
}

static void update_motor(uint8_t motor)
{
	_motorStatus[motor].motorPosition = spi_read_register(TMC_XACTUAL_REG, motor);
	_motorStatus[motor].motorEncoderPosition = spi_read_register(TMC_XENC_REG, motor);
	_motorStatus[motor].motorTargetPosition = spi_read_register(TMC_XTARGET_REG, motor);
	_motorStatus[motor].motorVelocity = spi_read_register(TMC_VACTUAL_REG, motor);
}

static void check_encoder(uint8_t motor)
{
	if(_encoderTolerance[motor] == 0)
		return;

	int32_t positionDifference = abs(_motorStatus[motor].motorPosition - _motorStatus[motor].motorEncoderPosition);

	if(positionDifference > _encoderTolerance[motor])
	{
		_motorStatus[motor].isStalled = true;
		stop_motor(motor);
	}
}

static void check_stop_bits(uint8_t motor)
{
	if(_motorStatus[motor].isMoving != true)
		return;

	for(int inputCount = 0; inputCount < NUMBER_OF_INPUTS; inputCount++)
	{
		uint8_t used = (_stopBits[motor] >> inputCount) & 1;
		if(used == false)
			continue;


		uint8_t isSet = gpio_manager_get_input(inputCount);
		if(isSet == _stopBitLevels[motor])
		{
			stop_motor(motor);
			return;
		}
	}
}

static void stop_motor(uint8_t motor)
{
	if(motor >= MOTOR_COUNT)
		return;

	gpio_manager_stop_motor(motor);

	int32_t motorPosition = spi_read_register(TMC_XACTUAL_REG, motor);

	spi_write_register(TMC_XTARGET_REG, motorPosition, motor);
	spi_write_register(TMC_XACTUAL_REG, motorPosition, motor);

	gpio_manager_start_motor(motor);
}

static void reset_motor(uint8_t motor)
{
	if(motor >= MOTOR_COUNT)
			return;

	gpio_manager_stop_motor(motor);

	_motorStatus[motor].motorTargetPosition = 0;
	_motorStatus[motor].motorPosition = 0;
	_motorStatus[motor].motorEncoderPosition = 0;

	spi_write_register(TMC_XTARGET_REG, 0, motor);
	spi_write_register(TMC_XACTUAL_REG, 0, motor);
	spi_write_register(TMC_XENC_REG, 0, motor);

	_encoderTolerance[motor] = 0;
	_stopBits[motor] = 0;
	_stopBitLevels[motor] = 0;

	_motorStatus[motor].isMoving = false;
	_motorStatus[motor].isStalled = false;

	gpio_manager_start_motor(motor);
	gpio_manager_disable_motor(motor);
}

static int32_t spi_read_register(uint8_t reg, uint8_t motor)
{
	int32_t data;
	SpiTxDatagram_t txDatagram;
	SpiRxDatagram_t rxDatagram;

	taskENTER_CRITICAL();

	if(_isInitialized == false)
		return 0;

	if(motor >= MOTOR_COUNT)
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

	taskEXIT_CRITICAL();

	return data;
}

static void spi_write_register(uint8_t reg, uint32_t data, uint8_t motor)
{
	SpiTxDatagram_t txDatagram;
	SpiRxDatagram_t rxDatagram;

	taskENTER_CRITICAL();

	if(_isInitialized == false)
		return;


	if(motor >= MOTOR_COUNT)
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

	taskEXIT_CRITICAL();
}

static void update_status(uint8_t status, uint8_t motor)
{
	_motorStatus[motor].status = status;

	if((_motorStatus[motor].status & TMC_STATUS_STANDSTILL_FLAG) && (_motorStatus[motor].isMoving == true))
	{
		_motorStatus[motor].moveDoneCnt = _motorStatus[motor].moveStartCnt;
		_motorStatus[motor].isMoving = false;
		_encoderTolerance[motor] = 0;
		_stopBits[motor] = 0;
		_stopBitLevels[motor] = 0;
		gpio_manager_disable_motor(motor);
	//	status_manager_send_status();
	}
	else if(!(_motorStatus[motor].status & TMC_STATUS_STANDSTILL_FLAG))
	{
		_motorStatus[motor].isMoving = true;
	//	status_manager_send_status();
	}
}
