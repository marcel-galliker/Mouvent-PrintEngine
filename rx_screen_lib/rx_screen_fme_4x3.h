// ****************************************************************************
//
//	rx_screen_fme_4x3.h
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
int rx_screen_plane_FME_1x1(SPlaneInfo *inplane, SPlaneInfo *outplane, void* pscreenConfig);
int rx_screen_plane_FME_1x1x2(SPlaneInfo *inplane, SPlaneInfo *outplane, void* pscreenConfig);
int rx_screen_plane_FME_1x1x4(SPlaneInfo *inplane, SPlaneInfo *outplane, void* pscreenConfig);
int rx_screen_init_FME_1x1(SScreenConfig* pscreenConfig);

int rx_screen_plane_FME_1x2g(SPlaneInfo *inplane, SPlaneInfo *outplane, void* pscreenConfig);
int rx_screen_plane_FME_1x2gx2(SPlaneInfo *inplane, SPlaneInfo *outplane, void* pscreenConfig);
int rx_screen_plane_FME_1x2gx4(SPlaneInfo *inplane, SPlaneInfo *outplane, void* pscreenConfig);
int rx_screen_init_FME_1x2g(SScreenConfig* pscreenConfig);

int rx_screen_plane_FME_1x3g(SPlaneInfo *inplane, SPlaneInfo *outplane, void* pscreenConfig);
int rx_screen_plane_FME_1x3gx2(SPlaneInfo *inplane, SPlaneInfo *outplane, void* pscreenConfig);
int rx_screen_plane_FME_1x3gx4(SPlaneInfo *inplane, SPlaneInfo *outplane, void* pscreenConfig);
int rx_screen_init_FME_1x3g(SScreenConfig* pscreenConfig);

