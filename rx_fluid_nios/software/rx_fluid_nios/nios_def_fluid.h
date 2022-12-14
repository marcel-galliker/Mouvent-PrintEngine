// ****************************************************************************
//
//	DIGITAL PRINTING - nios_def_fluid.h
//
//	Communication between ARM processor and FPGA
//
// ****************************************************************************
//
//	Copyright 2016 by Radex AG, Switzerland. All rights reserved.
//	Written by Marcel Galliker
//
// ****************************************************************************

#pragma once

#include "rx_ink_common.h"

//--- common definitions ------------------
#define NIOS_INK_SUPPLY_CNT	4

//--- Nios Configuration ---------------------------------------------

//--- cmd_ Flags --------------------------------------------------
typedef struct SNiosFluidCmd
{
	UINT32	shutdown:1;			// 	00:	sets the NIOS processor to a save state
	UINT32	error_reset:1;		// 	01: 
	UINT32	logging:1;			// 	02: 
	UINT32	lung_enabled:1;		// 	03:  
	UINT32	lung_needed:1;		// 	04:
	UINT32	cmd_05:1;		// 	05:
	UINT32	cmd_06:1;		// 	06:
	UINT32	cmd_07:1;		// 	07:
	UINT32	cmd_08:1;		// 	08:
	UINT32	cmd_09:1;		// 	09:
	UINT32	cmd_10:1;		// 	10:
	UINT32	cmd_11:1;		// 	11:
	UINT32	cmd_12:1;		// 	12:
	UINT32	cmd_13:1;		// 	13:
	UINT32	cmd_14:1;		// 	14:
	UINT32	cmd_15:1;		// 	15:
	UINT32	cmd_16:1;		// 	16:
	UINT32	cmd_17:1;		// 	17:
	UINT32	cmd_18:1;		// 	18:
	UINT32	cmd_19:1;		// 	19:
	UINT32	cmd_20:1;		// 	20:
	UINT32	cmd_21:1;		// 	21:
	UINT32	cmd_22:1;		// 	22:
	UINT32	cmd_23:1;		// 	23:
	UINT32	cmd_24:1;		// 	24:
	UINT32	cmd_25:1;		// 	25:
	UINT32	cmd_26:1;		// 	26:
	UINT32	cmd_27:1;		// 	27:
	UINT32	cmd_28:1;		// 	28:
	UINT32	cmd_29:1;		// 	29:
	UINT32	cmd_30:1;		// 	30:
	UINT32	cmd_31:1;		// 	31:
} SNiosFluidCmd;

//--- SIsAdapterCfg -------------------------
typedef struct
{
	INT32	present;
	EnFluidCtrlMode	ctrl_mode;
	INT32   cylinderPresSet;
	INT32	meniscusSet;
//	INT32	condPresOutSet;
	INT32	flushTime[3];
	INT32	purgeTime;
	
    INT32	delay_pos_y;
    INT32	act_pos_y;

//	INT32	fluid_PIDsetpoint_P;
//	INT32	fluid_PIDsetpoint_I;
//	INT32	fluid_PIDpump_P;
//	INT32	fluid_PIDpump_I;
	
	UINT32	heaterTemp;
	UINT32	heaterTempMax;
	UINT32	headTemp;

	INT32	alive;

	INT32	condPresIn;		
	INT32	condPresOut;	
	INT32	condMeniscus;
	INT32	condMeniscusDiff;
	UINT32	condPumpSpeed;
	UINT32	condPumpFeedback;
	
	INT32	canisterEmpty;
	
	//--- tests ---------------
	int		test_airValve;
	int		test_bleedValve;
	int		test_cylinderPres;
	//--- purge putty ---------------
	int		purge_putty_pressure;
	
	//--- test bleed line -----
	int		test_bleed_line;	
} SIsAdapterCfg;

//--- SNiosFluidCfg ----------------------------------------
typedef struct SNiosFluidCfg
{
	SNiosFluidCmd	cmd;
	
	INT32			fluid_ctrl_alive;
	INT32			test_airPressure;	// [mbar]
	INT16			test_flush;
	INT16			ctc_valves;
	UINT32			headsPerColor;

	INT32			test_lungPressure;	// [mbar]

	EPrinterType	printerType;
	
	SIsAdapterCfg ink_supply[NIOS_INK_SUPPLY_CNT];
} SNiosFluidCfg;

//*** memory written by NIOS processor ***************************************

//--- cmd_ Flags --------------------------------------------------
typedef struct SNiosFluidInfo
{
	UINT32	is_shutdown:1;	//	00: cmd_shutdown is finiished, NIOS in a save state
	UINT32	info_01 : 1;	// 	01:
	UINT32	info_02 : 1;	// 	02: 
	UINT32	info_03:1;		// 	03:
	UINT32	info_04:1;		// 	04:
	UINT32	info_05:1;		// 	05:
	UINT32	info_06:1;		// 	06:
	UINT32	info_07:1;		// 	07:
	UINT32	info_08:1;		// 	08:
	UINT32	info_09:1;		// 	09:
	UINT32	info_10:1;		// 	10:
	UINT32	info_11:1;		// 	11:
	UINT32	info_12:1;		// 	12:
	UINT32	info_13:1;		// 	13:
	UINT32	info_14:1;		// 	14:
	UINT32	info_15:1;		// 	15:
	UINT32	info_16:1;		// 	16:
	UINT32	info_17:1;		// 	17:
	UINT32	info_18:1;		// 	18:
	UINT32	info_19:1;		// 	19:
	UINT32	info_20:1;		// 	20:
	UINT32	info_21:1;		// 	21:
	UINT32	info_22:1;		// 	22:
	UINT32	info_23:1;		// 	23:
	UINT32	info_24:1;		// 	24:
	UINT32	info_25:1;		// 	25:
	UINT32	info_26:1;		// 	26:
	UINT32	info_27:1;		// 	27:
	UINT32	info_28:1;		// 	28:
	UINT32	info_29:1;		// 	29:
	UINT32	info_30:1;		// 	30:
	UINT32	info_31:1;		// 	31:
} SNiosFluidInfo;

//--- Error Flags --------------------------------------------------------
typedef enum ENiosFluidErr
{
	err_fpga_not_loaded		= 0x00000001,
	err_nios_not_loaded		= 0x00000002,
	err_nios_not_running	= 0x00000004,
	err_fpga_incompatible	= 0x00000008,			
	err_amc_fluid			= 0x00000010,
	err_amc_heater			= 0x00000020,
	err_amc_config_lost 	= 0x00000040,
//	err_overpressure		= 0x00000040,
//	err_inkpres_not_reached = 0x00000080,
	err_watchdog	        = 0x00000100,	
	err_printhead	        = 0x00000200,	
} ENiosFluidErr;

typedef enum ENiosInkSupplyErr
{
    err_ink_tank_pressure = 0x00000001,
    //	err_cal_inkpump_failed	= 0x00000002,
    err_heater_board = 0x00000004,
    err_heater_temp_frozen = 0x00000008,
    err_degasser_clogged = 0x00000010,
    err_degasser_leakage = 0x00000020,
    err_overpressure = 0x00000040,

    // Check 1 sequence
    err_ink_pump = 0x00000080,
    err_ink_tube_clogged = 0x00000100,
    err_bleed_tube_clogged = 0x00000200,
    err_damper = 0x00000400,
    err_ink_tube_suck_air = 0x00000800,
    err_Foam = 0x00001000,
    err_cylinder_empty = 0x00002000,

    // Check 2 sequence
    err_valves_leakage = 0x00004000,
    err_air_valve = 0x00008000,
    err_bleed_valve = 0x00010000,
    err_air_valveOK = 0x00020000,
    err_bleed_valveOK = 0x00040000,
    err_air_valve_tight = 0x00080000,
    err_bleed_valve_tight = 0x00100000,

    // Check 4 sequence
    err_feed_tube = 0x00200000,
    err_check4_timeout = 0x00400000,
    err_filter_clogged = 0x00800000,
    err_return_pipe = 0x01000000,
    err_ink_not_heating = 0x02000000,
    err_ink_too_hot = 0x04000000,

    // Prepare State
    err_pressure_too_high = 0x08000000,
	
} ENiosInkSupplyErr;

//--- SNiosInkSupplyStat -------------------------
typedef struct 
{
	ENiosInkSupplyErr	error;

	EnFluidCtrlMode	ctrl_state;
	
	UINT32	inkPumpSpeed_set;
	UINT32	inkPumpSpeed_measured;
	INT32   cylinderPresSet;
	
	INT32   IS_Pressure_Actual;
	INT32   IS_Pressure_Setpoint;
	INT32	IS_Pressure_Diff;
	INT32   COND_Pressure_Actual;
	INT32   COND_Pressure_Setpoint;
	INT32	PIDsetpoint_Output;
	INT32	PIDpump_Output;
	INT32	PIDairvalve_Output;
	
	INT32	condPresOut;
	INT32	heaterTemp;
	INT32	flushTime;
	
	UINT32  heaterOn:1;
	UINT32	heaterTempReady:1;

	UINT8   airValve;
	UINT8	bleedValve;

	INT32	fluid_PIDsetpoint_P;
	INT32	fluid_PIDsetpoint_I;
	INT32	fluid_PIDpump_P;
	INT32	fluid_PIDpump_I;
	
	INT32	time;
	INT32	diff;	
	
	// test bleed line sequence
	INT32   TestBleedLine_Phase;
	INT32	TestBleedLine_Pump_Phase1;
	INT32	TestBleedLine_Pump_Phase2;
	INT32	TestBleedLine_Pump_Phase3;
	
	// check info
	INT32	Check_State;
	INT32	Check_Time_State;
} SNiosInkSupplyStat;

//--- STATUS ----------------------------------------------------
#define LOG_STR_LEN	256
#define LOG_STR_CNT	50

typedef struct SNiosFluidStat
{
	SVersion        version;
	SVersion		FpgaVersion;
	SNiosFluidInfo	info;
	ENiosFluidErr	error;
	UINT32			qsys_id;
	UINT32			qsys_timestamp;
	
	//--- other status --------------------
	UINT32			alive;

	UINT8	vacuum_solenoid:1;	// 0=? 1=?
	UINT8	air_pressure_solenoid:1;// 0=? 1=?
    UINT8	ctc_valves;
    UINT8	flush_pump:1;		// 0=? 1=?
	UINT8	air_pump:1;			// 0=? 1=?
	UINT32	airPressureTime;

	UINT32	flush_sensorID;
	UINT32	degas_sensorID;
	UINT32	air_sensorID;
	
	INT32	degass_pressure;
	INT32	flush_pressure;
	INT32	flush_pump_val;
	INT32	flush_pres_head;
	INT32	air_pressure;
	INT32	duty_degasser;
	
	UINT32	pt100[8];
	
	UINT32  heater_amc_temp;

	UINT32	p_sensor_error[8];

	UINT32	input;

	SNiosInkSupplyStat ink_supply[NIOS_INK_SUPPLY_CNT];
	
	UINT32	logInIdx;
	UINT32	logOutIdx;
	char	logStr[LOG_STR_CNT][LOG_STR_LEN];
	
	// Vsupply check Heater board
	INT32	HeaterBoard_Vsupply_3V;
	INT32	HeaterBoard_Vsupply_5V;
	INT32	HeaterBoard_Vsupply_24V;
	INT32	HeaterBoard_Vsupply_24VP;
	
	// AMC registers
	INT32	AMC_Register_Power;
	INT32	AMC_Register_GPIO_Config;
	INT32	AMC_Register_Enable;
	INT32	AMC_Register_Gain;	
} SNiosFluidStat;


//*** summary complete memory ************************************************* 
typedef struct
{
	// nios writes to this memory
	union
	{
		SNiosFluidStat  stat;
		UCHAR nios_mem[16*1024];	// 16 KB
	};

	// arm writes to this memory
	union
	{
		SNiosFluidCfg	cfg;
		UCHAR			arm_mem[16*1024];	// 16 KB
	};
} SNiosFluidMemory;

#ifdef NIOS
	extern SNiosFluidStat *pRX_Status;
	extern SNiosFluidCfg  *pRX_Config;
#endif
