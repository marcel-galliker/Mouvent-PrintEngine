// ****************************************************************************
//
//	DIGITAL PRINTING - txrob.h
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

void txrob_init(void);
int  txrob_menu(void);
void txrob_main(int ticks, int menu);
	
int  txrob_handle_ctrl_msg(RX_SOCKET socket, int msgId, void *pdata);
void txrob_motor_test(int motor, int steps);



