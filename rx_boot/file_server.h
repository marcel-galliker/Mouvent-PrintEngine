// ****************************************************************************
//
//	DIGITAL PRINTING - (Remote)file_server.h
//
// ****************************************************************************
//
//	Copyright 2014 by Radex AG, Switzerland. All rights reserved.
//	Written by Marcel Galliker
//
// ****************************************************************************

#pragma  once

void fs_start(const char *deviceName);
void fs_end(void);

int  fs_get_port(void);
