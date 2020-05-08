// ****************************************************************************
//
//	step_std.h		Test Table Controol
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

void steps_init(RX_SOCKET psocket);

int	 steps_handle_gui_msg(RX_SOCKET socket, UINT32 cmd, void *data, int dataLen);

int  steps_handle_status(SStepperStat *pStatus);

void steps_set_curing(int curing);

int	 steps_to_print_pos(void);