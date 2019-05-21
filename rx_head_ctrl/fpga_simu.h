// ****************************************************************************
//
//	DIGITAL PRINTING - fpga_simu.h
//
//	Communication between ARM processor and FPGA
//
// ****************************************************************************
//
//	Copyright 2014 by Radex AG, Switzerland. All rights reserved.
//	Written by Marcel Galliker
//
// ****************************************************************************

#pragma once

#include "tcp_ip.h"

int simu_start(void);
int simu_end(void);

int  simu_config		(void);
int  simu_fpga_image	(SFpgaImageCmd *msg);
int  simu_print			(void);
void simu_print_done	(RX_SOCKET socket);

extern int simu_blockCnt;
