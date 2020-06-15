// ****************************************************************************
//
//	rx_screen_fms_1x2.c
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
#include "rx_screen_fms_1x2.h"

#ifdef linux
	#include "errno.h"
#endif

static BYTE			Val4[4];


//--- rx_screen_plane_FMS_1x2s ----------------------------------------------------------------------
int rx_screen_plane_FMS_1x2s(SPlaneInfo *inplane, SPlaneInfo *outplane, void* epscreenConfig)
{
	SScreenConfig* pscreenConfig = epscreenConfig;
	UINT32 l;
	int i, modl, taindex;
	BYTE *pSrc, *pDst;
	UINT16 *pWSrc, WSrc, WLimit = 32768, WT1 = 32768, WT2 = 32768;
	STaConfig *pta = pscreenConfig->TA[inplane->planenumber % 8];
	UINT16 *lut16 = pscreenConfig->lut16[inplane->planenumber];

	if ((pscreenConfig->inputResol.x * 2 == pscreenConfig->outputResol.x) && (pscreenConfig->inputResol.y * 2 == pscreenConfig->outputResol.y)) {
		return (rx_screen_plane_FMS_1x2sx2(inplane, outplane, pscreenConfig));
	}
	else if ((pscreenConfig->inputResol.x * 4 == pscreenConfig->outputResol.x) && (pscreenConfig->inputResol.y * 4 == pscreenConfig->outputResol.y)) {
		return (rx_screen_plane_FMS_1x2sx4(inplane, outplane, pscreenConfig));
	}

	if (rx_screen_outplane_init(inplane, outplane, pscreenConfig))
		return REPLY_ERROR;

	if (pscreenConfig->limit[outplane->planenumber][0]) {
		WT1 = WLimit = 65536 * pscreenConfig->limit[outplane->planenumber][0] / 100;
		if (!WT1) WT1 = 1;
		WT2 = 65536 - WT1;
		if (!WT2) WT2 = 1;
	}

	if (inplane->bitsPerPixel == 16) {
		for (l = 0; l < inplane->lengthPx; l++)
		{
			pWSrc = (UINT16 *)(inplane->buffer + (l * inplane->lineLen));
			modl = l % pta->heigth;
			pDst = outplane->buffer + (l * outplane->lineLen);
			for (i = 0; i < (int)inplane->WidthPx; i++)
			{
				if (pWSrc[i])
				{
					WSrc = lut16[pWSrc[i]];
					if (WSrc)
					{
						taindex = (modl * pta->width) + (i % pta->width);
						if (WSrc < WLimit) { // 0 - WLimit -> 0 or 1
							if ((UINT64)WSrc * 65536 / WT1 >= (UINT64)pta->ta16[taindex]) {
								outplane->DropCount[0]++;
								switch (i % 4) {
								case 0: *pDst = 0x40; break;
								case 1: *pDst |= 0x10; break;
								case 2: *pDst |= 0x04; break;
								case 3: *pDst |= 0x01; break;
								}
							}
						}
						else { // WLimit - 65535 -> 1 or 2
							if ((UINT64)(WSrc + 1 - WLimit) * 65536 / WT2 >= (UINT64)pta->ta16[taindex]) {
								outplane->DropCount[1]++;
								switch (i % 4) {
								case 0: *pDst = 0x80; break;
								case 1: *pDst |= 0x20; break;
								case 2: *pDst |= 0x08; break;
								case 3: *pDst |= 0x02; break;
								}
							}
							else {
								outplane->DropCount[0]++;
								switch (i % 4) {
								case 0: *pDst = 0x40; break;
								case 1: *pDst |= 0x10; break;
								case 2: *pDst |= 0x04; break;
								case 3: *pDst |= 0x01; break;
								}
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
					WSrc = lut16[pSrc[i]*257];
					if (WSrc)
					{
						taindex = (modl * pta->width) + (i % pta->width);
						if (WSrc < WLimit) { // 0 - WLimit -> 0 or 1
							if ((UINT64)WSrc * 65536 / WT1 >= (UINT64)pta->ta16[taindex]) {
								outplane->DropCount[0]++;
								switch (i % 4) {
								case 0: *pDst = 0x40; break;
								case 1: *pDst |= 0x10; break;
								case 2: *pDst |= 0x04; break;
								case 3: *pDst |= 0x01; break;
								}
							}
						}
						else { // WLimit - 65535 -> 1 or 2
							if ((UINT64)(WSrc + 1 - WLimit) * 65536 / WT2 >= (UINT64)pta->ta16[taindex]) {
								outplane->DropCount[1]++;
								switch (i % 4) {
								case 0: *pDst = 0x80; break;
								case 1: *pDst |= 0x20; break;
								case 2: *pDst |= 0x08; break;
								case 3: *pDst |= 0x02; break;
								}
							}
							else {
								outplane->DropCount[0]++;
								switch (i % 4) {
								case 0: *pDst = 0x40; break;
								case 1: *pDst |= 0x10; break;
								case 2: *pDst |= 0x04; break;
								case 3: *pDst |= 0x01; break;
								}
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


//--- rx_screen_plane_FMS_1x2sx2 ----------------------------------------------------------------------
int rx_screen_plane_FMS_1x2sx2(SPlaneInfo *inplane, SPlaneInfo *outplane, void* epscreenConfig)
{
	SScreenConfig* pscreenConfig = epscreenConfig;
	UINT32 li, lo;
	int pi, po, modl, taindex;
	BYTE *pSrc, *pDst;
	UINT16 *pWSrc, WSrc, WLimit = 32768, WT1 = 32768, WT2 = 32768;
	STaConfig *pta = pscreenConfig->TA[inplane->planenumber % 8];
	UINT16 *lut16 = pscreenConfig->lut16[inplane->planenumber];

	if ((pscreenConfig->inputResol.x * 4 == pscreenConfig->outputResol.x) && (pscreenConfig->inputResol.y * 4 == pscreenConfig->outputResol.y)) {
		return (rx_screen_plane_FMS_1x2sx4(inplane, outplane, pscreenConfig));
	}

	if (rx_screen_outplane_init(inplane, outplane, pscreenConfig))
		return REPLY_ERROR;

	if (pscreenConfig->limit[outplane->planenumber][0]) {
		WT1 = WLimit = 65536 * pscreenConfig->limit[outplane->planenumber][0] / 100;
		if (!WT1) WT1 = 1;
		WT2 = 65536 - WT1;
		if (!WT2) WT2 = 1;
	}

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
					for (po = (2 * pi); po < (2 * pi) + 2; po++)
					{
						if (pWSrc[pi])
						{
							WSrc = lut16[pWSrc[pi]];
							if (WSrc)
							{
								taindex = (modl * pta->width) + (po % pta->width);
								if (WSrc < WLimit) { // 0 - WLimit -> 0 or 1
									if ((UINT64)WSrc * 65536 / WT1 >= (UINT64)pta->ta16[taindex]) {
										outplane->DropCount[0]++;
										switch (po % 4) {
										case 0: *pDst = 0x40; break;
										case 1: *pDst |= 0x10; break;
										case 2: *pDst |= 0x04; break;
										case 3: *pDst |= 0x01; break;
										}
									}
								}
								else { // WLimit - 65535 -> 1 or 2
									if ((UINT64)(WSrc + 1 - WLimit) * 65536 / WT2 >= (UINT64)pta->ta16[taindex]) {
										outplane->DropCount[1]++;
										switch (po % 4) {
										case 0: *pDst = 0x80; break;
										case 1: *pDst |= 0x20; break;
										case 2: *pDst |= 0x08; break;
										case 3: *pDst |= 0x02; break;
										}
									}
									else {
										outplane->DropCount[0]++;
										switch (po % 4) {
										case 0: *pDst = 0x40; break;
										case 1: *pDst |= 0x10; break;
										case 2: *pDst |= 0x04; break;
										case 3: *pDst |= 0x01; break;
										}
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
							if (WSrc)
							{
								taindex = (modl * pta->width) + (po % pta->width);
								if (WSrc < WLimit) { // 0 - WLimit -> 0 or 1
									if ((UINT64)WSrc * 65536 / WT1 >= (UINT64)pta->ta16[taindex]) {
										outplane->DropCount[0]++;
										switch (po % 4) {
										case 0: *pDst = 0x40; break;
										case 1: *pDst |= 0x10; break;
										case 2: *pDst |= 0x04; break;
										case 3: *pDst |= 0x01; break;
										}
									}
								}
								else { // WLimit - 65535 -> 1 or 2
									if ((UINT64)(WSrc + 1 - WLimit) * 65536 / WT2 >= (UINT64)pta->ta16[taindex]) {
										outplane->DropCount[1]++;
										switch (po % 4) {
										case 0: *pDst = 0x80; break;
										case 1: *pDst |= 0x20; break;
										case 2: *pDst |= 0x08; break;
										case 3: *pDst |= 0x02; break;
										}
									}
									else {
										outplane->DropCount[0]++;
										switch (po % 4) {
										case 0: *pDst = 0x40; break;
										case 1: *pDst |= 0x10; break;
										case 2: *pDst |= 0x04; break;
										case 3: *pDst |= 0x01; break;
										}
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


//--- rx_screen_plane_FMS_1x2sx4 ----------------------------------------------------------------------
int rx_screen_plane_FMS_1x2sx4(SPlaneInfo *inplane, SPlaneInfo *outplane, void* epscreenConfig)
{
	SScreenConfig* pscreenConfig = epscreenConfig;
	UINT32 li, lo;
	int pi, po, modl, taindex;
	BYTE *pSrc, *pDst;
	UINT16 *pWSrc, WSrc, WLimit = 32768, WT1 = 32768, WT2 = 32768;
	STaConfig *pta = pscreenConfig->TA[inplane->planenumber % 8];
	UINT16 *lut16 = pscreenConfig->lut16[inplane->planenumber];

	if (rx_screen_outplane_init(inplane, outplane, pscreenConfig))
		return REPLY_ERROR;

	if (pscreenConfig->limit[outplane->planenumber][0]) {
		WT1 = WLimit = 65536 * pscreenConfig->limit[outplane->planenumber][0] / 100;
		if (!WT1) WT1 = 1;
		WT2 = 65536 - WT1;
		if (!WT2) WT2 = 1;
	}

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
							if (WSrc)
							{
								taindex = (modl * pta->width) + (po % pta->width);
								if (WSrc < WLimit) { // 0 - WLimit -> 0 or 1
									if ((UINT64)WSrc * 65536 / WT1 >= (UINT64)pta->ta16[taindex]) {
										outplane->DropCount[0]++;
										switch (po % 4) {
										case 0: *pDst = 0x40; break;
										case 1: *pDst |= 0x10; break;
										case 2: *pDst |= 0x04; break;
										case 3: *pDst |= 0x01; break;
										}
									}
								}
								else { // WLimit - 65535 -> 1 or 2
									if ((UINT64)(WSrc + 1 - WLimit) * 65536 / WT2 >= (UINT64)pta->ta16[taindex]) {
										outplane->DropCount[1]++;
										switch (po % 4) {
										case 0: *pDst = 0x80; break;
										case 1: *pDst |= 0x20; break;
										case 2: *pDst |= 0x08; break;
										case 3: *pDst |= 0x02; break;
										}
									}
									else {
										outplane->DropCount[0]++;
										switch (po % 4) {
										case 0: *pDst = 0x40; break;
										case 1: *pDst |= 0x10; break;
										case 2: *pDst |= 0x04; break;
										case 3: *pDst |= 0x01; break;
										}
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
							if (WSrc)
							{
								taindex = (modl * pta->width) + (po % pta->width);
								if (WSrc < WLimit) { // 0 - WLimit -> 0 or 1
									if ((UINT64)WSrc * 65536 / WT1 >= (UINT64)pta->ta16[taindex]) {
										outplane->DropCount[0]++;
										switch (po % 4) {
										case 0: *pDst = 0x40; break;
										case 1: *pDst |= 0x10; break;
										case 2: *pDst |= 0x04; break;
										case 3: *pDst |= 0x01; break;
										}
									}
								}
								else { // WLimit - 65535 -> 1 or 2
									if ((UINT64)(WSrc + 1 - WLimit) * 65536 / WT2 >= (UINT64)pta->ta16[taindex]) {
										outplane->DropCount[1]++;
										switch (po % 4) {
										case 0: *pDst = 0x80; break;
										case 1: *pDst |= 0x20; break;
										case 2: *pDst |= 0x08; break;
										case 3: *pDst |= 0x02; break;
										}
									}
									else {
										outplane->DropCount[0]++;
										switch (po % 4) {
										case 0: *pDst = 0x40; break;
										case 1: *pDst |= 0x10; break;
										case 2: *pDst |= 0x04; break;
										case 3: *pDst |= 0x01; break;
										}
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


//--- rx_screen_init_FMS_1x2s ----------------------------------------------------------------------
int rx_screen_init_FMS_1x2s(SScreenConfig* pscreenConfig)
{

	sprintf(pscreenConfig->typeName, "FMS_1x2s");
	pscreenConfig->fctplane = &rx_screen_plane_FMS_1x2s;
	pscreenConfig->fctclose = &rx_screen_close_null;

	return (REPLY_OK);
}


//--- rx_screen_plane_FMS_1x2g ----------------------------------------------------------------------
int rx_screen_plane_FMS_1x2g(SPlaneInfo *inplane, SPlaneInfo *outplane, void* epscreenConfig)
{
	SScreenConfig* pscreenConfig = epscreenConfig;
	UINT32 l;
	int i, modl, taindex;
	BYTE Dst, *pSrc, *pDst, Val1=1, Val2= 2;
	UINT16 *pWSrc, WSrc, WLimit = 32768, WT2= 32768;
	STaConfig *pta = pscreenConfig->TA[inplane->planenumber % 8];
	UINT16 *lut16 = pscreenConfig->lut16[inplane->planenumber];

	if ((pscreenConfig->inputResol.x * 2 == pscreenConfig->outputResol.x) && (pscreenConfig->inputResol.y * 2 == pscreenConfig->outputResol.y)) {
		return (rx_screen_plane_FMS_1x2gx2(inplane, outplane, pscreenConfig));
	}
	else if ((pscreenConfig->inputResol.x * 4 == pscreenConfig->outputResol.x) && (pscreenConfig->inputResol.y * 4 == pscreenConfig->outputResol.y)) {
		return (rx_screen_plane_FMS_1x2gx4(inplane, outplane, pscreenConfig));
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

	if (inplane->bitsPerPixel == 16) {
		for (l = 0; l < inplane->lengthPx; l++)
		{
			pWSrc = (UINT16 *)(inplane->buffer + (l * inplane->lineLen));
			modl = l % pta->heigth;
			pDst = outplane->buffer + (l * outplane->lineLen);
			for (i = 0; i < (int)inplane->WidthPx; i++)
			{
				if (pWSrc[i])
				{
					WSrc = lut16[pWSrc[i]];
					if (WSrc)
					{
						taindex = (modl * pta->width) + (i % pta->width);
						if ((WSrc >= pta->ta16[taindex])) {	// 1-limit -> 0 or 1
							Dst = Val1;
							if (WSrc > WLimit)				// limit-255 -> 0, 1 or 2				
								if ((UINT64)(WSrc + 1 - WLimit) * 65536 / WT2 >= (UINT64)pta->ta16[taindex])
									Dst = Val2;

							outplane->DropCount[Dst - 1]++;
							switch (i % 4) {
							case 0: *pDst = Dst << 6; break;
							case 1: *pDst |= Dst << 4; break;
							case 2: *pDst |= Dst << 2; break;
							case 3: *pDst |= Dst; break;
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
					WSrc = lut16[pSrc[i]*257];
					if (WSrc)
					{
						taindex = (modl * pta->width) + (i % pta->width);
						if ((WSrc >= pta->ta16[taindex])) {	// 1-limit -> 0 or 1
							Dst = Val1;
							if (WSrc > WLimit)				// limit-255 -> 0, 1 or 2				
								if ((UINT64)(WSrc + 1 - WLimit) * 65536 / WT2 >= (UINT64)pta->ta16[taindex])
									Dst = Val2;

							outplane->DropCount[Dst - 1]++;
							switch (i % 4) {
							case 0: *pDst = Dst << 6; break;
							case 1: *pDst |= Dst << 4; break;
							case 2: *pDst |= Dst << 2; break;
							case 3: *pDst |= Dst; break;
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


//--- rx_screen_plane_FMS_1x2gx2 ----------------------------------------------------------------------
int rx_screen_plane_FMS_1x2gx2(SPlaneInfo *inplane, SPlaneInfo *outplane, void* epscreenConfig)
{
	SScreenConfig* pscreenConfig = epscreenConfig;
	UINT32 li, lo;
	int pi, po, modl, taindex;
	BYTE Dst, *pSrc, *pDst, Val1 = 1, Val2 = 2;
	UINT16 *pWSrc, WSrc, WLimit = 32768, WT2 = 32768;
	STaConfig *pta = pscreenConfig->TA[inplane->planenumber % 8];
	UINT16 *lut16 = pscreenConfig->lut16[inplane->planenumber];

	if ((pscreenConfig->inputResol.x * 4 == pscreenConfig->outputResol.x) && (pscreenConfig->inputResol.y * 4 == pscreenConfig->outputResol.y)) {
		return (rx_screen_plane_FMS_1x2gx4(inplane, outplane, pscreenConfig));
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
					for (po = (2 * pi); po < (2 * pi) + 2; po++)
					{
						if (pWSrc[pi])
						{
							WSrc = lut16[pWSrc[pi]];
							if (WSrc)
							{
								taindex = (modl * pta->width) + (po % pta->width);
								if ((WSrc >= pta->ta16[taindex])) {	// 1-limit -> 0 or 1
									Dst = Val1;
									if (WSrc > WLimit)				// limit-255 -> 0, 1 or 2				
										if ((UINT64)(WSrc + 1 - WLimit) * 65536 / WT2 >= (UINT64)pta->ta16[taindex])
											Dst = Val2;

									outplane->DropCount[Dst - 1]++;
									switch (po % 4) {
									case 0: *pDst = Dst << 6; break;
									case 1: *pDst |= Dst << 4; break;
									case 2: *pDst |= Dst << 2; break;
									case 3: *pDst |= Dst; break;
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
							if (WSrc)
							{
								taindex = (modl * pta->width) + (po % pta->width);
								if ((WSrc >= pta->ta16[taindex])) {	// 1-limit -> 0 or 1
									Dst = Val1;
									if (WSrc > WLimit)				// limit-255 -> 0, 1 or 2				
										if ((UINT64)(WSrc + 1 - WLimit) * 65536 / WT2 >= (UINT64)pta->ta16[taindex])
											Dst = Val2;

									outplane->DropCount[Dst - 1]++;
									switch (po % 4) {
									case 0: *pDst = Dst << 6; break;
									case 1: *pDst |= Dst << 4; break;
									case 2: *pDst |= Dst << 2; break;
									case 3: *pDst |= Dst; break;
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


//--- rx_screen_plane_FMS_1x2gx4 ----------------------------------------------------------------------
int rx_screen_plane_FMS_1x2gx4(SPlaneInfo *inplane, SPlaneInfo *outplane, void* epscreenConfig)
{
	SScreenConfig* pscreenConfig = epscreenConfig;
	UINT32 li, lo;
	int pi, po, modl, taindex;
	BYTE Dst, *pSrc, *pDst, Val1 = 1, Val2 = 2;
	UINT16 *pWSrc, WSrc, WLimit = 32768, WT2 = 32768;
	STaConfig *pta = pscreenConfig->TA[inplane->planenumber % 8];
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
							if (WSrc)
							{
								taindex = (modl * pta->width) + (po % pta->width);
								if ((WSrc >= pta->ta16[taindex])) {	// 1-limit -> 0 or 1
									Dst = Val1;
									if (WSrc > WLimit)				// limit-255 -> 0, 1 or 2				
										if ((UINT64)(WSrc + 1 - WLimit) * 65536 / WT2 >= (UINT64)pta->ta16[taindex])
											Dst = Val2;

									outplane->DropCount[Dst - 1]++;
									switch (po % 4) {
									case 0: *pDst = Dst << 6; break;
									case 1: *pDst |= Dst << 4; break;
									case 2: *pDst |= Dst << 2; break;
									case 3: *pDst |= Dst; break;
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
							if (WSrc)
							{
								taindex = (modl * pta->width) + (po % pta->width);
								if ((WSrc >= pta->ta16[taindex])) {	// 1-limit -> 0 or 1
									Dst = Val1;
									if (WSrc > WLimit)				// limit-255 -> 0, 1 or 2				
										if ((UINT64)(WSrc + 1 - WLimit) * 65536 / WT2 >= (UINT64)pta->ta16[taindex])
											Dst = Val2;

									outplane->DropCount[Dst - 1]++;
									switch (po % 4) {
									case 0: *pDst = Dst << 6; break;
									case 1: *pDst |= Dst << 4; break;
									case 2: *pDst |= Dst << 2; break;
									case 3: *pDst |= Dst; break;
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


//--- rx_screen_init_FMS_1x2g ----------------------------------------------------------------------
int rx_screen_init_FMS_1x2g(SScreenConfig* pscreenConfig)
{


	sprintf(pscreenConfig->typeName, "FMS_1x2g");
	pscreenConfig->fctplane = &rx_screen_plane_FMS_1x2g;
	pscreenConfig->fctclose = &rx_screen_close_null;

	return (REPLY_OK);
}


//--- rx_screen_plane_FMS_1x2r ----------------------------------------------------------------------
int rx_screen_plane_FMS_1x2r(SPlaneInfo *inplane, SPlaneInfo *outplane, void* epscreenConfig)
{
	SScreenConfig* pscreenConfig = epscreenConfig;
	UINT32 l;
	int i, modl, taindex;
	BYTE Dst, *pSrc, *pDst, Val1 = 1, Val2 = 2;
	UINT16 *pWSrc, WSrc;
	STaConfig *pta = pscreenConfig->TA[inplane->planenumber % 8];
	UINT16 *lut16 = pscreenConfig->lut16[inplane->planenumber];
	UINT16 *lut16_L = pscreenConfig->lut16_L[inplane->planenumber];

	if ((pscreenConfig->inputResol.x * 2 == pscreenConfig->outputResol.x) && (pscreenConfig->inputResol.y * 2 == pscreenConfig->outputResol.y)) {
		return (rx_screen_plane_FMS_1x2rx2(inplane, outplane, pscreenConfig));
	}
	else if ((pscreenConfig->inputResol.x * 4 == pscreenConfig->outputResol.x) && (pscreenConfig->inputResol.y * 4 == pscreenConfig->outputResol.y)) {
		return (rx_screen_plane_FMS_1x2rx4(inplane, outplane, pscreenConfig));
	}

	if (rx_screen_outplane_init(inplane, outplane, pscreenConfig))
		return REPLY_ERROR;

	if (pscreenConfig->dropsize == 13)
		Val2 = 3;
	else if (pscreenConfig->dropsize == 23) {
		Val1 = 2;
		Val2 = 3;
	}

	if (inplane->bitsPerPixel == 16) {
		for (l = 0; l < inplane->lengthPx; l++)
		{
			pWSrc = (UINT16 *)(inplane->buffer + (l * inplane->lineLen));
			modl = l % pta->heigth;
			pDst = outplane->buffer + (l * outplane->lineLen);
			for (i = 0; i < (int)inplane->WidthPx; i++)
			{
				if (pWSrc[i])
				{
					WSrc = lut16[pWSrc[i]];
					if (WSrc)
					{
						taindex = (modl * pta->width) + (i % pta->width);
						if ((WSrc >= pta->ta16[taindex])) {	// 0 or bigger
							Dst = Val1;
							WSrc = lut16_L[pWSrc[i]];
							if ((WSrc >= pta->ta16[taindex])) // 1 or bigger			
								Dst = Val2;

							outplane->DropCount[Dst - 1]++;
							switch (i % 4) {
							case 0: *pDst = Dst << 6; break;
							case 1: *pDst |= Dst << 4; break;
							case 2: *pDst |= Dst << 2; break;
							case 3: *pDst |= Dst; break;
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
					if (WSrc)
					{
						taindex = (modl * pta->width) + (i % pta->width);
						if ((WSrc >= pta->ta16[taindex])) {	// 0 or bigger
							Dst = Val1;
							WSrc = lut16_L[pSrc[i] * 257];
							if ((WSrc >= pta->ta16[taindex])) // 1 or bigger				
								Dst = Val2;

							outplane->DropCount[Dst - 1]++;
							switch (i % 4) {
							case 0: *pDst = Dst << 6; break;
							case 1: *pDst |= Dst << 4; break;
							case 2: *pDst |= Dst << 2; break;
							case 3: *pDst |= Dst; break;
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


//--- rx_screen_plane_FMS_1x2rx2 ----------------------------------------------------------------------
int rx_screen_plane_FMS_1x2rx2(SPlaneInfo *inplane, SPlaneInfo *outplane, void* epscreenConfig)
{
	SScreenConfig* pscreenConfig = epscreenConfig;
	UINT32 li, lo;
	int pi, po, modl, taindex;
	BYTE Dst, *pSrc, *pDst, Val1 = 1, Val2 = 2;
	UINT16 *pWSrc, WSrc;
	STaConfig *pta = pscreenConfig->TA[inplane->planenumber % 8];
	UINT16 *lut16 = pscreenConfig->lut16[inplane->planenumber];
	UINT16 *lut16_L = pscreenConfig->lut16_L[inplane->planenumber];

	if ((pscreenConfig->inputResol.x * 4 == pscreenConfig->outputResol.x) && (pscreenConfig->inputResol.y * 4 == pscreenConfig->outputResol.y)) {
		return (rx_screen_plane_FMS_1x2rx4(inplane, outplane, pscreenConfig));
	}

	if (rx_screen_outplane_init(inplane, outplane, pscreenConfig))
		return REPLY_ERROR;

	if (pscreenConfig->dropsize == 13)
		Val2 = 3;
	else if (pscreenConfig->dropsize == 23) {
		Val1 = 2;
		Val2 = 3;
	}

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
					for (po = (2 * pi); po < (2 * pi) + 2; po++)
					{
						if (pWSrc[pi])
						{
							WSrc = lut16[pWSrc[pi]];
							if (WSrc)
							{
								taindex = (modl * pta->width) + (po % pta->width);
								if ((WSrc >= pta->ta16[taindex])) {	// // 0 or bigger
									Dst = Val1;
									WSrc = lut16_L[pWSrc[pi]];
									if ((WSrc >= pta->ta16[taindex])) // 1 or bigger				
										Dst = Val2;

									outplane->DropCount[Dst - 1]++;
									switch (po % 4) {
									case 0: *pDst = Dst << 6; break;
									case 1: *pDst |= Dst << 4; break;
									case 2: *pDst |= Dst << 2; break;
									case 3: *pDst |= Dst; break;
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
							if (WSrc)
							{
								taindex = (modl * pta->width) + (po % pta->width);
								if ((WSrc >= pta->ta16[taindex])) {	// 0 or bigger
									Dst = Val1;
									WSrc = lut16_L[pSrc[pi] * 257];
									if ((WSrc >= pta->ta16[taindex])) // 1 or bigger				
										Dst = Val2;

									outplane->DropCount[Dst - 1]++;
									switch (po % 4) {
									case 0: *pDst = Dst << 6; break;
									case 1: *pDst |= Dst << 4; break;
									case 2: *pDst |= Dst << 2; break;
									case 3: *pDst |= Dst; break;
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


//--- rx_screen_plane_FMS_1x2rx4 ----------------------------------------------------------------------
int rx_screen_plane_FMS_1x2rx4(SPlaneInfo *inplane, SPlaneInfo *outplane, void* epscreenConfig)
{
	SScreenConfig* pscreenConfig = epscreenConfig;
	UINT32 li, lo;
	int pi, po, modl, taindex;
	BYTE Dst, *pSrc, *pDst, Val1 = 1, Val2 = 2;
	UINT16 *pWSrc, WSrc;
	STaConfig *pta = pscreenConfig->TA[inplane->planenumber % 8];
	UINT16 *lut16 = pscreenConfig->lut16[inplane->planenumber];
	UINT16 *lut16_L = pscreenConfig->lut16_L[inplane->planenumber];

	if (rx_screen_outplane_init(inplane, outplane, pscreenConfig))
		return REPLY_ERROR;

	if (pscreenConfig->dropsize == 13)
		Val2 = 3;
	else if (pscreenConfig->dropsize == 23) {
		Val1 = 2;
		Val2 = 3;
	}

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
							if (WSrc)
							{
								taindex = (modl * pta->width) + (po % pta->width);
								if ((WSrc >= pta->ta16[taindex])) {	// 0 or bigger
									Dst = Val1;
									WSrc = lut16_L[pWSrc[pi]];
									if ((WSrc >= pta->ta16[taindex])) // 1 or bigger				
										Dst = Val2;

									outplane->DropCount[Dst - 1]++;
									switch (po % 4) {
									case 0: *pDst = Dst << 6; break;
									case 1: *pDst |= Dst << 4; break;
									case 2: *pDst |= Dst << 2; break;
									case 3: *pDst |= Dst; break;
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
							if (WSrc)
							{
								taindex = (modl * pta->width) + (po % pta->width);
								if ((WSrc >= pta->ta16[taindex])) {	// 0 or bigger
									Dst = Val1;
									WSrc = lut16_L[pSrc[pi] * 257];
									if ((WSrc >= pta->ta16[taindex])) // 1 or bigger				
										Dst = Val2;

									outplane->DropCount[Dst - 1]++;
									switch (po % 4) {
									case 0: *pDst = Dst << 6; break;
									case 1: *pDst |= Dst << 4; break;
									case 2: *pDst |= Dst << 2; break;
									case 3: *pDst |= Dst; break;
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



//--- rx_screen_init_FMS_1x2r ----------------------------------------------------------------------
int rx_screen_init_FMS_1x2r(SScreenConfig* pscreenConfig)
{
	float totR, largerR, totminR, totmaxR, largerminR, largermaxR;
	int i, j, r1Input, r2Input, r3Input;
	
	sprintf(pscreenConfig->typeName, "FMS_1x2r");
	pscreenConfig->fctplane = &rx_screen_plane_FMS_1x2r;
	pscreenConfig->fctclose = &rx_screen_close_null;

	for ( i = 0; i < MAX_COLORS; i++) {

		// check to avoid input=0
		if (pscreenConfig->ratio[i].r1.input == 0)
			pscreenConfig->ratio[i].r3 = pscreenConfig->ratio[i].r2 = pscreenConfig->ratio[i].r1 = pscreenConfig->ratio[i].r100;

		r1Input = 65536 * pscreenConfig->ratio[i].r1.input / 100;
		r2Input = 65536 * pscreenConfig->ratio[i].r2.input / 100;
		r3Input = 65536 * pscreenConfig->ratio[i].r3.input / 100;

		if (pscreenConfig->ratio[i].r1.input) { // 1st slice
			if (pscreenConfig->dropsize == 12) // init 
				largermaxR = (float)pscreenConfig->ratio[i].r1.mediumD / pscreenConfig->ratio[i].r1.input;
			else
				largermaxR = (float)pscreenConfig->ratio[i].r1.largeD / pscreenConfig->ratio[i].r1.input;
			if (largermaxR > 1.0) largermaxR = 1.0;
			largerR = largerminR = largermaxR;

			totmaxR = (float)(pscreenConfig->ratio[i].r1.smallD + pscreenConfig->ratio[i].r1.mediumD + pscreenConfig->ratio[i].r1.largeD) / pscreenConfig->ratio[i].r1.input;
			if (totmaxR > 1.0) totmaxR = 1.0;
			totR = totminR = totmaxR;

			for (j = 0; j < r1Input; j++) { // 1st slice from 0 to r1
				pscreenConfig->lut16_L[i][j] = (UINT16)(largerR * pscreenConfig->lut16[i][j]);	//  lut for larger dot
				pscreenConfig->lut16[i][j] = (UINT16)(totR * pscreenConfig->lut16[i][j]);		// lut for any dot
			}
		}

		if (pscreenConfig->ratio[i].r2.input > pscreenConfig->ratio[i].r1.input) { // 2nd slice
			if (pscreenConfig->dropsize == 12) // init 
				largermaxR = (float)pscreenConfig->ratio[i].r2.mediumD / pscreenConfig->ratio[i].r2.input ;
			else
				largermaxR = (float)pscreenConfig->ratio[i].r2.largeD  / pscreenConfig->ratio[i].r2.input;
			if (largermaxR > 1.0) largermaxR = 1.0;

			totmaxR = (float)(pscreenConfig->ratio[i].r2.smallD + pscreenConfig->ratio[i].r2.mediumD + pscreenConfig->ratio[i].r2.largeD)
							/ pscreenConfig->ratio[i].r2.input;
/*			totmaxR = (float)((pscreenConfig->ratio[i].r2.smallD + pscreenConfig->ratio[i].r2.mediumD + pscreenConfig->ratio[i].r2.largeD) -
				(pscreenConfig->ratio[i].r1.smallD + pscreenConfig->ratio[i].r1.mediumD + pscreenConfig->ratio[i].r1.largeD))
				/ (pscreenConfig->ratio[i].r2.input - pscreenConfig->ratio[i].r1.input);
*/
			if (totmaxR > 1.0) totmaxR = 1.0;

			for (j = r1Input; j < r2Input; j++) { // 2nd slice from r1 to r2
				largerR = largerminR + ((largermaxR - largerminR)*((float)(j- r1Input)/ (float)(r2Input-r1Input)));
				pscreenConfig->lut16_L[i][j] = (UINT16)(largerR * pscreenConfig->lut16[i][j]);	//  lut for larger dot
				totR = totminR + ((totmaxR - totminR) * ((float)(j - r1Input) / (float)(r2Input - r1Input)));
				pscreenConfig->lut16[i][j] = (UINT16)(totR * pscreenConfig->lut16[i][j]);		// lut for any dot
			}
			totminR = totR;
			largerminR = largerR;
		}
		else
			pscreenConfig->ratio[i].r2 = pscreenConfig->ratio[i].r1;

		if (pscreenConfig->ratio[i].r3.input > pscreenConfig->ratio[i].r2.input) { // 3rd slice
			if (pscreenConfig->dropsize == 12) // init
				largermaxR = (float)pscreenConfig->ratio[i].r3.mediumD  / pscreenConfig->ratio[i].r3.input ;
			else
				largermaxR = (float)pscreenConfig->ratio[i].r3.largeD  / pscreenConfig->ratio[i].r3.input ;
			if (largermaxR > 1.0) largermaxR = 1.0;

			totmaxR = (float)(pscreenConfig->ratio[i].r3.smallD + pscreenConfig->ratio[i].r3.mediumD + pscreenConfig->ratio[i].r3.largeD) 
				/ pscreenConfig->ratio[i].r3.input ;

			if (totmaxR > 1.0) totmaxR = 1.0;

			for (j = r2Input; j < r3Input; j++) { // 2nd slice from r1 to r2
				largerR = largerminR + ((largermaxR - largerminR)*((float)(j - r2Input) / (float)(r3Input - r2Input)));
				pscreenConfig->lut16_L[i][j] = (UINT16)(largerR * pscreenConfig->lut16[i][j]);	//  lut for larger dot
				totR = totminR + ((totmaxR - totminR) * ((float)(j - r2Input) / (float)(r3Input - r2Input)));
				pscreenConfig->lut16[i][j] = (UINT16)(totR * pscreenConfig->lut16[i][j]);		// lut for any dot
			}
			totminR = totR;
			largerminR = largerR;
		}
		else
			pscreenConfig->ratio[i].r3 = pscreenConfig->ratio[i].r2;


		if (100 > pscreenConfig->ratio[i].r3.input) { // last slice
			if (pscreenConfig->dropsize == 12) // init 
				largermaxR = (float)pscreenConfig->ratio[i].r100.mediumD  / 100 ;
			else
				largermaxR = (float)pscreenConfig->ratio[i].r100.largeD  / 100 ;
			if (largermaxR > 1.0) largermaxR = 1.0;

			totmaxR = (float)(pscreenConfig->ratio[i].r100.smallD + pscreenConfig->ratio[i].r100.mediumD + pscreenConfig->ratio[i].r100.largeD) 
				/ 100 ;

			if (totmaxR > 1.0) totmaxR = 1.0;

			for (j = r3Input; j < 65536; j++) { // 2nd slice from r1 to r2
				largerR = largerminR + ((largermaxR - largerminR)*((float)(j - r3Input) / (float)(65536 - r3Input)));
				pscreenConfig->lut16_L[i][j] = (UINT16)(largerR * pscreenConfig->lut16[i][j]);	//  lut for larger dot
				totR = totminR + ((totmaxR - totminR) * ((float)(j - r3Input) / (float)(65536 - r3Input)));
				pscreenConfig->lut16[i][j] = (UINT16)(totR * pscreenConfig->lut16[i][j]);		// lut for any dot
			}
		}
	}

	return (REPLY_OK);
}
