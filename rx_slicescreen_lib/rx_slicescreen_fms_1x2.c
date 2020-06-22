// ****************************************************************************
//
//	rx_slicescreen_fms_1x2.c
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
#include "rx_slicescreen_fms_1x2.h"

#ifdef linux
	#include "errno.h"
#endif

static BYTE			Val4[4];



//--- rx_screen_slice_FMS_1x2g ----------------------------------------------------------------------
int rx_screen_slice_FMS_1x2g(SSLiceInfo *inplane, SSLiceInfo *outplane, void * epplaneScreenConfig)
{
	SPlaneScreenConfig* pplaneScreenConfig = epplaneScreenConfig;
	UINT32 l;
	int i, modl, taindex;
	BYTE Dst, *pSrc, *pDst, Val1 = 1, Val2 = 2;
	UINT16 WSrc, WLimit = 32768, WT2 = 32768;
	STaConfig *pta = pplaneScreenConfig->TA;
	UINT16 *lut16 = pplaneScreenConfig->lut16;

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

	for (l = 0; l < inplane->lengthPx; l++)
	{
		pSrc = inplane->buffer + (l * inplane->lineLen);
		modl = l % pta->heigth;
		pDst = outplane->buffer + (l * outplane->lineLen);
		for (i = 0; i < (int)inplane->widthPx; i++)
		{
			if (pSrc[i])
			{
				WSrc = lut16[pSrc[i] * 257];
				if (WSrc)
				{
					taindex = (modl * pta->width) + ((i + inplane->Xoffset) % pta->width);
					if ((WSrc >= pta->ta16[taindex])) {	// 1-limit -> 0 or 1
						Dst = Val1;
						if (WSrc > WLimit)				// limit-255 -> 0, 1 or 2				
							if ((UINT64)(WSrc + 1 - WLimit) * 65536 / WT2 >= (UINT64)pta->ta16[taindex])
								Dst = Val2;

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
	return (REPLY_OK);
}


//--- rx_planescreen_init_FMS_1x2g ----------------------------------------------------------------------
int rx_planescreen_init_FMS_1x2g(SPlaneScreenConfig* pplaneScreenConfig)
{


	sprintf(pplaneScreenConfig->typeName, "FMS_1x2g");
	pplaneScreenConfig->fctplane = &rx_screen_slice_FMS_1x2g;
	pplaneScreenConfig->fctclose = &rx_planescreen_close_null;

	return (REPLY_OK);
}


//--- rx_screen_slice_FMS_1x2s ----------------------------------------------------------------------
int rx_screen_slice_FMS_1x2s(SSLiceInfo *inplane, SSLiceInfo *outplane, void * epplaneScreenConfig)
{
	SPlaneScreenConfig* pplaneScreenConfig = epplaneScreenConfig;
	UINT32 l;
	int i, modl, taindex;
	BYTE *pSrc, *pDst;
	UINT16 WSrc, WLimit = 32768, WT1 = 32768, WT2 = 32768;
	STaConfig *pta = pplaneScreenConfig->TA;


	if (rx_screen_outslice_init(inplane, outplane, pplaneScreenConfig))
		return REPLY_ERROR;

	if (pplaneScreenConfig->limit[0]) {
		WT1 = WLimit = 65536 * pplaneScreenConfig->limit[0] / 100;
		if (!WT1) WT1 = 1;
		WT2 = 65536 - WT1;
		if (!WT2) WT2 = 1;
	}

	for (l = 0; l < inplane->lengthPx; l++)
	{
		pSrc = inplane->buffer + (l * inplane->lineLen);
		modl = l % pta->heigth;
		pDst = outplane->buffer + (l * outplane->lineLen);
		for (i = 0; i < (int)inplane->widthPx; i++)
		{
			if (pSrc[i])
			{
				WSrc = pSrc[i] * 257;
				if (WSrc)
				{
					taindex = (modl * pta->width) + ((i + inplane->Xoffset) % pta->width);
					if (WSrc < WLimit) { // 0 - WLimit -> 0 or 1
						if ((UINT64)WSrc * 65536 / WT1 >= (UINT64)pta->ta16[taindex]) {
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
							switch (i % 4) {
							case 0: *pDst = 0x80; break;
							case 1: *pDst |= 0x20; break;
							case 2: *pDst |= 0x08; break;
							case 3: *pDst |= 0x02; break;
							}
						}
						else {
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
	return (REPLY_OK);
}


//--- rx_planescreen_init_FMS_1x2s ----------------------------------------------------------------------
int rx_planescreen_init_FMS_1x2s(SPlaneScreenConfig* pplaneScreenConfig)
{

	sprintf(pplaneScreenConfig->typeName, "FMS_1x2s");
	pplaneScreenConfig->fctplane = &rx_screen_slice_FMS_1x2s;
	pplaneScreenConfig->fctclose = &rx_planescreen_close_null;

	return (REPLY_OK);
}


//--- rx_screen_slice_FMS_1x2r ----------------------------------------------------------------------
int rx_screen_slice_FMS_1x2r(SSLiceInfo *inplane, SSLiceInfo *outplane, void * epplaneScreenConfig)
{
	SPlaneScreenConfig* pplaneScreenConfig = epplaneScreenConfig;
	UINT32 l;
	int i, modl, taindex;
	BYTE Dst, *pSrc, *pDst, Val1 = 1, Val2 = 2;
	UINT16 WSrc;
	STaConfig *pta = pplaneScreenConfig->TA;
	UINT16 *lut16 = pplaneScreenConfig->lut16;
	UINT16 *lut16_L = pplaneScreenConfig->lut16_L;

	if (rx_screen_outslice_init(inplane, outplane, pplaneScreenConfig))
		return REPLY_ERROR;

	if (pplaneScreenConfig->dropsize == 13)
		Val2 = 3;
	else if (pplaneScreenConfig->dropsize == 23) {
		Val1 = 2;
		Val2 = 3;
	}

	for (l = 0; l < inplane->lengthPx; l++)
	{
		pSrc = inplane->buffer + (l * inplane->lineLen);
		modl = l % pta->heigth;
		pDst = outplane->buffer + (l * outplane->lineLen);
		for (i = 0; i < (int)inplane->widthPx; i++)
		{
			if (pSrc[i])
			{
				WSrc = lut16[pSrc[i] * 257];
				if (WSrc)
				{
					taindex = (modl * pta->width) + ((i + inplane->Xoffset) % pta->width);
					if ((WSrc >= pta->ta16[taindex])) {	// 0 or bigger
						Dst = Val1;
						WSrc = lut16_L[pSrc[i] * 257];
						if ((WSrc >= pta->ta16[taindex])) // 1 or bigger				
							Dst = Val2;

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

	return (REPLY_OK);
}


//--- rx_planescreen_init_FMS_1x2r ----------------------------------------------------------------------
int rx_planescreen_init_FMS_1x2r(SPlaneScreenConfig* pplaneScreenConfig)
{
	float totR, largerR, totminR, totmaxR, largerminR, largermaxR;
	int j, r1Input, r2Input, r3Input;

	sprintf(pplaneScreenConfig->typeName, "FMS_1x2r");
	pplaneScreenConfig->fctplane = &rx_screen_slice_FMS_1x2r;
	pplaneScreenConfig->fctclose = &rx_planescreen_close_null;


	// check to avoid input=0
	if (pplaneScreenConfig->ratio.r1.input == 0)
		pplaneScreenConfig->ratio.r3 = pplaneScreenConfig->ratio.r2 = pplaneScreenConfig->ratio.r1 = pplaneScreenConfig->ratio.r100;

	r1Input = 65536 * pplaneScreenConfig->ratio.r1.input / 100;
	r2Input = 65536 * pplaneScreenConfig->ratio.r2.input / 100;
	r3Input = 65536 * pplaneScreenConfig->ratio.r3.input / 100;

	if (pplaneScreenConfig->ratio.r1.input) { // 1st slice
		if (pplaneScreenConfig->dropsize == 12) // init 
			largermaxR = (float)pplaneScreenConfig->ratio.r1.mediumD / pplaneScreenConfig->ratio.r1.input;
		else
			largermaxR = (float)pplaneScreenConfig->ratio.r1.largeD / pplaneScreenConfig->ratio.r1.input;
		if (largermaxR > 1.0) largermaxR = 1.0;
		largerR = largerminR = largermaxR;

		totmaxR = (float)(pplaneScreenConfig->ratio.r1.smallD + pplaneScreenConfig->ratio.r1.mediumD + pplaneScreenConfig->ratio.r1.largeD) / pplaneScreenConfig->ratio.r1.input;
		if (totmaxR > 1.0) totmaxR = 1.0;
		totR = totminR = totmaxR;

		for (j = 0; j < r1Input; j++) { // 1st slice from 0 to r1
			pplaneScreenConfig->lut16_L[j] = (UINT16)(largerR * pplaneScreenConfig->lut16[j]);	//  lut for larger dot
			pplaneScreenConfig->lut16[j] = (UINT16)(totR * pplaneScreenConfig->lut16[j]);		// lut for any dot
		}
	}

	if (pplaneScreenConfig->ratio.r2.input > pplaneScreenConfig->ratio.r1.input) { // 2nd slice
		if (pplaneScreenConfig->dropsize == 12) // init 
			largermaxR = (float)pplaneScreenConfig->ratio.r2.mediumD / pplaneScreenConfig->ratio.r2.input;
		else
			largermaxR = (float)pplaneScreenConfig->ratio.r2.largeD / pplaneScreenConfig->ratio.r2.input;
		if (largermaxR > 1.0) largermaxR = 1.0;

		totmaxR = (float)(pplaneScreenConfig->ratio.r2.smallD + pplaneScreenConfig->ratio.r2.mediumD + pplaneScreenConfig->ratio.r2.largeD)
			/ pplaneScreenConfig->ratio.r2.input;

		if (totmaxR > 1.0) totmaxR = 1.0;

		for (j = r1Input; j < r2Input; j++) { // 2nd slice from r1 to r2
			largerR = largerminR + ((largermaxR - largerminR)*((float)(j - r1Input) / (float)(r2Input - r1Input)));
			pplaneScreenConfig->lut16_L[j] = (UINT16)(largerR * pplaneScreenConfig->lut16[j]);	//  lut for larger dot
			totR = totminR + ((totmaxR - totminR) * ((float)(j - r1Input) / (float)(r2Input - r1Input)));
			pplaneScreenConfig->lut16[j] = (UINT16)(totR * pplaneScreenConfig->lut16[j]);		// lut for any dot
		}
		totminR = totR;
		largerminR = largerR;
	}
	else
		pplaneScreenConfig->ratio.r2 = pplaneScreenConfig->ratio.r1;

	if (pplaneScreenConfig->ratio.r3.input > pplaneScreenConfig->ratio.r2.input) { // 3rd slice
		if (pplaneScreenConfig->dropsize == 12) // init
			largermaxR = (float)pplaneScreenConfig->ratio.r3.mediumD / pplaneScreenConfig->ratio.r3.input;
		else
			largermaxR = (float)pplaneScreenConfig->ratio.r3.largeD / pplaneScreenConfig->ratio.r3.input;
		if (largermaxR > 1.0) largermaxR = 1.0;

		totmaxR = (float)(pplaneScreenConfig->ratio.r3.smallD + pplaneScreenConfig->ratio.r3.mediumD + pplaneScreenConfig->ratio.r3.largeD)
			/ pplaneScreenConfig->ratio.r3.input;

		if (totmaxR > 1.0) totmaxR = 1.0;

		for (j = r2Input; j < r3Input; j++) { // 2nd slice from r1 to r2
			largerR = largerminR + ((largermaxR - largerminR)*((float)(j - r2Input) / (float)(r3Input - r2Input)));
			pplaneScreenConfig->lut16_L[j] = (UINT16)(largerR * pplaneScreenConfig->lut16[j]);	//  lut for larger dot
			totR = totminR + ((totmaxR - totminR) * ((float)(j - r2Input) / (float)(r3Input - r2Input)));
			pplaneScreenConfig->lut16[j] = (UINT16)(totR * pplaneScreenConfig->lut16[j]);		// lut for any dot
		}
		totminR = totR;
		largerminR = largerR;
	}
	else
		pplaneScreenConfig->ratio.r3 = pplaneScreenConfig->ratio.r2;


	if (100 > pplaneScreenConfig->ratio.r3.input) { // last slice
		if (pplaneScreenConfig->dropsize == 12) // init 
			largermaxR = (float)pplaneScreenConfig->ratio.r100.mediumD / 100;
		else
			largermaxR = (float)pplaneScreenConfig->ratio.r100.largeD / 100;
		if (largermaxR > 1.0) largermaxR = 1.0;

		totmaxR = (float)(pplaneScreenConfig->ratio.r100.smallD + pplaneScreenConfig->ratio.r100.mediumD + pplaneScreenConfig->ratio.r100.largeD)
			/ 100;

		if (totmaxR > 1.0) totmaxR = 1.0;

		for (j = r3Input; j < 65536; j++) { // 2nd slice from r1 to r2
			largerR = largerminR + ((largermaxR - largerminR)*((float)(j - r3Input) / (float)(65536 - r3Input)));
			pplaneScreenConfig->lut16_L[j] = (UINT16)(largerR * pplaneScreenConfig->lut16[j]);	//  lut for larger dot
			totR = totminR + ((totmaxR - totminR) * ((float)(j - r3Input) / (float)(65536 - r3Input)));
			pplaneScreenConfig->lut16[j] = (UINT16)(totR * pplaneScreenConfig->lut16[j]);		// lut for any dot
		}
	}

	return (REPLY_OK);
}
