// ****************************************************************************
//
//	DIGITAL PRINTING - cond_def_head.h
//
//	Communication between NIOS and Conditioner
//
// ****************************************************************************
//
//	Copyright 2016 by Radex AG, Switzerland. All rights reserved.
//	Written by Stefan Weber
//
// ****************************************************************************

#ifndef _COND_DEF_HEAD_H_
#define _COND_DEF_HEAD_H_

#include "rx_ink_common.h"

#ifdef COND
	#pragma anon_unions
#endif

#define CONDITIONER_FLASH_USER_ADDR 0x200C0000

//--- Error Flags --------------------------------------------------------
	#define COND_ERR_connection_lost			(1<< 0)
    #define COND_ERR_status_struct_missmatch	(1<< 1)
	#define COND_ERR_version					(1<< 2)	// wrong version
	#define COND_ERR_alive						(1<< 3)
	#define COND_ERR_pres_in_hw					(1<< 4) // sensor hardware error
	#define COND_ERR_pres_out_hw				(1<< 5)	// sensor hardware error
	#define COND_ERR_pump_hw					(1<< 6) // sensor hardware error
	#define COND_ERR_temp_tank_not_changing		(1<< 7)
    #define COND_ERR_temp_head_not_changing		(1<< 8)
    #define COND_ERR_temp_head_overheat			(1<< 9)
    #define COND_ERR_temp_ink_overheat			(1<<10)
    #define COND_ERR_temp_heater_overheat		(1<<11)
    #define COND_ERR_flow_factor				(1<<12)
    #define COND_ERR_temp_tank_falling			(1<<13)
    #define COND_ERR_temp_tank_too_low			(1<<14)
    #define COND_ERR_p_in_too_high				(1<<15)
    #define COND_ERR_p_out_too_high				(1<<16)
    #define COND_ERR_pump_no_ink				(1<<17)
    #define COND_ERR_pump_watchdog				(1<<18)
	#define COND_ERR_meniscus					(1<<19)
	#define COND_ERR_sensor_offset				(1<<20)
	#define COND_ERR_download					(1<<21)
	#define COND_ERR_rebooted					(1<<22)
    #define COND_ERR_temp_head_hw				(1<<23)
	#define COND_ERR_temp_heater_hw				(1<<24)
	#define COND_ERR_temp_inlet_hw				(1<<25)
	#define COND_ERR_power_24V					(1<<26)
	#define COND_ERR_power_heater				(1<<27)	
	#define COND_ERR_flush_failed				(1<<28)
	#define COND_ERR_return_pipe				(1<<29)			
	#define COND_ERR_valve						(1<<30)			
	#define COND_ERR_31							(1<<31)			

//--- cmd_ Flags --------------------------------------------------
typedef union SCondInfo
{
	union
	{
		struct
		{
			UINT32	connected : 1;		//	00:
			UINT32	valve_ink   : 1;	//  01:
			UINT32	valve_flush : 1;	// thermistor_test_done : 1;		// 	02:
			UINT32	meniscus_warn : 1;	// 	03:
            UINT32	info_04		: 1;	 // 04:
			UINT32	temp_ready : 1;     //  05:
			UINT32	flowFactor_ok : 1;  // 	06:
			UINT32	info_07 : 1;		// 	07:
			UINT32	info_08 : 1;		// 	08:
			UINT32	info_09 : 1;		// 	09:
			UINT32	info_10 : 1;		// 	10:
			UINT32	info_11 : 1;		// 	11:
			UINT32	info_12 : 1;		// 	12:
			UINT32	info_13 : 1;		// 	13:
			UINT32	info_14 : 1;		// 	14:
			UINT32	info_15 : 1;		// 	15:
			UINT32	info_16 : 1;		// 	16:
			UINT32	info_17 : 1;		// 	17:
			UINT32	info_18 : 1;		// 	18:
			UINT32	info_19 : 1;		// 	19:
			UINT32	info_20 : 1;		// 	20:
			UINT32	info_21 : 1;		// 	21:
			UINT32	info_22 : 1;		// 	22:
			UINT32	info_23 : 1;		// 	23:
			UINT32	info_24 : 1;		// 	24:
			UINT32	info_25 : 1;		// 	25:
			UINT32	info_26 : 1;		// 	26:
			UINT32	info_27 : 1;		// 	27:
			UINT32	info_28 : 1;		// 	28:
			UINT32	info_29 : 1;		// 	29:
			UINT32	info_30 : 1;		// 	30:
			UINT32	info_31 : 1;		// 	31:		
		};
		UINT32 flags;
	};
} SCondInfo;


typedef union SCondStatus
{
	union
	{
		struct
		{
			UINT32 heater_pg:1;			//00
			UINT32 heater_flg:1;		//01
			UINT32 reserve_02:1;		//02
			UINT32 solenoidR_state:1;	//03
			UINT32 u_24v_pg:1;			//04
			UINT32 u_24v_flg:1;			//05
			UINT32 watchdog_reset:1;	//06
			UINT32 reserve_07:1;		//07
			UINT32 reserve_08:1;		//08
			UINT32 reserve_09:1;		//09
			UINT32 reserve_10:1;		//10
			UINT32 reserve_11:1;		//11
			UINT32 reserve_12:1;		//12
			UINT32 reserve_13:1;		//13
			UINT32 reserve_14:1;		//14
			UINT32 reserve_15:1;		//15
			UINT32 reserve_16:1;		//16
			UINT32 reserve_17:1;		//17
			UINT32 reserve_18:1;		//18
			UINT32 reserve_19:1;		//19
			UINT32 reserve_20:1;		//20
			UINT32 reserve_21:1;		//21
			UINT32 reserve_22:1;		//22
			UINT32 reserve_23:1;		//23
			UINT32 reserve_24:1;		//24
			UINT32 reserve_25:1;		//25
			UINT32 reserve_26:1;		//26
			UINT32 reserve_27:1;		//27
			UINT32 reserve_28:1;		//28
			UINT32 reserve_29:1;		//29
			UINT32 reserve_30:1;		//30
			UINT32 reserve_31:1;		//31		
		};
		UINT32 flags;
	};
} SCondStatus;

typedef union SCondCmd
{
	union
	{
		struct
		{
			UINT32 disable_meniscus_check:1;    //00
			UINT32 cmd_1:1;						//01
			UINT32 cmd_2:1;						//02
			UINT32 resetPumpTime:1;				//03
			UINT32 reset_errors:1;				//04
			UINT32 cmd_05:1;					//05
			UINT32 set_pid:1;					//06
			UINT32 save_eeprom:1;				//07
			UINT32 cmd_08:1;		//08
			UINT32 cmd_09:1;		//09
			UINT32 cmd_10:1;		//10
			UINT32 cmd_11:1;		//11
			UINT32 cmd_12:1;		//12
			UINT32 cmd_13:1;		//13
			UINT32 cmd_14:1;		//14
			UINT32 cmd_15:1;		//15
			UINT32 cmd_16:1;		//16
			UINT32 cmd_17:1;		//17
			UINT32 cmd_18:1;		//18
			UINT32 cmd_19:1;		//19
			UINT32 cmd_20:1;		//20
			UINT32 cmd_21:1;		//21
			UINT32 cmd_22:1;		//22
			UINT32 cmd_23:1;		//23
			UINT32 cmd_24:1;		//24
			UINT32 cmd_25:1;		//25
			UINT32 cmd_26:1;		//26
			UINT32 cmd_27:1;		//27
			UINT32 cmd_28:1;		//28
			UINT32 cmd_29:1;		//29
			UINT32 cmd_30:1;		//30
			UINT32 cmd_31:1;		//31		
		};
		UINT32 flags;
	};
} SCondCmd;

typedef struct SConditionerCfg_mcu
{
	//Values set by ARM/NIOS
	UINT32	alive;
	SCondCmd cmd;

	INT32	headsPerColor;			// FOR CALCULATION OF P PARAMETER during start-up phase (OFF->PRINT)
    INT32 	meniscus_setpoint;		// DESIRED MENISCUS PRESSURE (1/10 mbar)
	UINT32 	temp; 					// desired temperature (1/1000 °C)
	UINT32 	tempMax; 				// desired temperature (1/1000 °C)
	UINT32	tempHead;				// measured head temperature
	UINT32	mode;					// EnFluidCtrlMode
	UINT32	volume_printed;			// [ml/min]
	UINT16	flowResistance;
	INT32	purgeDelayPos_y;		// um wait before opening the valve
    UINT32	purgeTime;              // ms the valve is open
    INT32	purgeDelayTime;			// ms delay time for the head
    INT32	purge_pos_y;			// um the position of the cleaning robot referenced to the head 0	
		
	//--- status of fluid system -------------------
    INT32   cylinderPressure;
    INT32   cylinderPressureSet;
    INT32   fluidErr;
	
	//--- Counters ----------------------------------
	UINT32	clusterNo;
	UINT32	clusterTime;
} SConditionerCfg_mcu;

typedef struct
{
	INT16	no;
	INT16	pressure_in;
	INT16	pressure_out;
	INT16	meniscus;
	INT16	pump;
	UINT32	pump_ticks;
} SCondLogItem;

typedef struct SConditionerStat_mcu
{
	//Copy of Conditioner Firmware Version
	SVersion		version;
	
	SCondCmd		cmdConfirm;
		
	//actual values
	INT32			pressure_in;
	INT32			pressure_in_max;
	INT32			pressure_in_diff;
	INT32			pressure_in1;
	INT32			pressure_in2;
	INT32			pressure_out;		
	INT32			pressure_out_diff;
	
	INT32			meniscus;
	INT32			meniscus_setpoint;
	INT32			meniscus_diff;
	UINT16			flowResistance;
	UINT32			flowFactor;
	
	UINT32			pump;				// rpm (calculated based on actual/desired output pressure)
	UINT32			pump_measured;		// measured ml/min
	UINT32			tempIn;				// actual Temp (1/1000 °C)
    UINT32			tempHeater;		    // Temperature measured directly on heater cartridge (>= Revision #h) (1/1000 °C)
	UINT32			heater_percent;		// heater on time (between 0% and 80%)
	INT32			unused_tempReady;	// temp ink = setpoint +/- 1?C
	UINT32			mode;				// EnFluidCtrlMode
	SCondStatus		gpio_state;			// state of GPIO Inputs
	UINT32			error;
	SCondInfo		info;
	UINT32			alive;
	
	UINT32			aliveStat;
	UINT32			aliveCfg;

	UINT32			pid_P;
	UINT32			pid_I;
	INT32			pid_sum;

	//Saved values on Flash
	UINT32 			pumptime;		// count seconds
	INT32			clusterNo;		// stored in head EEPROM!
	UINT32			clusterTime;
	UINT32			machineMeters;
	
	UINT32			logCnt;
	SCondLogItem	log[6];
    
    char            pcb_rev;            // Hardware revision as lower ASCII char
}SConditionerStat_mcu;

#ifdef COND
	extern SConditionerCfg_mcu	RX_Config;
	extern SConditionerStat_mcu RX_Status;
#endif

#endif /* _COND_DEF_HEAD_H_ */
