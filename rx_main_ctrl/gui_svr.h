// ****************************************************************************
//
//	GuiSvr.h
//
// ****************************************************************************
//
//	Copyright 2013 by Radex AG, Switzerland. All rights reserved.
//	Written by Marcel Galliker 
//
// ****************************************************************************

#pragma once

#include "rx_common.h"
#include "rx_sok.h"
#include "rx_def.h"

int	gui_start(void);
int	gui_end(void);

void gui_tick(void);

int gui_send_cmd			(UINT32 cmd);
int gui_send_msg			(RX_SOCKET socket, void *msg);
int gui_send_msg_2			(RX_SOCKET socket, UINT32 cmd, int dataSize, void *data);
int gui_send_log			(SLogItem *plog);
int gui_send_print_queue	(UINT32 cmd, SPrintQueueItem *ppq);
int gui_send_printer_status (SPrinterStatus   *stat);

void gui_recv(RX_SOCKET socket);

void gui_test(void);
