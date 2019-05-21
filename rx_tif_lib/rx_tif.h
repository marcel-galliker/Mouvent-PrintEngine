// ****************************************************************************
//
//	rx_tif.h
//
// ****************************************************************************
//
//	Copyright 2014 by Radex AG, Switzerland. All rights reserved.
//	Written by Marcel Galliker 
//
// ****************************************************************************

#pragma once

#include "rx_def.h"
#include "tiffio.h"

// --- STiffInfo -------------------------------------------- -
typedef struct
{
	SHORT	SamplesPerPixel;
	SHORT	BitsPerSample;
	UINT32	Width;
	UINT32	Length;
	SPoint	Resol;	// in DPI
	SHORT	PlanarConfig; // 1 = PLANARCONFIG_CONTIG(default) 2=PLANARCONFIG_SEPARATE
} STiffInfo;

typedef void(*progress_fct) (SPageId *id, const char *colorSName, int progress);
typedef TIFF*(*tif_stream_open_fct)(int page, int bitmap);

#ifdef __cplusplus
extern "C"{
#endif
int tif_get_info	(const char *path, STiffInfo *pinfo);
int tif_load_full	(const char *path, SColorSplitCfg *psplit, int splitCnt, SBmpInfo *pinfo);
// int tif_get_size	(const char *path, UINT32 spacePx, UINT32 *width, UINT32 *height, UINT8 *bitsPerPixel);
int tif_get_size	(const char *path, UINT32 page, UINT32 spacePx, UINT32 *width, UINT32 *height, UINT8 *bitsPerPixel);
int tif_load_mt		(SPageId *id, const char *filedir, const char *filename, int printMode, UINT32 spacePx, SColorSplitCfg *psplit, int splitCnt, BYTE* buffer[MAX_COLORS], SBmpInfo *pinfo, progress_fct progress);
int tif_load		(SPageId *id, const char *filedir, const char *filename, int printMode, UINT32 spacePx, INT32 wakeupLen, SColorSplitCfg *psplit, int splitCnt, BYTE* buffer[MAX_COLORS], SBmpInfo *pinfo, progress_fct progress);
int tif_load_simple	(const char *path, BYTE **buffer, int bufsize, SBmpInfo *info);
int tif_load_8bppx	(const char *path, int printMode, SColorSplitCfg *psplit, int splitCnt, BYTE* buffer[MAX_COLORS], SBmpInfo *pinfo);
// int tif_write(const char *filedir, const char *filename, const char *ext, SBmpInfo *pinfo, int uncompress);
int tif_write		(const char *filedir, const char *filename, SBmpInfo *pinfo);
int tif_write_plane(const char *filepath, SPlaneInfo *pinfo, int uncompress);
void tif_abort		(void);

char *tif_error(void);
char *tif_warn(void);

extern tif_stream_open_fct tif_stream_open;

#ifdef __cplusplus
}
#endif
