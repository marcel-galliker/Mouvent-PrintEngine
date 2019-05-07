// ****************************************************************************
//
//	DIGITAL PRINTING - jet_correction.h
//
//
// ****************************************************************************
//
//	Copyright 2019 by Radex AG, Switzerland. All rights reserved.
//	Written by Marcel Galliker
//
// ****************************************************************************

#pragma once

#include "data.h"

int	 jc_init(void);
void jc_set_disabled_jets(SDisabledJets *jets);

int  jc_active(void);
int	 jc_correction (SBmpInfo *pBmpInfo,  SPrintListItem *pItem, int fromLine);
