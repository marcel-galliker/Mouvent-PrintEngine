// ****************************************************************************
//
//	DIGITAL PRINTING - cleaf.h
//
//	Communication between ARM processor and FPGA
//
// ****************************************************************************
//
//	Copyright 2016 by radex AG, Switzerland. All rights reserved.
//	Written by Marcel Galliker
//
// ****************************************************************************

#pragma once

#include "rx_sok.h"

void cleaf_init(void);
int  cleaf_menu(void);
void cleaf_main(int ticks, int menu);
	
int  cleaf_handle_ctrl_msg(RX_SOCKET socket, int msgId, void *pdata);


