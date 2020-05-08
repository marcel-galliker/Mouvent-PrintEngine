// ****************************************************************************
//
//	bmp.h
//
// ****************************************************************************
//
//	Copyright 2014 by Radex AG, Switzerland. All rights reserved.
//	Written by Marcel Galliker 
//
// ****************************************************************************

#pragma once

#include "rx_common.h"
#include "rx_def.h"


void bmp_color_path(const char *path, const char *colorname, char *filepath);

int bmp_get_size(const char *path, UINT32 *width, UINT32 *height, UINT8 *bitsPerPixel, UINT32 *memsize);

int bmp_load	(const char *path, BYTE **buffer, int bufsize, SBmpInfo *info);

int bmp_load_all(const char *path, int printMode, SColorSplitCfg *psplit, int splitCnt, BYTE* buffer[MAX_COLORS], SBmpInfo *pinfo);

int bmp_write	  (const char *filename, UCHAR *pBuffer, int bitsPerPixel, int width, int length, int bytesPerLine, int invert);
int bmp_write_head(const char *filename, UCHAR *pBuffer, int bitsPerPixel, int width, int length, int convert, UCHAR *memstart, UINT32 memsize);
