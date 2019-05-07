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
SDisabledJets RX_DisabledJets[MAX_COLORS];
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
	memset(RX_DisabledJets, 0, sizeof(RX_DisabledJets));
	_Active  = FALSE;
	return REPLY_OK;							
}

//--- jc_set_disabled_jets -----------------------
void jc_set_disabled_jets(SDisabledJets *jets)
{
	int h, n;
	if (RX_Spooler.printerType==printer_TX801 || RX_Spooler.printerType==printer_TX802) return;

	if (jets->color>=0 && jets->color<MAX_COLORS)
	{
		memcpy(&RX_DisabledJets[jets->color], jets, sizeof(RX_DisabledJets[jets->color]));
		for(h=0; !_Active && h<MAX_HEADS_COLOR; h++)
		{
			for (n=0; !_Active && n<MAX_DISABLED_JETS; n++)
				_Active |= RX_DisabledJets[jets->color].disabledJets[h][n];
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
	int color, n, head, jet;
	int pixelPerByte;
	SBmpSplitInfo	*pInfo; 

	if (pBmpInfo->bitsPerPixel<2)
	{
		_GetPixel = _get_pixel1;
		_SetPixel = _set_pixel1;
		pixelPerByte = 8;
	}
	else
	{
		_GetPixel = _get_pixel2;
		_SetPixel = _set_pixel2;
		pixelPerByte = 4;
	}

	for (color=0; color<SIZEOF(pBmpInfo->buffer); color++)
	{
		for (head=0; head<RX_Spooler.headsPerColor; head++)
		{
			if (RX_Spooler.headNo[color][head])
			{
				pInfo = &pItem->splitInfo[RX_Spooler.headNo[color][head]-1];
				if (pInfo->data && pBmpInfo->buffer[color] && pBmpInfo->bitsPerPixel)
				{
					for (n=0; n<MAX_DISABLED_JETS; n++)
					{
						jet = RX_DisabledJets[color].disabledJets[head][n];
						if (jet)
						{
							Error(LOG, 0, "Disable Jet color=%d, head=%d, jet=%d", color, head, jet);
							jet += pInfo->startBt*pixelPerByte + pInfo->jetPx0;
							_disable_jet(*pInfo->data, pBmpInfo->bitsPerPixel, pBmpInfo->lengthPx, pBmpInfo->lineLen, jet, fromLine);
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
				}
			}
		//	if (pBmpInfo->printMode==PM_SCANNING || pBmpInfo->printMode==PM_SINGLE_PASS) break;
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
	int jetMax=bytesPerLine*8/bitsPerPixel;
	
	if (jet>0 && jet<jetMax)
	{
		//--- special for "Jet Numbers" Image
		if (fromLine)
		{
			for (y=0; y<50; y++)
			{
				for (s=-15; s<=15; s++)
					_SetPixel(pBuffer, bytesPerLine, jet+s, fromLine-y, 3);
			}
			for (y=0; y<20; y++)
			{
				//--- arrow on top ----------------------------------------
				_SetPixel(pBuffer, bytesPerLine, jet-y-1, fromLine-y-1, 0);
				_SetPixel(pBuffer, bytesPerLine, jet-y,   fromLine-y-1, 0);
				_SetPixel(pBuffer, bytesPerLine, jet,     fromLine-y-1, 0);
				_SetPixel(pBuffer, bytesPerLine, jet+y,   fromLine-y-1, 0);
				_SetPixel(pBuffer, bytesPerLine, jet+y+1, fromLine-y-1, 0);

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
