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

typedef enum EFileType
{
	ft_undef,	//	0
    ft_bmp,		//	1
    ft_flz,		//  2
	ft_tif		//  3
} EFileType;

typedef struct
{
	char					filepath[MAX_PATH];
	char					dots[4];
	SPageId					id;
	int						offsetWidth;
	BOOL					sent;
	BOOL					clearBlockUsed;
	int						flags;
	UINT8					virtualPasses;
	UINT8					virtualPass;
	UINT8					penetrationPasses;
	int						headsUsed;
	int						headsInUse;
	int						lengthPx;
//	int						decompressing;
	int						processing;
	int						testOffsetPx;
	struct SBmpSplitInfo	*splitInfo;	// splitInfo[splitInfoCnt];
} SPrintListItem;

typedef struct SBmpSplitInfo
{
	SPrintListItem	*pListItem;
	BYTE **data;
	BYTE **allocated_buff; // to keep trace of initial allocated buffer (some test images could change "data" pointer above)
	int used;
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
	int	srcWidthBt;	// bytes, no alignment
	int srcLineLen;	// bytes, aligned to 64 Bit
	int dstLineLen;	// bytes, aligned to 512 Bits (32 Bytes)
	int blk0;		// number of block in printhead
	int blkCnt;		// number of used blocks
//	int wasFree;
	int sendFromBlk;
	int printMode;
	int clearBlockUsed;
	int	same;
	int	screening;
	SPoint resol;

//	int	scanStartBt;//
//	int usedSize;	// size of the used flags
//	BYTE *used;		// used flags
} SBmpSplitInfo;


void data_init(RX_SOCKET socket, int headCnt);
void data_end(void);

void data_abort		(void);
int	 data_cache		(SPageId *id, const char *path, char *localpath, SColorSplitCfg *psplit, int splitCnt);
void data_set_wakeuplen	(int len, int on);
int  data_get_size	(const char *path, UINT32 page, EFileType *pfileType, UINT32 *pspacePx, UINT32 *pwidth, UINT32 *plength, UINT8 *bitsPerPixel, UINT8 *multiCopy);
void data_clear		(BYTE* buffer[MAX_COLORS]);
UINT64 data_memsize(int printMode, UINT32 width, UINT32 height, UINT8 bitsPerPixel);
int  data_malloc	(int printMode, UINT32 width, UINT32 height, UINT8 bitsPerPixel, SColorSplitCfg *psplit, int splitCnt, UINT64 *pBufSize, BYTE* buffer[MAX_COLORS]);
int  data_free		(UINT64 *pBufSize, BYTE* buffer[MAX_COLORS]);
int data_load_file	(const char *filepath, SPageId *pid);

int  data_load		(SPageId *id, const char *filepath, EFileType fileType, int offsetPx, int lengthPx, UINT8 multiCopy, int gapPx, int blkNo, int blkCnt, int printMode, int variable, UINT8 virtualPasses, UINT8 virtualPass, int flags, int clearBlockUsed, int same, int smp_bufsize, const char *dots, BYTE* buffer[MAX_COLORS]);
void data_rip_same(SPrintListItem *pItem, BYTE *buffer[MAX_COLORS]);
int data_same(SPageId *id, int offsetWidth, int clrerearBlockUsed, SPrintListItem **pItem, int variable);
// int  data_reload	(SPageId *id);
void data_send_id	(SPageId *id);
int	 data_next_id	(void);
SBmpSplitInfo*		data_get_next	(int *headCnt);
void data_fill_blk	(SBmpSplitInfo *psplit, int blkNo, BYTE *dst, int test);
int  data_sent		(SBmpSplitInfo *psplit, int head); // return TRUE=all data sent
int  data_ready		(void);
int	 data_printList_idx(SPrintListItem *pitem);