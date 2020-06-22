// ****************************************************************************
//
//	rx_screen_fms_1x3.c
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
#include "rx_screen_fms_1x3.h"

#ifdef linux
	#include "errno.h"
#endif

static BYTE			Val4[4];


//--- rx_screen_plane_FMS_1x3s ----------------------------------------------------------------------
int rx_screen_plane_FMS_1x3s(SPlaneInfo *inplane, SPlaneInfo *outplane, void* epscreenConfig)
{
	SScreenConfig* pscreenConfig = epscreenConfig;
	UINT32 l;
	int i, modl, taindex;
	BYTE *pSrc, *pDst;
	UINT16 *pWSrc, WSrc, WLimit1 = 21845, WLimit2 = 43690, WT1 = 21845, WT2 = 21845, WT3 = 21845;
	STaConfig *pta = pscreenConfig->TA[inplane->planenumber % 8];
	UINT16 *lut16 = pscreenConfig->lut16[inplane->planenumber];

	if ((pscreenConfig->inputResol.x * 2 == pscreenConfig->outputResol.x) && (pscreenConfig->inputResol.y * 2 == pscreenConfig->outputResol.y)) {
		return (rx_screen_plane_FMS_1x3sx2(inplane, outplane, pscreenConfig));
	}
	else if ((pscreenConfig->inputResol.x * 4 == pscreenConfig->outputResol.x) && (pscreenConfig->inputResol.y * 4 == pscreenConfig->outputResol.y)) {
		return (rx_screen_plane_FMS_1x3sx4(inplane, outplane, pscreenConfig));
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
					if (WSrc) {

						taindex = (modl * pta->width) + (i % pta->width);
						if (WSrc < WLimit1) { // 0 - WLimit1 -> 0 or 1
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
						else if (WSrc < WLimit2) { // WLimit1 - WLimit2 -> 1 or 2
							if ((UINT64)(WSrc - WLimit1) * 65536 / WT2 >= (UINT64)pta->ta16[taindex]) {
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
						else {						// WLimit2 - 65535 -> 2 to 3
							if ((UINT64)(WSrc - WLimit2) * 65536 / WT3 >= (UINT64)pta->ta16[taindex]) {
								outplane->DropCount[2]++;
								switch (i % 4) {
								case 0: *pDst = 0xC0; break;
								case 1: *pDst |= 0x30; break;
								case 2: *pDst |= 0x0C; break;
								case 3: *pDst |= 0x03; break;
								}
							}
							else {
								switch (i % 4) {
									outplane->DropCount[1]++;
								case 0: *pDst = 0x80; break;
								case 1: *pDst |= 0x20; break;
								case 2: *pDst |= 0x08; break;
								case 3: *pDst |= 0x02; break;
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
					if (WSrc) {

						taindex = (modl * pta->width) + (i % pta->width);
						if (WSrc < WLimit1) { // 0 - WLimit1 -> 0 or 1
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
						else if (WSrc < WLimit2) { // WLimit1 - WLimit2 -> 1 or 2
							if ((UINT64)(WSrc - WLimit1) * 65536 / WT2 >= (UINT64)pta->ta16[taindex]) {
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
						else {						// WLimit2 - 65535 -> 2 to 3
							if ((UINT64)(WSrc - WLimit2) * 65536 / WT3 >= (UINT64)pta->ta16[taindex]) {
								outplane->DropCount[2]++;
								switch (i % 4) {
								case 0: *pDst = 0xC0; break;
								case 1: *pDst |= 0x30; break;
								case 2: *pDst |= 0x0C; break;
								case 3: *pDst |= 0x03; break;
								}
							}
							else {
								switch (i % 4) {
									outplane->DropCount[1]++;
								case 0: *pDst = 0x80; break;
								case 1: *pDst |= 0x20; break;
								case 2: *pDst |= 0x08; break;
								case 3: *pDst |= 0x02; break;
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


//--- rx_screen_plane_FMS_1x3sx2 ----------------------------------------------------------------------
int rx_screen_plane_FMS_1x3sx2(SPlaneInfo *inplane, SPlaneInfo *outplane, void* epscreenConfig)
{
	SScreenConfig* pscreenConfig = epscreenConfig;
	UINT32 li, lo;
	int pi, po, modl, taindex;
	BYTE *pSrc, *pDst;
	UINT16 *pWSrc, WSrc, WLimit1 = 21845, WLimit2 = 43690, WT1 = 21845, WT2 = 21845, WT3 = 21845;
	STaConfig *pta = pscreenConfig->TA[inplane->planenumber % 8];
	UINT16 *lut16 = pscreenConfig->lut16[inplane->planenumber];

	if ((pscreenConfig->inputResol.x * 4 == pscreenConfig->outputResol.x) && (pscreenConfig->inputResol.y * 4 == pscreenConfig->outputResol.y)) {
		return (rx_screen_plane_FMS_1x3sx4(inplane, outplane, pscreenConfig));
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
							if (WSrc) {

								taindex = (modl * pta->width) + (po % pta->width);
								if (WSrc < WLimit1) { // 0 - WLimit1 -> 0 or 1
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
								else if (WSrc < WLimit2) { // WLimit1 - WLimit2 -> 1 or 2
									if ((UINT64)(WSrc - WLimit1) * 65536 / WT2 >= (UINT64)pta->ta16[taindex]) {
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
								else {						// WLimit2 - 65535 -> 2 to 3
									if ((UINT64)(WSrc - WLimit2) * 65536 / WT3 >= (UINT64)pta->ta16[taindex]) {
										outplane->DropCount[2]++;
										switch (po % 4) {
										case 0: *pDst = 0xC0; break;
										case 1: *pDst |= 0x30; break;
										case 2: *pDst |= 0x0C; break;
										case 3: *pDst |= 0x03; break;
										}
									}
									else {
										switch (po % 4) {
											outplane->DropCount[1]++;
										case 0: *pDst = 0x80; break;
										case 1: *pDst |= 0x20; break;
										case 2: *pDst |= 0x08; break;
										case 3: *pDst |= 0x02; break;
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
							if (WSrc) {

								taindex = (modl * pta->width) + (po % pta->width);
								if (WSrc < WLimit1) { // 0 - WLimit1 -> 0 or 1
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
								else if (WSrc < WLimit2) { // WLimit1 - WLimit2 -> 1 or 2
									if ((UINT64)(WSrc - WLimit1) * 65536 / WT2 >= (UINT64)pta->ta16[taindex]) {
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
								else {						// WLimit2 - 65535 -> 2 to 3
									if ((UINT64)(WSrc - WLimit2) * 65536 / WT3 >= (UINT64)pta->ta16[taindex]) {
										outplane->DropCount[2]++;
										switch (po % 4) {
										case 0: *pDst = 0xC0; break;
										case 1: *pDst |= 0x30; break;
										case 2: *pDst |= 0x0C; break;
										case 3: *pDst |= 0x03; break;
										}
									}
									else {
										switch (po % 4) {
											outplane->DropCount[1]++;
										case 0: *pDst = 0x80; break;
										case 1: *pDst |= 0x20; break;
										case 2: *pDst |= 0x08; break;
										case 3: *pDst |= 0x02; break;
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


//--- rx_screen_plane_FMS_1x3sx4 ----------------------------------------------------------------------
int rx_screen_plane_FMS_1x3sx4(SPlaneInfo *inplane, SPlaneInfo *outplane, void* epscreenConfig)
{
	SScreenConfig* pscreenConfig = epscreenConfig;
	UINT32 li, lo;
	int pi, po, modl, taindex;
	BYTE *pSrc, *pDst;
	UINT16 *pWSrc, WSrc, WLimit1 = 21845, WLimit2 = 43690, WT1 = 21845, WT2 = 21845, WT3 = 21845;
	STaConfig *pta = pscreenConfig->TA[inplane->planenumber % 8];
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
								if (WSrc < WLimit1) { // 0 - WLimit1 -> 0 or 1
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
								else if (WSrc < WLimit2) { // WLimit1 - WLimit2 -> 1 or 2
									if ((UINT64)(WSrc - WLimit1) * 65536 / WT2 >= (UINT64)pta->ta16[taindex]) {
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
								else {						// WLimit2 - 65535 -> 2 to 3
									if ((UINT64)(WSrc - WLimit2) * 65536 / WT3 >= (UINT64)pta->ta16[taindex]) {
										outplane->DropCount[2]++;
										switch (po % 4) {
										case 0: *pDst = 0xC0; break;
										case 1: *pDst |= 0x30; break;
										case 2: *pDst |= 0x0C; break;
										case 3: *pDst |= 0x03; break;
										}
									}
									else {
										switch (po % 4) {
											outplane->DropCount[1]++;
										case 0: *pDst = 0x80; break;
										case 1: *pDst |= 0x20; break;
										case 2: *pDst |= 0x08; break;
										case 3: *pDst |= 0x02; break;
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
							if (WSrc) {

								taindex = (modl * pta->width) + (po % pta->width);
								if (WSrc < WLimit1) { // 0 - WLimit1 -> 0 or 1
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
								else if (WSrc < WLimit2) { // WLimit1 - WLimit2 -> 1 or 2
									if ((UINT64)(WSrc - WLimit1) * 65536 / WT2 >= (UINT64)pta->ta16[taindex]) {
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
								else {						// WLimit2 - 65535 -> 2 to 3
									if ((UINT64)(WSrc - WLimit2) * 65536 / WT3 >= (UINT64)pta->ta16[taindex]) {
										outplane->DropCount[2]++;
										switch (po % 4) {
										case 0: *pDst = 0xC0; break;
										case 1: *pDst |= 0x30; break;
										case 2: *pDst |= 0x0C; break;
										case 3: *pDst |= 0x03; break;
										}
									}
									else {
										switch (po % 4) {
											outplane->DropCount[1]++;
										case 0: *pDst = 0x80; break;
										case 1: *pDst |= 0x20; break;
										case 2: *pDst |= 0x08; break;
										case 3: *pDst |= 0x02; break;
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


//--- rx_screen_init_FMS_1x3s ----------------------------------------------------------------------
int rx_screen_init_FMS_1x3s(SScreenConfig* pscreenConfig)
{

	sprintf(pscreenConfig->typeName, "FMS_1x3s");
	pscreenConfig->fctplane = &rx_screen_plane_FMS_1x3s;
	pscreenConfig->fctclose = &rx_screen_close_null;

	return (REPLY_OK);
}


//--- rx_screen_init_FMS_1x3g ----------------------------------------------------------------------
int rx_screen_init_FMS_1x3g(SScreenConfig* pscreenConfig)
{


	sprintf(pscreenConfig->typeName, "FMS_1x3g");
	pscreenConfig->fctplane = &rx_screen_plane_FMS_1x3g;
	pscreenConfig->fctclose = &rx_screen_close_null;

	return (REPLY_OK);
}


//--- rx_screen_plane_FMS_1x3g ----------------------------------------------------------------------
int rx_screen_plane_FMS_1x3g(SPlaneInfo *inplane, SPlaneInfo *outplane, void* epscreenConfig)
{
	SScreenConfig* pscreenConfig = epscreenConfig;
	UINT32 l;
	int i, modl, taindex;
	BYTE Dst, *pSrc, *pDst;
	UINT16 *pWSrc, WSrc, WLimit1 = 21845, WLimit2 = 43690, WT1 = 21845, WT2 = 21845, WT3 = 21845;
	STaConfig *pta = pscreenConfig->TA[inplane->planenumber % 8];
	UINT16 *lut16 = pscreenConfig->lut16[inplane->planenumber];

	if ((pscreenConfig->inputResol.x * 2 == pscreenConfig->outputResol.x) && (pscreenConfig->inputResol.y * 2 == pscreenConfig->outputResol.y)) {
		return (rx_screen_plane_FMS_1x3gx2(inplane, outplane, pscreenConfig));
	}
	else if ((pscreenConfig->inputResol.x * 4 == pscreenConfig->outputResol.x) && (pscreenConfig->inputResol.y * 4 == pscreenConfig->outputResol.y)) {
		return (rx_screen_plane_FMS_1x3gx4(inplane, outplane, pscreenConfig));
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
						if ((WSrc >= pta->ta16[taindex])) {	// 1-WLimit1 -> 0 or 1
							Dst = 1;
							if (WSrc > WLimit1) {
								if (WSrc <= WLimit2) {		// WLimit1-WLimit2 -> 0, 1 or 2
									if ((UINT64)(WSrc - WLimit1) * 65536 / WT2 >= (UINT64)pta->ta16[taindex])
										Dst = 2;
								}
								else {							// limit2-255 -> 0, 2 or 3		
									if ((UINT64)(WSrc - WLimit2) * 65536 / WT3 >= (UINT64)pta->ta16[taindex])
										Dst = 3;
									else
										Dst = 2;
								}
							}

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
						if ((WSrc >= pta->ta16[taindex])) {	// 1-WLimit1 -> 0 or 1
							Dst = 1;
							if (WSrc > WLimit1) {
								if (WSrc <= WLimit2) {		// WLimit1-WLimit2 -> 0, 1 or 2
									if ((UINT64)(WSrc - WLimit1) * 65536 / WT2 >= (UINT64)pta->ta16[taindex])
										Dst = 2;
								}
								else {							// limit2-255 -> 0, 2 or 3		
									if ((UINT64)(WSrc - WLimit2) * 65536 / WT3 >= (UINT64)pta->ta16[taindex])
										Dst = 3;
									else
										Dst = 2;
								}
							}

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


//--- rx_screen_plane_FMS_1x3gx2----------------------------------------------------------------------
int rx_screen_plane_FMS_1x3gx2(SPlaneInfo *inplane, SPlaneInfo *outplane, void* epscreenConfig)
{
	SScreenConfig* pscreenConfig = epscreenConfig;
	UINT32 li, lo;
	int pi, po, modl, taindex;
	BYTE Dst, *pSrc, *pDst;
	UINT16 *pWSrc, WSrc, WLimit1 = 21845, WLimit2 = 43690, WT1 = 21845, WT2 = 21845, WT3 = 21845;
	STaConfig *pta = pscreenConfig->TA[inplane->planenumber % 8];
	UINT16 *lut16 = pscreenConfig->lut16[inplane->planenumber];

	if ((pscreenConfig->inputResol.x * 4 == pscreenConfig->outputResol.x) && (pscreenConfig->inputResol.y * 4 == pscreenConfig->outputResol.y)) {
		return (rx_screen_plane_FMS_1x3gx4(inplane, outplane, pscreenConfig));
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
							if (WSrc) {

								taindex = (modl * pta->width) + (po % pta->width);
								if ((WSrc >= pta->ta16[taindex])) {	// 1-WLimit1 -> 0 or 1
									Dst = 1;
									if (WSrc > WLimit1) {
										if (WSrc <= WLimit2) {		// WLimit1-WLimit2 -> 0, 1 or 2
											if ((UINT64)(WSrc - WLimit1) * 65536 / WT2 >= (UINT64)pta->ta16[taindex])
												Dst = 2;
										}
										else {							// limit2-255 -> 0, 2 or 3		
											if ((UINT64)(WSrc - WLimit2) * 65536 / WT3 >= (UINT64)pta->ta16[taindex])
												Dst = 3;
											else
												Dst = 2;
										}
									}

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
							if (WSrc) {

								taindex = (modl * pta->width) + (po % pta->width);
								if ((WSrc >= pta->ta16[taindex])) {	// 1-WLimit1 -> 0 or 1
									Dst = 1;
									if (WSrc > WLimit1) {
										if (WSrc <= WLimit2) {		// WLimit1-WLimit2 -> 0, 1 or 2
											if ((UINT64)(WSrc - WLimit1) * 65536 / WT2 >= (UINT64)pta->ta16[taindex])
												Dst = 2;
										}
										else {							// limit2-255 -> 0, 2 or 3		
											if ((UINT64)(WSrc - WLimit2) * 65536 / WT3 >= (UINT64)pta->ta16[taindex])
												Dst = 3;
											else
												Dst = 2;
										}
									}

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


//--- rx_screen_plane_FMS_1x3gx4----------------------------------------------------------------------
int rx_screen_plane_FMS_1x3gx4(SPlaneInfo *inplane, SPlaneInfo *outplane, void* epscreenConfig)
{
	SScreenConfig* pscreenConfig = epscreenConfig;
	UINT32 li, lo;
	int pi, po, modl, taindex;
	BYTE Dst, *pSrc, *pDst;
	UINT16 *pWSrc, WSrc, WLimit1 = 21845, WLimit2 = 43690, WT1 = 21845, WT2 = 21845, WT3 = 21845;
	STaConfig *pta = pscreenConfig->TA[inplane->planenumber % 8];
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
								if ((WSrc >= pta->ta16[taindex])) {	// 1-WLimit1 -> 0 or 1
									Dst = 1;
									if (WSrc > WLimit1) {
										if (WSrc <= WLimit2) {		// WLimit1-WLimit2 -> 0, 1 or 2
											if ((UINT64)(WSrc - WLimit1) * 65536 / WT2 >= (UINT64)pta->ta16[taindex])
												Dst = 2;
										}
										else {							// limit2-255 -> 0, 2 or 3		
											if ((UINT64)(WSrc - WLimit2) * 65536 / WT3 >= (UINT64)pta->ta16[taindex])
												Dst = 3;
											else
												Dst = 2;
										}
									}

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
							if (WSrc) {

								taindex = (modl * pta->width) + (po % pta->width);
								if ((WSrc >= pta->ta16[taindex])) {	// 1-WLimit1 -> 0 or 1
									Dst = 1;
									if (WSrc > WLimit1) {
										if (WSrc <= WLimit2) {		// WLimit1-WLimit2 -> 0, 1 or 2
											if ((UINT64)(WSrc - WLimit1) * 65536 / WT2 >= (UINT64)pta->ta16[taindex])
												Dst = 2;
										}
										else {							// limit2-255 -> 0, 2 or 3		
											if ((UINT64)(WSrc - WLimit2) * 65536 / WT3 >= (UINT64)pta->ta16[taindex])
												Dst = 3;
											else
												Dst = 2;
										}
									}

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


//--- rx_screen_init_FMS_1x3r ----------------------------------------------------------------------
int rx_screen_init_FMS_1x3r(SScreenConfig* pscreenConfig)
{

	float totR, mediumR, largerR, totminR, totmaxR, mediumminR, mediummaxR, largerminR, largermaxR;
	int i, j, r1Input, r2Input, r3Input;

	sprintf(pscreenConfig->typeName, "FMS_1x3r");
	pscreenConfig->fctplane = &rx_screen_plane_FMS_1x3r;
	pscreenConfig->fctclose = &rx_screen_close_null;

	for (i = 0; i < MAX_COLORS; i++) {
		// check to avoid input=0
		if (pscreenConfig->ratio[i].r1.input == 0)
			pscreenConfig->ratio[i].r3 = pscreenConfig->ratio[i].r2 = pscreenConfig->ratio[i].r1 = pscreenConfig->ratio[i].r100;

		r1Input = 65536 * pscreenConfig->ratio[i].r1.input / 100;
		r2Input = 65536 * pscreenConfig->ratio[i].r2.input / 100;
		r3Input = 65536 * pscreenConfig->ratio[i].r3.input / 100;

		if (pscreenConfig->ratio[i].r1.input) { // 1st slice
			largermaxR = (float)pscreenConfig->ratio[i].r1.largeD / pscreenConfig->ratio[i].r1.input; // init 
			if (largermaxR > 1.0) largermaxR = 1.0;
			largerR = largerminR = largermaxR;

			mediummaxR = (float)(pscreenConfig->ratio[i].r1.mediumD + pscreenConfig->ratio[i].r1.largeD) / pscreenConfig->ratio[i].r1.input;
			if (mediummaxR > 1.0) mediummaxR = 1.0;
			mediumR = mediumminR = mediummaxR;

			totmaxR = (float)(pscreenConfig->ratio[i].r1.smallD + pscreenConfig->ratio[i].r1.mediumD + pscreenConfig->ratio[i].r1.largeD) / pscreenConfig->ratio[i].r1.input;
			if (totmaxR > 1.0) totmaxR = 1.0;
			totR = totminR = totmaxR;

			for (j = 0; j < r1Input; j++) { // 1st slice from 0 to r1
				pscreenConfig->lut16_L[i][j] = (UINT16)(largerR * pscreenConfig->lut16[i][j]);	//  lut for larger dot
				pscreenConfig->lut16_M[i][j] = (UINT16)(mediumR * pscreenConfig->lut16[i][j]);	//  lut for medium dot
				pscreenConfig->lut16[i][j] = (UINT16)(totR * pscreenConfig->lut16[i][j]);		// lut for any dot
			}
		}

		if (pscreenConfig->ratio[i].r2.input > pscreenConfig->ratio[i].r1.input) { // 2nd slice
			largermaxR = (float)pscreenConfig->ratio[i].r2.largeD / pscreenConfig->ratio[i].r2.input; // init 
			if (largermaxR > 1.0) largermaxR = 1.0;
			largerminR = largerR;

			mediummaxR = (float)(pscreenConfig->ratio[i].r2.mediumD + pscreenConfig->ratio[i].r2.largeD) / pscreenConfig->ratio[i].r2.input;
			if (mediummaxR > 1.0) mediummaxR = 1.0;
			mediumminR = mediumR;

			totmaxR = (float)(pscreenConfig->ratio[i].r2.smallD + pscreenConfig->ratio[i].r2.mediumD + pscreenConfig->ratio[i].r2.largeD) / pscreenConfig->ratio[i].r2.input;
			if (totmaxR > 1.0) totmaxR = 1.0;

			for (j = r1Input; j < r2Input; j++) { // 2nd slice from r1 to r2
				largerR = largerminR + ((largermaxR - largerminR) * ((float)(j - r1Input) / (r2Input - r1Input)));
				pscreenConfig->lut16_L[i][j] = (UINT16)(largerR * pscreenConfig->lut16[i][j]);	//  lut for larger dot
				mediumR = mediumminR + ((mediummaxR - mediumminR) * ((float)(j - r1Input) / (r2Input - r1Input)));
				pscreenConfig->lut16_M[i][j] = (UINT16)(mediumR * pscreenConfig->lut16[i][j]);	//  lut for medium dot
				totR = totminR + ((totmaxR - totminR) * ((float)(j - r1Input) / (r2Input - r1Input)));
				pscreenConfig->lut16[i][j] = (UINT16)(totR * pscreenConfig->lut16[i][j]);		// lut for any dot
			}
			totminR = totR;
			mediumminR = mediumR;
			largerminR = largerR;
		}
		else
			pscreenConfig->ratio[i].r2 = pscreenConfig->ratio[i].r1;

		if (pscreenConfig->ratio[i].r3.input > pscreenConfig->ratio[i].r2.input) { // 3rd slice
			largermaxR = (float)pscreenConfig->ratio[i].r3.largeD / pscreenConfig->ratio[i].r3.input; // init 
			if (largermaxR > 1.0) largermaxR = 1.0;
			largerminR = largerR;

			mediummaxR = (float)(pscreenConfig->ratio[i].r3.mediumD + pscreenConfig->ratio[i].r3.largeD) / pscreenConfig->ratio[i].r3.input;
			if (mediummaxR > 1.0) mediummaxR = 1.0;
			mediumminR = mediumR;

			totmaxR = (float)(pscreenConfig->ratio[i].r3.smallD + pscreenConfig->ratio[i].r3.mediumD + pscreenConfig->ratio[i].r3.largeD) / pscreenConfig->ratio[i].r3.input;
			if (totmaxR > 1.0) totmaxR = 1.0;
			totminR = totR;

			for (j = r2Input; j < r3Input; j++) { // 3rd slice from r2 to r3
				largerR = largerminR + ((largermaxR - largerminR) * ((float)(j - r2Input) / (r3Input - r2Input)));
				pscreenConfig->lut16_L[i][j] = (UINT16)(largerR * pscreenConfig->lut16[i][j]);	//  lut for larger dot
				mediumR = mediumminR + ((mediummaxR - mediumminR) * ((float)(j - r2Input) / (r3Input - r2Input)));
				pscreenConfig->lut16_M[i][j] = (UINT16)(mediumR * pscreenConfig->lut16[i][j]);	//  lut for medium dot
				totR = totminR + ((totmaxR - totminR) * ((float)(j - r2Input) / (r3Input - r2Input)));
				pscreenConfig->lut16[i][j] = (UINT16)(totR * pscreenConfig->lut16[i][j]);		// lut for any dot
			}
			totminR = totR;
			mediumminR = mediumR;
			largerminR = largerR;
		}
		else
			pscreenConfig->ratio[i].r3 = pscreenConfig->ratio[i].r2;

		if (100 > pscreenConfig->ratio[i].r3.input) { // last slice
			largermaxR = (float)pscreenConfig->ratio[i].r100.largeD / 100; // init
			if (largermaxR > 1.0) largermaxR = 1.0;
			largerminR = largerR;

			mediummaxR = (float)(pscreenConfig->ratio[i].r100.mediumD + pscreenConfig->ratio[i].r100.largeD) / 100;
			if (mediummaxR > 1.0) mediummaxR = 1.0;
			mediumminR = mediumR;

			totmaxR = (float)(pscreenConfig->ratio[i].r100.smallD + pscreenConfig->ratio[i].r100.mediumD + pscreenConfig->ratio[i].r100.largeD) / 100;
			if (totmaxR > 1.0) totmaxR = 1.0;
			totminR = totR;

			for (j = r3Input; j < 65536; j++) { // last slice from r3 to 65536
				largerR = largerminR + ((largermaxR - largerminR) * ((float)(j - r3Input) / (65536 - r3Input)));
				pscreenConfig->lut16_L[i][j] = (UINT16)(largerR * pscreenConfig->lut16[i][j]);	//  lut for larger dot
				mediumR = mediumminR + ((mediummaxR - mediumminR) * ((float)(j - r3Input) / (65536 - r3Input)));
				pscreenConfig->lut16_M[i][j] = (UINT16)(mediumR * pscreenConfig->lut16[i][j]);	//  lut for medium dot
				totR = totminR + ((totmaxR - totminR) * ((float)(j - r3Input) / (65536 - r3Input)));
				pscreenConfig->lut16[i][j] = (UINT16)(totR * pscreenConfig->lut16[i][j]);		// lut for any dot
			}
		}
	}

	return (REPLY_OK);
}


//--- rx_screen_plane_FMS_1x3r ----------------------------------------------------------------------
int rx_screen_plane_FMS_1x3r(SPlaneInfo *inplane, SPlaneInfo *outplane, void* epscreenConfig)
{
	SScreenConfig* pscreenConfig = epscreenConfig;
	UINT32 l;
	int i, modl, taindex;
	BYTE Dst, *pSrc, *pDst;
	UINT16 *pWSrc, WSrc;
	STaConfig *pta = pscreenConfig->TA[inplane->planenumber % 8];
	UINT16 *lut16 = pscreenConfig->lut16[inplane->planenumber];
	UINT16 *lut16_M = pscreenConfig->lut16_M[inplane->planenumber];
	UINT16 *lut16_L = pscreenConfig->lut16_L[inplane->planenumber];

	if ((pscreenConfig->inputResol.x * 2 == pscreenConfig->outputResol.x) && (pscreenConfig->inputResol.y * 2 == pscreenConfig->outputResol.y)) {
		return (rx_screen_plane_FMS_1x3rx2(inplane, outplane, pscreenConfig));
	}
	else if ((pscreenConfig->inputResol.x * 4 == pscreenConfig->outputResol.x) && (pscreenConfig->inputResol.y * 4 == pscreenConfig->outputResol.y)) {
		return (rx_screen_plane_FMS_1x3rx4(inplane, outplane, pscreenConfig));
	}

	if (rx_screen_outplane_init(inplane, outplane, pscreenConfig))
		return REPLY_ERROR;

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
							Dst = 1;
							WSrc = lut16_M[pWSrc[i]];
							if ((WSrc >= pta->ta16[taindex])) {// 1 or bigger				
								Dst = 2;
								WSrc = lut16_L[pWSrc[i]];
								if ((WSrc >= pta->ta16[taindex])) {// 2 or bigger				
									Dst = 3;
								}
							}

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
							Dst = 1;
							WSrc = lut16_M[pSrc[i] * 257];
							if ((WSrc >= pta->ta16[taindex])) {// 1 or bigger				
								Dst = 2;
								WSrc = lut16_L[pSrc[i] * 257];
								if ((WSrc >= pta->ta16[taindex])) {// 2 or bigger				
									Dst = 3;
								}
							}

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


//--- rx_screen_plane_FMS_1x3rx2----------------------------------------------------------------------
int rx_screen_plane_FMS_1x3rx2(SPlaneInfo *inplane, SPlaneInfo *outplane, void* epscreenConfig)
{
	SScreenConfig* pscreenConfig = epscreenConfig;
	UINT32 li, lo;
	int pi, po, modl, taindex;
	BYTE Dst, *pSrc, *pDst;
	UINT16 *pWSrc, WSrc;
	STaConfig *pta = pscreenConfig->TA[inplane->planenumber % 8];
	UINT16 *lut16 = pscreenConfig->lut16[inplane->planenumber];
	UINT16 *lut16_M = pscreenConfig->lut16_M[inplane->planenumber];
	UINT16 *lut16_L = pscreenConfig->lut16_L[inplane->planenumber];

	if ((pscreenConfig->inputResol.x * 4 == pscreenConfig->outputResol.x) && (pscreenConfig->inputResol.y * 4 == pscreenConfig->outputResol.y)) {
		return (rx_screen_plane_FMS_1x3rx4(inplane, outplane, pscreenConfig));
	}

	if (rx_screen_outplane_init(inplane, outplane, pscreenConfig))
		return REPLY_ERROR;

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
							if (WSrc) {

								taindex = (modl * pta->width) + (po % pta->width);
								if ((WSrc >= pta->ta16[taindex])) {	// 0 or bigger
									Dst = 1;
									WSrc = lut16_M[pWSrc[pi]];
									if ((WSrc >= pta->ta16[taindex])) {// 1 or bigger				
										Dst = 2;
										WSrc = lut16_L[pWSrc[pi]];
										if ((WSrc >= pta->ta16[taindex])) {// 2 or bigger				
											Dst = 3;
										}
									}

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
							if (WSrc) {

								taindex = (modl * pta->width) + (po % pta->width);
								if ((WSrc >= pta->ta16[taindex])) {	// 0 or bigger
									Dst = 1;
									WSrc = lut16_M[pSrc[pi] * 257];
									if ((WSrc >= pta->ta16[taindex])) {// 1 or bigger				
										Dst = 2;
										WSrc = lut16_L[pSrc[pi] * 257];
										if ((WSrc >= pta->ta16[taindex])) {// 2 or bigger				
											Dst = 3;
										}
									}

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


//--- rx_screen_plane_FMS_1x3rx4----------------------------------------------------------------------
int rx_screen_plane_FMS_1x3rx4(SPlaneInfo *inplane, SPlaneInfo *outplane, void* epscreenConfig)
{
	SScreenConfig* pscreenConfig = epscreenConfig;
	UINT32 li, lo;
	int pi, po, modl, taindex;
	BYTE Dst, *pSrc, *pDst;
	UINT16 *pWSrc, WSrc;
	STaConfig *pta = pscreenConfig->TA[inplane->planenumber % 8];
	UINT16 *lut16 = pscreenConfig->lut16[inplane->planenumber];
	UINT16 *lut16_M = pscreenConfig->lut16_M[inplane->planenumber];
	UINT16 *lut16_L = pscreenConfig->lut16_L[inplane->planenumber];

	if (rx_screen_outplane_init(inplane, outplane, pscreenConfig))
		return REPLY_ERROR;

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
								if ((WSrc >= pta->ta16[taindex])) {	// 0 or bigger
									Dst = 1;
									WSrc = lut16_M[pWSrc[pi]];
									if ((WSrc >= pta->ta16[taindex])) {// 1 or bigger				
										Dst = 2;
										WSrc = lut16_L[pWSrc[pi]];
										if ((WSrc >= pta->ta16[taindex])) {// 2 or bigger				
											Dst = 3;
										}
									}
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
							if (WSrc) {

								taindex = (modl * pta->width) + (po % pta->width);
								if ((WSrc >= pta->ta16[taindex])) {	// 0 or bigger
									Dst = 1;
									WSrc = lut16_M[pSrc[pi] * 257];
									if ((WSrc >= pta->ta16[taindex])) {// 1 or bigger				
										Dst = 2;
										WSrc = lut16_L[pSrc[pi] * 257];
										if ((WSrc >= pta->ta16[taindex])) {// 2 or bigger				
											Dst = 3;
										}
									}

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
