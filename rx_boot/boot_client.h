// ****************************************************************************
//
//	DIGITAL PRINTING - boot_client.h
//
// ****************************************************************************
//
//	Copyright 2014 by Radex AG, Switzerland. All rights reserved.
//	Written by Marcel Galliker
//
// ****************************************************************************

#pragma  once

void boot_init(char *commName, char* deviceType, UINT32 serialNo);
void boot_start(void);
void boot_end(void);
