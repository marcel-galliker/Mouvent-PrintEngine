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
void lbrob_display_status(void);	
void lbrob_menu(int help);
void lbrob_handle_menu(char *str);
void lbrob_main(int ticks, int menu);
	
int  lbrob_handle_ctrl_msg(RX_SOCKET socket, int msgId, void *pdata);
void lbrob_reset_variables(void);
void lbrob_to_garage(void);
int _head_screw_pos(void);

