// ****************************************************************************
//
//	DIGITAL PRINTING - rx_head_ctrl.h
//
//
// ****************************************************************************
//
//	Copyright 2018 by Mouvent AG, Switzerland. All rights reserved.
//	Written by Marcel Galliker
//
// ****************************************************************************

#pragma once

#include "fpga_def_head.h"
#include "nios_def_head.h"

#define USE_HEAD_PRESOUT	// use PID of the head_presout.xml file
// #undef USE_HEAD_PRESOUT	// use calibrated valies, ignore head_presout.xml file

extern int				RX_LinuxDeployment;
extern RX_SOCKET		RX_MainSocket;
extern UINT32			RX_FpgaCmd;
extern SFpgaHeadStat	RX_FpgaStat;
extern SFpgaHeadStat	RX_FpgaStatRunning;
extern SFpgaHeadError	RX_FpgaError;
extern SFpgaDataStat	RX_FpgaData;
extern SFpgaPrintList	RX_FpgaPrint;
extern SFpgaEncoderCfg	RX_FpgaEncCfg;
extern UINT32			RX_BlockUsed[MAX_HEADS_BOARD];
extern int				RX_RGB[MAX_HEADS_BOARD];
extern char				RX_MacAddr[2][32];
extern char				RX_IpAddr[2][32];

extern SNiosStat		RX_NiosStat;
extern SFluidStateLight	RX_FluidStat[MAX_HEADS_BOARD];
