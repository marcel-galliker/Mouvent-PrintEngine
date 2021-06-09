// ****************************************************************************
//
//	DIGITAL PRINTING - sa.h	setup-assist
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

void sa_init(void);
int  sa_menu(void);
void sa_main(int ticks, int menu);
	
int  sa_handle_ctrl_msg(RX_SOCKET socket, void *pmsg);


