// ****************************************************************************
//
//	DIGITAL PRINTING - tx801.h
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

#define TX_PRINT_POS_MIN 1200

void tx801_init(void);
int  tx801_menu(void);
void tx801_main(int ticks, int menu);
	
int  tx801_handle_ctrl_msg(RX_SOCKET socket, int msgId, void *pdata);


