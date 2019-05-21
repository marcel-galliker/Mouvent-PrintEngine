// ****************************************************************************
//
//	DIGITAL PRINTING - rx_mac_address.c
//
// ****************************************************************************
//
//	Copyright 2013 by Radex AG, Switzerland. All rights reserved.
//	Written by Marcel Galliker
//
// ****************************************************************************

#include <stdio.h>
#include "rx_mac_address.h"

//--- macAddr_udp -----------------------------------
void macAddr_udp(int udpNo, int serialNo, UINT64 *macAddr)
{
	char str[32];

	sprintf(str, "00:07:ed:%02x:%02x:%02x", 2+udpNo, serialNo/256, serialNo&0xff);	// Altera
//	sprintf(str, "00:0b:eb:%02x:%02x:%02x", 2+udpNo, serialNo/256, serialNo&0xff);	// Systegra
//	sprintf(str, "01:02:03:%02x:%02x:%02x", 2+udpNo, serialNo/256, serialNo&0xff);
//	sprintf(str, "01:07:ed:%02x:%02x:%02x", 2+udpNo, serialNo/256, serialNo&0xff);	// Altera

	*macAddr = mac_as_i64(str);
	// this function does not return a UINT64 (UINT32 instead)
}