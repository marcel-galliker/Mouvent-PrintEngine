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
void jc_set_disabled_jets(SDisabledJetsMsg *pmsg);

int  jc_active(void);
int	 jc_changed(void);
// int	 jc_correction (SBmpInfo *pBmpInfo,  SPrintListItem *pItem, int fromLine, const char *dots);
int	 jc_correction (SPrintListItem *pItem, int fromLine, const char *dots);
int getmaxdropsize(const char *dot);
void jc_head_correct(SBmpSplitInfo *pInfoL, SBmpSplitInfo *pInfo, SBmpSplitInfo *pInfoR, short *disabledJets, int fromLine, int lengthPx, int lineLen, int maxdropsize);
