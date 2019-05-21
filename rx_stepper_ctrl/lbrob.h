// ****************************************************************************
//
//	DIGITAL PRINTING - lbrob.h
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

void lbrob_init(void);
int  lbrob_menu(void);
void lbrob_main(int ticks, int menu);
	
int  lbrob_handle_ctrl_msg(RX_SOCKET socket, int msgId, void *pdata);


