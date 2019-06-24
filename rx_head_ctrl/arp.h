// ****************************************************************************
//
//	DIGITAL PRINTING - arp.h
//
//
// ****************************************************************************
//
//	Copyright 2013 by Radex AG, Switzerland. All rights reserved.
//	Written by Marcel Galliker
//
// ****************************************************************************

#pragma once

extern long	ARP_RequestCtr[2];
extern long	ARP_ReplyCtr[2];


void arp_init(void);
void arp_main(int ticks, int menu);