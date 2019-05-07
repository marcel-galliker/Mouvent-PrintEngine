// ****************************************************************************
//
//	DIGITAL PRINTING - web.h
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

void lb701_init(void);
int  lb701_menu(void);
void lb701_main(int ticks, int menu);
	
int  lb701_handle_ctrl_msg(RX_SOCKET socket, int msgId, void *pdata);


