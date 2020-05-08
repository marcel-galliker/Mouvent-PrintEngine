// ****************************************************************************
//
//	DIGITAL PRINTING - bridge.h
//
//
// ****************************************************************************
//
//	Copyright 2017 by Mouvent AG, Switzerland. All rights reserved.
//	Written by Marcel Galliker
//
// ****************************************************************************

#pragma once

#include "stdafx.h"
#include "ndisapi.h"

int bridge_init(char *provider, char *client);
int bridge_end (void);
