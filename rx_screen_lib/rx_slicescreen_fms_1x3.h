// ****************************************************************************
//
//	rx_screen_fms_1x3.h
//
// ****************************************************************************
//
//	Copyright 2017 by Radex AG, Switzerland. All rights reserved.
//	Written by Gilbert Therville 
//
// ****************************************************************************

#pragma once

#include "rx_def.h"
#include "rx_slicescreen.h"


// --- Prototypes --------------------------------------------


int rx_planescreen_init_FMS_1x3s(SPlaneScreenConfig* pplaneScreenConfig);
int rx_planescreen_init_FMS_1x3g(SPlaneScreenConfig* pplaneScreenConfig);
int rx_planescreen_init_FMS_1x3r(SPlaneScreenConfig* pplaneScreenConfig);

int rx_screen_slice_FMS_1x3g_mag(SSLiceInfo *inplane, SSLiceInfo *outplane, void * epplaneScreenConfig);
int rx_screen_slice_FMS_1x3_abort(void);
