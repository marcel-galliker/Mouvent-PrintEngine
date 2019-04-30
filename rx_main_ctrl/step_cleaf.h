// ****************************************************************************
//
//	step_cleaf.h		Test Table Controol
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

void stepc_init(int stepperNo, RX_SOCKET *psocket);
void stepc_error_reset();

int	 stepc_handle_gui_msg(RX_SOCKET socket, UINT32 cmd, void *data, int dataLen);

int  stepc_handle_status(int stepperNo, STestTableStat *pStatus);

int	 stepc_to_print_pos(void);
int  stepc_abort_printing(void);
int	 stepc_to_up_pos(void);
