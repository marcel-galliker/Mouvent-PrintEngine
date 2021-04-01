#pragma 

#include "rx_robot_def.h"

#include "tcp_ip.h"

#include <stdint.h>
#include <stdbool.h>

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

// Status

#pragma pack(1)

typedef struct SGpioSetCmd
{
	SMsgHdr header;
	uint8_t outputs;
} SGpioSetCmd;

typedef struct SRobotStatusMsg
{
	SMsgHdr header;
	SBootloaderStatus bootloader;
	SGpioStatus gpio;
	SMotorStatus motors[MOTOR_COUNT];
} SRobotStatusMsg;

typedef struct SRobotMotorSetCfgCmd
{
	SMsgHdr header;
	uint8_t motor;
	SMotorConfig config;
} SRobotMotorSetCfgCmd;

typedef struct SRobotMotorsMoveCmd
{
	SMsgHdr header;
	uint8_t motors;						// bitset 
	int32_t targetPos[MOTOR_COUNT];
	int32_t encoderTol[MOTOR_COUNT];
	uint8_t stopBits[MOTOR_COUNT];		// bitset per motor
	uint8_t stopBitLevels[MOTOR_COUNT];	// one level per motor
} SRobotMotorsMoveCmd;

typedef struct SRobotMotorsStopCmd
{
	SMsgHdr header;
	uint8_t motors;
} SRobotMotorsStopCmd;

typedef struct SRobotMotorsResetCmd
{
	SMsgHdr header;
	uint8_t motors;
} SRobotMotorsResetCmd;

#pragma pack()