// ****************************************************************************
//
//	DIGITAL PRINTING - dp803.h
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

void dp803_init(void);
int  dp803_menu(void);
void dp803_main(int ticks, int menu);
	
int  dp803_handle_ctrl_msg(RX_SOCKET socket, int msgId, void *pdata);


