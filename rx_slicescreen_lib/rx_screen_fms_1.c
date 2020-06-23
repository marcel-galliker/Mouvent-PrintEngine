// ****************************************************************************
//
//	rx_screen_fms_1.c
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
#include "rx_screen_fms_1.h"

#ifdef linux
	#include "errno.h"
#endif

static BYTE			Val4[4];


//--- rx_screen_plane_FMS_1x1 ----------------------------------------------------------------------
int rx_screen_plane_FMS_1x1(SPlaneInfo *inplane, SPlaneInfo *outplane, void* epscreenConfig)
{
	SScreenConfig* pscreenConfig = epscreenConfig;
	UINT32 l;
	int i, modl, taindex;
	BYTE *pSrc, *pDst;
	UINT16 *pWSrc, WSrc;
	UINT64 *pDropCount = &outplane->DropCount[0];
	STaConfig *pta = pscreenConfig->TA[inplane->planenumber % 8];
	UINT16 *lut16 = pscreenConfig->lut16[inplane->planenumber];
	if ((pscreenConfig->inputResol.x * 2== pscreenConfig->outputResol.x) && (pscreenConfig->inputResol.y * 2 == pscreenConfig->outputResol.y)) {
		return (rx_screen_plane_FMS_1x1x2(inplane, outplane, pscreenConfig));
	}
	else if ((pscreenConfig->inputResol.x * 4 == pscreenConfig->outputResol.x) && (pscreenConfig->inputResol.y * 4 == pscreenConfig->outputResol.y)) {
		return (rx_screen_plane_FMS_1x1x4(inplane, outplane, pscreenConfig));
	}

	if (rx_screen_outplane_init(inplane, outplane, pscreenConfig))
		return REPLY_ERROR;
	if ((pscreenConfig->dropsize>1) && (pscreenConfig->dropsize<4))
		pDropCount = &outplane->DropCount[pscreenConfig->dropsize - 1];

	if (inplane->bitsPerPixel == 16) {
		for (l = 0; l < inplane->lengthPx; l++)
		{
			pWSrc = (UINT16 *) (inplane->buffer + (l * inplane->lineLen));
			modl = l % pta->heigth;
			pDst = outplane->buffer + (l * outplane->lineLen);
			for (i = 0; i < (int)inplane->WidthPx; i++)
			{
				if (pWSrc[i])
				{
					WSrc = lut16[pWSrc[i]];
					if (WSrc) {

						taindex = (modl * pta->width) + (i % pta->width);
						if (WSrc >= pta->ta16[taindex]) {
							(*pDropCount)++;
							switch (i % 4) {
							case 0: *pDst = Val4[0]; break;
							case 1: *pDst |= Val4[1]; break;
							case 2: *pDst |= Val4[2]; break;
							case 3: *pDst |= Val4[3]; break;
							}
						}
					}
				}
				if ((i % 4) == 3)
					pDst++;
			}
		}
	}

	else { // bitsPerPixel == 8
		for (l = 0; l < inplane->lengthPx; l++)
		{
			pSrc = inplane->buffer + (l * inplane->lineLen);
			modl = l % pta->heigth;
			pDst = outplane->buffer + (l * outplane->lineLen);
			for (i = 0; i < (int)inplane->WidthPx; i++)
			{
				if (pSrc[i])
				{
					WSrc = lut16[pSrc[i] * 257];
					if (WSrc) {
						taindex = (modl * pta->width) + (i % pta->width);
						if (WSrc >= pta->ta16[taindex]) {
							(*pDropCount)++;
							switch (i % 4) {
							case 0: *pDst = Val4[0]; break;
							case 1: *pDst |= Val4[1]; break;
							case 2: *pDst |= Val4[2]; break;
							case 3: *pDst |= Val4[3]; break;
							}
						}
					}
				}
				if ((i % 4) == 3)
					pDst++;
			}
		}
	}
	return (REPLY_OK);
}


// 600x600 bytes or word to 1200x1200 2 bits
//--- rx_screen_plane_FMS_1x1x2 ----------------------------------------------------------------------
int rx_screen_plane_FMS_1x1x2(SPlaneInfo *inplane, SPlaneInfo *outplane, void* epscreenConfig)
{
	SScreenConfig* pscreenConfig = epscreenConfig;
	UINT32 li, lo;
	int pi, po , modl, taindex;
	BYTE *pSrc, *pDst;
	UINT16 *pWSrc, WSrc;
	UINT64 *pDropCount = &outplane->DropCount[0];
	STaConfig *pta = pscreenConfig->TA[inplane->planenumber % 8];
	UINT16 *lut16 = pscreenConfig->lut16[inplane->planenumber];

	if ((pscreenConfig->inputResol.x * 4 == pscreenConfig->outputResol.x) && (pscreenConfig->inputResol.y * 4 == pscreenConfig->outputResol.y)) {
		return (rx_screen_plane_FMS_1x1x4(inplane, outplane, pscreenConfig));
	}

	if (rx_screen_outplane_init(inplane, outplane, pscreenConfig))
		return REPLY_ERROR;
	if ((pscreenConfig->dropsize > 1) && (pscreenConfig->dropsize < 4))
		pDropCount = &outplane->DropCount[pscreenConfig->dropsize - 1];

	if (inplane->bitsPerPixel == 16) {
		for (li = 0; li < inplane->lengthPx; li++)
		{
			pWSrc = (UINT16 *)(inplane->buffer + (li * inplane->lineLen));
			for (lo = (2 * li); lo < (2 * li) + 2; lo++)
			{
				modl = lo % pta->heigth;
				pDst = outplane->buffer + (lo * outplane->lineLen);
				for (pi = 0; pi < (int)inplane->WidthPx; pi++)
				{
					for (po = (2* pi); po < (2 * pi) + 2; po++)
					{
						if (pWSrc[pi])
						{
							WSrc = lut16[pWSrc[pi]];
							if (WSrc) {

								taindex = (modl * pta->width) + (po % pta->width);
								if (WSrc >= pta->ta16[taindex]) {
									(*pDropCount)++;
									switch (po % 4) {
									case 0: *pDst = Val4[0]; break;
									case 1: *pDst |= Val4[1]; break;
									case 2: *pDst |= Val4[2]; break;
									case 3: *pDst |= Val4[3]; break;
									}
								}
							}
						}
						if ((po % 4) == 3)
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
				modl = lo % pta->heigth;
				pDst = outplane->buffer + (lo * outplane->lineLen);
				for (pi = 0; pi < (int)inplane->WidthPx; pi++)
				{
					for (po = (2 * pi); po < (2 * pi) + 2; po++)
					{
						if (pSrc[pi])
						{
							WSrc = lut16[pSrc[pi] * 257];
							if (WSrc) {
								taindex = (modl * pta->width) + (po % pta->width);
								if (WSrc >= pta->ta16[taindex]) {
									(*pDropCount)++;
									switch (po % 4) {
									case 0: *pDst = Val4[0]; break;
									case 1: *pDst |= Val4[1]; break;
									case 2: *pDst |= Val4[2]; break;
									case 3: *pDst |= Val4[3]; break;
									}
								}
							}
						}
						if ((po % 4) == 3)
							pDst++;
					}
				}
			}
		}
	}
	return (REPLY_OK);
}


// 300x300 bytes or word to 1200x1200 2 bits
//--- rx_screen_plane_FMS_1x1x4 ----------------------------------------------------------------------
int rx_screen_plane_FMS_1x1x4(SPlaneInfo *inplane, SPlaneInfo *outplane, void* epscreenConfig)
{
	SScreenConfig* pscreenConfig = epscreenConfig;
	UINT32 li, lo;
	int pi, po, modl, taindex;
	BYTE *pSrc, *pDst;
	UINT16 *pWSrc, WSrc;
	UINT64 *pDropCount = &outplane->DropCount[0];
	STaConfig *pta = pscreenConfig->TA[inplane->planenumber % 8];
	UINT16 *lut16 = pscreenConfig->lut16[inplane->planenumber];

	if (rx_screen_outplane_init(inplane, outplane, pscreenConfig))
		return REPLY_ERROR;
	if ((pscreenConfig->dropsize > 1) && (pscreenConfig->dropsize < 4))
		pDropCount = &outplane->DropCount[pscreenConfig->dropsize - 1];

	if (inplane->bitsPerPixel == 16) {
		for (li = 0; li < inplane->lengthPx; li++)
		{
			pWSrc = (UINT16 *)(inplane->buffer + (li * inplane->lineLen));
			for (lo = (4 * li); lo < (4 * li) + 4; lo++)
			{
				modl = lo % pta->heigth;
				pDst = outplane->buffer + (lo * outplane->lineLen);
				for (pi = 0; pi < (int)inplane->WidthPx; pi++)
				{
					for (po = (4 * pi); po < (4 * pi) + 4; po++)
					{
						if (pWSrc[pi])
						{
							WSrc = lut16[pWSrc[pi]];
							if (WSrc) {

								taindex = (modl * pta->width) + (po % pta->width);
								if (WSrc >= pta->ta16[taindex]) {
									(*pDropCount)++;
									switch (po % 4) {
									case 0: *pDst = Val4[0]; break;
									case 1: *pDst |= Val4[1]; break;
									case 2: *pDst |= Val4[2]; break;
									case 3: *pDst |= Val4[3]; break;
									}
								}
							}
						}
						if ((po % 4) == 3)
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
				modl = lo % pta->heigth;
				pDst = outplane->buffer + (lo * outplane->lineLen);
				for (pi = 0; pi < (int)inplane->WidthPx; pi++)
				{
					for (po = (4 * pi); po < (4 * pi) + 4; po++)
					{
						if (pSrc[pi])
						{
							WSrc = lut16[pSrc[pi] * 257];
							if (WSrc) {
								taindex = (modl * pta->width) + (po % pta->width);
								if (WSrc >= pta->ta16[taindex]) {
									(*pDropCount)++;
									switch (po % 4) {
									case 0: *pDst = Val4[0]; break;
									case 1: *pDst |= Val4[1]; break;
									case 2: *pDst |= Val4[2]; break;
									case 3: *pDst |= Val4[3]; break;
									}
								}
							}
						}
						if ((po % 4) == 3)
							pDst++;
					}
				}
			}
		}
	}
	return (REPLY_OK);
}


//--- rx_screen_init_FMS_1x1 ----------------------------------------------------------------------
int rx_screen_init_FMS_1x1(SScreenConfig* pscreenConfig)
{
	sprintf(pscreenConfig->typeName, "FMS_1x1");
	pscreenConfig->fctplane = &rx_screen_plane_FMS_1x1;
	pscreenConfig->fctclose = &rx_screen_close_null;
	if (pscreenConfig->dropsize == 1) {
		Val4[0] = 0x40;
		Val4[1] = 0x10;
		Val4[2] = 0x04;
		Val4[3] = 0x01;
	}
	else if	(pscreenConfig->dropsize == 2) {
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
	return (REPLY_OK);
}


//--- rx_screen_init_FMS_1x1r ----------------------------------------------------------------------
int rx_screen_init_FMS_1x1r(SScreenConfig* pscreenConfig)
{
	float totR;
	int  i, j;

	rx_screen_init_FMS_1x1(pscreenConfig);
	sprintf(pscreenConfig->typeName, "FMS_1x1r");


	for (i = 0; i < MAX_COLORS; i++) {
		totR = (float) (pscreenConfig->ratio[i].r100.smallD + pscreenConfig->ratio[i].r100.mediumD + pscreenConfig->ratio[i].r100.largeD) /100; // max 100
		if (totR > 1.0) totR = 1.0;

		for (j = 0; j < 65536; j++) {
			pscreenConfig->lut16[i][j] = (UINT16) (totR * pscreenConfig->lut16[i][j]); // used smallD ratio for any drop size
		}
	}

	return (REPLY_OK);
}

