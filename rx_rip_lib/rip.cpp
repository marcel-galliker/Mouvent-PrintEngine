// ****************************************************************************
//
//	rip.cpp
//
// ****************************************************************************
//
//	Copyright 2013 by Radex AG, Switzerland. All rights reserved.
//	Written by Marcel Galliker 
//
// ****************************************************************************


//--- Include -----------------------------------------------------------------
#include "stdafx.h"
#include "rx_common.h"
#include <stdlib.h>
#include <malloc.h>
#include <wchar.h>
// #include "ErrorRip.h"
#include "rx_barcodes.h"
#include "rx_free_type.h"
#include "TCP_IP_OEM.h"
#include "rip.h"
#include "rx_rip_lib.h"
#include FT_GLYPH_H

#ifdef JOB_COMPOSER
	#include "ErrorRip.h"
	#define CR 0x0c
#else
	#include "bmp.h"
	#include "rx_error.h"
#endif

#ifdef linux
	#define HBITMAP HANDLE
#endif

//--- defines --------------------------------------------------------------------------------

typedef struct
{
	FT_Face		font;
	HBarCode*	pBarCode;
	HBITMAP		hbmp;

	union	
	{
		STextBox	txtBox;
		SBarcodeBox	bcBox;
		SBitmapBox	bmpBox;
	};
} SRipBox;

typedef struct
{
	int		boxCnt;
	SRipBox	*box;
} SRipLayout;


//--- EXTERNAL Variables ----------------------------------------------------------------------
//--- Global variables -------------------------------------------------------------------------

static int _Init=FALSE;
static SRipLayout _Layout[LAYOUT_CNT]; 

static int			_DpiX, _DpiY;
static int			_MonoDropSize;
static BYTE			_BCData[8*1024];
static FT_Face		_DefaultFont;

//--- PTROTOTYPES ------------------------------------------------
static void _prepare_bitmap(RX_Bitmap *pBmp,  RX_Bitmap *pLabel);

//--- rip_init ----------------------------------------
void rip_init(int dpiX, int dpiY, int dotSizes, int monoDropSize)
{
	if (!_Init)
	{
		_MonoDropSize = monoDropSize;
		_DpiX = dpiX;
		_DpiY = dpiY;

		ft_init(dpiX, dpiY, monoDropSize);
		memset(_Layout, 0, sizeof(_Layout));
		
//		_DefaultFont = ft_load_font((char*)"Arial");
	}
	_Init = TRUE;
}

//--- rip_end ----------------------------------
void rip_end()
{
	int	l;
	int	b;
	
	if (_Init)
	{
		for (l=0; l<SIZEOF(_Layout); l++)
		{
			if (_Layout[l].box)
			{
				for(b=0; b<_Layout[l].boxCnt; b++)
				{
					if (_Layout[l].box[b].font)
						ft_free_font(_Layout[l].box[b].font);
					_Layout[l].box[b].font = NULL;
					if (_Layout[l].box[b].pBarCode)	bc_release_box(_Layout[l].box[b].pBarCode);
				}
				free(_Layout[l].box);
			}
		}
		memset(_Layout, 0, sizeof(_Layout));
	}
//	bmp_destroy(&_pBmp);
	_Init = FALSE;
}

//--- rip_set_layout --------------------------------------
int rip_set_layout(SMsgLayout *layout)//, SBmpInfo *pInfo, BYTE *label, BYTE *color)
{
	int			b;
	int			pos, len;
	SRipLayout	*lay;
	SRipBox		*box;
	USHORT		type;

	if (layout->layoutNo>=LAYOUT_CNT)
	{
		Error(ERR_CONT, 610, "RIP: Supprots only layoutNo %d", LAYOUT_CNT);
		return REPLY_ERROR;
	}

	lay			= &_Layout[layout->layoutNo];
	lay->boxCnt = layout->boxCnt;
	len			= lay->boxCnt*sizeof(SRipBox);
	lay->box	= (SRipBox*)malloc(len);
	memset(lay->box, 0, len);
	pos=0;
	for (b=0; b<lay->boxCnt; b++)
	{
		box = &lay->box[b];
		type = *(USHORT*)&layout->data[pos];
		if		(type==BOX_TYPE_TEXT)		len=sizeof(STextBox);
		else if (type==BOX_TYPE_BARCODE)	len=sizeof(SBarcodeBox);
		else if (type==BOX_TYPE_BITMAP)		len=sizeof(SBitmapBox);
		else Error(ERR_CONT, 611, "_Layout.box[%d].type=%d, undefined", b+1, type);
		memcpy(&box->bcBox, &layout->data[pos], len);
		pos += len;
		if (type==BOX_TYPE_TEXT)    
		{
			box->font = ft_load_font(box->txtBox.font.lfFaceName);
		}
		else if (type==BOX_TYPE_BARCODE) 
		{			
			box->pBarCode = bc_init_box(&box->bcBox, b, _DpiX, _DpiY);
		}
		else if (type==BOX_TYPE_BITMAP)	 
		{
		}
	}
	return REPLY_OK;
}

//--- _prepare_bitmap ------------------------------------------
static void _prepare_bitmap(RX_Bitmap *pBmp,  RX_Bitmap *pLabel)
{
	if (pLabel && pLabel->buffer)
	{
		int h, len;
		BYTE *src = pLabel->buffer;
		BYTE *dst = pBmp->buffer;
		len = pLabel->lineLen;
		if (len>pBmp->lineLen) len=pBmp->lineLen;
		h = pLabel->height;
		if (h>pBmp->height) h=pBmp->height;
		for (; h; h--)
		{
			memcpy(dst, src, len);
			dst += pBmp->lineLen;
			src += pLabel->lineLen;
		}
	} 
	else memset(pBmp->buffer, 0x00, pBmp->sizeUsed);
}

//--- rip_data -----------------------------------------
int rip_data(int x0, int y0, SMsgPrintData *printData, RX_Bitmap *pBmp, RX_Bitmap *pLabel, RX_Bitmap *pColor, int black)
{
	USHORT	   lineCnt;
	USHORT	   lineLen;
	BYTE	   *src;
	int			b, l;
	int			x, y;
	SRipLayout	*lay;
	SRipBox		*box;
//	char		test[64];
	
	if (!_Init) return REPLY_ERROR;

	if (printData->layoutNo>=LAYOUT_CNT)
	{
		Error(ERR_CONT, 613, "RIP: Data LayoutNo %d out of range", printData->layoutNo);
		return REPLY_ERROR;
	}
	/*
	if (_Layout[printData->layoutNo].boxCnt==0)
	{
		Error(ERR, 0, L"RIP: Data _Layout not defined", printData->layoutNo);
		return REPLY_ERROR;
	}
	*/
	lay = &_Layout[printData->layoutNo];
	if (lay->boxCnt!=printData->boxCnt)
	{
		Error(ERR_CONT, 614, "RIP: Data Got %d boxes, expected %d", printData->layoutNo, lay->boxCnt);
		return REPLY_ERROR;
	}

	if (!(x0+y0)) _prepare_bitmap(pBmp, pLabel);
	
	ft_set_color(pColor);

	//--- RIP ------------------------------------------------------
	x0 = (int)(x0*_DpiX/25400.0+0.5);
	y0 = (int)(y0*_DpiY/25400.0+0.5);
	//--- align y0 to byte ---
	if (!pBmp->bppx) return REPLY_ERROR;
	y0 = ((y0*pBmp->bppx+7)/8)*(8/pBmp->bppx);

	src = (BYTE*)&printData->data;
	for (b=0; b<printData->boxCnt; b++)
	{
		box = &lay->box[b];
		memcpy(&lineCnt, src, sizeof(USHORT)); src += sizeof(USHORT);

		switch (box->txtBox.hdr.boxType)
		{
		case BOX_TYPE_TEXT:		//--- TEXT-Box -----------------------------------------------------------				
								for (l=0; l<lineCnt; l++)
								{
									memcpy(&lineLen, src, sizeof(USHORT)); src += sizeof(USHORT);
									x = (int)(x0+box->txtBox.pos.x+l*box->txtBox.dx+0.5);
									y = (int)(y0+box->txtBox.pos.y+l*box->txtBox.dy+0.5);

									size_t  text_length = lineLen / 2;
									UINT16 *ptr_text    = reinterpret_cast<UINT16*>(src);

									FT_Vector origin;
									origin.x = x;
									origin.y = y;
									FT_Pos box_width  = labs(box->txtBox.hdr.rect.right - box->txtBox.hdr.rect.left);
									FT_Pos box_height = labs(box->txtBox.hdr.rect.bottom - box->txtBox.hdr.rect.top);
									ft_text_out(
										pBmp,
										origin,
										box_width,
										box_height,
										box->font,
										box->txtBox.alignment,
										box->txtBox.font.lfHeight,
										box->txtBox.font.lfOrientation,
										ptr_text,
										text_length,
										black);
									src += lineLen;
								}
								break;

		case BOX_TYPE_BARCODE:	//--- BARCODE Box ----------------------------------------------------------
								{
									int		len = 0;
									ERRCODE	eCode;

									memset(_BCData, 0, sizeof(_BCData));
									for (l=0; l<lineCnt; l++)
									{
										memcpy(&lineLen, src, sizeof(USHORT));	src += sizeof(USHORT);
										if (len+lineLen<SIZEOF(_BCData))
										{
											memcpy(&_BCData[len], src, lineLen);
											len += lineLen;
										}
										src += lineLen;
									}
									if ((pColor && pColor->buffer) || black)
									{
									//	wchar_to_char((const wchar_t*)_BCData, test, sizeof(test));
										eCode = bc_draw_code(pBmp, x0, y0, &box->bcBox, box->pBarCode, &box->bcBox.hdr.rect, (UTF16*)_BCData, len);
										if (eCode == S_OK)
										{
											bc_draw_bitmap(pBmp, x0, y0, &lay->box[b].bcBox, lay->box[b].pBarCode, _MonoDropSize, pColor);
										}
										else
										{
											char str[100];
											UTF16 wstr[100];
											BCGetErrorTextA(eCode, str, SIZEOF(str));
											rx_mbstowcs(wstr, str, SIZEOF(wstr));
											ft_text_out(pBmp, x0+box->bcBox.hdr.rect.left, y0+box->bcBox.hdr.rect.top, _DefaultFont, 500, 0, (UINT16*)wstr, (int)strlen(str), black);
											eCode=eCode;									
										}
									}
								}
								break;
		}
	}
	return REPLY_OK;
}
