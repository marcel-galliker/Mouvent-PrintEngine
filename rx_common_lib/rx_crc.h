// ****************************************************************************
//
//	rx_crc32.h
//
// ****************************************************************************
//
//	Copyright 2015 by Radex AG, Switzerland. All rights reserved.
//	Written by Marcel Galliker 
//
// ****************************************************************************

#pragma once

#include "rx_common.h"

UINT16 rx_crc16(const void *buf, UINT32 size);
UINT32 rx_crc32(const void *buf, UINT32 size);
UINT32 rx_hash32(const char *str);
