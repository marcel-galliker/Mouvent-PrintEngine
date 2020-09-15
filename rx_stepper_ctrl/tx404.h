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

void tx404_init(void);
int tx404_menu(void);
void tx404_main(int ticks, int menu);

int tx404_handle_ctrl_msg(RX_SOCKET socket, int msgId, void *pdata);
