// ****************************************************************************
//
//	rx_bitmap.h
//
// ****************************************************************************
//
//	Copyright 2014 by Radex AG, Switzerland. All rights reserved.
//	Written by Marcel Galliker 
//
// ****************************************************************************

#pragma once

#include "rx_common.h"

#ifdef __cplusplus
extern "C"{
#endif

typedef struct 
{
	INT32	bppx;		// bits per pixel
	INT32	width;		// width in pixel 
	INT32	height;		// height in pixel
	INT32	lineLen;	// line length in bytes	
	INT32	sizeUsed;	// buffer size used
	INT32	sizeAlloc;	// buffer size allocated
	BYTE	*buffer;	// buffer
} RX_Bitmap;


int bmp_create (RX_Bitmap *pbmp, int width, int height, int bppx);
int bmp_clear  (RX_Bitmap *pbmp, int x, int y, int width, int height);
int bmp_destroy(RX_Bitmap *pbmp);


#ifdef __cplusplus
}
#endif