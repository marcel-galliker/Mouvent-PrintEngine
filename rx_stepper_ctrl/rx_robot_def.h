#pragma once

#include <stdint.h>
#include <stdbool.h>

/* Defines */

// Network defines
#define PORT_UDP_BOOT_SVR		7004
#define PORT_UDP_BOOT_CLNT		7005

#define PORT_UDP_BOOTLOADER		7018
#define PORT_UDP_COMMUNICATION	7017

// GPIO defines
// I/O count
#define NUMBER_OF_INPUTS	7
#define NUMBER_OF_OUTPUTS	4

// Input names
#define	ENDSTOP_Z_DOWN			1
#define ENDSTOP_Z_UP			2
#define ENDSTOP_SCREW_ROTATION	0
#define ENDSTOP_X_AXIS			5
#define ENDSTOP_GARAGE			4
#define ENDSTOP_Y_AXIS			3

// Motor defines
#define MOTOR_COUNT				4
#define MOTOR_XY_0				1
#define MOTOR_XY_1				0
#define MOTOR_SCREW				2
#define MOTOR_Z					3

#define TMC_RAMPSTAT_POS_REACHED_FLAG	0x80
#define TMC_RAMPSTAT_STOP_SG_FLAG		0x40

#define TMC_STATUS_RESET_FLAG				0X01
#define TMC_STATUS_DRIVER_ERROR_FLAG		0x02
#define TMC_STATUS_SG_FLAG					0x04
#define TMC_STATUS_STANDSTILL_FLAG			0x08
#define TMC_STATUS_VELOCITY_REACHED_FLAG	0x10
#define TMC_STATUS_POSITION_REACHED_FLAG	0x20

#define TMC_GCONF_REG			0x00
#define TMC_GSTAT_REG			0x01
#define TMC_IFCNT_REG			0x02
#define TMC_SLAVECONF_REG		0x03
#define TMC_IOIN_REG			0x04
#define TMC_OUTPUT_REG			0x04
#define TMC_XCOMPARE_REG		0x05
#define TMC_IHOLDIRUN_REG		0x10
#define TMC_TPOWERDOWN_REG		0x11
#define TMC_TSTEP_REG			0x12
#define TMC_TPWMTHRS_REG		0x13
#define TMC_TCOOLTHRS_REG		0x14
#define TMC_THIGH_REG			0x15
#define TMC_RAMPMODE_REG		0x20
#define TMC_XACTUAL_REG			0x21
#define TMC_VACTUAL_REG			0x22
#define TMC_VSTART_REG			0x23
#define TMC_A1_REG				0x24
#define TMC_V1_REG				0x25
#define TMC_AMAX_REG			0x26
#define TMC_VMAX_REG			0x27
#define TMC_DMAX_REG			0x28
#define TMC_D1_REG				0x2A
#define TMC_VSTOP_REG			0x2B
#define TMC_TZEROWAIT_REG		0x2C
#define TMC_XTARGET_REG			0x2D
#define TMC_VDCMIN_REG			0x33
#define TMC_SWMODE_REG			0x34
#define TMC_RAMPSTAT_REG		0x35
#define TMC_XLATCH_REG			0x36
#define TMC_ENCMODE_REG			0x38
#define TMC_XENC_REG			0x39
#define TMC_ENCCONST_REG		0x3A
#define TMC_ENCSTATUS_REG		0x3B
#define TMC_ENCLATCH_REG		0x3C
#define TMC_MSLUT0_REG			0x60
#define TMC_MSLUT1_REG			0x61
#define TMC_MSLUT2_REG			0x62
#define TMC_MSLUT3_REG			0x63
#define TMC_MSLUT4_REG			0x64
#define TMC_MSLUT5_REG			0x65
#define TMC_MSLUT6_REG			0x66
#define TMC_MSLUT7_REG			0x67
#define TMC_MSLUTSEL_REG		0x68
#define TMC_MSLUTSTART_REG		0x69
#define TMC_MSCNT_REG			0x6A
#define TMC_MSCURACT_REG		0x6B
#define TMC_CHOPCONF_REG		0x6C
#define TMC_COOLCONF_REG		0x6D
#define TMC_DCCTRL_REG			0x6E
#define TMC_DRVSTATUS_REG		0x6F
#define TMC_PWMCONF_REG			0x70
#define TMC_PWMSCALE_REG		0x71
#define TMC_ENCMCTRL_REG		0x72
#define TMC_LOSTTEPS_REG		0x73

/* Data structures */

#pragma pack(1)

// Bootloader

typedef enum {
	UNINITIALIZED,
	WAITING_FOR_DATA,
	WAITING_FOR_CONFIRM
} EBootloaderStatus;

typedef struct {
	EBootloaderStatus status;
	uint32_t progMemPos;
	uint32_t progMemBlocksUsed;
	uint32_t progSize;
} SBootloaderStatus;

// GPIO
typedef struct {
	uint8_t inputs;
	uint8_t outputs;
	int32_t inputEdges[NUMBER_OF_INPUTS];
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

typedef struct MotorStatus
{
	int32_t motorTargetPos;
	int32_t motorPos;
	int32_t motorVel;
	int32_t motorEncPos;
	uint8_t isMoving;
	uint8_t isStalled;
	uint8_t isConfigured;
	uint8_t status;
} SMotorStatus;

typedef struct MotorConfig
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