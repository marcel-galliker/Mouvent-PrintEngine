#pragma once
#include "stdint.h"

#define MOTOR_COUNT				4
#define MOTOR_XY_1				0
#define MOTOR_XY_0				1
#define MOTOR_SCREW				2
#define MOTOR_Z					3

#define POS_UNKNOWN	0
#define POS_UP		1
#define POS_DOWN	2

#define INPUT_COUNT		7
#define OUTPUT_COUNT	4

// Defines
#define SIZE_OF_DATA	256

#define COMMAND_PIPELINE_WIDTH	2
#define COMMAND0				0
#define COMMAND1				1

// Command Masks
#define COMMAND_TYPE_MASK	0xFF000000

#define STATUS_MASK			0x10000000
#define GPIO_MASK			0x20000000
#define MOTOR_MASK			0x30000000
#define COMMUNICATION_MASK	0x40000000
#define MAIN_MASK			0x50000000
#define BOOTLOADER_MASK		0x60000000

// Commands
#define IDLE					0

#define NO_CMD					0

#define STATUS_UPDATE				(STATUS_MASK | 0x00000001)

#define GPIO_SET_OUTPUT				(GPIO_MASK | 0x00000001)

#define MOTORS_DO_REFERENCE			(MOTOR_MASK | 0x00000001)
#define MOTOR_MOVE_Y_RELATIVE		(MOTOR_MASK | 0x00000002)
#define MOTOR_MOVE_X_RELATIVE		(MOTOR_MASK | 0x00000003)
#define MOTOR_TURN_SCREW_RELATIVE	(MOTOR_MASK | 0x00000004)
#define MOTOR_MOVE_Z_UP				(MOTOR_MASK | 0x00000005)
#define MOTOR_MOVE_Z_DOWN			(MOTOR_MASK | 0x00000006)
#define MOTOR_TURN_SCREW_LEFT		(MOTOR_MASK | 0x00000007)
#define MOTOR_TURN_SCREW_RIGHT		(MOTOR_MASK | 0x00000008)
#define MOTOR_ESTOP					(MOTOR_MASK | 0x00000009)
#define MOTOR_SET_SCREW_CURRENT		(MOTOR_MASK | 0x0000000A)



#define BOOTLOADER_INIT_CMD			(BOOTLOADER_MASK | 0x00000001)
#define BOOTLOADER_WRITE_DATA_CMD	(BOOTLOADER_MASK | 0x00000002)
#define BOOTLOADER_CONFIRM_CMD		(BOOTLOADER_MASK | 0x00000003)
#define BOOTLOADER_RESET_CMD		(BOOTLOADER_MASK | 0x00000004)

// Errors
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
#define MOTOR_SCREW_CURRENT_ERROR		(MOTOR_MASK | 0x00000008)
#define MOTOR_STALLED					(MOTOR_MASK | 0x00000009)
#define MOTOR_TIMEOUTED					(MOTOR_MASK | 0x0000000A)
#define MOTOR_COMMAND_FAILED			(MOTOR_MASK | 0x0000000B)

#define COMMUNICATION_USB_READ_ERROR				(COMMUNICATION_MASK | 0x00000001)
#define COMMUNICATION_INVALID_MESSAGE_LENGTH_ERROR	(COMMUNICATION_MASK | 0x00000002)
#define COMMUNICATION_ANSWER_MESSAGE_ERROR			(COMMUNICATION_MASK | 0x00000003)

#define MAIN_FATAL_ERROR							(MAIN_MASK | 0x00000001)
#define MAIN_FATAL_RUNTIME_ERROR					(MAIN_MASK | 0x00000002)
#define MAIN_FATAL_GPIO_TICK_ERROR					(MAIN_MASK | 0x00000003)
#define MAIN_FATAL_MOTOR_TICK_ERROR					(MAIN_MASK | 0x00000004)
#define MAIN_FATAL_USB_TICK_ERROR					(MAIN_MASK | 0x00000005)
#define MAIN_FATAL_COMMUNICATION_TICK_ERROR			(MAIN_MASK | 0x00000006)
#define MAIN_FATAL_WATCHDOG_TIMEOUT_ERROR			(MAIN_MASK | 0x00000007)
#define MAIN_FATAL_CONTROL_TICK_ERROR				(MAIN_MASK | 0x00000008)
#define MAIN_SYSTICK_ERROR							(MAIN_MASJ | 0x00000009)

#define BOOTLOADER_INVALID_CMD_ERROR 	(BOOTLOADER_MASK | 0x00000001)
#define BOOTLOADER_INIT_ERROR			(BOOTLOADER_MASK | 0x00000002)
#define BOOTLOADER_WRITE_DATA_ERROR		(BOOTLOADER_MASK | 0x00000003)
#define BOOTLOADER_CONFIRM_ERROR		(BOOTLOADER_MASK | 0x00000004)
#define BOOTLOADER_RESET_ERROR			(BOOTLOADER_MASK | 0x00000005)



// Data structures
#pragma pack(1)

typedef enum {
	UNINITIALIZED,
	WAITING_FOR_DATA,
	WAITING_FOR_CONFIRM
} EBootloaderStatus;

typedef struct
{
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
    uint8_t screwCurrent;
    uint8_t screwSpeed;
	uint8_t zPos;
	uint8_t isInGarage;
	uint8_t isInRef;
	uint32_t commandRunning[COMMAND_PIPELINE_WIDTH];
	uint32_t commandStep[COMMAND_PIPELINE_WIDTH];
    uint32_t version;
    EBootloaderStatus bootloaderStatus;
} SRobiStatus;

typedef struct
{
	uint32_t command;
	uint32_t id;
	uint16_t length;
	uint8_t data[SIZE_OF_DATA];
} SUsbTxMsg;

typedef struct
{
	uint32_t command;
	uint32_t id;
	uint32_t error;
	SRobiStatus robi;
	uint16_t length;
	uint8_t data[SIZE_OF_DATA];
} SUsbRxMsg;

extern SRobiStatus RX_RobiStatus;




#pragma pack()