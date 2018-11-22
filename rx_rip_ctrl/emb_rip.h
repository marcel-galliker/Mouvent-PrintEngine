// ****************************************************************************
//
//	es_rip.h	Ergosoft Rip Interface
//
// ****************************************************************************
//
//	Copyright 2014 by Radex AG, Switzerland. All rights reserved.
//	Written by Marcel Galliker 
//
// ****************************************************************************

#pragma once

#include "rx_sok.h"
#include "rx_def.h"

#define REPLY_NOT_RIPPED	1

void embrip_init			(void);
void embrip_end(void);
void embrip_send_print_env(RX_SOCKET socket);
int  embrip_rip_file(RX_SOCKET socket, const char *jobName, const char *filepath, const char *printEnv, int orientation, int columns, int coldist);