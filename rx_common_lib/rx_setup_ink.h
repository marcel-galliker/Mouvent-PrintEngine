// ****************************************************************************
//
//	DIGITAL PRINTING - Setup_ink.h
//
// ****************************************************************************
//
//	Copyright 2013 by Radex AG, Switzerland. All rights reserved.
//	Written by Marcel Galliker
//
// ****************************************************************************

#pragma once

#include "rx_common.h"
#include "rx_def.h"
#include "rx_setup_file.h"

int setup_ink		 (const char *filepath, SInkDefinition	*pink, EN_setup_Action  action);
