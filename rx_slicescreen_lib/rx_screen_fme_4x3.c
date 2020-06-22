// ****************************************************************************
//
//	rx_screen_fme_4x3.c
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
#include "rx_screen_fme_4x3.h"


#ifdef linux
	#include "errno.h"
#endif

int NoiseDone = 0;
UINT16 NoiseTable[65536];

int RandDone = 0;
UINT16 RandTable[65536];



//--- Globals -------------------------------------------------------------------------

static BYTE			Val4[4];

//--- rx_screen_plane_FME_1x1 ----------------------------------------------------------------------
int rx_screen_plane_FME_1x1(SPlaneInfo *inplane, SPlaneInfo *outplane, void* epscreenConfig)
{
	SScreenConfig* pscreenConfig = epscreenConfig;
	UINT32 l;
	int i, valError, distError;
	int rindex = (rand() % 256) * 257;
	int * pError, *pLineError;
	BYTE *pSrc, *pDst;
	UINT16 *pWSrc, *pLine=NULL, *pWNewSrc;
	UINT64 *pDropCount = &outplane->DropCount[0];
	UINT16 *lut16 = pscreenConfig->lut16[inplane->planenumber];

	if ((pscreenConfig->inputResol.x * 2 == pscreenConfig->outputResol.x) && (pscreenConfig->inputResol.y * 2 == pscreenConfig->outputResol.y)) {
			return (rx_screen_plane_FME_1x1x2(inplane, outplane, pscreenConfig));
		}
	else if ((pscreenConfig->inputResol.x * 4 == pscreenConfig->outputResol.x) && (pscreenConfig->inputResol.y * 4 == pscreenConfig->outputResol.y)) {
		return (rx_screen_plane_FME_1x1x4(inplane, outplane, pscreenConfig));
	}

	if (rx_screen_outplane_init(inplane, outplane, pscreenConfig))
		return REPLY_ERROR;
	if ((pscreenConfig->dropsize > 1) && (pscreenConfig->dropsize < 4))
		pDropCount = &outplane->DropCount[pscreenConfig->dropsize - 1];

	// alloc line for data
	pLine = (UINT16 *)rx_mem_alloc(inplane->WidthPx* sizeof (UINT16));
	if (pLine == NULL) return Error(ERR_CONT, 0, "No buffer line for screening: %d");
	pWNewSrc = pLine;

	// alloc line for error
	pError= pLineError = (int *)rx_mem_alloc((inplane->WidthPx)*sizeof (int));
	if (pLineError == NULL) {
		rx_mem_free((BYTE**)&pLine);
		return Error(ERR_CONT, 0, "No buffer line for screening: %d");
	}
	memset (pLineError, 0, (inplane->WidthPx) * sizeof(int));
	
	for (l = 0; l < inplane->lengthPx; l++)
	{

		// lut table from 16 bits
		if (inplane->bitsPerPixel == 16) {
			pWSrc = (UINT16 *)(inplane->buffer + (l * inplane->lineLen));
			for (i = 0; i < (int)inplane->WidthPx; i++)
				pWNewSrc[i] = lut16[pWSrc[i]];
		}

		// lut table from 8 bits
		else {
			pSrc = (BYTE *)(inplane->buffer + (l * inplane->lineLen));
			for (i = 0; i < (int)inplane->WidthPx; i++)
				pWNewSrc[i] = lut16[pSrc[i]*257];
		}
			
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
						(*pDropCount)++;
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
			pDst = outplane->buffer + ((l +1) * outplane->lineLen) -1;
			for (i = (int)inplane->WidthPx - 1; i > 0; i--)
			{
				if (pWNewSrc[i]) {

					// drop and calculate error to remove
					if (pWNewSrc[i] > NoiseTable[rindex++&(0xFFFF)]) {
						valError = (int)pWNewSrc[i] - 65535;
						(*pDropCount)++;
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


//--- rx_screen_plane_FME_1x1x2 ----------------------------------------------------------------------
int rx_screen_plane_FME_1x1x2(SPlaneInfo *inplane, SPlaneInfo *outplane, void* epscreenConfig)
{
	SScreenConfig* pscreenConfig = epscreenConfig;
	UINT32 li, lo;
	int pi, po, valError, distError;
	int rindex = (rand() % 256) * 257;
	int * pError, *pLineError;
	BYTE *pSrc, *pDst;
	UINT16 *pWSrc, *pLine = NULL, *pWNewSrc;
	UINT64 *pDropCount = &outplane->DropCount[0];
	UINT16 *lut16 = pscreenConfig->lut16[inplane->planenumber];


	if ((pscreenConfig->inputResol.x * 4 == pscreenConfig->outputResol.x) && (pscreenConfig->inputResol.y * 4 == pscreenConfig->outputResol.y)) {
		return (rx_screen_plane_FME_1x1x4(inplane, outplane, pscreenConfig));
	}
		if (rx_screen_outplane_init(inplane, outplane, pscreenConfig))
			return REPLY_ERROR;
		if ((pscreenConfig->dropsize > 1) && (pscreenConfig->dropsize < 4))
			pDropCount = &outplane->DropCount[pscreenConfig->dropsize - 1];

		// alloc line for data
		pLine = (UINT16 *)rx_mem_alloc(outplane->WidthPx * sizeof(UINT16));
		if (pLine == NULL) return Error(ERR_CONT, 0, "No buffer line for screening: %d");
		pWNewSrc = pLine;

		// alloc line for error
		pError = pLineError = (int *)rx_mem_alloc((outplane->WidthPx) * sizeof(int));
		if (pLineError == NULL) {
			rx_mem_free((BYTE**)&pLine);
			return Error(ERR_CONT, 0, "No buffer line for screening: %d");
		}
		memset(pLineError, 0, (outplane->WidthPx) * sizeof(int));

		for (li = 0; li < inplane->lengthPx; li++)
		{
			for (lo = 2 * li; lo < (2 * li) + 2; lo++)
			{
				// lut table from 16 bits
				if (inplane->bitsPerPixel == 16) {
					pWSrc = (UINT16 *)(inplane->buffer + (li * inplane->lineLen));
					for (pi = 0; pi < (int)inplane->WidthPx; pi++)
						for (po = 2 * pi; po < (2 * pi) + 2; po++)
							pWNewSrc[po] = lut16[pWSrc[pi]];
				}

				// lut table from 8 bits
				else {
					pSrc = (BYTE *)(inplane->buffer + (li * inplane->lineLen));
					for (pi = 0; pi < (int)inplane->WidthPx; pi++)
						for (po = 2 * pi; po < (2 * pi) + 2; po++)
							pWNewSrc[po] = lut16[pSrc[pi] * 257];
				}

				// add error from previous line if any
				for (po = 0; po < (int)outplane->WidthPx; po++)
				{
					if (pError[po]) {
						if (pWNewSrc[po]) {
							if (pError[po] > 0) {
								if (pError[po] + pWNewSrc[po] > 65535)
									pWNewSrc[po] = 65535;
								else
									pWNewSrc[po] = (UINT16)(pError[po] + pWNewSrc[po]);
							}
							else if (pError[po] < 0) {
								if (pError[po] + pWNewSrc[po] < 0)
									pWNewSrc[po] = 0;
								else
									pWNewSrc[po] = (UINT16)(pError[po] + pWNewSrc[po]);
							}
						}
						pError[po] = 0;
					}
				}

				// 1 st line is F&S even line
				if ((lo % 2) == 0) {
					pDst = outplane->buffer + (lo * outplane->lineLen);

					for (po = 0; po < (int)outplane->WidthPx; po++)
					{
						if (pWNewSrc[po]) {

							// drop and calculate error to remove
							if (pWNewSrc[po] > NoiseTable[rindex++&(0xFFFF)]) {
								valError = (int)pWNewSrc[po] - 65535;
								(*pDropCount)++;
								switch (po % 4) {
								case 0: *pDst = Val4[0]; break;
								case 1: *pDst |= Val4[1]; break;
								case 2: *pDst |= Val4[2]; break;
								case 3: *pDst |= Val4[3]; break;
								}
							}

							// no drop and calculate error to add
							else {
								valError = pWNewSrc[po];
							}

							if (valError) { // distribute error
								valError /= 16;

								// distribute 7/16 at right
								if (po + 1 < (int)outplane->WidthPx) {
									if (pWNewSrc[po + 1]) {
										distError = valError * 7;
										if (distError > 0) {
											if (distError + pWNewSrc[po + 1] > 65535)
												pWNewSrc[po + 1] = 65535;
											else
												pWNewSrc[po + 1] = (UINT16)(distError + pWNewSrc[po + 1]);
										}
										else {
											if (distError + pWNewSrc[po] < 0)
												pWNewSrc[po + 1] = 0;
											else
												pWNewSrc[po + 1] = (UINT16)(distError + pWNewSrc[po + 1]);
										}
									}

									// distribute 1/16 next line right
									pError[po + 1] += valError;
								}

								// distribute 3/16 next line left
								if (po > 0) {
									pError[po - 1] += valError * 3;
								}

								// distribute 5/16 next line
								pError[po] += valError * 5;
							}
						}
						else
							pError[po] = 0;

						if ((po % 4) == 3)
							pDst++;
					}
				}

				// F&S odd line
				else {
					pDst = outplane->buffer + ((lo + 1) * outplane->lineLen) - 1;
					for (po = (int)outplane->WidthPx - 1; po > 0; po--)
					{
						if (pWNewSrc[po]) {

							// drop and calculate error to remove
							if (pWNewSrc[po] > NoiseTable[rindex++&(0xFFFF)]) {
								valError = (int)pWNewSrc[po] - 65535;
								(*pDropCount)++;
								switch (po % 4) {
								case 0: *pDst |= Val4[0]; break;
								case 1: *pDst |= Val4[1]; break;
								case 2: *pDst |= Val4[2]; break;
								case 3: *pDst = Val4[3]; break;
								}
							}

							// no drop and calculate error to add
							else {
								valError = pWNewSrc[po];
							}

							if (valError) { // distribute error
								valError /= 16;

								// distribute 7/16 at left
								if (po - 1 > 0) {
									if (pWNewSrc[po - 1]) {
										distError = valError * 7;
										if (distError > 0) {
											if (distError + pWNewSrc[po - 1] > 65535)
												pWNewSrc[po - 1] = 65535;
											else
												pWNewSrc[po - 1] = (UINT16)(distError + pWNewSrc[po - 1]);
										}
										else {
											if (distError + pWNewSrc[po] < 0)
												pWNewSrc[po - 1] = 0;
											else
												pWNewSrc[po - 1] = (UINT16)(distError + pWNewSrc[po - 1]);
										}
									}

									// distribute 1/16 next line left
									pError[po - 1] += valError;
								}

								// distribute 3/16 next line right
								if (po < (int)inplane->WidthPx - 1) {
									pError[po + 1] += valError * 3;
								}

								// distribute 5/16 next line
								pError[po] += valError * 5;
							}
						}
						else
							pError[po] = 0;

						if ((po % 4) == 0)
							pDst--;
					}
				}
			}
		}


		if (pLine)
			rx_mem_free((BYTE**)&pLine);
		if (pLineError)
			rx_mem_free((BYTE**)&pLineError);

		return (REPLY_OK);
	}


//--- rx_screen_plane_FME_1x1x4 ----------------------------------------------------------------------
int rx_screen_plane_FME_1x1x4(SPlaneInfo *inplane, SPlaneInfo *outplane, void* epscreenConfig)
{
	SScreenConfig* pscreenConfig = epscreenConfig;
	UINT32 li, lo;
	int pi, po, valError, distError;
	int rindex = (rand() % 256) * 257;
	int * pError, *pLineError;
	BYTE *pSrc, *pDst;
	UINT16 *pWSrc, *pLine = NULL, *pWNewSrc;
	UINT64 *pDropCount = &outplane->DropCount[0];
	UINT16 *lut16 = pscreenConfig->lut16[inplane->planenumber];

	if (rx_screen_outplane_init(inplane, outplane, pscreenConfig))
		return REPLY_ERROR;
	if ((pscreenConfig->dropsize > 1) && (pscreenConfig->dropsize < 4))
		pDropCount = &outplane->DropCount[pscreenConfig->dropsize - 1];

	// alloc line for data
	pLine = (UINT16 *)rx_mem_alloc(outplane->WidthPx * sizeof(UINT16));
	if (pLine == NULL) return Error(ERR_CONT, 0, "No buffer line for screening: %d");
	pWNewSrc = pLine;

	// alloc line for error
	pError = pLineError = (int *)rx_mem_alloc((outplane->WidthPx) * sizeof(int));
	if (pLineError == NULL) {
		rx_mem_free((BYTE**)&pLine);
		return Error(ERR_CONT, 0, "No buffer line for screening: %d");
	}
	memset(pLineError, 0, (outplane->WidthPx) * sizeof(int));

	for (li = 0; li < inplane->lengthPx; li++)
	{
		for (lo = 4 * li; lo < (4 * li) + 4; lo++)
		{
			// lut table from 16 bits
			if (inplane->bitsPerPixel == 16) {
				pWSrc = (UINT16 *)(inplane->buffer + (li * inplane->lineLen));
				for (pi = 0; pi < (int)inplane->WidthPx; pi++)
					for (po = 4 * pi; po < (4 * pi) + 4; po++)
						pWNewSrc[po] = lut16[pWSrc[pi]];
			}

			// lut table from 8 bits
			else {
				pSrc = (BYTE *)(inplane->buffer + (li * inplane->lineLen));
				for (pi = 0; pi < (int)inplane->WidthPx; pi++)
					for (po = 4 * pi; po < (4 * pi) + 4; po++)
						pWNewSrc[po] = lut16[pSrc[pi] * 257];
			}

			// add error from previous line if any
			for (po = 0; po < (int)outplane->WidthPx; po++)
			{
				if (pError[po]) {
					if (pWNewSrc[po]) {
						if (pError[po] > 0) {
							if (pError[po] + pWNewSrc[po] > 65535)
								pWNewSrc[po] = 65535;
							else
								pWNewSrc[po] = (UINT16)(pError[po] + pWNewSrc[po]);
						}
						else if (pError[po] < 0) {
							if (pError[po] + pWNewSrc[po] < 0)
								pWNewSrc[po] = 0;
							else
								pWNewSrc[po] = (UINT16)(pError[po] + pWNewSrc[po]);
						}
					}
					pError[po] = 0;
				}
			}

			// 1 st line is F&S even line
			if ((lo % 2) == 0) {
				pDst = outplane->buffer + (lo * outplane->lineLen);

				for (po = 0; po < (int)outplane->WidthPx; po++)
				{
					if (pWNewSrc[po]) {

						// drop and calculate error to remove
						if (pWNewSrc[po] > NoiseTable[rindex++&(0xFFFF)]) {
							valError = (int)pWNewSrc[po] - 65535;
							(*pDropCount)++;
							switch (po % 4) {
							case 0: *pDst = Val4[0]; break;
							case 1: *pDst |= Val4[1]; break;
							case 2: *pDst |= Val4[2]; break;
							case 3: *pDst |= Val4[3]; break;
							}
						}

						// no drop and calculate error to add
						else {
							valError = pWNewSrc[po];
						}

						if (valError) { // distribute error
							valError /= 16;

							// distribute 7/16 at right
							if (po + 1 < (int)outplane->WidthPx) {
								if (pWNewSrc[po + 1]) {
									distError = valError * 7;
									if (distError > 0) {
										if (distError + pWNewSrc[po + 1] > 65535)
											pWNewSrc[po + 1] = 65535;
										else
											pWNewSrc[po + 1] = (UINT16)(distError + pWNewSrc[po + 1]);
									}
									else {
										if (distError + pWNewSrc[po] < 0)
											pWNewSrc[po + 1] = 0;
										else
											pWNewSrc[po + 1] = (UINT16)(distError + pWNewSrc[po + 1]);
									}
								}

								// distribute 1/16 next line right
								pError[po + 1] += valError;
							}

							// distribute 3/16 next line left
							if (po > 0) {
								pError[po - 1] += valError * 3;
							}

							// distribute 5/16 next line
							pError[po] += valError * 5;
						}
					}
					else
						pError[po] = 0;

					if ((po % 4) == 3)
						pDst++;
				}
			}

			// F&S odd line
			else {
				pDst = outplane->buffer + ((lo + 1) * outplane->lineLen) - 1;
				for (po = (int)outplane->WidthPx - 1; po > 0; po--)
				{
					if (pWNewSrc[po]) {

						// drop and calculate error to remove
						if (pWNewSrc[po] > NoiseTable[rindex++&(0xFFFF)]) {
							valError = (int)pWNewSrc[po] - 65535;
							(*pDropCount)++;
							switch (po % 4) {
							case 0: *pDst |= Val4[0]; break;
							case 1: *pDst |= Val4[1]; break;
							case 2: *pDst |= Val4[2]; break;
							case 3: *pDst = Val4[3]; break;
							}
						}

						// no drop and calculate error to add
						else {
							valError = pWNewSrc[po];
						}

						if (valError) { // distribute error
							valError /= 16;

							// distribute 7/16 at left
							if (po - 1 > 0) {
								if (pWNewSrc[po - 1]) {
									distError = valError * 7;
									if (distError > 0) {
										if (distError + pWNewSrc[po - 1] > 65535)
											pWNewSrc[po - 1] = 65535;
										else
											pWNewSrc[po - 1] = (UINT16)(distError + pWNewSrc[po - 1]);
									}
									else {
										if (distError + pWNewSrc[po] < 0)
											pWNewSrc[po - 1] = 0;
										else
											pWNewSrc[po - 1] = (UINT16)(distError + pWNewSrc[po - 1]);
									}
								}

								// distribute 1/16 next line left
								pError[po - 1] += valError;
							}

							// distribute 3/16 next line right
							if (po < (int)inplane->WidthPx - 1) {
								pError[po + 1] += valError * 3;
							}

							// distribute 5/16 next line
							pError[po] += valError * 5;
						}
					}
					else
						pError[po] = 0;

					if ((po % 4) == 0)
						pDst--;
				}
			}
		}
	}
	
	if (pLine)
		rx_mem_free((BYTE**)&pLine);
	if (pLineError)
		rx_mem_free((BYTE**)&pLineError);

	return (REPLY_OK);
}



//--- rx_screen_init_FME_1x1 ----------------------------------------------------------------------
int rx_screen_init_FME_1x1(SScreenConfig* pscreenConfig)
{
	int i;
	sprintf(pscreenConfig->typeName, "FMS_1x1");
	pscreenConfig->fctplane = &rx_screen_plane_FME_1x1;
	pscreenConfig->fctclose = &rx_screen_close_null;
	if (pscreenConfig->dropsize == 1) {
		Val4[0] = 0x40;
		Val4[1] = 0x10;
		Val4[2] = 0x04;
		Val4[3] = 0x01;
	}
	else if (pscreenConfig->dropsize == 2) {
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

	if (pscreenConfig->noise == 0)
		for (i = 0; i < 65536; i++) {
			NoiseTable[i] = 32767;
		}
	else {
		int noise = pscreenConfig->noise;
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

	return (REPLY_OK);
}


//--- rx_screen_plane_FME_1x2g ----------------------------------------------------------------------
int rx_screen_plane_FME_1x2g(SPlaneInfo *inplane, SPlaneInfo *outplane, void* epscreenConfig)
{
	SScreenConfig* pscreenConfig = epscreenConfig;
	UINT32 l;
	int i, valError, distError, ret= REPLY_OK;
	int nindex = (rand() % 256) * 257;
	int rindex = (rand() % 256) * 257;
	int * pError, *pLineError, *LimitTable;
	BYTE *pSrc, *pDst, *pLineLim, Val1 = 1, Val2 = 2;
	UINT16 *pWSrc, *pLine = NULL, *pWNewSrc, WLimit = 32768, WT2 = 32768;
	UINT16 *lut16 = pscreenConfig->lut16[inplane->planenumber];

	if ((pscreenConfig->inputResol.x * 2 == pscreenConfig->outputResol.x) && (pscreenConfig->inputResol.y * 2 == pscreenConfig->outputResol.y)) {
		return (rx_screen_plane_FME_1x2gx2(inplane, outplane, pscreenConfig));
	}
	else if ((pscreenConfig->inputResol.x * 4 == pscreenConfig->outputResol.x) && (pscreenConfig->inputResol.y * 4 == pscreenConfig->outputResol.y)) {
		return (rx_screen_plane_FME_1x2gx4(inplane, outplane, pscreenConfig));
	}

	if (rx_screen_outplane_init(inplane, outplane, pscreenConfig))
		return REPLY_ERROR;

	if (pscreenConfig->dropsize == 13)
		Val2 = 3;
	else if (pscreenConfig->dropsize == 23) {
		Val1 = 2;
		Val2 = 3;
	}
	if (pscreenConfig->limit[outplane->planenumber][0]) {
		WLimit = 65536 * pscreenConfig->limit[outplane->planenumber][0] / 100;
		WT2 = 65536 - WLimit;
		if (!WT2) WT2 = 1;
	}
	// alloc LimitTable1
	LimitTable = (int *)rx_mem_alloc(65536 * sizeof(int));
	if (LimitTable == NULL) {
		ret = Error(ERR_CONT, 0, "No buffer line for screening: %d"); goto End;
	}
	for (i = 0; i < 65536; i++) {
		LimitTable[i] = WLimit -1 + (RandTable[i] * WT2 / 65536);
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

		// lut table from 16 bits
		if (inplane->bitsPerPixel == 16) {
			pWSrc = (UINT16 *)(inplane->buffer + (l * inplane->lineLen));
			for (i = 0; i < (int)inplane->WidthPx; i++)
				pWNewSrc[i] = lut16[pWSrc[i]];
		}

		// lut table from 8 bits
		else {
			pSrc = (BYTE *)(inplane->buffer + (l * inplane->lineLen));
			for (i = 0; i < (int)inplane->WidthPx; i++)
				pWNewSrc[i] = lut16[pSrc[i] * 257];
		}

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

						outplane->DropCount[pLineLim[i] - 1]++;
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

						outplane->DropCount[pLineLim[i] - 1]++;
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


//--- rx_screen_plane_FME_1x2gx2 ----------------------------------------------------------------------
int rx_screen_plane_FME_1x2gx2(SPlaneInfo *inplane, SPlaneInfo *outplane, void* epscreenConfig)
{
	SScreenConfig* pscreenConfig = epscreenConfig;
	UINT32 li, lo;
	int cpt, pi, po, valError, distError, ret = REPLY_OK;
	int rindex = (rand() % 256) * 257;
	int nindex = (rand() % 256) * 257;
	int * pError, *pLineError, *LimitTable;
	BYTE *pSrc, *pDst, *pLineLim, Val1 = 1, Val2 = 2;
	UINT16 *pWSrc, *pLine = NULL, *pWNewSrc, WLimit = 32768, WT2 = 32768;
	UINT16 *lut16 = pscreenConfig->lut16[inplane->planenumber];

	if ((pscreenConfig->inputResol.x * 4 == pscreenConfig->outputResol.x) && (pscreenConfig->inputResol.y * 4 == pscreenConfig->outputResol.y)) {
		return (rx_screen_plane_FME_1x2gx4(inplane, outplane, pscreenConfig));
	}
	if (rx_screen_outplane_init(inplane, outplane, pscreenConfig))
		return REPLY_ERROR;

	if (pscreenConfig->dropsize == 13)
		Val2 = 3;
	else if (pscreenConfig->dropsize == 23) {
		Val1 = 2;
		Val2 = 3;
	}
	if (pscreenConfig->limit[outplane->planenumber][0]) {
		WLimit = 65536 * pscreenConfig->limit[outplane->planenumber][0] / 100;
		WT2 = 65536 - WLimit;
		if (!WT2) WT2 = 1;
	}
	// alloc LimitTable1
	LimitTable = (int *)rx_mem_alloc(65536 * sizeof(int));
	if (LimitTable == NULL) {
		ret = Error(ERR_CONT, 0, "No buffer line for screening: %d"); goto End;
	}
	for (cpt = 0; cpt < 65536; cpt++) {
		LimitTable[cpt] = WLimit - 1 + (RandTable[cpt] * WT2 / 65536);
	}

	// alloc line for data
	pLine = (UINT16 *)rx_mem_alloc(outplane->WidthPx * sizeof(UINT16));
	if (pLine == NULL) {
		ret = Error(ERR_CONT, 0, "No buffer line for screening: %d"); goto End;
	}
	pWNewSrc = pLine;

	// alloc line for error
	pError = pLineError = (int *)rx_mem_alloc((outplane->WidthPx) * sizeof(int));
	if (pLineError == NULL) {
		ret = Error(ERR_CONT, 0, "No buffer line for screening: %d"); goto End;
	}
	memset(pLineError, 0, (outplane->WidthPx) * sizeof(int));

	// alloc line for limit
	pLineLim = (BYTE *)rx_mem_alloc((outplane->WidthPx) * sizeof(BYTE));
	if (pLineLim == NULL) {
		ret = Error(ERR_CONT, 0, "No buffer line for screening: %d"); goto End;
	}

	for (li = 0; li < inplane->lengthPx; li++)
	{
		for (lo = 2 * li; lo < (2 * li) + 2; lo++)
		{
			// lut table from 16 bits
			if (inplane->bitsPerPixel == 16) {
				pWSrc = (UINT16 *)(inplane->buffer + (li * inplane->lineLen));
				for (pi = 0; pi < (int)inplane->WidthPx; pi++)
					for (po = 2 * pi; po < (2 * pi) + 2; po++)
						pWNewSrc[po] = lut16[pWSrc[pi]];
			}

			// lut table from 8 bits
			else {
				pSrc = (BYTE *)(inplane->buffer + (li * inplane->lineLen));
				for (pi = 0; pi < (int)inplane->WidthPx; pi++)
					for (po = 2 * pi; po < (2 * pi) + 2; po++)
						pWNewSrc[po] = lut16[pSrc[pi] * 257];
			}

			// add error from previous line if any
			for (po = 0; po < (int)outplane->WidthPx; po++)
			{
				pLineLim[po] = Val1;
				if (pWNewSrc[po] > WLimit)
					if (pWNewSrc[po] >= LimitTable[rindex++&(0xFFFF)]) {
						pLineLim[po] = Val2;
					}

				if (pError[po]) {
					if (pWNewSrc[po]) {
						if (pError[po] > 0) {
							if (pError[po] + pWNewSrc[po] > 65535)
								pWNewSrc[po] = 65535;
							else
								pWNewSrc[po] = (UINT16)(pError[po] + pWNewSrc[po]);
						}
						else if (pError[po] < 0) {
							if (pError[po] + pWNewSrc[po] < 0)
								pWNewSrc[po] = 0;
							else
								pWNewSrc[po] = (UINT16)(pError[po] + pWNewSrc[po]);
						}
					}
					pError[po] = 0;
				}
			}

			// 1 st line is F&S even line
			if ((lo % 2) == 0) {
				pDst = outplane->buffer + (lo * outplane->lineLen);

				for (po = 0; po < (int)outplane->WidthPx; po++)
				{
					if (pWNewSrc[po]) {

						// drop and calculate error to remove
						if (pWNewSrc[po] > NoiseTable[nindex++&(0xFFFF)]) {
							valError = ((int)pWNewSrc[po] - 65535) ;

							outplane->DropCount[pLineLim[po] - 1]++;
							switch (po % 4) {
							case 0: *pDst = pLineLim[po] << 6; break;
							case 1: *pDst |= pLineLim[po] << 4; break;
							case 2: *pDst |= pLineLim[po] << 2; break;
							case 3: *pDst |= pLineLim[po]; break;
							}
						}

						// no drop and calculate error to add
						else {
							valError = pWNewSrc[po];
						}

						if (valError) { // distribute error
							valError /= 16;

							// distribute 7/16 at right
							if (po + 1 < (int)outplane->WidthPx) {
								if (pWNewSrc[po + 1]) {
									distError = valError * 7;
									if (distError > 0) {
										if (distError + pWNewSrc[po + 1] > 65535)
											pWNewSrc[po + 1] = 65535;
										else
											pWNewSrc[po + 1] = (UINT16)(distError + pWNewSrc[po + 1]);
									}
									else {
										if (distError + pWNewSrc[po] < 0)
											pWNewSrc[po + 1] = 0;
										else
											pWNewSrc[po + 1] = (UINT16)(distError + pWNewSrc[po + 1]);
									}
								}

								// distribute 1/16 next line right
								pError[po + 1] += valError;
							}

							// distribute 3/16 next line left
							if (po > 0) {
								pError[po - 1] += valError * 3;
							}

							// distribute 5/16 next line
							pError[po] += valError * 5;
						}
					}
					else
						pError[po] = 0;

					if ((po % 4) == 3)
						pDst++;
				}
			}

			// F&S odd line
			else {
				pDst = outplane->buffer + ((lo + 1) * outplane->lineLen) - 1;
				for (po = (int)outplane->WidthPx - 1; po > 0; po--)
				{
					if (pWNewSrc[po]) {

						// drop and calculate error to remove
						if (pWNewSrc[po] > NoiseTable[nindex++&(0xFFFF)]) {
							valError = ((int)pWNewSrc[po] - 65535) ;

							outplane->DropCount[pLineLim[po] - 1]++;
							switch (po % 4) {
							case 0: *pDst |= pLineLim[po] << 6; break;
							case 1: *pDst |= pLineLim[po] << 4; break;
							case 2: *pDst |= pLineLim[po] << 2; break;
							case 3: *pDst = pLineLim[po]; break;
							}
						}

						// no drop and calculate error to add
						else {
							valError = pWNewSrc[po];
						}

						if (valError) { // distribute error
							valError /= 16;

							// distribute 7/16 at left
							if (po - 1 > 0) {
								if (pWNewSrc[po - 1]) {
									distError = valError * 7;
									if (distError > 0) {
										if (distError + pWNewSrc[po - 1] > 65535)
											pWNewSrc[po - 1] = 65535;
										else
											pWNewSrc[po - 1] = (UINT16)(distError + pWNewSrc[po - 1]);
									}
									else {
										if (distError + pWNewSrc[po] < 0)
											pWNewSrc[po - 1] = 0;
										else
											pWNewSrc[po - 1] = (UINT16)(distError + pWNewSrc[po - 1]);
									}
								}

								// distribute 1/16 next line left
								pError[po - 1] += valError;
							}

							// distribute 3/16 next line right
							if (po < (int)inplane->WidthPx - 1) {
								pError[po + 1] += valError * 3;
							}

							// distribute 5/16 next line
							pError[po] += valError * 5;
						}
					}
					else
						pError[po] = 0;

					if ((po % 4) == 0)
						pDst--;
				}
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


//--- rx_screen_plane_FME_1x2gx4 ----------------------------------------------------------------------
int rx_screen_plane_FME_1x2gx4(SPlaneInfo *inplane, SPlaneInfo *outplane, void* epscreenConfig)
{
	SScreenConfig* pscreenConfig = epscreenConfig;
	UINT32 li, lo;
	int cpt ,pi, po, valError, distError, ret = REPLY_OK;
	int rindex = (rand() % 256) * 257;
	int nindex = (rand() % 256) * 257;
	int * pError, *pLineError, *LimitTable;
	BYTE *pSrc, *pDst, *pLineLim, Val1 = 1, Val2 = 2;
	UINT16 *pWSrc, *pLine = NULL, *pWNewSrc, WLimit = 32768, WT2 = 32768;
	UINT16 *lut16 = pscreenConfig->lut16[inplane->planenumber];

	if (rx_screen_outplane_init(inplane, outplane, pscreenConfig))
		return REPLY_ERROR;

	if (pscreenConfig->dropsize == 13)
		Val2 = 3;
	else if (pscreenConfig->dropsize == 23) {
		Val1 = 2;
		Val2 = 3;
	}
	if (pscreenConfig->limit[outplane->planenumber][0]) {
		WLimit = 65536 * pscreenConfig->limit[outplane->planenumber][0] / 100;
		WT2 = 65536 - WLimit;
		if (!WT2) WT2 = 1;
	}
	// alloc LimitTable1
	LimitTable = (int *)rx_mem_alloc(65536 * sizeof(int));
	if (LimitTable == NULL) {
		ret = Error(ERR_CONT, 0, "No buffer line for screening: %d"); goto End;
	}
	for (cpt = 0; cpt < 65536; cpt++) {
		LimitTable[cpt] = WLimit - 1 + (RandTable[cpt] * WT2 / 65536);
	}

	// alloc line for data
	pLine = (UINT16 *)rx_mem_alloc(outplane->WidthPx * sizeof(UINT16));
	if (pLine == NULL) {
		ret = Error(ERR_CONT, 0, "No buffer line for screening: %d"); goto End;
	}
	pWNewSrc = pLine;

	// alloc line for error
	pError = pLineError = (int *)rx_mem_alloc((outplane->WidthPx) * sizeof(int));
	if (pLineError == NULL) {
		ret = Error(ERR_CONT, 0, "No buffer line for screening: %d"); goto End;
	}
	memset(pLineError, 0, (outplane->WidthPx) * sizeof(int));

	// alloc line for limit
	pLineLim = (BYTE *)rx_mem_alloc((outplane->WidthPx) * sizeof(BYTE));
	if (pLineLim == NULL) {
		ret = Error(ERR_CONT, 0, "No buffer line for screening: %d"); goto End;
	}

	for (li = 0; li < inplane->lengthPx; li++)
	{
		for (lo = 4 * li; lo < (4 * li) + 4; lo++)
		{
			// lut table from 16 bits
			if (inplane->bitsPerPixel == 16) {
				pWSrc = (UINT16 *)(inplane->buffer + (li * inplane->lineLen));
				for (pi = 0; pi < (int)inplane->WidthPx; pi++)
					for (po = 4 * pi; po < (4 * pi) + 4; po++)
						pWNewSrc[po] = lut16[pWSrc[pi]];
			}

			// lut table from 8 bits
			else {
				pSrc = (BYTE *)(inplane->buffer + (li * inplane->lineLen));
				for (pi = 0; pi < (int)inplane->WidthPx; pi++)
					for (po = 4 * pi; po < (4 * pi) + 4; po++)
						pWNewSrc[po] = lut16[pSrc[pi] * 257];
			}

			// add error from previous line if any
			for (po = 0; po < (int)outplane->WidthPx; po++)
			{
				pLineLim[po] = Val1;
				if (pWNewSrc[po] > WLimit)
					if (pWNewSrc[po] >= LimitTable[rindex++&(0xFFFF)]) {
						pLineLim[po] = Val2;
					}

				if (pError[po]) {
					if (pWNewSrc[po]) {
						if (pError[po] > 0) {
							if (pError[po] + pWNewSrc[po] > 65535)
								pWNewSrc[po] = 65535;
							else
								pWNewSrc[po] = (UINT16)(pError[po] + pWNewSrc[po]);
						}
						else if (pError[po] < 0) {
							if (pError[po] + pWNewSrc[po] < 0)
								pWNewSrc[po] = 0;
							else
								pWNewSrc[po] = (UINT16)(pError[po] + pWNewSrc[po]);
						}
					}
					pError[po] = 0;
				}
			}

			// 1 st line is F&S even line
			if ((lo % 2) == 0) {
				pDst = outplane->buffer + (lo * outplane->lineLen);

				for (po = 0; po < (int)outplane->WidthPx; po++)
				{
					if (pWNewSrc[po]) {

						// drop and calculate error to remove
						if (pWNewSrc[po] > NoiseTable[nindex++&(0xFFFF)]) {
							valError = (int)pWNewSrc[po] - 65535;

							outplane->DropCount[pLineLim[po] - 1]++;
							switch (po % 4) {
							case 0: *pDst = pLineLim[po] << 6; break;
							case 1: *pDst |= pLineLim[po] << 4; break;
							case 2: *pDst |= pLineLim[po] << 2; break;
							case 3: *pDst |= pLineLim[po]; break;
							}
						}

						// no drop and calculate error to add
						else {
							valError = pWNewSrc[po];
						}

						if (valError) { // distribute error
							valError /= 16;

							// distribute 7/16 at right
							if (po + 1 < (int)outplane->WidthPx) {
								if (pWNewSrc[po + 1]) {
									distError = valError * 7;
									if (distError > 0) {
										if (distError + pWNewSrc[po + 1] > 65535)
											pWNewSrc[po + 1] = 65535;
										else
											pWNewSrc[po + 1] = (UINT16)(distError + pWNewSrc[po + 1]);
									}
									else {
										if (distError + pWNewSrc[po] < 0)
											pWNewSrc[po + 1] = 0;
										else
											pWNewSrc[po + 1] = (UINT16)(distError + pWNewSrc[po + 1]);
									}
								}

								// distribute 1/16 next line right
								pError[po + 1] += valError;
							}

							// distribute 3/16 next line left
							if (po > 0) {
								pError[po - 1] += valError * 3;
							}

							// distribute 5/16 next line
							pError[po] += valError * 5;
						}
					}
					else
						pError[po] = 0;

					if ((po % 4) == 3)
						pDst++;
				}
			}

			// F&S odd line
			else {
				pDst = outplane->buffer + ((lo + 1) * outplane->lineLen) - 1;
				for (po = (int)outplane->WidthPx - 1; po > 0; po--)
				{
					if (pWNewSrc[po]) {

						// drop and calculate error to remove
						if (pWNewSrc[po] > NoiseTable[nindex++&(0xFFFF)]) {
							valError = (int)pWNewSrc[po] - 65535;

							outplane->DropCount[pLineLim[po] - 1]++;
							switch (po % 4) {
							case 0: *pDst |= pLineLim[po] << 6; break;
							case 1: *pDst |= pLineLim[po] << 4; break;
							case 2: *pDst |= pLineLim[po] << 2; break;
							case 3: *pDst = pLineLim[po]; break;
							}
						}

						// no drop and calculate error to add
						else {
							valError = pWNewSrc[po];
						}

						if (valError) { // distribute error
							valError /= 16;

							// distribute 7/16 at left
							if (po - 1 > 0) {
								if (pWNewSrc[po - 1]) {
									distError = valError * 7;
									if (distError > 0) {
										if (distError + pWNewSrc[po - 1] > 65535)
											pWNewSrc[po - 1] = 65535;
										else
											pWNewSrc[po - 1] = (UINT16)(distError + pWNewSrc[po - 1]);
									}
									else {
										if (distError + pWNewSrc[po] < 0)
											pWNewSrc[po - 1] = 0;
										else
											pWNewSrc[po - 1] = (UINT16)(distError + pWNewSrc[po - 1]);
									}
								}

								// distribute 1/16 next line left
								pError[po - 1] += valError;
							}

							// distribute 3/16 next line right
							if (po < (int)inplane->WidthPx - 1) {
								pError[po + 1] += valError * 3;
							}

							// distribute 5/16 next line
							pError[po] += valError * 5;
						}
					}
					else
						pError[po] = 0;

					if ((po % 4) == 0)
						pDst--;
				}
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
int rx_screen_init_FME_1x2g(SScreenConfig* pscreenConfig)
{
	int i;
	sprintf(pscreenConfig->typeName, "FME_1x2g");
	pscreenConfig->fctplane = &rx_screen_plane_FME_1x2g;
	pscreenConfig->fctclose = &rx_screen_close_null;

	for (i = 0; i < 65536; i++) {
		RandTable[i] = ((rand() % 256) * 256) + (rand() % 256);
	}

	if (pscreenConfig->noise == 0)
		for (i = 0; i < 65536; i++) {
			NoiseTable[i] = 32767;
		}
	else {
		int noise = pscreenConfig->noise;
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

	return (REPLY_OK);
}


//--- rx_screen_plane_FME_1x3g ----------------------------------------------------------------------
int rx_screen_plane_FME_1x3g(SPlaneInfo *inplane, SPlaneInfo *outplane, void* epscreenConfig)
{
	SScreenConfig* pscreenConfig = epscreenConfig;
	UINT32 l;
	int cpt, i, valError, distError, ret= REPLY_OK;
	int nindex = (rand() % 256) * 257;
	int rindex = (rand() % 256) * 257;
	int * pError, *pLineError, *LimitTable1, *LimitTable2;
	BYTE *pSrc, *pDst, *pLineLim;
	UINT16 *pWSrc, *pLine = NULL, *pWNewSrc, WLimit1 = 21845, WLimit2 = 43690, WT1 = 21845, WT2 = 21845, WT3 = 21845;
	UINT16 *lut16 = pscreenConfig->lut16[inplane->planenumber];


	if ((pscreenConfig->inputResol.x * 2 == pscreenConfig->outputResol.x) && (pscreenConfig->inputResol.y * 2 == pscreenConfig->outputResol.y)) {
		return (rx_screen_plane_FME_1x3gx2(inplane, outplane, pscreenConfig));
	}
	else if ((pscreenConfig->inputResol.x * 4 == pscreenConfig->outputResol.x) && (pscreenConfig->inputResol.y * 4 == pscreenConfig->outputResol.y)) {
		return (rx_screen_plane_FME_1x3gx4(inplane, outplane, pscreenConfig));
	}

	if (rx_screen_outplane_init(inplane, outplane, pscreenConfig))
		return REPLY_ERROR;

	if (pscreenConfig->limit[outplane->planenumber][0]) {
		WT1 = WLimit1 = 65536 * pscreenConfig->limit[outplane->planenumber][0] / 100;
		if (!WT1) WT1 = 1;
		WLimit2 = 65536 * pscreenConfig->limit[outplane->planenumber][1] / 100;
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

		// lut table from 16 bits
		if (inplane->bitsPerPixel == 16) {
			pWSrc = (UINT16 *)(inplane->buffer + (l * inplane->lineLen));
			for (i = 0; i < (int)inplane->WidthPx; i++)
				pWNewSrc[i] = lut16[pWSrc[i]];
		}

		// lut table from 8 bits
		else {
			pSrc = (BYTE *)(inplane->buffer + (l * inplane->lineLen));
			for (i = 0; i < (int)inplane->WidthPx; i++)
				pWNewSrc[i] = lut16[pSrc[i] * 257];
		}

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

						outplane->DropCount[pLineLim[i] - 1]++;
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

						outplane->DropCount[pLineLim[i] - 1]++;
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


//--- rx_screen_plane_FME_1x3gx2 ----------------------------------------------------------------------
int rx_screen_plane_FME_1x3gx2(SPlaneInfo *inplane, SPlaneInfo *outplane, void* epscreenConfig)
{
	SScreenConfig* pscreenConfig = epscreenConfig;
	UINT32 li, lo;
	int cpt, pi, po, valError, distError, ret = REPLY_OK;
	int rindex = (rand() % 256) * 257;
	int nindex = (rand() % 256) * 257;
	int * pError, *pLineError, *LimitTable1, *LimitTable2;
	BYTE *pSrc, *pDst, *pLineLim;
	UINT16 *pWSrc, *pLine = NULL, *pWNewSrc, WLimit1 = 21845, WLimit2 = 43690, WT1 = 21845, WT2 = 21845, WT3 = 21845;
	UINT16 *lut16 = pscreenConfig->lut16[inplane->planenumber];

	if ((pscreenConfig->inputResol.x * 4 == pscreenConfig->outputResol.x) && (pscreenConfig->inputResol.y * 4 == pscreenConfig->outputResol.y)) {
		return (rx_screen_plane_FME_1x3gx4(inplane, outplane, pscreenConfig));
	}
	if (rx_screen_outplane_init(inplane, outplane, pscreenConfig))
		return REPLY_ERROR;

	if (pscreenConfig->limit[outplane->planenumber][0]) {
		WT1 = WLimit1 = 65536 * pscreenConfig->limit[outplane->planenumber][0] / 100;
		if (!WT1) WT1 = 1;
		WLimit2 = 65536 * pscreenConfig->limit[outplane->planenumber][1] / 100;
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
	pLine = (UINT16 *)rx_mem_alloc(outplane->WidthPx * sizeof(UINT16));
	if (pLine == NULL) {
		ret = Error(ERR_CONT, 0, "No buffer line for screening: %d"); goto End;
	}
	pWNewSrc = pLine;

	// alloc line for error
	pError = pLineError = (int *)rx_mem_alloc((outplane->WidthPx) * sizeof(int));
	if (pLineError == NULL) {
		ret = Error(ERR_CONT, 0, "No buffer line for screening: %d"); goto End;
	}
	memset(pLineError, 0, (outplane->WidthPx) * sizeof(int));

	// alloc line for limit
	pLineLim = (BYTE *)rx_mem_alloc((outplane->WidthPx) * sizeof(BYTE));
	if (pLineLim == NULL) {
		ret = Error(ERR_CONT, 0, "No buffer line for screening: %d"); goto End;
	}

	for (li = 0; li < inplane->lengthPx; li++)
	{
		for (lo = 2 * li; lo < (2 * li) + 2; lo++)
		{
			// lut table from 16 bits
			if (inplane->bitsPerPixel == 16) {
				pWSrc = (UINT16 *)(inplane->buffer + (li * inplane->lineLen));
				for (pi = 0; pi < (int)inplane->WidthPx; pi++)
					for (po = 2 * pi; po < (2 * pi) + 2; po++)
						pWNewSrc[po] = lut16[pWSrc[pi]];
			}

			// lut table from 8 bits
			else {
				pSrc = (BYTE *)(inplane->buffer + (li * inplane->lineLen));
				for (pi = 0; pi < (int)inplane->WidthPx; pi++)
					for (po = 2 * pi; po < (2 * pi) + 2; po++)
						pWNewSrc[po] = lut16[pSrc[pi] * 257];
			}

			// add error from previous line if any
			for (po = 0; po < (int)outplane->WidthPx; po++)
			{
				pLineLim[po] = 1;
				if (pWNewSrc[po] > WLimit2) {
					pLineLim[po] = 2;
					if (pWNewSrc[po] >= LimitTable2[rindex++&(0xFFFF)]) {
						pLineLim[po] = 3;
					}
				}
				else if (pWNewSrc[po] > WLimit1) {
					if (pWNewSrc[po] >= LimitTable1[rindex++&(0xFFFF)]) {
						pLineLim[po] = 2;
					}
				}

				if (pError[po]) {
					if (pWNewSrc[po]) {
						if (pError[po] > 0) {
							if (pError[po] + pWNewSrc[po] > 65535)
								pWNewSrc[po] = 65535;
							else
								pWNewSrc[po] = (UINT16)(pError[po] + pWNewSrc[po]);
						}
						else if (pError[po] < 0) {
							if (pError[po] + pWNewSrc[po] < 0)
								pWNewSrc[po] = 0;
							else
								pWNewSrc[po] = (UINT16)(pError[po] + pWNewSrc[po]);
						}
					}
					pError[po] = 0;
				}
			}

			// 1 st line is F&S even line
			if ((lo % 2) == 0) {
				pDst = outplane->buffer + (lo * outplane->lineLen);

				for (po = 0; po < (int)outplane->WidthPx; po++)
				{
					if (pWNewSrc[po]) {

						// drop and calculate error to remove
						if (pWNewSrc[po] > NoiseTable[nindex++&(0xFFFF)]) {
							valError = ((int)pWNewSrc[po] - 65535);

							outplane->DropCount[pLineLim[po] - 1]++;
							switch (po % 4) {
							case 0: *pDst = pLineLim[po] << 6; break;
							case 1: *pDst |= pLineLim[po] << 4; break;
							case 2: *pDst |= pLineLim[po] << 2; break;
							case 3: *pDst |= pLineLim[po]; break;
							}
						}

						// no drop and calculate error to add
						else {
							valError = pWNewSrc[po];
						}

						if (valError) { // distribute error
							valError /= 16;

							// distribute 7/16 at right
							if (po + 1 < (int)outplane->WidthPx) {
								if (pWNewSrc[po + 1]) {
									distError = valError * 7;
									if (distError > 0) {
										if (distError + pWNewSrc[po + 1] > 65535)
											pWNewSrc[po + 1] = 65535;
										else
											pWNewSrc[po + 1] = (UINT16)(distError + pWNewSrc[po + 1]);
									}
									else {
										if (distError + pWNewSrc[po] < 0)
											pWNewSrc[po + 1] = 0;
										else
											pWNewSrc[po + 1] = (UINT16)(distError + pWNewSrc[po + 1]);
									}
								}

								// distribute 1/16 next line right
								pError[po + 1] += valError;
							}

							// distribute 3/16 next line left
							if (po > 0) {
								pError[po - 1] += valError * 3;
							}

							// distribute 5/16 next line
							pError[po] += valError * 5;
						}
					}
					else
						pError[po] = 0;

					if ((po % 4) == 3)
						pDst++;
				}
			}

			// F&S odd line
			else {
				pDst = outplane->buffer + ((lo + 1) * outplane->lineLen) - 1;
				for (po = (int)outplane->WidthPx - 1; po > 0; po--)
				{
					if (pWNewSrc[po]) {

						// drop and calculate error to remove
						if (pWNewSrc[po] > NoiseTable[nindex++&(0xFFFF)]) {
							valError = ((int)pWNewSrc[po] - 65535);

							outplane->DropCount[pLineLim[po] - 1]++;
							switch (po % 4) {
							case 0: *pDst |= pLineLim[po] << 6; break;
							case 1: *pDst |= pLineLim[po] << 4; break;
							case 2: *pDst |= pLineLim[po] << 2; break;
							case 3: *pDst = pLineLim[po]; break;
							}
						}

						// no drop and calculate error to add
						else {
							valError = pWNewSrc[po];
						}

						if (valError) { // distribute error
							valError /= 16;

							// distribute 7/16 at left
							if (po - 1 > 0) {
								if (pWNewSrc[po - 1]) {
									distError = valError * 7;
									if (distError > 0) {
										if (distError + pWNewSrc[po - 1] > 65535)
											pWNewSrc[po - 1] = 65535;
										else
											pWNewSrc[po - 1] = (UINT16)(distError + pWNewSrc[po - 1]);
									}
									else {
										if (distError + pWNewSrc[po] < 0)
											pWNewSrc[po - 1] = 0;
										else
											pWNewSrc[po - 1] = (UINT16)(distError + pWNewSrc[po - 1]);
									}
								}

								// distribute 1/16 next line left
								pError[po - 1] += valError;
							}

							// distribute 3/16 next line right
							if (po < (int)inplane->WidthPx - 1) {
								pError[po + 1] += valError * 3;
							}

							// distribute 5/16 next line
							pError[po] += valError * 5;
						}
					}
					else
						pError[po] = 0;

					if ((po % 4) == 0)
						pDst--;
				}
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


//--- rx_screen_plane_FME_1x3gx4 ----------------------------------------------------------------------
int rx_screen_plane_FME_1x3gx4(SPlaneInfo *inplane, SPlaneInfo *outplane, void* epscreenConfig)
{
	SScreenConfig* pscreenConfig = epscreenConfig;
	UINT32 li, lo;
	int cpt, pi, po, valError, distError, ret = REPLY_OK;
	int rindex = (rand() % 256) * 257;
	int nindex = (rand() % 256) * 257;
	int * pError, *pLineError, *LimitTable1, *LimitTable2;
	BYTE *pSrc, *pDst, *pLineLim;
	UINT16 *pWSrc, *pLine = NULL, *pWNewSrc, WLimit1 = 21845, WLimit2 = 43690, WT1 = 21845, WT2 = 21845, WT3 = 21845;
	UINT16 *lut16 = pscreenConfig->lut16[inplane->planenumber];

	if (rx_screen_outplane_init(inplane, outplane, pscreenConfig))
		return REPLY_ERROR;

	if (pscreenConfig->limit[outplane->planenumber][0]) {
		WT1 = WLimit1 = 65536 * pscreenConfig->limit[outplane->planenumber][0] / 100;
		if (!WT1) WT1 = 1;
		WLimit2 = 65536 * pscreenConfig->limit[outplane->planenumber][1] / 100;
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
	pLine = (UINT16 *)rx_mem_alloc(outplane->WidthPx * sizeof(UINT16));
	if (pLine == NULL) {
		ret = Error(ERR_CONT, 0, "No buffer line for screening: %d"); goto End;
	}
	pWNewSrc = pLine;

	// alloc line for error
	pError = pLineError = (int *)rx_mem_alloc((outplane->WidthPx) * sizeof(int));
	if (pLineError == NULL) {
		ret = Error(ERR_CONT, 0, "No buffer line for screening: %d"); goto End;
	}
	memset(pLineError, 0, (outplane->WidthPx) * sizeof(int));

	// alloc line for limit
	pLineLim = (BYTE *)rx_mem_alloc((outplane->WidthPx) * sizeof(BYTE));
	if (pLineLim == NULL) {
		ret = Error(ERR_CONT, 0, "No buffer line for screening: %d"); goto End;
	}

	for (li = 0; li < inplane->lengthPx; li++)
	{
		for (lo = 4 * li; lo < (4 * li) + 4; lo++)
		{
			// lut table from 16 bits
			if (inplane->bitsPerPixel == 16) {
				pWSrc = (UINT16 *)(inplane->buffer + (li * inplane->lineLen));
				for (pi = 0; pi < (int)inplane->WidthPx; pi++)
					for (po = 4 * pi; po < (4 * pi) + 4; po++)
						pWNewSrc[po] = lut16[pWSrc[pi]];
			}

			// lut table from 8 bits
			else {
				pSrc = (BYTE *)(inplane->buffer + (li * inplane->lineLen));
				for (pi = 0; pi < (int)inplane->WidthPx; pi++)
					for (po = 4 * pi; po < (4 * pi) + 4; po++)
						pWNewSrc[po] = lut16[pSrc[pi] * 257];
			}

			// add error from previous line if any
			for (po = 0; po < (int)outplane->WidthPx; po++)
			{
				pLineLim[po] = 1;
				if (pWNewSrc[po] > WLimit2) {
					pLineLim[po] = 2;
					if (pWNewSrc[po] >= LimitTable2[rindex++&(0xFFFF)]) {
						pLineLim[po] = 3;
					}
				}
				else if (pWNewSrc[po] > WLimit1) {
					if (pWNewSrc[po] >= LimitTable1[rindex++&(0xFFFF)]) {
						pLineLim[po] = 2;
					}
				}

				if (pError[po]) {
					if (pWNewSrc[po]) {
						if (pError[po] > 0) {
							if (pError[po] + pWNewSrc[po] > 65535)
								pWNewSrc[po] = 65535;
							else
								pWNewSrc[po] = (UINT16)(pError[po] + pWNewSrc[po]);
						}
						else if (pError[po] < 0) {
							if (pError[po] + pWNewSrc[po] < 0)
								pWNewSrc[po] = 0;
							else
								pWNewSrc[po] = (UINT16)(pError[po] + pWNewSrc[po]);
						}
					}
					pError[po] = 0;
				}
			}

			// 1 st line is F&S even line
			if ((lo % 2) == 0) {
				pDst = outplane->buffer + (lo * outplane->lineLen);

				for (po = 0; po < (int)outplane->WidthPx; po++)
				{
					if (pWNewSrc[po]) {

						// drop and calculate error to remove
						if (pWNewSrc[po] > NoiseTable[nindex++&(0xFFFF)]) {
							valError = (int)pWNewSrc[po] - 65535;

							outplane->DropCount[pLineLim[po] - 1]++;
							switch (po % 4) {
							case 0: *pDst = pLineLim[po] << 6; break;
							case 1: *pDst |= pLineLim[po] << 4; break;
							case 2: *pDst |= pLineLim[po] << 2; break;
							case 3: *pDst |= pLineLim[po]; break;
							}
						}

						// no drop and calculate error to add
						else {
							valError = pWNewSrc[po];
						}

						if (valError) { // distribute error
							valError /= 16;

							// distribute 7/16 at right
							if (po + 1 < (int)outplane->WidthPx) {
								if (pWNewSrc[po + 1]) {
									distError = valError * 7;
									if (distError > 0) {
										if (distError + pWNewSrc[po + 1] > 65535)
											pWNewSrc[po + 1] = 65535;
										else
											pWNewSrc[po + 1] = (UINT16)(distError + pWNewSrc[po + 1]);
									}
									else {
										if (distError + pWNewSrc[po] < 0)
											pWNewSrc[po + 1] = 0;
										else
											pWNewSrc[po + 1] = (UINT16)(distError + pWNewSrc[po + 1]);
									}
								}

								// distribute 1/16 next line right
								pError[po + 1] += valError;
							}

							// distribute 3/16 next line left
							if (po > 0) {
								pError[po - 1] += valError * 3;
							}

							// distribute 5/16 next line
							pError[po] += valError * 5;
						}
					}
					else
						pError[po] = 0;

					if ((po % 4) == 3)
						pDst++;
				}
			}

			// F&S odd line
			else {
				pDst = outplane->buffer + ((lo + 1) * outplane->lineLen) - 1;
				for (po = (int)outplane->WidthPx - 1; po > 0; po--)
				{
					if (pWNewSrc[po]) {

						// drop and calculate error to remove
						if (pWNewSrc[po] > NoiseTable[nindex++&(0xFFFF)]) {
							valError = (int)pWNewSrc[po] - 65535;

							outplane->DropCount[pLineLim[po] - 1]++;
							switch (po % 4) {
							case 0: *pDst |= pLineLim[po] << 6; break;
							case 1: *pDst |= pLineLim[po] << 4; break;
							case 2: *pDst |= pLineLim[po] << 2; break;
							case 3: *pDst = pLineLim[po]; break;
							}
						}

						// no drop and calculate error to add
						else {
							valError = pWNewSrc[po];
						}

						if (valError) { // distribute error
							valError /= 16;

							// distribute 7/16 at left
							if (po - 1 > 0) {
								if (pWNewSrc[po - 1]) {
									distError = valError * 7;
									if (distError > 0) {
										if (distError + pWNewSrc[po - 1] > 65535)
											pWNewSrc[po - 1] = 65535;
										else
											pWNewSrc[po - 1] = (UINT16)(distError + pWNewSrc[po - 1]);
									}
									else {
										if (distError + pWNewSrc[po] < 0)
											pWNewSrc[po - 1] = 0;
										else
											pWNewSrc[po - 1] = (UINT16)(distError + pWNewSrc[po - 1]);
									}
								}

								// distribute 1/16 next line left
								pError[po - 1] += valError;
							}

							// distribute 3/16 next line right
							if (po < (int)inplane->WidthPx - 1) {
								pError[po + 1] += valError * 3;
							}

							// distribute 5/16 next line
							pError[po] += valError * 5;
						}
					}
					else
						pError[po] = 0;

					if ((po % 4) == 0)
						pDst--;
				}
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



//--- rx_screen_init_FME_1x2g ----------------------------------------------------------------------
int rx_screen_init_FME_1x3g(SScreenConfig* pscreenConfig)
{
	int i;
	sprintf(pscreenConfig->typeName, "FME_1x3g");
	pscreenConfig->fctplane = &rx_screen_plane_FME_1x3g;
	pscreenConfig->fctclose = &rx_screen_close_null;

	for (i = 0; i < 65536; i++) {
		RandTable[i] = ((rand() % 256) * 256) + (rand() % 256);
	}

	if (pscreenConfig->noise == 0)
		for (i = 0; i < 65536; i++) {
			NoiseTable[i] = 32767;
		}
	else {
		int noise = pscreenConfig->noise;
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

	return (REPLY_OK);
}
