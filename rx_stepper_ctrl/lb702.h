// ****************************************************************************
//
//	DIGITAL PRINTING - lb702.h
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

void lb702_init(void);
int  lb702_menu(void);
void lb702_main(int ticks, int menu);
	
int  lb702_handle_ctrl_msg(RX_SOCKET socket, int msgId, void *pdata);


