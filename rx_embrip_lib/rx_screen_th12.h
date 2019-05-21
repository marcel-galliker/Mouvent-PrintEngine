// ****************************************************************************
//
//	rx_screen_th12.h
//
// ****************************************************************************
//
//	Copyright 2017 by Radex AG, Switzerland. All rights reserved.
//	Written by Gilbert Therville 
//
// ****************************************************************************

#pragma once

#include "rx_def.h"
#include "rx_embrip.h"


// --- Prototypes --------------------------------------------
int bmp_plane_TH12screen	(SHORT plane, SBmpInfo *pinfo, SScreenConfig* pscreenConfig);
int rx_screen_TH12init		(SScreenConfig* pscreenConfig);