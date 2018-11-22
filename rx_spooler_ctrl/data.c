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

#include "rx_common.h"
#include "rx_bitmap.h"
#include "rx_def.h"
#include "rx_error.h"
#include "rx_file.h"
#include "rx_mem.h"
#include "rx_threads.h"
#include "rx_tif.h"
#include "rx_trace.h"
#include "rx_rip_lib.h"
#include "bmp.h"
#include "tcp_ip.h"
#include "spool_rip.h"
#include "ctrl_client.h"
#include "data_client.h"

#include "data.h"

//--- defines ------------------------------------------------------
#define PRINT_LIST_SIZE 128
#define MNT_PATH	"/mnt/ripped-data"


//--- static variables ---------------------------------------------

static int				_Init=FALSE;
static int				_Abort=FALSE;
static int				_ForceFree=FALSE;
static SPrintListItem	_PrintList[PRINT_LIST_SIZE];
static int				_InIdx;
static int				_SendIdx;
static int				_OutIdx;
static int				_HeadCnt;
static UINT64			_MaxMemory=0;
static int				_MaxBufers=64;
static UINT64			_BufSize=0;
static HANDLE			_SendSem=NULL;
static int				_Mounted=FALSE;
static int				_IsLocal=FALSE;
static int				_IsMain =TRUE;

static SPageId			_LastId;
static char				_LastFilePath[MAX_PATH];
static SBmpInfo			_LastBmpInfo;
static UINT32			_BlkNo[HEAD_BOARD_CNT][MAX_HEADS_BOARD];

static BYTE*			_TestBuf[MAX_COLORS][MAX_HEADS_COLOR];	// buffered in case of same image

static int  _local_path(const char *filepath, char *localPath);
static int	_file_used (const char *filename);
static int  _copy_file (SPageId *pid, char *srcDir, char *filename, char *dstDir);

static int  _data_split		           (SPageId *id, SBmpInfo *pBmpInfo, int offsetPx, int lengthPx, int blkNo, int clearBlockUsed, int same, int mirror, SPrintListItem *pItem);
static int  _data_split_scan           (SPageId *id, SBmpInfo *pBmpInfo, int offsetPx, int lengthPx, int blkNo, int clearBlockUsed, int same, SPrintListItem *pItem);
static int  _data_split_scan_no_overlap(SPageId *id, SBmpInfo *pBmpInfo, int offsetPx, int lengthPx, int blkNo, int clearBlockUsed, int same, SPrintListItem *pItem);
static int  _data_split_test           (SPageId *id, SBmpInfo *pBmpInfo, int offsetPx, int lengthPx, int blkNo, int clearBlockUsed, int same, SPrintListItem *pItem);

//--- data_init ------------------------------------------------------------------------
void data_init(RX_SOCKET socket, int headCnt)
{
	int i;

	if (_SendSem==NULL)
	{
		char addr[32];
		_SendSem=rx_sem_create();
		rx_sem_post(_SendSem);
		sok_get_socket_name(socket, NULL, addr, NULL);
//		_IsMain  = !strcmp(addr, RX_CTRL_MAIN);
		_IsMain  = !strcmp(addr, RX_CTRL_MAIN) || !strcmp(addr, "127.0.0.1");
//		_IsLocal =                                !strcmp(addr, "127.0.0.1");
		_IsLocal = !strcmp(addr, RX_CTRL_MAIN) || !strcmp(addr, "127.0.0.1");
	}
	
	_InIdx   = 0;
	_OutIdx  = 0;
	_SendIdx = 0;
	_Abort   = FALSE;
	if (_MaxMemory==0)
	{
		_MaxMemory  = rx_mem_physical();
		_MaxMemory -= 1024*1024*1024; // leave one GB for the system
	}
	memset(&_LastId, 0, sizeof(_LastId));
	memset(_LastFilePath, 0, sizeof(_LastFilePath));
	memset(&_LastBmpInfo, 0, sizeof(_LastBmpInfo));
	memset(_BlkNo, 0, sizeof(_BlkNo));

	if (_HeadCnt!=headCnt)
	{
		for (i=0; i<SIZEOF(_PrintList); i++)
		{
			if (_PrintList[i].splitInfo) 
			{
				if (i<127) free(_PrintList[i].splitInfo);
			}
			_PrintList[i].splitInfo = (SBmpSplitInfo*)malloc(headCnt*sizeof(SBmpSplitInfo));
			memset(_PrintList[i].splitInfo, 0,  headCnt*sizeof(SBmpSplitInfo));
		}
	}
	_HeadCnt = headCnt;
}

//--- data_end -------------------------------------------------------------------------
void data_end()
{
	_Abort=TRUE;
}

//--- data_abort ------------------
void data_abort		(void)
{	
	_Abort=TRUE;
	_ForceFree=TRUE;
	tif_abort();
}

//--- _local_path ------------------------------------------------------------------------
static int _local_path(const char *filepath, char *localPath)
{
	char name[MAX_PATH];
	int len;
	char *ch;

	if (_IsLocal)
	{
		strcpy(localPath, filepath);
	}
	else if (_IsMain)
	{
		//--- create local path --------------
		strcpy(localPath, filepath);
		split_path(localPath, NULL, name, NULL);
	
		//--- convert to linux path ---
		if (filepath[1]==':')
		{
			Error(WARN, 0, "CODE NOT TESTED");
			len = sprintf(localPath, "%s", PATH_RIPPED_DATA);
			strcpy(&localPath[len], &filepath[3]);
			for (ch=localPath; *ch; ch++)
				if (*ch=='\\') *ch='/';
		}
	}
	else
	{
		if (!_Mounted) _Mounted = rx_file_mount("//" RX_CTRL_MAIN "/ripped-data", MNT_PATH, "radex", "radex")==REPLY_OK;
		strcpy(localPath, filepath);
	}

	return REPLY_OK;
}

//--- data_cache ----------------------------------------------------------------------
int	 data_cache	(SPageId *pid, const char *path, char *localPath, SColorSplitCfg *psplit, int splitCnt)
{
	int ret=REPLY_OK;

	_Abort = FALSE; 
	_local_path(path, localPath);

	if (!_IsMain)
	{	
		SEARCH_Handle search;
		char		  mntPath[MAX_PATH];
		char		  fileName[MAX_PATH];
		UINT64		  time;
		UINT32		  filesize, isDir;
//		char		  dstDir[MAX_PATH];

		sprintf(mntPath, "%s/%s", MNT_PATH, &localPath[strlen(PATH_RIPPED_DATA)]);
		
	//	sprintf(dstDir, PATH_TEMP "%s", &localPath[strlen(PATH_RIPPED_DATA)]);	// TEST

		search=rx_search_open(mntPath, "*");
		while (ret==REPLY_OK && rx_search_next(search, fileName, sizeof(fileName), &time, &filesize, &isDir))
		{
			if (!isDir && _file_used(fileName)) ret=_copy_file(pid, mntPath, fileName, localPath);
		}
		rx_search_close(search);
	}
	return ret;
}

//--- _file_used -----------------------------------------------------
static int	_file_used (const char *filename)
{
	int i;
	char str[10];
	for (i=0; i<SIZEOF(RX_Color); i++)
	{
		if (RX_Color[i].spoolerNo == RX_SpoolerNo)
		{
			sprintf(str, "_%s.", RX_ColorNameShort(i));
			if (strstr(filename, str)) return TRUE;
		}
	}
	return FALSE;
}

//--- _copy_file -----------------------------
static int _copy_file(SPageId *pid, char *srcDir, char *fileName, char *dstDir)
{
	char	srcPath[MAX_PATH];
	char	dstPath[MAX_PATH];
	time_t	srcTime;
	time_t	dstTime;
	
	sprintf(srcPath, "%s/%s", srcDir, fileName);
	sprintf(dstPath, "%s/%s", dstDir, fileName);

	rx_mkdir_path(dstPath);
	
	srcTime = rx_file_get_mtime(srcPath);
	dstTime = rx_file_get_mtime(dstPath);

	if (dstTime==srcTime)  return REPLY_OK;

	TrPrintfL(TRUE, "copy file %s", dstPath);

	{
		FILE *src=NULL;
		FILE *dst=NULL;
		BYTE *buf=NULL;
		int bufsize=10240;
		int len;
		int time, last=0;
		INT64 size;
		INT64 copied=0;

		src = fopen(srcPath, "rb");
		dst = fopen(dstPath, "wb");
		buf = malloc(bufsize);
		if (src && dst && buf)
		{
			size = rx_file_get_size(srcPath);
			while ((len=fread(buf, 1, bufsize, src)) && !_Abort)
			{
				if (fwrite(buf, 1, len, dst)!=len) break;
				copied+=len;
				time = rx_get_ticks()/1000;
				if (time!=last) 
				{
					last=time;
					ctrl_send_load_progress(pid, fileName, (int)(100*copied/size));
				}
			}
		}
		TrPrintfL(TRUE, "end Transfer: errno=%d\n", errno);
		if(buf) free(buf);
		if (src) fclose(src);
		if (dst) fclose(dst);
		if (_Abort) remove(dstPath);
	#ifdef linux
		if (errno==ENOSPC) 
		{
		//	_clear_cache(); // disk full!
			return REPLY_ERROR;
		}
	#endif
		rx_file_set_mtime(dstPath, srcTime);
	}
	ctrl_send_load_progress(pid, fileName, 100);
	return REPLY_OK;
}

//--- data_get_size ---------------------------------------------------------------
int  data_get_size	(const char *path, UINT32 page, UINT32 spacePx, UINT32 *pwidth, UINT32 *plength, UINT8 *pbitsPerPixel)
{
	int ret;
	UINT32 memsize;
	ret = tif_get_size(path, page, spacePx, pwidth, plength, pbitsPerPixel);
	if (ret == REPLY_NOT_FOUND) 
	{
		char filepath[MAX_PATH];
		bmp_color_path(path, RX_ColorNameShort(0), filepath);
		ret = bmp_get_size(filepath, pwidth, plength, pbitsPerPixel, &memsize);
	}
	return ret;
}

//--- data_malloc -------------------------------------------------------------------------
int  data_malloc	(int printMode, UINT32 width, UINT32 height, UINT8 bitsPerPixel, SColorSplitCfg *psplit, int splitCnt, BYTE* buffer[MAX_COLORS])
{	
	UINT64	memsize;
	int i, found, error, lineLen;

//	lineLen = (width*bitsPerPixel+7)/8;			// workes for TIFF
//	lineLen = ((width*bitsPerPixel+15)/16)*2;	// used for BMP files!
	lineLen = ((width*bitsPerPixel+31) & ~31)/8;	// used for BMP files!
	
	if (FALSE && printMode==PM_SCANNING) 
	{
		int	cnt;
		for (cnt=1; cnt<8; cnt++)
		{
			memsize = cnt * (UINT64)width * bitsPerPixel; // in bits
			if ((memsize % 8)==0) break;
		}
		memsize = memsize/8*height;
	}
	else memsize = (UINT64)lineLen*height;
	
	if (printMode==PM_TEST) memsize*=RX_Spooler.headsPerColor;
	
	memsize = (memsize+0xfffff) & ~0xffff; // round up to next MB

	//--- allocate -------------------------
	/*
	TrPrintf(TRUE, "Free buffers: Force=%d", _ForceFree);
	for (i=0; i<MAX_COLORS; i++)
	{
		if (psplit[i].color.name[0])
		{
			if (psplit[i].lastLine>psplit[i].firstLine)
			{
				TrPrintfL(1, "buffer[%d]: used: cnt=%d", i, rx_mem_cnt(buffer[i]));
			}
		}	
	}	
	*/

	error=0;
	if(_BufSize >= memsize && !_ForceFree)
	{
		for (i=0; i<MAX_COLORS; i++)
		{
			if (psplit[i].color.name[0] && psplit[i].lastLine>psplit[i].firstLine)
			{
				TrPrintfL(1, "buffer[%d]: WAIT UNUSED %p", i, buffer[i]);
				while (!_Abort && rx_mem_cnt(buffer[i]))
				{
					rx_sleep(10);
				}
				TrPrintfL(1, "buffer[%d]: IS UNUSED", i);								
			}
		}
		_BufSize = memsize;
	}
	else
	{
		for (i=0, error=FALSE; !error && i<MAX_COLORS; i++)
		{
			if (psplit[i].color.name[0])
			{
				found = TRUE;
				if (psplit[i].lastLine>psplit[i].firstLine)
				{
					TrPrintfL(1, "buffer[%d]: WAIT FREE %p", i, buffer[i]);
					while (!_Abort)
					{
						if (_ForceFree) rx_mem_free_force(&buffer[i]);
						else			rx_mem_free(&buffer[i]);
						if (!buffer[i]) break;
						rx_sleep(10);
					}
					_BufSize = 0;

					TrPrintfL(1, "buffer[%d]: IS FREE", i);
					if (!_Abort)
					{
					//	TrPrintfL(1, "data_malloc buffer 1[%d], %p, free=%d MB, size=%d MB", i, buffer[i], rx_mem_get_freeMB(), memsize/1024/1024);
						buffer[i] = rx_mem_alloc(memsize);
						TrPrintfL(1, "data_malloc buffer[%d], %p, free=%d MB, size=%d MB", i, buffer[i], rx_mem_get_freeMB(), memsize/1024/1024);
						if (buffer[i]==NULL) error=TRUE;
						_BufSize = memsize;
					}
				}
			}
		}
		_BufSize = memsize;
	}
	
	_ForceFree = FALSE;
	if (error || _Abort)
	{
		for (i=0; i<MAX_COLORS; i++)
		{
			if (buffer[i]) rx_mem_free(&buffer[i]);
		}
		_BufSize=0;
		return REPLY_ERROR;
	}

	if (printMode==PM_TEST) _ForceFree = TRUE;
	return REPLY_OK;
}

//--- data_free ----------------------------------------------
int  data_free(BYTE* buffer[MAX_COLORS])
{
	int i;
	for (i=0; i<MAX_COLORS; i++)
	{
		if (buffer[i]) rx_mem_free(&buffer[i]);
	}
	_BufSize=0;
	return REPLY_OK;			
}

//--- data_load ------------------------------------------------------------------------
int data_load(SPageId *id, const char *filepath, int offsetPx, int lengthPx, int gapPx, int blkNo, int printMode, int variable, int mirror, int clearBlockUsed, int same, BYTE* buffer[MAX_COLORS])
{
	SBmpInfo		bmpInfo;
	int ret;
	int nextIdx;
	char filename[MAX_PATH];
	char	*tiffErr;

	TrPrintfL(TRUE, "data_load id=%d, page=%d, copy=%d, scan=%d, data_load >>%s<<", id->id, id->page, id->copy, id->scan, filepath);
 	nextIdx = (_InIdx+1) % PRINT_LIST_SIZE;
	if (nextIdx == _OutIdx) 
		return Error(ERR_CONT, 0, "Print List Overflow");
	
	if (variable)	ret = sr_rip_label (buffer, &bmpInfo); 
	else
	{
		if (id->id!=_LastId.id || id->page!=_LastId.page || strcmp(filepath, _LastFilePath))
		{
			ret = 1;
			bmpInfo.printMode = printMode;
			if (printMode==PM_TEST || printMode==PM_TEST_SINGLE_COLOR)
			{
				int color;
				for (color=0; color<MAX_COLORS; color++)
				{
					if (buffer[color])
					{
						ret=bmp_load(filepath, &buffer[color], 100000, &bmpInfo);
//						ret=tif_load_simple(filepath, &buffer[color], 100000, &bmpInfo);
						if (FALSE)
						{
							char str[MAX_PATH];
							sprintf(str, "%sTEST.bmp", PATH_TEMP);

							bmp_write(str, buffer[0], bmpInfo.bitsPerPixel, bmpInfo.srcWidthPx, bmpInfo.lengthPx, bmpInfo.lineLen, TRUE);
						}
						bmpInfo.buffer[color] = &buffer[color];
						bmpInfo.inkSupplyNo[color]=0;
						bmpInfo.colorCode[color]=0;
					}
				}
			}
			split_path(filepath, NULL, filename, NULL);
			if (ret) ret = tif_load(id, filepath, filename, printMode, gapPx, RX_Color, SIZEOF(RX_Color), buffer, &bmpInfo, ctrl_send_load_progress);
			if (ret==REPLY_NOT_FOUND)
			{
				tiffErr=tif_error();
				if (*tiffErr) Error(ERR_CONT, 0, "File not found: >>%s<<", str_start_cut(tiffErr, PATH_RIPPED_DATA));
			}
				
			if (ret) ret = bmp_load_all(filepath, printMode, RX_Color, SIZEOF(RX_Color), buffer, &bmpInfo);
		}
		else
		{
			memcpy(&bmpInfo, &_LastBmpInfo, sizeof(bmpInfo));
			ret=REPLY_OK;
		}
	}
	memcpy(&_LastId, id, sizeof(_LastId));
	memcpy(&_LastBmpInfo, &bmpInfo, sizeof(bmpInfo));
	strcpy(_LastFilePath, filepath);
	TrPrintfL(TRUE, "data_load: File loaded");

//	if (ret == REPLY_NOT_FOUND) ret = bmp_load_all(filepath, RX_Color, SIZEOF(RX_Color), &bmpInfo);
	if (ret==REPLY_OK)
	{
		#ifdef DEBUG
		if (FALSE)
		{
			int i;
			char str[MAX_PATH];
			for (i=0; i<SIZEOF(bmpInfo.buffer); i++)
			{
				if (bmpInfo.buffer[i])
				{
					sprintf(str, "%sSCAN_%s_%d.bmp", PATH_TEMP, RX_ColorNameShort(bmpInfo.inkSupplyNo[i]), id->copy);
					bmp_write(str, *bmpInfo.buffer[i], bmpInfo.bitsPerPixel, bmpInfo.srcWidthPx, bmpInfo.lengthPx, bmpInfo.lineLen, FALSE);
				}	
			}
		}
		#endif

		_data_split(id, &bmpInfo, offsetPx, lengthPx, blkNo, mirror, clearBlockUsed, same, &_PrintList[_InIdx]);
		TrPrintfL(TRUE, "data_load DONE idx=%d, id=%d, page=%d, copy=%d, scan=%d, data_load >>%s<<, COPY=%d", _InIdx, id->id, id->page, id->copy, id->scan, filepath, _PrintList[_InIdx].id.copy);
		_InIdx = nextIdx;
		return REPLY_OK;
	}
	TrPrintfL(TRUE, "data_load error");
	return REPLY_ERROR;
}

/*
//--- data_reload ------------------------------
int  data_reload	(SPageId *id)
{
	int nextIdx;

	TrPrintfL(TRUE, "data_reload id=%d, page=%d, copy=%d, scan=%d", id->id, id->page, id->copy, id->scan);
 	nextIdx = (_InIdx+1) % PRINT_LIST_SIZE;
	if (nextIdx == _OutIdx) 
		return Error(ERR_CONT, 0, "Print List Overflow");						
	memcpy(&_PrintList[_InIdx], &_LastPrintListItem, sizeof(_PrintList[_InIdx]));
	memcpy(&_PrintList[_InIdx].id, id, sizeof(_PrintList[_InIdx].id));
	_PrintList[_InIdx].splitInfo->pListItem = &_PrintList[_InIdx];
	
	_InIdx = nextIdx;
	return REPLY_OK;
}
*/

//--- data_get_next ----------------------------------------------------------------
SBmpSplitInfo*  data_get_next	(int *headCnt)
{
	int idx;
	if (_SendIdx!=_OutIdx) return NULL;
	if (_SendIdx==_InIdx) return NULL;
	idx = _SendIdx;
	_SendIdx = (_SendIdx+1) % PRINT_LIST_SIZE;
	*headCnt = _HeadCnt;
	return _PrintList[idx].splitInfo;	
}

//--- _data_split -----------------------------------------------------------------------
static int _data_split(SPageId *id, SBmpInfo *pBmpInfo, int offsetPx, int lengthPx, int blkNo, int mirror, int clearBlockUsed, int same, SPrintListItem *pItem)
{
	memset(pItem->splitInfo, 0, _HeadCnt * sizeof(SBmpSplitInfo));
	memcpy(&pItem->id, id, sizeof(pItem->id));
	pItem->mirror    = mirror;
	pItem->headsUsed = 0;
	
	switch(pBmpInfo->printMode)
	{
		case PM_SCANNING:			if (RX_Spooler.overlap)	
										return _data_split_scan(id, pBmpInfo, offsetPx, lengthPx, blkNo, clearBlockUsed, same, pItem);
									else		
										return _data_split_scan_no_overlap(id, pBmpInfo, offsetPx, lengthPx, blkNo, clearBlockUsed, same, pItem);
		case PM_TEST:		
		case PM_TEST_SINGLE_COLOR:	return _data_split_test(id, pBmpInfo, offsetPx, lengthPx, blkNo, clearBlockUsed, same, pItem);
		case PM_SINGLE_PASS:		return _data_split_scan(id, pBmpInfo, offsetPx, min(RX_Spooler.barWidthPx, (int)pBmpInfo->srcWidthPx), blkNo, clearBlockUsed, same, pItem);
		default:					return Error(ERR_STOP, 0, "Splitting algorithme not implemented");
	}
}

//--- _data_split_test ---------------------------------------------------------------------
static int _data_split_test(SPageId *id, SBmpInfo *pBmpInfo, int offsetPx, int lengthPx, int blkNo, int clearBlockUsed, int same, SPrintListItem *pItem)
{
	int color, n, head;
	struct SBmpSplitInfo	*pInfo;

	TrPrintfL(TRUE, "_data_split_test");

	for (color=0; color<SIZEOF(RX_Spooler.headNo); color++)
	{
		for (n=0; n<SIZEOF(RX_Spooler.headNo[color]); n++)
		{	
			head=RX_Spooler.headNo[color][n];
			if (!head) break;
			head--;
			pInfo = &pItem->splitInfo[head];
			rx_mem_use(*pBmpInfo->buffer[color]);
			pItem->headsUsed++;
			pInfo->pListItem	= pItem;
			pInfo->printMode	= pBmpInfo->printMode;
			pInfo->clearBlockUsed = clearBlockUsed;
			pInfo->same           = same;
			pInfo->data	= pBmpInfo->buffer[color];
			
			if (id->id==3 && pInfo->data && n>0)
			{
				_TestBuf[color][n] = (*pBmpInfo->buffer[color])+n*pBmpInfo->dataSize;
				pInfo->data	= &_TestBuf[color][n];
				memset(*pInfo->data, 0, 256);
				memcpy(*pInfo->data, *pBmpInfo->buffer[color], pBmpInfo->dataSize);
			}
			
			pInfo->used			= TRUE;
			pInfo->board		= head/RX_Spooler.headsPerBoard;
			pInfo->head			= head%RX_Spooler.headsPerBoard;
			pInfo->blk0			= _BlkNo[pInfo->board][pInfo->head]+RX_Spooler.dataBlkCntHead*(pInfo->head%RX_Spooler.headsPerBoard);
			pInfo->bitsPerPixel	= pBmpInfo->bitsPerPixel;
			pInfo->colorCode	= RX_Color[color].color.colorCode;
			pInfo->inkSupplyNo  = RX_Color[color].inkSupplyNo;
			pInfo->jetPx0		= 0;
			pInfo->fillBt		= 0;
			pInfo->widthPx		= pBmpInfo->srcWidthPx;
			pInfo->startBt		= 0;
			pInfo->widthBt		= pBmpInfo->lineLen;
			pInfo->srcLineLen	= pBmpInfo->lineLen;
			pInfo->srcLineCnt	= pBmpInfo->lengthPx;
			pInfo->dstLineLen	= (pBmpInfo->lineLen+31) & ~31; // align to 256 Bits (32 Bytes)
			pInfo->blkCnt		= (pInfo->dstLineLen*pInfo->srcLineCnt +RX_Spooler.dataBlkSize-1) / RX_Spooler.dataBlkSize;

			if (pBmpInfo->printMode==PM_TEST_SINGLE_COLOR && (color+1)!=id->scan)
			{
				//--- empty bitmap -------
				pInfo->data			= NULL;
				pInfo->bitsPerPixel	= 1;
				pInfo->widthPx		= 1;
				pInfo->widthBt		= 1;
				pInfo->srcLineLen	= 1;
				pInfo->srcLineCnt	= 1;
				pInfo->dstLineLen	= 32; // align to 256 Bits (32 Bytes)				
				pInfo->blkCnt		= 1;
			}
				
			if (id->id==3 && pInfo->data) 
			{
				RX_Bitmap bmp;
				bmp.bppx		= pInfo->bitsPerPixel;
				bmp.width		= pInfo->widthPx;
				bmp.height		= pInfo->srcLineCnt;
				bmp.lineLen		= pInfo->srcLineLen;
				bmp.sizeUsed	= 0;
				bmp.sizeAlloc	= 0;
				bmp.buffer		=  *pInfo->data;
				rip_test_data(&bmp, RX_TestData[head]);
			}
			if (clearBlockUsed) _BlkNo[pInfo->board][pInfo->head] = (_BlkNo[pInfo->board][pInfo->head]+pInfo->blkCnt)%(RX_Spooler.dataBlkCntHead);
		}
	}	
	return REPLY_OK;
}


//--- _data_split_scan -----------------------------------------------------------------------
static int _data_split_scan(SPageId *id, SBmpInfo *pBmpInfo, int offsetPx, int lengthPx, int blkNo, int clearBlockUsed, int same, SPrintListItem *pItem)
{
//  0                         2048  2176  
//  |                            |   |
//  |-offsetPx-|------------------------BITMAP-----------------------------------|        
//	|128|----------1920----------|128|----------1920----------|128|----------1920----------|128|
//	|------------- HEAD 1------------|
//                               |------------- HEAD 2 -----------|
//                                                            |------ HEAD 3 ----|
	int color, n;
	int endPx;
	int startPx;
	int widthPx;
	int maxWidth;
	int head;
	int pixelPerByte;
	int colorOffset;
	struct SBmpSplitInfo	*pInfo;

	if (pBmpInfo->bitsPerPixel==0) 
		return REPLY_ERROR;
	pixelPerByte = 8/pBmpInfo->bitsPerPixel;

	//--- image info -------------------------------------------------
//	memset(pItem->splitInfo, 0, _HeadCnt * sizeof(SBmpSplitInfo));
//	memcpy(&pItem->id, id, sizeof(pItem->id));
//	pItem->headsUsed = 0;

	if (offsetPx == 0)	widthPx = RX_Spooler.barWidthPx + 1*RX_Spooler.headOverlapPx;	// stitch on right
	else widthPx = RX_Spooler.barWidthPx + 2*RX_Spooler.headOverlapPx;	// stitch on left+right

	maxWidth = RX_Spooler.headWidthPx+RX_Spooler.headOverlapPx;
	//--- do the split ------------------------------------------------
	for (color=0; color<SIZEOF(pBmpInfo->colorCode); color++)
	{
		if (pBmpInfo->buffer[color])
		{
//			rx_mem_use(pBmpInfo->buffer[color]);

			colorOffset = RX_Color[color].offsetPx;

		//	Error(WARN,0,"TEST"); colorOffset=color;

			startPx	= offsetPx-colorOffset;
			endPx	= startPx+widthPx;
			if (endPx > lengthPx) endPx = lengthPx;
			
			n=0;
			while((endPx>0 && startPx<endPx) || RX_Spooler.headNo[color][n]==0)
			{
				head = RX_Spooler.headNo[color][n];
				if (head==0) break;
				head--;
				pInfo = &pItem->splitInfo[head];
				{	
					rx_mem_use(*pBmpInfo->buffer[color]);
					pItem->headsUsed++;
					pInfo->printMode	= pBmpInfo->printMode;
					pInfo->clearBlockUsed = clearBlockUsed;
					pInfo->same			  = same;
					pInfo->data			= pBmpInfo->buffer[color];
					pInfo->pListItem	= pItem;
					pInfo->used			= TRUE;
					pInfo->board		= head/RX_Spooler.headsPerBoard;
					pInfo->head			= head%RX_Spooler.headsPerBoard;
					pInfo->blk0			= _BlkNo[pInfo->board][pInfo->head]+RX_Spooler.dataBlkCntHead*(pInfo->head%RX_Spooler.headsPerBoard);
					pInfo->bitsPerPixel	= pBmpInfo->bitsPerPixel;
					pInfo->colorCode	= pBmpInfo->colorCode[color];
					pInfo->inkSupplyNo  = pBmpInfo->inkSupplyNo[color];
					if (startPx<0)
					{
						pInfo->fillBt  = (-startPx+pixelPerByte-1)/pixelPerByte;
						pInfo->startBt = 0;
					}
					else
					{
						pInfo->startBt	= startPx/pixelPerByte;
						pInfo->fillBt	= 0;	
					}
					pInfo->widthPx		= endPx-startPx;
					if (pInfo->widthPx>maxWidth)	pInfo->widthPx=maxWidth;
					pInfo->widthBt		= (pInfo->widthPx+pixelPerByte-1)/pixelPerByte;
					if (startPx%pixelPerByte)
					{		
						if (startPx<0) 	pInfo->jetPx0  = pixelPerByte-(startPx+RX_Spooler.headWidthPx+pixelPerByte)%pixelPerByte;
						else		    pInfo->jetPx0  = pixelPerByte-(startPx+pixelPerByte)%pixelPerByte;
						pInfo->widthPx += pixelPerByte;
						pInfo->widthBt ++;
					}
					else pInfo->jetPx0	= 0;

					pInfo->srcLineLen	= pBmpInfo->lineLen;
					pInfo->srcLineCnt	= pBmpInfo->lengthPx;
					pInfo->dstLineLen	= (pInfo->widthBt+31) & ~31; // align to 256 Bits (32 Bytes)
					pInfo->blkCnt		= (pInfo->dstLineLen * pBmpInfo->lengthPx + RX_Spooler.dataBlkSize-1) / RX_Spooler.dataBlkSize;
					if (clearBlockUsed) _BlkNo[pInfo->board][pInfo->head] = (_BlkNo[pInfo->board][pInfo->head]+pInfo->blkCnt)%(RX_Spooler.dataBlkCntHead);
					TrPrintfL(TRUE, "Split[%d]: startPx=%d, WidthPx=%d, fillBt=%d, jetPx0=%d, pixelPerByte=%d, buffer=0x%08x", head, startPx, pInfo->widthPx, pInfo->fillBt,  pInfo->jetPx0, pixelPerByte, pInfo->data);
				}
				//--- increment ---
				startPx += RX_Spooler.headWidthPx;
				n++;
			}
		}

		//--- fill empty heads with data ---
		for (n=0; n<SIZEOF(RX_Spooler.headNo[color]); n++)
		{	
			head=RX_Spooler.headNo[color][n];
			if (!head) break;
			head--;
			pInfo = &pItem->splitInfo[head];
			if (!pInfo->used)
			{
				pItem->headsUsed++;
				pInfo->pListItem	= pItem;
				pInfo->printMode	= pBmpInfo->printMode;
				pInfo->clearBlockUsed = clearBlockUsed;
				pInfo->same			  = same;					
				pInfo->data			= NULL;
				pInfo->used			= TRUE;
				pInfo->board		= head/RX_Spooler.headsPerBoard;
				pInfo->head			= head%RX_Spooler.headsPerBoard;
				pInfo->blk0			= _BlkNo[pInfo->board][pInfo->head]+RX_Spooler.dataBlkCntHead*(pInfo->head%RX_Spooler.headsPerBoard);
				pInfo->bitsPerPixel	= 1;
				pInfo->colorCode	= pBmpInfo->colorCode[color];
				pInfo->inkSupplyNo  = pBmpInfo->inkSupplyNo[color];
				pInfo->jetPx0		= 0;
				pInfo->fillBt		= 0;
				pInfo->widthPx		= 1;
				pInfo->startBt		= 0;
				pInfo->widthBt		= 1;
				pInfo->srcLineLen	= 1;
				pInfo->srcLineCnt	= 1;
				pInfo->dstLineLen	= 32; // align to 256 Bits (32 Bytes) 
				pInfo->blkCnt		= 1;
				if (clearBlockUsed)	_BlkNo[pInfo->board][pInfo->head] = (_BlkNo[pInfo->board][pInfo->head]+pInfo->blkCnt)%(RX_Spooler.dataBlkCntHead);
			}
		}
	}

	//for (color=0; color<SIZEOF(pBmpInfo->colorCode); color++)
	//{
	//	rx_mem_free(&pBmpInfo->buffer[color]);
	//}
	return REPLY_OK;
}

//--- _data_split_scan_no_overlap -----------------------------------------------------------------------
static int _data_split_scan_no_overlap(SPageId *id, SBmpInfo *pBmpInfo, int offsetPx, int lengthPx, int blkNo, int clearBlockUsed, int same, SPrintListItem *pItem)
{
	//  0                         2048  2176  
	//  |                            |   |
	//  |-offsetPx-|------------------------BITMAP-----------------------------------|        
	//	|128|----------1920----------|128|----------1920----------|128|----------1920----------|128|
	//	|------------- HEAD 1------------|
	//                               |------------- HEAD 2 -----------|
	//                                                            |------ HEAD 3 ----|
	int color, n;
	int endPx;
	int startPx;
	int widthPx;
	int maxWidth;
	int headWidth;
	int head;
	int pixelPerByte;
	int colorOffset;
	struct SBmpSplitInfo	*pInfo;

	if (pBmpInfo->bitsPerPixel==0) 
		return REPLY_ERROR;
	pixelPerByte = 8/pBmpInfo->bitsPerPixel;

	//--- image info -------------------------------------------------
	//	memset(pItem->splitInfo, 0, _HeadCnt * sizeof(SBmpSplitInfo));
	//	memcpy(&pItem->id, id, sizeof(pItem->id));
	//	pItem->headsUsed = 0;

	widthPx = RX_Spooler.barWidthPx+RX_Spooler.headOverlapPx;	// stitch on right

	maxWidth = widthPx;
	//--- do the split ------------------------------------------------
	for (color=0; color<SIZEOF(pBmpInfo->colorCode); color++)
	{
		if (pBmpInfo->buffer[color])
		{
			//			rx_mem_use(pBmpInfo->buffer[color]);

			colorOffset = RX_Color[color].offsetPx;

			//	Error(WARN,0,"TEST"); colorOffset=color;

			startPx	= offsetPx-colorOffset;
			endPx	= startPx+widthPx;
			if (endPx > lengthPx) endPx = lengthPx;

			n=0;
			while((endPx>0 && startPx<endPx) || RX_Spooler.headNo[color][n]==0)
			{
				head = RX_Spooler.headNo[color][n];
				if (head==0) break;
				head--;
				headWidth = RX_Spooler.headWidthPx;
				if (n+1<RX_Spooler.headsPerColor) headWidth+=RX_Spooler.headOverlapPx;
				pInfo = &pItem->splitInfo[head];
				{	
					rx_mem_use(*pBmpInfo->buffer[color]);
					pItem->headsUsed++;
					pInfo->printMode	= pBmpInfo->printMode;
					pInfo->clearBlockUsed = clearBlockUsed;
					pInfo->same			  = same;
					pInfo->data			= pBmpInfo->buffer[color];
					pInfo->pListItem	= pItem;
					pInfo->used			= TRUE;
					pInfo->board		= head/RX_Spooler.headsPerBoard;
					pInfo->head			= head%RX_Spooler.headsPerBoard;
					pInfo->blk0			= _BlkNo[pInfo->board][pInfo->head]+RX_Spooler.dataBlkCntHead*(pInfo->head%RX_Spooler.headsPerBoard);
					pInfo->bitsPerPixel	= pBmpInfo->bitsPerPixel;
					pInfo->colorCode	= pBmpInfo->colorCode[color];
					pInfo->inkSupplyNo  = pBmpInfo->inkSupplyNo[color];
					if (startPx<0)
					{
						pInfo->fillBt  = (RX_Spooler.headOverlapPx-startPx+pixelPerByte-1)/pixelPerByte;
						pInfo->startBt = 0;
					}
					else
					{
						pInfo->startBt	= startPx/pixelPerByte;
						pInfo->fillBt   = RX_Spooler.headOverlapPx/pixelPerByte;
					}
					
					pInfo->widthPx		= endPx-startPx;
					if (pInfo->widthPx>maxWidth)	pInfo->widthPx=maxWidth;
					if (pInfo->widthPx>headWidth)	pInfo->widthPx=headWidth;
					pInfo->widthBt		= (pInfo->widthPx+pixelPerByte-1)/pixelPerByte;
					if (startPx%pixelPerByte)
					{						
						pInfo->jetPx0  = pixelPerByte-(startPx+pixelPerByte)%pixelPerByte;
						pInfo->widthPx += pixelPerByte;
						pInfo->widthBt ++;
					}
					else pInfo->jetPx0	= 0;

					pInfo->srcLineLen	= pBmpInfo->lineLen;
					pInfo->srcLineCnt	= pBmpInfo->lengthPx;
					pInfo->dstLineLen	= (pInfo->widthBt+31) & ~31; // align to 256 Bits (32 Bytes)
					pInfo->blkCnt		= (pInfo->dstLineLen * pBmpInfo->lengthPx + RX_Spooler.dataBlkSize-1) / RX_Spooler.dataBlkSize;
					if (clearBlockUsed) _BlkNo[pInfo->board][pInfo->head] = (_BlkNo[pInfo->board][pInfo->head]+pInfo->blkCnt)%(RX_Spooler.dataBlkCntHead);
					TrPrintfL(TRUE, "Split[%d]: startPx=%d, WidthPx=%d, buffer=0x%08x", head, startPx, pInfo->widthPx, pInfo->data);
				}
				//--- increment ---
				startPx += RX_Spooler.headWidthPx;
				n++;
			}
		}

		//--- fill empty heads with data ---
		for (n=0; n<SIZEOF(RX_Spooler.headNo[color]); n++)
		{	
			head=RX_Spooler.headNo[color][n];
			if (!head) break;
			head--;
			pInfo = &pItem->splitInfo[head];
			if (!pInfo->used)
			{
				pItem->headsUsed++;
				pInfo->pListItem	= pItem;
				pInfo->printMode	= pBmpInfo->printMode;
				pInfo->clearBlockUsed = clearBlockUsed;
				pInfo->same			  = same;
				pInfo->data			= NULL;
				pInfo->used			= TRUE;
				pInfo->board		= head/RX_Spooler.headsPerBoard;
				pInfo->head			= head%RX_Spooler.headsPerBoard;
				pInfo->blk0			= _BlkNo[pInfo->board][pInfo->head]+RX_Spooler.dataBlkCntHead*(pInfo->head%RX_Spooler.headsPerBoard);
				pInfo->bitsPerPixel	= 1;
				pInfo->colorCode	= pBmpInfo->colorCode[color];
				pInfo->inkSupplyNo  = pBmpInfo->inkSupplyNo[color];
				pInfo->jetPx0		= 0;
				pInfo->fillBt		= 0;
				pInfo->widthPx		= 1;
				pInfo->startBt		= 0;
				pInfo->widthBt		= 1;
				pInfo->srcLineLen	= 1;
				pInfo->srcLineCnt	= 1;
				pInfo->dstLineLen	= 32; // align to 256 Bits (32 Bytes) 
				pInfo->blkCnt		= 1;

				if (clearBlockUsed) _BlkNo[pInfo->board][pInfo->head] = (_BlkNo[pInfo->board][pInfo->head]+pInfo->blkCnt)%(RX_Spooler.dataBlkCntHead);
			}
		}
	}

	//for (color=0; color<SIZEOF(pBmpInfo->colorCode); color++)
	//{
	//	rx_mem_free(&pBmpInfo->buffer[color]);
	//}
	return REPLY_OK;
}

//--- _data_fill_blk_scan -------------------------------------------------------
static void _data_fill_blk_scan(SBmpSplitInfo *psplit, int blkNo, BYTE *dst)
{
	int		mirror=psplit->pListItem->mirror&1;
	int		len, l;
	int		size    = 0;
	int		pos		= blkNo * RX_Spooler.dataBlkSize;
	int		line	= pos / psplit->dstLineLen;
	int		start   = pos % psplit->dstLineLen;
	int		x       = start;
	int		fillLen = psplit->fillBt;
	int		dstLen  = psplit->dstLineLen-start;
	int		srcLen	= psplit->widthBt-start;
	int		endLine = psplit->srcLineCnt;
	BYTE**	ptr     = psplit->data;
	BYTE	*src    = *ptr;
		
	if (RX_Color[psplit->inkSupplyNo].rectoVerso==rv_verso) mirror = !mirror;
	if (mirror)
	{
		line	 = psplit->srcLineCnt-line-1;
		endLine  = -1;
	}
	
	src += (UINT64)line*psplit->srcLineLen+psplit->startBt%psplit->srcLineLen;
		
	start   = start%psplit->srcLineLen;

	while (size<RX_Spooler.dataBlkSize && line!=endLine)
	{
		//--- each line ---------------------
		if (fillLen)
		{
			if (size==0) // first line
			{
				if (x<fillLen) l=fillLen-x;
				else l=0;
			}
			else l=fillLen;
			if (size+l>=RX_Spooler.dataBlkSize) 
			{
				memset(&dst[size], 0x00, RX_Spooler.dataBlkSize-size);
				return;
			}
			memset(&dst[size], 0x00, l);
			size   += l;
			dstLen -= l;
			if (size==0) start = (x-fillLen)%psplit->srcLineLen; // first line
			else         start = 0;
		}
//		else if (size==0) start=psplit->startBt%psplit->srcLineLen;

		//--- copy the image data ----------------------------
		{
			if (size+srcLen>RX_Spooler.dataBlkSize) srcLen = RX_Spooler.dataBlkSize-size;
			if (size+dstLen>RX_Spooler.dataBlkSize) dstLen = RX_Spooler.dataBlkSize-size;

			for(len=0; len<srcLen; len+=l)
			{
				l = srcLen-len;
				if (start+l>psplit->srcLineLen) l=psplit->srcLineLen-start;
				memcpy(&dst[size+len], src+start, l);
				start=0;
			}
			if (dstLen>srcLen) memset(&dst[size+srcLen], 0x00, dstLen-srcLen);
		}
		start=0;
		size += dstLen;
		srcLen  = psplit->widthBt-fillLen;
		dstLen  = psplit->dstLineLen;
		if (mirror)	
		{
			src	-= psplit->srcLineLen;
			line--;
		}
		else
		{
			src	+= psplit->srcLineLen;
			line++;
		}
	}
	if (size<RX_Spooler.dataBlkSize) memset(&dst[size], 0x00, RX_Spooler.dataBlkSize-size);
}

//--- _data_fill_blk_test --------------------------------------------
static void _data_fill_blk_test(SBmpSplitInfo *psplit, int blkNo, BYTE *dst)
{
	int		mirror=psplit->pListItem->mirror&1;
	int		size    = 0;
	int		pos		= blkNo * RX_Spooler.dataBlkSize;
	int		line	= pos / psplit->dstLineLen;
	int		endLine = psplit->srcLineCnt;
	int		start	= pos % psplit->dstLineLen;
	int		srcLen	= psplit->widthBt-start;
	int		dstLen  = psplit->dstLineLen-start;

//	BYTE	*src    = &psplit->data[line*psplit->srcLineLen + psplit->startBt];
	BYTE**	ptr     = psplit->data;
	BYTE	*src    = *ptr;
	
	if (mirror)
	{
		line	 = psplit->srcLineCnt-line-1;
		endLine  = -1;
	}

	src += (UINT64)line*psplit->srcLineLen+psplit->startBt;//%psplit->srcLineLen;

	while (size<RX_Spooler.dataBlkSize && line!=endLine)
	{
		if (size+srcLen>RX_Spooler.dataBlkSize) srcLen=RX_Spooler.dataBlkSize-size;
		memcpy(&dst[size], src+start, srcLen);
		if (dstLen>srcLen) memset(&dst[size+srcLen], 0, dstLen-srcLen);
//		TrPrintfL(TRUE, " copy line=%d: src=0x%08x, len=%d, next=0x%08x", line, src, srcLen, src+psplit->srcLineLen);
		size += dstLen;
		srcLen  = psplit->widthBt;
		dstLen  = psplit->dstLineLen;
		start=0;
		if (mirror)	
		{
			src	-= psplit->srcLineLen;
			line--;
		}
		else
		{
			src	+= psplit->srcLineLen;
			line++;
		}
	}
}

//--- _data_fill_blk_single_pass ------------------------------------------------------
static void _data_fill_blk_single_pass(SBmpSplitInfo *psplit, int blkNo, BYTE *dst)
{
	int		size    = 0;
	int		pos		= blkNo * RX_Spooler.dataBlkSize;
	int		line	= pos / psplit->dstLineLen;
	int		start	= pos % psplit->dstLineLen;
	int		srcLen	= psplit->widthBt-start;
	int		dstLen  = psplit->dstLineLen-start;
	int		fillLen = psplit->fillBt;
	int		l;
//	BYTE	*src    = &psplit->data[line*psplit->srcLineLen + psplit->startBt];
	BYTE**	ptr     = psplit->data;
	BYTE	*src    = *ptr;
	src += (UINT64)line*psplit->srcLineLen+psplit->startBt%psplit->srcLineLen;

	while (size<RX_Spooler.dataBlkSize && line<psplit->srcLineCnt)
	{
		if (start>=fillLen) start -= fillLen;
		else // if (start==0)
		{
			l = fillLen-start;
			if (size+l>=RX_Spooler.dataBlkSize) 
			{
				memset(&dst[size], 0x00, RX_Spooler.dataBlkSize-size);
				return;
			}
			memset(&dst[size], 0x00, l);
			size+=l;
			dstLen-=l;
			start = 0;
		}

		if (size+srcLen>RX_Spooler.dataBlkSize) srcLen=RX_Spooler.dataBlkSize-size;
		memcpy(&dst[size], src+start, srcLen);
		if (dstLen>srcLen) memset(&dst[size+srcLen], 0, dstLen-srcLen);
//		TrPrintfL(TRUE, " copy line=%d: src=0x%08x, len=%d, next=0x%08x", line, src, srcLen, src+psplit->srcLineLen);
		size += dstLen;
		src	 += psplit->srcLineLen;
		srcLen  = psplit->widthBt;
		dstLen  = psplit->dstLineLen;
		start=0;
		line++;
	}
}

//--- data_fill_blk -------------------------------------------------------
void data_fill_blk(SBmpSplitInfo *psplit, int blkNo, BYTE *dst)
{
	if (psplit->data==NULL)
	{
		memset(dst, 0, RX_Spooler.dataBlkSize);
		return;
	}

//	if (blkNo%100==0)	TrPrintfL(TRUE, "fillBlk[%d]: line=%d, start=%d, buf=0x%08x", blkNo, line, start, psplit->data);
	switch(psplit->printMode)
	{
		case PM_TEST:		
		case PM_TEST_SINGLE_COLOR:	_data_fill_blk_test(psplit, blkNo, dst); break;
		case PM_SCANNING:			_data_fill_blk_scan(psplit, blkNo, dst); break;
//		case PM_SINGLE_PASS:		_data_fill_blk_single_pass(psplit, blkNo, dst); break;
		case PM_SINGLE_PASS:		_data_fill_blk_scan(psplit, blkNo, dst); break;
	}	
}

//--- data_sent -----------------------------------------
int data_sent(SBmpSplitInfo *psplit, int head)
{
	if (rx_sem_wait(_SendSem, 1000)==REPLY_OK)
	{
		int idx;
		idx = psplit->pListItem - _PrintList;
		// TrPrintfL(1, "Head[%d.%d]: data_sent used=%d, idx=%d", psplit->board, psplit->head, psplit->pListItem->headsUsed, idx);

		psplit->pListItem->headsUsed--;
		
		TrPrintfL(TRUE, "data_sent: headsUsed=%d, data=0x%08x", psplit->pListItem->headsUsed, psplit->data);
		if (psplit->data)
		{
			rx_mem_unuse(psplit->data);
		}
		psplit->data = NULL;

		//--- send data to rx_main_ctrl ---
		if  (psplit->pListItem->headsUsed==0)
		{
			SPrintFileMsg evt;
			static int _time=0;
			int time = rx_get_ticks();
		
	//		_OutIdx = (_OutIdx+1) % PRINT_LIST_SIZE;

			evt.hdr.msgId   = EVT_PRINT_FILE;
			evt.hdr.msgLen  = sizeof(evt);
			memcpy(&evt.id, &psplit->pListItem->id, sizeof(evt.id));
			evt.evt			= DATA_SENT;
			evt.spoolerNo   = RX_SpoolerNo;
			evt.bufReady	= data_ready();
			ctrl_send(&evt);

			TrPrintfL(TRUE, "*** FILE SENT *** idx=%d (id=%d, page=%d, copy=%d, scan=%d) time=%d ms", _OutIdx, evt.id.id, evt.id.page, evt.id.copy, evt.id.scan, time-_time);
			_time=time;
			
			extern int _MsgGot0, _MsgGot, _MsgSent, _MsgId;
			// TrPrintfL(TRUE, "EVT_PRINT_FILE: DATA_SENT _InIdx=%d, _OutIdx=%d, bufReady=%d, _MsgGot0=%d, _MsgGot=%d, _MsgSent=%d, _MsgId=0x%08x", _InIdx, _OutIdx, evt.bufReady, _MsgGot0, _MsgGot, _MsgSent, _MsgId);

			_OutIdx = (_OutIdx+1) % PRINT_LIST_SIZE;
			rx_sem_post(_SendSem);
			return TRUE;
		}		
		rx_sem_post(_SendSem);
	}
	else
	{
		printf("Timeout\n");		
	}
	return FALSE;
}

//--- data_ready -----------------------------------------------------
int  data_ready		(void)
{
	unsigned int cnt;
	cnt = _InIdx-_OutIdx;
	cnt %= PRINT_LIST_SIZE;
	int ret = rx_mem_allocated()<_MaxMemory && cnt<(unsigned int)_MaxBufers;
	if (ret==0) 
		TrPrintfL(TRUE, "bufReady=FALSE, _InIdx=%d, _OutIdx=%d", _InIdx, _OutIdx);
	return ret;
}