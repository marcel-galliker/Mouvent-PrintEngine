// ****************************************************************************
//
//	spool_rip.c
//
// ****************************************************************************
//
//	Copyright 2015 by Radex AG, Switzerland. All rights reserved.
//	Written by Marcel Galliker 
//
// ****************************************************************************


//--- includes ---------------------------------------------------------------
#include "rx_def.h"
#include "rx_error.h"
#include "rx_tif.h"
#include "rx_flz.h"
#include "rx_trace.h"
#include "rx_bitmap.h"
#include "rx_mem.h"
#include "rx_rip_lib.h"
#include "rx_data.h"
#include "rx_counter.h"
#include "bmp.h"
#include "rx_threads.h"
// #include "rx_free_type.h"
#include "data.h"
#include "ctrl_client.h"
#include "spool_rip.h"
#include "screening.h"

#ifdef linux
#include <errno.h>
#endif


//----------------------------------------------------------------------
#define WIN_USER	"Guest"
#define WIN_PWD		"Guest"
#define LX_PWD		"radex"
#define MNT_PATH	"/mnt/ergosoft"

//--- defines ----------------------------------------------------------------
#define DATA_BUF_SIZE	32

//--- global  variables ------------------------------------------------------
static BYTE		  *_DataPtr;

static char			_DataPath[MAX_PATH];
static SLayoutDef   _Layout;
static SFileDef     _Filedef;
static SBmpInfo		_BmpInfoLabel;
static SBmpInfo		_BmpInfoColor;
static UINT64		_BufferLabelSize=0;
static UINT64		_BufferColorSize=0;
static BYTE		   *_BufferLabel[MAX_COLORS];
static BYTE		   *_BufferColor[MAX_COLORS];
static int			_Column;

static BYTE		   	_DataBuf[DATA_BUF_SIZE][MAX_MESSAGE_SIZE];
static int			_DataBufIn;
static int			_DataBufOut;

static BOOL _Abort = FALSE;

//--- prototypes -------------------------------------------------
static void _free_buffer();
static void _load_buffer(const char *filedir, const char *filename, SBmpInfo *pinfo, UINT64 *pBufSize, BYTE *buffer[MAX_COLORS]);
static void _multiply_image(SBmpInfo *pinfo, UINT32 width, BYTE *buffer[MAX_COLORS]);
static void _rip_label();


//--- sr_mnt_drive -----------------------------------------------
void sr_mnt_drive(const char *path)
{
	/*
	#ifdef linux
	#define OUTPUT "/tmp/mount"
	if (*path)
	{		
		// Giving access right in ubuntu:
		// > sudo visudo
		// >> add line "radex ALL = NOPASSWD: /bin/mount"
		// >> add line "radex ALL = NOPASSWD: /bin/mkdir"
		
		char command[2048];
		int ret;

		TrPrintfL(TRUE, "sr_mnt_drive: Check mount");
		//--- check wether already mounted ---
		{
			FILE *file;
			int result;

			result=system("mount -l -tcifs > " OUTPUT);
			file = fopen(OUTPUT, "r");
			if (file)
			{
				result=fread(command, 1, sizeof(command), file);
				fclose(file);
				remove(OUTPUT);
				if (strstr(command, MNT_PATH)) 
				{
					TrPrintfL(TRUE, "sr_mnt_drive: already mounted: end");
					return;
				}
			}
		}

		TrPrintfL(TRUE, "sr_mnt_drive: Check mnt");
		if (access(MNT_PATH, 0))
		{
			sprintf(command, "echo %s | sudo -S mkdir -m777 %s", LX_PWD, MNT_PATH); 
			ret=system(command);
		}

		TrPrintfL(TRUE, "sr_mnt_drive: mount");
		int uid = getuid();	// actual user ID
		int gid = getgid();	// actual group ID
		sprintf(command, "echo %s | sudo -S mount -t cifs %s %s -o user=%s,password=%s,uid=%d,gid=%d", LX_PWD, path, MNT_PATH, WIN_USER, WIN_PWD, uid, gid); 
		ret=system(command);

		TrPrintfL(TRUE, "sr_mnt_drive: end");
	}
	#endif
	*/
}

//--- sr_mnt_path ------------------------------------------------------

void sr_mnt_path(const char *orgPath, char *mntPath)
{
	#ifdef linux
		strcpy(mntPath, MNT_PATH);
		strcat(mntPath, &orgPath[strlen(RX_Spooler.dataRoot)]);
	#else
		strcpy(mntPath, orgPath);
	#endif
}

//--- sr_reset ---------------------------------------------------
void sr_reset()
{
	_DataBufIn = _DataBufOut = 0;
	_Abort = FALSE;
	_free_buffer();
	TrPrintfL(TRUE, "sr_reset");
}

//--- sr_set_layout_start --------------------------------
void sr_set_layout_start(RX_SOCKET socket, char *dataPath)
{
	for (int i = 0; i < MAX_COLORS; i++)
	{
		if (_BufferLabel[i] || _BufferColor[i])
		{
			TrPrintfL(1, "sr_set_layout_end: _BufferLabel[%d] WAIT FREE %p, used=%d", i, _BufferLabel[i], rx_mem_cnt(_BufferLabel[i]));
			while (!_Abort && (rx_mem_cnt(_BufferLabel[i]) || rx_mem_cnt(_BufferColor[i])))
			{
				rx_sleep(10);
			}
		}
	}
	strcpy(_DataPath, dataPath);
	_DataPtr = (BYTE*)&_Layout;
}

//--- sr_set_layout_blk -----------------------------------
void sr_set_layout_blk  (RX_SOCKET socket, SMsgHdr *hdr)
{
	int len = hdr->msgLen-sizeof(SMsgHdr);
	memcpy(_DataPtr, &hdr[1], len);
	_DataPtr+=len;
}

//--- sr_abort ------------------------
void sr_abort(void)
{
	_Abort = TRUE;
}

//--- sr_set_layout_end -----------------------------------
void sr_set_layout_end  (RX_SOCKET socket, SMsgHdr *msg)
{
	SRipBmpInfo info;
	int width;
	char filename[MAX_PATH];

	_free_buffer();

	//--- default values in case there is no label bitmap ---
	memset(&_BmpInfoLabel, 0, sizeof(_BmpInfoLabel));
	memset(&_BmpInfoColor, 0, sizeof(_BmpInfoColor));
	_BmpInfoLabel.printMode		= _BmpInfoColor.printMode    = PM_SINGLE_PASS;
	_BmpInfoLabel.bitsPerPixel	= _BmpInfoColor.bitsPerPixel = 1;
	width    = (_Layout.columns-1)*_Layout.columnDist + _Layout.width;
	_BmpInfoLabel.srcWidthPx	= _BmpInfoColor.srcWidthPx   = (int)((double)width/25400.0*DPIX);
	_BmpInfoLabel.lengthPx		= _BmpInfoColor.lengthPx	 = (int)(_Layout.height/25400.0*DPIX);
	_BmpInfoLabel.lineLen		= _BmpInfoColor.lineLen      = (_BmpInfoLabel.srcWidthPx+7)/8;
	_BmpInfoLabel.dataSize		= _BmpInfoColor.dataSize	   = (UINT64)_BmpInfoLabel.lineLen * _BmpInfoLabel.lengthPx;
	
	split_path(_Layout.label, NULL, filename, NULL);

	if (*_Layout.label) _load_buffer(_Layout.label,	filename, &_BmpInfoLabel, &_BufferLabelSize, _BufferLabel);
	
	if (*_Layout.label && *_Layout.colorLayer) 
	{
		strcat(filename, "-Color");
		_load_buffer(_Layout.label, filename, &_BmpInfoColor, &_BufferColorSize, _BufferColor);
	}

	info.width		  = _BmpInfoLabel.srcWidthPx;
	info.height		  = _BmpInfoLabel.lengthPx;
	info.bitsPerPixel = _BmpInfoLabel.bitsPerPixel;
	info.lineLen	  = _BmpInfoLabel.lineLen;

	rip_add_fonts(_DataPath);
	rip_set_layout(&_Layout, &info);
	_Column = 0;
}

//--- sr_get_label_size ----------------------
void sr_get_label_size(int *width, int *length, UCHAR *bitsPerPixel)
{
	*width			= _BmpInfoLabel.srcWidthPx;
	*length			= _BmpInfoLabel.lengthPx;
	*bitsPerPixel	= _BmpInfoLabel.bitsPerPixel;
}

//--- sr_set_filedef_start --------------------------------
void sr_set_filedef_start(RX_SOCKET socket, SMsgHdr *msg)
{
	for (int i = 0; i < MAX_COLORS; i++)
	{
		if (_BufferLabel[i] || _BufferColor[i])
		{
			TrPrintfL(1, "sr_set_layout_end: _BufferLabel[%d] WAIT FREE %p, used=%d", i, _BufferLabel[i], rx_mem_cnt(_BufferLabel[i]));
			while (!_Abort && (rx_mem_cnt(_BufferLabel[i]) || rx_mem_cnt(_BufferColor[i])))
			{
				rx_sleep(10);
			}
		}
	}
	_DataPtr = (BYTE*)&_Filedef;
}

//--- sr_set_filedef_blk -----------------------------------
void sr_set_filedef_blk  (RX_SOCKET socket, SMsgHdr *hdr)
{
	int len = hdr->msgLen-sizeof(SMsgHdr);
	memcpy(_DataPtr, &hdr[1], len);
	_DataPtr+=len;
}

//--- sr_set_filedef_end -----------------------------------
void sr_set_filedef_end  (RX_SOCKET socket, SMsgHdr *msg)
{
	dat_set_file_def(&_Filedef);
}

//--- sr_data_record ---------------------------------------------
void sr_data_record		(SPrintDataMsg *pmsg)
{
	int next;

	next=(_DataBufIn+1) % DATA_BUF_SIZE;
	if (next==_DataBufOut) 
	{
		Error(ERR_STOP, 0, "Data Buffer Overflow");
		return;
	}
	memcpy(&_DataBuf[_DataBufIn], pmsg, pmsg->hdr.msgLen);
	_DataBufIn=next;
}

//--- _free_buffer ------------------------------------------------------
static void _free_buffer()
{
	int i;
	for (i=0; i<SIZEOF(_BufferLabel); i++) 
	{
		rx_mem_use_clear(_BufferLabel[i]);
		rx_mem_free(&_BufferLabel[i]);
	}
	_BufferLabelSize = 0;
	for (i=0; i<SIZEOF(_BufferColor); i++) 
	{
		rx_mem_use_clear(_BufferColor[i]);
		rx_mem_free(&_BufferColor[i]);
	}
	_BufferColorSize = 0;
}

//--- _multiply_image ------------------------------------------------------------------
static void _multiply_image(SBmpInfo *pinfo, UINT32 width, BYTE *buffer[MAX_COLORS])
{
	int color;
	int srclen= (width*pinfo->bitsPerPixel+7)/8;
	int dist  = (int)((double)_Layout.columnDist/25400.0*DPIX);
	dist = (dist*pinfo->bitsPerPixel+7)/8;
	
	for (color=0; color<MAX_COLORS; color++)
	{
		if (buffer[color])
		{
			int x, y;
			BYTE *src;
			BYTE *dst;
			for (src = buffer[color], y=0; y<(int)pinfo->lengthPx; y++, src+=pinfo->lineLen)
			{
				for(dst=src, x=1; x<_Layout.columns; x++)
				{
					dst += dist;
					memcpy(dst, src, srclen);
				}
			}
		}
	}
}

//--- _load_buffer ------------------------------------------------------
static void _load_buffer(const char *filedir, const char *filename, SBmpInfo *pinfo, UINT64 *pBufSize, BYTE *buffer[MAX_COLORS])
{
	int ret;
	EFileType fileType;
	int width, length;
	UINT32 gapPx = 0;
	UINT8 multiCopy;

	SPageId id;
	int scanning = FALSE;
	char path[MAX_PATH];

	memset(&id, 0, sizeof(id));
//	sr_mnt_path(filepath, path);
	data_cache(&id, filedir, path, RX_Color, SIZEOF(RX_Color));

	if (data_get_size(path, 0, &fileType, &gapPx, &width, &length, (UCHAR *)&pinfo->bitsPerPixel, &multiCopy))
	{
		memset(buffer, 0, MAX_COLORS*sizeof(BYTE*));
		return;
	}

	int dist = (int)((double)_Layout.columnDist / 25400.0 * DPIX);

	if ((ret = data_malloc(scanning, width * _Layout.columns + dist * (_Layout.columns - 1), length, pinfo->bitsPerPixel, RX_Color, SIZEOF(RX_Color), pBufSize, buffer)) == REPLY_OK)
	{
		if (_Layout.columns>1 && width < pinfo->srcWidthPx) 
		{
			switch (fileType)
			{
				case ft_flz:
					ret = flz_load(&id, filedir, filename, PM_SINGLE_PASS, pinfo->srcWidthPx - width+8/pinfo->bitsPerPixel, 0, RX_Color, SIZEOF(RX_Color), buffer, pinfo, ctrl_send_load_progress, NULL);
					break;
				case ft_tif:
					ret = tif_load(&id, filedir, filename, PM_SINGLE_PASS, pinfo->srcWidthPx - width+8/pinfo->bitsPerPixel, 0, RX_Color, SIZEOF(RX_Color), buffer, pinfo, ctrl_send_load_progress);
					break;
				default:
					Error(ERR_ABORT, 0, "Filetype not implemented");
					return;
			}
			_multiply_image(pinfo, width, buffer);
		}
		else
			switch (fileType)
			{
				case ft_flz:
					ret = flz_load(&id, filedir, filename, PM_SINGLE_PASS, 0, 0, RX_Color, SIZEOF(RX_Color), buffer, pinfo, ctrl_send_load_progress, NULL);
					break;
				case ft_tif:
					ret = tif_load(&id, filedir, filename, PM_SINGLE_PASS, 0, 0, RX_Color, SIZEOF(RX_Color), buffer, pinfo, ctrl_send_load_progress);
					break;
				default:
					Error(ERR_ABORT, 0, "Filetype not implemented");
					return;
			}
	}
	if (ret != REPLY_OK) Error(ERR_ABORT, 0, "Error reading VDP image %s", filename);
	
	}

void sr_rip_unused()
{
	for (int color = 0; color < MAX_COLORS; color++)
	{
		int black = (RX_Color[color].color.colorCode == 0);
		if (black)
		{
			if (_BufferLabel[color]) rx_mem_unuse(&_BufferLabel[color]);
			if (_BufferColor[color]) rx_mem_unuse(&_BufferColor[color]);
		}
	}
}

static int _screen(SBmpInfo *pInfo, int offsetPx, int lengthPx, int blkNo, int blkCnt, const char *filepath)
{
	const int pixelsPerByte = 4;
	const int stitchingBt = 32; //128 pixels in 2 bits per pixel on stitching 
	const headPx = 2048; // pixel by heads
	const int headBt = headPx / pixelsPerByte; // bytes per head

	SPageId id;
	int headCnt = RX_Spooler.headsPerColor * RX_Spooler.colorCnt;
	SPrintListItem *item = NULL;
	int jetPx0 = 0;

	id.copy = id.id = id.page = id.scan = (pInfo == &_BmpInfoLabel);

	if ((item = (SPrintListItem *)malloc(sizeof(SPrintListItem))) == NULL) return Error(ERR_ABORT, 0, "Could not allocate memory");

	if ((item->splitInfo = (SBmpSplitInfo *)malloc(headCnt * sizeof(SBmpSplitInfo))) == NULL) 
	{
		free(item);
		return Error(ERR_ABORT, 0, "Could not allocate memory");
	}

	strcpy(item->filepath, filepath);
	data_split(&id, pInfo, offsetPx, lengthPx, blkNo, blkCnt, 0, 0, 0, item);
	scr_wait(10);

	pInfo->bitsPerPixel = 2;
	pInfo->lineLen = (pInfo->srcWidthPx + pixelsPerByte - 1) / pixelsPerByte; // recompute width in Bytes

	// recreate the original big image in 2 bits per pixel
	for (int color = 0; color < RX_Spooler.colorCnt; color++)
	{
		if (!pInfo->buffer[color]) continue; // missing color
		BYTE *dst = *pInfo->buffer[color];

		// find where the image starts
		int colorOffset = RX_Color[color].offsetPx;
		int startPx = offsetPx - colorOffset;
		int headStart = 0;
		int btHeadStart = 0;
		if (startPx <= 0) 
		{
			headStart = -startPx / headPx;
			btHeadStart = ((-startPx % headPx) + pixelsPerByte + 1) / pixelsPerByte;
		}

		// cancel partially color offset as it is already in the screened image
		if (pInfo == &_BmpInfoLabel && startPx% pixelsPerByte)
		{
			RX_Color[color].offsetPx += (startPx % pixelsPerByte);
			if (startPx < 0) RX_Color[color].offsetPx += pixelsPerByte; 
		}

		// rebuild each line of the screened image
		for (long l = 0; l < pInfo->lengthPx; l++)
		{
			int wrtBt =0;
			// with the screened images of each head of the color
			for (int h = headStart; h < RX_Spooler.headsPerColor; h++)
			{
				int head = color * RX_Spooler.headsPerColor + h;
				if (item->splitInfo[head].data && *item->splitInfo[head].data)
				{
					int bt;
					int widthBt = min(item->splitInfo[head].widthBt, headBt);
					BYTE *src = *item->splitInfo[head].data + l * item->splitInfo[head].widthBt;
					if (h == headStart) 
					{
						if (startPx <= 0)
						{
							src += btHeadStart;
							widthBt -= btHeadStart;
						} 
						else
						{
							memset(dst, 0, startPx / pixelsPerByte);
							dst += startPx / pixelsPerByte;
							wrtBt += startPx / pixelsPerByte;
						}
					}
					else
					{
						int sBt = min(item->splitInfo[head].widthBt, stitchingBt);
						for (bt = 0; bt < sBt ; bt++)
							*dst++ |= *src++ & 0b11001100; // only even pixels
						widthBt -= sBt;
						wrtBt += sBt;
					}

					if (widthBt > 0)
					{
						memcpy(dst, src, widthBt);
						dst += widthBt;
						src += widthBt;
						wrtBt += widthBt;
						for (bt = headBt; bt < item->splitInfo[head].widthBt; bt++)
						{
							dst[bt - headBt] = *src++ & 0b00110011; // only odd pixels
						}
					}
				}
			}
			// and complete the image with white if print bar is smaller than original image
			if (pInfo->lineLen > wrtBt)
			{
				memset(dst, 0, pInfo->lineLen - wrtBt);
				dst += pInfo->lineLen - wrtBt;
			}
		}
	}
	free(item->splitInfo);
	free(item);

	if (FALSE && pInfo == &_BmpInfoLabel)
	{
		char dir[MAX_PATH];
		char fname[MAX_PATH];
		sprintf(dir, PATH_RIPPED_DATA "trace/vdp");

		strcpy(fname, "screened");
		pInfo->planes = RX_Spooler.colorCnt;
		tif_write(dir, fname, pInfo, NULL);
	}
	return jetPx0;
}

//--- sr_rip_label -----------------------------------------------------------
int sr_rip_label(BYTE *buffer[MAX_COLORS], SBmpInfo *pInfo, int offsetPx, int lengthPx, int blkNo, int blkCnt, const char *filepath)
{

	int column, len, color;
	BOOL jcneeded = TRUE;
	int time0 = rx_get_ticks();

	// screen the background and the mask if needed
	if (_BmpInfoColor.bitsPerPixel >= 8) _screen(&_BmpInfoColor, offsetPx, lengthPx, blkNo, blkCnt, filepath);
	if (_BmpInfoLabel.bitsPerPixel >= 8)
	{
		_screen(&_BmpInfoLabel, offsetPx, lengthPx, blkNo, blkCnt, filepath);
		jcneeded = FALSE; // jet compensation is done by screening and not need
	}

	int dataOut, data;
	int black;
	RX_Bitmap bmp, bmpLabel, bmpColor;
	SPrintDataMsg *pmsg = (SPrintDataMsg *)&_DataBuf[_DataBufOut];

	memcpy(pInfo, &_BmpInfoLabel, sizeof(*pInfo));
	pInfo->variable = TRUE;

	bmp.width = _BmpInfoLabel.srcWidthPx;
	bmp.height = _BmpInfoLabel.lengthPx;
	bmp.bppx = _BmpInfoLabel.bitsPerPixel;
	bmp.lineLen = _BmpInfoLabel.lineLen;
	bmp.sizeUsed = bmp.sizeAlloc = bmp.height * bmp.lineLen;
	memcpy(&bmpLabel, &bmp, sizeof(bmpLabel));
	memcpy(&bmpColor, &bmp, sizeof(bmpColor));
	data = dataOut = _DataBufOut;

	for (color=0; color<MAX_COLORS; color++)
	{
		black = (RX_Color[color].color.colorCode == 0);
		if (black) {
			pInfo->buffer[color] = &buffer[color];
			pInfo->inkSupplyNo[color] = RX_Color[color].inkSupplyNo; 
	//		TrPrintfL(1, "use buffer[%d], 0x%08x, size=%d", color, pInfo->buffer[color], pInfo->dataSize);
			if (buffer[color])
			{
				int time1=rx_get_ticks();
				bmp.buffer		= buffer[color];
				rx_mem_use(_BufferLabel[color]);
				bmpLabel.buffer = _BufferLabel[color];
				if (*_Layout.colorLayer) 
				{
					bmpColor.buffer = _BufferColor[color];
					rx_mem_use(_BufferColor[color]);
				}
				else bmpColor.buffer = NULL;
				data = dataOut;

				for (column = 0; column < _Layout.columns; column++)
				{
					if (data==_DataBufIn) 
					{
						Error(ERR_ABORT, 0, "VDP: Data Buffer underflow");
						return FALSE;
					}
					pmsg = (SPrintDataMsg*)&_DataBuf[data];
					len = pmsg->hdr.msgLen-sizeof(SPrintDataMsg)+1;
					int counter = pmsg->id.copy-1;
					if (ctr_increment_mode() == INC_perLabel) counter = counter*_Layout.columns+column;
					ctr_set_counter(counter);
					dat_set_buffer(0, len/2, pmsg->data);
					rip_data(&_Layout, column*_Layout.columnDist, 0, &bmp, &bmpLabel, &bmpColor, black);
					data = (data+1) % DATA_BUF_SIZE;
				}
		
				TrPrintfL(TRUE, "Rip Time[%d]=%dms", color, rx_get_ticks()-time1);

				//--- TEST ------------------------------
				if (FALSE)
				{
					char path[MAX_PATH];
					sprintf(path, "%s/test/%s_%d[%d]_%s.bmp", PATH_TEMP, "variable", pmsg->id.id, pmsg->id.copy, RX_ColorNameShort(RX_Color[color].inkSupplyNo));
					bmp_write(path, buffer[color], _BmpInfoLabel.bitsPerPixel, _BmpInfoLabel.srcWidthPx, _BmpInfoLabel.lengthPx, _BmpInfoLabel.lineLen, FALSE);
				}
			}
		}
	}
	TrPrintfL(TRUE, "Rip TimeTotal=%dms", rx_get_ticks()-time0);

	_DataBufOut = data;
	return jcneeded;
}