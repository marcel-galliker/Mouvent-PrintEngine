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
#include "rx_trace.h"
#include "rx_bitmap.h"
#include "rx_mem.h"
#include "rx_rip_lib.h"
#include "rx_data.h"
#include "rx_counter.h"
#include "bmp.h"
// #include "rx_free_type.h"
#include "data.h"
#include "ctrl_client.h"
#include "spool_rip.h"

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
	_free_buffer();
}

//--- sr_set_layout_start --------------------------------
void sr_set_layout_start(RX_SOCKET socket, char *dataPath)
{
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
	_BmpInfoLabel.dataSize		= _BmpInfoColor.dataSize     = _BmpInfoLabel.lineLen * _BmpInfoLabel.lengthPx;
	
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
		rx_mem_unuse(&_BufferLabel[i]); //??
		rx_mem_free(&_BufferLabel[i]);
	}
	for (i=0; i<SIZEOF(_BufferColor); i++) 
	{
		rx_mem_unuse(&_BufferColor[i]); //??
		rx_mem_free(&_BufferColor[i]); //??
	}
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
	int i;
	int width, length;
	SPageId id;
	int scanning = FALSE;
	char path[MAX_PATH];

	memset(&id, 0, sizeof(id));
//	sr_mnt_path(filepath, path);
	data_cache(&id, filedir, path, RX_Color, SIZEOF(RX_Color));

	if (data_get_size(path, 0, 0, &width, &length, (UCHAR*)&pinfo->bitsPerPixel))
	{
		memset(buffer, 0, MAX_COLORS*sizeof(BYTE*));
		return;
	}
	
	ret = data_malloc (scanning, pinfo->srcWidthPx, pinfo->lengthPx, pinfo->bitsPerPixel, RX_Color, SIZEOF(RX_Color), pBufSize, buffer);
	if (_Layout.columns>1 && width < pinfo->srcWidthPx) 
	{
		ret = tif_load(&id, filedir, filename, PM_SINGLE_PASS, pinfo->srcWidthPx - width+8/pinfo->bitsPerPixel, RX_Color, SIZEOF(RX_Color), buffer, pinfo, ctrl_send_load_progress);
		_multiply_image(pinfo, width, buffer);
	}
	else ret = tif_load(&id, filedir, filename, PM_SINGLE_PASS, 0, RX_Color, SIZEOF(RX_Color), buffer, pinfo, ctrl_send_load_progress);	
	
	//--- TEST ------------------------------------------------------------------------
	if (FALSE)
	{
		char path[MAX_PATH];
		for (i=0; i<SIZEOF(RX_Color); i++)
		{
			if (pinfo->buffer[i])
			{
				sprintf(path, "%s%s_%s.bmp", PATH_TEMP, filename, RX_ColorNameShort(RX_Color[i].inkSupplyNo));
				bmp_write(path, *(pinfo->buffer)[i], pinfo->bitsPerPixel, pinfo->srcWidthPx, pinfo->lengthPx, pinfo->lineLen, FALSE);
			}
		}
	}
}

//--- sr_rip_label -----------------------------------------------------------
int  sr_rip_label(BYTE* buffer[MAX_COLORS], SBmpInfo *pInfo)
{
	int				column, len, color;
	int				dataOut, data;
	int				black;
	RX_Bitmap		bmp, bmpLabel, bmpColor;
	SPrintDataMsg	*pmsg = (SPrintDataMsg*)&_DataBuf[_DataBufOut];

	bmp.width	 = _BmpInfoLabel.srcWidthPx;
	bmp.height	 = _BmpInfoLabel.lengthPx;
	bmp.bppx	 = _BmpInfoLabel.bitsPerPixel;
	bmp.lineLen	 = _BmpInfoLabel.lineLen;
	bmp.sizeUsed = bmp.sizeAlloc = bmp.height*bmp.lineLen;
	memcpy(&bmpLabel, &bmp, sizeof(bmpLabel));
	memcpy(&bmpColor, &bmp, sizeof(bmpColor));
	memcpy(pInfo, &_BmpInfoLabel, sizeof(*pInfo));
	dataOut = _DataBufOut;
	for (color=0; color<MAX_COLORS; color++)
	{
		pInfo->buffer[color] = &buffer[color];
//		TrPrintfL(1, "use buffer[%d], 0x%08x, size=%d", color, pInfo->buffer[color], pInfo->dataSize);
		if (buffer[color])
		{
			bmp.buffer		= buffer[color];
			bmpLabel.buffer = _BufferLabel[color];
			if (*_Layout.colorLayer) bmpColor.buffer = _BufferColor[color];
			else bmpColor.buffer = NULL;
			black = (RX_Color[color].color.colorCode == 0);
			data = dataOut;
			for (column=0; column<_Layout.columns; column++)
			{
				if (data==_DataBufIn) return Error(ERR_STOP, 0, "Data Buffer underflow");
				pmsg = (SPrintDataMsg*)&_DataBuf[data];
				len = pmsg->hdr.msgLen-sizeof(SPrintDataMsg)+1;
				ctr_set_counter(pmsg->id.page);
				dat_set_buffer(0, len/2, pmsg->data);
				rip_data(&_Layout, column*_Layout.columnDist, 0, &bmp, &bmpLabel, &bmpColor, black);		
				data = (data+1) % DATA_BUF_SIZE;
			}

			//--- TEST ------------------------------
			if (TRUE)
			{
				char path[MAX_PATH];
				sprintf(path, "%s/test/%s_%d[%d]_%s.bmp", PATH_TEMP, "variable", pmsg->id.id, pmsg->id.page, RX_ColorNameShort(RX_Color[color].inkSupplyNo));
				bmp_write(path, buffer[color], _BmpInfoLabel.bitsPerPixel, _BmpInfoLabel.srcWidthPx, _BmpInfoLabel.lengthPx, _BmpInfoLabel.lineLen, FALSE);
			}
		}
	}
	_DataBufOut = data;
	return REPLY_OK;
}