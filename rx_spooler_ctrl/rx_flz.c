// ****************************************************************************
//
//	rx_flz.c
//
// ****************************************************************************
//
//	Copyright 20158 by Mouvent AG, Switzerland. All rights reserved.
//	Written by Gilbert Therville 
//
// ****************************************************************************

//--- includes ---------------------------------------------------------------------

#include "rx_file.h"
#include "rx_error.h"
#include "rx_trace.h"
#include "rx_mem.h"
#include "rx_threads.h"

#include "fastlz.h"
#include "rx_flz.h"

#ifdef linux
	#include "errno.h"
#endif

//--- defines -------------------------------------------------------------------------
#define fLZ_FILE_EXT	L"flz"
#define MAX4GB 4294967296

//--- SFlzSliceThread-------------------------------------------------------------------------
typedef struct
{
	int			no;
	int			result;
	BYTE	   *fileBuf;
	INT64		fileSize;
	BYTE	   *buffer;
	int			y_from;
	int			y_to;
	int			y;
	HANDLE		sem_start;
	HANDLE		sem_done;
} SFlzThreadPar;

static int				_ThreadCnt=0;
static SFlzThreadPar*	_ThreadPar;
static int				_ThreadRunning=TRUE;
static int				_Init=FALSE;
static int				_Abort=FALSE;

static INT64			_FileBufSize=0;
static BYTE*			_FileBuf=NULL;

//--- prototypes --------------------------------
static void _flz_init(void);
static void *_flz_read_thread(void* lpParameter);

//--- _tif_init -------------------------------
static void _flz_init(void)
{
	if (_Init) return;
	_Init = TRUE;
	
	int i;
	_ThreadCnt = rx_core_cnt();
	_ThreadPar = rx_malloc(_ThreadCnt*sizeof(SFlzThreadPar));
	for (i=0; i<_ThreadCnt; i++)
	{
		_ThreadPar[i].no = i;
		_ThreadPar[i].sem_start = rx_sem_create();
		_ThreadPar[i].sem_done  = rx_sem_create();
		rx_thread_start(_flz_read_thread, &_ThreadPar[i], 0, "tif_read_thread");
	}
}


//--- _flz_color_path ---------------------------------
static void _flz_color_path(const char *path, UINT32 page, const char *nameExt, const char *colorname, char *filepath)
{
	char name[128];
	if (_stricmp(&path[strlen(path)-4], ".flz"))
	{
		split_path(path, NULL, name, NULL);
		if (page > 1)	sprintf(filepath, "%s/%s%s_P%06d_%s.flz", path, name, nameExt, page, colorname);
		else			sprintf(filepath, "%s/%s%s_%s.flz", path, name, nameExt, colorname);
	}
	else strcpy(filepath, path);
}

//--- flz_get_info --------------------------------------------------------------------------------------
int flz_get_info(const char *path, UINT32 page, SFlzInfo *pflzinfo)
{
	int ret=REPLY_ERROR;
	int color;
	FILE* file;
	char filepath[MAX_PATH];

	// check and init
	if (!pflzinfo) return Error(ERR_CONT, 0, "flz_get_size parameter error", path);

	memset(pflzinfo, 0, sizeof(SFlzInfo));
	
	for (color = 0; color < MAX_COLORS; color++)
	{
		if (!RX_Color[color].lastLine) continue;

		_flz_color_path(path, page, "", RX_ColorNameShort(color), filepath);	
	
		file=fopen (filepath, "rb");
		if (file==NULL) return REPLY_NOT_FOUND;

		if(fread(pflzinfo,sizeof(SFlzInfo),1,file)==1) ret = REPLY_OK;
		else										   ret = Error(ERR_CONT, 0, "File %s: error reading flz info", path); 

		fclose(file);
		return ret;	
	}
	return REPLY_ERROR;
}


//--- flz_abort -----------------------------------------------
void flz_abort(void)
{
	_Abort = TRUE;
}

//--- flz_load (multi threaded) ------------------------------------------------------------------------------------------
int flz_load(SPageId *id, const char *filedir, const char *filename, int printMode, UINT32 spacePx, SColorSplitCfg *psplit, int splitCnt, BYTE* buffer[MAX_COLORS], SBmpInfo *pinfo, progress_fct progress)
{
	int c, i;
	int y, h;
	SFlzInfo *pFlzInfo;
	INT32 height;
	INT64 fileSize;
	char filepath[MAX_PATH];
	BYTE* dst;
		
	_Abort = FALSE;
	memset(pinfo, 0, sizeof(SBmpInfo));
	_flz_init();
	for (c=0; c<MAX_COLORS && c<splitCnt; c++)
	{
		if (psplit[c].color.name[0] && psplit[c].lastLine)
		{
			pinfo->colorCode[c]=psplit[c].color.colorCode;
			pinfo->inkSupplyNo[c] =psplit[c].inkSupplyNo;
			if (*filename)
			{
				if (id->page>1) sprintf(filepath, "%s/%s_P%06d_%s.flz", filedir, filename, id->page, RX_ColorNameShort(pinfo->inkSupplyNo[c]));
				else			sprintf(filepath, "%s/%s_%s.flz", filedir, filename, RX_ColorNameShort(pinfo->inkSupplyNo[c]));
								
				{
					fileSize=rx_file_get_size(filepath);
					if (fileSize>_FileBufSize)
					{
						if (_FileBuf!=NULL) rx_mem_free(&_FileBuf);
						_FileBufSize = (2*fileSize+0xfffff) & ~0xffff; // round up to next MB
						_FileBuf	 = rx_mem_alloc(_FileBufSize);
					}
					
					INT64 len;
					INT64 blksize=0x10000;
					FILE *file = fopen(filepath, "rb");			
					if (file==NULL) return REPLY_NOT_FOUND;
					for (dst=_FileBuf, len=fileSize; len>=blksize && !_Abort; len-=blksize)
					{
						dst += fread(dst, 1, (int)blksize, file);
					}
					dst+=fread(dst, (int)len, 1, file);					
					fclose(file);
				}
			}
			else 
			{
				Error(ERR_ABORT, 0, "Straeming not implemented yet");
			}
			
			if (_Abort) return REPLY_OK;
			
			pFlzInfo = (SFlzInfo*)_FileBuf;
			
			pinfo->printMode     = printMode;
			pinfo->bitsPerPixel  = pFlzInfo->bitsPerPixel;
			pinfo->srcWidthPx    = pFlzInfo->WidthPx+spacePx;
			pinfo->lengthPx		 = pFlzInfo->lengthPx;
			pinfo->lineLen		 = pFlzInfo->lineLen;
			pinfo->dataSize		 = (INT32)pFlzInfo->dataSize;								
			pinfo->buffer[c]	 = &buffer[c];
			height = pinfo->lengthPx;
			if (psplit[c].lastLine<height) height=psplit[c].lastLine;
			
//			y = psplit[c].firstLine;
//			h = height/_ThreadCnt;
			y = 0;
			h = pFlzInfo->bands/_ThreadCnt;
			for (i=0; i<_ThreadCnt; i++)
			{
				_ThreadPar[i].no	   = i;	
				_ThreadPar[i].fileBuf  = _FileBuf;
				_ThreadPar[i].fileSize = fileSize;
				_ThreadPar[i].buffer   = buffer[c];
				_ThreadPar[i].y_from   = y;
				_ThreadPar[i].y_to	   = y+h;
				y += h;
			}
			_ThreadPar[_ThreadCnt-1].y_to = pFlzInfo->bands;

			for (i=0; i<_ThreadCnt; i++) rx_sem_post(_ThreadPar[i].sem_start);

			while (rx_sem_wait(_ThreadPar[0].sem_done, 500)!=REPLY_OK)
			{
				if (progress!=NULL && h!=0) progress(id, RX_ColorNameShort(pinfo->inkSupplyNo[c]), 101*_ThreadPar[0].y / h);					
			}
			if (progress!=NULL) progress(id, RX_ColorNameShort(pinfo->inkSupplyNo[c]), 100);					

			for (i=1; i<_ThreadCnt; i++) rx_sem_wait(_ThreadPar[i].sem_done, 0);
		}
	}
	return REPLY_OK;
}

//--- _flz_read_thread ------------------------------------
static void *_flz_read_thread(void* lpParameter)
{
	SFlzThreadPar *par=(SFlzThreadPar*)lpParameter;
	while (_ThreadRunning)
	{
		rx_sem_wait(par->sem_start, 0);

		{			
			SFlzInfo *pFlzInfo;
			SFlzBand *bands, *band;
			int ret;

			BYTE *dst;
			BYTE *cmpbuf;
			
			par->result	= REPLY_ERROR;
			pFlzInfo	= (SFlzInfo*)par->fileBuf;
			bands		= (SFlzBand*)(par->fileBuf+sizeof(SFlzInfo));
										
			for(par->y=par->y_from; par->y<par->y_to && !_Abort; par->y++)
			{										
				band	= &bands[par->y];
				dst		= par->buffer + ((UINT64)band->start * pFlzInfo->lineLen);
				cmpbuf  = par->fileBuf + band->offset;
				ret		= fastlz_decompress(cmpbuf, (int)band->bandSize, dst, band->rows * pFlzInfo->lineLen);
			}					
			
			par->result	= REPLY_OK;
		}
		
		rx_sem_post(par->sem_done);
	}
	return NULL;
}
