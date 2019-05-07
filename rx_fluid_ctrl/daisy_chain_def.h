// ****************************************************************************
//
//	DIGITAL PRINTING - daisy_chain_def.h
//
//	Communication between Fluid and UART connected boards
//
// ****************************************************************************
//
//	Copyright 2018 by Mouvent AG, Switzerland. All rights reserved.
//	Written by Patrick Walther
//
// ****************************************************************************
#pragma once

//--- includes ----------------------------------------------------------------
#include "rx_def.h"

//--- defines -----------------------------------------------------------------

// Device IDs
#define UNDEF_DEVICE_ID			0
#define ADC_DEVICE_ID 			1

#define DEVICE_CNT				1

// Commands
#define DEVICE_DETECT_CMD		0x01

#define SET_ADC_CONFIG 			0x10
#define GET_ADC_CONFIG 			0x11
#define GET_ADC_DATA 			0x12

// Errors
#define SUCCESS_MSG				0x00
#define DEVICE_ID_MISSMATCH 	0x01
#define UNKNOWN_COMMAND_ERROR 	0x02

#define ADC_NO_ANSWER			0x10
#define ADC_INVALID_CONFIG		0x11

#pragma pack(1) // exact fit - no padding

typedef struct
{
	uint8_t dev_nr;
	uint8_t dev_id;
	uint8_t command;
	uint8_t length;
	uint8_t data[128];
} SCommandMsg;

typedef struct
{
	uint8_t dev_ctr;
	uint8_t dev_id;
	uint8_t command;
	uint8_t error;
	uint8_t length;
	uint8_t data[128];
} SAnswerMsg;

//--- devices ---------------------------
#define LOADCELL_CNT	6
typedef struct SDevice_Adc
{
	INT32	weight[LOADCELL_CNT];
	INT32	temp;
} SDevice_Adc; 

#pragma pack() //back to whatever the previous packing mode was 