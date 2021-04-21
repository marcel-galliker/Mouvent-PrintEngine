#pragma once

#include <stdint.h>
#include <stdbool.h>

/* Defines */

// GPIO defines
// I/O count
#define INPUT_CNT	7
#define OUTPUT_CNT	4

// Motor defines
#define MOTOR_CNT	4

/* Data structures */

#pragma pack(1)

// GPIO
typedef struct {
	uint8_t inputs;
	uint8_t outputs;
	int32_t inputEdges[INPUT_CNT];
} SGpioStatus;

// Motor
typedef union SpiStatus
{
	struct
	{
		uint8_t resetFlag 		:1;
		uint8_t driverError 	:1;
		uint8_t sg2 			:1;
		uint8_t standstill 		:1;
		uint8_t velocityReached :1;
		uint8_t positionReached :1;
		uint8_t statusStopL 	:1;
		uint8_t statusStopR 	:1;
	} flags;
	uint8_t statusReg;
} SSpiStatus;

typedef struct
{
	int32_t targetPos;
	int32_t motorPos;
	int32_t encPos;
	int32_t speed;
	uint8_t isMoving;
	uint8_t isStalled;
	uint8_t isConfigured;
	uint8_t status;
	uint8_t	moveIdStarted;
	uint8_t moveIdDone;
} SMotorStatus;

typedef struct SMotorConfig
{
	uint32_t gconf;
	uint32_t swmode;
	uint32_t iholdirun;
	uint32_t tpowerdown;
	uint32_t tpwmthrs;
	uint32_t rampmode;
	uint32_t amax;
	uint32_t dmax;
	uint32_t a1;
	uint32_t d1;
	uint32_t vmax;
	uint32_t v1;
	uint32_t vstart;
	uint32_t vstop;
	uint32_t tzerowait;
	uint32_t vdcmin;
	uint32_t chopconf;
	uint32_t coolconf;
	uint32_t pwmconf;
	uint32_t tcoolthrs;
	uint32_t thigh;
	uint32_t encmode;
	uint32_t encconst;
} SMotorConfig;

#pragma pack()
