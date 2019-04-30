// ****************************************************************************
//
//	rx_flz.h
//
// ****************************************************************************
//
//	Copyright 2014 by Radex AG, Switzerland. All rights reserved.
//	Written by Marcel Galliker 
//
// ****************************************************************************

#pragma once

#include "rx_def.h"

//--- SFlzBand ---------------------------------------------
typedef struct
{
	UINT64 offset;
	UINT64 bandSize;
	UINT32 start;
	UINT32 rows;
} SFlzBand;

//--- SFlzInfo ---------------------------------------------
typedef struct
{
	UINT32 WidthPx;
	UINT32 lengthPx;
	UINT32 bitsPerPixel;
	UINT32 lineLen;		// in bytes
	UINT32 aligment;	// 8, 16, 32 ,....
	UINT64 dataSize;
	SPoint resol;
	UINT32 bands;
	UINT32 maxBandHeight;
} SFlzInfo;

// followed by SFlzBand flzband[bands];
// followed by bands * bandsize

typedef void(*progress_fct) (SPageId *id, const char *colorSName, int progress);


#ifdef __cplusplus
extern "C"{
#endif
void flz_abort(void);
int flz_get_info(const char *path, UINT32 page, SFlzInfo *pflzinfo);
int flz_load	(SPageId *id, const char *filedir, const char *filename, int printMode, UINT32 spacePx, SColorSplitCfg *psplit, int splitCnt, BYTE* buffer[MAX_COLORS], SBmpInfo *pinfo, progress_fct progress);

#ifdef __cplusplus
}
#endif
