// ****************************************************************************
//
//	rx_screen_th12.c
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
#include "rx_screen_th12.h"

#ifdef linux
	#include "errno.h"
#endif

//--- TH12 -------------------------------------------------------------------------
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
} TH12;


//--- Place -------------------------------------------------------------------------
typedef struct
{
	BYTE		row;		// where to put the value in X direction
	BYTE		line;		// where to put the value in Y direction
} Place;


//--- Globals -------------------------------------------------------------------------
static TH12			TH12_13[13];
static TH12			TabTH12[256];
static BYTE			Tab256x4x4[256][4][4];

static BYTE			Val256x4x4x2[256][4][4][2]; 

//--- _bmp_plane_TH12screen ----------------------------------------------------------------------
int bmp_plane_TH12screen(SHORT plane, SBmpInfo *pinfo, SScreenConfig* pscreenConfig)
{
	UINT32 l, NewLineLen;
	int i, modulol4, modulol2;
	BYTE *pSrc, *pDst, *pSrcBis, *pDstBis, *pNew = NULL;
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
		modulol4 = l % 4;
		modulol2 = l % 2;
		pDst = pNew + (l * 2 * NewLineLen);
		pDstBis = pDst + NewLineLen;
		for (i = 0; i+1 < (int)pinfo->srcWidthPx; i+=2)
		{
			if (pSrc[i]||pSrc[i+1])
			{
				*pDst++ = Val256x4x4x2[lut[pSrc[i]]][modulol4][i % 4][0] | Val256x4x4x2[lut[pSrc[i+1]]][modulol4][(i+1) % 4][0];
				*pDstBis++ = Val256x4x4x2[lut[pSrc[i]]][modulol4][i % 4][1] | Val256x4x4x2[lut[pSrc[i+1]]][modulol4][(i+1) % 4][1];
			}
			else
			{
				pDst++; pDstBis++;
			}
		}
		if ((i + 1 == (int)pinfo->srcWidthPx) && (pSrc[i])) {
			*pDst++ = Val256x4x4x2[lut[pSrc[i]]][modulol4][i % 4][0];
			*pDstBis++ = Val256x4x4x2[lut[pSrc[i]]][modulol4][i % 4][1];
		}
	}

	rx_mem_free(pinfo->buffer[plane]);
	*pinfo->buffer[plane] = pNew;

	return (REPLY_OK);
}

//--- _bmp_plane_TH12screen1 ----------------------------------------------------------------------
int bmp_plane_TH12screen1(SHORT plane, SBmpInfo *pinfo, SScreenConfig* pscreenConfig)
{
	UINT32 l, NewLineLen;
	int i, modulol4, modulol2 ;
	BYTE *pSrc, *pDst, *pSrcBis, *pDstBis, *pNew = NULL;
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
		modulol4 = l % 4;
		modulol2 = l % 2;
		pDst = pNew + (l * 2 * NewLineLen);
		pDstBis = pDst + NewLineLen;
		for (i = 0; i < (int)pinfo->srcWidthPx; i++)
		{
//			static BYTE memo = 0; // for debug if needed
//			if (pSrc[i] != memo)
//				memo = pSrc[i];
			if (pSrc[i])
			{
				if (!(i % 2))	// pixel even 
				{
					*pDst = Val256x4x4x2[lut[pSrc[i]]][modulol4][i % 4][0];
					*pDstBis = Val256x4x4x2[lut[pSrc[i]]][modulol4][i % 4][1];
				}
				else
				{
					*pDst++ |= Val256x4x4x2[lut[pSrc[i]]][modulol4][i % 4][0];
					*pDstBis++ |= Val256x4x4x2[lut[pSrc[i]]][modulol4][i % 4][1];
				}
	/*

				Src = Tab256x4x4[lut[pSrc[i]]][modulol4][i % 4]; // set value
				if (!modulol2)
				{
					if (!(i % 2))	// line even, pixel even 
					{
						*pDst = TH12_13[Src].EL1_L;
						*pDstBis = TH12_13[Src].EL2_L;
					}
					else           // line even, pixel odd 
					{
						*pDst |= TH12_13[Src].EL1_R;
						*pDstBis |= TH12_13[Src].EL2_R;
						pDst++; pDstBis++;
					}
				}
				else
				{
					if (!(i % 2))	// line odd, pixel even 
					{
						*pDst = TH12_13[Src].OL1_L;
						*pDstBis = TH12_13[Src].OL2_L;
					}
					else           // line odd, pixel odd 
					{
						*pDst |= TH12_13[Src].OL1_R;
						*pDstBis |= TH12_13[Src].OL2_R;
						pDst++; pDstBis++;
					}
				}
	*/
			}
			else
			{
				if (i % 2)
				{
					pDst++; pDstBis++;
				}
			}
		}
	}

	rx_mem_free(pinfo->buffer[plane]);
	*pinfo->buffer[plane] = pNew;

	return (REPLY_OK);
}



//--- _rx_screen_TH12init ----------------------------------------------------------------------
int rx_screen_TH12init(SScreenConfig* pscreenConfig)
{
	Place			Range[16];
	//static BYTE		Tab4x4[4][4];
	BYTE			Tab16x4x4[16][4][4];
	BYTE			Tab193x4x4[193][4][4];
	int line, row, val, val2, i;

	sprintf(pscreenConfig->typeName, "TH12xg");
	pscreenConfig->inputResol.x = pscreenConfig->inputResol.y = 600;
	pscreenConfig->outputResol.x = pscreenConfig->outputResol.y = 1200;

		// init TH12 13 Tab
	memset(TH12_13, 0, sizeof (TH12)* 13);
	TH12_13[1].EL1_R = 0x4;		TH12_13[2].EL1_R = 0x8;		TH12_13[3].EL1_R = 0xC;
	TH12_13[1].EL2_R = 0x0; 	TH12_13[2].EL2_R = 0x0;		TH12_13[3].EL2_R = 0x0;
	TH12_13[1].OL1_R = 0x1;		TH12_13[2].OL1_R = 0x2;		TH12_13[3].OL1_R = 0x3;
	TH12_13[1].OL2_R = 0x0;		TH12_13[2].OL2_R = 0x0;		TH12_13[3].OL2_R = 0x0;

	TH12_13[4].EL1_R = 0xC;		TH12_13[5].EL1_R = 0xC;		TH12_13[6].EL1_R = 0xC;
	TH12_13[4].EL2_R = 0x1; 	TH12_13[5].EL2_R = 0x2;		TH12_13[6].EL2_R = 0x3;
	TH12_13[4].OL1_R = 0x3;		TH12_13[5].OL1_R = 0x3;		TH12_13[6].OL1_R = 0x3;
	TH12_13[4].OL2_R = 0x4;		TH12_13[5].OL2_R = 0x8;		TH12_13[6].OL2_R = 0xC;

	TH12_13[7].EL1_R = 0xC;		TH12_13[8].EL1_R = 0xC;		TH12_13[9].EL1_R = 0xC;
	TH12_13[7].EL2_R = 0x7; 	TH12_13[8].EL2_R = 0xB;		TH12_13[9].EL2_R = 0xF;
	TH12_13[7].OL1_R = 0x3;		TH12_13[8].OL1_R = 0x3;		TH12_13[9].OL1_R = 0x3;
	TH12_13[7].OL2_R = 0xD;		TH12_13[8].OL2_R = 0xE;		TH12_13[9].OL2_R = 0xF;

	TH12_13[10].EL1_R = 0xD;	TH12_13[11].EL1_R = 0xE;	TH12_13[12].EL1_R = 0xF;
	TH12_13[10].EL2_R = 0xF; 	TH12_13[11].EL2_R = 0xF;	TH12_13[12].EL2_R = 0xF;
	TH12_13[10].OL1_R = 0x7;	TH12_13[11].OL1_R = 0xB;	TH12_13[12].OL1_R = 0xF;
	TH12_13[10].OL2_R = 0xF;	TH12_13[11].OL2_R = 0xF;	TH12_13[12].OL2_R = 0xF;
	for (i = 1; i < 13; i++)
	{
		TH12_13[i].EL1_L = TH12_13[i].EL1_R << 4;
		TH12_13[i].EL2_L = TH12_13[i].EL2_R << 4;
		TH12_13[i].OL1_L = TH12_13[i].OL1_R << 4;
		TH12_13[i].OL2_L = TH12_13[i].OL2_R << 4;
	}

	/*
	Tab4x4[0][0] = 1;	Tab4x4[2][2] = 2;	Tab4x4[0][2] = 3;	Tab4x4[2][0] = 4;
	Tab4x4[1][1] = 5;	Tab4x4[3][3] = 6;	Tab4x4[1][3] = 7;	Tab4x4[3][1] = 8;
	Tab4x4[0][1] = 9;	Tab4x4[2][3] = 10;	Tab4x4[0][3] = 11;	Tab4x4[2][1] = 12;
	Tab4x4[1][0] = 13;	Tab4x4[3][2] = 14;	Tab4x4[1][2] = 15;	Tab4x4[3][0] = 16;
	*/

	Range[0].row = 0;	 Range[0].line = 0;  Range[1].row = 2;  Range[1].line = 2;    Range[2].row= 0;  Range[2].line = 2;  Range[3].row = 2;  Range[3].line = 0;
	Range[4].row = 1;	 Range[4].line = 1;  Range[5].row = 3;  Range[5].line = 3;    Range[6].row= 1;  Range[6].line = 3;  Range[7].row = 3;  Range[7].line = 1;
	Range[8].row = 0;	 Range[8].line = 1;  Range[9].row = 2;  Range[9].line = 3;   Range[10].row= 0; Range[10].line = 3; Range[11].row = 2; Range[11].line = 1;
	Range[12].row = 1;  Range[12].line = 0; Range[13].row = 3; Range[13].line = 2;   Range[14].row= 1; Range[14].line = 2; Range[15].row = 3; Range[15].line = 0;

	// Clear Tab16x4x4
	for (val = 0; val < 16; val++) {
		for (line = 0; line < 4; line++) {
			for (row = 0; row < 4; row++)
				Tab16x4x4[val][line][row] = 0;
		}
	}

	// Filling order for 16 steps 
	for (val = 0; val < 16; val++) {
		for (val2 = val; val2 <16; val2++)
			Tab16x4x4[val2][Range[val].line][Range[val].row] = 1;
	}

	// Clear Tab193x4x4
	for (val = 0; val < 193; val++) {
		for (line = 0; line < 4; line++) {
			for (row = 0; row < 4; row++)
				Tab193x4x4[val][line][row] = 0;
		}
	}

	// prepare per steps
	for (val = 1; val < 12; val++) {
		for (val2 = val * 16; val2 < 192; val2++) {
			for (line = 0; line < 4; line++) {
				for (row = 0; row < 4; row++)
					Tab193x4x4[val2 + 1][line][row] += 1;
			}
		}
	}

	// Add 12 grey levels for each steps
	for (val = 0; val < 192; val++) {
		for (line = 0; line < 4; line++) {
			for (row = 0; row < 4; row++)
				Tab193x4x4[val + 1][line][row] += Tab16x4x4[val % 16][line][row];
		}
	}

	// Clear Tab256x4x4
	for (val = 0; val < 256; val++) {
		for (line = 0; line < 4; line++) {
			for (row = 0; row < 4; row++)
				Tab256x4x4[val][line][row] = 0;
		}
	}

	// Tab193 to Tab256 (could use lut table here)
	for (val = 0; val < 256; val++) {
		val2 = (int)((192.0 * val / 255.0) + 0.49999);
		for (line = 0; line < 4; line++) {
			for (row = 0; row < 4; row++)
				Tab256x4x4[val][line][row] = Tab193x4x4[val2][line][row];
		}
	}


	// Tab256 to Val256 (could use lut table here)
	for (val = 0; val < 256; val++) {
		for (line = 0; line < 4; line++) {
			for (row = 0; row < 4; row++) {
				if (!(line % 2)) {
					if (!(row % 2)) {		// line even row even
						Val256x4x4x2[val][line][row][0] = TH12_13[Tab256x4x4[val][line][row]].EL1_L;
						Val256x4x4x2[val][line][row][1] = TH12_13[Tab256x4x4[val][line][row]].EL2_L;
					}
					else{				// line even row odd
						Val256x4x4x2[val][line][row][0] = TH12_13[Tab256x4x4[val][line][row]].EL1_R;
						Val256x4x4x2[val][line][row][1] = TH12_13[Tab256x4x4[val][line][row]].EL2_R;
					}
				}
				else {
					if (!(row % 2)) {		// line odd row even
						Val256x4x4x2[val][line][row][0] = TH12_13[Tab256x4x4[val][line][row]].OL1_L;
						Val256x4x4x2[val][line][row][1] = TH12_13[Tab256x4x4[val][line][row]].OL2_L;
					}
					else{				// line odd row odd
						Val256x4x4x2[val][line][row][0] = TH12_13[Tab256x4x4[val][line][row]].OL1_R;
						Val256x4x4x2[val][line][row][1] = TH12_13[Tab256x4x4[val][line][row]].OL2_R;
					}
				}
			}
		}
	}

	return (REPLY_OK);
}