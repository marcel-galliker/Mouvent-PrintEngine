// ****************************************************************************
//
//	data_client.h
//
// ****************************************************************************
//
//	Copyright 2015 by Radex AG, Switzerland. All rights reserved.
//	Written by Marcel Galliker 
//
// ****************************************************************************

#pragma once

int dc_start(RX_SOCKET socket, const char *ipAddr_dataServer);
int dc_end(void);

int dc_is_local(void);
int dc_local_path(const char *filepath, char *localPath);
int dc_cache_file(SPageId *id, char *colorSN, const char *filepath, char *localPath);
int dc_read_file(const char *filepath,  const char *localPath);
int dc_abort(void);