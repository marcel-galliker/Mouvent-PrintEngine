// ****************************************************************************
//
//	data.c
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
#include "rx_sok.h"

typedef struct
{
	SPageId					id;
	int						mirror;
	int						headsUsed;
	struct SBmpSplitInfo	*splitInfo;	// splitInfo[splitInfoCnt];
} SPrintListItem;

typedef struct SBmpSplitInfo
{
	SPrintListItem	*pListItem;
	BYTE **data;
	int	used;
	int	board;		// starting with 1
	int head;		// head on board
	int bitsPerPixel;
	int colorCode;	// for simulation
	int inkSupplyNo;// for simulation
	int fillBt;		// filler pixels at beginning
	int jetPx0;		// number of pixels to shift data to left
//	int startPx;
	int widthPx;
	int startBt;	// bytes
	int widthBt;	// bytes
	int srcLineCnt;
	int srcLineLen;	// bytes
	int dstLineLen;	// bytes, aligned to 512 Bits (32 Bytes)
	int blk0;		// number of block in printhead
	int blkCnt;		// number of used blocks
//	int wasFree;
	int sendFromBlk;
	int printMode;
	int clearBlockUsed;
	int	same;
//	int	scanStartBt;//
//	int usedSize;	// size of the used flags
//	BYTE *used;		// used flags
} SBmpSplitInfo;


void data_init(RX_SOCKET socket, int headCnt);
void data_end(void);

void data_abort		(void);
int	 data_cache		(SPageId *id, const char *path, char *localpath, SColorSplitCfg *psplit, int splitCnt);
int  data_get_size	(const char *path, UINT32 page, UINT32 spacePx, UINT32 *width, UINT32 *height, UINT8 *bitsPerPixel);
int  data_malloc	(int printMode, UINT32 width, UINT32 height, UINT8 bitsPerPixel, SColorSplitCfg *psplit, int splitCnt, BYTE* buffer[MAX_COLORS]);
int  data_free		(BYTE* buffer[MAX_COLORS]);
int  data_load		(SPageId *id, const char *filepath, int offsetPx, int lengthPx, int gapPx, int blkNo, int printMode, int variable, int mirror, int clearBlockUsed, int same, BYTE* buffer[MAX_COLORS]);
// int  data_reload	(SPageId *id);
SBmpSplitInfo*		data_get_next	(int *headCnt);
void data_fill_blk	(SBmpSplitInfo *psplit, int blkNo, BYTE *dst);
int  data_sent		(SBmpSplitInfo *psplit, int head); // return TRUE=all data sent
int  data_ready		(void);