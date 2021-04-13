#pragma once


#include <stdint.h>
#include <stdbool.h>
#include "rx_robot_def.h"

#ifndef __FT900__
	#include "tcp_ip.h"#endif
// Network defines
#define PORT_UDP_BOOT_SVR		7004
#define PORT_UDP_BOOT_CLNT		7005

#define PORT_UDP_BOOTLOADER		7018
#define PORT_UDP_COMMUNICATION	7017

// Command masks
#define COMMAND_TYPE_MASK		(0xFF000000)

#define STATUS_COMMAND_MASK		(0x10000000)
#define BOOTLOADER_COMMAND_MASK (0x20000000)
#define GPIO_COMMAND_MASK		(0x40000000)
#define MOTOR_COMMAND_MASK		(0x80000000)

// Status commands
#define CMD_STATUS_GET			(STATUS_COMMAND_MASK | 0x00000001)
#define CMD_TRACE				(STATUS_COMMAND_MASK | 0x00000002)

// Bootloader commands
#define CMD_BOOTLOADER_START	(BOOTLOADER_COMMAND_MASK | 0x00000001)
#define CMD_BOOTLOADER_DATA		(BOOTLOADER_COMMAND_MASK | 0x00000002)
#define CMD_BOOTLOADER_CONFIRM	(BOOTLOADER_COMMAND_MASK | 0x00000003)

// GPIO commands
#define CMD_GPIO_SET			(GPIO_COMMAND_MASK | 0x00000001)
#define CMD_GPIO_RESET			(GPIO_COMMAND_MASK | 0x00000002)

// Motor commands
#define CMD_MOTOR_SET_CONFIG	(MOTOR_COMMAND_MASK | 0x00000001)
#define CMD_MOTORS_MOVE			(MOTOR_COMMAND_MASK | 0x00000002)
#define CMD_MOTORS_STOP			(MOTOR_COMMAND_MASK | 0x00000003)
#define CMD_MOTORS_RESET		(MOTOR_COMMAND_MASK | 0x00000004)

/* Data structures */

// Status

#pragma pack(1)

#ifdef __FT900__

	#define UINT8  uint8_t
	#define UINT32 uint32_t
	#define INT32  int32_t

	typedef struct SMsgHdr
	{
		INT32  msgLen;
		UINT32 msgId;
	} SMsgHdr;
#endif

typedef struct SGpioSetCmd
{
	SMsgHdr header;
	UINT8 outputs;
} SGpioSetCmd;

typedef struct SRobotStatusMsg
{
	SMsgHdr header;
	UINT32				alive;
	SBootloaderStatus bootloader;
	SGpioStatus gpio;
	SMotorStatus motor[MOTOR_CNT];
} SRobotStatusMsg;

typedef struct
{
	SMsgHdr header;
	char	message[256];
} SRobotTraceMsg;

typedef struct SRobotMotorSetCfgCmd
{
	SMsgHdr header;
	UINT8 motor;
	SMotorConfig config;
} SRobotMotorSetCfgCmd;

typedef struct SRobotMotorsMoveCmd
{
	SMsgHdr header;
	UINT8  motors;						// bitset
	UINT8  moveId[MOTOR_CNT];
	UINT32 targetPos[MOTOR_CNT];
	UINT32 encoderTol[MOTOR_CNT];
	UINT8  stopBits[MOTOR_CNT];		// bitset per motor
	UINT8  stopBitLevels[MOTOR_CNT];	// one level per motor
} SRobotMotorsMoveCmd;

typedef struct SRobotMotorsStopCmd
{
	SMsgHdr header;
	UINT8 motors;
} SRobotMotorsStopCmd;

typedef struct SRobotMotorsResetCmd
{
	SMsgHdr header;
	UINT8 motors;
} SRobotMotorsResetCmd;

#pragma pack()
