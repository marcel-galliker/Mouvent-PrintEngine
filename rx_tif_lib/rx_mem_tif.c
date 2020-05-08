// ****************************************************************************
//
//	rx_mem_tif.h
//
// ****************************************************************************
//
//	Copyright 2014 by Radex AG, Switzerland. All rights reserved.
//	Written by Marcel Galliker 
//
// ****************************************************************************

#include "rx_common.h"
#include "rx_mem_tif.h"

typedef struct
{
	BYTE	*data;
	int64	size;
	int64	pos;
} SMemInfo;

//--- prototypes ----------------------------------------------------

static tmsize_t _tiffReadProc		(thandle_t fd, void* buf, tmsize_t size);
static tmsize_t _tiffWriteProc		(thandle_t fd, void* buf, tmsize_t size);
static uint64	_tiffSeekProc		(thandle_t fd, uint64 off, int whence);
static int		_tiffCloseProc		(thandle_t fd);
static uint64	_tiffSizeProc		(thandle_t fd);
static int		_tiffDummyMapProc	(thandle_t fd, void** pbase, toff_t* psize);
static void		_tiffDummyUnmapProc	(thandle_t fd, void* base, toff_t size);

//--- rx_mem_tif_open -------------------------------------------------
TIFF* rx_mem_tif_open(void *data, UINT64 size)
{
	TIFF *tif;
	SMemInfo *info = (SMemInfo*)rx_malloc(sizeof(SMemInfo));
	info->data = (BYTE*)data;
	info->size = size;
	tif = TIFFClientOpen("", "r", (thandle_t)info, _tiffReadProc, _tiffWriteProc, _tiffSeekProc, _tiffCloseProc, _tiffSizeProc, _tiffDummyMapProc,_tiffDummyUnmapProc);
	return tif;	
}

//--- _tiffReadProc ----------------------------
static tmsize_t _tiffReadProc(thandle_t fd, void* buf, tmsize_t size)
{
	SMemInfo *info = (SMemInfo*)fd;
	INT64 size64=(INT64)size;
	if (info->pos+size64>info->size) size64=info->size-info->pos;
	memcpy(buf, &info->data[info->pos], (tmsize_t)size64);
	info->pos += size64;
	return (tmsize_t)size64;
}

//--- _tiffWriteProc ----------------------------
static tmsize_t _tiffWriteProc(thandle_t fd, void* buf, tmsize_t size)
{
	SMemInfo *info = (SMemInfo*)fd;
	INT64	size64 = (INT64)size;
	if (info->pos+size64>info->size) size64=info->size-info->pos;
	memcpy(&info->data[info->pos], buf, (tmsize_t)size64);
	info->pos += size64;
	return (tmsize_t)size64;
}

//--- _tiffSeekProc ----------------------------
static uint64 _tiffSeekProc(thandle_t fd, uint64 off, int whence)
{
	SMemInfo *info = (SMemInfo*)fd;
	switch(whence)
	{
		case SEEK_SET:	info->pos = off;			break;
		case SEEK_CUR:	info->pos += off;			break;
		case SEEK_END:	info->pos = info->size+off;	break;
		default:		info->pos = off;			break;
	}
	if (info->pos > info->size) info->pos=info->size;
	if (info->pos < 0)	        info->pos=0;
	return(info->pos);
}

//--- _tiffCloseProc ----------------------------
static int _tiffCloseProc(thandle_t fd)
{
	free(fd);
	return 0;
}

//--- _tiffSizeProc ----------------------------
static uint64 _tiffSizeProc(thandle_t fd)
{
	SMemInfo *info = (SMemInfo*)fd;
	return(info->size);
}

//--- _tiffDummyMapProc ----------------------------
static int _tiffDummyMapProc(thandle_t fd, void** pbase, toff_t* psize)
{
	(void) fd;
	(void) pbase;
	(void) psize;
	return (0);
}

//--- _tiffDummyUnmapProc ----------------------------
static void _tiffDummyUnmapProc(thandle_t fd, void* base, toff_t size)
{
	(void) fd;
	(void) base;
	(void) size;
}


