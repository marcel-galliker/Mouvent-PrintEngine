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
#include "rx_trace.h"



//-----------------------------------------------------------------------
INT16 RX_DisabledJets[MAX_COLORS*MAX_HEADS_COLOR][MAX_DISABLED_JETS];
static int _Active;
static int _Changed;
static int _First;

//--- prototypes --------------------------------------------------------

static int  _get_pixel1(UCHAR *pBuffer, int bytesPerLine, int x, int y);
static int  _get_pixel2(UCHAR *pBuffer, int bytesPerLine, int x, int y);
static void _set_pixel1(UCHAR *pBuffer, int bytesPerLine, int x, int y, int val);
static void _set_pixel2(UCHAR *pBuffer, int bytesPerLine, int x, int y, int val);

static int _disable_jet(UCHAR *pBuffer, int bitsPerPixel, int length, int bytesPerLine, int jet, int fromLine, int maxdropsize);

static int  (*_GetPixel)(UCHAR *pBuffer, int bytesPerLine, int x, int y);
static void (*_SetPixel)(UCHAR *pBuffer, int bytesPerLine, int x, int y, int val);

//--- jc_init -----------------------------------------------------------
int jc_init(void)
{
	memset(RX_DisabledJets, 0, sizeof(RX_DisabledJets));
	_Active  = FALSE;
	_Changed = FALSE;
	return REPLY_OK;							
}

//--- jc_set_disabled_jets -----------------------
void jc_set_disabled_jets(SDisabledJetsMsg *pmsg)
{
	int n, ok;
	INT16	old[MAX_DISABLED_JETS];
	for (n=0, ok=FALSE; n<MAX_DISABLED_JETS; n++)
	{
		ok=(pmsg->disabledJets[n]!=0);
	}

	memcpy(old, &RX_DisabledJets[pmsg->head], sizeof(old));
	if (ok) memcpy(&RX_DisabledJets[pmsg->head], pmsg->disabledJets, sizeof(RX_DisabledJets[pmsg->head]));
	else    memset(&RX_DisabledJets[pmsg->head], -1, sizeof(RX_DisabledJets[pmsg->head]));

	if (pmsg->head>=0 && pmsg->head<SIZEOF(RX_DisabledJets))
	{
		for (n=0; n<MAX_DISABLED_JETS; n++)
		{
			if (RX_DisabledJets[pmsg->head][n]!=old[n]) _Changed = TRUE;
			if (pmsg->disabledJets[n]>=0)
			{
				_Active = TRUE;
				TrPrintfL(TRUE, "Head %d: Disable jet %d", pmsg->head, pmsg->disabledJets[n]);
			}
		}
	}
	_First = TRUE;
}

//--- jc_active ---------------------------------------------------
int  jc_active(void)
{
	if (_First)
	{
		int m;
		for (int head=0; head<SIZEOF(RX_DisabledJets); head++)
		{
			for (int n=0; n<MAX_DISABLED_JETS; n++)
			{
				//--- disable jet in left overlap ---
				if (RX_DisabledJets[head][n]>0 && RX_DisabledJets[head][n]<=HEAD_OVERLAP_SAMBA && head>0) 
				{
					for (m=0; m<MAX_DISABLED_JETS; m++)
					{
						if (RX_DisabledJets[head - 1][m] < 0 || RX_DisabledJets[head - 1][m] == RX_DisabledJets[head][n] + HEAD_WIDTH_SAMBA)
						{
							RX_DisabledJets[head - 1][m] = RX_DisabledJets[head][n] + HEAD_WIDTH_SAMBA;
							break;
						}
					}					
				}

				//--- disable jet in right overlap ---
				if (RX_DisabledJets[head][n]>=HEAD_WIDTH_SAMBA && head<SIZEOF(RX_DisabledJets)) 
				{
					for (m=0; m<MAX_DISABLED_JETS; m++)
					{
						if (RX_DisabledJets[head + 1][m] < 0 || RX_DisabledJets[head + 1][m] == RX_DisabledJets[head][n] - HEAD_WIDTH_SAMBA)
						{
							RX_DisabledJets[head + 1][m] = RX_DisabledJets[head][n] - HEAD_WIDTH_SAMBA;
							break;
						}
					}					
				}

			}
		}
		_First=FALSE;
	}
	return _Active;
}

//--- jc_changed ------------------------------------------------------
int jc_changed(void)
{	
	return _Changed;
}

void jc_head_correct(SBmpSplitInfo *pInfo, short *disabledJets, int fromLine, int lengthPx, int lineLen, int maxdropsize)
{
	int pixelPerByte;

	if (pInfo->bitsPerPixel<2)
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

	if (pInfo->data && pInfo->bitsPerPixel)
	{
		for (int n = 0; n < MAX_DISABLED_JETS; n++)
		{
			int jet = disabledJets[n];
			if (jet < 0) break;
			jet += (pInfo->startBt - pInfo->fillBt) * pixelPerByte + pInfo->jetPx0;
			if (jet >= 0)
			{
				TrPrintfL(TRUE, "correct %d drops on jet %d", _disable_jet(*pInfo->data, pInfo->bitsPerPixel, lengthPx, lineLen, jet, fromLine, maxdropsize), jet);
			}
		}
	}
}

int getmaxdropsize(const char *dots)
{
	int maxdropsize = 1;
	if (strchr(dots, 'M')) maxdropsize = 2;
	if (strchr(dots, 'L')) maxdropsize = 3;
	return maxdropsize;
}

//--- _jet_correction --------------------------------------------------
int	jc_correction (SBmpInfo *pBmpInfo,  SPrintListItem *pItem, int fromLine, const char* dots)
{
	int color, head;
	SBmpSplitInfo	*pInfo; 

	_Changed = FALSE;

	if (pBmpInfo->bitsPerPixel>=8) return REPLY_OK;	// correction after screening 
    if (rx_def_is_tx(RX_Spooler.printerType)) return REPLY_OK;


	for (color=0; color<SIZEOF(pBmpInfo->buffer); color++)
	{
		for (head=0; head<RX_Spooler.headsPerColor; head++)
		{
			if (RX_Spooler.headNo[color][head])
			{
				pInfo = &pItem->splitInfo[RX_Spooler.headNo[color][head]-1];
				jc_head_correct(pInfo, RX_DisabledJets[color * RX_Spooler.headsPerColor + head], fromLine, pBmpInfo->lengthPx, pBmpInfo->lineLen, getmaxdropsize(dots));
			}
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
	if (x < 0 || x >= 8 * bytesPerLine) return; // do not modify outside the image
	int shift = (7 - (x % 8));
	if (val>1) val=1;
	pBuffer += y*bytesPerLine + x/8;
	*pBuffer &= ~(0x01<<shift);
	*pBuffer |= val<<shift;
}

//--- _set_pixel2 --------------------------------------------
static void _set_pixel2(UCHAR *pBuffer, int bytesPerLine, int x, int y, int val)
{
	if (x < 0 || x >= 4 * bytesPerLine) return; // do not modify outside the image
	int shift=(2*(3-(x%4)));
	if (val>3) val=3;
	pBuffer += y*bytesPerLine + x/4;
	*pBuffer &= ~(0x03<<shift);
	*pBuffer |= val<<shift;
}

//--- _disable_jet -----------------------------------------------------
static int _disable_jet(UCHAR *pBuffer, int bitsPerPixel, int length, int bytesPerLine, int jet, int fromLine, int maxdropsize)
{
	int jetMax = bytesPerLine * 8 / bitsPerPixel;
	int ncorrect = 0;
	if (jet>0 && jet<jetMax)
	{
		//--- special for "Jet Numbers" Image
		if (fromLine)
		{
            int y, s;
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
				_SetPixel(pBuffer, bytesPerLine, jet-y-1, length-20+y, 3);
				_SetPixel(pBuffer, bytesPerLine, jet-y,   length-20+y, 3);
				_SetPixel(pBuffer, bytesPerLine, jet,     length-20+y, 3);
				_SetPixel(pBuffer, bytesPerLine, jet+y,   length-20+y, 3);
				_SetPixel(pBuffer, bytesPerLine, jet+y+1, length-20+y, 3);
			}
		}

        {
			int y;
            int d;			// droplets to compensate in this line
            int droplets=0;	// droplets to compensate total
			int max;		// maximum droplets that can be compensated in this line
			int side=0;		// side: to change left/right
            int org[2];		// original dot size 
            int comp[2];	// compensated dot size
			int limit = 200 * maxdropsize; // take into account the jc ratio in percent

			for (y=fromLine; y<length; y++)
			{
				if ((d = _GetPixel(pBuffer, bytesPerLine, jet, y)))
				{
					droplets += RX_Spooler.jc_ratio * d; // jc ratio could be more than 100%
					_SetPixel(pBuffer, bytesPerLine, jet, y, 0);
					if (droplets>limit) droplets=limit;
				}
				if (droplets > 0)
				{
					org[0] = comp[0] = _GetPixel(pBuffer, bytesPerLine, jet-1, y);
					org[1] = comp[1] = _GetPixel(pBuffer, bytesPerLine, jet+1, y);
					max = 2 * maxdropsize - org[0] - org[1];
					while (droplets > 0 && max > 0)
					{
						if (comp[side] < maxdropsize)
						{
							comp[side]++;
							ncorrect++;
							droplets -= 100; // ratio in percent
							max--;
						}
						side = 1 - side;
					}
					if (comp[0]!=org[0]) _SetPixel(pBuffer, bytesPerLine, jet-1, y, comp[0]);
					if (comp[1]!=org[1]) _SetPixel(pBuffer, bytesPerLine, jet+1, y, comp[1]);
				}
			}            
        }
	}
	return ncorrect;
}
