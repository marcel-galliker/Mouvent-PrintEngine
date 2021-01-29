// ****************************************************************************
//
//	rx_tif.c
//
// ****************************************************************************
//
//	Copyright 2015 by Radex AG, Switzerland. All rights reserved.
//	Written by Marcel Galliker 
//
// ****************************************************************************

//--- includes ---------------------------------------------------------------------

#include "rx_error.h"
#include "rx_file.h"
#include "rx_threads.h"
#include "rx_trace.h"
#include "rx_mem.h"
#include "tiffio.h"
#include "tiffiop.h"
#include "rx_mem_tif.h"
#include "rx_tif.h"

#ifdef linux
	#include "errno.h"
#endif

#ifdef DEBUG
	#define MULTI_THREADED
#endif

//--- defines -------------------------------------------------------------------------
#define TIF_FILE_EXT	L"tif"

typedef struct
{
	int			no;
	int			result;
	TIFF	   *file;
	BYTE	   *fileBuf;
	INT64		fileSize;
	SBmpInfo   *pinfo;
	BYTE	   *buffer;
	int			y_from;
	int			y_to;
	int			gap;
	int			y;
	HANDLE		sem_start;
} STifThreadPar;

static STifThreadPar	_ThreadPar[MAX_COLORS];
static int				_ThreadRunning=TRUE;
static HANDLE			_SemDone;
static int				_Init=FALSE;

static INT64			_FileBufSize=0;
static BYTE*			_FileBuf=NULL;

static char				_LastFilePath[MAX_PATH]="";

static int _Abort;
static char _ErrorStr[256]={0};
static char _WarnStr[256] ={0};


//--- prototypes -----------------------------------------
static void _tif_ErrorHandler(const char* module, const char* fmt, va_list ap);
static void _tif_WarningHandler(const char* module, const char* fmt, va_list ap);
static TIFF* _tif_stream_open(int page, int bitmap);

static void _tif_init(void);
static void *_tif_read_thread(void* lpParameter);


tif_stream_open_fct tif_stream_open = _tif_stream_open;

//--- tif_error -------------------------------------------------
char *tif_error(void)
{
	return _ErrorStr;
}

//--- tif_warn --------------------------------------------------
char *tif_warn(void)
{
	return _WarnStr;
}

//--- tif_open_stream ------------------------------------------
TIFF* tif_open_stream (int page, int bitmap)
{
	return NULL;
}

//--- _tif_init -------------------------------
static void _tif_init(void)
{
	int i;
	for (i=0; i<SIZEOF(_ThreadPar); i++)
	{
		_ThreadPar[i].file	  = NULL;
		_ThreadPar[i].fileBuf = NULL;
	}
	
	if (_Init) return;
	_Init = TRUE;
	
	for (i=0; i<SIZEOF(_ThreadPar); i++)
	{
		_ThreadPar[i].no = i;
		_ThreadPar[i].sem_start = rx_sem_create();
//		_ThreadPar[i].sem_done  = rx_sem_create();
		rx_thread_start(_tif_read_thread, &_ThreadPar[i], 0, "tif_read_thread");
	}
}

//--- _tif_color_path ---------------------------------
static void _tif_color_path(const char *path, UINT32 page, const char *nameExt, const char *colorname, char *filepath)
{
	char name[128];
	if (_stricmp(&path[strlen(path)-4], ".tif"))
	{
		split_path(path, NULL, name, NULL);
		if (page > 1)	sprintf(filepath, "%s/%s%s_P%06d_%s.tif", path, name, nameExt, page, colorname);
		else			sprintf(filepath, "%s/%s%s_%s.tif", path, name, nameExt, colorname);
	}
	else strcpy(filepath, path);
}

//--- tif_get_size --------------------------------------------------------------------------------------
int tif_get_size(const char *path, UINT32 page, UINT32 spacePx, UINT32 *width, UINT32 *height, UINT8 *bitsPerPixel)
{
	TIFF* file;
	INT16 data;
	INT32 val;
	int color;
	char filepath[MAX_PATH];

	*_ErrorStr=0;
	*_WarnStr=0;
	_TIFFerrorHandler   = _tif_ErrorHandler;
	_TIFFwarningHandler = _tif_WarningHandler;

	if (width) *width = 0;
	if (height) *height = 0;
	if (bitsPerPixel) *bitsPerPixel = 0;

//	color_path(path, RX_ColorNameList[0].shortName, filepath, sizeof(filepath));
	for (color = 0; color < MAX_COLORS; color++)
	{
		if (!RX_Color[color].lastLine) continue;

		if (*path)
		{
			_tif_color_path(path, page, "", RX_ColorNameShort(color), filepath);	
			file=TIFFOpen (filepath, "r");
		}
		else
		{
			file=tif_stream_open(page, color);
		}
		if (file==NULL) continue;

		/*
		if (StartPageNo && file->tif_dirnumber>StartPageNo) return BMP_ERROR;
		while (file->tif_dirnumber<StartPageNo) 
		{
			if (TIFFReadDirectory(file)==0) return BMP_ERROR | BMP_EOF;
		}
		*/

		//--- read header ------------------------------------------
		{
			//--- check format: planes ----------
			if (!TIFFGetField (file, TIFFTAG_SAMPLESPERPIXEL, &data)) return Error(ERR_CONT, 0, "File %s: Encoded using %d planes. Only one plane supported",  data, filepath);

			//--- bit per pixel ---
			if (bitsPerPixel && (!TIFFGetField (file, TIFFTAG_BITSPERSAMPLE, &val))) return Error(ERR_CONT, 0, "File %s: Could not get bit per sample value", filepath);
			if (bitsPerPixel) *bitsPerPixel = (UCHAR)val;
	//		if (*bitsPerPixel<(UINT8)1 || *bitsPerPixel>(UINT8)2) return Error(ERR_CONT, 0, "File %s: %d bit per pixel encoded. Can only process 1, 2 bit per pixel", filepath, *bitsPerPixel);
			/*
			if (*bitsPerPixel ==1) 
			{
				TIFFGetField (file, TIFFTAG_PHOTOMETRIC, &data);
				if	(data==PHOTOMETRIC_MINISWHITE)		{filler=0x00; pBmpInfo->dotConversion=CVT_MONO;}
				else if (data==PHOTOMETRIC_MINISBLACK)	{filler=0xff; pBmpInfo->dotConversion=CVT_MONO_INVERT;}	
			}
			else pBmpInfo->dotConversion = CVT_GREY_LEVELS;
			*/

			// --- Size ---
			if (width  && !TIFFGetField (file, TIFFTAG_IMAGEWIDTH,  width))  return Error(ERR_CONT, 0, "File %s: Could not get image width", filepath);
			if (height && !TIFFGetField (file, TIFFTAG_IMAGELENGTH, height)) return Error(ERR_CONT, 0, "File %s: Could not get image height", filepath);
		}
		TIFFClose(file);
		(*width) += spacePx;
		return REPLY_OK;
	}
	return REPLY_NOT_FOUND;
}

//--- tif_get_info --------------------------------------------------------------------------------------
int tif_get_info(const char *path, STiffInfo *pinfo)
{
	TIFF* file;
	INT16 unit=1;
	float resolution;
	float ratio = 1.0; 
	int ret = REPLY_OK;

	// early return
	if (!pinfo)
		return REPLY_ERROR;

	*_ErrorStr = 0;
	*_WarnStr = 0;
	_TIFFerrorHandler = _tif_ErrorHandler;
	_TIFFwarningHandler = _tif_WarningHandler;
	memset(pinfo, 0, sizeof(STiffInfo));

	if (*path) file = TIFFOpen(path, "r");
	else       file = tif_stream_open(0, 0);
	if (file == NULL) return REPLY_NOT_FOUND;

	if (!TIFFGetField(file, TIFFTAG_SAMPLESPERPIXEL, &pinfo->SamplesPerPixel))	{ ret = Error(ERR_CONT, 0, "File %s: Could not get samples per pixel", path);		goto End; }
	if (!TIFFGetField(file, TIFFTAG_BITSPERSAMPLE, &pinfo->BitsPerSample))		{ ret = Error(ERR_CONT, 0, "File %s: Could not get bits per sample value", path);	goto End; }
	if (!TIFFGetField(file, TIFFTAG_IMAGEWIDTH, &pinfo->Width))					{ ret = Error(ERR_CONT, 0, "File %s: Could not get image width", path);				goto End; }
	if (!TIFFGetField(file, TIFFTAG_IMAGELENGTH, &pinfo->Length))				{ ret = Error(ERR_CONT, 0, "File %s: Could not get image height", path);			goto End; }
	if (!TIFFGetField(file, TIFFTAG_PLANARCONFIG, &pinfo->PlanarConfig))		{ ret = Error(ERR_CONT, 0, "File %s: Could not get image planar config", path);		goto End; }
	TIFFGetField(file, TIFFTAG_RESOLUTIONUNIT, &unit);	
	if (unit == RESUNIT_CENTIMETER)	ratio = (float)2.54;
	if (!TIFFGetField(file, TIFFTAG_XRESOLUTION, &resolution))					{ ret = Error(ERR_CONT, 0, "File %s: Could not get x resolution", path);	goto End; }
	pinfo->Resol.x = (UINT32)(resolution * ratio);
	if (!TIFFGetField(file, TIFFTAG_YRESOLUTION, &resolution))					{ ret = Error(ERR_CONT, 0, "File %s: Could not get y resolution", path);	goto End; }
	pinfo->Resol.y = (UINT32)(resolution * ratio);

End:
	if (file)
		TIFFClose(file);
	return (ret);
}

//--- tif_abort -----------------------------------------------
void tif_abort(void)
{
	_Abort = TRUE;
}

//--- tif_load_mt (multi threaded) ------------------------------------------------------------------------------------------
int tif_load_mt(SPageId *id, const char *filedir, const char *filename, int printMode, UINT32 spacePx, SColorSplitCfg *psplit, int splitCnt, BYTE* buffer[MAX_COLORS], SBmpInfo *pinfo, progress_fct progress)
{
	int c, i;
	int y, h;
	int compression;
	int	threadCnt;
	INT32 height;
	INT64 fileSize;
	char filepath[MAX_PATH];
	TIFF *tiff;
	BYTE* dst;
		
	_Abort = FALSE;
	*_ErrorStr=0;
	*_WarnStr=0;
	_TIFFerrorHandler   = _tif_ErrorHandler;
	_TIFFwarningHandler = _tif_WarningHandler;
	memset(pinfo, 0, sizeof(SBmpInfo));
	_tif_init();
	for (c=0; c<MAX_COLORS && c<splitCnt; c++)
	{
		if (psplit[c].color.name[0] && psplit[c].lastLine)
		{
			pinfo->colorCode[c]=psplit[c].color.colorCode;
			pinfo->inkSupplyNo[c] =psplit[c].inkSupplyNo;
			if (*filename)
			{
				if (id->page>1) sprintf(filepath, "%s/%s_P%06d_%s.tif", filedir, filename, id->page, RX_ColorNameShort(pinfo->inkSupplyNo[c]));
				else			sprintf(filepath, "%s/%s_%s.tif", filedir, filename, RX_ColorNameShort(pinfo->inkSupplyNo[c]));
								
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
					for (dst=_FileBuf, len=fileSize; len>=blksize; len-=blksize)
					{
						dst += fread(dst, 1, (int)blksize, file);
					}
					strcpy(_LastFilePath, filepath);
					fread(dst, (int)len, 1, file);					
					fclose(file);
				}
			}
			else 
			{
				Error(ERR_ABORT, 0, "Straeming not implemented yet");
			}
			
			tiff=rx_mem_tif_open(_FileBuf, fileSize);
			
			if (!tiff) return REPLY_NOT_FOUND;
			pinfo->printMode     = printMode;
			if (!TIFFGetField (tiff, TIFFTAG_BITSPERSAMPLE, &pinfo->bitsPerPixel))	return Error(ERR_CONT, 0, "File %s: Could not get bit per sample value", filepath);
			if (!TIFFGetField (tiff, TIFFTAG_IMAGEWIDTH,    &pinfo->srcWidthPx))	return Error(ERR_CONT, 0, "File %s: Could not get image width", filepath);
			if (!TIFFGetField (tiff, TIFFTAG_IMAGELENGTH,   &pinfo->lengthPx))		return Error(ERR_CONT, 0, "File %s: Could not get image height", filepath);
			TIFFGetField (tiff, TIFFTAG_COMPRESSION,   &compression);			
			
			pinfo->srcWidthPx	+= spacePx; 
			pinfo->lineLen		= (pinfo->srcWidthPx*pinfo->bitsPerPixel+7)/8;
			pinfo->dataSize		= pinfo->lineLen*pinfo->lengthPx;								
			pinfo->buffer[c]	= &buffer[c];
			height = pinfo->lengthPx;
			if (psplit[c].lastLine<height) height=psplit[c].lastLine;
			
			y = psplit[c].firstLine;
			if (compression==COMPRESSION_NONE) 	threadCnt = rx_core_cnt();	// seeking only allowed for uncompressed files
			else								threadCnt = 1;
			
			h = height/threadCnt;
			for (i=0; i<threadCnt; i++)
			{
				_ThreadPar[i].no	   = i;	
				_ThreadPar[i].fileBuf  = _FileBuf;
				_ThreadPar[i].fileSize = fileSize;
				_ThreadPar[i].pinfo    = pinfo;
				_ThreadPar[i].buffer   = buffer[c];
				_ThreadPar[i].y_from   = y;
				_ThreadPar[i].y_to	   = y+h;
				_ThreadPar[i].gap	   = spacePx;
				y += h;
			}
			_ThreadPar[threadCnt-1].y_to =height;				

			_SemDone  = rx_sem_create();
			for (i=0; i<threadCnt; i++) rx_sem_post(_ThreadPar[i].sem_start);
			{
				int running = threadCnt;
				int done;
				while (running)
				{
					if(rx_sem_wait(_SemDone, 500) == REPLY_OK) running--;
					else
					{
						if (progress!=NULL && height!=0) 
						{
							for (i=0, done=0; i<threadCnt; i++) done += _ThreadPar[i].y;
							progress(id, "", 101*done / (height*threadCnt));
						}
					}
				}
				if (progress!=NULL) progress(id, "", 100);
			}
			rx_sem_destroy(&_SemDone);
		}
	}
	return REPLY_OK;
}

//--- tif_load (multi threaded) ------------------------------------------------------------------------------------------
int tif_load(SPageId *id, const char *filedir, const char *filename, int printMode, UINT32 spacePx, INT32 wakeupLen, SColorSplitCfg *psplit, int splitCnt, BYTE* buffer[MAX_COLORS], SBmpInfo *pinfo, progress_fct progress)
{
	int c, i;
	int threadCnt;
	int	wakeupOn;
	int time;
	INT32 height, lineLen;
	FLOAT val;
	char filepath[MAX_PATH];
	STifThreadPar *ppar;
			
	time = rx_get_ticks();
	
	wakeupOn = FALSE;
	if(wakeupLen > 0) wakeupOn = TRUE;
	else			  wakeupLen = -wakeupLen;
	
	_Abort = FALSE;
	
	*_ErrorStr=0;
	*_WarnStr=0;
	_TIFFerrorHandler   = _tif_ErrorHandler;
	_TIFFwarningHandler = _tif_WarningHandler;
	memset(pinfo, 0, sizeof(SBmpInfo));
	_tif_init();
	threadCnt = 0;
	pinfo->colorCnt=0;
	for (c=0; c<MAX_COLORS && c<splitCnt; c++)
	{
		if (psplit[c].color.name[0] && psplit[c].lastLine)
		{			
			ppar = &_ThreadPar[threadCnt];
			pinfo->colorCode[c]=psplit[c].color.colorCode;
			pinfo->inkSupplyNo[c] =psplit[c].inkSupplyNo;
			if (*filename)
			{
				if (id->page>1) sprintf(filepath, "%s/%s_P%06d_%s.tif", filedir, filename, id->page, RX_ColorNameShort(pinfo->inkSupplyNo[c]));
				else			sprintf(filepath, "%s/%s_%s.tif", filedir, filename, RX_ColorNameShort(pinfo->inkSupplyNo[c]));							
				ppar->file = TIFFOpen (filepath, "r");
				if (ppar->file) strcpy(_LastFilePath, filepath);
				else Error(LOG, 0, "%d: %s color %s not found", id->id, filename, RX_ColorNameShort(pinfo->inkSupplyNo[c]));
			}
			else 
			{
				Error(ERR_ABORT, 0, "Straeming not implemented yet");
			}
			
			pinfo->printMode     = printMode;
			if (ppar->file)
			{
				if (!TIFFGetField (ppar->file, TIFFTAG_BITSPERSAMPLE, &pinfo->bitsPerPixel))	return Error(ERR_CONT, 0, "File %s: Could not get bit per sample value", filepath);
				if (!TIFFGetField (ppar->file, TIFFTAG_IMAGEWIDTH,    &pinfo->srcWidthPx))		return Error(ERR_CONT, 0, "File %s: Could not get image width", filepath);
				if (!TIFFGetField (ppar->file, TIFFTAG_IMAGELENGTH,   &pinfo->lengthPx))		return Error(ERR_CONT, 0, "File %s: Could not get image height", filepath);
				if (TIFFGetField (ppar->file, TIFFTAG_XRESOLUTION,   &val))	pinfo->resol.x=(int)val; else pinfo->resol.x=DPI_X;
				if (TIFFGetField (ppar->file, TIFFTAG_YRESOLUTION,   &val))	pinfo->resol.y=(int)val; else pinfo->resol.y=DPI_Y;
			}

			pinfo->srcWidthPx	+= spacePx; 
			pinfo->lineLen		= lineLen = (pinfo->srcWidthPx*pinfo->bitsPerPixel+7)/8;
			pinfo->dataSize		= pinfo->lineLen*pinfo->lengthPx;
			if (ppar->file) pinfo->buffer[c] = &buffer[c];
			else			pinfo->buffer[c] = NULL;
			height = pinfo->lengthPx;
			if (psplit[c].lastLine<height) height=psplit[c].lastLine;

			ppar->no	   = threadCnt;	
			ppar->pinfo    = pinfo;
			ppar->buffer   = buffer[c]+wakeupLen*lineLen;
			ppar->y_from   = psplit[c].firstLine;
			ppar->y_to	   = height;
			ppar->gap	   = spacePx;
			pinfo->colorCnt++;
			
			threadCnt++;
		}
	}
	
	if (threadCnt)
	{
		_SemDone  = rx_sem_create();
			
		for (i=0; i<threadCnt; i++) rx_sem_post(_ThreadPar[i].sem_start);

		//--- wait --------------------------
		{
			int running = threadCnt;
			int done;
			while (running)
			{
				if(rx_sem_wait(_SemDone, 500) == REPLY_OK) running--;
				else
				{
					if (progress!=NULL && height!=0) 
					{
						for (i=0, done=0; i<threadCnt; i++) done += _ThreadPar[i].y;
						progress(id, "", 101*done / (height*threadCnt));
					}
				}
			}
			if (!_Abort && progress) progress(id, "", 100);
		}
		rx_sem_destroy(&_SemDone);
		
		//--- add wakeup ----------------------
		if (wakeupLen)
		{
			for (i=0; i<threadCnt; i++) 
			{
				if (_ThreadPar[i].buffer)
				{
					int start;
					start = -(INT32)wakeupLen;
					memset(_ThreadPar[i].buffer+start*lineLen,           0x00, wakeupLen * lineLen);
					memset(_ThreadPar[i].buffer+(UINT64)pinfo->lengthPx*lineLen, 0x00, wakeupLen * lineLen);
				
					if (wakeupOn)
					{
						start=(-(INT32)wakeupLen+i*WAKEUP_BAR_LEN);
						memset(_ThreadPar[i].buffer+start*lineLen, 0xff, WAKEUP_BAR_LEN * lineLen);
						memset(_ThreadPar[i].buffer+((UINT64)pinfo->lengthPx+wakeupLen-(i+1)*WAKEUP_BAR_LEN)*lineLen, 0xff, WAKEUP_BAR_LEN * lineLen);															
					}
				}
			};
		}
		pinfo->lengthPx += 2*wakeupLen;
			
		/*
		// Tests: Mark end of label
		{
			for (i=0; i<threadCnt; i++)
			{
				memset(_ThreadPar[i].buffer, 0xff, lineLen/2);
				memset(_ThreadPar[i].buffer+pinfo->dataSize-3*lineLen/2, 0x33, lineLen/2);
				memset(_ThreadPar[i].buffer+pinfo->dataSize-lineLen/2, 0xCC, lineLen/2);
			}
		}
		*/
	}
	
//	Error(LOG, 0, "Loaded, time=%d ms", rx_get_ticks()-time);
	return REPLY_OK;
}

//--- tif_last_filepath --------------------------------------
char* tif_last_filepath(void)
{
	return _LastFilePath;
}

//--- _tif_read_thread ------------------------------------
static void *_tif_read_thread(void* lpParameter)
{
	STifThreadPar *par=(STifThreadPar*)lpParameter;
	while (_ThreadRunning)
	{
		rx_sem_wait(par->sem_start, 0);
		{			
			TIFF *file;
			BYTE *dst;
			int srcLen;
			int spaceLen;
			int ret;
			
			par->result	= REPLY_ERROR;
			
			if(par->fileBuf) file = rx_mem_tif_open(par->fileBuf, par->fileSize);
			else			 file = par->file;
			
			if(file)
			{				
				srcLen				= ((par->pinfo->srcWidthPx-par->gap)*par->pinfo->bitsPerPixel + 7) / 8; 			
				spaceLen			= par->pinfo->lineLen - srcLen;
				dst					= par->buffer + (par->y_from*par->pinfo->lineLen);				
								
				for(par->y = par->y_from; par->y <= par->y_to; par->y++)
				{										
					if (_Abort)
					{
						Error(LOG, 0, "Abort Thread %d", par->no);
						break;							
					}
					ret = TIFFReadScanline(file, dst, par->y, 1);
					dst += srcLen;
					if(spaceLen)
					{
						memset(dst, 0x00, spaceLen);
						dst += spaceLen;
					}
				}					
				
				TIFFClose(file);
				par->result			= REPLY_OK;
			}
		}
		
		rx_sem_post(_SemDone);
	}
	return NULL;
}


//--- tif_write ------------------------------------------------------------------------------------------
int tif_write(const char *filedir, const char *filename, SBmpInfo *pinfo, char *color)
{
	int c, ret = REPLY_OK;
	char filepath[MAX_PATH];
	TIFF *file = NULL;
	BYTE *dst;
	char desc[64];
//	uint16 colormap[12];
//	int i;

	*_ErrorStr = 0;
	*_WarnStr = 0;
	_TIFFerrorHandler = _tif_ErrorHandler;
	_TIFFwarningHandler = _tif_WarningHandler;
	for (c = 0; c<pinfo->planes; c++)
	{
		if (pinfo->buffer[c])
		{
            if (color && strcmp(RX_ColorNameShort(pinfo->inkSupplyNo[c]), color)) continue;

			sprintf(filepath, "%s/%s_%s.tif", filedir, filename, RX_ColorNameShort(pinfo->inkSupplyNo[c]));
			file = TIFFOpen(filepath, "w");
			if (file)
			{
				int line;

				// Header
				if (!TIFFSetField(file, TIFFTAG_IMAGEWIDTH, pinfo->srcWidthPx))			{ ret = Error(ERR_CONT, 0, "File %s: Could not set width value", filepath);				goto End; }
				if (!TIFFSetField(file, TIFFTAG_IMAGELENGTH, pinfo->lengthPx))			{ ret = Error(ERR_CONT, 0, "File %s: Could not set length value", filepath);			goto End; }
				if (!TIFFSetField(file, TIFFTAG_BITSPERSAMPLE, pinfo->bitsPerPixel))	{ ret = Error(ERR_CONT, 0, "File %s: Could not set bit per sample value", filepath);	goto End; }
				if (!TIFFSetField(file, TIFFTAG_SAMPLESPERPIXEL, 1))					{ ret = Error(ERR_CONT, 0, "File %s: Could not set samples per pixel value", filepath);	goto End; }
				if (!TIFFSetField(file, TIFFTAG_COMPRESSION, /*COMPRESSION_NONE*/ COMPRESSION_LZW))			{ ret = Error(ERR_CONT, 0, "File %s: Could not set compression value", filepath);		goto End; }
				if (!TIFFSetField(file, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_MINISWHITE))	{ ret = Error(ERR_CONT, 0, "File %s: Could not set photometric value", filepath);		goto End; }
				if (!TIFFSetField(file, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG))		{ ret = Error(ERR_CONT, 0, "File %s: Could not set planar config value", filepath);		goto End; }
				if (!TIFFSetField(file, TIFFTAG_RESOLUTIONUNIT, RESUNIT_INCH))			{ ret = Error(ERR_CONT, 0, "File %s: Could not set resolution unit value", filepath);	goto End; }
				if (!TIFFSetField(file, TIFFTAG_XRESOLUTION, (double)pinfo->resol.x))	{ ret = Error(ERR_CONT, 0, "File %s: Could not set x resolution value", filepath);		goto End; }
				if (!TIFFSetField(file, TIFFTAG_YRESOLUTION, (double)pinfo->resol.y))	{ ret = Error(ERR_CONT, 0, "File %s: Could not set y resolution value", filepath);		goto End; }
				sprintf(desc, "%s (%s)", RX_ColorNameLong(pinfo->colorCode[c]), RX_ColorNameShort(pinfo->inkSupplyNo[c]));
				if (!TIFFSetField(file, TIFFTAG_IMAGEDESCRIPTION, desc))				{ ret = Error(ERR_CONT, 0, "File %s: Could not set image description value", filepath);	goto End; }
	/*			if (!TIFFSetField(file, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_PALETTE))		{ ret = Error(ERR_CONT, 0, "File %s: Could not set photometric value", filepath);		goto End; }

				// red          green         blue
				colormap[0] = colormap[4] = colormap[8] = 65535;  // white
				colormap[1] = colormap[5] = colormap[9] = 43690;  // pale grey
				colormap[2] = colormap[6] = colormap[10] = 21845; // dark grey
				colormap[3] = colormap[7] = colormap[11] = 0;     // black
				if (!TIFFSetField(file, TIFFTAG_COLORMAP, colormap))			{ ret = Error(ERR_CONT, 0, "File %s: Could not set colormap value", filepath);			goto End; }
	*/
				// Write lines
				for (line = 0; line < pinfo->lengthPx; ++line) {
					dst = *pinfo->buffer[c] + pinfo->lineLen*line;
					if (TIFFWriteScanline(file, dst , line, 1)<0) { ret = Error(ERR_CONT, 0, "File %s: Could not write line %d", filepath, line);	goto End; }
	//				for (i = 0; i < pinfo->lineLen; i++, dst++ )
	//				if (*dst)
	//					TrPrintf(TRUE, "byte not empty >>%x<< at line %d row %d", *dst, line, i);
				}


				// Close 
				TIFFClose(file);
				file = NULL;
			}
			else 
			{ 
				ret = Error(ERR_CONT, 0, "File %s: Could not create", filepath);				goto End; 
			}
		}
	}
End:
	if (file)
		TIFFClose(file);

	return (ret);
}

//--- tif_load_simple -----------------------------------------------------
int tif_load_simple	(const char *path, BYTE **buffer, int bufsize, SBmpInfo *pinfo)
{
	UINT32 h, height;
	UINT32 step;
	int srcLen;
	int spaceLen;
	int	ok=FALSE;
	TIFF *file;
	BYTE *dst;
	
	_Abort = FALSE;
	*_ErrorStr=0;
	*_WarnStr=0;
	_TIFFerrorHandler   = _tif_ErrorHandler;
	_TIFFwarningHandler = _tif_WarningHandler;
	
	if (file=TIFFOpen (path, "r"))
	{
		ok = TRUE;
		if (!TIFFGetField (file, TIFFTAG_BITSPERSAMPLE, &pinfo->bitsPerPixel))	return Error(ERR_CONT, 0, "File %s: Could not get bit per sample value", path);
		if (!TIFFGetField (file, TIFFTAG_IMAGEWIDTH,    &pinfo->srcWidthPx))	return Error(ERR_CONT, 0, "File %s: Could not get image width", path);
		if (!TIFFGetField (file, TIFFTAG_IMAGELENGTH,   &pinfo->lengthPx))		return Error(ERR_CONT, 0, "File %s: Could not get image height", path);

		srcLen				= (pinfo->srcWidthPx*pinfo->bitsPerPixel+7)/8; 
		pinfo->srcWidthPx	+= 0; 
		pinfo->lineLen		= (pinfo->srcWidthPx*pinfo->bitsPerPixel+7)/8;
		pinfo->dataSize		= pinfo->lineLen*pinfo->lengthPx;
				
		spaceLen			= pinfo->lineLen-srcLen;
				
		dst = *buffer;
		if (dst==NULL) return REPLY_ERROR;
		printf("rx_mem buffer %p\n", dst);
		height = pinfo->lengthPx;
		step = height/10;
		for (h=0; h<height && !_Abort; h++)
		{
			TIFFReadScanline(file, dst, h, 1);		
			dst += srcLen;
			if (spaceLen)
			{
				memset(dst, 0x00, spaceLen);
				dst += spaceLen;
			}
		}
		TIFFClose(file);
	}
	else
	{
		if (str_start(_ErrorStr, ": Cannot open") && !ok)  *_ErrorStr=0;
		return REPLY_NOT_FOUND;				
	}

	return REPLY_OK;						
}


//--- tif_load_8bppx -------------------------------------------------------------------------------------------
// loads TEST bitmap and converts 8 bit/pixel to 2 bit/pixel
int tif_load_8bppx(const char *path, int printMode, SColorSplitCfg *psplit, int splitCnt, BYTE* buffer[MAX_COLORS], SBmpInfo *pinfo)
{
	int c, namelen;
	int h;
	int srcLineLen, x;
	char filepath[MAX_PATH];
	TIFF *file;
	BYTE *src, *dst, val;
	 
	strcpy(filepath, path);
	namelen = (int)strlen(filepath);

	_TIFFerrorHandler   = _tif_ErrorHandler;
	_TIFFwarningHandler = _tif_WarningHandler;

	memset(pinfo, 0, sizeof(SBmpInfo));
	for (c=0; c<MAX_COLORS && c<splitCnt; c++)
	{
		if (psplit[c].color.name[0])
		{
			sprintf(&filepath[namelen-4], "_%s.tif", RX_ColorNameShort(psplit[c].inkSupplyNo));
			if (file=TIFFOpen (filepath, "r"))
			{
				pinfo->printMode       = printMode;
				pinfo->colorCode[c]	   = psplit[c].color.colorCode;
				pinfo->inkSupplyNo[c]  = psplit[c].inkSupplyNo;
				if (!TIFFGetField (file, TIFFTAG_BITSPERSAMPLE, &pinfo->bitsPerPixel))	return Error(ERR_CONT, 0, "File %s: Could not get bit per sample value", filepath);
				if (!TIFFGetField (file, TIFFTAG_IMAGEWIDTH,    &pinfo->srcWidthPx))	return Error(ERR_CONT, 0, "File %s: Could not get image width", filepath);
				if (!TIFFGetField (file, TIFFTAG_IMAGELENGTH,   &pinfo->lengthPx))		return Error(ERR_CONT, 0, "File %s: Could not get image height", filepath);
				srcLineLen			= pinfo->srcWidthPx;
				pinfo->bitsPerPixel = 2;
				pinfo->lineLen		= (pinfo->srcWidthPx*pinfo->bitsPerPixel+7)/8;
				pinfo->dataSize		= pinfo->lineLen*pinfo->lengthPx;
				src = (BYTE*)rx_mem_alloc(srcLineLen);
				pinfo->buffer[c] = &buffer[c];
				dst = buffer[c];
				printf("rx_mem buffer %p\n", dst);
				for (h=psplit[c].firstLine; h<psplit[c].lastLine && h<pinfo->lengthPx; h++)
				{
					TIFFReadScanline(file, src, h, 0);
					//--- convert bits -----
					// 0x00 -> 3
					// 0x55 -> 2
					// 0xaa -> 1
					// 0xff -> 0
					memset(dst, 0x00, pinfo->lineLen);
					for (x=0; x<srcLineLen; x++)
					{
						switch(src[x])
						{
						case 0x00: val=0x03; break;
						case 0x55: val=0x02; break;
						case 0xaa: val=0x01; break;
						case 0xff: val=0x00; break;
						}
						dst[x/4] |= val<<(6-(2*(x%4)));
					}
					dst += pinfo->lineLen;
				}
				TIFFClose(file);
				rx_mem_free(&src);
			}
		}
	}

	return REPLY_OK;	
}



//--- tif_load_full --------------------------------------------------------------------------------------
int tif_load_full(const char *path, SColorSplitCfg *psplit, int splitCnt, SBmpInfo *pinfo)
{
	TIFF* file;
	STiffInfo TiffInfo;
	static BYTE* buffer[MAX_COLORS];
	int ret = REPLY_OK, r, s, i;
	BYTE* dst=0, *src=0, *psrc= 0;

	// early return
	if (!pinfo)
		return REPLY_ERROR;

	*_ErrorStr = 0;
	*_WarnStr = 0;
	_TIFFerrorHandler = _tif_ErrorHandler;
	_TIFFwarningHandler = _tif_WarningHandler;
	memset(&TiffInfo, 0, sizeof(STiffInfo));
	memset(pinfo, 0, sizeof(SBmpInfo));

	ret = tif_get_info(path, &TiffInfo);
	if (TiffInfo.BitsPerSample != 8) return Error(ERR_CONT, 0, "File %s: Bits per sample value must be 8 but %d", path, TiffInfo.BitsPerSample);
	if ((TiffInfo.SamplesPerPixel < 1) || (TiffInfo.SamplesPerPixel > splitCnt))  return Error(ERR_CONT, 0, "File %s: Wrong samples per pixel: %d>%d", path, TiffInfo.SamplesPerPixel, splitCnt);
	for (s = 0; s < TiffInfo.SamplesPerPixel; s++)
	{
		buffer[s] = rx_mem_alloc(TiffInfo.Width*TiffInfo.Length);
		pinfo->buffer[s] = &buffer[s];
		if (!pinfo->buffer[s]) return Error(ERR_CONT, 0, "File %s: No buffer for sample: %d", path, s);
	}

	if (ret) return (ret);
	file = TIFFOpen(path, "r");
	if (file == NULL) return REPLY_NOT_FOUND;
	if (TiffInfo.PlanarConfig != PLANARCONFIG_SEPARATE)
	{
		src = (BYTE*)rx_mem_alloc(TiffInfo.Width*TiffInfo.SamplesPerPixel);
		if (!src) { ret = Error(ERR_CONT, 0, "File %s: Could not alloc %d byte", path, (TiffInfo.Width*TiffInfo.SamplesPerPixel));		goto End; }
	}

	for (r = 0; r < (int) TiffInfo.Length; r++)
	{
		if (TiffInfo.PlanarConfig == PLANARCONFIG_SEPARATE)
		{
			for (s = 0; s < TiffInfo.SamplesPerPixel; s++)
			{
				dst = *pinfo->buffer[s] + (TiffInfo.Width*r);
				if (TIFFReadScanline(file, dst, r, s) < 0) { ret = Error(ERR_CONT, 0, "File %s: Could not read plane %d line %d ", path, s, r);		goto End; }
			}
		}
		else // PLANARCONFIG_CONTIG
		{
			static BYTE mem = 0;
			if (TIFFReadScanline(file, src, r, 1) < 0) { ret = Error(ERR_CONT, 0, "File %s: Could not read line %d ", path, r);		goto End; }
			//			for (i = 0; i < (TiffInfo.Width*TiffInfo.SamplesPerPixel); i++) {
			//				if (*(src+i))
			//					TrPrintf(TRUE, "byte not empty >>%x<< at line %d row %d", *(src + i), r, i);
			//			}
			for (i = 0; i < (int)TiffInfo.Width; i++)
			{
				for (s = 0; s < TiffInfo.SamplesPerPixel; s++)
				{
					dst = *pinfo->buffer[s] + (TiffInfo.Width*r) + i;
					psrc = src + TiffInfo.SamplesPerPixel*i + s;
					
					if ((*psrc) && (*psrc != mem))
						mem = *psrc;
					*dst = *psrc;
				}
			}

		}
	}
	for (s = 0; s < TiffInfo.SamplesPerPixel; s++)
	{
		if (psplit[s].color.name[0] && psplit[s].lastLine)
		{
			pinfo->colorCode[s] = psplit[s].color.colorCode;
		}
		else 
		{
			ret = Error(ERR_CONT, 0, "File %s: No color define for sample: %d", path, s); goto End;
		}
	}
	pinfo->bitsPerPixel = TiffInfo.BitsPerSample;
	pinfo->srcWidthPx = TiffInfo.Width;
	pinfo->lengthPx = TiffInfo.Length;
	pinfo->lineLen = TiffInfo.Width;
	pinfo->dataSize = pinfo->lineLen*pinfo->lengthPx;
	pinfo->aligment = 8;
	pinfo->planes = TiffInfo.SamplesPerPixel;
	pinfo->resol = TiffInfo.Resol;

End:
	if (file)
		TIFFClose(file);
	if (src)
		rx_mem_free(&src);
	return (ret);
}

//--- _tif_ErrorHandler ----------------------------------------------------------
static void _tif_ErrorHandler(const char* module, const char* fmt, va_list ap)
{
	vsprintf(_ErrorStr, fmt, ap);
}

//--- _tif_WarningHandler --------------------------------------------------------
static void _tif_WarningHandler(const char* module, const char* fmt, va_list ap)
{
	vsprintf(_WarnStr, fmt, ap);
}

//--- _tif_stream_open -----------------------------------------------------
static TIFF* _tif_stream_open(int page, int bitmap)
{
	return NULL;
}

