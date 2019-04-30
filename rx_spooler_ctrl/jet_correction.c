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


//-----------------------------------------------------------------------

static int _DisabledJets[MAX_COLORS][128];
static int _Active;
static int _MaxDropSize=3;

//--- prototypes --------------------------------------------------------

static int  _get_pixel1(UCHAR *pBuffer, int bytesPerLine, int x, int y);
static int  _get_pixel2(UCHAR *pBuffer, int bytesPerLine, int x, int y);
static void _set_pixel1(UCHAR *pBuffer, int bytesPerLine, int x, int y, int val);
static void _set_pixel2(UCHAR *pBuffer, int bytesPerLine, int x, int y, int val);

static void _disable_jet(UCHAR *pBuffer, int bitsPerPixel, int length, int bytesPerLine, int jet, int fromLine);

static int  (*_GetPixel)(UCHAR *pBuffer, int bytesPerLine, int x, int y);
static void (*_SetPixel)(UCHAR *pBuffer, int bytesPerLine, int x, int y, int val);

//--- jc_init -----------------------------------------------------------
int jc_init(void)
{
	memset(_DisabledJets, 0, sizeof(_DisabledJets));
	_Active  = FALSE;
	return REPLY_OK;							
}

//--- jc_set_disabled_jets -----------------------
void jc_set_disabled_jets(int color, INT16 *jets)
{
	int i;
	for (i=0; jets[i]; i++)
	{
		if (i<SIZEOF(_DisabledJets[color])) 
		{
			_DisabledJets[color][i] = jets[i];
			_Active = TRUE;
		}
		else 
		{
			Error(WARN, 0, "Color[%d]: Too many disabled jets", color);
			return;
		}
	}
}

//--- jc_active ---------------------------------------------------
int  jc_active(void)
{
	return _Active;
}

//--- _jet_correction --------------------------------------------------
int	jc_correction (SBmpInfo *pBmpInfo,  SPrintListItem *pItem, int fromLine)
{
	int i, n, jet, jet0;
//	return;
	for (i=0; i<SIZEOF(pBmpInfo->buffer); i++)
	{
		if (pBmpInfo->buffer[i] && pItem->splitInfo->bitsPerPixel)
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
			jet0 = pItem->splitInfo->fillBt * (8/pItem->splitInfo->bitsPerPixel) - pItem->splitInfo->jetPx0;
			for (n=0; n<SIZEOF(_DisabledJets[i]) && _DisabledJets[i][n]; n++)
			{
				Error(LOG, 0, "Jet in HeadPresout: %d", _DisabledJets[i][n]);
				jet = _DisabledJets[i][n]-jet0;
				if (jet>=0 && jet<pBmpInfo->srcWidthPx)
					_disable_jet(*pBmpInfo->buffer[i], pBmpInfo->bitsPerPixel, pBmpInfo->lengthPx, pBmpInfo->lineLen, jet, fromLine);
			}

			/*
			for (jet=0; jet<pBmpInfo->srcWidthPx; jet+=2048)
			{
				_disable_jet(*pBmpInfo->buffer[i], pBmpInfo->bitsPerPixel, pBmpInfo->lengthPx, pBmpInfo->lineLen, jet-128);
				_disable_jet(*pBmpInfo->buffer[i], pBmpInfo->bitsPerPixel, pBmpInfo->lengthPx, pBmpInfo->lineLen, jet);
			}
			*/
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
	return (pBuffer[y*bytesPerLine + x/8]>>(7-(x%8))) & 0x01;
}

//--- _get_pixel2 -------------------------------
static int _get_pixel2(UCHAR *pBuffer, int bytesPerLine, int x, int y)
{
	return (pBuffer[y*bytesPerLine + x/4]>>(2*(3-(x%4)))) & 0x03;
}

//--- _set_pixel1 -------------------------------
static void _set_pixel1(UCHAR *pBuffer, int bytesPerLine, int x, int y, int val)
{
	int shift=(7-(x%8));
	if (val>1) val=1;
	pBuffer += y*bytesPerLine + x/8;
	*pBuffer &= ~(0x01<<shift);
	*pBuffer |= val<<shift;
}

//--- _set_pixel2 --------------------------------------------
static void _set_pixel2(UCHAR *pBuffer, int bytesPerLine, int x, int y, int val)
{
	int shift=(2*(3-(x%4)));
	if (val>3) val=3;
	pBuffer += y*bytesPerLine + x/4;
	*pBuffer &= ~(0x03<<shift);
	*pBuffer |= val<<shift;
}

//--- _disable_jet -----------------------------------------------------
static void _disable_jet(UCHAR *pBuffer, int bitsPerPixel, int length, int bytesPerLine, int jet, int fromLine)
{
	int y, size, s;
	int offset;
	
	if (jet>0 && jet<bytesPerLine*8/bitsPerPixel)
	{
		//--- special for "Jet Numbers" Image
		if (fromLine)
		{
			for (y=0; y<10; y++)
			{
				//--- arrow on top ----------------------------------------
				_SetPixel(pBuffer, bytesPerLine, jet-y-1, fromLine-y-1, 3);
				_SetPixel(pBuffer, bytesPerLine, jet-y,   fromLine-y-1, 3);
				_SetPixel(pBuffer, bytesPerLine, jet,     fromLine-y-1, 3);
				_SetPixel(pBuffer, bytesPerLine, jet+y,   fromLine-y-1, 3);
				_SetPixel(pBuffer, bytesPerLine, jet+y+1, fromLine-y-1, 3);

				//--- arrow at bottom -------------------------------------
				_SetPixel(pBuffer, bytesPerLine, jet-y-1, length-10+y, 3);
				_SetPixel(pBuffer, bytesPerLine, jet-y,   length-10+y, 3);
				_SetPixel(pBuffer, bytesPerLine, jet,     length-10+y, 3);
				_SetPixel(pBuffer, bytesPerLine, jet+y,   length-10+y, 3);
				_SetPixel(pBuffer, bytesPerLine, jet+y+1, length-10+y, 3);
			}
		}

		for (y=fromLine; y<length; y++)
		{
			if (FALSE)
			{
				_SetPixel(pBuffer, bytesPerLine, jet-1, y, 0);
				_SetPixel(pBuffer, bytesPerLine, jet,   y, 3);
				_SetPixel(pBuffer, bytesPerLine, jet+1, y, 0);
			}
			else
			{
				if (size=_GetPixel(pBuffer, bytesPerLine, jet, y))
				{
				//	if (y&1) _SetPixel(pBuffer, bytesPerLine, jet-1,   y, 3);
				//	else     _SetPixel(pBuffer, bytesPerLine, jet+1,   y, 3);
					if (y&1) offset=1;
					else	 offset=-1;
					s=_GetPixel(pBuffer, bytesPerLine, jet-offset, y);
					if (s<_MaxDropSize) _SetPixel(pBuffer, bytesPerLine, jet-offset, y, s+1);
					else				_SetPixel(pBuffer, bytesPerLine, jet+offset, y, s+_GetPixel(pBuffer, bytesPerLine, jet+offset, y));
				}
				_SetPixel(pBuffer, bytesPerLine, jet, y, 0);
			}
		}
	}
	Error(LOG, 0, "_disable_jet %d", jet);
}
