
#pragma once

//	#include "type_defs.h"	extern includen!

//--- ICP/IP Addresses -----------------------------------------
#define SETUP_ASSIST_IP_ADDR	"192.168.200.99"
#define SETUP_ASSIST_PORT		1337

//--- COMMANDS ---------------------------------------------------
#define COMMAND_TYPE_MASK		(0xFF000000)

#define STATUS_COMMAND_MASK		(0x10000000)
#define MOTOR_COMMAND_MASK		(0x20000000)
#define DENSIO_COMMAND_MASK		(0x30000000)

// Status commands
#define CMD_STATUS_GET			(STATUS_COMMAND_MASK | 0x00000001)

// Motor commands
#define CMD_MOTOR_REFERENCE		(MOTOR_COMMAND_MASK | 0x00000001)
#define CMD_MOTOR_MOVE			(MOTOR_COMMAND_MASK | 0x00000002)
#define CMD_MOTOR_STOP			(MOTOR_COMMAND_MASK | 0x00000003)


// Densio commands
#define CMD_OUT_SET				(DENSIO_COMMAND_MASK | 0x00000001)
#define CMD_OUT_TRIGGER			(DENSIO_COMMAND_MASK | 0x00000002)

#pragma pack(1)

//--- SMsgHdr ------------------------------------
typedef struct SSAMsgHdr
{
	INT32  msgLen;
	UINT32 msgId;
} SSAMsgHdr;

//--- SSetupAssist_StatusMsg ---------------------------------------
typedef struct
{
	SSAMsgHdr header;
	UINT32	powerStepStatus;	
	INT32	motorPosition;
	UINT32	motorVoltage;
	UINT32	motorMoveCurrent;
	UINT32	motorHoldCurrent;	// woher kommt der Wert? Ist er fix in derf SW, oder sollte er über TCP/IP gesetzt werden?
    UINT8	moveCnt;
	UINT8	refDone;
	UINT8	moving;	
	UINT8	inputs;
		#define IN_STOP_LEFT	(1<<0)
		#define IN_STOP_RIGHT	(1<<1)
} SSetupAssist_StatusMsg;

//--- SMotorMoveCommand ------------------------
typedef struct
{
	SSAMsgHdr header;
	INT32 steps;
	UINT32 speed;
	UINT32 acc;
	UINT32 current;
} SetupAssist_MoveCmd;

//--- SetupAssist_OutSetCmd ------------------------
typedef struct
{
	SSAMsgHdr header;
	UINT8	out;	// needed??
	UINT8	state;	
} SetupAssist_OutSetCmd;

//--- SetupAssist_OutTriggerCmd ------------------------
typedef struct
{
	SSAMsgHdr header;
	UINT8	out;		// needed??
	UINT32	time_ms;	
} SetupAssist_OutTriggerCmd;

#pragma pack()
