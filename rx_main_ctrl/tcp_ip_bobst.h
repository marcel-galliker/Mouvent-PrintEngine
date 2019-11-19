// ****************************************************************************
//
//	tcp_ip_bobst.h
//
// ****************************************************************************
//
//	Copyright 2019 by Mouvent AG, Switzerland. All rights reserved.
//	Written by Marcel Galliker 
//
// ****************************************************************************

#pragma once

#include "rx_common.h"
#include "tcp_ip.h"

//--- -------------------------------------------
#define CMD_START_PRINTING		0x01000201
#define CMD_STOP_PRINTING		0x01000203
#define CMD_CHANGE_JOB			0x01000204
#define CMD_ADD_DIST			0x01000205	
#define CMD_ADD_LATERAL			0x01000206

#define PAR_THICKNESS			0x01000301
#define PAR_ENCODER_ADJ			0x01000302

#define EVT_STATE				0x01001000
#define EVT_WARN				0x01001001
#define EVT_ERROR				0x01001002


//--- message header --------------------------------------------
#ifndef TCP_IP_H
typedef struct SMsgHdr
{
	INT32  msgLen;
	UINT32 msgId;
} SMsgHdr;
#endif