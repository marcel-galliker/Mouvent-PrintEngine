// ****************************************************************************
//
//	rx_screen_fms_1x2.h
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
int rx_screen_plane_FMS_1x2s(SPlaneInfo *inplane, SPlaneInfo *outplane, void* pscreenConfig);
int rx_screen_plane_FMS_1x2sx2(SPlaneInfo *inplane, SPlaneInfo *outplane, void* pscreenConfig);
int rx_screen_plane_FMS_1x2sx4(SPlaneInfo *inplane, SPlaneInfo *outplane, void* pscreenConfig);
int rx_screen_init_FMS_1x2s(SScreenConfig* pscreenConfig);

int rx_screen_plane_FMS_1x2g(SPlaneInfo *inplane, SPlaneInfo *outplane, void* pscreenConfig);
int rx_screen_plane_FMS_1x2gx2(SPlaneInfo *inplane, SPlaneInfo *outplane, void* pscreenConfig);
int rx_screen_plane_FMS_1x2gx4(SPlaneInfo *inplane, SPlaneInfo *outplane, void* pscreenConfig);
int rx_screen_init_FMS_1x2g(SScreenConfig* pscreenConfig);

int rx_screen_plane_FMS_1x2r(SPlaneInfo *inplane, SPlaneInfo *outplane, void* pscreenConfig);
int rx_screen_plane_FMS_1x2rx2(SPlaneInfo *inplane, SPlaneInfo *outplane, void* pscreenConfig);
int rx_screen_plane_FMS_1x2rx4(SPlaneInfo *inplane, SPlaneInfo *outplane, void* pscreenConfig);
int rx_screen_init_FMS_1x2r(SScreenConfig* pscreenConfig);


