// ****************************************************************************
//
//	DIGITAL PRINTING - Fpga_def.h
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

#include "rx_common.h"

//*** main address mapping *****************************************************

#define FPGA_BASE_ADDR	0xC0000000UL

#define FPGA_CFG_ADDR	(FPGA_BASE_ADDR + 0x10000000UL)	// SFpgaEncCfg
#define FPGA_STAT_ADDR 	(FPGA_BASE_ADDR + 0x20000000UL)	// SFpgaEncStat


//*** structures ****************************************************************
#define MAX_ENC_IN	3
#define MAX_ENC_OUT	8

#define  MAX_STAT_VALUES	100000

//=== ENCODER CONFIGURATION ======================================================

//--- Encoder Commands -----------------------------------------------------
typedef struct SEncCmd
{
	UINT16	enable:1;
	UINT16	error_reset:1;
	UINT16	enable_statistics;
	UINT16	cmd_3;
	UINT16  cmd_4;
	UINT16  cmd_5;
	UINT16  cmd_6;
	UINT16  cmd_7;
	UINT16  cmd_8;
	UINT16  cmd_9;
	UINT16  cmd10;
	UINT16  cmd11;
	UINT16  cmd12;
	UINT16  cmd13;
	UINT16  cmd14;
	UINT16  cmd15;
	UINT16  cmd16;
	UINT16  cmd17;
	UINT16  cmd18;
	UINT16  cmd19;
	UINT16  cmd20;
	UINT16  cmd21;
	UINT16  cmd22;
	UINT16  cmd23;
	UINT16  cmd24;
	UINT16  cmd25;
	UINT16  cmd26;
	UINT16  cmd27;
	UINT16  cmd28;
	UINT16  cmd29;
	UINT16  cmd30;
	UINT16  cmd31;
} SEncCmd;

//--- ENEncOrientation -------------------------------------------
typedef enum ENEncOrientation
{
	undef,
	cw,
	ccw,
} ENEncOrientation;

//--- SEncInCfg -----------------------------------------------
typedef struct SEncInCfg
{
	SEncCmd				cmd;
	UINT32				inc_meter;	// increments per meter
	ENEncOrientation	orientation;
} SEncInCfg;

//--- SEncOutCfg -----------------------------------------------
typedef struct SEncOutCfg
{
	UINT32	in;			// input encoder
	UINT32	frequence;	// [Hz] 0=external encoder
	UINT32	dpi;		// defines distance of firepulse strokes
	UINT8	subStrokes;	// number of substrokes per firulse stroke
} SEncOutCfg;


//--- ENCompMode -----------------------------------------------
typedef enum ENCompMode
{
	unused,		// no compensation
	type1,
	type2,
	type3,
	type4,
} ENCompMode;

//--- SCompCfg ------------------------------------------------
typedef struct SCompCfg
{
	UINT32	mode;
	UINT32	par0;
	UINT32	par1;
	UINT32	par2;
	UINT32	par3;
} SCompCfg;

//--- SScanCfg ------------------------------------------------
typedef struct SScanCfg
{
	UINT32	enable;
	UINT32	pg_fwd;		// [µm] print go position forward
	UINT32 	pg_bwd;		// [µm] print go position backward
} SScanCfg;

//--- ENPgMode ------------------------------------------------
typedef enum ENPgMode
{
	undef,
	pg_external,
	pg_internal_fixed,
	pg_interlal_variable,
} ENPgMode;

typedef struct SPrintGoVarDist
{
	UINT8		inIdx;		// index of next item to be set by ARM
	UINT8		outIdx;		// index of last used item
	UINT32		dist[256];	// [µm] from last printGo
} SPrintGoVarDist;

//--- SPrintGoCfg ---------------------------------------------
typedef struct SPrintGoCfg
{
	ENPgMode	mode;

	//--- external PrintGo ---------------------------------
	UINT32		inputNo;	// when using external PrintGo (PG)
	UINT32		filter;		// [µm] when using mode==pg_external

	//--- internal PrintGo ---------------------------------
	UINT32			pg_pos;		// [µm] encoder position for the first PrintGo [-1=external PG]
	UINT32			dist;		// [µm] distance between the PrintGos (fixed internal)
	SPrintGoVarDist varDist;
} SPrintGoCfg;

//--- SFpgaEncCfg ------------------------------------------------
typedef struct SFpgaEncCfg
{
	SEncInCfg	enc_in[MAX_ENC_IN];
	SEncOutCfg	enc_out[MAX_ENC_OUT];
	SPrintGoCfg	pg[MAX_ENC_OUT];
} SFpgaEncCfg;

//=== EOCNDER STATUS ===========================================

//--- Info Flags --------------------------------------------------
typedef struct SFpgaEncInfo
{
	UINT32	info_0:1;
	UINT32	info_1:1;
	UINT32	info_2:1;
	UINT32	info_3:1;
	UINT32	info_4:1;
	UINT32	info_5:1;
	UINT32	info_6:1;
	UINT32	info_7:1;
	UINT32	info_8:1;
	UINT32	info_9:1;
	UINT32	info10:1;
	UINT32	info11:1;
	UINT32	info12:1;
	UINT32	info13:1;
	UINT32	info14:1;
	UINT32	info15:1;
	UINT32	info16:1;
	UINT32	info17:1;
	UINT32	info18:1;
	UINT32	info19:1;
	UINT32	info20:1;
	UINT32	info21:1;
	UINT32	info22:1;
	UINT32	info23:1;
	UINT32	info24:1;
	UINT32	info25:1;
	UINT32	info26:1;
	UINT32	info27:1;
	UINT32	info28:1;
	UINT32	info29:1;
	UINT32	info30:1;
	UINT32	info31:1;
} SFpgaEncInfo;

//--- Error Flags --------------------------------------------------
typedef struct SFpgaEncErr
{
	UINT32	err_0:1;
	UINT32	err_1:1;
	UINT32	err_2:1;
	UINT32	err_3:1;
	UINT32	err_4:1;
	UINT32	err_5:1;
	UINT32	err_6:1;
	UINT32	err_7:1;
	UINT32	err_8:1;
	UINT32	err_9:1;
	UINT32	err10:1;
	UINT32	err11:1;
	UINT32	err12:1;
	UINT32	err13:1;
	UINT32	err14:1;
	UINT32	err15:1;
	UINT32	err16:1;
	UINT32	err17:1;
	UINT32	err18:1;
	UINT32	err19:1;
	UINT32	err20:1;
	UINT32	err21:1;
	UINT32	err22:1;
	UINT32	err23:1;
	UINT32	err24:1;
	UINT32	err25:1;
	UINT32	err26:1;
	UINT32	err27:1;
	UINT32	err28:1;
	UINT32	err29:1;
	UINT32	err30:1;
	UINT32	err31:1;
} SFpgaEncErr;

//--- SFpgaEncStatistics ------------------------------------------
//-- used to store history of encoder strokes for analysis and implementation of compensation algorithmes.
typedef struct SFpgaEncStatistics
{
	UINT32		cnt;
	UINT32		time[MAX_STAT_VALUES][MAX_ENC_IN];
} SFpgaEncStatistics;

//--- SFpgaHeadStat -----------------------------------------------
typedef struct SFpgaEncStat
{
	SFpgaEncInfo		info;
	SFpgaEncErr			err;
	UINT32				position[MAX_ENC_IN];	// [µm]
	INT32				speed[MAX_ENC_IN];		// [µm/sec]
	UINT32				encoderPosFS;			// in Fireuulse Strokes [FS]
	UINT8				encoderPosFSS;			// firepulse substroke [FSS]
	INT32				freqFS;					// [Hz] frequence of fire stroke
	SFpgaEncStatistics	statistics;
} SFpgaEncStat;

//--- SEncoderStrokeMsg ---------------------------------------------
// this message is sent via the M-LSVD channel
// typedef for documentation only!
typedef struct SEncoderStrokeMsg
{
	UINT32  no:3;			//	Encoder number [0..7]
	UINT32  reserved_1:1;	//
	UINT32  pos:20;			//	Position in 1200 dpi units. [0..22 m]
	UINT32  sub_pos:4;      //	Positions within one 1200 dpi stroke. [0..16]?      
	UINT32	print_go:1;     //	Flag, indicating print-go. Is high for all sub strokes in the triggering position (redundancy). Is evaluated on sub_pos=0.
	UINT32  direction:1;    //	Flag, used in scanning mode,  defines the printing direction
	UINT32  reserved_2:1;	//
	UINT32  reserved_3:1;	//

	UINT8   chk:4;			//	checksum
	UINT8	reserved_4:4;	//	not used, not transmitted
} SEncoderStrokeMsg;
