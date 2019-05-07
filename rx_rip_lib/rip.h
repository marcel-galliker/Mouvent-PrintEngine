// ****************************************************************************
//
//	rip.h
//
// ****************************************************************************
//
//	Copyright 2013 by Radex AG, Switzerland. All rights reserved.
//	Written by Marcel Galliker 
//
// ****************************************************************************

#pragma once

#include "TCP_IP_OEM.h"
#include "rx_bitmap.h"
#include "rx_rip_lib.h"

void rip_init		(int dpiX, int dpiY, int dotSizes, int monoDropSize);
void rip_end		();
int  rip_set_layout	(SMsgLayout *layout);
int  rip_data		(int x, int y, SMsgPrintData *printData, RX_Bitmap *pBmp, RX_Bitmap *pLabel, RX_Bitmap *pColor, int black);