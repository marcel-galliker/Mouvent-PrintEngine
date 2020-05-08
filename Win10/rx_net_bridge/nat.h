// ****************************************************************************
//
//	DIGITAL PRINTING - nat.h
//
//
// ****************************************************************************
//
//	Copyright 2017 by Mouvent AG, Switzerland. All rights reserved.
//	Written by Marcel Galliker
//
// ****************************************************************************

#pragma once

#include "NetworkInterface.h"
#include "ndisapi.h"

typedef struct 
{
	HANDLE				hNdisApi;
	TCP_AdapterList		adapterList;
	CNetworkInterface   *netCards;
	int					netCardCnt;
	CNetworkInterface   *providerCard;
	CNetworkInterface   *clientCard;
	struct in_addr		dNSIp;
} SNatThreadArgs;


void nat_start(SNatThreadArgs *pargs);
void nat_end(void);
