#ifndef INCLUDES_COMMUNICATION_DEF_H_
#define INCLUDES_COMMUNICATION_DEF_H_

#include <stdint.h>
#include <stdbool.h>

#include "gpio_manager_def.h"
#include "bootloader_manager_def.h"
#include "status_manager_def.h"
#include "motor_manager_def.h"

/* Defines */


// Command masks
#define COMMAND_TYPE_MASK		(0xFF000000)

#define STATUS_COMMAND_MASK		(0x10000000)
#define BOOTLOADER_COMMAND_MASK (0x20000000)
#define GPIO_COMMAND_MASK		(0x40000000)
#define MOTOR_COMMAND_MASK		(0x80000000)

// Status commands
#define CMD_STATUS_GET			(STATUS_COMMAND_MASK | 0x00000001)

// Bootloader commands
#define CMD_BOOTLOADER_START	(BOOTLOADER_COMMAND_MASK | 0x00000001)
#define CMD_BOOTLOADER_DATA		(BOOTLOADER_COMMAND_MASK | 0x00000002)
#define CMD_BOOTLOADER_CONFIRM	(BOOTLOADER_COMMAND_MASK | 0x00000003)

// GPIO commands
#define CMD_GPIO_SET			(GPIO_COMMAND_MASK | 0x00000001)

// Motor commands
#define CMD_MOTOR_SET_CONFIG	(MOTOR_COMMAND_MASK | 0x00000001)
#define CMD_MOTORS_MOVE			(MOTOR_COMMAND_MASK | 0x00000002)
#define CMD_MOTORS_STOP			(MOTOR_COMMAND_MASK | 0x00000003)
#define CMD_MOTORS_RESET		(MOTOR_COMMAND_MASK | 0x00000004)

/* Data structures */

#pragma pack(1)

typedef struct RobotMessageHeader
{
	int32_t  messageLength;
	uint32_t messageId;
} RobotMessageHeader_t;

typedef struct GpioSetCommand
{
	RobotMessageHeader_t header;
	uint8_t outputs;
} GpioSetCommand_t;

typedef struct BootloaderStartCommand
{
	RobotMessageHeader_t header;
	uint32_t size;
	uint32_t checksum; // Not used yet
} BootloaderStartCommand_t;

typedef struct BootloaderDataCommand
{
	RobotMessageHeader_t header;
	uint8_t data[BOOTLOADER_DATA_FRAME_SIZE];
	uint32_t length;
} BootloaderDataCommand_t;

typedef struct RobotStatusMessage
{
	RobotMessageHeader_t header;
	BootloaderStatus_t bootloader;
	GpioStatus_t gpio;
	MotorStatus_t motors[MOTOR_COUNT];
} RobotStatusMessage_t;

typedef struct RobotMotorSetConfigCommand
{
	RobotMessageHeader_t header;
	uint8_t motor;
	MotorConfig_t config;
} RobotMotorSetConfigCommand_t;

typedef struct RobotMotorsMoveCommand
{
	RobotMessageHeader_t header;
	uint8_t motors;
	int32_t targetPosition[MOTOR_COUNT];
	int32_t encoderTolerance[MOTOR_COUNT];
	uint8_t stopBits[MOTOR_COUNT];
	uint8_t stopBitLevels[MOTOR_COUNT];
} RobotMotorsMoveCommand_t;

typedef struct RobotMotorsStopCommand
{
	RobotMessageHeader_t header;
	uint8_t motors;
} RobotMotorsStopCommand_t;

typedef struct RobotMotorsResetCommand
{
	RobotMessageHeader_t header;
	uint8_t motors;
} RobotMotorsResetCommand_t;

#pragma pack()


#endif /* INCLUDES_COMMUNICATION_DEF_H_ */
