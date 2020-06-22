// ****************************************************************************
//
//	rx_screen_fms_1.h
//
// ****************************************************************************
//
//	Copyright 2017 by Radex AG, Switzerland. All rights reserved.
//	Written by Gilbert Therville 
//
// ****************************************************************************

#pragma once

#include "rx_def.h"
#include "rx_screen.h"


// --- Prototypes --------------------------------------------
int rx_screen_plane_FMS_1x1(SPlaneInfo *inplane, SPlaneInfo *outplane, void* pscreenConfig);
int rx_screen_plane_FMS_1x1x2(SPlaneInfo *inplane, SPlaneInfo *outplane, void* pscreenConfig);
int rx_screen_plane_FMS_1x1x4(SPlaneInfo *inplane, SPlaneInfo *outplane, void* pscreenConfig);
int rx_screen_init_FMS_1x1	(SScreenConfig* pscreenConfig);

int rx_screen_init_FMS_1x1r(SScreenConfig* pscreenConfig);




