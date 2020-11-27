// ****************************************************************************
//
//	screening.c
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
#include "bmp.h"
#include "tcp_ip.h"
#include "spool_rip.h"
#include "ctrl_client.h"
#include "head_client.h"
#include "jet_correction.h"
#include "data_client.h"
#include "rx_slicescreen.h"
#include "rx_screen_fms_1x3.h"
#include "gpu.h"

#include "data.h"
#include "screening.h"

//--- defines --------------------------------------

#define BITS_PER_PIXEL_SEP	8
#define BITS_PER_PIXEL_SCR	2

#define SCR_FIFO_SIZE	128
#define SCR_BUF_SIZE	2

//--- local structures -------------------------
typedef struct
{
	UINT64	sizeSep;
	UINT64	sizeScr;
	BYTE	*separated;
	BYTE	*screened[SCR_BUF_SIZE];
	int		screenedIdx;
	double	fact[MAX_DENSITY_FACTORS];	// 0x00..0xff
} SScrMem;

//--- static variables ---------------------------------------------
static int					_Init=FALSE;
static int					_TimeTotal;
static int					_TimeStart;
static int					_TimeEnd;
static int					_GPU=FALSE;

static SPageId				_Id;
static SPlaneScreenConfig	_PlaneScreenConfig[HEAD_BOARD_CNT*MAX_HEADS_BOARD];
static char					_PlaneFilePath[HEAD_BOARD_CNT*MAX_HEADS_BOARD][MAX_PATH];

static SScrMem				_ScrMem[HEAD_BOARD_CNT][MAX_HEADS_BOARD];

static HANDLE				_SemScreeningReady;
static HANDLE				_SemScreeningStart;
static HANDLE				_SemScreeningDone;
static SBmpSplitInfo*		_ScrFifo[SCR_FIFO_SIZE];
static HANDLE				_ScrFifoMutex;
static int					_ScrFifoInIdx;
static int					_ScrFifoOutIdx;
static int					_ScrFifoDoneIdx;

static int					_ScrThreadRunning;
static int					_ScrThreadCnt;

static void _write_tif(char *title, SBmpSplitInfo *pInfo, PBYTE *buffer, int lineLen);

static void _scr_fill_blk(SBmpSplitInfo *psplit, int dstLineLen, BYTE *dst);
static void _scr_load(SBmpSplitInfo *pInfo, int threadNo);
static void *_screening_thread(void* lpParameter);

//--- scr_init ----------------------------------------------
void scr_init(void)
{
	memset(_PlaneFilePath, 0, sizeof(_PlaneFilePath));
	_GPU = (gpu_init()==REPLY_OK);
	_TimeTotal=0;
	_TimeStart=_TimeEnd=0;

	if (!_Init)
	{
		_Init = TRUE;
		memset(_ScrMem, 0, sizeof(_ScrMem));
		for(int board=0; board<HEAD_BOARD_CNT; board++)
		{
			for(int head=0; head<MAX_HEADS_BOARD; head++)
			{
				memset(_ScrMem[board][head].fact, 0xff, sizeof(_ScrMem[board][head].fact));
			}
		}
		_ScrFifoInIdx		= 0;
		_ScrFifoOutIdx		= 0;
		_ScrFifoDoneIdx		= 0;
		_ScrFifoMutex		= rx_mutex_create();
		_SemScreeningStart	= rx_sem_create();
		_SemScreeningDone	= rx_sem_create();
		
		_ScrThreadRunning = TRUE;
		_ScrThreadCnt = (_GPU)? gpu_stream_cnt():rx_core_cnt();
		for (int i=0; i<_ScrThreadCnt; i++)
		{
			char name[MAX_PATH];
			sprintf(name, "Screening[%d]", i);
			rx_thread_start(_screening_thread, (void*)(size_t)i, 0, name);
		}
	}

	if (_SemScreeningReady) rx_sem_destroy(&_SemScreeningReady);
	_SemScreeningReady	= rx_sem_create();
	rx_sem_post(_SemScreeningReady);
}

//--- scr_end --------------------------
void scr_end(void)
{
	for (int i=0; i<SIZEOF(_PlaneScreenConfig); i++)
	{
		rx_planescreen_close(&_PlaneScreenConfig[i]);
	}
}

//--- scr_malloc ---------------------------------------------
void scr_malloc(UINT32 lengthPx, UINT8 bitsPerPixel)
{
	if (bitsPerPixel==BITS_PER_PIXEL_SEP)
	{
		int widthPx = RX_Spooler.headWidthPx+RX_Spooler.headOverlapPx;
	
		UINT64 sizeSep=(int)data_memsize(PM_SCANNING, widthPx, lengthPx, BITS_PER_PIXEL_SEP);
		UINT64 sizeScr=(int)data_memsize(PM_SCANNING, widthPx, lengthPx, BITS_PER_PIXEL_SCR);
		int c, n;
		int b, h;
		for (c=0; c<RX_Spooler.colorCnt; c++)
		{
			if (RX_Color[c].spoolerNo==RX_SpoolerNo)
			{
				for (n=0; n<RX_Spooler.headsPerColor; n++)
				{
					h = RX_Spooler.headNo[c][n]-1;
					b = h/MAX_HEADS_BOARD;
					h = h%MAX_HEADS_BOARD;
					if (sizeScr>_ScrMem[b][h].sizeScr)
					{
						if (_ScrMem[b][h].separated) rx_mem_free(&_ScrMem[b][h].separated);
						_ScrMem[b][h].separated = rx_mem_alloc((size_t)sizeSep);
						if (_ScrMem[b][h].separated==NULL)		
						{
							Error(ERR_ABORT, 0, "Head[%d][%d] Could not allocate separated memory", b, h); 
							_ScrMem[b][h].sizeSep=0;
						}
						else _ScrMem[b][h].sizeSep=sizeSep;
						
						for (int i=0; i<SCR_BUF_SIZE; i++)
						{
							if (_ScrMem[b][h].screened[i])  rx_mem_free(&_ScrMem[b][h].screened[i]);
							_ScrMem[b][h].screened[i]  = rx_mem_alloc((size_t)sizeScr);
							if (_ScrMem[b][h].screened[i]==NULL)  
							{
								Error(ERR_ABORT, 0, "Head[%d][%d] Could not allocate screened memory", b, h);
								_ScrMem[b][h].sizeScr=0;
							}
							else _ScrMem[b][h].sizeScr = sizeScr;
						//	TrPrintfL(TRUE, "Head[%d.%d].screened[%d]=%p", b, h, i, &_ScrMem[b][h].screened[i]);
						}
						_ScrMem[b][h].screenedIdx=SCR_BUF_SIZE-1;
					}
				}
			}
		}
	}
}

//--- scr_abort ------------------------
void scr_abort(void)
{
	rx_sem_post(_SemScreeningReady);
}

//--- scr_set_values -----------------------------------------------------------------
void scr_set_values(int headNo, int min, int max, INT16 values[MAX_DENSITY_VALUES])
{
	int board, head, jet, i, ok;
	double sector = (double)MAX_DENSITY_FACTORS/(MAX_DENSITY_VALUES-1);
	double val[MAX_DENSITY_VALUES];

	if (RX_Spooler.headsPerColor==0) return;

	board	= headNo/RX_Spooler.headsPerBoard;
	head	= headNo%RX_Spooler.headsPerBoard;

//	if (TRUE || values[0]) 
	if (FALSE)
	{
		char str[128];
		int len = sprintf(str, "scr_set_values[%d.%d]: ", board, head);
		for (int i=0; i<MAX_DENSITY_VALUES; i++) len += sprintf(&str[len], "%d ", values[i]);
		Error(LOG, 0, str);
		TrPrintfL(TRUE, str);
	}

	ok=TRUE;
	for (i=0; i<MAX_DENSITY_VALUES; i++)
	{
		int v;
		v = values[i];
		if (v==0)		 v=max;
		else if (v<min)	{v=min; ok=FALSE;}
		else if (v>max) {v=max; ok=FALSE;}
			
		val[i] = (double)(v - min) / (double)(max - min);
	}
	
	if (!ok) Error(WARN, 0, "Head[%d.%d]: Density Value out of range!", board, head);

	for (jet=0; jet<MAX_DENSITY_FACTORS; jet++)
	{
		double d=(double)jet/sector;
		i=(int)d;
		_ScrMem[board][head].fact[jet] = (val[i]+(d-i)*(val[i+1]-val[i]));
	}
	
	//--- special in stitch region ----------------------------------------
	if (FALSE)
	{
		double corr[4]={0.010, 0.0075, 0.0050, 0.0025};
		for (jet=0;             jet<128;      jet++)      _ScrMem[board][head].fact[jet] += corr[jet/32];
		for (jet=128,  i=128;   jet<256;      jet++, i--) _ScrMem[board][head].fact[jet] += corr[3]*i/128.0;
		for (jet=2048;          jet<2048+128; jet++)	  _ScrMem[board][head].fact[jet] += corr[3-(jet-2048)/32];
		for (jet=2048, i=128;   jet>2048-128; jet--, i--) _ScrMem[board][head].fact[jet] += corr[3]*i/128.0;
	}

	//--- mark stitch ---------------------------------------
	if (FALSE)
	{
		_ScrMem[board][head].fact[0]	= 10;
		_ScrMem[board][head].fact[128]  = 10;
	}

	//--- disabled jets -----------------------------------------------------
	if (TRUE)
	{
		int jet;
		for (int i=0; i<MAX_DISABLED_JETS; i++)
		{
			if (jet=RX_DisabledJets[headNo][i])
			{
				double d=_ScrMem[board][head].fact[jet];
				if (jet>0)						_ScrMem[board][head].fact[jet-1] += d/2.0;
												_ScrMem[board][head].fact[jet]    = 0.0;
				if (jet+1<MAX_DENSITY_FACTORS)	_ScrMem[board][head].fact[jet+1] += d/2.0;
			}
		}
	}
}

//--- _write_tif --------------------------------------------------------------
static void _write_tif(char *title, SBmpSplitInfo *pInfo, PBYTE *buffer, int lineLen)
{
#ifdef DEBUG
	char dir[MAX_PATH];
	char fname[MAX_PATH];
	int headsPerColor=RX_Spooler.headsPerColor;
	if (headsPerColor==0) headsPerColor=1;
	sprintf(dir, PATH_RIPPED_DATA "trace/%s", RX_ColorNameShort(pInfo->inkSupplyNo));
	sprintf(fname, "%s(id=%d, p=%d, c=%d, s=%d, h=%d)", title, pInfo->pListItem->id.id, pInfo->pListItem->id.page, pInfo->pListItem->id.copy, pInfo->pListItem->id.scan, pInfo->head%headsPerColor);
	SBmpInfo info;
	memset(&info, 0, sizeof(info));
	info.planes		    = 1;
	info.srcWidthPx     = pInfo->widthPx;
	info.lengthPx	    = pInfo->srcLineCnt;	
	info.lineLen	    = lineLen;
	info.bitsPerPixel   = pInfo->bitsPerPixel;
	info.resol.x	    = DPI_X;
	info.resol.y	    = DPI_Y;
	info.colorCode[0]   = pInfo->colorCode;
	info.inkSupplyNo[0] = 0;
	info.buffer[0]      = buffer;

	tif_write(dir, fname, &info, NULL);
	Error(LOG, 0, "Written File >>%s<<", fname);
#endif
}

//--- _rx_screen_write_ta -----------------------------------
static int _rx_screen_write_ta(void * epplaneScreenConfig)
{
	SSLiceInfo outplane;
	SPlaneScreenConfig* pplaneScreenConfig = epplaneScreenConfig;

	memset(&outplane, 0, sizeof(outplane));
	outplane.widthPx		= pplaneScreenConfig->TA->width;
	outplane.lengthPx		= pplaneScreenConfig->TA->heigth;
	outplane.bitsPerPixel	= 16;
	outplane.aligment		= 8;
	outplane.lineLen		= ((outplane.widthPx * outplane.bitsPerPixel) + outplane.aligment - 1) / outplane.aligment;
	outplane.planeNumber	= 0;
	outplane.Xoffset		= 0;

	outplane.dataSize = (UINT64)outplane.lineLen*outplane.lengthPx;
	outplane.buffer = rx_mem_alloc(outplane.dataSize);
	if (outplane.buffer == NULL) return Error(ERR_CONT, 0, "No buffer for screening: %d");

	memcpy(outplane.buffer, pplaneScreenConfig->TA->ta16, outplane.dataSize);

	char dir[MAX_PATH];
	char fname[MAX_PATH];
	sprintf(dir, PATH_RIPPED_DATA "trace/");
	sprintf(fname, "TA");
	SBmpInfo info;
	memset(&info, 0, sizeof(info));
	info.planes			= 1;
	info.srcWidthPx		= outplane.widthPx;
	info.lengthPx		= outplane.lengthPx;	
	info.lineLen		= outplane.lineLen;
	info.bitsPerPixel	= outplane.bitsPerPixel;
	info.resol.x		= DPI_X;
	info.resol.y		= DPI_Y;
	info.colorCode[0]	= 0;
	info.inkSupplyNo[0] = 0;
	info.buffer[0]      = &outplane.buffer;

	tif_write(dir, fname, &info, NULL);
	Error(LOG, 0, "Written File >>%s<<", fname);

	rx_mem_free(&outplane.buffer);
	return REPLY_OK;
}

//--- scr_wait_ready ---------------------------------------------
int scr_wait_ready(void)
{	
	return rx_sem_wait(_SemScreeningReady, 500);
}

//--- scr_start --------------------------------------------------
void scr_start(SBmpSplitInfo *pInfo)
{
	int idx = (_ScrFifoInIdx+1)%SCR_FIFO_SIZE;
	if (idx==_ScrFifoOutIdx) 
	{
		Error(ERR_ABORT, 0, "ScrFifo Overvlow");
		return;
	}
	
	memcpy(&_Id, &pInfo->pListItem->id, sizeof(_Id));
    TrPrintfL(TRUE, "Head[%d.%d]: screening START, (id=%d, page=%d, copy=%d, scan=%d)",pInfo->board, pInfo->head, _Id.id, _Id.page, _Id.copy, _Id.scan);
	//--- calculating blkCnt -------------------
	{
		int dstLineLen;
		int bitsPerPixel=2;
		int lengthPx		= pInfo->srcLineCnt*DPI_X/pInfo->resol.x;
		dstLineLen			= (pInfo->widthPx*bitsPerPixel+7)/8;
		pInfo->dstLineLen   = (dstLineLen+31)&~31; // align to 256 bit
		pInfo->blkCnt		= (pInfo->dstLineLen * lengthPx + RX_Spooler.dataBlkSize-1) / RX_Spooler.dataBlkSize;
	//	if (_Id.copy==4 && _Id.scan==36 && pInfo->board==3 && pInfo->head==1)
	//		TrPrintfL(TRUE, "BLK1: srcLineCnt=%d, lengthPx=%d, widthPx=%d, dstLineLen=%d, blkCnt=%d\n",  pInfo->srcLineCnt, lengthPx, pInfo->widthPx, dstLineLen, pInfo->blkCnt);
	}

	if (_TimeStart==0)
	{
		_TimeStart=rx_get_ticks();
        TrPrintfL(TRUE, "Screening START, idle=%d ms", _TimeStart-_TimeEnd);
		_TimeEnd=0;
	}
	rx_mutex_lock(_ScrFifoMutex);
	{
		_ScrFifo[_ScrFifoInIdx] = pInfo;
		_ScrFifoInIdx = idx;
	}
	rx_mutex_unlock(_ScrFifoMutex);
	
	rx_sem_post(_SemScreeningStart);
    TrPrintfL(TRUE, "Head[%d.%d]: screening STARTED, (id=%d, page=%d, copy=%d, scan=%d)",pInfo->board, pInfo->head, _Id.id, _Id.page, _Id.copy, _Id.scan);
}

//--- scr_wait -----------------------------------------------------
int scr_wait(int timeout)
{
	if (_ScrFifoDoneIdx!=_ScrFifoInIdx)
	{
		// pause the printing while waiting to ensure there is always image info 
		if (rx_def_is_tx(RX_Spooler.printerType)) ctrl_pause_printing();
		TrPrintfL(TRUE, "Screening WAIT");
		while (_ScrFifoDoneIdx!=_ScrFifoInIdx)
		{
			rx_sem_wait(_SemScreeningDone, timeout);
		}
		if (rx_def_is_tx(RX_Spooler.printerType)) ctrl_start_printing();
	}
	_TimeEnd = rx_get_ticks();
	TrPrintfL(TRUE, "Screening WAIT END time=%d ms (%d threads)", _TimeEnd-_TimeStart, _ScrThreadCnt);
	//	Error(LOG, 0, "Screening (id=%d, page=%d, copy=%d, scan=%d) time=%d ms (%d threads, GPU=%d)", _Id.id, _Id.page, _Id.copy, _Id.scan, _TimeEnd-_TimeStart, _ScrThreadCnt, gpu_is_board_present());
	_TimeStart=0;
	rx_sem_post(_SemScreeningReady);
	return REPLY_OK;
}

//--- _screening_thread ------------------------------------------------
static void *_screening_thread(void* lpParameter)
{
	int threadNo = (int)(size_t)lpParameter;
	SBmpSplitInfo *pInfo;
//	TrPrintfL(TRUE, "_screening_thread[%d] started", threadNo);
	while (TRUE)
	{
	//	TrPrintfL(TRUE, "_screening_thread[%d] WAIT", threadNo);
		rx_sem_wait(_SemScreeningStart, 0);

		if (!_ScrThreadRunning) return NULL;
	
		TrPrintfL(TRUE, "_screening_thread[%d] START", threadNo);
		rx_mutex_lock(_ScrFifoMutex);
		{
			if (_ScrFifoInIdx==_ScrFifoOutIdx) Error(ERR_ABORT, 0, "ScrFifo Empty");
			pInfo = _ScrFifo[_ScrFifoOutIdx];
			_ScrFifoOutIdx = (_ScrFifoOutIdx+1)%SCR_FIFO_SIZE;
		}
		rx_mutex_unlock(_ScrFifoMutex);

		_scr_load(pInfo, threadNo);

		rx_mutex_lock(_ScrFifoMutex);
		{
			_ScrFifoDoneIdx = (_ScrFifoDoneIdx+1)%SCR_FIFO_SIZE;
		}
		rx_mutex_unlock(_ScrFifoMutex);

		TrPrintfL(TRUE, "_screening_thread[%d] END", threadNo);
		rx_sem_post(_SemScreeningDone);
	}
	return NULL;
}

//--- scr_load -----------------------------------------------------------
static void _scr_load(SBmpSplitInfo *pInfo, int threadNo)
{
	int b=pInfo->board;
	int h=pInfo->head;
	int no=pInfo->board*MAX_HEADS_BOARD +pInfo->head;
	int time=rx_get_ticks();

//	TrPrintfL(TRUE, "_scr_load[%d] START", threadNo);
	#ifdef linux
//	Error(LOG, 0, "scr_load: head=%d, threadId=%d", no, gettid());
	#endif

	int trace=FALSE;
	
	if (FALSE)
	{
		SPageId *pID=&pInfo->pListItem->id;
		TrPrintfL(TRUE, "_scr_load (id=%d, page=%d, copy=%d, scan=%d) to mem[%d]", pID->id, pID->page, pID->copy, pID->scan, _ScrMem[b][h].screenedIdx);
	}

	//--- screening ---------------------------------------------------------
	{
		char settingsPath[MAX_PATH];
		SSLiceInfo linplane, loutplane;
		int ret;

		if (strcmp(pInfo->pListItem->filepath, _PlaneFilePath[no]))
		{	
			if (_PlaneScreenConfig[no].fctclose) rx_planescreen_close(&_PlaneScreenConfig[no]);
			if (pInfo->printMode==PM_TEST) sprintf(settingsPath,	"%s/prEnvSettings.xml", PATH_BIN_SPOOLER);
			else						   sprintf(settingsPath,	"%s/prEnvSettings.xml", pInfo->pListItem->filepath);
			ret = rx_planescreen_init(pInfo->inkSupplyNo, settingsPath, PATH_BIN_SPOOLER, _ScrMem[b][h].fact, &_PlaneScreenConfig[no]);
			strcpy(_PlaneFilePath[no], pInfo->pListItem->filepath);
		}

		if (rx_def_is_scanning(RX_Spooler.printerType) || !pInfo->same)
			_ScrMem[b][h].screenedIdx = (_ScrMem[b][h].screenedIdx+1) % SCR_BUF_SIZE;

		linplane.planeNumber	= pInfo->inkSupplyNo;	// plane number
		linplane.Xoffset		= pInfo->pListItem->offsetWidth; // X offset 
		linplane.widthPx		= pInfo->widthPx*pInfo->resol.x/DPI_X;
		linplane.lengthPx		= pInfo->srcLineCnt;
		linplane.bitsPerPixel	= pInfo->bitsPerPixel;
		{
			int blkSize=gpu_blk_size();
			int widthBt				= pInfo->widthBt*pInfo->resol.x/DPI_X;
			linplane.lineLen        = ((widthBt+blkSize-1)/blkSize)*blkSize; // must be multiple of blkSize
		}
		linplane.aligment		= 0; // lbmp.aligment;			// 8, 16, 32 ,....
		memcpy(&linplane.resol, &pInfo->pListItem->splitInfo->resol, sizeof(linplane.resol));
        linplane.resol.x		= (pInfo->resol.x)? pInfo->resol.x : DPI_X;
        linplane.resol.y		= (pInfo->resol.y)? pInfo->resol.y : DPI_Y;
		linplane.dataSize		= linplane.lengthPx*linplane.lineLen;
		linplane.buffer			= _ScrMem[b][h].separated;

		memset(&loutplane, 0, sizeof(loutplane));		
		loutplane.resol.x		= DPI_X;
		loutplane.resol.y		= DPI_Y;
		loutplane.widthPx		= linplane.widthPx*loutplane.resol.x/linplane.resol.x;
		loutplane.lengthPx		= linplane.lengthPx*loutplane.resol.y/linplane.resol.y;
		loutplane.bitsPerPixel	= 2; //pplaneScreenConfig->outputbitsPerPixel;
		loutplane.aligment		= 8;

	//	loutplane.lineLen		= pInfo->dstLineLen*loutplane.bitsPerPixel/linplane.bitsPerPixel;
		loutplane.lineLen		= ((loutplane.widthPx * loutplane.bitsPerPixel) + loutplane.aligment - 1) / loutplane.aligment;
		loutplane.planeNumber	= linplane.planeNumber;
		loutplane.Xoffset		= linplane.Xoffset;

		loutplane.buffer		= _ScrMem[b][h].screened[_ScrMem[b][h].screenedIdx];
		loutplane.dataSize		= _ScrMem[b][h].sizeScr;
		time=rx_get_ticks();
		
		rx_mem_use(loutplane.buffer);

		if (rx_def_is_scanning(RX_Spooler.printerType) || !pInfo->same)
		{
			time=rx_get_ticks();
			_scr_fill_blk(pInfo, linplane.lineLen, _ScrMem[b][h].separated);
			if (trace) Error(LOG, 0, "scr_load time2=%d", rx_get_ticks()-time);
			if (FALSE && pInfo->board==2 && pInfo->head==1)
			{
				SBmpSplitInfo info;
				memcpy(&info, pInfo, sizeof(info));
				info.widthPx = linplane.widthPx;
				_write_tif("org", &info, &_ScrMem[b][h].separated, linplane.lineLen);
			}

		//	ret=_rx_screen_write_ta(&_PlaneScreenConfig[no]);
		//	ret = rx_screen_slice_FMS_1x3g(&linplane, &loutplane, &_PlaneScreenConfig[no]);
		//	ret = rx_screen_slice_FMS_1x3g_mag(&linplane, &loutplane, &_PlaneScreenConfig[no]);
			ret = gpu_screen_FMS_1x3g(&linplane, &loutplane, &_PlaneScreenConfig[no], pInfo->pListItem->dots, threadNo);

			_TimeTotal += rx_get_ticks()-time;
			/*
			if (gpu_is_board_present())
				Error(LOG, 0, "scr_load screening , sizeIn=%dMB, sizeOut=%dMB, time=%d, GPU=%d  %d  %d", linplane.dataSize/1024/1024, loutplane.dataSize/1024/1024, rx_get_ticks()-time, gpu_time(0), gpu_time(1), gpu_time(2),_TimeTotal);
			else
				Error(LOG, 0, "scr_load screening , sizeIn=%dMB, sizeOut=%dMB, time=%d", linplane.dataSize/1024/1024, loutplane.dataSize/1024/1024, rx_get_ticks()-time);
			*/
		}
		else ret=REPLY_OK;
		rx_mem_unuse(pInfo->allocated_buff);

		if (ret==REPLY_OK)
		{
			//--- change from 8 bitsPerPixel to 4 bitsPerPixel ----------------------------------
			int oldbppx = pInfo->bitsPerPixel;
			int blk0   = pInfo->blk0;
			int blkCnt = pInfo->blkCnt;
			pInfo->bitsPerPixel = 2;
			pInfo->data			= &_ScrMem[b][h].screened[_ScrMem[b][h].screenedIdx];
		//	TrPrintfL(TRUE, "Head[%d.%d].data=%p", b, h, pInfo->data);
			pInfo->srcLineCnt	= pInfo->srcLineCnt*loutplane.resol.y/linplane.resol.y;
			pInfo->srcLineLen	= loutplane.lineLen;
			pInfo->srcWidthBt	= (pInfo->widthPx*pInfo->bitsPerPixel+7)/8;
			pInfo->dstLineLen	= (pInfo->srcWidthBt+31)&~31;	// align to 256 bit
			pInfo->fillBt		= 0;
			pInfo->startBt		= 0;
			pInfo->widthBt		= pInfo->srcWidthBt;
			pInfo->blkCnt		= (pInfo->dstLineLen * pInfo->srcLineCnt + RX_Spooler.dataBlkSize-1) / RX_Spooler.dataBlkSize;

			if (pInfo->blkCnt!=blkCnt)
			{				
		//	if (_Id.copy==4 && _Id.scan==36 && pInfo->board==3 && pInfo->head==1)
		//		TrPrintfL(TRUE, "BLK2: srcLineCnt=%d, lengthPx=%d, widthPx=%d, dstLineLen=%d, blkCnt=%d\n",  pInfo->srcLineCnt, pInfo->srcLineCnt, pInfo->widthPx, pInfo->dstLineLen, pInfo->blkCnt);
				TrPrintfL(TRUE, "Screening[%d][%d] ERROR: blk0=%d, blkCnt=%d, blk0=%d, blkCnt=%d", b, h, blk0, blkCnt, pInfo->blk0, pInfo->blkCnt);
				TrPrintfL(TRUE, "SrcBlkCnt[%d][%d]: dstLineLen=%d, lengthPx=%d, blkCnt=%d", pInfo->board, pInfo->head, pInfo->dstLineLen, pInfo->srcLineCnt, pInfo->blkCnt);
				Error(ERR_CONT, 0,"BlkCnt Mismatch, (id=%d, page=%d, copy=%d, scan=%d) time=%d ms (board=%d, head=%d)", _Id.id, _Id.page, _Id.copy, _Id.scan, pInfo->board, pInfo->head);
			}

		//	_ScrMem[b][h].screenedIdx = (_ScrMem[b][h].screenedIdx+1) % SCR_BUF_SIZE;
		//	Error(LOG, 0, "Head[%d.%d]:widthPx=%d, bitsPerPixel=%d, srcWidthBt=%d, widthBt=%d, srcLineLen=%d, dstLineLen=%d, blkCnt=%d", b, h, pInfo->widthPx, pInfo->bitsPerPixel, pInfo->srcWidthBt, pInfo->widthBt, pInfo->srcLineLen, pInfo->dstLineLen, pInfo->blkCnt);
			
			TrPrintfL(TRUE, "Screening[%d][%d]: blk0=%d, blkCnt=%d, blk0=%d, blkCnt=%d", b, h, blk0, blkCnt, pInfo->blk0, pInfo->blkCnt);

			if (FALSE && pInfo->board==2 && pInfo->head==1)
				_write_tif("screened", pInfo, pInfo->data, pInfo->srcLineLen);
		}		
	}
//	TrPrintfL(TRUE, "_scr_load[%d] END", threadNo);
}

//--- _scr_fill_blk ---------------------------------------------
static void _scr_fill_blk(SBmpSplitInfo *psplit, int dstLineLen, BYTE *dst)
{
//	int		mirror=FALSE; // psplit->pListItem->flags&FLAG_MIRROR; // image is mirrored at sending
	int		len, l, width;
	int		line	   = 0;
	int		start	   = 0;
	int		startBt    = psplit->startBt;
	int		srcWidthBt = psplit->srcWidthBt;
	int		fillLen    = psplit->fillBt;
	int		endLine    = psplit->srcLineCnt;

	BYTE**	ptr     = psplit->data;
	BYTE	*src    = *ptr;
	BYTE	*s;

	if (src == NULL) // Abort
		return;

	if (psplit->resol.x!=1200)
	{
		startBt    = startBt*psplit->resol.x/1200;
		fillLen    = fillLen*psplit->resol.x/1200;
	}

	//	printf("head=%d, blkNo=%d, dstLen=%d\n", psplit->head, blkNo, dstLen);
	if (fillLen>dstLineLen) 
		fillLen=dstLineLen;

	/*
	if (RX_Color[psplit->inkSupplyNo].rectoVerso==rv_verso) mirror = !mirror;
	if (mirror)
	{
		line	 = psplit->srcLineCnt-line-1;
		endLine  = -1;
		src += (UINT64)line*psplit->srcLineLen; // +psplit->startBt%psplit->srcLineLen;
	}
	*/

	while (line!=endLine)
	{
		//--- each line ---------------------
		if (fillLen)
		{
			memset(dst, 0x00, fillLen);
			dst += fillLen;
		}
		start = startBt % srcWidthBt;
		len   = dstLineLen-fillLen;
		s     = src+start;
		width = srcWidthBt-start;

		//--- copy the image data ----------------------------
        // and repeat it if needed (for textile image multiple copy)
        while (len)
        {
            l = (len > width) ? width : len;
            memcpy(dst, s, l);
            len -= l;
            dst += l;
            // we need to repeat the image on TX (next copy) BUT not on LB
			if (len && rx_def_is_lb(RX_Spooler.printerType)) 
			{
                // so if we are not at the end of the bitmap we put 0 
				memset(dst, 0x00, len);
                dst += len;
				break;
            }
            s = src;
            width = srcWidthBt;
        }

		if (psplit->srcLineLen == 0) return; // Abort
		src	+= psplit->srcLineLen;
		line++;
	}
}