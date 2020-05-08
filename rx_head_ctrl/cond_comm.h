// ****************************************************************************
//
//	DIGITAL PRINTING - cond_comm.h
//
//	Downlaoding the conditioner
//
// ****************************************************************************
//
//	Copyright 2018 by Mouvent AG, Switzerland. All rights reserved.
//	Written by Marcel Galliekr
//
// ****************************************************************************


#pragma once

#include "cond_def_head.h"

int  cond_comm_init(void);
int  cond_comm_end(void);

char *cond_comm_state(int condNo, char *str);
int  cond_comm_version_ok(int condNo);

void cond_comm_send(int condNo, SConditionerCfg_mcu *pcfg);
