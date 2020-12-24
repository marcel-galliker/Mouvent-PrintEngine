// ****************************************************************************
//
//	lh702_tcp_ip.h
//
// ****************************************************************************
//
//	Copyright 2019 by Mouvent AG, Switzerland. All rights reserved.
//	Written by Marcel Galliker 
//
// ****************************************************************************

#pragma once

#include "rx_common.h"

#pragma pack(1)

//--- -------------------------------------------
#define CMD_START_PRINTING		0x01000201
#define CMD_STOP_PRINTING		0x01000203
#define CMD_CHANGE_JOB			0x01000204
#define CMD_ADD_DIST			0x01000205	
#define CMD_ADD_LATERAL			0x01000206
#define CMD_GET_STATE			0x01000207

#define PAR_MATERIAL			0x01000301
#define PAR_THICKNESS			0x01000302
#define PAR_HEAD_HEIGHT			0x01000303
#define PAR_ENCODER_ADJ			0x01000304

#define EVT_STATE				0x01001000
#define EVT_LOG					0x01001001
#define EVT_WARN				0x01001002
#define EVT_ERROR				0x01001003

//--- message header --------------------------------------------
typedef struct SLH702_MsgHdr
{
	INT32  msgLen;
	UINT32 msgId;
} SLH702_MsgHdr;

//--- SLH702_Value --------------------------------------
typedef struct SLH702_Value
{
	SLH702_MsgHdr	hdr;
	// msgId: CMD_ADD_DIST
	// msgId: CMD_ADD_LATERAL
	// msgId: PAR_THICKNESS
	// msgId: PAR_HEAD_HEIGHT
	// msgId: PAR_ENCODER_ADJ
	INT32	value;
} SLH702_Value;

//--- SLH702_Value --------------------------------------
typedef struct SLH702_Str
{
	SLH702_MsgHdr	hdr;
	// msgId: PAR_MATERIAL
	char str[64];
} SLH702_Str;

//--- SLH702_State -----------------------------
typedef struct SLH702_State
{
	SLH702_MsgHdr	hdr;
	// msgId: EVT_STATE
	
	char	material[64];
	UINT32	printState;
		#define PS_OFF		0
		#define PS_STARTING	1
		#define PS_PRINTING	2
	
	INT32	head_height;	// in microns
	INT32	thickness;		// in microns
	INT32	encoder_adj;
	
	INT32	id;
	INT32	dist;			// in microns
	INT32	lateral;		// in microns
	INT32	copies_printed;
	INT32	meters_k;
	INT32	meters_color;
	INT32	meters_color_w;
} SLH702_State;

//--- SLH702_Message --------------------------
typedef struct SLH702_Message
{
	SLH702_MsgHdr	hdr;
	// msgId: EVT_LOG
	// msgId: EVT_WARN
	// msgId: EVT_ERROR
		
	INT32 no;
	char device[64];
	char msg[256];
} SLH702_Message;

#pragma pack()
