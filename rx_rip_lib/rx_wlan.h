// ****************************************************************************
//
//	rx_wlan.h
//
// ****************************************************************************
//
//	Copyright 2017 by Mouvent AG, Switzerland. All rights reserved.
//	Written by Marcel Galliker 
//
// ****************************************************************************

#pragma once

#include "rx_common.h"

#ifdef WIN32
	#define EXPORT EXTERN_C _declspec(dllexport) 
#else 
	#define EXPORT
#endif

typedef struct
{
	UCHAR  ssid[32];
	INT32  bssType;
	UINT32 signalQuality;
} WLAN_NETWORK_INFO;

EXPORT int rx_wlan_get_interfaces(WLAN_INTERFACE_INFO *interfaces, int size);
EXPORT int rx_wlan_get_networks	 (GUID guid,		char *networks, int size, int *pconnected);
EXPORT int rx_wlan_get_profiles	 (GUID guid,		char *profiles,	int size);
EXPORT int rx_wlan_add_profile	 (GUID guid,		char *name, char *password);
EXPORT int rx_wlan_del_profile	 (GUID guid,		char *name);

EXPORT int rx_wlan_connect		 (GUID guid,		char *profile);
EXPORT int rx_wlan_disconnect	 (GUID guid);
