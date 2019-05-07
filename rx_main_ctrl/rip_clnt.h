// ****************************************************************************
//
//	rip_clnt.h	Ergosoft Rip Interface
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

// void rip_clnt_init			(void);
// void rip_clnt_end			(void);
void rip_start				();
void rip_send_print_env		(RX_SOCKET socket);
void rip_set_config			(RX_SOCKET socket);
int  rip_file_ripped		(const char *filepath, const char *printEnv, char *dataPath, char *ripState);
int  rip_file				(const char *jobName, const char *filepath, const char *printEnv, int orientation, int columns, int coldist);