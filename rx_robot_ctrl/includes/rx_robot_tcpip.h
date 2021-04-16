#pragma once


#include <stdint.h>
#include <stdbool.h>
#include "rx_robot_def.h"

#ifndef __FT900__
	#include "tcp_ip.h"
#endif

// Network defines
#define PORT_UDP_BOOT_SVR		7004
#define PORT_UDP_BOOT_CLNT		7005

#define PORT_UDP_BOOTLOADER		7018
#define PORT_UDP_COMMUNICATION	7017

// Command masks
#define COMMAND_TYPE_MASK		(0xFF000000)

#define STATUS_COMMAND_MASK		(0x01000000)
#define BOOTLOADER_COMMAND_MASK (0x02000000)
#define GPIO_COMMAND_MASK		(0x04000000)
#define MOTOR_COMMAND_MASK		(0x08000000)

// Status commands
#define CMD_STATUS_GET			(STATUS_COMMAND_MASK | 0x00000001)
#define CMD_TRACE				(STATUS_COMMAND_MASK | 0x00000002)

// Bootloader commands
#define CMD_BOOTLOADER_START	(BOOTLOADER_COMMAND_MASK | 0x00000001)
#define CMD_BOOTLOADER_DATA		(BOOTLOADER_COMMAND_MASK | 0x00000002)
#define CMD_BOOTLOADER_END		(BOOTLOADER_COMMAND_MASK | 0x00000003)
#define CMD_BOOTLOADER_ABORT	(BOOTLOADER_COMMAND_MASK | 0x00000004)
#define CMD_BOOTLOADER_REBOOT	(BOOTLOADER_COMMAND_MASK | 0x00000005)
#define CMD_BOOTLOADER_SERIALNO	(BOOTLOADER_COMMAND_MASK | 0x00000006)

// GPIO commands
#define CMD_GPIO_OUT_SETLOW		(GPIO_COMMAND_MASK | 0x00000001)
#define CMD_GPIO_OUT_SETHIGH	(GPIO_COMMAND_MASK | 0x00000002)
#define CMD_GPIO_IN_RESET		(GPIO_COMMAND_MASK | 0x00000003)

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
	#define UINT16 uint16_t
	#define UINT32 uint32_t
	#define INT32  int32_t
	#define TRUE	true
	#define FALSE	false

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
	char		 version[32];
	UINT16		 serialNo;
	UINT32		 alive;
	SGpioStatus	 gpio;
	SMotorStatus motor[MOTOR_CNT];
} SRobotStatusMsg;

typedef struct SRobotTraceMsg
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

//--- BOOTLOADER -------------------------------------------------------------------
typedef struct SBootloaderStartCmd
{
	SMsgHdr header;
	uint32_t size;
} SBootloaderStartCmd;

typedef struct SBootloaderDataRequestCmd
{
	SMsgHdr header;
	UINT32	filePos;
} SBootloaderDataRequestCmd;

#define BOOTLOADER_DATA_FRAME_SIZE	1024
typedef struct SBootloaderDataCmd
{
	SMsgHdr header;
	UINT32	filePos;
	UINT32  length;
	UINT8	data[BOOTLOADER_DATA_FRAME_SIZE];
} SBootloaderDataCmd;

typedef struct SBootloaderSerialNoCmd
{
	SMsgHdr header;
	UINT16	serialNo;
} SBootloaderSerialNoCmd;

#pragma pack()
