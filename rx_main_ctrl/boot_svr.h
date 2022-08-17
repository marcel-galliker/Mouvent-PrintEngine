// ****************************************************************************
//
//	boot_srv.h
//
//	main controller tasks.
//
// ****************************************************************************
//
//	Copyright 2013 by Radex AG, Switzerland. All rights reserved.
//	Written by Marcel Galliker 
//
// ****************************************************************************

#pragma once

void boot_start(void);
void boot_end(void);

void boot_request (UINT32 msgId);
void boot_set_flashing(UINT64 macAddr);

int boot_ipsettings_ok(void);