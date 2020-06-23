// ****************************************************************************
//
//	rx_screen_fmb_4x3.h
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
int rx_screen_plane_FMB_4x3(SPlaneInfo *inplane, SPlaneInfo *outplane, void* pscreenConfig);
int rx_screen_init_FMB_4x3	(SScreenConfig* pscreenConfig);
