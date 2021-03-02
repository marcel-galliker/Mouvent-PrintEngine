// ****************************************************************************
//
//	DIGITAL PRINTING - crc_fuji.c
//
//	Communication between ARM processor and FPGA
//
// ****************************************************************************
//
//	Copyright 2021 by radex AG, Switzerland. All rights reserved.
//	Written by Marcel Galliker
//
// ****************************************************************************

#include "rx_common.h"

UCHAR fuji_crc(const UCHAR * data, int size);