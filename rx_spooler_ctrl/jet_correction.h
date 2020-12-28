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

#include "tcp_ip.h"
#include "data.h"


extern INT16 RX_DisabledJets[MAX_COLORS*MAX_HEADS_COLOR][MAX_DISABLED_JETS];

int	 jc_init(void);
void jc_set_disabled_jets(SDensityMsg *pmsg);

int  jc_active(void);
int	 jc_changed(void);
int	 jc_correction (SBmpInfo *pBmpInfo,  SPrintListItem *pItem, int fromLine);
