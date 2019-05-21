// ****************************************************************************
//
//	rx_mem_tif.h
//
// ****************************************************************************
//
//	Copyright 2018 by Mouivent AG, Switzerland. All rights reserved.
//	Written by Marcel Galliker 
//
// ****************************************************************************

#pragma once

#include "rx_common.h"
#include "tiffio.h"


#ifdef __cplusplus
extern "C"{
#endif
	
	TIFF* rx_mem_tif_open(void *data, UINT64 size);

#ifdef __cplusplus
}
#endif
