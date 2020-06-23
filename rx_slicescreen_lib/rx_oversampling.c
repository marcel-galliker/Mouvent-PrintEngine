// ****************************************************************************
//
//	rx_oversampling.c
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
#include "rx_screen.h"

#ifdef linux
	#include "errno.h"
#endif


//--- SOverSamplingThread-------------------------------------------------------------------------
typedef struct
{
	int				running;			// running yes/no
	HANDLE			hWaitEvent;			// handle on wait event
	int				ret;				// ret code
	SPlaneInfo		inplane;			// inputplane info ptr
	SPlaneInfo		outplane;			// inputplane info ptr
	SScreenConfig*	pscreenConfig;		// screen config ptr
} SOverSamplingThread;

SOverSamplingThread oversamplingthread[MAX_COLORS];


//--- _rx_oversampling_outplane_init ----------------------------------------------------------------------
int _rx_oversampling_outplane_init(SPlaneInfo *inplane, SPlaneInfo *outplane, SScreenConfig* pscreenConfig)
{
	outplane->WidthPx = inplane->WidthPx / 2;
	outplane->lengthPx = inplane->lengthPx / 2;
	outplane->bitsPerPixel = inplane->bitsPerPixel;
	outplane->aligment = 8;
	outplane->lineLen = ((outplane->WidthPx * outplane->bitsPerPixel) + outplane->aligment - 1) / outplane->aligment;
	outplane->dataSize = (UINT64)outplane->lineLen*outplane->lengthPx;
	outplane->resol.x = inplane->resol.x / 2;
	outplane->resol.y = inplane->resol.y / 2;
	outplane->planenumber = inplane->planenumber;
	outplane->DropCount[0] = outplane->DropCount[1] = outplane->DropCount[2] = 0;

	outplane->buffer = rx_mem_alloc(outplane->dataSize);
	TrPrintfL(60, "data_malloc buffer: %p, size=%u MB, free=%u MB", outplane->buffer, outplane->dataSize / 1024 / 1024, rx_mem_get_freeMB());
	if (outplane->buffer == NULL) return Error(ERR_CONT, 0, "No buffer for screening: %d", inplane->planenumber);
	memset(outplane->buffer, 0, outplane->dataSize);
	return REPLY_OK;
}


//--- _rx_oversampling_normal ----------------------------------------------------------------------
int _rx_oversampling_normal(SPlaneInfo *inplane, SPlaneInfo *outplane, SScreenConfig* pscreenConfig)
{
	UINT32 l;
	int i;
	BYTE *pSrc, *pSrcBis, *pDst;
	UINT16 *pWSrc, *pWSrcBis, *pWDst;

	if (_rx_oversampling_outplane_init(inplane, outplane, pscreenConfig))
		return REPLY_ERROR;
	if (inplane->bitsPerPixel == 16) {
		for (l = 0; l < outplane->lengthPx; l++)
		{
			pWSrc = (UINT16 *)(inplane->buffer + ((UINT64)l * 2 * inplane->lineLen));
			pWSrcBis = (UINT16 *)(inplane->buffer + ((((UINT64)l * 2 )+1)* inplane->lineLen));
			pWDst = (UINT16 *)(outplane->buffer + ((UINT64)l * outplane->lineLen));
			for (i = 0; i < (int)outplane->WidthPx; i++)
			{
				(*pWDst) = ((*pWSrc) / 4) + ((*(pWSrc + 1)) / 4) + ((*pWSrcBis) / 4) + ((*(pWSrcBis + 1)) / 4);
				pWSrc += 2;
				pWSrcBis += 2;
				pWDst++;
			}
		}
	}
	else { // bitsPerPixel == 8
		for (l = 0; l < outplane->lengthPx; l++)
		{
			pSrc = (inplane->buffer + ((UINT64)l * 2 * inplane->lineLen));
			pSrcBis = pSrc + (UINT64)(inplane->lineLen);
			pDst = (outplane->buffer + ((UINT64)l * outplane->lineLen));
			for (i = 0; i < (int)outplane->WidthPx; i++)
			{
				(*pDst) = ((*pSrc) / 4) + ((*(pSrc + 1)) / 4) + ((*pSrcBis) / 4) + ((*(pSrcBis + 1)) / 4);
				pSrc += 2;
				pSrcBis += 2;
				pDst++;
			}
		}
	}
	return (REPLY_OK);
}


//--- _rx_oversampling_light ----------------------------------------------------------------------
int _rx_oversampling_light(SPlaneInfo *inplane, SPlaneInfo *outplane, SScreenConfig* pscreenConfig)
{
	UINT32 l;
	int i;
	BYTE *pSrc, *pSrcBis, *pDst;
	UINT16 *pWSrc, *pWSrcBis, *pWDst;

	if (_rx_oversampling_outplane_init(inplane, outplane, pscreenConfig))
		return REPLY_ERROR;
	if (inplane->bitsPerPixel == 16) {
		for (l = 0; l < outplane->lengthPx; l++)
		{
			pWSrc = (UINT16 *)(inplane->buffer + ((UINT64)l * 2 * inplane->lineLen));
			pWSrcBis = (UINT16 *)(inplane->buffer + ((((UINT64)l * 2) + 1)* inplane->lineLen));
			pWDst = (UINT16 *)(outplane->buffer + ((UINT64)l * outplane->lineLen));
			for (i = 0; i < (int)outplane->WidthPx; i++)
			{
				if (*pWSrc <= *(pWSrc + 1)) {
					if (*pWSrc <= *pWSrcBis) {
						if (*pWSrc <= *(pWSrcBis + 1)) {
							*pWDst = *pWSrc;
						}
						else {
							*pWDst = *(pWSrcBis + 1);
						}
					}
					else {
						if (*pWSrcBis <= *(pWSrcBis + 1)) {
							*pWDst = *pWSrcBis;
						}
						else {
							*pWDst = *(pWSrcBis + 1);
						}
					}
				}
				else {
					if (*(pWSrc + 1) <= *pWSrcBis) {
						if (*(pWSrc + 1) <= *(pWSrcBis + 1)) {
							*pWDst = *(pWSrc + 1);
						}
						else {
							*pWDst = *(pWSrcBis + 1);
						}
					}
					else {
						if (*pWSrcBis <= *(pWSrcBis + 1)) {
							*pWDst = *pWSrcBis;
						}
						else {
							*pWDst = *(pWSrcBis + 1);
						}
					}
				}
				pWSrc += 2;
				pWSrcBis += 2;
				pWDst++;
			}
		}
	}
	else { // bitsPerPixel == 8
		for (l = 0; l < outplane->lengthPx; l++)
		{
			pSrc = (inplane->buffer + ((UINT64)l * 2 * inplane->lineLen));
			pSrcBis = pSrc + ((UINT64)inplane->lineLen);
			pDst = (outplane->buffer + ((UINT64)l * outplane->lineLen));
			for (i = 0; i < (int)outplane->WidthPx; i++)
			{
				if (*pSrc <= *(pSrc + 1)) {
					if (*pSrc <= *pSrcBis) {
						if (*pSrc <= *(pSrcBis + 1)) {
							*pDst = *pSrc;
						}
						else {
							*pDst = *(pSrcBis + 1);
						}
					}
					else {
						if (*pSrcBis <= *(pSrcBis + 1)) {
							*pDst = *pSrcBis;
						}
						else {
							*pDst = *(pSrcBis + 1);
						}
					}
				}
				else {
					if (*(pSrc + 1) <= *pSrcBis) {
						if (*(pSrc + 1) <= *(pSrcBis + 1)) {
							*pDst = *(pSrc + 1);
						}
						else {
							*pDst = *(pSrcBis + 1);
						}
					}
					else {
						if (*pSrcBis <= *(pSrcBis + 1)) {
							*pDst = *pSrcBis;
						}
						else {
							*pDst = *(pSrcBis + 1);
						}
					}
				}				pSrc += 2;
				pSrcBis += 2;
				pDst++;
			}
		}
	}
	return (REPLY_OK);
}


//--- _rx_oversampling_bold ----------------------------------------------------------------------
int _rx_oversampling_bold(SPlaneInfo *inplane, SPlaneInfo *outplane, SScreenConfig* pscreenConfig)
{
	UINT32 l;
	int i;
	BYTE *pSrc, *pSrcBis, *pDst;
	UINT16 *pWSrc, *pWSrcBis, *pWDst;

	if (_rx_oversampling_outplane_init(inplane, outplane, pscreenConfig))
		return REPLY_ERROR;
	if (inplane->bitsPerPixel == 16) {
		for (l = 0; l < outplane->lengthPx; l++)
		{
			pWSrc = (UINT16 *)(inplane->buffer + ((UINT64)l * 2 * inplane->lineLen));
			pWSrcBis = (UINT16 *)(inplane->buffer + ((((UINT64)l * 2) + 1)* inplane->lineLen));
			pWDst = (UINT16 *)(outplane->buffer + ((UINT64)l * outplane->lineLen));
			for (i = 0; i < (int)outplane->WidthPx; i++)
			{
				if (*pWSrc > *(pWSrc + 1)) {
					if (*pWSrc > *pWSrcBis) {
						if (*pWSrc > *(pWSrcBis + 1)) {
							*pWDst = *pWSrc;
						}
						else {
							*pWDst = *(pWSrcBis + 1);
						}
					}
					else {
						if (*pWSrcBis > *(pWSrcBis + 1)) {
							*pWDst = *pWSrcBis;
						}
						else {
							*pWDst = *(pWSrcBis + 1);
						}
					}
				}
				else {
					if (*(pWSrc + 1) > *pWSrcBis) {
						if (*(pWSrc + 1) > *(pWSrcBis + 1)) {
							*pWDst = *(pWSrc + 1);
						}
						else {
							*pWDst = *(pWSrcBis + 1);
						}
					}
					else {
						if (*pWSrcBis > *(pWSrcBis + 1)) {
							*pWDst = *pWSrcBis;
						}
						else {
							*pWDst = *(pWSrcBis + 1);
						}
					}
				}
				pWSrc += 2;
				pWSrcBis += 2;
				pWDst++;
			}
		}
	}
	else { // bitsPerPixel == 8
		for (l = 0; l < outplane->lengthPx; l++)
		{
			pSrc = (inplane->buffer + ((UINT64)l * 2 * inplane->lineLen));
			pSrcBis = pSrc + ((UINT64)inplane->lineLen);
			pDst = (outplane->buffer + ((UINT64)l * outplane->lineLen));
			for (i = 0; i < (int)outplane->WidthPx; i++)
			{
				if (*pSrc > *(pSrc + 1)) {
					if (*pSrc > *pSrcBis) {
						if (*pSrc > *(pSrcBis + 1)) {
							*pDst = *pSrc;
						}
						else {
							*pDst = *(pSrcBis + 1);
						}
					}
					else {
						if (*pSrcBis > *(pSrcBis + 1)) {
							*pDst = *pSrcBis;
						}
						else {
							*pDst = *(pSrcBis + 1);
						}
					}
				}
				else {
					if (*(pSrc + 1) > *pSrcBis) {
						if (*(pSrc + 1) > *(pSrcBis + 1)) {
							*pDst = *(pSrc + 1);
						}
						else {
							*pDst = *(pSrcBis + 1);
						}
					}
					else {
						if (*pSrcBis > *(pSrcBis + 1)) {
							*pDst = *pSrcBis;
						}
						else {
							*pDst = *(pSrcBis + 1);
						}
					}
				}				pSrc += 2;
				pSrcBis += 2;
				pDst++;
			}
		}
	}
	return (REPLY_OK);
}


//--- _rx_oversampling_plane ----------------------------------------------------------------------
int _rx_oversampling_plane(SPlaneInfo *inplane, SPlaneInfo *outplane, SScreenConfig* pscreenConfig)
{

	if (inplane->buffer == NULL) return Error(ERR_CONT, 0, "No output buffer for rx_oversampling_plane: %d", inplane->planenumber);

	switch (pscreenConfig->overSampling) {
	case 1:
		return (_rx_oversampling_normal(inplane, outplane, pscreenConfig));
		break;
	case 2:
		return (_rx_oversampling_light(inplane, outplane, pscreenConfig));
		break;
	case 3:
		return (_rx_oversampling_bold(inplane, outplane, pscreenConfig));
		break;
	default:
		return Error(ERR_CONT, 0, "bad oversampling value <%d> in rx_oversampling_plane: %d", pscreenConfig->overSampling);
	}

}


//---  _rx_thread_oversampling_plane ------------------------------------------------------------
static void *_rx_thread_oversampling_plane(void *par)
{
	SOverSamplingThread *poversamplingthread = (void *)par;
	poversamplingthread->running = 1;
	poversamplingthread->ret = _rx_oversampling_plane(&poversamplingthread->inplane, &poversamplingthread->outplane, poversamplingthread->pscreenConfig);
	if (poversamplingthread->inplane.buffer) {
		rx_mem_free(&poversamplingthread->inplane.buffer);
		poversamplingthread->inplane.buffer = NULL;
	}
	poversamplingthread->running = 0;
	rx_sem_post(poversamplingthread->hWaitEvent);

	return NULL;
}


//--- rx_oversampling_bmp ----------------------------------------------------------------------
int rx_oversampling_bmp(SBmpInfo *pinfo, SScreenConfig* pscreenConfig)
{
	int iCntPlane, l_threadMax, j, i;
	int ret = REPLY_OK;
	HANDLE hWaitEvent;

	if (!pinfo->planes) return Error(ERR_CONT, 0, "No planes in bitmap");
	for (j = 0; j < pinfo->planes; j++)
	{
		oversamplingthread[j].ret = REPLY_OK;
		oversamplingthread[j].running = 0;
	}
	iCntPlane = 0;
	l_threadMax = rx_get_maxnumthreads(NULL);
	for (i=0; i < pinfo->planes; i+= l_threadMax)
	{
		hWaitEvent = rx_sem_create();
		if (!hWaitEvent)
			return Error(ERR_CONT, 0, "Error rx_sem_create in rx_oversampling_bmp");
		for (j = i; j < i + l_threadMax; j++)
		{
			if (iCntPlane >= pinfo->planes)
				break;
			oversamplingthread[j].inplane.aligment = pinfo->aligment;
			oversamplingthread[j].inplane.WidthPx = pinfo->srcWidthPx;
			oversamplingthread[j].inplane.lengthPx = pinfo->lengthPx;
			oversamplingthread[j].inplane.bitsPerPixel = pinfo->bitsPerPixel;
			oversamplingthread[j].inplane.lineLen = pinfo->lineLen;
			oversamplingthread[j].inplane.dataSize = pinfo->dataSize;
			oversamplingthread[j].inplane.resol = pinfo->resol;
			oversamplingthread[j].inplane.planenumber = iCntPlane;
			oversamplingthread[j].inplane.buffer = *pinfo->buffer[j];

			oversamplingthread[j].running = 1;
			oversamplingthread[j].ret = REPLY_OK;
			oversamplingthread[j].hWaitEvent = hWaitEvent;
			oversamplingthread[j].pscreenConfig = pscreenConfig;
			rx_thread_start(_rx_thread_oversampling_plane, (void*)&oversamplingthread[j], 0, "_rx_thread_oversampling_plane");
			iCntPlane++;
		}

		while (1) {
			rx_sem_wait(hWaitEvent, INFINITE);
			for (j = 0; j < iCntPlane; j++) {
				if (oversamplingthread[j].running)
					break;
			}
			if (j == iCntPlane)
				break;
		}
		rx_sem_destroy(&hWaitEvent);
	}

	pinfo->srcWidthPx	= oversamplingthread[0].outplane.WidthPx;
	pinfo->lengthPx		= oversamplingthread[0].outplane.lengthPx;
	pinfo->bitsPerPixel = oversamplingthread[0].outplane.bitsPerPixel;
	pinfo->lineLen		= oversamplingthread[0].outplane.lineLen;
	pinfo->dataSize		= oversamplingthread[0].outplane.dataSize;
	pinfo->resol		= oversamplingthread[0].outplane.resol;
	for (j = 0; j < iCntPlane; j++)
	{
		if (oversamplingthread[j].ret != REPLY_OK)
			ret = oversamplingthread[j].ret;
		pinfo->buffer[j]	   = &oversamplingthread[j].outplane.buffer;
		pinfo->DropCount[j][0] = oversamplingthread[j].outplane.DropCount[0];
		pinfo->DropCount[j][1] = oversamplingthread[j].outplane.DropCount[1];
		pinfo->DropCount[j][2] = oversamplingthread[j].outplane.DropCount[2];
	}

	return (ret);
}