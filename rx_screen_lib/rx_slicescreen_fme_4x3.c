// ****************************************************************************
//
//	rx_slicescreen_fme_4x3.c
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
#include "rx_slicescreen_fme_4x3.h"


#ifdef linux
	#include "errno.h"
#endif

int NoiseDone = 0;
UINT16 NoiseTable[65536];

int RandDone = 0;
UINT16 RandTable[65536];



//--- Globals -------------------------------------------------------------------------

static BYTE			Val4[4];

static int _Abort=FALSE;

//--- rx_screen_slice_FME_1x1 ----------------------------------------------------------------------
int rx_screen_slice_FME_1x1(SSLiceInfo *inplane, SSLiceInfo *outplane, void * epplaneScreenConfig)
{
	SPlaneScreenConfig* pplaneScreenConfig = epplaneScreenConfig;
	UINT32 l;
	int i, valError, distError;
	int rindex = (rand() % 256) * 257;
	int * pError, *pLineError;
	BYTE *pSrc, *pDst;
	UINT16 *pLine = NULL, *pWNewSrc;

	if (rx_screen_outslice_init(inplane, outplane, pplaneScreenConfig))
		return REPLY_ERROR;

	// alloc line for data
	pLine = (UINT16 *)rx_mem_alloc(inplane->WidthPx * sizeof(UINT16));
	if (pLine == NULL) return Error(ERR_CONT, 0, "No buffer line for screening: %d");
	pWNewSrc = pLine;

	// alloc line for error
	pError = pLineError = (int *)rx_mem_alloc((inplane->WidthPx) * sizeof(int));
	if (pLineError == NULL) {
		rx_mem_free((BYTE**)&pLine);
		return Error(ERR_CONT, 0, "No buffer line for screening: %d");
	}
	memset(pLineError, 0, (inplane->WidthPx) * sizeof(int));

	for (l = 0; l < inplane->lengthPx; l++)
	{

		// from 8 bits
			pSrc = (BYTE *)(inplane->buffer + (l * inplane->lineLen));
			for (i = 0; i < (int)inplane->WidthPx; i++)
				pWNewSrc[i] = pSrc[i] * 257;

		// add error from previous line if any
		for (i = 0; i < (int)inplane->WidthPx; i++)
		{
			if (pError[i]) {
				if (pWNewSrc[i]) {
					if (pError[i] > 0) {
						if (pError[i] + pWNewSrc[i] > 65535)
							pWNewSrc[i] = 65535;
						else
							pWNewSrc[i] = (UINT16)(pError[i] + pWNewSrc[i]);
					}
					else if (pError[i] < 0) {
						if (pError[i] + pWNewSrc[i] < 0)
							pWNewSrc[i] = 0;
						else
							pWNewSrc[i] = (UINT16)(pError[i] + pWNewSrc[i]);
					}
				}
				pError[i] = 0;
			}
		}

		// F&S even line
		if ((l % 2) == 0) {
			pDst = outplane->buffer + (l * outplane->lineLen);

			for (i = 0; i < (int)inplane->WidthPx; i++)
			{
				if (pWNewSrc[i]) {

					// drop and calculate error to remove
					if (pWNewSrc[i] > NoiseTable[rindex++&(0xFFFF)]) {
						valError = (int)pWNewSrc[i] - 65535;
						switch (i % 4) {
						case 0: *pDst = Val4[0]; break;
						case 1: *pDst |= Val4[1]; break;
						case 2: *pDst |= Val4[2]; break;
						case 3: *pDst |= Val4[3]; break;
						}
					}

					// no drop and calculate error to add
					else {
						valError = pWNewSrc[i];
					}

					if (valError) { // distribute error
						valError /= 16;

						// distribute 7/16 at right
						if (i + 1 < (int)inplane->WidthPx) {
							if (pWNewSrc[i + 1]) {
								distError = valError * 7;
								if (distError > 0) {
									if (distError + pWNewSrc[i + 1] > 65535)
										pWNewSrc[i + 1] = 65535;
									else
										pWNewSrc[i + 1] = (UINT16)(distError + pWNewSrc[i + 1]);
								}
								else {
									if (distError + pWNewSrc[i] < 0)
										pWNewSrc[i + 1] = 0;
									else
										pWNewSrc[i + 1] = (UINT16)(distError + pWNewSrc[i + 1]);
								}
							}

							// distribute 1/16 next line right
							pError[i + 1] += valError;
						}

						// distribute 3/16 next line left
						if (i > 0) {
							pError[i - 1] += valError * 3;
						}

						// distribute 5/16 next line
						pError[i] += valError * 5;
					}
				}
				else
					pError[i] = 0;

				if ((i % 4) == 3)
					pDst++;
			}
		}

		// F&S odd line
		else {
			pDst = outplane->buffer + ((l + 1) * outplane->lineLen) - 1;
			for (i = (int)inplane->WidthPx - 1; i > 0; i--)
			{
				if (pWNewSrc[i]) {

					// drop and calculate error to remove
					if (pWNewSrc[i] > NoiseTable[rindex++&(0xFFFF)]) {
						valError = (int)pWNewSrc[i] - 65535;
						switch (i % 4) {
						case 0: *pDst |= Val4[0]; break;
						case 1: *pDst |= Val4[1]; break;
						case 2: *pDst |= Val4[2]; break;
						case 3: *pDst = Val4[3]; break;
						}
					}

					// no drop and calculate error to add
					else {
						valError = pWNewSrc[i];
					}

					if (valError) { // distribute error
						valError /= 16;

						// distribute 7/16 at left
						if (i - 1 > 0) {
							if (pWNewSrc[i - 1]) {
								distError = valError * 7;
								if (distError > 0) {
									if (distError + pWNewSrc[i - 1] > 65535)
										pWNewSrc[i - 1] = 65535;
									else
										pWNewSrc[i - 1] = (UINT16)(distError + pWNewSrc[i - 1]);
								}
								else {
									if (distError + pWNewSrc[i] < 0)
										pWNewSrc[i - 1] = 0;
									else
										pWNewSrc[i - 1] = (UINT16)(distError + pWNewSrc[i - 1]);
								}
							}

							// distribute 1/16 next line left
							pError[i - 1] += valError;
						}

						// distribute 3/16 next line right
						if (i < (int)inplane->WidthPx - 1) {
							pError[i + 1] += valError * 3;
						}

						// distribute 5/16 next line
						pError[i] += valError * 5;
					}
				}
				else
					pError[i] = 0;

				if ((i % 4) == 0)
					pDst--;
			}
		}
	}


	if (pLine)
		rx_mem_free((BYTE**)&pLine);
	if (pLineError)
		rx_mem_free((BYTE**)&pLineError);

	return (REPLY_OK);
}


//--- rx_planescreen_init_FME_1x1 ----------------------------------------------------------------------
int rx_planescreen_init_FME_1x1(SPlaneScreenConfig* pplaneScreenConfig)
{
	int i;
	sprintf(pplaneScreenConfig->typeName, "FMS_1x1");
	pplaneScreenConfig->fctplane = &rx_screen_slice_FME_1x1;
	pplaneScreenConfig->fctclose = &rx_planescreen_close_null;
	if (pplaneScreenConfig->dropsize == 1) {
		Val4[0] = 0x40;
		Val4[1] = 0x10;
		Val4[2] = 0x04;
		Val4[3] = 0x01;
	}
	else if (pplaneScreenConfig->dropsize == 2) {
		Val4[0] = 0x80;
		Val4[1] = 0x20;
		Val4[2] = 0x08;
		Val4[3] = 0x02;
	}
	else
	{
		Val4[0] = 0xC0;
		Val4[1] = 0x30;
		Val4[2] = 0x0C;
		Val4[3] = 0x03;
	}

	if (!NoiseDone)
	{
		NoiseDone= 1;
		if (pplaneScreenConfig->noise == 0)
			for (i = 0; i < 65536; i++) {
				NoiseTable[i] = 32767;
			}
		else {
			int noise = pplaneScreenConfig->noise;
			if (noise > 100) // noise max : 100%
				noise = 100;
			if (noise < 2) // min noise : 2% (+1 % - 1 %)
				noise = 2;

			for (i = 0; i < 65536; i++) {
				NoiseTable[i] = ((rand() % 256) * 256) + (rand() % 256);
				if (NoiseTable[i] > 32767)
					NoiseTable[i] = 32768 + ((NoiseTable[i] - 32768) * noise / 200);
				else
					NoiseTable[i] = 32768 - ((32768 - NoiseTable[i]) * noise / 200);
			}
		}
	}

	return (REPLY_OK);
}


//--- rx_screen_slice_FME_1x2g ----------------------------------------------------------------------
int rx_screen_slice_FME_1x2g(SSLiceInfo *inplane, SSLiceInfo *outplane, void * epplaneScreenConfig)
{
	SPlaneScreenConfig* pplaneScreenConfig = epplaneScreenConfig;
	UINT32 l;
	int i, valError, distError, ret = REPLY_OK;
	int nindex = (rand() % 256) * 257;
	int rindex = (rand() % 256) * 257;
	int * pError, *pLineError, *LimitTable;
	BYTE *pSrc, *pDst, *pLineLim, Val1 = 1, Val2 = 2;
	UINT16 *pLine = NULL, *pWNewSrc, WLimit = 32768, WT2 = 32768;


	if (rx_screen_outslice_init(inplane, outplane, pplaneScreenConfig))
		return REPLY_ERROR;

	if (pplaneScreenConfig->dropsize == 13)
		Val2 = 3;
	else if (pplaneScreenConfig->dropsize == 23) {
		Val1 = 2;
		Val2 = 3;
	}
	if (pplaneScreenConfig->limit[0]) {
		WLimit = 65536 * pplaneScreenConfig->limit[0] / 100;
		WT2 = 65536 - WLimit;
		if (!WT2) WT2 = 1;
	}
	// alloc LimitTable1
	LimitTable = (int *)rx_mem_alloc(65536 * sizeof(int));
	if (LimitTable == NULL) {
		ret = Error(ERR_CONT, 0, "No buffer line for screening: %d"); goto End;
	}
	for (i = 0; i < 65536; i++) {
		LimitTable[i] = WLimit - 1 + (RandTable[i] * WT2 / 65536);
	}

	// alloc line for data
	pLine = (UINT16 *)rx_mem_alloc(inplane->WidthPx * sizeof(UINT16));
	if (pLine == NULL) {
		ret = Error(ERR_CONT, 0, "No buffer line for screening: %d"); goto End;
	}
	pWNewSrc = pLine;

	// alloc line for error
	pError = pLineError = (int *)rx_mem_alloc((inplane->WidthPx) * sizeof(int));
	if (pLineError == NULL) {
		ret = Error(ERR_CONT, 0, "No buffer line for screening: %d"); goto End;
	}
	memset(pLineError, 0, (inplane->WidthPx) * sizeof(int));

	// alloc line for limit
	pLineLim = (BYTE *)rx_mem_alloc((outplane->WidthPx) * sizeof(BYTE));
	if (pLineLim == NULL) {
		ret = Error(ERR_CONT, 0, "No buffer line for screening: %d"); goto End;
	}

	for (l = 0; l < inplane->lengthPx; l++)
	{

		// from 8 bits
			pSrc = (BYTE *)(inplane->buffer + (l * inplane->lineLen));
			for (i = 0; i < (int)inplane->WidthPx; i++)
				pWNewSrc[i] = pSrc[i] * 257;

		// add error from previous line if any
		for (i = 0; i < (int)inplane->WidthPx; i++)
		{
			pLineLim[i] = Val1;
			if (pWNewSrc[i] > WLimit)
				if (pWNewSrc[i] >= LimitTable[rindex++&(0xFFFF)]) {
					pLineLim[i] = Val2;
				}

			if (pError[i]) {
				if (pWNewSrc[i]) {
					if (pError[i] > 0) {
						if (pError[i] + pWNewSrc[i] > 65535)
							pWNewSrc[i] = 65535;
						else
							pWNewSrc[i] = (UINT16)(pError[i] + pWNewSrc[i]);
					}
					else if (pError[i] < 0) {
						if (pError[i] + pWNewSrc[i] < 0)
							pWNewSrc[i] = 0;
						else
							pWNewSrc[i] = (UINT16)(pError[i] + pWNewSrc[i]);
					}
				}
				pError[i] = 0;
			}
		}

		// F&S even line
		if ((l % 2) == 0) {
			pDst = outplane->buffer + (l * outplane->lineLen);

			for (i = 0; i < (int)inplane->WidthPx; i++)
			{
				if (pWNewSrc[i]) {

					// drop and calculate error to remove
					if (pWNewSrc[i] > NoiseTable[nindex++&(0xFFFF)]) {
						valError = (int)pWNewSrc[i] - 65535;

						switch (i % 4) {
						case 0: *pDst = pLineLim[i] << 6; break;
						case 1: *pDst |= pLineLim[i] << 4; break;
						case 2: *pDst |= pLineLim[i] << 2; break;
						case 3: *pDst |= pLineLim[i]; break;
						}
					}

					// no drop and calculate error to add
					else {
						valError = pWNewSrc[i];
					}

					if (valError) { // distribute error
						valError /= 16;

						// distribute 7/16 at right
						if (i + 1 < (int)inplane->WidthPx) {
							if (pWNewSrc[i + 1]) {
								distError = valError * 7;
								if (distError > 0) {
									if (distError + pWNewSrc[i + 1] > 65535)
										pWNewSrc[i + 1] = 65535;
									else
										pWNewSrc[i + 1] = (UINT16)(distError + pWNewSrc[i + 1]);
								}
								else {
									if (distError + pWNewSrc[i] < 0)
										pWNewSrc[i + 1] = 0;
									else
										pWNewSrc[i + 1] = (UINT16)(distError + pWNewSrc[i + 1]);
								}
							}

							// distribute 1/16 next line right
							pError[i + 1] += valError;
						}

						// distribute 3/16 next line left
						if (i > 0) {
							pError[i - 1] += valError * 3;
						}

						// distribute 5/16 next line
						pError[i] += valError * 5;
					}
				}
				else
					pError[i] = 0;

				if ((i % 4) == 3)
					pDst++;
			}
		}

		// F&S odd line
		else {
			pDst = outplane->buffer + ((l + 1) * outplane->lineLen) - 1;
			for (i = (int)inplane->WidthPx - 1; i > 0; i--)
			{
				if (pWNewSrc[i]) {

					// drop and calculate error to remove
					if (pWNewSrc[i] > NoiseTable[nindex++&(0xFFFF)]) {
						valError = (int)pWNewSrc[i] - 65535;

						switch (i % 4) {
						case 0: *pDst |= pLineLim[i] << 6; break;
						case 1: *pDst |= pLineLim[i] << 4; break;
						case 2: *pDst |= pLineLim[i] << 2; break;
						case 3: *pDst = pLineLim[i]; break;
						}
					}

					// no drop and calculate error to add
					else {
						valError = pWNewSrc[i];
					}

					if (valError) { // distribute error
						valError /= 16;

						// distribute 7/16 at left
						if (i - 1 > 0) {
							if (pWNewSrc[i - 1]) {
								distError = valError * 7;
								if (distError > 0) {
									if (distError + pWNewSrc[i - 1] > 65535)
										pWNewSrc[i - 1] = 65535;
									else
										pWNewSrc[i - 1] = (UINT16)(distError + pWNewSrc[i - 1]);
								}
								else {
									if (distError + pWNewSrc[i] < 0)
										pWNewSrc[i - 1] = 0;
									else
										pWNewSrc[i - 1] = (UINT16)(distError + pWNewSrc[i - 1]);
								}
							}

							// distribute 1/16 next line left
							pError[i - 1] += valError;
						}

						// distribute 3/16 next line right
						if (i < (int)inplane->WidthPx - 1) {
							pError[i + 1] += valError * 3;
						}

						// distribute 5/16 next line
						pError[i] += valError * 5;
					}
				}
				else
					pError[i] = 0;

				if ((i % 4) == 0)
					pDst--;
			}
		}
	}

End:
	if (LimitTable)
		rx_mem_free((BYTE**)&LimitTable);
	if (pLine)
		rx_mem_free((BYTE**)&pLine);
	if (pLineError)
		rx_mem_free((BYTE**)&pLineError);
	if (pLineLim)
		rx_mem_free((BYTE**)&pLineLim);

	return (ret);
}


//--- rx_screen_init_FME_1x2g ----------------------------------------------------------------------
int rx_planescreen_init_FME_1x2g(SPlaneScreenConfig* pplaneScreenConfig)
{
	int i;
	sprintf(pplaneScreenConfig->typeName, "FME_1x2g");
	pplaneScreenConfig->fctplane = &rx_screen_slice_FME_1x2g;
	pplaneScreenConfig->fctclose = &rx_planescreen_close_null;

	if (!RandDone)
	{
		RandDone = 1;
		for (i = 0; i < 65536; i++) {
			RandTable[i] = ((rand() % 256) * 256) + (rand() % 256);
		}
	}

	if (!NoiseDone)
	{
		NoiseDone = 1;
		if (pplaneScreenConfig->noise == 0)
			for (i = 0; i < 65536; i++) {
				NoiseTable[i] = 32767;
			}
		else {
			int noise = pplaneScreenConfig->noise;
			if (noise > 100) // noise max : 100%
				noise = 100;
			if (noise < 2) // min noise : 2% (+1 % - 1 %)
				noise = 2;

			for (i = 0; i < 65536; i++) {
				if (RandTable[i] > 32767)
					NoiseTable[i] = 32768 + ((RandTable[i] - 32768) * noise / 200);
				else
					NoiseTable[i] = 32768 - ((32768 - RandTable[i]) * noise / 200);
			}
		}
	}

	return (REPLY_OK);
}


int rx_screen_slice_FME_1x3g(SSLiceInfo *inplane, SSLiceInfo *outplane, void * epplaneScreenConfig)
{
	SPlaneScreenConfig* pplaneScreenConfig = epplaneScreenConfig;
	UINT32 l;
	int cpt, i, valError, distError, ret = REPLY_OK;
	int nindex = (rand() % 256) * 257;
	int rindex = (rand() % 256) * 257;
	int * pError, *pLineError, *LimitTable1, *LimitTable2;
	BYTE *pSrc, *pDst, *pLineLim;
	UINT16*pLine = NULL, *pWNewSrc, WLimit1 = 21845, WLimit2 = 43690, WT1 = 21845, WT2 = 21845, WT3 = 21845;
	UINT16 *lut16 = pplaneScreenConfig->lut16;

	if (rx_screen_outslice_init(inplane, outplane, pplaneScreenConfig))
		return REPLY_ERROR;

	if (pplaneScreenConfig->limit[0]) {
		WT1 = WLimit1 = 65536 * pplaneScreenConfig->limit[0] / 100;
		if (!WT1) WT1 = 1;
		WLimit2 = 65536 * pplaneScreenConfig->limit[1] / 100;
		WT3 = 65536 - WLimit2;
		if (!WT3) WT3 = 1;
		WT2 = 65536 - (WT1 + WT3);
		if (!WT2) WT2 = 1;
	}

	// alloc LimitTable1
	LimitTable1 = (int *)rx_mem_alloc(65536 * sizeof(int));
	if (LimitTable1 == NULL) {
		ret = Error(ERR_CONT, 0, "No buffer line for screening: %d"); goto End;
	}
	LimitTable2 = (int *)rx_mem_alloc(65536 * sizeof(int));
	if (LimitTable2 == NULL) {
		ret = Error(ERR_CONT, 0, "No buffer line for screening: %d"); goto End;
	}
	for (cpt = 0; cpt < 65536; cpt++) {
		LimitTable1[cpt] = WLimit1 - 1 + (RandTable[cpt] * WT2 / 65536);
		LimitTable2[cpt] = WLimit2 - 1 + (RandTable[cpt] * WT3 / 65536);
	}

	// alloc line for data
	pLine = (UINT16 *)rx_mem_alloc(inplane->WidthPx * sizeof(UINT16));
	if (pLine == NULL) {
		ret = Error(ERR_CONT, 0, "No buffer line for screening: %d"); goto End;
	}
	pWNewSrc = pLine;

	// alloc line for error
	pError = pLineError = (int *)rx_mem_alloc((inplane->WidthPx) * sizeof(int));
	if (pLineError == NULL) {
		ret = Error(ERR_CONT, 0, "No buffer line for screening: %d"); goto End;
	}
	memset(pLineError, 0, (inplane->WidthPx) * sizeof(int));

	// alloc line for limit
	pLineLim = (BYTE *)rx_mem_alloc((outplane->WidthPx) * sizeof(BYTE));
	if (pLineLim == NULL) {
		ret = Error(ERR_CONT, 0, "No buffer line for screening: %d"); goto End;
	}

	for (l = 0; l < inplane->lengthPx; l++)
	{


		//  from 8 bits
		pSrc = (BYTE *)(inplane->buffer + (l * inplane->lineLen));
		for (i = 0; i < (int)inplane->WidthPx; i++)
			pWNewSrc[i] = lut16[pSrc[i] * 257];

		// add error from previous line if any
		for (i = 0; i < (int)inplane->WidthPx; i++)
		{
			pLineLim[i] = 1;
			if (pWNewSrc[i] > WLimit2) {
				pLineLim[i] = 2;
				if (pWNewSrc[i] >= LimitTable2[rindex++&(0xFFFF)]) {
					pLineLim[i] = 3;
				}
			}
			else if (pWNewSrc[i] > WLimit1) {
				if (pWNewSrc[i] >= LimitTable1[rindex++&(0xFFFF)]) {
					pLineLim[i] = 2;
				}
			}

			if (pError[i]) {
				if (pWNewSrc[i]) {
					if (pError[i] > 0) {
						if (pError[i] + pWNewSrc[i] > 65535)
							pWNewSrc[i] = 65535;
						else
							pWNewSrc[i] = (UINT16)(pError[i] + pWNewSrc[i]);
					}
					else if (pError[i] < 0) {
						if (pError[i] + pWNewSrc[i] < 0)
							pWNewSrc[i] = 0;
						else
							pWNewSrc[i] = (UINT16)(pError[i] + pWNewSrc[i]);
					}
				}
				pError[i] = 0;
			}
		}

		// F&S even line
		if ((l % 2) == 0) {
			pDst = outplane->buffer + (l * outplane->lineLen);

			for (i = 0; i < (int)inplane->WidthPx; i++)
			{
				if (pWNewSrc[i]) {

					// drop and calculate error to remove
					if (pWNewSrc[i] > NoiseTable[nindex++&(0xFFFF)]) {
						valError = (int)pWNewSrc[i] - 65535;

						switch (i % 4) {
						case 0: *pDst = pLineLim[i] << 6; break;
						case 1: *pDst |= pLineLim[i] << 4; break;
						case 2: *pDst |= pLineLim[i] << 2; break;
						case 3: *pDst |= pLineLim[i]; break;
						}
					}

					// no drop and calculate error to add
					else {
						valError = pWNewSrc[i];
					}

					if (valError) { // distribute error
						valError /= 16;

						// distribute 7/16 at right
						if (i + 1 < (int)inplane->WidthPx) {
							if (pWNewSrc[i + 1]) {
								distError = valError * 7;
								if (distError > 0) {
									if (distError + pWNewSrc[i + 1] > 65535)
										pWNewSrc[i + 1] = 65535;
									else
										pWNewSrc[i + 1] = (UINT16)(distError + pWNewSrc[i + 1]);
								}
								else {
									if (distError + pWNewSrc[i] < 0)
										pWNewSrc[i + 1] = 0;
									else
										pWNewSrc[i + 1] = (UINT16)(distError + pWNewSrc[i + 1]);
								}
							}

							// distribute 1/16 next line right
							pError[i + 1] += valError;
						}

						// distribute 3/16 next line left
						if (i > 0) {
							pError[i - 1] += valError * 3;
						}

						// distribute 5/16 next line
						pError[i] += valError * 5;
					}
				}
				else
					pError[i] = 0;

				if ((i % 4) == 3)
					pDst++;
			}
		}

		// F&S odd line
		else {
			pDst = outplane->buffer + ((l + 1) * outplane->lineLen) - 1;
			for (i = (int)inplane->WidthPx - 1; i > 0; i--)
			{
				if (pWNewSrc[i]) {

					// drop and calculate error to remove
					if (pWNewSrc[i] > NoiseTable[nindex++&(0xFFFF)]) {
						valError = (int)pWNewSrc[i] - 65535;

						switch (i % 4) {
						case 0: *pDst |= pLineLim[i] << 6; break;
						case 1: *pDst |= pLineLim[i] << 4; break;
						case 2: *pDst |= pLineLim[i] << 2; break;
						case 3: *pDst = pLineLim[i]; break;
						}
					}

					// no drop and calculate error to add
					else {
						valError = pWNewSrc[i];
					}

					if (valError) { // distribute error
						valError /= 16;

						// distribute 7/16 at left
						if (i - 1 > 0) {
							if (pWNewSrc[i - 1]) {
								distError = valError * 7;
								if (distError > 0) {
									if (distError + pWNewSrc[i - 1] > 65535)
										pWNewSrc[i - 1] = 65535;
									else
										pWNewSrc[i - 1] = (UINT16)(distError + pWNewSrc[i - 1]);
								}
								else {
									if (distError + pWNewSrc[i] < 0)
										pWNewSrc[i - 1] = 0;
									else
										pWNewSrc[i - 1] = (UINT16)(distError + pWNewSrc[i - 1]);
								}
							}

							// distribute 1/16 next line left
							pError[i - 1] += valError;
						}

						// distribute 3/16 next line right
						if (i < (int)inplane->WidthPx - 1) {
							pError[i + 1] += valError * 3;
						}

						// distribute 5/16 next line
						pError[i] += valError * 5;
					}
				}
				else
					pError[i] = 0;

				if ((i % 4) == 0)
					pDst--;
			}
		}
	}

End:
	if (LimitTable1)
		rx_mem_free((BYTE**)&LimitTable1);
	if (LimitTable2)
		rx_mem_free((BYTE**)&LimitTable2);
	if (pLine)
		rx_mem_free((BYTE**)&pLine);
	if (pLineError)
		rx_mem_free((BYTE**)&pLineError);
	if (pLineLim)
		rx_mem_free((BYTE**)&pLineLim);

	return (ret);
}


//--- rx_planescreen_init_FME_1x3g ----------------------------------------------------------------------
int rx_planescreen_init_FME_1x3g(SPlaneScreenConfig* pplaneScreenConfig)
{
	int i;
	sprintf(pplaneScreenConfig->typeName, "FME_1x3g");
	pplaneScreenConfig->fctplane = &rx_screen_slice_FME_1x3g;
	pplaneScreenConfig->fctclose = &rx_planescreen_close_null;

	if (!RandDone)
	{
		RandDone = 1;
		for (i = 0; i < 65536; i++) {
			RandTable[i] = ((rand() % 256) * 256) + (rand() % 256);
		}
	}

	if (!NoiseDone)
	{
		NoiseDone = 1;
		if (pplaneScreenConfig->noise == 0)
			for (i = 0; i < 65536; i++) {
				NoiseTable[i] = 32767;
			}
		else {
			int noise = pplaneScreenConfig->noise;
			if (noise > 100) // noise max : 100%
				noise = 100;
			if (noise < 2) // min noise : 2% (+1 % - 1 %)
				noise = 2;

			for (i = 0; i < 65536; i++) {
				if (RandTable[i] > 32767)
					NoiseTable[i] = 32768 + ((RandTable[i] - 32768) * noise / 200);
				else
					NoiseTable[i] = 32768 - ((32768 - RandTable[i]) * noise / 200);
			}
		}
	}

	return (REPLY_OK);
}
