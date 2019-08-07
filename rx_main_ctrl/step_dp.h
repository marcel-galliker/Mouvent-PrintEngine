// ****************************************************************************
//
//	step_dp.h		Test Table Controol
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

void stepdp_init(int no, RX_SOCKET *psocket);

int	 stepdp_handle_gui_msg(RX_SOCKET socket, UINT32 cmd, void *data, int dataLen);

int  stepdp_handle_status(int no, SStepperStat *pStatus);

int	 stepdp_to_print_pos(void);
int  stepdp_abort_printing(void);
int	 stepdp_to_up_pos(void);
