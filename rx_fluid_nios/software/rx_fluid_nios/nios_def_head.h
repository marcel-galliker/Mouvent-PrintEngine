// ****************************************************************************
//
//	DIGITAL PRINTING - nios_def_head.h
//
//	Communication between ARM processor and FPGA
//
// ****************************************************************************
//
//	Copyright 2013 by Radex AG, Switzerland. All rights reserved.
//	Written by Marcel Galliker
//
// ****************************************************************************

#pragma once

//--- basic data types --------------------------
#include "cond_def_head.h"

//--- common definitions ------------------
#define MAX_HEADS_BOARD			4
#define EEPROM_DATA_SIZE		128
#define EEPROM_MAX_ADR			4096
#define EEPROM_USER_DATA_START	512

#define EEPROM_MVT_ADDR			  0
#define EEPROM_DENSITY_ADDR		128

#define EXE_BLOCK_SIZE			512

//--- Nios Configuration ---------------------------------------------
typedef struct SExecutable
{
	UINT16	size;
	UINT16	blkAddr;
	BYTE	blkData[EXE_BLOCK_SIZE];
} SExecutable;

#define MAX_DISABLED_JETS	32
#define MAX_DENSITY_VALUES	(2+10)

#pragma pack(1)
// eeprom page size: 32 Bytes
typedef struct
 {
	INT16	angle;	// 0=undef
	INT16	dist;	// 0=undef
	UINT8	crc;
 } SRobInfo;

typedef struct
{
	UINT16	clusterNo;				// 0x00
	UINT16	flowResistance;			// 0x02
	UINT8	flowResistanceCRC;		// 0x04
	UINT32	_dropletsPrinted_old;	//  0x05..0x08
	UINT8	_dropletsPrintedCRC_old;	//	0x09
	INT16	_disabledJets[MAX_DISABLED_JETS];	// 0x0a..0x4b
	UINT8	_disabledJetsCRC;					//	0x4a
	INT16	_densityValue[MAX_DENSITY_VALUES];	// 0x4b..0x62
	UINT8	_densityValueCRC;					// 0x63
	UINT8	_voltage;							// 0x64: Firepulse voltage
	UINT8	_voltageCRC;						// 0x65
	UINT64	dropletsPrinted;					// 0x66..0x6d
	UINT8	dropletsPrintedCRC;					// 0x6e
	SRobInfo	robot;
	UINT8	filler[12];
} SHeadEEpromMvt;	// size must be 0x80!!

typedef struct // size must be a multiple of 32!
{
	INT16	densityValue[MAX_DENSITY_VALUES];	// 0x00..0x17
	INT16	disabledJets[MAX_DISABLED_JETS];	// 0x18..0x57
	UINT8	voltage;							// 0x58: Firepulse voltage
	UINT8	filler[6];
	UINT8	crc;								// 0x59
} SHeadEEpromDensity;

#pragma pack()

//*** memory written by ARM processor ****************************************

//--- cmd_ Flags --------------------------------------------------
typedef union SNiosHeadCmd
{
	UINT32 cmd;
	struct
	{		
		UINT32	shutdown:1;				// 	00:	sets the NIOS processor to a save state
		UINT32	error_reset:1;			// 	01: 
		UINT32	exe_valid:1;			// 	02:
		UINT32	firepulse_on:1;			// 	03:

		UINT32	cmd_04:1;	// 	04:
		UINT32	cmd_05:1;	// 	05:
		UINT32	cmd_06:1;	// 	06:
		UINT32	cmd_07:1;	// 	07:

		UINT32	cmd_08:1;	// 	08:
		UINT32	cmd_09:1;				// 	09:
		UINT32	cmd_10:1;				// 	10:
		UINT32	cmd_11:1;				// 	11:

		UINT32	debug:1;				// 	12:
		UINT32	cmd_13:1;				// 	13:
		UINT32	cmd_14:1;				// 	14:
		UINT32	cmd_15:1;				// 	15:
		UINT32	cmd_16:1;				// 	16:
		UINT32	cmd_17:1;				// 	17:
		UINT32	cmd_18:1;				// 	18:
		UINT32	cmd_19:1;				// 	19:
		UINT32	cmd_20:1;				// 	20:
		UINT32	cmd_21:1;				// 	21:
		UINT32	cmd_22:1;				// 	22:
		UINT32	cmd_23:1;				// 	23:
		UINT32	cmd_24:1;				// 	24:
		UINT32	cmd_25:1;				// 	25:
		UINT32	cmd_26:1;				// 	26:
		UINT32	cmd_27:1;				// 	27:
		UINT32	cmd_28:1;				// 	28:
		UINT32	cmd_29:1;				// 	29:
		UINT32	cmd_30:1;				// 	30:
		UINT32	cmd_31:1;				// 	31:			
	};
} SNiosHeadCmd;

typedef struct SNiosCfg
{
	SNiosHeadCmd	cmd;
	SConditionerCfg_mcu	cond[MAX_HEADS_BOARD];
	BYTE			eeprom_fuji_readCnt[MAX_HEADS_BOARD];
	BYTE			eeprom_mvt_readCnt[MAX_HEADS_BOARD];
    BYTE			eeprom_mvt_writeCnt[MAX_HEADS_BOARD];
	BYTE			eeprom_density_readCnt[MAX_HEADS_BOARD];
	BYTE			eeprom_density_writeCnt[MAX_HEADS_BOARD];

	union
	{
		SExecutable		exe;
		struct 
		{
			SHeadEEpromMvt			eeprom_mvt[MAX_HEADS_BOARD];
			SHeadEEpromDensity		eeprom_density[MAX_HEADS_BOARD];
		};
	};
} SNiosCfg;

//*** memory written by NIOS processor ***************************************

//--- cmd_ Flags --------------------------------------------------
typedef struct SNiosHeadInfo
{
	UINT32	is_shutdown:1;		//	00: cmd_shutdown is finiished, NIOS in a save state
	UINT32	wf_ready:1;			//	01: wave form is ready to fire
	UINT32	nios_ready:1;		// 	02:
	UINT32	watchdog_dbg_mode:1;// 	03:
	UINT32	watchdog_catch_fp_err:1;		// 	04: if the correct AMC IS is read out
	UINT32	cooler_pcb_present:1;		// 	05: 
	UINT32	u_firepulse_48V:1;		// 	06:
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
	UINT32	info_17:1;		//	17:
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
} SNiosHeadInfo;

//--- Error Flags --------------------------------------------------------
typedef struct SNiosHeadErr
{
	union
	{
		struct
		{
			UINT32	fpga_incompatible:1;	// 00 fpga not compatible (QSYS-ID or TimeStamp)
			UINT32	u_plus_2v5:1;			// 01
			UINT32	u_plus_3v3:1;			// 02
			UINT32	u_plus_5v:1;			// 03
			UINT32	u_minus_5v:1;			// 04
			UINT32	u_firepulse:1;			// 05
			UINT32	fp_ac:1;				// 06
			UINT32	fp_dc:1;				// 07
			UINT32	head_pcb_overheated:1;	// 08
			UINT32	arm_timeout:1;			// 09
			UINT32	cooler_temp_hw:1;		// 10
			UINT32	amc7891:1;				// 11
			UINT32	cooler_overheated:1;	// 12
			UINT32	cooler_pressure_hw:1;	// 13
			UINT32	cooler_overpressure:1;	// 14
			UINT32	head_eeprom_read:1;		// 15
			UINT32	watchdog:1;				// 16
			UINT32	fpga_overheated:1;		// 17
			UINT32	power_all_off_timeout:1;// 18
			UINT32	power_amp_on_timeout:1;	// 19
			UINT32	power_all_on_timeout;	// 20
			UINT32	err_21:1;				// 21
			UINT32	no_head_therm:1;    	// 22
			UINT32	head_eeprom_write:1;	// 23
			UINT32	err_24:1;				// 24
			UINT32	err_25:1;				// 25
			UINT32	err_26:1;				// 26
			UINT32	err_27:1;				// 27
			UINT32	err_28:1;				// 28
			UINT32	err_29:1;				// 29
			UINT32	err_30:1;				// 30
			UINT32	err_31:1;				// 31
		};
		UINT32 err;
	};
} SNiosHeadErr;

typedef enum
{
	power_off,			// 0
	power_wait_all_off,	// 1
	power_wait_amp_on,	// 2
	power_wait_all_on,	// 3
	power_pre_all_on,	// 4	
	power_all_on,		// 5	
	power_sd_fpga,		// 6
	power_sd,			// 7
	power_sd_3v3,		// 8
	power_sd_amp,		// 9
	power_down			// 10	
} EnPowerState;

char *PowerStateStr(EnPowerState state);


//--- STATUS  -----------------------------------------------
typedef struct SNiosStat
{
	SVersion        version;
	SNiosHeadInfo	info;
	SNiosHeadErr	error;
	INT32			QSYS_id;
	INT32			QSYS_timestamp;
	BYTE			eeprom_fuji_readCnt[MAX_HEADS_BOARD];
	BYTE			eeprom_fuji[MAX_HEADS_BOARD][EEPROM_DATA_SIZE];
	BYTE			eeprom_mvt_readCnt[MAX_HEADS_BOARD];
    BYTE			eeprom_mvt_writeCnt[MAX_HEADS_BOARD];
	SHeadEEpromMvt	eeprom_mvt[MAX_HEADS_BOARD];
	BYTE			eeprom_density_readCnt[MAX_HEADS_BOARD];
	BYTE			eeprom_density_writeCnt[MAX_HEADS_BOARD];
	INT32			eeprom_density_writeRes[MAX_HEADS_BOARD];

	SHeadEEpromDensity	eeprom_density[MAX_HEADS_BOARD];
	//--- other status --------------------
	UINT32			alive;
	INT32			u_firepulse;
	INT32			u_minus_5v;
	INT32			u_plus_5v;
	INT32			u_plus_2v5;
	UINT32			fp_ac_error;
	UINT32			fp_dc_error;
	UINT32			head_con_3v3_error;
	UINT32			voltage_error;
	EnPowerState	powerState;

	//--- Temp. Sensors -------------------
	UINT32			headcon_amc_temp;			// Head Connection Board AMC internal Temp. Sensor
	UINT32			head_temp[MAX_HEADS_BOARD];	// Print Head internal Temp. Sensor

	//--- cooler ----------------
	UINT32 			cooler_temp;
	INT32			cooler_pressure;
	UINT32			cooler_pressure_reset_cnt;
	
	//--- Conditioner State
	SConditionerStat_mcu cond[MAX_HEADS_BOARD];
		
	//--- programming -------------------
	INT32			memSize;
	UINT32			condDnlNo;
	EnDownloadState	condDnlState;
	UINT16			condDnlBlkAddr;	
} SNiosStat;


//*** summary complete memory ************************************************* 
typedef struct
{
	SNiosStat  stat;
	SNiosCfg   cfg;
} SNiosMemory;

#ifdef NIOS
	extern SNiosCfg		*pRX_Config;
	extern SNiosStat	*pRX_Status;
#endif
