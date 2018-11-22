// ****************************************************************************
//
//	DIGITAL PRINTING - balance_def.h
//
//	Communication between Fluid Board and Ventilation/Balance Boards
//
// ****************************************************************************
//
//	Copyright 2016 by Radex AG, Switzerland. All rights reserved.
//	Written by Stefan Weber
//
// ****************************************************************************

#ifndef _VENT_BALANCE_DEF_H
#define _VENT_BALANCE_DEF_H

#include "rx_ink_common.h"

#define BALANCE_CNT	8
#define SENSOR_CNT 3

#define SCALES_CALIBRATION_CNT	12

#pragma pack(1)

//--- Command Flags -----------------------------------------------------
typedef union 
{
	union
	{
		struct
		{		
			UINT16	lamp : 1; 
			UINT16	led_prev_0 : 1; 
			UINT16	led_prev_1 : 1;
			UINT16	led_next_0 : 1;
			UINT16	led_next_1 : 1;
			UINT16	alt_pump_mode_en : 1; 
			UINT16	alt_pump_state_on_off : 1; 
			UINT16	pump_mode_en : 1; 
			UINT16	cmd08 : 1; 
			UINT16	cmd09 : 1; 
			UINT16	cmd10 : 1;
			UINT16	cmd11 : 1;
			UINT16	cmd12 : 1;
			UINT16	cmd13 : 1;
			UINT16	cmd14 : 1;
			UINT16	cmd15 : 1;
		};
		UINT16 flags;
	};
} SBallanceCmd;

//--- Config -------------------------------------------------------------

typedef struct 
{
	SBallanceCmd	cmd;
} SBalanceCfg;

//--- Error Flags --------------------------------------------------------
typedef union 
{
	union
	{
		struct
		{		
			UINT16	pump  : 1; 
			UINT16	invalid_mode_setting : 1; 
			UINT16	err02 : 1;
			UINT16	err03 : 1;
			UINT16	err04 : 1; 
			UINT16	err05 : 1; 
			UINT16	err06 : 1; 
			UINT16	err07 : 1; 
			UINT16	err08 : 1; 
			UINT16	err09 : 1; 
			UINT16	err10 : 1;
			UINT16	err11 : 1;
			UINT16	err12 : 1;
			UINT16	err13 : 1;
			UINT16	err14 : 1;
			UINT16	err15 : 1;
		};
		UINT16 flags;
	};
} SBalanceError;

typedef struct SBalanceStat_alt_pump
{
    union
    {
        struct
        {
            BYTE    alt_pump_mode_is_active   :1;
            BYTE    alt_pump_state_off   :1;
            BYTE    alt_pump_state_on   :1;
            BYTE    alt_pump_mode_supported   :1;
            BYTE    pump_mode_is_active   :1;
            BYTE    stat5   :1;
            BYTE    stat6   :1;
            BYTE    stat7   :1;        
        };
        BYTE flags;
    };   
}SBalance_alt_pump;
typedef struct SBalanceStat
{
    UCHAR			board_revision;
	SBalance_alt_pump alt_stat;
	//--- inputs -----
	SBalanceError	err;
	UINT16			alive;
	UINT16			temp;
	UINT16			balance[SENSOR_CNT];
	INT16			humidity;
	BYTE			prev_msg_received;
	BYTE			prev_msg_send;
	BYTE			next_msg_received;
	BYTE			next_msg_sent;
} SBalanceStat;

//--- SScalesCalibration -------------------------------------
typedef struct SScalesCalibration
{
	UINT16			value_0Kg [SENSOR_CNT];
	UINT16			value_10Kg[SENSOR_CNT];
} SScalesCalibration;

extern SScalesCalibration RX_ScalesCalibration[SCALES_CALIBRATION_CNT];
	
#pragma pack()

#endif /* _COND_DEF_HEAD_H_ */
