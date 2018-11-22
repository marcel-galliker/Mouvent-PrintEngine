// ****************************************************************************
//
//	step_lb.h		Test Table Controol
//
// ****************************************************************************
//
//	Copyright 2016 by Radex AG, Switzerland. All rights reserved.
//	Written by Marcel Galliker 
//
// ****************************************************************************

#pragma once

#include "rx_def.h"
#include "rx_sok.h"

void steplb_init(int no, RX_SOCKET *psocket, UINT32 ipaddr);

int	 steplb_handle_gui_msg(RX_SOCKET socket, UINT32 cmd, void *data, int dataLen);

int  steplb_handle_status(int no, STestTableStat *pStatus);

int	 steplb_to_print_pos(void);
int  steplb_abort_printing(void);
int	 steplb_to_up_pos(void);
