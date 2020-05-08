// ****************************************************************************
//
//	DIGITAL PRINTING - prod_log.h
//
// ****************************************************************************
//
//	Copyright 2019 by Mouvent AG, Switzerland. All rights reserved.
//	Written by Marcel Galliker
//
// ****************************************************************************

#pragma once

#include "rx_common.h"
#include "rx_def.h"

void pl_init(void);

void pl_load(void);
void pl_start(SPrintQueueItem *pitem, char *localpath);
void pl_stop(SPrintQueueItem *pitem);
void pl_save(void);
