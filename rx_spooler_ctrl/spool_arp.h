// ****************************************************************************
//
//	spool_arp.h
//
// ****************************************************************************
//
//	Copyright 2016 by Radex AG, Switzerland. All rights reserved.
//	Written by Marcel Galliker 
//
// ****************************************************************************

#include "rx_def.h"
#include "rx_sok.h"

void arp_init(void);
int  arp_request(RX_SOCKET socket, UINT32 dst_ip);
