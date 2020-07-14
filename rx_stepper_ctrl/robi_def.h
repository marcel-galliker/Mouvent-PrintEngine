#pragma once
#include "stdint.h"

#define MOTOR_COUNT				3
#define MOTOR_XY_0				0
#define MOTOR_XY_1				1
#define MOTOR_SCREW				2

#define POS_UNKNOWN	0
#define POS_UP		1
#define POS_DOWN	2

#define INPUT_COUNT		7
#define OUTPUT_COUNT	4

#define SIZE_OF_DATA	64

#define COMMAND_PIPELINE_WIDTH	2
#define COMMAND0 0
#define COMMAND1 1

#define COMMAND_TYPE_MASK	0xFF000000

#define STATUS_MASK	0x10000000
#define GPIO_MASK	0x20000000
#define MOTOR_MASK	0x30000000

#define IDLE					0

#define STATUS_UPDATE			(STATUS_MASK | 0x00000001)

#define GPIO_SET_OUTPUT			(GPIO_MASK | 0x00000001)

#define MOTORS_DO_REFERENCE			(MOTOR_MASK | 0x00000001)
#define MOTOR_MOVE_Y_RELATIVE		(MOTOR_MASK | 0x00000002)
#define MOTOR_MOVE_X_RELATIVE		(MOTOR_MASK | 0x00000003)
#define MOTOR_TURN_SCREW_RELATIVE	(MOTOR_MASK | 0x00000004)
#define MOTOR_MOVE_Z_UP				(MOTOR_MASK | 0x00000005)
#define MOTOR_MOVE_Z_DOWN			(MOTOR_MASK | 0x00000006)
#define MOTOR_TURN_SCREW_LEFT		(MOTOR_MASK | 0x00000007)
#define MOTOR_TURN_SCREW_RIGHT		(MOTOR_MASK | 0x00000008)
#define MOTOR_ESTOP					(MOTOR_MASK | 0x00000009)

#define NO_ERROR						0x00000000
#define INVALIDE_COMMAND_TYPE_ERROR		0x00000001

#define STATUS_INIT_NOT_DONE			(STATUS_MASK | 0x00000001)

#define GPIO_INVALID_PAREMETERS			(GPIO_MASK | 0x00000001)

#define INVALID_MOTOR_CMD_ERROR			(MOTOR_MASK | 0x00000001)
#define MOTOR_CANT_MOVE_X				(MOTOR_MASK | 0x00000002)
#define MOTOR_CANT_MOVE_Y				(MOTOR_MASK | 0x00000003)
#define MOTOR_CANT_TURN_SCREW			(MOTOR_MASK | 0x00000004)
#define MOTOR_CANT_MOVE_Z				(MOTOR_MASK | 0x00000005)
#define MOTOR_CANT_REF					(MOTOR_MASK | 0x00000006)
#define MOTOR_ESTOP_FAILED				(MOTOR_MASK | 0x00000007)



#pragma pack(1)

typedef struct {
	uint8_t inputs;
	uint8_t outputs;
	uint8_t fan;
} SGpioStatus;

typedef struct
{
	int32_t motorTargetPosition;
	int32_t motorPosition;
	int32_t motorVelocity;
	int32_t motorEncoderPosition;
	uint32_t timeout;
	uint8_t isMoving;
	uint8_t isReferenced;
	uint8_t isStalled;
	uint8_t status;
} SMotorStatus;

typedef struct
{
	SGpioStatus gpio;
	SMotorStatus motors[MOTOR_COUNT];
	uint8_t zPos;
	uint8_t isInGarage;
	uint8_t isInRef;
	uint32_t commandRunning[COMMAND_PIPELINE_WIDTH];
	uint32_t commandStep[COMMAND_PIPELINE_WIDTH];
} SRobiStatus;

typedef struct
{
	uint32_t command;
	uint32_t id;
	uint8_t length;
	uint8_t data[SIZE_OF_DATA];
} SUsbTxMsg;

typedef struct
{
	uint32_t command;
	uint32_t id;
	uint32_t error;
	SRobiStatus robi;
	uint8_t length;
	uint8_t data[SIZE_OF_DATA];
} SUsbRxMsg;

#pragma pack()