
#pragma once

#ifdef __SAME70N20B__
	#include "type_defs.h"
	
	//--- SMsgHdr ------------------------------------
	typedef struct SMsgHdr
	{
		INT32  msgLen;
		UINT32 msgId;
	} SMsgHdr;

#else
	#include "rx_common.h"
	#include "tcp_ip.h"
#endif

#define SETUP_ASSIST_IP_ADDR	"192.168.200.40"
#define SETUP_ASSIST_PORT		1337

#define TCP_MESSAGE_DATA_SIZE	128

#define COMMAND_TYPE_MASK		(0xFF000000)

#define STATUS_COMMAND_MASK		(0x10000000)
#define MOTOR_COMMAND_MASK		(0x20000000)
#define DENSIO_COMMAND_MASK		(0x30000000)

// Status commands
#define CMD_STATUS_GET			(STATUS_COMMAND_MASK | 0x00000001)

// Motor commands
#define CMD_MOTOR_MOVE			(MOTOR_COMMAND_MASK | 0x00000001)
#define CMD_MOTOR_REFERENCE		(MOTOR_COMMAND_MASK | 0x00000002)


// Densio commands
#define CMD_SET_DENSIO_TRIGGER		(DENSIO_COMMAND_MASK | 0x00000001)
#define CMD_RESET_DENSIO_TRIGGER	(DENSIO_COMMAND_MASK | 0x00000002)

#pragma pack(1)

//--- SGenericMessage ---------------------------
typedef struct
{
	SMsgHdr header;
	UINT8 data[TCP_MESSAGE_DATA_SIZE];
} SGenericMessage;

//--- SSetupAssist_StatusMsg ---------------------------------------
typedef struct
{
	SMsgHdr header;
	UINT32	powerStepStatus;	
	INT32	motorPosition;
	UINT32	motorVoltage;
	UINT32	motorMoveCurrent;
	UINT32	motorHoldCurrent;
	UINT8	refDone;
	UINT8	moving;	
	UINT8	inputs;
		#define IN_STOP_LEFT	(1<<0)
		#define IN_STOP_RIGHT	(1<<1)
//	UINT8	outputs;
//	    #define OUT_DENSIO_TRIGGER	(1<<0)
} SSetupAssist_StatusMsg;

//--- SMotorMoveCommand ------------------------
typedef struct
{
	SMsgHdr header;
	INT32 steps;
	UINT32 speed;
	UINT32 acc;
	UINT32 current;
} SetupAssist_MoveCmd;

#pragma pack()
