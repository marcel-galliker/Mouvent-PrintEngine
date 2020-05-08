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

int esrip_init				(void);
void esrip_end				(void);
void esrip_map_drive		(const char *path);
void esrip_send_print_env	(RX_SOCKET socket);
int  esrip_file_ripped		(const char *filepath, const char *printEnv, char *dataPath, char *ripState);
int  esrip_rip_file			(RX_SOCKET socket, const char *jobName, const char *filepath, const char *printEnv, int orientation, int columns, int coldist);