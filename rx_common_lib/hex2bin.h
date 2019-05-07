// ****************************************************************************
//
//	hex2bin.h
//
// ****************************************************************************
//
//	Copyright 2015 by Radex AG, Switzerland. All rights reserved.
//	Written by Marcel Galliker
//
// ****************************************************************************

#pragma once

#include "rx_def.h"

void hex2bin(const char *hexfilepath, BYTE *buffer, INT32 bufSize, INT32 *pusedsize);
