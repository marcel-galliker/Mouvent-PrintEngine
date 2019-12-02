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

typedef struct
{
	INT32		height;
	BYTE	   *buffer;
	INT64		fileSize;
	void	   *loaded_arg;
} SDecompressPar;

static int				_ThreadCnt=0;
static SFlzThreadPar*	_ThreadPar;
static int				_ThreadRunning=TRUE;

static SDecompressPar	_DecompressPar;
static	HANDLE			_sem_decompress_start;
static 	HANDLE			_sem_decompress_done;
static int				_Init=FALSE;
static int				_Abort=FALSE;

static int				_FileBufLoadIdx=0;
static int				_FileBufDecompIdx=0;
static INT64			_FileBufSize[2]={0,0};
static BYTE*			_FileBuf[2]={NULL, NULL};
static char				_LastFilePath[MAX_PATH]="";

//--- prototypes --------------------------------
static void *_flz_decompress_master_thread(void* lpParameter);
static void *_flz_decompress_thread(void* lpParametr);

//--- _tif_init -------------------------------
void flz_init(void)
{
	_Abort = TRUE;

	if (!_Init)
	{
		_Init = TRUE;
	
		int i;
		_ThreadCnt = rx_core_cnt();
		_ThreadPar = rx_malloc(_ThreadCnt*sizeof(SFlzThreadPar));
		_sem_decompress_start = rx_sem_create();
		rx_thread_start(_flz_decompress_master_thread, NULL, 0, "_flz_decompress_master_thread");
		for (i=0; i<_ThreadCnt; i++)
		{
			_ThreadPar[i].no = i;
			_ThreadPar[i].sem_start = rx_sem_create();
			_ThreadPar[i].sem_done  = rx_sem_create();
			rx_thread_start(_flz_decompress_thread, &_ThreadPar[i], 0, "_flz_decompress_thread");
		}		
	}

	//--- start of new job ----------------------------
	if (_sem_decompress_done) rx_sem_destroy(&_sem_decompress_done);
	_sem_decompress_done  = rx_sem_create();
//	rx_sem_post(_sem_decompress_done);
	_FileBufLoadIdx=0;
	_FileBufDecompIdx=0;
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
int flz_load(SPageId *id, const char *filedir, const char *filename, int printMode, UINT32 spacePx, SColorSplitCfg *psplit, int splitCnt, BYTE* buffer[MAX_COLORS], SBmpInfo *pinfo, progress_fct progress, void* loaded_arg)
{
	int c;
	int time;
	SFlzInfo *pFlzInfo;
	INT64 fileSize;
	char filepath[MAX_PATH];
	BYTE* dst;
		
	_Abort = FALSE;
	memset(pinfo, 0, sizeof(SBmpInfo));
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
						
				time = rx_get_ticks();
				{
					fileSize=rx_file_get_size(filepath);
					if (fileSize>_FileBufSize[_FileBufLoadIdx])
					{
						if (_FileBuf[_FileBufLoadIdx]!=NULL) rx_mem_free(&_FileBuf[_FileBufLoadIdx]);
						_FileBufSize[_FileBufLoadIdx] = (2*fileSize/ 0x100000) * 0x100000; // fill to next MB, min=10
						if (_FileBufSize[_FileBufLoadIdx]<10*0x100000) _FileBufSize[_FileBufLoadIdx] = 10*0x100000;
						_FileBuf[_FileBufLoadIdx] = rx_mem_alloc(_FileBufSize[_FileBufLoadIdx]);
						Error(LOG, 0, "ALLOC MEMORY [%d] %d MB", _FileBufLoadIdx, _FileBufSize[_FileBufLoadIdx]/0x100000);
					}
					
					INT64 len;
					INT64 blksize=0x10000;
					FILE *file = fopen(filepath, "rb");			
					if (file==NULL) return REPLY_NOT_FOUND;
					for (dst=_FileBuf[_FileBufLoadIdx], len=fileSize; len>=blksize && !_Abort; len-=blksize)
					{
						dst += fread(dst, 1, (int)blksize, file);
					}
					dst+=fread(dst, (int)len, 1, file);					
					fclose(file);
					strcpy(_LastFilePath, filepath); 

					_FileBufLoadIdx = (_FileBufLoadIdx+1) & 1;
				}				
				TrPrintfL(TRUE, "LOADING >>%s<<, page %d, time=%d ms", filepath, id->page, rx_get_ticks()-time);
			}
			else 
			{
				Error(ERR_ABORT, 0, "Streaming not implemented yet");
			}
			
			if (_Abort) return REPLY_OK;

			//--- wait last decompressing finished ---------------------------------

//			rx_sem_wait(_sem_decompress_done, 0);
			
			//--- start decompressing ----------------------------------------------
			
			pFlzInfo = (SFlzInfo*)_FileBuf[_FileBufDecompIdx];
			
			pinfo->printMode     = printMode;
			pinfo->bitsPerPixel  = pFlzInfo->bitsPerPixel;
			pinfo->srcWidthPx    = pFlzInfo->WidthPx+spacePx;
			pinfo->lengthPx		 = pFlzInfo->lengthPx;
			pinfo->lineLen		 = pFlzInfo->lineLen;
			pinfo->dataSize		 = (INT32)pFlzInfo->dataSize;								
			pinfo->buffer[c]	 = &buffer[c];

			_DecompressPar.loaded_arg = loaded_arg;
			_DecompressPar.fileSize = fileSize;
			_DecompressPar.buffer = buffer[c];
			_DecompressPar.height = pinfo->lengthPx;
			if (psplit[c].lastLine<_DecompressPar.height) _DecompressPar.height=psplit[c].lastLine;			
									
			rx_sem_post(_sem_decompress_start);
			rx_sem_wait(_sem_decompress_done, 0);
		}
	}
	return REPLY_OK;
}

//--- flz_last_filepath --------------------------------------
char* flz_last_filepath(void)
{
	return _LastFilePath;
}

//--- _flz_decompress_master_thread ------------------------------------
static void *_flz_decompress_master_thread(void* lpParameter)
{
	int time;
	int	i;
	int height, y, h;
	SFlzInfo	  *pFlzInfo;
	
	while (_ThreadRunning)
	{
		rx_sem_wait(_sem_decompress_start, 0);
		time = rx_get_ticks();

		pFlzInfo = (SFlzInfo*)_FileBuf[_FileBufDecompIdx];

		height = _DecompressPar.height;
			
		y = 0;
		h = pFlzInfo->bands/_ThreadCnt;
		for (i=0; i<_ThreadCnt; i++)
		{
			_ThreadPar[i].no	   = i;	
			_ThreadPar[i].fileBuf  = _FileBuf[_FileBufDecompIdx];
			_ThreadPar[i].fileSize = _DecompressPar.fileSize;
			_ThreadPar[i].buffer   = _DecompressPar.buffer;
			_ThreadPar[i].y_from   = y;
			_ThreadPar[i].y_to	   = y+h;
			y += h;
		}
		_ThreadPar[_ThreadCnt-1].y_to = pFlzInfo->bands;

		for (i=0; i<_ThreadCnt; i++) rx_sem_post(_ThreadPar[i].sem_start);

		while (rx_sem_wait(_ThreadPar[0].sem_done, 500)!=REPLY_OK)
		{
		//	if (progress!=NULL && h!=0) progress(id, RX_ColorNameShort(pinfo->inkSupplyNo[c]), 101*_ThreadPar[0].y / h);					
		}
	//	if (progress!=NULL) progress(id, RX_ColorNameShort(pinfo->inkSupplyNo[c]), 100);					

		for (i=1; i<_ThreadCnt; i++) rx_sem_wait(_ThreadPar[i].sem_done, 0);

		_FileBufDecompIdx = (_FileBufDecompIdx+1) & 1;
		
		TrPrintfL(TRUE, "DECOMPRESSING time=%d ms", rx_get_ticks()-time);
		if (flz_loaded!=NULL) flz_loaded(_DecompressPar.loaded_arg);
		rx_sem_post(_sem_decompress_done);
	}
}

//--- _flz_decompress_thread ------------------------------------
static void *_flz_decompress_thread(void* lpParameter)
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
