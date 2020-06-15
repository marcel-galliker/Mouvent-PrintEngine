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
int rx_screen_plane_FMS_1x3s(SPlaneInfo *inplane, SPlaneInfo *outplane, void* pscreenConfig);
int rx_screen_plane_FMS_1x3sx2(SPlaneInfo *inplane, SPlaneInfo *outplane, void* pscreenConfig);
int rx_screen_plane_FMS_1x3sx4(SPlaneInfo *inplane, SPlaneInfo *outplane, void* pscreenConfig);
int rx_screen_init_FMS_1x3s	(void* pscreenConfig);

int rx_screen_plane_FMS_1x3g(SPlaneInfo *inplane, SPlaneInfo *outplane, void* pscreenConfig);
int rx_screen_plane_FMS_1x3gx2(SPlaneInfo *inplane, SPlaneInfo *outplane, void* pscreenConfig);
int rx_screen_plane_FMS_1x3gx4(SPlaneInfo *inplane, SPlaneInfo *outplane, void* pscreenConfig);
int rx_screen_init_FMS_1x3g(void* pscreenConfig);

int rx_screen_plane_FMS_1x3r(SPlaneInfo *inplane, SPlaneInfo *outplane, void* pscreenConfig);
int rx_screen_plane_FMS_1x3rx2(SPlaneInfo *inplane, SPlaneInfo *outplane, void* pscreenConfig);
int rx_screen_plane_FMS_1x3rx4(SPlaneInfo *inplane, SPlaneInfo *outplane, void* pscreenConfig);
int rx_screen_init_FMS_1x3r(void* pscreenConfig);

