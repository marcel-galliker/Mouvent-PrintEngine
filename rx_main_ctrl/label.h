// ****************************************************************************
//
//	label.h	
//
// ****************************************************************************
//
//	Copyright 2015 by Radex AG, Switzerland. All rights reserved.
//	Written by Marcel Galliker 
//
// ****************************************************************************

#pragma once

#include "rx_def.h"

void label_reset();
int  label_load(SPrintQueueItem *pitem, char *datapath, int size);
int  label_send_data(SPageId *pid);

void label_rep_file_hdr();
void label_rep_file_block();
