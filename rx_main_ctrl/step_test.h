// ****************************************************************************
//
//	step_test.h		Test Table Controol
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

void steptest_init(int no, RX_SOCKET *psocket);

int	 steptest_handle_gui_msg(RX_SOCKET socket, UINT32 cmd, void *data, int dataLen);

int  steptest_handle_status(int no, STestTableStat *pStatus);

int	 steptest_to_print_pos(void);