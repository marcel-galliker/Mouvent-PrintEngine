// ****************************************************************************
//
//	DIGITAL PRINTING - jet_correction.c
//
//	Communication between ARM processor and FPGA
//
// ****************************************************************************
//
//	Copyright 2019 by radex AG, Switzerland. All rights reserved.
//	Written by Marcel Galliker
//
// ****************************************************************************

//--- includes ----------------------------------------------------------------
#include "rx_common.h"
#include "rx_def.h"
#include "rx_error.h"
#include "jet_correction.h"


//--- prototypes --------------------------------------------------------

static int  _get_pixel1(UCHAR *pBuffer, int bytesPerLine, int x, int y);
static int  _get_pixel2(UCHAR *pBuffer, int bytesPerLine, int x, int y);
static void _set_pixel1(UCHAR *pBuffer, int bytesPerLine, int x, int y, int val);
static void _set_pixel2(UCHAR *pBuffer, int bytesPerLine, int x, int y, int val);

static void _disable_jet(UCHAR *pBuffer, int bitsPerPixel, int length, int bytesPerLine, int jet);

static int  (*_GetPixel)(UCHAR *pBuffer, int bytesPerLine, int x, int y);
static void (*_SetPixel)(UCHAR *pBuffer, int bytesPerLine, int x, int y, int val);

//--- jc_init -----------------------------------------------------------
int jc_init(void)
{
	return REPLY_OK;							
}

//--- _jet_correction --------------------------------------------------
int	jc_correction (SBmpInfo *pBmpInfo,  SPrintListItem *pItem)
{
	int i, jet;

	for (i=0; i<SIZEOF(pBmpInfo->buffer); i++)
	{
		if (pBmpInfo->buffer[i])
		{
			if (pBmpInfo->bitsPerPixel<2)
			{
				_GetPixel = _get_pixel1;
				_SetPixel = _set_pixel1;
			}
			else
			{
				_GetPixel = _get_pixel2;
				_SetPixel = _set_pixel2;
			}
			for (jet=0; jet<pBmpInfo->srcWidthPx; jet+=2048)
			{
				_disable_jet(*pBmpInfo->buffer[i], pBmpInfo->bitsPerPixel, pBmpInfo->lengthPx, pBmpInfo->lineLen, jet-128);
				_disable_jet(*pBmpInfo->buffer[i], pBmpInfo->bitsPerPixel, pBmpInfo->lengthPx, pBmpInfo->lineLen, jet);
			}
			/* --------------- Line for stitch test --------------
			for (jet=0; jet<pBmpInfo->srcWidthPx; jet++)
			{
				_SetPixel(*pBmpInfo->buffer[i], pBmpInfo->lineLen, 0, jet+1, 3);
				_SetPixel(*pBmpInfo->buffer[i], pBmpInfo->lineLen, pBmpInfo->srcWidthPx-1, jet+1, 3);
				_SetPixel(*pBmpInfo->buffer[i], pBmpInfo->lineLen, jet, jet, 0);
				_SetPixel(*pBmpInfo->buffer[i], pBmpInfo->lineLen, jet, jet+1, 3);
				_SetPixel(*pBmpInfo->buffer[i], pBmpInfo->lineLen, jet, jet+2, 0);
			}
			*/
		}
	}
	return REPLY_OK;							
}

//--- _get_pixel1 -------------------------------
static int _get_pixel1(UCHAR *pBuffer, int bytesPerLine, int x, int y)
{
	return (pBuffer[y*bytesPerLine + x/8]>>(x%8)) & 0x01;
}

//--- _get_pixel2 -------------------------------
static int _get_pixel2(UCHAR *pBuffer, int bytesPerLine, int x, int y)
{
	return (pBuffer[y*bytesPerLine + x/4]>>(2*(x%4))) & 0x03;
}

//--- _set_pixel1 -------------------------------
static void _set_pixel1(UCHAR *pBuffer, int bytesPerLine, int x, int y, int val)
{
	int shift=(7-(x%8));
	pBuffer += y*bytesPerLine + x/8;
	*pBuffer &= ~(0x01<<shift);
	*pBuffer |= val<<shift;
}

//--- _set_pixel2 --------------------------------------------
static void _set_pixel2(UCHAR *pBuffer, int bytesPerLine, int x, int y, int val)
{
	int shift=(2*(3-(x%4)));
	pBuffer += y*bytesPerLine + x/4;
	*pBuffer &= ~(0x03<<shift);
	*pBuffer |= val<<shift;
}

//--- _disable_jet -----------------------------------------------------
static void _disable_jet(UCHAR *pBuffer, int bitsPerPixel, int length, int bytesPerLine, int jet)
{
	int y;
	
	if (jet>0 && jet<bytesPerLine*bitsPerPixel/8);

	for (y=0; y<length; y++)
	{
		_SetPixel(pBuffer, bytesPerLine, jet, y, 0);
//		_SetPixel(pBuffer, bytesPerLine, jet, y, 3);
	}

	Error(LOG, 0, "_disable_jet %d", jet);
}
