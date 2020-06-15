// ****************************************************************************
//
//	rx_screen.c
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
#include "rx_threads.h"
#include "rx_mem.h"
#include "rx_file.h"
#include "rx_setup_file.h"
#include "rx_screen.h"
#include "rx_luts.h"
#include "rx_tas.h"
#include "rx_screen_fme_4x3.h"
#include "rx_screen_fmb_4x3.h"
#include "rx_screen_fms_1.h"
#include "rx_screen_fms_1x2.h"
#include "rx_screen_fms_1x3.h"

#ifdef linux
	#include "errno.h"
#endif

//--- SScreenThread-------------------------------------------------------------------------
typedef struct
{
	int				running;			// running yes/no
	HANDLE			hWaitEvent;			// handle on wait event
	int				ret;				// ret code
	SPlaneInfo		inplane;			// inputplane info ptr
	SPlaneInfo		outplane;			// inputplane info ptr
	SScreenConfig*	pscreenConfig;		// screen config ptr
} SScreenThread;

SScreenThread screenthread[MAX_COLORS];

//--- SInkLimitThread-------------------------------------------------------------------------
typedef struct
{
	int				running;			// running yes/no
	HANDLE			hWaitEvent;			// handle on wait event
	SBmpInfo		*pinfo;				// bmp ptr
	SScreenConfig*	pscreenConfig;		// screen config ptr
	int				start;				// start of slice
	int				end;				// end of slice
} SInkLimitThread;

SInkLimitThread inklimitthread[THREADS_MAX];

UINT16 RandTable[65536]; // Random values table used for 16 bits to 8 bits depth conversion
int	RandIndex;			 // Rand index in table from 0 to infinite & 0xFFFFF;


//--- _rx_slice_inklimit ----------------------------------------------------------------------
int _rx_slice_inklimit(SBmpInfo *pinfo, SScreenConfig* pscreenConfig, int start, int end)
{
	int p, l, i, Max, Min;
	int /*MinCMY, */Total, ToRemove, PenetrationFluid;

	if (pinfo->bitsPerPixel == 8)
	{
		BYTE * pPlane[MAX_COLORS];
		Max = pscreenConfig->totalInkLimit * 255 / 100;
		Min = pscreenConfig->minPenetrationFluid * 255 / 100;
		for (l = start; l < end; l++)
		{
			for (p = 0; p < pinfo->planes; p++)
			{
				UINT64 deb = (UINT64)l * (UINT64)pinfo->lineLen;
				pPlane[p] = *pinfo->buffer[p] + deb;
			}
			for (i = 0; i < (int)pinfo->srcWidthPx; i++)
			{
				// cumulate
				Total = PenetrationFluid = 0;
				for (p = 0; p < pinfo->planes; p++)
				{
					if (pPlane[p][i]) {
						if ((pscreenConfig->inMaxInk[p]) && (!pscreenConfig->penetrationFluid[p]))
							Total += pPlane[p][i];
						if (pscreenConfig->penetrationFluid[p])
							PenetrationFluid += pPlane[p][i];
					}
				}
				
				// if ink and no penetration fluid - => add mini of penetration fluid
				if ((Total) && (PenetrationFluid == 0)) {
					for (p = 0; p < pinfo->planes; p++)
					{
						if (pscreenConfig->penetrationFluid[p]) {
							pPlane[p][i] = Min;
							PenetrationFluid += pPlane[p][i];
						}
					}
				}

				// Ink limit to apply
				if (Total + PenetrationFluid > Max) 
				{
					// remove First Penetration fluid
					if (PenetrationFluid) {
						ToRemove = (Total + PenetrationFluid) - Max;
						if (ToRemove > PenetrationFluid)
							ToRemove = PenetrationFluid;
						for (p = 0; p < pinfo->planes; p++)
						{
							if ((pscreenConfig->penetrationFluid[p]) && (pPlane[p][i])) {
								if (ToRemove >= pPlane[p][i]) {
									PenetrationFluid -= pPlane[p][i];
									ToRemove -= pPlane[p][i];
									pPlane[p][i] = 0;
								}
								else {
									pPlane[p][i] -= ToRemove;
									PenetrationFluid -= ToRemove;
									ToRemove = 0;
									break;
								}
							}
						}
					}

					// decrease other color if still something to remove
					if (Total + PenetrationFluid > Max) 
					{
						ToRemove = (Total + PenetrationFluid) - Max;
						float ratio = (float)ToRemove / (float)(Total - pPlane[3][i]); // ratio to remove on all color except black
						for (p = 0; p < pinfo->planes; p++)
						{
							if ((p != 3) && (pscreenConfig->inMaxInk[p]) && (pPlane[p][i])) {
								ToRemove = (int)(ratio * (0.5 + pPlane[p][i]));
								pPlane[p][i] -= ToRemove;
							}
						}
					}
				}
			}
		}
	}
	else // i.e. 16 bits per pixel
	{
		UINT16 * pWPlane[MAX_COLORS];
		Max = pscreenConfig->totalInkLimit * 65535 / 100;
		Min = pscreenConfig->minPenetrationFluid * 65535 / 100;
		for (l = start; l < end; l++)
		{
			for (p = 0; p < pinfo->planes; p++)
			{
				pWPlane[p] = (UINT16 *)(*pinfo->buffer[p] + ((UINT64)l * pinfo->lineLen));
			}
			for (i = 0; i < (int)pinfo->srcWidthPx; i++)
			{
				Total = PenetrationFluid = 0;

				// Cumulate
				for (p = 0; p < pinfo->planes; p++)
				{
					if ((pscreenConfig->inMaxInk[p]) && (!pscreenConfig->penetrationFluid[p]))
						Total += pWPlane[p][i];
					if (pscreenConfig->penetrationFluid[p])
						PenetrationFluid += pWPlane[p][i];
				}

				// if ink and no penetration fluid - => add mini of penetration fluid
				if ((Total) && (PenetrationFluid == 0)) {
					for (p = 0; p < pinfo->planes; p++)
					{
						if (pscreenConfig->penetrationFluid[p]) {
							pWPlane[p][i] = Min;
							PenetrationFluid += pWPlane[p][i];
						}
					}
				}

				// Ink limit to apply
				if (Total + PenetrationFluid > Max)
				{
					// remove First Penetration fluid
					if (PenetrationFluid) {
						ToRemove = (Total + PenetrationFluid) - Max;
						if (ToRemove > PenetrationFluid)
							ToRemove = PenetrationFluid;
						for (p = 0; p < pinfo->planes; p++)
						{
							if ((pscreenConfig->penetrationFluid[p]) && (pWPlane[p][i])) {
								if (ToRemove >= pWPlane[p][i]) {
									PenetrationFluid -= pWPlane[p][i];
									ToRemove -= pWPlane[p][i];
									pWPlane[p][i] = 0;
								}
								else {
									pWPlane[p][i] -= ToRemove;
									PenetrationFluid -= ToRemove;
									ToRemove = 0;
									break;
								}
							}
						}
					}

					// decrease other color if still something to remove
					if (Total + PenetrationFluid > Max)
					{
						ToRemove = (Total + PenetrationFluid) - Max;
						float ratio = (float)ToRemove / (float)(Total - pWPlane[3][i]); // ratio to remove on all color except black
						for (p = 0; p < pinfo->planes; p++)
						{
							if ((p != 3) && (pscreenConfig->inMaxInk[p]) && (pWPlane[p][i])) {
								ToRemove = (int)(ratio * (0.5 + pWPlane[p][i]));
								pWPlane[p][i] -= ToRemove;
							}
						}
					}
				}
			}
		}
	}

	return (REPLY_OK);
}


//---  _rx_thread_inklimit_slice ------------------------------------------------------------
static void *_rx_thread_inklimit_slice(void *par)
{
	SInkLimitThread *pinklimitthread = (void *)par;
	pinklimitthread->running = 1;
	_rx_slice_inklimit(pinklimitthread->pinfo, pinklimitthread->pscreenConfig, pinklimitthread->start, pinklimitthread->end);
	pinklimitthread->running = 0;
	rx_sem_post(pinklimitthread->hWaitEvent);

	return NULL;
}


//--- rx_inklimit_bmp ----------------------------------------------------------------------
int rx_inklimit_bmp(SBmpInfo *pinfo, SScreenConfig* pscreenConfig)
{
	int l_threadMax, j, h, start = 0, end = BAND_HEIGHT, slice = BAND_HEIGHT;

	if (!pinfo->planes) return Error(ERR_CONT, 0, "No planes in bitmap");
	if (end > (int)pinfo->lengthPx)
		end = (int)pinfo->lengthPx;
	l_threadMax = rx_get_maxnumthreads(NULL);
	while (start <  (int)pinfo->lengthPx)
	{
		HANDLE hWaitEvent = rx_sem_create();
		if (!hWaitEvent)
			return Error(ERR_CONT, 0, "Error rx_sem_create in rx_inklimit_bmp");

		for (j = 0; j < l_threadMax; j++)
		{
			if (start >= (int)pinfo->lengthPx)
				break;
			inklimitthread[j].pinfo = pinfo;
			inklimitthread[j].pscreenConfig = pscreenConfig;
			inklimitthread[j].start = start;
			inklimitthread[j].end = end;
			inklimitthread[j].running = 0;
			inklimitthread[j].hWaitEvent = hWaitEvent;
			if (start < (int)pinfo->lengthPx)
				rx_thread_start(_rx_thread_inklimit_slice, (void*)&inklimitthread[j], 0, "_rx_thread_inklimit_slice");
			start += slice;
			end += slice;
			if (end >(int)pinfo->lengthPx)
				end = (int)pinfo->lengthPx;
		}
		while (1) {
			rx_sem_wait(hWaitEvent, 1000);
			for (h = 0; h < j; h++) {
				if (inklimitthread[h].running)
					break;
			}
			if (h == j)
				break;
		}
		rx_sem_destroy(&hWaitEvent);
	}

	return (REPLY_OK);
}



//---  _rx_thread_lut_plane ------------------------------------------------------------
static void *_rx_thread_lut_plane(void *par)
{
	SScreenThread *pscreenthread = (void *)par;
	pscreenthread->running = 1;
	pscreenthread->ret= rx_lut_plane(&pscreenthread->inplane, pscreenthread->pscreenConfig);
	pscreenthread->running = 0;
	rx_sem_post(pscreenthread->hWaitEvent);

	return NULL;
}



//--- rx_lut_bmp ----------------------------------------------------------------------
int rx_lut_bmp(SBmpInfo *pinfo, SScreenConfig* pscreenConfig)
{
	int iCntPlane, l_threadMax, j, i;
	int ret = REPLY_OK;

	if (pscreenConfig->method == 3) // No screening
		return (ret);
	if (!pinfo->planes) return Error(ERR_CONT, 0, "No planes in bitmap");

	for (j = 0; j < pinfo->planes; j++)
	{
		screenthread[j].ret = REPLY_OK;
		screenthread[j].running = 0;
	}
	iCntPlane = 0;
	l_threadMax = rx_get_maxnumthreads(NULL);
	for (i = 0; i < pinfo->planes; i += l_threadMax)
	{
		HANDLE hWaitEvent = rx_sem_create();
		if (!hWaitEvent)
			return Error(ERR_CONT, 0, "Error rx_sem_create in rx_lut_bmp");

		for (j = i; j < i + l_threadMax; j++)
		{
			if (iCntPlane >= pinfo->planes)
				break;
			screenthread[j].inplane.aligment = pinfo->aligment;
			screenthread[j].inplane.WidthPx = pinfo->srcWidthPx;
			screenthread[j].inplane.lengthPx = pinfo->lengthPx;
			screenthread[j].inplane.bitsPerPixel = pinfo->bitsPerPixel;
			screenthread[j].inplane.lineLen = pinfo->lineLen;
			screenthread[j].inplane.dataSize = pinfo->dataSize;
			screenthread[j].inplane.resol = pinfo->resol;
			screenthread[j].inplane.planenumber = iCntPlane;
			screenthread[j].inplane.buffer = *pinfo->buffer[iCntPlane];

			screenthread[j].running = 1;
			screenthread[j].hWaitEvent = hWaitEvent;
			screenthread[j].pscreenConfig = pscreenConfig;
			rx_thread_start(_rx_thread_lut_plane, (void*)&screenthread[j], 0, "_rx_thread_lut_plane");
			iCntPlane++;
		}

		while (1) {
			rx_sem_wait(hWaitEvent, INFINITE);
			for (j = 0; j < iCntPlane; j++) {
				if (screenthread[j].running) {
					break;
				}
			}
			if (j == iCntPlane)
				break;
		}
		rx_sem_destroy(&hWaitEvent);
	}

	pinfo->bitsPerPixel = screenthread[0].inplane.bitsPerPixel;
	pinfo->lineLen		= screenthread[0].inplane.lineLen;
	pinfo->dataSize		= screenthread[0].inplane.dataSize;
	for (j = 0; j < pinfo->planes; j++)
	{
		pinfo->buffer[j] = &screenthread[j].inplane.buffer;
		if (screenthread[j].ret != REPLY_OK)
			ret = screenthread[j].ret;
	}

	return (ret);
}


//--- rx_lut_plane ----------------------------------------------------------------------
int rx_lut_plane(SPlaneInfo *inplane, SScreenConfig* pscreenConfig)
{
	UINT32 l, i;


	if (inplane->buffer == NULL) return Error(ERR_CONT, 0, "No input buffer for rx_lut_plane: %d", inplane->planenumber);
	if (inplane->bitsPerPixel == 8)
	{
		if (pscreenConfig->inputbitsPerPixel == 16) // 8 bits to 16 bits
		{
			PBYTE newbuffer;
			BYTE *pSrc;
			UINT16 *lut16 = pscreenConfig->lut16[inplane->planenumber];
			UINT16 *pDst16;
			newbuffer = rx_mem_alloc(inplane->dataSize * 2);
			if (newbuffer == NULL) return Error(ERR_CONT, 0, "No output buffer for rx_lut_plane: %d", inplane->planenumber);
			memset(newbuffer, 0, inplane->dataSize * 2);
			for (l = 0; l < inplane->lengthPx; l++)
			{
				pSrc = inplane->buffer + ((UINT64)l * inplane->lineLen);
				pDst16 = (UINT16*)(newbuffer + ((UINT64)l * inplane->lineLen * 2));
				 {
					for (i = 0; i < (int)inplane->WidthPx; i++)
					{
						if (pSrc[i])
							pDst16[i] = lut16[pSrc[i] * 257];
					}
				}
			}
			inplane->bitsPerPixel = 16;
			inplane->dataSize *= 2;
			inplane->lineLen *= 2;
			rx_mem_free(&inplane->buffer);
			inplane->buffer = newbuffer;
		}

		else // 8 bits to 8 bits
		{

			BYTE *lut = pscreenConfig->lut[inplane->planenumber];
			BYTE *pSrc;
			for (l = 0; l < inplane->lengthPx; l++)
			{
				pSrc = inplane->buffer + ((UINT64)l * inplane->lineLen);
				for (i = 0; i < (int)inplane->WidthPx; i++)
				{
					if (pSrc[i])
						pSrc[i] = lut[pSrc[i]];
				}
			}
		}
	}
	else
	{
		if (pscreenConfig->inputbitsPerPixel == 16) // 16 bits to 16 bits
		{
			UINT16 *lut16 = pscreenConfig->lut16[inplane->planenumber];
			UINT16 *pSrc16;
			for (l = 0; l < inplane->lengthPx; l++)
			{
				pSrc16 = (UINT16*)(inplane->buffer + ((UINT64)l * inplane->lineLen));
				for (i = 0; i < (int)inplane->WidthPx; i++)
				{
					if (pSrc16[i])
						pSrc16[i] = lut16[pSrc16[i]];
				}
			}
		}
	
		else // 16 bits to 8 bits
		{
			int rindex = (rand() % 256) *257;
			PBYTE newbuffer;
			UINT16 *pSrc16;
			BYTE *pDst;
			UINT16 *lut16 = pscreenConfig->lut16[inplane->planenumber];

			newbuffer = rx_mem_alloc(inplane->dataSize / 2);
			if (newbuffer == NULL) return Error(ERR_CONT, 0, "No output buffer for rx_screen_plane: %d", inplane->planenumber);
			memset(newbuffer, 0, inplane->dataSize / 2);
			for (l = 0; l < (int)inplane->lengthPx; l++)
			{
				pSrc16 = (UINT16 *)(inplane->buffer + (l * inplane->lineLen));
				pDst = newbuffer + (l * inplane->lineLen / 2);
				for (i = 0; i < (int)inplane->WidthPx; i++)
				{
					if (pSrc16[i]) {
						int val = (int)lut16[pSrc16[i]] + RandTable[rindex++&(0xFFFF)];
						if (val < 65536)
							pDst[i] = (BYTE)(val / 256);
						else
							pDst[i] = 255;
					}
				}
			}
			inplane->bitsPerPixel = 8;
			inplane->dataSize /= 2;
			inplane->lineLen /= 2;
			rx_mem_free(&inplane->buffer);
			inplane->buffer = newbuffer;
		}
	}

	return (REPLY_OK);
}



//---  _rx_thread_screen_plane ------------------------------------------------------------
static void *_rx_thread_screen_plane(void *par)
{
	SScreenThread *pscreenthread = (void *)par;
	pscreenthread->running = 1;
	pscreenthread->ret= rx_screen_plane(&pscreenthread->inplane, &pscreenthread->outplane, pscreenthread->pscreenConfig);
	if (pscreenthread->inplane.buffer) {
		rx_mem_free(&pscreenthread->inplane.buffer);
		pscreenthread->inplane.buffer = NULL;
	}
	pscreenthread->running = 0;
	rx_sem_post(pscreenthread->hWaitEvent);

	return NULL;
}


//--- rx_screen_bmp ----------------------------------------------------------------------
int rx_screen_bmp (SBmpInfo *pinfo, SScreenConfig* pscreenConfig)
{
	int iCntPlane, l_threadMax, i, j;
	int ret=REPLY_OK;
	HANDLE hWaitEvent;

	if (pscreenConfig->method == 3) // No screening
		return (ret);
	if ((pscreenConfig->screenByPrinter) && (pinfo->bitsPerPixel== pscreenConfig->outputbitsPerPixel) &&
		(pinfo->resol.x == pscreenConfig->outputResol.x) && (pinfo->resol.y == pscreenConfig->outputResol.y)) // printer screening with expected resol and bitsperpixel screening
		return (ret);
	if (!pinfo->planes) return Error(ERR_CONT, 0, "No planes in bitmap");

	iCntPlane = 0;
	l_threadMax = rx_get_maxnumthreads(NULL);
	for (j = 0; j < pinfo->planes; j++)
	{
		screenthread[j].ret = REPLY_OK;
		screenthread[j].running = 0;
	}
	for (i = 0; i < pinfo->planes; i+= l_threadMax)
	{
		hWaitEvent = rx_sem_create();
		if (!hWaitEvent)
			return Error(ERR_CONT, 0, "Error rx_sem_create in rx_screen_bmp");
		for (j = i; j < i + l_threadMax; j++)
		{
			if (iCntPlane >= pinfo->planes)
				break;
			screenthread[j].inplane.aligment = pinfo->aligment;
			screenthread[j].inplane.WidthPx = pinfo->srcWidthPx;
			screenthread[j].inplane.lengthPx = pinfo->lengthPx;
			screenthread[j].inplane.bitsPerPixel = pinfo->bitsPerPixel;
			screenthread[j].inplane.lineLen = pinfo->lineLen;
			screenthread[j].inplane.dataSize = pinfo->dataSize;
			screenthread[j].inplane.resol = pinfo->resol;
			screenthread[j].inplane.planenumber = iCntPlane;
			screenthread[j].inplane.buffer = *pinfo->buffer[iCntPlane];

			screenthread[j].running = 1;
			screenthread[j].hWaitEvent = hWaitEvent;
			screenthread[j].pscreenConfig = pscreenConfig;
			rx_thread_start(_rx_thread_screen_plane, (void*)&screenthread[j], 0, "_rx_thread_screen_plane");
			iCntPlane++;
		}

		while (1) {
			rx_sem_wait(hWaitEvent, INFINITE);
			for (j = 0; j < iCntPlane; j++) {
				if (screenthread[j].running)
					break;
			}
			if (j == iCntPlane)
				break;
		}
		rx_sem_destroy(&hWaitEvent);
	}

	pinfo->srcWidthPx	= screenthread[0].outplane.WidthPx;
	pinfo->lengthPx		= screenthread[0].outplane.lengthPx;
	pinfo->bitsPerPixel = screenthread[0].outplane.bitsPerPixel;
	pinfo->lineLen		= screenthread[0].outplane.lineLen;
	pinfo->dataSize		= screenthread[0].outplane.dataSize;
	pinfo->resol		= screenthread[0].outplane.resol;
	for (j = 0; j < pinfo->planes; j++)
	{
		if (screenthread[j].ret != REPLY_OK)
			ret = screenthread[j].ret;
		pinfo->buffer[j] = &screenthread[j].outplane.buffer;
		pinfo->DropCount[j][0] = screenthread[j].outplane.DropCount[0];
		pinfo->DropCount[j][1] = screenthread[j].outplane.DropCount[1];
		pinfo->DropCount[j][2] = screenthread[j].outplane.DropCount[2];
	}

	return (ret);
}



//--- rx_screen_plane ----------------------------------------------------------------------
int rx_screen_plane (SPlaneInfo *inplane, SPlaneInfo *outplane, SScreenConfig* pscreenConfig)
{

	if (inplane->buffer == NULL) return Error(ERR_CONT, 0, "No output buffer for rx_screen_plane: %d", inplane->planenumber);

	if (pscreenConfig->fctplane)
		return(pscreenConfig->fctplane(inplane, outplane, pscreenConfig));
	else
	{
		Error(ERR_CONT, 0, "Screen type %d: <%s> not supported", pscreenConfig->type, pscreenConfig->typeName);
		return (REPLY_ERROR);
	}

}


//--- rx_missinglines_add ----------------------------------------------------------------------
int rx_missinglines_add(SBmpInfo *pinfo, SScreenConfig* pscreenConfig)
{
	int i, l ;
	int ret = REPLY_OK;

	if (!pscreenConfig->missinglinecpt) return REPLY_OK;
	if (!pinfo->planes) return Error(ERR_CONT, 0, "No planes in bitmap");

	for (i = 0; i < pscreenConfig->missinglinecpt; i++) {
		if ((pscreenConfig->missingline[i][0] < pinfo->planes) && (pscreenConfig->missingline[i][1]< (int)pinfo->srcWidthPx))
		{
			if (pinfo->bitsPerPixel == 16) {
				UINT16 *pSrc16;
				for (l = 0; l < (int)pinfo->lengthPx; l++)
				{
					pSrc16 = (UINT16*)(*pinfo->buffer[pscreenConfig->missingline[i][0]] + ((UINT64)l * pinfo->lineLen) + ((UINT64)pscreenConfig->missingline[i][1] * 2));
					*pSrc16 = 0;
				}
			}
			else { // 1, 2, 4, 8

				BYTE mask, *pSrc;
				int byteNumber = pscreenConfig->missingline[i][1] * pinfo->bitsPerPixel / 8;
				if (pinfo->bitsPerPixel == 8) mask = 0xFF;
				else if (pinfo->bitsPerPixel == 4)
					mask = 0xF0 >> pscreenConfig->missingline[i][1] % 2;
				else if (pinfo->bitsPerPixel == 2)
					mask = 0xC0 >> pscreenConfig->missingline[i][1] % 4;
				else if (pinfo->bitsPerPixel == 1)
					mask = 0x80 >> pscreenConfig->missingline[i][1] % 8;
				else
					return Error(ERR_CONT, 0, "Bad bitsPerPixel: %d in bitmap", pinfo->bitsPerPixel);
				for (l = 0; l < (int) pinfo->lengthPx; l++)
				{
					pSrc = (BYTE*)(*pinfo->buffer[pscreenConfig->missingline[i][0]] + ((UINT64)l * pinfo->lineLen) + ((UINT64)byteNumber));
					*pSrc &= ~mask;
				}
			}
		}
	}
	return (ret);
}


void _RatioFill(HANDLE file, SRatio * ratio) {
	SRatio lratio;

	setup_uint32(file, "Input", READ, &lratio.input, ratio->input);
	if (lratio.input > 100) lratio.input = 100;
	setup_uint32(file, "Small", READ, &lratio.smallD, ratio->smallD);
	if (lratio.smallD > 100) lratio.smallD = 100;
	setup_uint32(file, "Medium", READ, &lratio.mediumD, ratio->mediumD);
	if (lratio.mediumD > 100) lratio.mediumD = 100;
	setup_uint32(file, "Large", READ, &lratio.largeD, ratio->largeD);
	if (lratio.largeD > 100) lratio.largeD = 100;
	*ratio = lratio;
}


//--- rx_screen_load ------------------------------------------------------------
int rx_screen_load(const char * path, SScreenConfig* pscreenConfig)
{
	int i, planenumber, rationumber, first, second;
	char Name[32];
	HANDLE  file;
	file = setup_create();
	setup_load(file, path);
	if (setup_chapter(file, "PrinterEnvSet", -1, READ) != REPLY_OK)
		return Error(ERR_CONT, 0, "Cannot find PrinterEnvSet item in file %s", path);

	if (setup_chapter(file, "Screen", -1, READ) != REPLY_OK)
		return Error(ERR_CONT, 0, "Cannot find Screen item in file %s", path);
	setup_uint32(file, "Type", READ, (unsigned int *)&pscreenConfig->type, 2);
	switch (pscreenConfig->type)
	{
	case 0://FME_1x3x2
		pscreenConfig->method = 0;			/* error diffusion */
		pscreenConfig->dropsize = 123;		/* 123 */
		pscreenConfig->fillmethod = 1;		/* fill and grow */
		pscreenConfig->inputResol.x = pscreenConfig->inputResol.y = 600;
		pscreenConfig->outputResol.x = pscreenConfig->outputResol.y = 1200;
		pscreenConfig->outputbitsPerPixel = 2;
		break;
	case 1://FMB_4x3
		pscreenConfig->method = 1;			/* Bayer */
		pscreenConfig->dropsize = 123;		/* 123 */
		pscreenConfig->fillmethod = 1;		/* fill and grow */
		pscreenConfig->inputResol.x = pscreenConfig->inputResol.y = 600;
		pscreenConfig->outputResol.x = pscreenConfig->outputResol.y = 1200;
		pscreenConfig->outputbitsPerPixel = 2;
		break;
	default:
	case 2://FMS_1x1
		pscreenConfig->method = 2;			/* Threshold */
		pscreenConfig->dropsize = 1;		/* 1 */
		pscreenConfig->fillmethod = 1;		/* fill and grow */
		pscreenConfig->inputResol.x = pscreenConfig->inputResol.y = 1200;
		pscreenConfig->outputResol.x = pscreenConfig->outputResol.y = 1200;
		//		pscreenConfig->outputbitsPerPixel = 1;
		pscreenConfig->outputbitsPerPixel = 2;
		break;
	case 3:// FMS_1x3s
		pscreenConfig->method = 2;			/* Threshold */
		pscreenConfig->dropsize = 123;		/* 123 */
		pscreenConfig->fillmethod = 0;		/* fill then grow */
		pscreenConfig->inputResol.x = pscreenConfig->inputResol.y = 1200;
		pscreenConfig->outputResol.x = pscreenConfig->outputResol.y = 1200;
		pscreenConfig->outputbitsPerPixel = 2;
		break;
	case 4:// FMS_1x3g
		pscreenConfig->method = 2;			/* Threshold */
		pscreenConfig->dropsize = 123;		/* 123 */
		pscreenConfig->fillmethod = 1;		/* fill and grow */
		pscreenConfig->inputResol.x = pscreenConfig->inputResol.y = 1200;
		pscreenConfig->outputResol.x = pscreenConfig->outputResol.y = 1200;
		pscreenConfig->outputbitsPerPixel = 2;
		break;
	case 5:// FMS_1x3sx2
		pscreenConfig->method = 2;			/* Threshold */
		pscreenConfig->dropsize = 123;		/* 123 */
		pscreenConfig->fillmethod = 0;		/* fill then grow */
		pscreenConfig->inputResol.x = pscreenConfig->inputResol.y = 600;
		pscreenConfig->outputResol.x = pscreenConfig->outputResol.y = 1200;
		pscreenConfig->outputbitsPerPixel = 2;
		break;
	case 6:// FMS_1x3gx2
		pscreenConfig->method = 2;			/* Threshold */
		pscreenConfig->dropsize = 123;		/* 123 */
		pscreenConfig->fillmethod = 1;		/* fill and grow */
		pscreenConfig->inputResol.x = pscreenConfig->inputResol.y = 600;
		pscreenConfig->outputResol.x = pscreenConfig->outputResol.y = 1200;
		pscreenConfig->outputbitsPerPixel = 2;
		break;
	case 7:// FMS_1x2s
		pscreenConfig->method = 2;			/* Threshold */
		pscreenConfig->dropsize = 12;		/* 12 */
		pscreenConfig->fillmethod = 0;		/* fill then grow */
		pscreenConfig->inputResol.x = pscreenConfig->inputResol.y = 1200;
		pscreenConfig->outputResol.x = pscreenConfig->outputResol.y = 1200;
		pscreenConfig->outputbitsPerPixel = 2;
		break;
	case 8:// FMS_1x2g
		pscreenConfig->method = 2;			/* Threshold */
		pscreenConfig->dropsize = 12;		/* 12 */
		pscreenConfig->fillmethod = 0;		/* fill then grow */
		pscreenConfig->inputResol.x = pscreenConfig->inputResol.y = 1200;
		pscreenConfig->outputResol.x = pscreenConfig->outputResol.y = 1200;
		pscreenConfig->outputbitsPerPixel = 2;
		break;
	case 9:// FMS_1x2gx2
		pscreenConfig->method = 2;			/* Threshold */
		pscreenConfig->dropsize = 12;		/* 12 */
		pscreenConfig->fillmethod = 0;		/* fill then grow */
		pscreenConfig->inputResol.x = pscreenConfig->inputResol.y = 600;
		pscreenConfig->outputResol.x = pscreenConfig->outputResol.y = 1200;
		pscreenConfig->outputbitsPerPixel = 2;
		break;
	case 10:// FMS_1x2gx2
		pscreenConfig->method = 2;			/* Threshold */
		pscreenConfig->dropsize = 12;		/* 12 */
		pscreenConfig->fillmethod = 1;		/* fill and grow */
		pscreenConfig->inputResol.x = pscreenConfig->inputResol.y = 600;
		pscreenConfig->outputResol.x = pscreenConfig->outputResol.y = 1200;
		pscreenConfig->outputbitsPerPixel = 2;
		break;
	case 11:// FMS_1x1sx2
		pscreenConfig->method = 2;			/* Threshold */
		pscreenConfig->dropsize = 1;		/* 1 */
		pscreenConfig->fillmethod = 0;		/* fill then grow */
		pscreenConfig->inputResol.x = pscreenConfig->inputResol.y = 600;
		pscreenConfig->outputResol.x = pscreenConfig->outputResol.y = 1200;
		//		pscreenConfig->outputbitsPerPixel = 1;
		pscreenConfig->outputbitsPerPixel = 2;
		break;
	case 12:///FMS_1x1gx2
		pscreenConfig->method = 2;			/* Threshold */
		pscreenConfig->dropsize = 1;		/* 1 */
		pscreenConfig->fillmethod = 0;		/* fill and grow */
		pscreenConfig->inputResol.x = pscreenConfig->inputResol.y = 600;
		pscreenConfig->outputResol.x = pscreenConfig->outputResol.y = 1200;
		//		pscreenConfig->outputbitsPerPixel = 1;
		pscreenConfig->outputbitsPerPixel = 2;
		break;
	}
	setup_uint32(file, "Method", READ, (unsigned int *)&pscreenConfig->method, pscreenConfig->method);
	if ((pscreenConfig->method != 0) && (pscreenConfig->method != 1) && (pscreenConfig->method != 2) && (pscreenConfig->method != 3))
		pscreenConfig->method = 2;
	setup_uint32(file, "InputResol", READ, (unsigned int *)&pscreenConfig->inputResol.x, pscreenConfig->inputResol.x);
	if ((pscreenConfig->inputResol.x != 300) && (pscreenConfig->inputResol.x != 600) && (pscreenConfig->inputResol.x != 1200))
		pscreenConfig->inputResol.x = 1200;
	pscreenConfig->inputResol.y = pscreenConfig->inputResol.x;
	setup_uint32(file, "DropSize", READ, (unsigned int *)&pscreenConfig->dropsize, pscreenConfig->dropsize);
	if ((pscreenConfig->dropsize != 1) && (pscreenConfig->dropsize != 2) && (pscreenConfig->dropsize != 3) && (pscreenConfig->dropsize != 12) && (pscreenConfig->dropsize != 13) && (pscreenConfig->dropsize != 23) && (pscreenConfig->dropsize != 123))
		pscreenConfig->dropsize = 1;
	setup_uint32(file, "FillMethod", READ, (unsigned int *)&pscreenConfig->fillmethod, 1);
	if ((pscreenConfig->fillmethod < 0) && (pscreenConfig->fillmethod > 2))
		pscreenConfig->fillmethod = 1;
	setup_uint32(file, "DropCount", READ, (unsigned int *)&pscreenConfig->dropcount, 0);
	if ((pscreenConfig->dropcount != 0) && (pscreenConfig->dropcount != 1))
		pscreenConfig->dropcount = 0;
	setup_uint32(file, "ScreenByPrinter", READ, (unsigned int *)&pscreenConfig->screenByPrinter, 0);
	if ((pscreenConfig->screenByPrinter != 0) && (pscreenConfig->screenByPrinter != 1))
		pscreenConfig->screenByPrinter = 0;
	if (pscreenConfig->screenByPrinter)
		pscreenConfig->dropcount = 0;
	setup_uint32(file, "Output", READ, (unsigned int *)&pscreenConfig->output, 0);
	if ((pscreenConfig->output < 0) || (pscreenConfig->output > 5))
		pscreenConfig->output = 0;
	setup_uint32(file, "BitsPerPixel", READ, (unsigned int *)&pscreenConfig->inputbitsPerPixel, 8);
	if ((pscreenConfig->inputbitsPerPixel != 8) && (pscreenConfig->inputbitsPerPixel != 8))
		pscreenConfig->inputbitsPerPixel = 16;
	setup_uint32(file, "TotalInkLimit", READ, (unsigned int *)&pscreenConfig->totalInkLimit, 1600);
	if ((pscreenConfig->totalInkLimit < 100) || (pscreenConfig->totalInkLimit > 1600))
		pscreenConfig->totalInkLimit = 1600;
	setup_uint32(file, "MinPenetrationFluid", READ, (unsigned int *)&pscreenConfig->minPenetrationFluid, 0);
	if ((pscreenConfig->minPenetrationFluid < 0) || (pscreenConfig->minPenetrationFluid > 400))
		pscreenConfig->minPenetrationFluid = 0;
	setup_uint32(file, "OverSampling", READ, (unsigned int *)&pscreenConfig->overSampling, 0);
	if ((pscreenConfig->overSampling < 0) || (pscreenConfig->overSampling > 3))
		pscreenConfig->overSampling = 0;
	setup_uint32(file, "AntiBleeding", READ, (unsigned int *)&pscreenConfig->antiBleeding, 0);
	if ((pscreenConfig->antiBleeding < 0) || (pscreenConfig->antiBleeding > 4))
		pscreenConfig->antiBleeding = 0;
	setup_uint32(file, "Noise", READ, (unsigned int *)&pscreenConfig->noise, 0);
	if ((pscreenConfig->noise < 0) || (pscreenConfig->noise > 100))
		pscreenConfig->noise = 0;

	for (i = 0; i < MAX_COLORS; i++) {
		if (pscreenConfig->dropsize == 123) {
			pscreenConfig->limit[i][0] = 33;
			pscreenConfig->limit[i][1] = 67;
		}
		else { 
			pscreenConfig->limit[i][0] = 50; //will be used if 12
			pscreenConfig->limit[i][1] = 50; // not used
		}

	}

	setup_chapter(file, "..", -1, READ);
	if (setup_chapter(file, "ScreenLimitSet", -1, READ) != REPLY_OK) goto Next1;
	if (setup_chapter(file, "ScreenLimit", -1, READ) != REPLY_OK) {
		setup_chapter(file, "..", -1, READ);
		goto Next1;
	}
	while (1) {
		setup_int32(file, "PlaneNumber", READ, &planenumber, -1);
		setup_uint32(file, "First", READ, &first, pscreenConfig->limit[0][0]);
		if (first < 1) first = 1;
		if (first > 98) first = 98;
		setup_uint32(file, "Second", READ, &second, pscreenConfig->limit[0][1]);
		if (second < first + 1) second = first + 1;
		if (second > 99) second = 99;
		if (planenumber < 0) {
			for (i = 0; i < MAX_COLORS; i++) {
				pscreenConfig->limit[i][0] = first;
				pscreenConfig->limit[i][1] = second;
			}
		}
		else if ((planenumber>=0) && (planenumber<MAX_COLORS))
		{
			pscreenConfig->limit[planenumber][0] = first;
			pscreenConfig->limit[planenumber][1] = second;
		}
		setup_chapter_next(file, READ, Name, sizeof(Name));
		if (strcmp(Name, "ScreenLimit")) {
			setup_chapter(file, "..", -1, READ);
			break;
		}
	}

	setup_chapter(file, "..", -1, READ);


Next1: // Init ratios

	if (pscreenConfig->dropsize < 4) { // 1 drop size config only r100
		pscreenConfig->ratio[0].r100.input = 100;
		if (pscreenConfig->dropsize == 1)
			pscreenConfig->ratio[0].r100.smallD = 100;
		else  if (pscreenConfig->dropsize == 2)
			pscreenConfig->ratio[0].r100.mediumD = 100;
		else // 3
			pscreenConfig->ratio[0].r100.largeD = 100;
		pscreenConfig->ratio[0].r3 = pscreenConfig->ratio[0].r2 = pscreenConfig->ratio[0].r1 = pscreenConfig->ratio[0].r100;
	}

	else if (pscreenConfig->dropsize < 24) { // 2 drop sizes: config R100 then R1 and R2
		pscreenConfig->ratio[0].r100.input = 100;
		if (pscreenConfig->dropsize == 12)
			pscreenConfig->ratio[0].r100.mediumD = 100;
		else // 13 or 23
			pscreenConfig->ratio[0].r100.largeD = 100;

		pscreenConfig->ratio[0].r1.input = 25; // 1st slice 25% 
		if (pscreenConfig->dropsize == 23)
			pscreenConfig->ratio[0].r1.mediumD = 25;
		else // 12 or 13
			pscreenConfig->ratio[0].r1.smallD = 25;

		pscreenConfig->ratio[0].r2.input = 75; // 2nd slice 25% to 75% 
		if (pscreenConfig->dropsize == 23)
			pscreenConfig->ratio[0].r2.mediumD = 25;
		else // 12 or 13
			pscreenConfig->ratio[0].r2.smallD = 25;
		if (pscreenConfig->dropsize == 12)
			pscreenConfig->ratio[0].r2.mediumD = 50;
		else // 13 or 23
			pscreenConfig->ratio[0].r2.largeD = 50;
		pscreenConfig->ratio[0].r3 = pscreenConfig->ratio[0].r2; // r3 no effect
	}
	else  { // 3 drop sizes: config R100 then R1, R2 and R3

		pscreenConfig->ratio[0].r1.input = 25; // 1st slice 25% 
		pscreenConfig->ratio[0].r1.smallD = 25;

		pscreenConfig->ratio[0].r2.input = 50; // 2nd slice 25% to 50% 
		pscreenConfig->ratio[0].r2.smallD = 25;
		pscreenConfig->ratio[0].r2.mediumD = 25;

		pscreenConfig->ratio[0].r2.input = 75; // 3rd slice 50% to 75% 
		pscreenConfig->ratio[0].r2.smallD = 25;
		pscreenConfig->ratio[0].r2.mediumD = 25;
		pscreenConfig->ratio[0].r100.largeD = 25;

		pscreenConfig->ratio[0].r100.input = 100; // last slice 75% to 100%
		pscreenConfig->ratio[0].r100.largeD = 75;
		pscreenConfig->ratio[0].r100.mediumD = 25;
	}
	for (i = 1; i < MAX_COLORS; i++) {
		pscreenConfig->ratio[i] = pscreenConfig->ratio[0];
		pscreenConfig->ratio[i] = pscreenConfig->ratio[0];
		pscreenConfig->ratio[i] = pscreenConfig->ratio[0];
		pscreenConfig->ratio[i] = pscreenConfig->ratio[0];
	}

// Fill ratio
	if (setup_chapter(file, "RatioSet", -1, READ) != REPLY_OK) goto Next2;
	if (setup_chapter(file, "RatioForPlane", -1, READ) != REPLY_OK) {
		setup_chapter(file, "..", -1, READ);
		goto Next2;
	}

	while (1) {
		setup_int32(file, "PlaneNumber", READ, &planenumber, -1);
		setup_int32(file, "RatioNumber", READ, &rationumber, 100);
		if (planenumber < 0) {
			if (rationumber == 100) {
				_RatioFill(file, &pscreenConfig->ratio[0].r100);
				for (i = 1; i < MAX_COLORS; i++) 
					pscreenConfig->ratio[i].r100 = pscreenConfig->ratio[0].r100;
			}
			else if (rationumber == 1) {
				_RatioFill(file, &pscreenConfig->ratio[0].r1);
				for (i = 1; i < MAX_COLORS; i++)
					pscreenConfig->ratio[i].r1 = pscreenConfig->ratio[0].r1;
			}
			else if (rationumber == 2) {
				_RatioFill(file, &pscreenConfig->ratio[0].r2);
				for (i = 1; i < MAX_COLORS; i++)
					pscreenConfig->ratio[i].r2 = pscreenConfig->ratio[0].r2;
			}
			else if (rationumber == 3) {
				_RatioFill(file, &pscreenConfig->ratio[0].r3);
				for (i = 1; i < MAX_COLORS; i++)
					pscreenConfig->ratio[i].r3 = pscreenConfig->ratio[0].r3;
			}
		}
		else {
			if (rationumber == 100) {
				_RatioFill(file, &pscreenConfig->ratio[planenumber].r100);
			}
			else if (rationumber == 1) {
				_RatioFill(file, &pscreenConfig->ratio[planenumber].r1);
			}
			else if (rationumber == 2) {
				_RatioFill(file, &pscreenConfig->ratio[planenumber].r2);
			}
			else if (rationumber == 3) {
				_RatioFill(file, &pscreenConfig->ratio[planenumber].r3);
			}
		}

		setup_chapter_next(file, READ, Name, sizeof(Name));
		if (strcmp(Name, "RatioForPlane")) {
			setup_chapter(file, "..", -1, READ);
			break;
		}
	}
	if (pscreenConfig->dropsize < 4) {
		for (i = 0; i < MAX_COLORS; i++) {
			pscreenConfig->ratio[i].r3 = pscreenConfig->ratio[i].r2 = pscreenConfig->ratio[i].r1 = pscreenConfig->ratio[i].r100;
		}
	}
	else {
		for (i = 0; i < MAX_COLORS; i++) {
			if (pscreenConfig->ratio[i].r1.input==0) // r1 not set
				pscreenConfig->ratio[i].r3= pscreenConfig->ratio[i].r2 = pscreenConfig->ratio[i].r1 = pscreenConfig->ratio[i].r100;
			if (pscreenConfig->ratio[i].r2.input == 0) // r2 not set
				pscreenConfig->ratio[i].r3= pscreenConfig->ratio[i].r2 = pscreenConfig->ratio[i].r1;
			if (pscreenConfig->ratio[i].r3.input == 0) // r2 not set
				pscreenConfig->ratio[i].r3 = pscreenConfig->ratio[i].r2;
		}
	}
	setup_chapter(file, "..", -1, READ);



Next2: // Missing line emulation
	pscreenConfig->missinglinecpt = 0;
	if (setup_chapter(file, "MissingLineSet", -1, READ) != REPLY_OK) goto End;
	if (setup_chapter(file, "MissingLine", -1, READ) != REPLY_OK) goto End;
	while (pscreenConfig->missinglinecpt <32) {
		setup_uint32(file, "Plane", READ, &pscreenConfig->missingline[pscreenConfig->missinglinecpt][0], 0);
		setup_uint32(file, "Line", READ, &pscreenConfig->missingline[pscreenConfig->missinglinecpt][1], 0);
		pscreenConfig->missinglinecpt++;
		setup_chapter_next(file, READ, Name, sizeof(Name));
		if (strcmp(Name, "MissingLine")) break;
	}

End:
	setup_destroy(file);
	return (REPLY_OK);
}


//--- rx_screen_init ----------------------------------------------------------------------
int rx_screen_init(const char * jobSettingsDir, const char * printEnvDir, SColorSpaceConfig *pcs, int pattern, SScreenConfig* pscreenConfig)
{
	char filepath [MAX_PATH];
	int c, i, lnbpenetrationFluid= 0;
	
	memset(pscreenConfig, 0, sizeof (SScreenConfig));

	sprintf(filepath, "%s\\prEnvSettings.xml", jobSettingsDir);
	if (!rx_file_exists(filepath))
		sprintf(filepath, "%s\\settings.xml", printEnvDir);
	if (rx_screen_load(filepath, pscreenConfig))
		Error(WARN, 0, "Screen parameters file <%s> maybe inconsistent", filepath);

	// correct resol if Printer resol is nort 1200 x 1200
	if (pscreenConfig->outputResol.x != pcs->resol.x) {
		pscreenConfig->inputResol.x = pscreenConfig->inputResol.x * pcs->resol.x / pscreenConfig->outputResol.x;
		pscreenConfig->outputResol.x = pcs->resol.x;
	}
	if (pscreenConfig->outputResol.y != pcs->resol.y) {
		pscreenConfig->inputResol.y = pscreenConfig->inputResol.y * pcs->resol.y / pscreenConfig->outputResol.y;
		pscreenConfig->outputResol.y = pcs->resol.y;
	}

	for (i = 0; i < 65536; i++) {
		RandTable[i] = (rand() % 256);
	}

	// init lut tables
	sprintf(filepath, "%s\\profile.ted", printEnvDir);
	if (rx_luts_load(filepath, pcs, pscreenConfig->lut, pscreenConfig->lut16))
		Error(WARN, 0, "Lut tables file <%s> maybe inconsistent", filepath);

	if (pattern==1) { // Line pattern
		pscreenConfig->totalInkLimit = 1600;
	}
	else {
		sprintf(filepath, "%s\\lastadjust.cfg", printEnvDir);
		if (rx_file_exists(filepath)) {
			float adjust[MAX_COLORS];
			rx_adjust_load(filepath, pcs, adjust);
			rx_luts_adjust(pcs, adjust, pscreenConfig->lut, pscreenConfig->lut16);
		}
	}

	if ((pscreenConfig->method >= 2) && (pscreenConfig->screenByPrinter==0))// need ta ? 
	{
		sprintf(filepath, "%s\\TA.dat", printEnvDir);
		if (!rx_file_exists(filepath)) {
			sprintf(filepath, "%s\\../../TA/TA.dat", printEnvDir); // find generic
			if (!rx_file_exists(filepath)) {
				Error(ERR_CONT, 0, "Threshold arrays file <%s> not found", filepath);
				return (REPLY_ERROR);
			}
		}

		// init threshold array
		if (rx_tas_load(filepath, &pscreenConfig->TA[0])) {
			Error(ERR_CONT, 0, "Threshold arrays file <%s> are inconsistent", filepath);
			return (REPLY_ERROR);
		}

		for (i=1;i <4; i++)
			if (rx_tas_rotate(pscreenConfig->TA[0], i, &pscreenConfig->TA[i]))
				return (REPLY_ERROR);

		for (i = 0; i < 4; i++)
			if (rx_tas_shift(pscreenConfig->TA[i], &pscreenConfig->TA[i+4]))
				return (REPLY_ERROR);

		if (pscreenConfig->noise > 0)
			for (i = 0; i < 8; i++)
				if (rx_tas_addnoise(pscreenConfig->TA[i], pscreenConfig->noise)) {
					Error(ERR_CONT, 0, "Threshold arrays file <%s> are inconsistent", filepath);
					return (REPLY_ERROR);
			}
	}

	// Fill technical and penetration Fluid color type info
	for (c = 0; c < MAX_COLORS && c < pcs->count; c++) {
		pscreenConfig->inMaxInk[c] = pcs->color[c].inMaxInk;
		pscreenConfig->penetrationFluid[c] = pcs->color[c].penetrationFluid;
		if (pscreenConfig->penetrationFluid[c])
			lnbpenetrationFluid++;
	}
	if (pscreenConfig->minPenetrationFluid > (lnbpenetrationFluid * 100))
		pscreenConfig->minPenetrationFluid = lnbpenetrationFluid * 100;

	if (pscreenConfig->screenByPrinter == 1) {
		sprintf(pscreenConfig->typeName, "byPrinter");
		pscreenConfig->fctplane = &rx_screen_plane_Printer;
		pscreenConfig->fctclose = &rx_screen_close_null;
		pscreenConfig->outputbitsPerPixel = 8;

	}
	else if (pscreenConfig->method == 0) {
		if (pscreenConfig->dropsize < 12) {
			rx_screen_init_FME_1x1(pscreenConfig); // error diffusion, 1200, 1, 2 or 3
		if (pscreenConfig->inputResol.x == 600)
			pscreenConfig->fctplane = &rx_screen_plane_FME_1x1x2; // error diffusion, 600, 1, 2 or 3
		else if (pscreenConfig->inputResol.x == 300)
			pscreenConfig->fctplane = &rx_screen_plane_FME_1x1x4; // error diffusion, 300, 1, 2 or 3		{
		}
		else if (pscreenConfig->dropsize < 123)
		{
			rx_screen_init_FME_1x2g(pscreenConfig);// error diffusion, 1200, 12, fill and grow 1 and 2
			if (pscreenConfig->inputResol.x == 600)
				pscreenConfig->fctplane = &rx_screen_plane_FME_1x2gx2; // error diffusion, fill and grow 1 and 2
			else if (pscreenConfig->inputResol.x == 300)
				pscreenConfig->fctplane = &rx_screen_plane_FME_1x2gx4; // error diffusion, fill and grow 1 and 2
		}
		else {
			rx_screen_init_FME_1x3g(pscreenConfig);// error diffusion, 1200, fill and grow 1, 2 and 3
			if (pscreenConfig->inputResol.x == 600)
				pscreenConfig->fctplane = &rx_screen_plane_FME_1x3gx2; // error diffusion, 600, fill and grow 1, 2 and 3
			else if (pscreenConfig->inputResol.x == 300)
				pscreenConfig->fctplane = &rx_screen_plane_FME_1x3gx4; // error diffusion, 300, fill and grow 1, 2 and 3
		}
	}
	else if (pscreenConfig->method == 1) // Bayer 600x4  1, 2 and 3
		rx_screen_init_FMB_4x3 (pscreenConfig);
	else 
	{
		if (pscreenConfig->dropsize < 12)
		{
			if (pscreenConfig->fillmethod < 2) 
				rx_screen_init_FMS_1x1(pscreenConfig); // Threshold, 1200, 1, 2 or 3
			else
				rx_screen_init_FMS_1x1r(pscreenConfig); // Threshold, 1200, ratio mode, 1, 2 or 3

				if (pscreenConfig->inputResol.x == 600)
					pscreenConfig->fctplane = &rx_screen_plane_FMS_1x1x2; // Threshold, 600,  1, 2 or 3
				else if (pscreenConfig->inputResol.x == 300)
					pscreenConfig->fctplane = &rx_screen_plane_FMS_1x1x4; // Threshold, 300,  1, 2 or 3
		}
		else if (pscreenConfig->dropsize < 123)
		{
			if (pscreenConfig->fillmethod < 1) {
				rx_screen_init_FMS_1x2s(pscreenConfig);// Threshold, 1200, fill then grow 1 and 2
				if (pscreenConfig->inputResol.x == 600)
					pscreenConfig->fctplane = &rx_screen_plane_FMS_1x2sx2; // Threshold, 600, fill then grow 1 and 2
				else if	(pscreenConfig->inputResol.x == 300)
					pscreenConfig->fctplane = &rx_screen_plane_FMS_1x2sx4; // Threshold, 300, fill then grow 1 and 2
			}
			else 	if (pscreenConfig->fillmethod < 2) {
				rx_screen_init_FMS_1x2g(pscreenConfig); // Threshold, 1200, fill and grow 1 and 2
				if (pscreenConfig->inputResol.x == 600)
					pscreenConfig->fctplane = &rx_screen_plane_FMS_1x2gx2; // Threshold, 600, fill and grow 1 and 2
				else if (pscreenConfig->inputResol.x == 300)
					pscreenConfig->fctplane = &rx_screen_plane_FMS_1x2gx4; // Threshold, 300, fill and grow 1 and 2
			}
			else {
				rx_screen_init_FMS_1x2r(pscreenConfig); // Threshold, 1200, ratio mode 1 and 2
				if (pscreenConfig->inputResol.x == 600)
					pscreenConfig->fctplane = &rx_screen_plane_FMS_1x2rx2; // Threshold, 600, ratio mode 1 and 2
				else if	(pscreenConfig->inputResol.x == 300)
					pscreenConfig->fctplane = &rx_screen_plane_FMS_1x2rx4; // Threshold, 300, ratio mode 1 and 2
			}
		}
		else
		{
			if (pscreenConfig->fillmethod < 1) {
				rx_screen_init_FMS_1x3s(pscreenConfig);// Threshold, 1200, fill then grow 1, 2 and 3
				if (pscreenConfig->inputResol.x == 600)
					pscreenConfig->fctplane = &rx_screen_plane_FMS_1x3sx2; // Threshold, 600, fill then grow 1, 2 and 3
				else if	(pscreenConfig->inputResol.x == 300)
					pscreenConfig->fctplane = &rx_screen_plane_FMS_1x3sx4; // Threshold, 300, fill then grow 1, 2 and 3
			}
			else 	if (pscreenConfig->fillmethod < 2) {
				rx_screen_init_FMS_1x3g(pscreenConfig);// Threshold, 1200, fill and grow 1, 2 and 3
				if (pscreenConfig->inputResol.x == 600)
					pscreenConfig->fctplane = &rx_screen_plane_FMS_1x3gx2; // Threshold, 600, fill and grow 1, 2 and 3
				else if (pscreenConfig->inputResol.x == 300)
					pscreenConfig->fctplane = &rx_screen_plane_FMS_1x3gx4; // Threshold, 300, fill and grow 1, 2 and 3
			}
			else {
				rx_screen_init_FMS_1x3r(pscreenConfig);// Threshold, 1200, ratio mode 1, 2 and 3
				if (pscreenConfig->inputResol.x == 600)
					pscreenConfig->fctplane = &rx_screen_plane_FMS_1x3rx2; // Threshold, 600, ratio mode 1, 2 and 3
				else if (pscreenConfig->inputResol.x == 300)
					pscreenConfig->fctplane = &rx_screen_plane_FMS_1x3rx4; // Threshold, 300, ratio mode 1, 2 and 3
			}
		}
	}
	return (REPLY_OK);
}


//--- rx_screen_close ----------------------------------------------------------------------
int rx_screen_close(SScreenConfig* pscreenConfig)
{
	int i;
	for (i = 0; i < 8; i++) {
		if (pscreenConfig->TA[i]) // need ta ? 
		{
			rx_tas_free((STaConfig *)pscreenConfig->TA[i]);
		}
	}

	if (pscreenConfig->fctclose)
		return(pscreenConfig->fctclose(pscreenConfig));
	else 
	{
		Error(ERR_CONT, 0, "Screen type %d: <%s> not supported", pscreenConfig->type, pscreenConfig->typeName);
		return (REPLY_ERROR);
	}

}


//--- rx_screen_outplane_init ----------------------------------------------------------------------
int rx_screen_outplane_init(SPlaneInfo *inplane, SPlaneInfo *outplane, SScreenConfig* pscreenConfig)
{
	outplane->WidthPx = inplane->WidthPx*pscreenConfig->outputResol.x / pscreenConfig->inputResol.x;
	outplane->lengthPx = inplane->lengthPx*pscreenConfig->outputResol.y / pscreenConfig->inputResol.y;
	outplane->bitsPerPixel = pscreenConfig->outputbitsPerPixel;
	outplane->aligment = 8;
	outplane->lineLen = ((outplane->WidthPx * outplane->bitsPerPixel) + outplane->aligment - 1) / outplane->aligment;
	outplane->dataSize = (UINT64)outplane->lineLen*outplane->lengthPx;
	outplane->resol.x = inplane->resol.x * pscreenConfig->outputResol.x / pscreenConfig->inputResol.x;
	outplane->resol.y = inplane->resol.y * pscreenConfig->outputResol.y / pscreenConfig->inputResol.y;
	outplane->planenumber = inplane->planenumber;
	outplane->DropCount[0] = outplane->DropCount[1] = outplane->DropCount[2] = 0;

	outplane->buffer = rx_mem_alloc(outplane->dataSize);
	TrPrintfL(60, "data_malloc buffer: %p, size=%u MB, free=%u MB", outplane->buffer, outplane->dataSize / 1024 / 1024, rx_mem_get_freeMB());
	if (outplane->buffer == NULL) return Error(ERR_CONT, 0, "No buffer for screening: %d");
	memset(outplane->buffer, 0, outplane->dataSize);
	return REPLY_OK;
}




//--- rx_screen_close_null ----------------------------------------------------------------------
int rx_screen_close_null(void* pscreenConfig)
{

	return (REPLY_OK);
}



//--- rx_screen_plane_Printer ----------------------------------------------------------------------
int rx_screen_plane_Printer(SPlaneInfo *inplane, SPlaneInfo *outplane, void* epscreenConfig)
{
	SScreenConfig* pscreenConfig = epscreenConfig;
	UINT32 l;
	int i;
	BYTE *pSrc, *pDst;
	UINT16 *pWSrc;

	if ((pscreenConfig->inputResol.x * 2 == pscreenConfig->outputResol.x) && (pscreenConfig->inputResol.y * 2 == pscreenConfig->outputResol.y)) {
		return (rx_screen_plane_Printerx2(inplane, outplane, pscreenConfig));
	}
	else if ((pscreenConfig->inputResol.x * 4 == pscreenConfig->outputResol.x) && (pscreenConfig->inputResol.y * 4 == pscreenConfig->outputResol.y)) {
		return (rx_screen_plane_Printerx4(inplane, outplane, pscreenConfig));
	}

	if (rx_screen_outplane_init(inplane, outplane, pscreenConfig))
		return REPLY_ERROR;



	if (inplane->bitsPerPixel == 16) {
		for (l = 0; l < inplane->lengthPx; l++)
		{
			pWSrc = (UINT16 *)(inplane->buffer + (l * inplane->lineLen));
			pDst = outplane->buffer + (l * outplane->lineLen);
			for (i = 0; i < (int)inplane->WidthPx; i++)
			{
				if (pWSrc[i])
				{
					*pDst = (BYTE)(pWSrc[i] / 257);
				}
				pDst++;
			}
		}
	}

	else { // bitsPerPixel == 8
		for (l = 0; l < inplane->lengthPx; l++)
		{
			pSrc = inplane->buffer + (l * inplane->lineLen);
			pDst = outplane->buffer + (l * outplane->lineLen);
			for (i = 0; i < (int)inplane->WidthPx; i++)
			{
				if (pSrc[i])
				{
					*pDst = pSrc[i];
				}
				pDst++;
			}
		}
	}

	return (REPLY_OK);
}


//--- rx_screen_plane_Printerx2 ----------------------------------------------------------------------
int rx_screen_plane_Printerx2(SPlaneInfo *inplane, SPlaneInfo *outplane, void* epscreenConfig)
{
	SScreenConfig* pscreenConfig = epscreenConfig;
	UINT32 li, lo;
	int pi, po;
	BYTE *pSrc, *pDst;
	UINT16 *pWSrc;

	if ((pscreenConfig->inputResol.x * 4 == pscreenConfig->outputResol.x) && (pscreenConfig->inputResol.y * 4 == pscreenConfig->outputResol.y)) {
		return (rx_screen_plane_FMS_1x2gx4(inplane, outplane, pscreenConfig));
	}

	if (rx_screen_outplane_init(inplane, outplane, pscreenConfig))
		return REPLY_ERROR;


	if (inplane->bitsPerPixel == 16) {
		for (li = 0; li < inplane->lengthPx; li++)
		{
			pWSrc = (UINT16 *)(inplane->buffer + (li * inplane->lineLen));
			for (lo = (2 * li); lo < (2 * li) + 2; lo++)
			{
				pDst = outplane->buffer + (lo * outplane->lineLen);
				for (pi = 0; pi < (int)inplane->WidthPx; pi++)
				{
					for (po = (2 * pi); po < (2 * pi) + 2; po++)
					{
						if (pWSrc[pi])
						{
							*pDst = (BYTE)(pWSrc[pi] / 257);
						}
						pDst++;
					}
				}
			}
		}
	}

	else { // bitsPerPixel == 8
		for (li = 0; li < inplane->lengthPx; li++)
		{
			pSrc = inplane->buffer + (li * inplane->lineLen);
			for (lo = (2 * li); lo < (2 * li) + 2; lo++)
			{
				pDst = outplane->buffer + (lo * outplane->lineLen);
				for (pi = 0; pi < (int)inplane->WidthPx; pi++)
				{
					for (po = (2 * pi); po < (2 * pi) + 2; po++)
					{
						if (pSrc[pi])
						{
							*pDst = pSrc[pi];
						}
						pDst++;
					}
				}
			}
		}
	}
	return (REPLY_OK);
}


//--- rx_screen_plane_Printerx4 ----------------------------------------------------------------------
int rx_screen_plane_Printerx4(SPlaneInfo *inplane, SPlaneInfo *outplane, void* epscreenConfig)
{
	SScreenConfig* pscreenConfig = epscreenConfig;
	UINT32 li, lo;
	int pi, po;
	BYTE *pSrc, *pDst;
	UINT16 *pWSrc;

	if (rx_screen_outplane_init(inplane, outplane, pscreenConfig))
		return REPLY_ERROR;

	if (inplane->bitsPerPixel == 16) {
		for (li = 0; li < inplane->lengthPx; li++)
		{
			pWSrc = (UINT16 *)(inplane->buffer + (li * inplane->lineLen));
			for (lo = (4 * li); lo < (4 * li) + 4; lo++)
			{
				pDst = outplane->buffer + (lo * outplane->lineLen);
				for (pi = 0; pi < (int)inplane->WidthPx; pi++)
				{
					for (po = (4 * pi); po < (4 * pi) + 4; po++)
					{
						if (pWSrc[pi])
						{
							*pDst = (BYTE)(pWSrc[pi] / 257);
						}
						pDst++;
					}
				}
			}
		}
	}

	else { // bitsPerPixel == 8
		for (li = 0; li < inplane->lengthPx; li++)
		{
			pSrc = inplane->buffer + (li * inplane->lineLen);
			for (lo = (4 * li); lo < (4 * li) + 4; lo++)
			{
				pDst = outplane->buffer + (lo * outplane->lineLen);
				for (pi = 0; pi < (int)inplane->WidthPx; pi++)
				{
					for (po = (4 * pi); po < (4 * pi) + 4; po++)
					{
						if (pSrc[pi])
						{
							*pDst = pSrc[pi];
						}
						pDst++;
					}
				}
			}
		}
	}
	return (REPLY_OK);
}


