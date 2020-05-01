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
#include "rx_flz.h"
#include "rx_tif.h"
#include "rx_trace.h"
#include "rx_rip_lib.h"
#include "args.h"
#include "bmp.h"
#include "tcp_ip.h"
#include "spool_rip.h"
#include "ctrl_client.h"
#include "head_client.h"
#include "jet_correction.h"
#include "data_client.h"

#include "data.h"

//--- defines ------------------------------------------------------
#define PRINT_LIST_SIZE 128
#define MNT_PATH	"/mnt/ripped-data"

//--- local structures -------------------------
typedef struct
{
	int			no;
	int			progress;
	SBmpInfo	*pBmpInfo;
	UINT8		multiCopy;
	HANDLE		sem_start;
} SMultiCopyPar;

//--- static variables ---------------------------------------------

static int				_Init=FALSE;
static int				_Abort=FALSE;
static SPrintListItem	_PrintList[PRINT_LIST_SIZE];
static int				_SplitInfoSize;
static int				_InIdx;
static int				_SendIdx;
static int				_OutIdx;
static int				_ScansStart;
static int				_HeadCnt;
static UINT64			_MaxMemory=0;
static int				_MaxBufers=64;
static HANDLE			_SendSem=NULL;
static int				_Mounted=FALSE;
static int				_IsLocal=FALSE;
static int				_IsMain =TRUE;
static int				_CacheInfo=FALSE;
static int				_PrintMode;
static UINT16			_SmpFlags;
static UINT16			_SmpBufSize;
static int				_SmpOffset[MAX_COLORS];
static int				_WakeupLen;
static int				_LastWakeupLen=0; 

static SPageId			_LastId;
static int				_LastSplitId;
static SPageId			_SendingId;
static char				_LastFilePath[MAX_PATH];
static char				_FileTimePath[MAX_PATH];
static time_t			_LastFileTime;

static SBmpInfo			_LastBmpInfo;
static int				_LastColorOffset[MAX_COLORS];
static int				_LastOffset;
static UINT32			_BlkNo[HEAD_BOARD_CNT][MAX_HEADS_BOARD];

static BYTE*			_TestBuf[MAX_COLORS][MAX_HEADS_COLOR];	// buffered in case of same image

static int				_MultiCopyThreadCnt=0;
static SMultiCopyPar   *_MultiCopyPar=NULL;
static HANDLE			_MultiCopyDone;	// semaphore
static int				_ThreadRunning = TRUE;


static int  _local_path(const char *filepath, char *localPath);
static int	_file_used (const char *filename);
static int  _copy_file (SPageId *pid, char *srcDir, char *filename, char *dstDir);

static int  _data_split		           (SPageId *id, SBmpInfo *pBmpInfo, int offsetPx, int lengthPx, int blkNo, int blkCnt, int clearBlockUsed, int same, int flags, SPrintListItem *pItem);
static int  _data_split_scan           (SPageId *id, SBmpInfo *pBmpInfo, int offsetPx, int lengthPx, int blkNo, int blkCnt, int clearBlockUsed, int same, SPrintListItem *pItem);
static int  _data_split_scan_no_overlap(SPageId *id, SBmpInfo *pBmpInfo, int offsetPx, int lengthPx, int blkNo, int blkCnt, int clearBlockUsed, int same, SPrintListItem *pItem);
static int  _data_split_test           (SPageId *id, SBmpInfo *pBmpInfo, int offsetPx, int lengthPx, int blkNo, int blkCnt, int clearBlockUsed, int same, SPrintListItem *pItem);
static int _data_split_prod			   (SPageId *id, SBmpInfo *pBmpInfo, int offsetPx, int lengthPx, int blkNo, int blkCnt, int clearBlockUsed, int same, SPrintListItem *pItem);

static void _data_multi_copy		   (SPageId *id, SBmpInfo *pBmpInfo, UINT8 multiCopy);
static void _data_multi_copy_64		   (SPageId *id, SBmpInfo *pBmpInfo, UINT8 multiCopy);

static void *_multicopy_thread(void* lpParameter);


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
		_IsMain  = !strcmp(addr, RX_CTRL_MAIN) || !strcmp(addr, "127.0.0.1");
		_IsLocal = !strcmp(addr, RX_CTRL_MAIN) || !strcmp(addr, "127.0.0.1");
	}
	
	_InIdx   = 0;
	_OutIdx  = 0;
	_SendIdx = 0;
	_LastSplitId = 0;
	_CacheInfo = FALSE;
	_Abort   = FALSE;
	if (_MaxMemory==0)
	{
		_MaxMemory  = rx_mem_physical();
		_MaxMemory -= 1024*1024*1024; // leave one GB for the system
		memset(&_LastId, 0, sizeof(_LastId));
		memset(_LastFilePath, 0, sizeof(_LastFilePath));
		memset(_FileTimePath, 0, sizeof(_FileTimePath));
		memset(&_LastBmpInfo, 0, sizeof(_LastBmpInfo));
		memset(_LastColorOffset, 0, sizeof(_LastColorOffset));
	}
	memset(_BlkNo, 0, sizeof(_BlkNo));
	memset(_SmpOffset, 0xff, sizeof(_SmpOffset));

	if (_HeadCnt!=headCnt)
	{
		for (i=0; i<SIZEOF(_PrintList); i++)
		{
			if (_PrintList[i].splitInfo) 
			{
				if (i<127) free(_PrintList[i].splitInfo);
			}
			_SplitInfoSize = headCnt*sizeof(SBmpSplitInfo);
			_PrintList[i].splitInfo = (SBmpSplitInfo*)malloc(_SplitInfoSize);
		}
	}
	
	for (i=0; i<SIZEOF(_PrintList); i++)
	{
		if (_PrintList[i].splitInfo) 
		{
			memset(_PrintList[i].splitInfo, 0, _SplitInfoSize);
			for (int n=0; n<headCnt; n++)
				_PrintList[i].splitInfo[n].test = i;			
		}
	}
	
	//--- MultiCopy Threads ---------------
	if (_MultiCopyPar==NULL)
	{
		_MultiCopyThreadCnt = rx_core_cnt();
		_MultiCopyPar = (SMultiCopyPar*)malloc(_MultiCopyThreadCnt*sizeof(SMultiCopyPar));
		for (i=0; i<_MultiCopyThreadCnt; i++)
		{
			_MultiCopyPar[i].no = i;
			_MultiCopyPar[i].sem_start = rx_sem_create();
			rx_thread_start(_multicopy_thread, &_MultiCopyPar[i], 0, "_multicopy_thread");			
		}			
	}

	flz_init();
	if (rx_def_is_web(RX_Spooler.printerType)) RX_Spooler.overlap=FALSE;
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
	tif_abort();
	flz_abort();
}

//--- data_set_wakeuplen -------------------------------------------------
void data_set_wakeuplen(int len, int on)
{
	if(on)	_WakeupLen = len;
	else	_WakeupLen = -len;
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
	if (!_CacheInfo) 
		Error(LOG, 0, "Tansferring files");
	_CacheInfo=TRUE;
	{
		FILE *src=NULL;
		FILE *dst=NULL;
		BYTE *buf=NULL;
		int bufsize=10240;
		INT64 len;
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
				if ((size_t)fwrite(buf, 1, (UINT32)len, dst)!=len) break;
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
		if (buf) free(buf);
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
int  data_get_size	(const char *path, UINT32 page, UINT32 *pspacePx, UINT32 *pwidth, UINT32 *plength, UINT8 *pbitsPerPixel, UINT8 *multiCopy)
{
	int ret;
	UINT32 memsize;
	SFlzInfo info;
			
	if(flz_get_info(path, page, &info)==REPLY_OK)
	{
		*pwidth			= info.WidthPx+*pspacePx;
		*plength		= info.lengthPx+2*abs(_WakeupLen);
		*pbitsPerPixel	= info.bitsPerPixel;
		ret = REPLY_OK;
	}
	else
	{
		ret = tif_get_size(path, page, *pspacePx, pwidth, plength, pbitsPerPixel);
		if (ret == REPLY_NOT_FOUND) 
		{
			char filepath[MAX_PATH];
			bmp_color_path(path, RX_ColorNameShort(0), filepath);
			ret = bmp_get_size(filepath, pwidth, plength, pbitsPerPixel, &memsize);
		}
	}
	
	*multiCopy = 1;
	if (ret==REPLY_OK && (RX_Spooler.printerType==printer_TX801 || RX_Spooler.printerType==printer_TX802))
	{
		*plength += 2*abs(_WakeupLen);

		//--- allign width to full byte ---
		if (*pspacePx)
		{
			UINT32 width=*pwidth;
			*pwidth    = ((width*(*pbitsPerPixel)+7)&~7) / (*pbitsPerPixel);
			*pspacePx += (*pwidth)-width;
		}
		else
		{
			// possible improvement:
			//	1. read the file
			//	2. check last byte over all rows
			//	3. if all bytes are "0" we don't need the multiCopy option
			int pixPByte = 8/(*pbitsPerPixel);
			for ((*multiCopy)=1; ((*pwidth)*(*multiCopy)) % pixPByte; (*multiCopy)++)
			{
			};				
			*pwidth *= (*multiCopy);
		}
	}
	return ret;
}

//--- data_clear ---------------------------------
void data_clear(BYTE* buffer[MAX_COLORS])
{
	int n;
	for (n=0; n<MAX_COLORS; n++)
		rx_mem_use_clear(buffer[n]);		
}

//--- data_memsize --------------------------------------------
UINT64 data_memsize(int printMode, UINT32 width, UINT32 height, UINT8 bitsPerPixel)
{
	UINT64	memsize;
	int lineLen;

//	lineLen = (width*bitsPerPixel+7)/8;			// workes for TIFF
//	lineLen = ((width*bitsPerPixel+15)/16)*2;	// used for BMP files!
//	lineLen = ((width*bitsPerPixel+31) & ~31)/8;	// used for BMP files!
	lineLen = ((width*bitsPerPixel+63) & ~63)/8;	// used in case it is multicopy!
	memsize = (UINT64)lineLen*height;			
	switch(printMode)
	{
    case PM_TEST:				memsize*=RX_Spooler.headsPerColor; break; // Density Test
	case PM_TEST_JETS:			memsize*=RX_Spooler.headsPerColor; break;
	case PM_TEST_SINGLE_COLOR:	memsize*=RX_Spooler.headsPerColor; break;
	default:													   break;
	}
	
	return (memsize/(1024*1024)+1) * (1024*1024); 
}

//--- data_malloc -------------------------------------------------------------------------
int  data_malloc(int printMode, UINT32 width, UINT32 height, UINT8 bitsPerPixel, SColorSplitCfg *psplit, int splitCnt, UINT64 *pBufSize, BYTE* buffer[MAX_COLORS])
{	
	UINT64	memsize;
	int i, found, error;

	memsize = data_memsize(printMode, width, height, bitsPerPixel);

	_PrintMode = printMode;

	error=0;
	if(memsize <= *pBufSize)
	{
	//	Error(LOG, 0, "data_malloc: use mem %dMB of %dMB", memsize/1024/1024, (*pBufSize)/1024/1024);
		for (i=0; i<MAX_COLORS; i++)
		{
			if (psplit[i].color.name[0] && psplit[i].lastLine>psplit[i].firstLine)
			{
				TrPrintfL(1, "buffer[%d]: WAIT UNUSED %p, used=%d, abort=%d", i, buffer[i], rx_mem_cnt(buffer[i]), _Abort);
				while (!_Abort && rx_mem_cnt(buffer[i]))
				{
					rx_sleep(10);
				}
				TrPrintfL(1, "buffer[%d]: IS UNUSED", i);								
			}
		}
	}
	else
	{
	//	Error(LOG, 0, "data_malloc: Change mem from %dMB to %dMB", (*pBufSize)/1024/1024, memsize/1024/1024);
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
						rx_mem_free(&buffer[i]);
						if (!buffer[i]) break;
						rx_sleep(10);
					}
					*pBufSize = 0;

					TrPrintfL(1, "buffer[%d]: IS FREE", i);
					if (!_Abort)
					{
					//	TrPrintfL(1, "data_malloc buffer 1[%d], %p, free=%d MB, size=%d MB", i, buffer[i], rx_mem_get_freeMB(), memsize/1024/1024);
						buffer[i] = rx_mem_alloc(memsize);
						TrPrintfL(1, "data_malloc buffer[%d], %p, free=%d MB, size=%d MB", i, buffer[i], rx_mem_get_freeMB(), memsize/1024/1024);
						if (buffer[i]==NULL) error=TRUE;
						*pBufSize = memsize;
					}
				}
			}
		}
		*pBufSize = memsize;
	}
		
	if (error || _Abort)
	{
		for (i=0; i<MAX_COLORS; i++)
		{
			if (buffer[i]) rx_mem_free(&buffer[i]);
		}
		*pBufSize=0;
		return REPLY_ERROR;
	}

	/*
	//--- clear buffer ----------------------------------------
	{
		for (i=0; i<MAX_COLORS; i++)
		{
			if (buffer[i]) memset(buffer[i], 0x55, *pBufSize);
		}
	}
	*/
	return REPLY_OK;
}

//--- data_free ----------------------------------------------
int  data_free(UINT64 *pBufSize, BYTE* buffer[MAX_COLORS])
{
	int i;
	for (i=0; i<MAX_COLORS; i++)
	{
		if (buffer[i]) rx_mem_free(&buffer[i]);
	}
	*pBufSize=0;
	return REPLY_OK;			
}

//--- _flz_data_loaded -----------------------
static void _flz_data_loaded(void *data)
{
	SPrintListItem	*pitem = (SPrintListItem*)data;
	pitem->decompressing = FALSE;
	hc_send_next();
}

//--- data_load ------------------------------------------------------------------------
int data_load(SPageId *id, const char *filepath, int offsetPx, int lengthPx, UINT8 multiCopy, int gapPx, int blkNo, int blkCnt, int printMode, int variable, UINT8 virtualPasses, UINT8 virtualPass, int flags, int clearBlockUsed, int same, int smp_bufSize, BYTE* buffer[MAX_COLORS])
{
	SBmpInfo		bmpInfo;
	int ret;
	int newOffsets;		// image must be reloaded for jet correction!
	int color;
	int nextIdx;
	int loaded=FALSE;
	char filename[MAX_PATH];
	char *tiffErr;
	
//	Error(LOG, 0, "data_load id=%d, page=%d, copy=%d, scan=%d", id->id, id->page, id->copy, id->scan);
	TrPrintfL(TRUE, "data_load id=%d, page=%d, copy=%d, scan=%d, offsetPx=%d, data_load >>%s<<", id->id, id->page, id->copy, id->scan, offsetPx, filepath);
	TrPrintfL(TRUE, "FirstPage=%d, LastPage=%d", flags & FLAG_SMP_FIRST_PAGE, flags & FLAG_SMP_LAST_PAGE);
	
	if (flags & FLAG_SMP_LAST_PAGE) 
		Error(LOG, 0, "data_load id=%d, page=%d, copy=%d, scan=%d, data_load >>%s<< LAST PAGE", id->id, id->page, id->copy, id->scan, filepath);
		
 	nextIdx = (_InIdx+1) % PRINT_LIST_SIZE;
	if (nextIdx == _OutIdx) 
		return Error(ERR_CONT, 0, "Print List Overflow");
	
	if (variable)	ret = sr_rip_label (buffer, &bmpInfo); 
	else
	{			
		newOffsets = FALSE;
		if (jc_active()) 
		{
			for (color=0; color<MAX_COLORS; color++)
			{
				if (RX_Color[color].offsetPx!=_LastColorOffset[color]) newOffsets =TRUE;
				_LastColorOffset[color] = RX_Color[color].offsetPx;
			}
			if (offsetPx != _LastOffset)
			{
				_LastOffset = offsetPx;
				if (rx_def_is_web(RX_Spooler.printerType)) newOffsets =TRUE;
			}
		}		
		
	//	if (/*id->id!=_LastId.id || */ id->page!=_LastId.page || strcmp(filepath, _LastFilePath) || _WakeupLen!=_LastWakeupLen || newOffsets || rx_file_get_mtime (_FileTimePath)!=_LastFileTime || printMode==PM_TEST_JETS)
		if (/*id->id!=_LastId.id || */ id->page!=_LastId.page || strcmp(filepath, _LastFilePath) || _WakeupLen!=_LastWakeupLen || newOffsets || rx_file_get_mtime (_FileTimePath)!=_LastFileTime) // || printMode==PM_TEST_JETS) Overwrites head info!
		{
			ret = 1;
			loaded = TRUE;
			memset(_LastFilePath, 0, sizeof(_LastFilePath));
			if (_PrintMode==PM_SCAN_MULTI_PAGE && !(flags & FLAG_SMP_FIRST_PAGE))
				ctrl_pause_printing();
			bmpInfo.printMode = printMode;
			if (printMode==PM_TEST || printMode==PM_TEST_JETS || printMode==PM_TEST_SINGLE_COLOR)
			{
				for (color=0; color<MAX_COLORS; color++)
				{
					if (buffer[color])
					{
						ret=bmp_load(filepath, &buffer[color], 100000, &bmpInfo);
						if (ret==REPLY_OK) strcpy(_FileTimePath, filepath);
//						ret=tif_load_simple(filepath, &buffer[color], 100000, &bmpInfo);
						if (TRUE)
						{
							char str[MAX_PATH];
							sprintf(str, "%sTEST.bmp", PATH_TEMP);

							bmp_write(str, buffer[color], bmpInfo.bitsPerPixel, bmpInfo.srcWidthPx, bmpInfo.lengthPx, bmpInfo.lineLen, TRUE);
						}
						bmpInfo.buffer[color] = &buffer[color];
						bmpInfo.inkSupplyNo[color]=0;
						bmpInfo.colorCode[color]=0;
					}
				}
			}
			split_path(filepath, NULL, filename, NULL);
			int time0=rx_get_ticks();
			if (ret)
			{		
				if(RX_Spooler.printerType==printer_DP803)
				{
					flz_loaded = _flz_data_loaded;
					ret = flz_load(id, filepath, filename, printMode, gapPx, RX_Color, SIZEOF(RX_Color), buffer, &bmpInfo, NULL, (void*)&_PrintList[_InIdx]);
					if (ret==REPLY_OK) strcpy(_FileTimePath, flz_last_filepath());
					else					
					{
						ret = tif_load_mt(id, filepath, filename, printMode, gapPx, RX_Color, SIZEOF(RX_Color), buffer, &bmpInfo, NULL);
						strcpy(_FileTimePath, tif_last_filepath());
					}
				}
				else
				{
					ret = tif_load(id, filepath, filename, printMode, gapPx, _WakeupLen, RX_Color, SIZEOF(RX_Color), buffer, &bmpInfo, ctrl_send_load_progress);
					strcpy(_FileTimePath, tif_last_filepath());				
				}
				
				{ // test ------------
					int i;
					for (i=0; i<RX_Spooler.colorCnt; i++)
						TrPrintfL(TRUE, "ColorData[%d] Loaded to buffer %03d", i, ctrl_get_bufferNo(buffer[i]));
				}
			}
//			Error(LOG, 0, "Page %d: Tif Load Time=%d ms, _Abort=%d", id->page, rx_get_ticks()-time0, _Abort);
			if (ret==REPLY_NOT_FOUND)
			{
				tiffErr=tif_error();
				if (*tiffErr) Error(ERR_CONT, 0, "File not found: >>%s<<", str_start_cut(tiffErr, PATH_RIPPED_DATA));
			}
				
			if (ret) ret = bmp_load_all(filepath, printMode, RX_Color, SIZEOF(RX_Color), buffer, &bmpInfo);

			/* is done in split !
			if (printMode==PM_SCANNING)
			{
//				Error(LOG, 0, "Reset _BlkNo, blkNo=%d", blkNo);
				memset(_BlkNo, 0, sizeof(_BlkNo));			
			}
			*/
			if (*_FileTimePath) _LastFileTime=rx_file_get_mtime (_FileTimePath);
		}
		else
		{
			memcpy(&bmpInfo, &_LastBmpInfo, sizeof(bmpInfo));
			ret=REPLY_OK;
		}
		if (_Abort) 
			return REPLY_ERROR;
		ctrl_start_printing();
	}
	TrPrintfL(TRUE, "data_load: File loaded, ret=%d", ret);

//	if (ret == REPLY_NOT_FOUND) ret = bmp_load_all(filepath, RX_Color, SIZEOF(RX_Color), &bmpInfo);
	if (ret==REPLY_OK)
	{	
		_PrintList[_InIdx].lengthPx		 = bmpInfo.lengthPx;
		_PrintList[_InIdx].virtualPasses = virtualPasses; 
		_PrintList[_InIdx].virtualPass   = virtualPass;
		_SmpFlags   = flags;
		_SmpBufSize = smp_bufSize;
		int time=rx_get_ticks();
		if (multiCopy>1)
		{
			_data_multi_copy(id, &bmpInfo, multiCopy);
			time = rx_get_ticks()-time;
		//	if (time) Error(LOG, 0, "MultiCopy time=%d ms", time);				
		}
		_data_split(id, &bmpInfo, offsetPx, lengthPx, blkNo, blkCnt, flags, clearBlockUsed, same, &_PrintList[_InIdx]);
		
		if (loaded || printMode==PM_TEST || printMode==PM_TEST_JETS || printMode==PM_TEST_SINGLE_COLOR)
		{
			if      (printMode==PM_TEST_JETS && id->id==PQ_TEST_JET_NUMBERS) jc_correction(&bmpInfo, &_PrintList[_InIdx], 4385);
			else if (printMode!=PM_TEST && printMode!=PM_TEST_SINGLE_COLOR)  jc_correction(&bmpInfo, &_PrintList[_InIdx], 0);
		}
		#ifdef DEBUG
		if (FALSE && loaded)
//		if (loaded)
		{
			int i;
			char str[MAX_PATH];
			for (i=0; i<SIZEOF(bmpInfo.buffer); i++)
			{
			//	if (bmpInfo.buffer[i])
				if (buffer[i] && RX_ColorNameShort(bmpInfo.inkSupplyNo[i])[0]=='K')
				{
				//	sprintf(str, "%sPAGE_%d_%s.bmp", PATH_TEMP, id->page, RX_ColorNameShort(bmpInfo.inkSupplyNo[i]));
					sprintf(str, "%sID_%d_%s.bmp", PATH_HOME PATH_RIPPED_DATA_DIR "trace/", id->id, RX_ColorNameShort(bmpInfo.inkSupplyNo[i]));
				//	if (multiCopy > 2)
				//		bmp_write(str, *bmpInfo.buffer[i], bmpInfo.bitsPerPixel, bmpInfo.srcWidthPx / multiCopy + 200, 2000, bmpInfo.lineLen, FALSE);
				//	else					
						bmp_write(str, *bmpInfo.buffer[i], bmpInfo.bitsPerPixel, bmpInfo.srcWidthPx, bmpInfo.lengthPx, bmpInfo.lineLen, FALSE);
					Error(WARN, 0, "Test: Written bitmap to >>%s<<", str);
				}
			}
			TrPrintfL(TRUE, "Testfile Written");				
		}
		#endif
	
	//	if (loaded) Error(WARN, 0, "data_load DONE idx=%d, id=%d, page=%d, copy=%d, scan=%d, data_load >>%s<<, COPY=%d", _InIdx, id->id, id->page, id->copy, id->scan, filepath, _PrintList[_InIdx].id.copy);
	
		TrPrintfL(TRUE, "data_load DONE idx=%d, id=%d, page=%d, copy=%d, scan=%d, data_load >>%s<<, COPY=%d", _InIdx, id->id, id->page, id->copy, id->scan, filepath, _PrintList[_InIdx].id.copy);
		_InIdx = nextIdx;
	}
	memcpy(&_LastId, id, sizeof(_LastId));
	memcpy(&_LastBmpInfo, &bmpInfo, sizeof(bmpInfo));
	strcpy(_LastFilePath, filepath);
	_LastWakeupLen = _WakeupLen;
	return ret;
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
//--- data_same ------------------------------------------------------------------
int data_same(SPageId *id)
{
	int nextIdx;
	int src;

	nextIdx = (_InIdx+1) % PRINT_LIST_SIZE;
	if (nextIdx == _OutIdx) 
		return Error(ERR_CONT, 0, "Print List Overflow");

	for (src=0; src<SIZEOF(_PrintList); src++)
	{
		SPageId *p = &_PrintList[src].id;
		if ((id->id==p->id) && (id->page==p->page) && (id->scan==p->scan))
		{				
			SBmpSplitInfo *ptr=_PrintList[_InIdx].splitInfo;
			memcpy(&_PrintList[_InIdx], &_PrintList[src], sizeof(_PrintList[_InIdx]));
			_PrintList[_InIdx].splitInfo = ptr;
			memcpy(_PrintList[_InIdx].splitInfo, _PrintList[src].splitInfo, _SplitInfoSize);
			memcpy(&_PrintList[_InIdx].id, id, sizeof(_PrintList[_InIdx].id));
			
			for (int h=0; h<_HeadCnt; h++)
			{
				_PrintList[_InIdx].splitInfo[h].pListItem = &_PrintList[_InIdx];
			}
		
			/*
			TrPrintfL(TRUE, "data_same: PrintList[%d].idx=%d", _InIdx, data_printList_idx(_PrintList[_InIdx].splitInfo[0].pListItem));
			{
				int i;
				for (i=0; i<=_InIdx; i++)
				{
					TrPrintfL(TRUE, "_PrintList[%d].id=%d, p=%d, c=%d, idx=%d, splitinfo=0x%08x", i, _PrintList[i].id.id, _PrintList[i].id.page, _PrintList[i].id.copy, data_printList_idx(_PrintList[i].splitInfo[0].pListItem), _PrintList[i].splitInfo);
				}
			}
			*/
			
			_InIdx = nextIdx;
			
			return REPLY_OK;
		}
	}
	

	return Error(ERR_CONT, 0, "Same Data not found");;
}

//--- data_send_id ------------------------------------------------------
void data_send_id(SPageId *id)
{
	if (id)
	{
	    TrPrintfL(TRUE, "data_send_id id=%d", id->id);
		memcpy(&_SendingId, id, sizeof(_SendingId));
		if (arg_tracePQ) Error(LOG, 0, "data_send_id _SendingId=%d", _SendingId.id);
	}
	else
    {
	    TrPrintfL(TRUE, "data_send_id id=NULL");
        memset(&_SendingId, 0, sizeof(_SendingId));
	}
}

//--- data_get_next ----------------------------------------------------------------
SBmpSplitInfo*  data_get_next	(int *headCnt)
{
	int idx;
		
	TrPrintfL(TRUE, "data_get_next _InIdx=%d, _SendIdx=%d, _OutIdx=%d, _SendingId.id=%d", _InIdx, _SendIdx, _OutIdx, _SendingId.id);
	if (_SendIdx!=_OutIdx) return NULL;
	if (_SendIdx==_InIdx) return NULL;

	if (_PrintList[_SendIdx].decompressing)
	{
		TrPrintfL(TRUE, "data_get_next _PrintList[_SendIdx].decompressing", _SendIdx);
		return NULL;		
	}
	
	if (_PrintMode==PM_SCAN_MULTI_PAGE)
	{
		if (!(_SmpFlags & FLAG_SMP_LAST_PAGE))
		{
			int cnt = (_InIdx-_SendIdx) % PRINT_LIST_SIZE;
		//	Error(LOG, 0, "data_get_next _InIdx=%d, _OutIdx=%d, _SendIdx=%d, cnt=%d, buf=%d", _InIdx, _OutIdx, _SendIdx, cnt, _SmpBufSize);
			if(cnt < _SmpBufSize + 1)
			{
			//	Error(LOG, 0, "data_get_next WAIT");
				return NULL;
			}
		}
	}
	
	if (_SendingId.id && _PrintList[_SendIdx].id.id!=_SendingId.id) 
	{
		TrPrintfL(TRUE, "data_get_next _PrintList[%d].id=%d _SendingId=%d", _SendIdx, _PrintList[_SendIdx].id.id, _SendingId.id);	
		return NULL;		
	}
		
	idx = _SendIdx;
	_SendIdx = (_SendIdx+1) % PRINT_LIST_SIZE;
	*headCnt = _HeadCnt;
	_PrintList[idx].headsInUse = _PrintList[idx].headsUsed;
	SPageId *pid=&_PrintList[idx].id;
	TrPrintfL(TRUE, "data_get_next idx=%d, headsUsed=%d (id=%d, page=%d, copy=%d, scan=%d)", idx, _PrintList[idx].headsUsed, pid->id, pid->page, pid->copy, pid->scan);
	
	/*
	{
		int i;
		for (i=0; i<_InIdx; i++)
		{
			TrPrintfL(TRUE, "_PrintList[%d].id=%d, p=%d, c=%d, idx=%d", i, _PrintList[i].id.id, _PrintList[i].id.page, _PrintList[i].id.copy, data_printList_idx(_PrintList[i].splitInfo[0].pListItem));
		}
		TrPrintfL(TRUE, "_InIdx=%d, _OutIdx=%d, _SendIdx=%d, idx=%d", _InIdx, _OutIdx, _SendIdx, idx);
	}
	
	TrPrintfL(TRUE, "data_get_next: idx=%d, (id=%d, p=%d, c=%d) pl[%d]", idx, _PrintList[idx].id.id, _PrintList[idx].id.page, _PrintList[idx].id.copy, data_printList_idx(_PrintList[idx].splitInfo[0].pListItem));
	*/
	
	return _PrintList[idx].splitInfo;	
}

//--- data_next_id -------------------------
int	 data_next_id(void)
{
	int lastidx=(_OutIdx+PRINT_LIST_SIZE-1)%PRINT_LIST_SIZE;
	if (arg_tracePQ) Error(LOG, 0, "data_next_id _SendingId=%d, lastidx=%d, outIdx=%d, sendIdx=%d, last.id=%d, out.id=%d, send.id=%d", _SendingId.id, lastidx, _OutIdx, _SendIdx, _PrintList[lastidx].id.id, _PrintList[_OutIdx].id.id, _PrintList[_SendIdx].id.id);
	if (_PrintList[lastidx].id.id==_SendingId.id && _PrintList[_OutIdx].id.id!=_SendingId.id && _PrintList[_OutIdx].id.id)
	{
		data_send_id(&_PrintList[_OutIdx].id);
		return TRUE;
	}
	else if (arg_tracePQ) Error(LOG, 0, "data_next_id KEEP");
	return FALSE;
}

//--- _data_multi_copy_64 ---------------------------------------------------------------
static void _data_multi_copy_64(SPageId *id, SBmpInfo *pBmpInfo, UINT8 multiCopy)
{
	if (multiCopy>1 && !pBmpInfo->multiCopy)
	{
		BYTE *src;
		BYTE *dst;
		
		int  buf;
		int  x, y, m;
		int srcLineBt  = (pBmpInfo->srcWidthPx*pBmpInfo->bitsPerPixel+7)/8;
		int srcLinelen = pBmpInfo->lineLen;
		int dstLineLen = (((INT64)pBmpInfo->srcWidthPx*multiCopy*pBmpInfo->bitsPerPixel+63) & ~63)/8;
		
		for (buf=0; buf<SIZEOF(pBmpInfo->buffer); buf++)
		{
			if (pBmpInfo->buffer[buf] && pBmpInfo->lengthPx>0)
			{
				src = (*pBmpInfo->buffer[buf]) + (INT64)pBmpInfo->lengthPx;
				dst = (*pBmpInfo->buffer[buf]) + (INT64)dstLineLen*pBmpInfo->lengthPx;

				for (y=pBmpInfo->lengthPx; y>0;)
				{
					y--;
					src -= srcLinelen;
					dst -= dstLineLen;
					memcpy(dst, src, srcLineBt);
					if (y) memset(src, 0x00, srcLineBt);
					for (m=1; m<multiCopy; m++)
					{
						int shr = m*8/multiCopy;
						int shl = 64-shr;
						UINT64 *s64 = (UINT64*)dst;
						UINT64 *d64 = (UINT64*)(dst + m*pBmpInfo->srcWidthPx*pBmpInfo->bitsPerPixel/8);
						UINT64 data=0;
						UINT64 old;
						for (x=0; x<srcLineBt; x+=8)
						{
							old     = data<<shl;
							data    = swap_uint64(*s64++);
							*d64++ |= swap_uint64(old | data>>shr);
						}
					}
				}
				Error(LOG, 0, "MultiCopy[%d] done", buf);
			}
		}
		pBmpInfo->srcWidthPx *= multiCopy;
		pBmpInfo->lineLen     = dstLineLen;
		pBmpInfo->multiCopy   = multiCopy;
	}
}

static void _data_multi_copy(SPageId *pid, SBmpInfo *pBmpInfo, UINT8 multiCopy)
{
	int i;
	if (multiCopy>1 && !pBmpInfo->multiCopy)
	{
		int running = _MultiCopyThreadCnt;
		int done;			
	//	Error(LOG, 0, "MultiCopy=%d", multiCopy);

		_MultiCopyDone  = rx_sem_create();

		for (i=0; i<_MultiCopyThreadCnt; i++)
		{
			_MultiCopyPar[i].pBmpInfo  = pBmpInfo;
			_MultiCopyPar[i].multiCopy = multiCopy;
			rx_sem_post(_MultiCopyPar[i].sem_start);
		}
		//--- wait ----
		while (running)
		{
			if (rx_sem_wait(_MultiCopyDone, 500) == REPLY_OK) running--;
			else
			{
				for (i=0, done=0; i<_MultiCopyThreadCnt; i++) done += _MultiCopyPar[i].progress;
					ctrl_send_load_progress(pid, "MultiCopy", 101*done / (pBmpInfo->lengthPx*pBmpInfo->colorCnt));
			}
		}
		ctrl_send_load_progress(pid, "MultiCopy", 100);
		rx_sem_destroy(&_MultiCopyDone);

		pBmpInfo->srcWidthPx *= multiCopy;
		pBmpInfo->lineLen = (((INT64)pBmpInfo->srcWidthPx*pBmpInfo->bitsPerPixel+63) & ~63)/8;
		pBmpInfo->multiCopy   = multiCopy;	
	}							
}

static void *_multicopy_thread(void* lpParameter)
{
	SMultiCopyPar *par=(SMultiCopyPar*)lpParameter;
	while (_ThreadRunning)
	{
		rx_sem_wait(par->sem_start, 0);
		{
			SBmpInfo *pBmpInfo = par->pBmpInfo;
			UINT8 multiCopy	   = par->multiCopy;

			int  buf;
			int  x, y, m;
			int srcLineBt  = (pBmpInfo->srcWidthPx*pBmpInfo->bitsPerPixel+7)/8;
			int srcLinelen = pBmpInfo->lineLen;
			int dstLineLen = (((INT64)pBmpInfo->srcWidthPx*multiCopy*pBmpInfo->bitsPerPixel+63) & ~63)/8;
			par->progress=0;
					
			for (buf=par->no; buf<SIZEOF(pBmpInfo->buffer); buf+=_MultiCopyThreadCnt)
			{
				if (pBmpInfo->buffer[buf] && pBmpInfo->lengthPx>0)
				{
				//	(*pBmpInfo->buffer[buf])[(_WakeupLen-1)*srcLinelen] = 0xc0;
				//	(*pBmpInfo->buffer[buf])[(_WakeupLen-2)*srcLinelen+pBmpInfo->srcWidthPx/4] = 0x03<<(2*(4-pBmpInfo->srcWidthPx%4));
				//	(*pBmpInfo->buffer[buf])[(_WakeupLen-3)*srcLinelen+pBmpInfo->srcWidthPx/4] = 0x03<<6;
				//	(*pBmpInfo->buffer[buf])[(_WakeupLen-4)*srcLinelen+pBmpInfo->srcWidthPx/4] = 0x03<<4;
				//	(*pBmpInfo->buffer[buf])[(_WakeupLen-5)*srcLinelen+pBmpInfo->srcWidthPx/4] = 0x03<<2;
				//	(*pBmpInfo->buffer[buf])[(_WakeupLen-6)*srcLinelen+pBmpInfo->srcWidthPx/4] = 0x03<<0;
					BYTE *src = (*pBmpInfo->buffer[buf]) + (INT64)srcLinelen*pBmpInfo->lengthPx;
					BYTE *dst = (*pBmpInfo->buffer[buf]) + (INT64)dstLineLen*pBmpInfo->lengthPx;
					for (y=pBmpInfo->lengthPx; y>0; par->progress++)
					{
						y--;
						src -= srcLinelen;
						dst -= dstLineLen;
						memcpy(dst, src, srcLineBt);
						memset(dst+srcLineBt, 0x00, dstLineLen-srcLineBt);
						if (y) memset(src, 0x00, srcLineBt);
						/*
						for (m=1; m<multiCopy; m++)
						{
							int shr = m*8/multiCopy;
							int shl = 64-shr;
							UINT64 *s64 = (UINT64*)dst;
							UINT64 *d64 = (UINT64*)(dst + m*pBmpInfo->srcWidthPx*pBmpInfo->bitsPerPixel/8);
							UINT64 data=0;
							UINT64 old;
							for (x=0; x<srcLineBt; x+=8)
							{
								old     = data<<shl;
								data    = swap_uint64(*s64++);
								*d64++ |= swap_uint64(old | data>>shr);
							}
						}
						*/
						//--- 8 bit ---------------------
						for (m=1; m<multiCopy; m++)
						{
							int shr = (m*pBmpInfo->srcWidthPx*pBmpInfo->bitsPerPixel)%8;
							int shl = 8-shr;
							UINT8 *s8 = (UINT8*)dst;
							UINT8 *d8 = (UINT8*)(dst + (m*pBmpInfo->srcWidthPx*pBmpInfo->bitsPerPixel)/8);
							UINT8 data=0;
							UINT8 old;
							for (x=0; x<=srcLineBt; x+=1)
							{
								old     = data<<shl;
								data    = *s8++;
								*d8++  |= old | data>>shr;
							}
						}
					}
				}
			}
		}

		rx_sem_post(_MultiCopyDone);
	}
	return NULL;
}

//--- _data_split -----------------------------------------------------------------------
static int _data_split(SPageId *id, SBmpInfo *pBmpInfo, int offsetPx, int lengthPx, int blkNo, int blkCnt, int flags, int clearBlockUsed, int same, SPrintListItem *pItem)
{		
	if (pBmpInfo->printMode==PM_SCANNING && id->id!=_LastSplitId)
	{
		_LastSplitId = id->id;
		memset(_BlkNo, 0, sizeof(_BlkNo));
	}
	
	int test=pItem->splitInfo->test;
	memset(pItem->splitInfo, 0, _HeadCnt * sizeof(SBmpSplitInfo));
	for (int i=0; i<_HeadCnt; i++)
		pItem->splitInfo[i].test=test;
	memcpy(&pItem->id, id, sizeof(pItem->id));
	pItem->flags     = flags;
	pItem->headsUsed = 0;
		
	switch(pBmpInfo->printMode)
	{
		case PM_TEST_SINGLE_COLOR:
		case PM_TEST_JETS:
									return _data_split_test(id, pBmpInfo, offsetPx, lengthPx, blkNo, blkCnt, clearBlockUsed, same, pItem);

		case PM_SCANNING:			
		case PM_SCAN_MULTI_PAGE:	return _data_split_prod(id, pBmpInfo, offsetPx, lengthPx, blkNo, blkCnt, clearBlockUsed, same, pItem);

									if (RX_Spooler.overlap)	
										return _data_split_scan(id, pBmpInfo, offsetPx, lengthPx, blkNo, blkCnt,clearBlockUsed, same, pItem);
									else		
										return _data_split_scan_no_overlap(id, pBmpInfo, offsetPx, lengthPx, blkNo, blkCnt ,clearBlockUsed, same, pItem);

		case PM_SINGLE_PASS:		// return _data_split_prod(id, pBmpInfo, offsetPx, min(RX_Spooler.barWidthPx, (int)pBmpInfo->srcWidthPx), blkNo, blkCnt, clearBlockUsed, same, pItem);
									return _data_split_scan(id, pBmpInfo, offsetPx, min(RX_Spooler.barWidthPx, (int)pBmpInfo->srcWidthPx), blkNo, blkCnt, clearBlockUsed, same, pItem);
		default:					return Error(ERR_STOP, 0, "Splitting algorithme not implemented");
	}
}

//--- _data_split_test ---------------------------------------------------------------------
static int _data_split_test(SPageId *id, SBmpInfo *pBmpInfo, int offsetPx, int lengthPx, int blkNo, int blkCnt, int clearBlockUsed, int same, SPrintListItem *pItem)
{
	int color, n, head;
	struct SBmpSplitInfo	*pInfo;

	TrPrintfL(TRUE, "_data_split_test");

	for (color=0; color<SIZEOF(RX_Spooler.headNo); color++)
	{
		if (RX_Color[color].spoolerNo==RX_SpoolerNo)
		{
			for (n=0; n<SIZEOF(RX_Spooler.headNo[color]); n++)
			{	
				head=RX_Spooler.headNo[color][n];
				if (!head) break;
				head--;
				pInfo = &pItem->splitInfo[head];
				rx_mem_use(*pBmpInfo->buffer[color]);
			
				pItem->headsUsed++;
				pInfo->pListItem		= pItem;
				pInfo->printMode		= pBmpInfo->printMode;
				pInfo->clearBlockUsed	= clearBlockUsed;
				pInfo->same				= same;
				pInfo->data				= pBmpInfo->buffer[color];
			
				if ((id->id==PQ_TEST_JETS || id->id==PQ_TEST_JET_NUMBERS) && pInfo->data)
				{
					_TestBuf[color][n] = (*pBmpInfo->buffer[color])+n*pBmpInfo->dataSize;
					if (id->copy+id->scan==1 && n) memcpy(_TestBuf[color][n], *pBmpInfo->buffer[color], pBmpInfo->dataSize);					
					pInfo->data	= &_TestBuf[color][n];
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
				if (pInfo->blkCnt>blkCnt) Error(ERR_ABORT, 0, "Data: blkCnt=%d, max=%d", pInfo->blkCnt, blkCnt);

				if (pBmpInfo->printMode==PM_TEST_SINGLE_COLOR && (color+1)!=id->scan)
				{
					//--- empty bitmap -------
					pInfo->data			= NULL;
					pInfo->bitsPerPixel	= 1;
					pInfo->widthPx		= 1;
					pInfo->widthBt		= 1;
					pInfo->srcWidthBt	= 1;
					pInfo->srcLineLen	= 1;
					pInfo->srcLineCnt	= 1;
					pInfo->dstLineLen	= 32; // align to 256 Bits (32 Bytes)				
					pInfo->blkCnt		= 1;
				}

				if (rx_def_is_web(RX_Spooler.printerType) 
				&& (id->id==PQ_TEST_JETS || id->id==PQ_TEST_JET_NUMBERS)  
				&& (RX_Spooler.colorCnt==0 || ((id->copy-1)%RX_Spooler.colorCnt)!=color))
				{
					//--- only one color per PrintGo  -------
					pInfo->data			= NULL;
					pInfo->bitsPerPixel	= 1;
					pInfo->widthPx		= 1;
					pInfo->widthBt		= 1;
					pInfo->srcWidthBt	= 1;
					pInfo->srcLineLen	= 1;
					pInfo->srcLineCnt	= 1;
					pInfo->dstLineLen	= 32; // align to 256 Bits (32 Bytes)				
					pInfo->blkCnt		= 1;				
				}
				
				//--- rip the test data -----------------------------------------
				if (pInfo->data && (id->id==PQ_TEST_JETS || id->id==PQ_TEST_JET_NUMBERS))
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
	}
	return REPLY_OK;
}

//--- _data_split_prod -----------------------------------------------------------------------
static int _data_split_prod(SPageId *id, SBmpInfo *pBmpInfo, int offsetPx, int lengthPx, int blkNo, int blkCnt, int clearBlockUsed, int same, SPrintListItem *pItem)
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
	int barWidthPx;
	int headWidthPx;
	int head;
	int pixelPerByte;
	int colorOffset;
	int firstFillPx;
	SBmpSplitInfo	*pInfo;

	if (pBmpInfo->bitsPerPixel==0) 
		return REPLY_ERROR;
	pixelPerByte = 8/pBmpInfo->bitsPerPixel;

	barWidthPx  = RX_Spooler.barWidthPx + RX_Spooler.headOverlapPx;	// stitch on right
	if (RX_Spooler.overlap) barWidthPx	+= RX_Spooler.headOverlapPx;	// stitch on left+right

	//--- do the split ------------------------------------------------
	for (color=0; color<SIZEOF(pBmpInfo->colorCode); color++)
	{
		if (RX_Color[color].spoolerNo==RX_SpoolerNo && pBmpInfo->buffer[color])
		{
//			rx_mem_use(pBmpInfo->buffer[color]);

			headWidthPx = RX_Spooler.headWidthPx+RX_Spooler.headOverlapPx;
			if (RX_Spooler.overlap) headWidthPx += RX_Spooler.headOverlapPx;

			colorOffset = RX_Color[color].offsetPx;

			startPx	= offsetPx-colorOffset;
			endPx	= startPx+barWidthPx;
			if (endPx > lengthPx) endPx = lengthPx;
			
			n=0;
			if (rx_def_is_tx(RX_Spooler.printerType) && !RX_Spooler.overlap) firstFillPx = RX_Spooler.headOverlapPx;
			else															 firstFillPx = 0;

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
				//	if (RX_Spooler.printerType==printer_LB702_UV)
					if (rx_def_is_web(RX_Spooler.printerType))
						pInfo->blk0			= blkNo+RX_Spooler.dataBlkCntHead*(pInfo->head%RX_Spooler.headsPerBoard);
					else
						pInfo->blk0			= _BlkNo[pInfo->board][pInfo->head]+RX_Spooler.dataBlkCntHead*(pInfo->head%RX_Spooler.headsPerBoard);
					pInfo->bitsPerPixel	= pBmpInfo->bitsPerPixel;
					pInfo->colorCode	= pBmpInfo->colorCode[color];
					pInfo->inkSupplyNo  = pBmpInfo->inkSupplyNo[color];

					if (startPx<0)
					{
                        pInfo->fillBt  = (firstFillPx-startPx+pixelPerByte-1)/pixelPerByte;
						pInfo->startBt = 0;
					}
					else
					{
                        pInfo->fillBt  = firstFillPx / pixelPerByte;
						pInfo->startBt = startPx/pixelPerByte;
					}
					pInfo->widthPx		= endPx-startPx+pInfo->fillBt*pixelPerByte;
					
					if (color==4 && id->scan==6)
						printf("Black\n");

					if (pInfo->widthPx>barWidthPx)	pInfo->widthPx=barWidthPx;
					if (pInfo->widthPx>headWidthPx)	pInfo->widthPx=headWidthPx;

					pInfo->widthBt		= (pInfo->widthPx+pixelPerByte-1)/pixelPerByte;
					if (startPx%pixelPerByte)
					{		
						if (startPx<0) 	pInfo->jetPx0  = pixelPerByte-(startPx+RX_Spooler.headWidthPx+pixelPerByte)%pixelPerByte;
						else		    pInfo->jetPx0  = pixelPerByte-(startPx+pixelPerByte)%pixelPerByte;
						pInfo->widthPx += pixelPerByte;
						pInfo->widthBt ++;
					}
					else pInfo->jetPx0	= 0;

					pInfo->srcWidthBt	= (pBmpInfo->srcWidthPx*pBmpInfo->bitsPerPixel)/8;
					pInfo->srcLineLen	= pBmpInfo->lineLen;
					pInfo->srcLineCnt	= pBmpInfo->lengthPx;
					pInfo->dstLineLen	= (pInfo->widthBt+31) & ~31; // align to 256 Bits (32 Bytes)
					pInfo->blkCnt		= (pInfo->dstLineLen * pBmpInfo->lengthPx + RX_Spooler.dataBlkSize-1) / RX_Spooler.dataBlkSize;
					if (pInfo->blkCnt>blkCnt) Error(ERR_ABORT, 0, "Data: blkCnt=%d, max=%d", pInfo->blkCnt, blkCnt);
					if (clearBlockUsed) _BlkNo[pInfo->board][pInfo->head] = (_BlkNo[pInfo->board][pInfo->head]+pInfo->blkCnt)%(RX_Spooler.dataBlkCntHead);
					TrPrintfL(TRUE, "Split[%d.%d]: startPx=%d, WidthPx=%d, FillBt=%d, buffer=%03d, blk0=%d, blkCnt=%d", pInfo->board,  pInfo->head, startPx, pInfo->widthPx, pInfo->fillBt, ctrl_get_bufferNo(*pInfo->data), pInfo->blk0, pInfo->blkCnt);
				}
				//--- increment ---
				if (rx_def_is_web(RX_Spooler.printerType)) startPx += RX_Spooler.headWidthPx;
				else									   
				{
					startPx    += (RX_Spooler.headWidthPx - firstFillPx);
					if (n+2>=RX_Spooler.headsPerColor) headWidthPx = RX_Spooler.headWidthPx;
				}
				firstFillPx =0;
				n++;
			}
		}

		//--- fill empty heads with data ---
		for (n=0; n<SIZEOF(RX_Spooler.headNo[color]); n++)
		{	
			if (RX_Color[color].spoolerNo==RX_SpoolerNo)
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
				//	if (RX_Spooler.printerType==printer_LB702_UV)
					if (rx_def_is_web(RX_Spooler.printerType))
						pInfo->blk0			= blkNo+RX_Spooler.dataBlkCntHead*(pInfo->head%RX_Spooler.headsPerBoard);
					else
						pInfo->blk0			= _BlkNo[pInfo->board][pInfo->head]+RX_Spooler.dataBlkCntHead*(pInfo->head%RX_Spooler.headsPerBoard);
					pInfo->bitsPerPixel	= 1;
					pInfo->colorCode	= pBmpInfo->colorCode[color];
					pInfo->inkSupplyNo  = pBmpInfo->inkSupplyNo[color];
					pInfo->jetPx0		= 0;
					pInfo->fillBt		= 0;
					pInfo->widthPx		= 1;
					pInfo->startBt		= 0;
					pInfo->widthBt		= 1;
					pInfo->srcWidthBt	= 1;
					pInfo->srcLineLen	= 1;
					pInfo->srcLineCnt	= 1;
					pInfo->dstLineLen	= 32; // align to 256 Bits (32 Bytes) 
					pInfo->blkCnt		= 1;
					if (clearBlockUsed)	_BlkNo[pInfo->board][pInfo->head] = (_BlkNo[pInfo->board][pInfo->head]+pInfo->blkCnt)%(RX_Spooler.dataBlkCntHead);
				}
			}
		}	
	}
	
	//for (color=0; color<SIZEOF(pBmpInfo->colorCode); color++)
	//{
	//	rx_mem_free(&pBmpInfo->buffer[color]);
	//}
	return REPLY_OK;
}

//--- _data_split_scan -----------------------------------------------------------------------
static int _data_split_scan(SPageId *id, SBmpInfo *pBmpInfo, int offsetPx, int lengthPx, int blkNo, int blkCnt, int clearBlockUsed, int same, SPrintListItem *parItem)
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
	int colorOffsetScans;	
	SPrintListItem	*pItem;
	SBmpSplitInfo	*pInfo;

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
		if (RX_Color[color].spoolerNo==RX_SpoolerNo && pBmpInfo->buffer[color])
		{
//			rx_mem_use(pBmpInfo->buffer[color]);

			colorOffsetScans= 0;
			colorOffset = RX_Color[color].offsetPx;

			//	Error(WARN,0,"TEST"); colorOffset=color;

			if (pBmpInfo->printMode==PM_SCAN_MULTI_PAGE && !(_SmpFlags&FLAG_SMP_FIRST_PAGE))
			{				
				colorOffsetScans = colorOffset / (RX_Spooler.barWidthPx+RX_Spooler.headOverlapPx);
				colorOffset	     = colorOffset % (RX_Spooler.barWidthPx+RX_Spooler.headOverlapPx);				
			}

			startPx	= offsetPx-colorOffset;
			endPx	= startPx+widthPx;
			if (endPx > lengthPx) endPx = lengthPx;
			
			n=0;
			while((endPx>0 && startPx<endPx) || RX_Spooler.headNo[color][n]==0)
			{
				head = RX_Spooler.headNo[color][n];
				if (head==0) break;
				head--;
				
				if(colorOffsetScans)
				{
					int idx=(_InIdx+PRINT_LIST_SIZE-_SmpBufSize+colorOffsetScans)%PRINT_LIST_SIZE;										
					pItem = &_PrintList[idx];
					pInfo = &_PrintList[idx].splitInfo[head];						
				}
				else
				{
					pItem = parItem;
					pInfo = &pItem->splitInfo[head];						
				}
								
				{	
					rx_mem_use(*pBmpInfo->buffer[color]);
					if (pItem==parItem) pItem->headsUsed++;
					pInfo->printMode	= pBmpInfo->printMode;
					pInfo->clearBlockUsed = clearBlockUsed;
					pInfo->same			  = same;
					pInfo->data			= pBmpInfo->buffer[color];
					pInfo->pListItem	= pItem;
					pInfo->used			= TRUE;
					pInfo->board		= head/RX_Spooler.headsPerBoard;
					pInfo->head			= head%RX_Spooler.headsPerBoard;
				//	if (rx_def_is_web(RX_Spooler.printerType))
					if (RX_Spooler.printerType==printer_LB702_UV)
						pInfo->blk0			= blkNo+RX_Spooler.dataBlkCntHead*(pInfo->head%RX_Spooler.headsPerBoard);
					else
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
						pInfo->fillBt	= 0;	
						pInfo->startBt	= startPx/pixelPerByte;
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

					pInfo->srcWidthBt	= (pBmpInfo->srcWidthPx*pBmpInfo->bitsPerPixel)/8;
					pInfo->srcLineLen	= pBmpInfo->lineLen;
					pInfo->srcLineCnt	= pBmpInfo->lengthPx;
					pInfo->dstLineLen	= (pInfo->widthBt+31) & ~31; // align to 256 Bits (32 Bytes)
					pInfo->blkCnt		= (pInfo->dstLineLen * pBmpInfo->lengthPx + RX_Spooler.dataBlkSize-1) / RX_Spooler.dataBlkSize;
					if (pInfo->blkCnt>blkCnt) Error(ERR_ABORT, 0, "Data: blkCnt=%d, max=%d", pInfo->blkCnt, blkCnt);
					if (clearBlockUsed)
					{
						int blkNo=_BlkNo[pInfo->board][pInfo->head];
						_BlkNo[pInfo->board][pInfo->head] = (_BlkNo[pInfo->board][pInfo->head]+pInfo->blkCnt)%(RX_Spooler.dataBlkCntHead);
						// TrPrintfL(TRUE, "Split[%d.%d]: blkOld=%d, blkNew=%d", pInfo->board, pInfo->head, blkNo, _BlkNo[pInfo->board][pInfo->head]);
					}
					// TrPrintfL(TRUE, "Split[%d.%d]: startPx=%d, WidthPx=%d, fillBt=%d, jetPx0=%d, pixelPerByte=%d, buffer=0x%08x, blkNo=%d", pInfo->board, pInfo->head, startPx, pInfo->widthPx, pInfo->fillBt,  pInfo->jetPx0, pixelPerByte, pInfo->data, pInfo->blk0);
				}
				//--- increment ---
				startPx += RX_Spooler.headWidthPx;
				n++;
			}
		}

		//--- fill empty heads with data ---
		pItem = parItem;
		for (n=0; n<SIZEOF(RX_Spooler.headNo[color]); n++)
		{	
			if (RX_Color[color].spoolerNo==RX_SpoolerNo)
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
				//	if (rx_def_is_web(RX_Spooler.printerType))
					if (RX_Spooler.printerType==printer_LB702_UV)
						pInfo->blk0			= blkNo+RX_Spooler.dataBlkCntHead*(pInfo->head%RX_Spooler.headsPerBoard);
					else
						pInfo->blk0			= _BlkNo[pInfo->board][pInfo->head]+RX_Spooler.dataBlkCntHead*(pInfo->head%RX_Spooler.headsPerBoard);
					pInfo->bitsPerPixel	= 1;
					pInfo->colorCode	= pBmpInfo->colorCode[color];
					pInfo->inkSupplyNo  = pBmpInfo->inkSupplyNo[color];
					pInfo->jetPx0		= 0;
					pInfo->fillBt		= 0;
					pInfo->widthPx		= 1;
					pInfo->startBt		= 0;
					pInfo->widthBt		= 1;
					pInfo->srcWidthBt	= 1;
					pInfo->srcLineLen	= 1;
					pInfo->srcLineCnt	= 1;
					pInfo->dstLineLen	= 32; // align to 256 Bits (32 Bytes) 
					pInfo->blkCnt		= 1;
					if (clearBlockUsed)	_BlkNo[pInfo->board][pInfo->head] = (_BlkNo[pInfo->board][pInfo->head]+pInfo->blkCnt)%(RX_Spooler.dataBlkCntHead);
				}
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
static int _data_split_scan_no_overlap(SPageId *id, SBmpInfo *pBmpInfo, int offsetPx, int lengthPx, int blkNo, int blkCnt, int clearBlockUsed, int same, SPrintListItem *parItem)
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
	int colorOffsetScans;
	int idx, cnt;
	SPrintListItem			*pItem;
	struct SBmpSplitInfo	*pInfo;
	
	if (pBmpInfo->bitsPerPixel==0) 
		return REPLY_ERROR;
	pixelPerByte = 8/pBmpInfo->bitsPerPixel;
	
	//--- image info -------------------------------------------------
	widthPx = RX_Spooler.barWidthPx+RX_Spooler.headOverlapPx;	// stitch on right
	maxWidth = widthPx;
		
	//--- do the split ------------------------------------------------
	for (color=0; color<SIZEOF(pBmpInfo->colorCode); color++)
	{
		if (RX_Color[color].spoolerNo==RX_SpoolerNo && pBmpInfo->buffer[color])
		{
			//			rx_mem_use(pBmpInfo->buffer[color]);
			colorOffsetScans= 0;
			colorOffset		= RX_Color[color].offsetPx;				

		//	if (RX_Color[color].color.colorCode==3)
		//		printf("Yellow\n");

			if (pBmpInfo->printMode==PM_SCAN_MULTI_PAGE && !(_SmpFlags&FLAG_SMP_FIRST_PAGE))
			{				
		//		if (RX_Color[color].color.colorCode==0)
		//			printf("Black\n");
				colorOffsetScans = colorOffset / RX_Spooler.barWidthPx;
				colorOffset	     = colorOffset % RX_Spooler.barWidthPx;
			}
			
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
				
				if (_SmpOffset[color]<0) _SmpOffset[color]=_InIdx;				
				if(pBmpInfo->printMode == PM_SCAN_MULTI_PAGE && !(_SmpFlags&FLAG_SMP_FIRST_PAGE))
//					 idx = (_InIdx + PRINT_LIST_SIZE - _SmpBufSize + _SmpOffset[color] + 1) % PRINT_LIST_SIZE;
					 idx = (_InIdx + PRINT_LIST_SIZE - colorOffsetScans) % PRINT_LIST_SIZE;
				else idx = _InIdx;

				pItem = &_PrintList[idx];
				pInfo = &_PrintList[idx].splitInfo[head];	
				
				{	
					rx_mem_use(*pBmpInfo->buffer[color]);
					int cnt=rx_mem_cnt(*pBmpInfo->buffer[color]);

					if (!pInfo->used) pItem->headsUsed++;

					pInfo->printMode	= pBmpInfo->printMode;
					pInfo->clearBlockUsed = clearBlockUsed;
					pInfo->same			  = same;
					pInfo->data			= pBmpInfo->buffer[color];
					pInfo->pListItem	= pItem;
					pInfo->used			= TRUE;
					pInfo->board		= head/RX_Spooler.headsPerBoard;
					pInfo->head			= head%RX_Spooler.headsPerBoard;

					TrPrintfL(TRUE, "SPLIT _BlkNo[%d][%d]: idx=%d, old=%d, cnt=%d, buffer=%03d, test=%d", pInfo->board, pInfo->head, idx, pInfo->blk0, pInfo->blkCnt, ctrl_get_bufferNo(*pInfo->data), pInfo->test);
					rx_sleep(1);
					
					pInfo->blk0			= _BlkNo[pInfo->board][pInfo->head]+RX_Spooler.dataBlkCntHead*(pInfo->head%RX_Spooler.headsPerBoard);
					pInfo->bitsPerPixel	= pBmpInfo->bitsPerPixel;
					pInfo->colorCode	= pBmpInfo->colorCode[color];
					pInfo->inkSupplyNo  = pBmpInfo->inkSupplyNo[color];

					if (startPx<0)
					{
						if (n==0) pInfo->fillBt  = (RX_Spooler.headOverlapPx-startPx+pixelPerByte-1)/pixelPerByte;
						else      pInfo->fillBt  = (-startPx+pixelPerByte-1)/pixelPerByte;
						pInfo->startBt = 0;
					}
					else
					{
						if (n==0) pInfo->fillBt = RX_Spooler.headOverlapPx / pixelPerByte;
						else	  pInfo->fillBt	= 0;
						pInfo->startBt	= startPx/pixelPerByte;
					}

				//	if (RX_Color[color].color.colorCode==3)
				//		printf("Yellow\n");
					pInfo->widthPx		= endPx-startPx;
					pInfo->widthPx += pInfo->fillBt*pixelPerByte;
					
					if (color==4 && id->scan==6)
						printf("black\n");

					if (pInfo->widthPx>maxWidth)	pInfo->widthPx=maxWidth;
					if (pInfo->widthPx>headWidth)	pInfo->widthPx=headWidth;
					pInfo->widthBt		= (pInfo->widthPx+pixelPerByte-1)/pixelPerByte;
					if (startPx%pixelPerByte)
					{					
						if (startPx<0) 	pInfo->jetPx0  = pixelPerByte-(startPx+RX_Spooler.headWidthPx+pixelPerByte)%pixelPerByte;
						else		    pInfo->jetPx0  = pixelPerByte-(startPx+pixelPerByte)%pixelPerByte;
						pInfo->widthPx += pixelPerByte;
						pInfo->widthBt ++;
					}
					else pInfo->jetPx0	= 0;

					pInfo->srcWidthBt	= (pBmpInfo->srcWidthPx*pBmpInfo->bitsPerPixel)/8;
					pInfo->srcLineLen	= pBmpInfo->lineLen;
					pInfo->srcLineCnt	= pBmpInfo->lengthPx;
					pInfo->dstLineLen	= (pInfo->widthBt+31) & ~31; // align to 256 Bits (32 Bytes)
					pInfo->blkCnt		= (pInfo->dstLineLen * pBmpInfo->lengthPx + RX_Spooler.dataBlkSize-1) / RX_Spooler.dataBlkSize;
					if (pInfo->blkCnt>blkCnt) Error(ERR_ABORT, 0, "Data: blkCnt=%d, max=%d", pInfo->blkCnt, blkCnt);
					if (clearBlockUsed)
					{			
						int blk=_BlkNo[pInfo->board][pInfo->head];
						_BlkNo[pInfo->board][pInfo->head] = (_BlkNo[pInfo->board][pInfo->head]+pInfo->blkCnt)%(RX_Spooler.dataBlkCntHead);
						TrPrintfL(TRUE, "SPLIT _BlkNo[%d][%d]: idx=%d, act=%d, cnt=%d, next=%d, blk0=%d, buffer=%03d, test=%d", pInfo->board, pInfo->head, idx, blk, pInfo->blkCnt, _BlkNo[pInfo->board][pInfo->head], pInfo->blk0, ctrl_get_bufferNo(*pInfo->data), pInfo->test);
						rx_sleep(1);
					}
					TrPrintfL(TRUE, "Split[%d.%d]: startPx=%d, WidthPx=%d, FillBt=%d, buffer=%03d, blk0=%d, blkCnt=%d", pInfo->board,  pInfo->head, startPx, pInfo->widthPx, pInfo->fillBt, ctrl_get_bufferNo(*pInfo->data), pInfo->blk0, pInfo->blkCnt);
				}				

				//--- increment ---
				if (n==0) startPx += (RX_Spooler.headWidthPx - RX_Spooler.headOverlapPx); 
				else	  startPx += RX_Spooler.headWidthPx;
				n++;
			}
		}
		
		//--- fill empty heads with data ---
		for (n=0; n<SIZEOF(RX_Spooler.headNo[color]); n++)
		{	
			if(RX_Color[color].spoolerNo==RX_SpoolerNo)
			{
				head=RX_Spooler.headNo[color][n];
				if (!head) break;
				head--;
				if (pBmpInfo->printMode==PM_SCAN_MULTI_PAGE && !(_SmpFlags&FLAG_SMP_FIRST_PAGE))
					idx =(_InIdx+PRINT_LIST_SIZE- _SmpBufSize+_SmpOffset[color]+1)%PRINT_LIST_SIZE;
				else
					idx = _InIdx;
				cnt = (_SmpFlags&FLAG_SMP_LAST_PAGE)? PRINT_LIST_SIZE : 1;
				for(; cnt; cnt--, idx=(idx+1)%PRINT_LIST_SIZE)
				{
					pItem = &_PrintList[idx];
					pInfo = &_PrintList[idx].splitInfo[head];
					
					if (!pInfo->used)
					{
					//	Error(LOG, 0, "_PrintList[%d-%d][%d] EMPTY,  _InIdx=%d, _SmpBufSize=%d", head/4+1, head%4, idx, _InIdx, _SmpBufSize);
					
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

			//			if (clearBlockUsed) _BlkNo[pInfo->board][pInfo->head] = (_BlkNo[pInfo->board][pInfo->head]+pInfo->blkCnt)%(RX_Spooler.dataBlkCntHead);
						if (clearBlockUsed)
						{			
							int blk=_BlkNo[pInfo->board][pInfo->head];
							_BlkNo[pInfo->board][pInfo->head] = (_BlkNo[pInfo->board][pInfo->head]+pInfo->blkCnt)%(RX_Spooler.dataBlkCntHead);
							TrPrintfL(TRUE, "SPLIT _BlkNo[%d][%d]: idx=%d, act=%d, cnt=%d, next=%d, EMPTY, blk0=%d, test=%d", pInfo->board, pInfo->head, idx, blk, pInfo->blkCnt, _BlkNo[pInfo->board][pInfo->head], pInfo->blk0, pInfo->test);
							rx_sleep(1);
						}
					}
				}
				if (idx==_InIdx) break;
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
static void _data_fill_blk_scan(SBmpSplitInfo *psplit, int blkNo, BYTE *dst, int test)
{
	int		mirror= psplit->pListItem->flags&FLAG_MIRROR;
	int		flags = psplit->pListItem->flags;
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
	
	BYTE	*test0  = &dst[-1];	// overwrites BlockNo!
	BYTE	*test1  = &dst[RX_Spooler.dataBlkSize];
	BYTE    t0=*test0;
	BYTE	t1=*test1;

	if (test)
	{
		*test0 = 0x33;
		*test1 = 0x66;
	}
	
	if (RX_Color[psplit->inkSupplyNo].rectoVerso==rv_verso) mirror = !mirror;
	if (mirror)
	{
		line	 = psplit->srcLineCnt-line-1;
		endLine  = -1;
	}
	
	src += (UINT64)line*psplit->srcLineLen; // +psplit->startBt%psplit->srcLineLen;

//	printf("head=%d, blkNo=%d, dstLen=%d\n", psplit->head, blkNo, dstLen);
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
			if (l>dstLen) 
				l=dstLen;
			if (size+l>=RX_Spooler.dataBlkSize) 
			{
				memset(&dst[size], 0x00, RX_Spooler.dataBlkSize-size);
				if (test)
				{
				 	if (*test0!=0x33) Error(ERR_STOP, 0, "_data_fill_blk_scan lower border");
				 	if (*test1!=0x66) Error(ERR_STOP, 0, "_data_fill_blk_scan upper border");
				 	*test0=t0;
				 	*test1=t1;
				}
				return;
			}
			memset(&dst[size], 0x00, l);
			if (test)
			{
			 	if (*test0!=0x33) 
					Error(ERR_STOP, 0, "_data_fill_blk_scan lower border");
			 	if (*test1!=0x66) 
					Error(ERR_STOP, 0, "_data_fill_blk_scan upper border");
			}
	//		printf("head=%d, blkNo=%d, fill(%d, %d), dstLen=%d\n", psplit->head, blkNo, size, l, dstLen-l);
			size   += l;
			dstLen -= l;
			if (size==0) start = x-fillLen; // first line
			else         start = 0;
		}
//		start = (start+psplit->startBt) % psplit->srcLineLen;
		start = (start+psplit->startBt) % psplit->srcWidthBt;

		/*
		if (psplit->colorCode==0)
		{
			static int lastscan = 1234;
			if (psplit->pListItem->id.scan==4)
				printf("Black scan=%d, block=%d\n", psplit->pListItem->id.scan, blkNo);
			lastscan = psplit->pListItem->id.scan;
		}
		*/
		
		//--- copy the image data ----------------------------
		{
			if (size+srcLen>RX_Spooler.dataBlkSize) srcLen = RX_Spooler.dataBlkSize-size;
			if (size+dstLen>RX_Spooler.dataBlkSize) dstLen = RX_Spooler.dataBlkSize-size;
			int srcWidthBt = psplit->srcWidthBt - start;
			int srcLineLen = psplit->srcWidthBt - start;

			for(len=0; len<srcLen; len+=l)
			{
				l = srcLen-len;
				if (l > srcWidthBt) l = srcWidthBt;
				memcpy(&dst[size+len], src+start, l);
				if (test)
				{
					if (*test0!=0x33) Error(ERR_STOP, 0, "_data_fill_blk_scan lower border size=%d, len=%d, l=%d, max=%d, data=%d", size, len, l, RX_Spooler.dataBlkSize, dst[RX_Spooler.dataBlkSize]);
					if (*test1!=0x66) Error(ERR_STOP, 0, "_data_fill_blk_scan upper border size=%d, len=%d, l=%d, max=%d, data=%d", size, len, l, RX_Spooler.dataBlkSize, dst[RX_Spooler.dataBlkSize]);	
				}
				start=0;
				srcWidthBt = psplit->srcWidthBt;
				srcLineLen = psplit->srcLineLen;
			}
			if (dstLen>srcLen && srcLen>0) memset(&dst[size+srcLen], 0x00, dstLen-srcLen);
			if (test)
			{
			 	if (*test0!=0x33) Error(ERR_STOP, 0, "_data_fill_blk_scan lower border");
			 	if (*test1!=0x66) Error(ERR_STOP, 0, "_data_fill_blk_scan upper border");					
			}
		}
		
		//---------------------------------
		if (psplit->pListItem->virtualPasses 
		&& (psplit->pListItem->penetrationPasses<2 || psplit->colorCode!=30)) // penetration is colorode 30
		{	
			if ((line % psplit->pListItem->virtualPasses) != psplit->pListItem->virtualPass) memset(&dst[size], 0x00, dstLen);
			/*
			if (TRUE)
			{
				if ((flags&FLAG_PASS_1OF2) && (line&1)==1) memset(&dst[size], 0x00, dstLen);
				if ((flags&FLAG_PASS_2OF2) && (line&1)==0) memset(&dst[size], 0x00, dstLen);
			}
			else
			{
				BYTE mask;
				BYTE *data;
				int len;
				if (flags&FLAG_PASS_1OF2) mask =  0x33; 
				if (flags&FLAG_PASS_2OF2) mask = ~0x33;
				if (line&1) mask=~mask;
				for (data=&dst[size], len=dstLen; len; len--) *data++ &= mask;
			}
			*/
		}
		//-----------------------------------
		
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
	if (test)
	{
	 	if (*test0!=0x33) Error(ERR_STOP, 0, "_data_fill_blk_scan lower border");
	 	if (*test1!=0x66) Error(ERR_STOP, 0, "_data_fill_blk_scan upper border");
	 	*test0=t0;
	 	*test1=t1;		
	}
}

//--- _data_fill_blk_test --------------------------------------------
static void _data_fill_blk_test(SBmpSplitInfo *psplit, int blkNo, BYTE *dst)
{
	int		mirror=psplit->pListItem->flags&FLAG_MIRROR;
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
/*
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
*/

//--- data_fill_blk -------------------------------------------------------
void data_fill_blk(SBmpSplitInfo *psplit, int blkNo, BYTE *dst, int test)
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
		case PM_TEST_SINGLE_COLOR:	
		case PM_TEST_JETS:
									_data_fill_blk_test(psplit, blkNo, dst); break;
		case PM_SCANNING:			
		case PM_SCAN_MULTI_PAGE:	_data_fill_blk_scan(psplit, blkNo, dst, test); break;
//		case PM_SINGLE_PASS:		_data_fill_blk_single_pass(psplit, blkNo, dst); break;
		case PM_SINGLE_PASS:		_data_fill_blk_scan(psplit, blkNo, dst, test); break;
	}	
}

//--- data_sent -----------------------------------------
int data_sent(SBmpSplitInfo *psplit, int head)
{
	if (rx_sem_wait(_SendSem, 1000)==REPLY_OK)
	{
		int idx;
		idx = (int)(psplit->pListItem - _PrintList);
	//	TrPrintfL(1, "Head[%d.%d]: data_sent used=%d, idx=%d", psplit->board, psplit->head, psplit->pListItem->headsInUse, idx);
		
		psplit->pListItem->headsInUse--;
		
//		TrPrintfL(TRUE, "data_sent: headsInUse=%d, data=0x%08x", psplit->pListItem->headsInUse, psplit->data);
		if (psplit->data)
		{
			rx_mem_unuse(psplit->data);
			int cnt1=rx_mem_cnt(*psplit->data);
//			if (head==0)
//				Error(LOG, 0, "rx_mem_unuse[0]=%d\n", cnt1);
		}
		psplit->data = NULL;

		if (FALSE)
		{
			SPrintFileMsg evt;
					
			memcpy(&evt.id, &psplit->pListItem->id, sizeof(evt.id));
			TrPrintfL(TRUE, "data_sent idx=%d (id=%d, page=%d, copy=%d, scan=%d) headsInUse=%d", _OutIdx, evt.id.id, evt.id.page, evt.id.copy, evt.id.scan, psplit->pListItem->headsInUse);
		}
		
		//--- send data to rx_main_ctrl ---
		if  (psplit->pListItem->headsInUse==0)
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

			if (RX_Spooler.printerType==printer_DP803) 
			{
				double lengthPx = psplit->pListItem->lengthPx/(1200.0*40.0);
				double speed=1000;
				if (time!=_time)
				{
					speed= lengthPx * 60 * 1000 / (time-_time);
				}
			//	Error(LOG, 0, "FILE SENT: (id=%d, page=%d, copy=%d, scan=%d) cycletime=%d ms, speed=%d m/min", evt.id.id, evt.id.page, evt.id.copy, evt.id.scan, time-_time, (int)speed);

			//	TrPrintfL(TRUE, "*** FILE SENT *** idx=%d (id=%d, page=%d, copy=%d, scan=%d) cycletime=%d ms", _OutIdx, evt.id.id, evt.id.page, evt.id.copy, evt.id.scan, time-_time);
			}
			TrPrintfL(TRUE, "*** FILE SENT *** idx=%d (id=%d, page=%d, copy=%d, scan=%d) cycletime=%d ms", _OutIdx, evt.id.id, evt.id.page, evt.id.copy, evt.id.scan, time-_time);
			_time=time;
			
			extern int _MsgGot0, _MsgGot, _MsgSent, _MsgId;
			// TrPrintfL(TRUE, "EVT_PRINT_FILE: DATA_SENT _InIdx=%d, _OutIdx=%d, bufReady=%d, _MsgGot0=%d, _MsgGot=%d, _MsgSent=%d, _MsgId=0x%08x", _InIdx, _OutIdx, evt.bufReady, _MsgGot0, _MsgGot, _MsgSent, _MsgId);
			{ //--- used for printqueue handling of TX machines ------------------------------
				int lastidx=(_OutIdx+PRINT_LIST_SIZE-1)%PRINT_LIST_SIZE;
				memset(&_PrintList[lastidx].id, 0, sizeof(_PrintList[lastidx].id));
			}
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

//--- data_printList_idx ------------------------------------------
int	 data_printList_idx(SPrintListItem *pitem)
{
	return (int)(pitem-_PrintList);
};