// ****************************************************************************
//
//	rx_screen_ed12.c
//
// ****************************************************************************
//
//	Copyright 2017 by Radex AG, Switzerland. All rights reserved.
//	Written by Gilbert Therville 
//
// ****************************************************************************

//--- includes ---------------------------------------------------------------------

#include "rx_error.h"
#include "rx_trace.h"
#include "rx_mem.h"
#include "rx_screen_ed12.h"


#ifdef linux
	#include "errno.h"
#endif

//--- ED12 -------------------------------------------------------------------------
typedef struct
{
	BYTE		EL1_L;	// even line to 1st line left  quartet
	BYTE		EL2_L;	// even line to 2nd line left  quartet
	BYTE		OL1_L;	// even line to 1st line left  quartet
	BYTE		OL2_L;	// even line to 2nd line left  quartet
	BYTE		EL1_R;	// odd	line to 1st line right quartet
	BYTE		EL2_R;	// odd  line to 2nd line right quartet
	BYTE		OL1_R;	// odd  line to 1st line right quartet
	BYTE		OL2_R;	// odd  line to 2nd line right quartet
	SHORT		ED_R;	// Error diffusion to right
	SHORT		ED_D;	// Error diffusion to down
	SHORT		ED_DR;	// Error diffusion to down and right
} ED12;


//--- Globals -------------------------------------------------------------------------
static ED12			ED12_13[13];
static ED12			TabED12[256];


//--- _bmp_plane_ED12screen ----------------------------------------------------------------------
int bmp_plane_ED12screen(SHORT plane, SBmpInfo *pinfo, SScreenConfig* pscreenConfig)
{
	UINT32 l, NewLineLen;
	int i;
	SHORT diff;
	BYTE Src, *pSrc, *pDst, *pSrcBis, *pDstBis, *pNew = NULL;
	BYTE *lut = pscreenConfig->lut[plane];

	NewLineLen = ((pinfo->srcWidthPx * 2 * 2) + 7) / 8;
	pNew = rx_mem_alloc(NewLineLen*pinfo->lengthPx * 2);
	TrPrintfL(1, "data_malloc buffer: %p, size=%u MB, free=%u MB", pNew, (NewLineLen*pinfo->lengthPx * 2) / 1024 / 1024, rx_mem_get_freeMB());
	if (pNew == NULL) return Error(ERR_CONT, 0, "No buffer for screening: %d");
	memset(pNew, 0, (NewLineLen*pinfo->lengthPx * 2));

	for (l = 0; l < pinfo->lengthPx; l++)
	{
		pSrc = *pinfo->buffer[plane] + (l * pinfo->lineLen);
		pSrcBis = pSrc + pinfo->lineLen;

		if (!(l % 2))			// line even
		{
			pDst = pNew + (l * 2 * NewLineLen);
			pDstBis = pDst + NewLineLen;
			for (i = 0; i < (int)pinfo->srcWidthPx; i++)
			{
				if (pSrc[i])
				{
					Src = lut[pSrc[i]]; // set value
					if (!(i % 2))	// line even, pixel even 
					{
						*pDst = TabED12[Src].EL1_L;
						*pDstBis = TabED12[Src].EL2_L;
					}
					else           // line even, pixel odd 
					{
						*pDst |= TabED12[Src].EL1_R;
						*pDstBis |= TabED12[Src].EL2_R;
						pDst++; pDstBis++;
					}
					// diff error
					if ((TabED12[Src].ED_R) && (i < (int)pinfo->srcWidthPx - 1)) // right
					{
						diff = TabED12[Src].ED_R + pSrc[i + 1];
						if ((diff >= 0) && (diff < 256))
							pSrc[i + 1] = (BYTE)diff;
					}
					if ((TabED12[Src].ED_D) && (l < pinfo->lengthPx - 1)) // down
					{
						diff = TabED12[Src].ED_D + pSrcBis[i];
						if ((diff >= 0) && (diff < 256))
							pSrcBis[i] = (BYTE)diff;
					}
					if ((TabED12[Src].ED_DR) && (l < pinfo->lengthPx - 1) && (i < (int)pinfo->srcWidthPx - 1)) // right & down
					{
						diff = TabED12[Src].ED_DR + pSrcBis[i + 1];
						if ((diff >= 0) && (diff < 256))
							pSrcBis[i + 1] = (BYTE)diff;
					}
				}
				else
				{
					if (i % 2)  // line even, pixel odd
					{
						pDst++; pDstBis++;
					}
				}
			}
		}
		else				// line odd
		{
			pDst = pNew + (l * 2 * NewLineLen) + NewLineLen - 1;
			pDstBis = pDst + NewLineLen;
			*pDstBis = *pDst = 0;
			for (i = pinfo->srcWidthPx - 1; i >= 0; i--)
			{
				if (pSrc[i])
				{
					Src = lut[pSrc[i]]; // set value
					if (!(i % 2))	// line odd, pixel even 
					{
						*pDst |= TabED12[Src].OL1_L;
						*pDstBis |= TabED12[Src].OL2_L;
						pDst--; pDstBis--;
					}
					else           // line odd, pixel odd 
					{
						*pDst = TabED12[Src].OL1_R;
						*pDstBis = TabED12[Src].OL2_R;
					}
					// diff error
					if ((TabED12[Src].ED_R) && (i > 0)) // right
					{
						diff = TabED12[Src].ED_R + pSrc[i - 1];
						if ((diff >= 0) && (diff < 256))
							pSrc[i - 1] = (BYTE)diff;
					}
					if ((TabED12[Src].ED_D) && (l < pinfo->lengthPx - 1)) // down
					{
						diff = TabED12[Src].ED_D + pSrcBis[i];
						if ((diff >= 0) && (diff < 256))
							pSrcBis[i] = (BYTE)diff;
					}
					if ((TabED12[Src].ED_DR) && (l < pinfo->lengthPx - 1) && (i >0)) // right & down
					{
						diff = TabED12[Src].ED_DR + pSrcBis[i - 1];
						if ((diff >= 0) && (diff < 256))
							pSrcBis[i - 1] = (BYTE)diff;
					}
				}
				else
				{
					if (!(i % 2))	// line odd, pixel even 
					{
						pDst--; pDstBis--;
					}
				}
			}
		}

	}

	rx_mem_free(pinfo->buffer[plane]);
	*pinfo->buffer[plane] = pNew;

	return (REPLY_OK);
}



//--- _rx_screen_ED12init ----------------------------------------------------------------------
int rx_screen_ED12init(SScreenConfig* pscreenConfig)
{
	int i, diff, val;
	float fmin = 0, fmax, fstep, fval;

	sprintf(pscreenConfig->typeName, "ED12xg");
	pscreenConfig->inputResol.x = pscreenConfig->inputResol.y = 600;
	pscreenConfig->outputResol.x = pscreenConfig->outputResol.y = 1200;

	// init ED12 13 Tab
	memset(ED12_13, 0, sizeof (ED12)* 13);
	ED12_13[1].EL1_R = 0x4;		ED12_13[2].EL1_R = 0x8;		ED12_13[3].EL1_R = 0xC;
	ED12_13[1].EL2_R = 0x0; 	ED12_13[2].EL2_R = 0x0;		ED12_13[3].EL2_R = 0x0;
	ED12_13[1].OL1_R = 0x1;		ED12_13[2].OL1_R = 0x2;		ED12_13[3].OL1_R = 0x3;
	ED12_13[1].OL2_R = 0x0;		ED12_13[2].OL2_R = 0x0;		ED12_13[3].OL2_R = 0x0;

	ED12_13[4].EL1_R = 0xC;		ED12_13[5].EL1_R = 0xC;		ED12_13[6].EL1_R = 0xC;
	ED12_13[4].EL2_R = 0x1; 	ED12_13[5].EL2_R = 0x2;		ED12_13[6].EL2_R = 0x3;
	ED12_13[4].OL1_R = 0x3;		ED12_13[5].OL1_R = 0x3;		ED12_13[6].OL1_R = 0x3;
	ED12_13[4].OL2_R = 0x4;		ED12_13[5].OL2_R = 0x8;		ED12_13[6].OL2_R = 0xC;

	ED12_13[7].EL1_R = 0xC;		ED12_13[8].EL1_R = 0xC;		ED12_13[9].EL1_R = 0xC;
	ED12_13[7].EL2_R = 0x7; 	ED12_13[8].EL2_R = 0xB;		ED12_13[9].EL2_R = 0xF;
	ED12_13[7].OL1_R = 0x3;		ED12_13[8].OL1_R = 0x3;		ED12_13[9].OL1_R = 0x3;
	ED12_13[7].OL2_R = 0xD;		ED12_13[8].OL2_R = 0xE;		ED12_13[9].OL2_R = 0xF;

	ED12_13[10].EL1_R = 0xD;	ED12_13[11].EL1_R = 0xE;	ED12_13[12].EL1_R = 0xF;
	ED12_13[10].EL2_R = 0xF; 	ED12_13[11].EL2_R = 0xF;	ED12_13[12].EL2_R = 0xF;
	ED12_13[10].OL1_R = 0x7;	ED12_13[11].OL1_R = 0xB;	ED12_13[12].OL1_R = 0xF;
	ED12_13[10].OL2_R = 0xF;	ED12_13[11].OL2_R = 0xF;	ED12_13[12].OL2_R = 0xF;
	for (i = 1; i < 13; i++)
	{
		ED12_13[i].EL1_L = ED12_13[i].EL1_R << 4;
		ED12_13[i].EL2_L = ED12_13[i].EL2_R << 4;
		ED12_13[i].OL1_L = ED12_13[i].OL1_R << 4;
		ED12_13[i].OL2_L = ED12_13[i].OL2_R << 4;
	}


	// init ED12 256 Tab
	memset(TabED12, 0, sizeof (ED12)* 256);
	fmax = fstep = 255.0 / 12.0; val = 0;
	for (i = 1; i < 256; i++)
	{
		// calculate val
		fval = (float)(i);
		if (fval > fmax) {
			fmin += fstep;
			fmax += fstep;
			val++;
		}
		if ((fval - fmin) < (fmax - fval)) // nearer min diff is positive
		{
			TabED12[i] = ED12_13[val];
			diff = (int)((fval - fmin) + 0.5);
			while (diff) {
				TabED12[i].ED_R++; diff--;
				if (diff) {
					TabED12[i].ED_D++; diff--;
					if (diff) {
						TabED12[i].ED_R++; diff--;
						if (diff) {
							TabED12[i].ED_DR++; diff--;
						}
					}
				}
			}
		}
		else // nearer  max: diff is negative
		{
			TabED12[i] = ED12_13[val + 1];
			diff = (int)((fval - fmax) - 0.5);
			while (diff) {
				TabED12[i].ED_R--; diff++;
				if (diff) {
					TabED12[i].ED_D--; diff++;
					if (diff) {
						TabED12[i].ED_R--; diff++;
						if (diff) {
							TabED12[i].ED_DR--; diff++;
						}
					}
				}
			}
		}
	}

	return (REPLY_OK);
}