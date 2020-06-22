// ****************************************************************************
//
//	rx_slicescreen_fms_1x3.c
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
#include "rx_slicescreen.h"
#include "rx_slicescreen_fms_1x3.h"

#ifdef linux
	#include "errno.h"
#endif


static BYTE			Val4[4];

static int	_Abort=FALSE;

//--- rx_screen_slice_FMS_1x3s ----------------------------------------------------------------------
int rx_screen_slice_FMS_1x3s(SSLiceInfo *inplane, SSLiceInfo *outplane, void * epplaneScreenConfig)
{
	SPlaneScreenConfig* pplaneScreenConfig = epplaneScreenConfig;
	UINT32 l;
	int i, modl, taindex;
	BYTE *pSrc, *pDst;
	UINT16 WSrc, WLimit1 = 21845, WLimit2 = 43690, WT1 = 21845, WT2 = 21845, WT3 = 21845;
	STaConfig *pta = pplaneScreenConfig->TA;
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
				if (WSrc) {

					taindex = (modl * pta->width) + ((i + inplane->Xoffset) % pta->width);
					if (WSrc < WLimit1) { // 0 - WLimit1 -> 0 or 1
						if ((UINT64)WSrc * 65536 / WT1 >= (UINT64)pta->ta16[taindex]) {
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
					else {						// WLimit2 - 65535 -> 2 to 3
						if ((UINT64)(WSrc - WLimit2) * 65536 / WT3 >= (UINT64)pta->ta16[taindex]) {
							switch (i % 4) {
							case 0: *pDst = 0xC0; break;
							case 1: *pDst |= 0x30; break;
							case 2: *pDst |= 0x0C; break;
							case 3: *pDst |= 0x03; break;
							}
						}
						else {
							switch (i % 4) {
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
	return (REPLY_OK);
}


//--- rx_planescreen_init_FMS_1x3s ----------------------------------------------------------------------
int rx_planescreen_init_FMS_1x3s(SPlaneScreenConfig* pplaneScreenConfig)
{

	sprintf(pplaneScreenConfig->typeName, "FMS_1x3s");
	pplaneScreenConfig->fctplane = &rx_screen_slice_FMS_1x3s;
	pplaneScreenConfig->fctclose = &rx_planescreen_close_null;

	return (REPLY_OK);
}


//--- rx_screen_slice_FMS_1x3g ----------------------------------------------------------------------
int rx_screen_slice_FMS_1x3g(SSLiceInfo *inplane, SSLiceInfo *outplane, void * epplaneScreenConfig)
{
	SPlaneScreenConfig* pplaneScreenConfig = epplaneScreenConfig;
	UINT32 l;
	int i, modl, taindex;
	BYTE Dst, *pSrc, *pDst;
	UINT16 WSrc, WLimit1 = 21845, WLimit2 = 43690, WT1 = 21845, WT2 = 21845, WT3 = 21845;
	STaConfig *pta = pplaneScreenConfig->TA;
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

//--- rx_screen_slice_FMS_1x3_abort -------------------------
int rx_screen_slice_FMS_1x3_abort(void)
{
	_Abort = TRUE;
    return REPLY_OK;
}

//--- rx_screen_slice_FMS_1x3g_mag ----------------------------------------------------------------------
int rx_screen_slice_FMS_1x3g_mag(SSLiceInfo *inplane, SSLiceInfo *outplane, void * epplaneScreenConfig)
{
	SPlaneScreenConfig* pplaneScreenConfig = epplaneScreenConfig;
	UINT32 l;
	int i;
	BYTE *pSrc, *pDst, dot;
	UINT16 WSrc, WLimit1 = 21845, WLimit2 = 43690, WT1 = 21845, WT2 = 21845, WT3 = 21845;
	STaConfig *pta = pplaneScreenConfig->TA;
	UINT16 *lut16 = pplaneScreenConfig->lut16;
	UINT16 *taPtr;
	UINT16	ta;
	int ta_width;

	_Abort = FALSE;

	if (pplaneScreenConfig->limit[0]) {
		WT1 = WLimit1 = 65536 * pplaneScreenConfig->limit[0] / 100;
		if (!WT1) WT1 = 1;
		WLimit2 = 65536 * pplaneScreenConfig->limit[1] / 100;
		WT3 = 65536 - WLimit2;
		if (!WT3) WT3 = 1;
		WT2 = 65536 - (WT1 + WT3);
		if (!WT2) WT2 = 1;
	}

	ta_width = pta->width;
    for (l = 0; l < inplane->lengthPx; l++)
	{
		if (_Abort) return REPLY_ERROR;

		pSrc = inplane->buffer + l*inplane->lineLen;
		pDst = outplane->buffer+ l*outplane->lineLen;
		taPtr   = &pta->ta16[(l % pta->heigth) * ta_width];
		for (i = 0; i < (int)inplane->widthPx; )
		{
			WSrc = ((UINT16)*pSrc++) * pplaneScreenConfig->densityFactor[i];

			ta = taPtr[i % ta_width];
			if      (WSrc > WLimit2+ta) dot = 0x03;
			else if (WSrc > WLimit1+ta) dot = 0x02;
			else if (WSrc > ta)			dot = 0x01;
			else                        dot = 0x00;
			switch(i++&3)
			{
			case 0: *pDst    = dot << 6; break;
			case 1: *pDst   |= dot << 4; break;
			case 2: *pDst   |= dot << 2; break;
			case 3: *pDst++ |= dot;      break;
			}
		}
	}
	return (REPLY_OK);
}

//--- rx_planescreen_init_FMS_1x3g ----------------------------------------------------------------------
int rx_planescreen_init_FMS_1x3g(SPlaneScreenConfig* pplaneScreenConfig)
{

	sprintf(pplaneScreenConfig->typeName, "FMS_1x3g");
	pplaneScreenConfig->fctplane = &rx_screen_slice_FMS_1x3g;
	pplaneScreenConfig->fctclose = &rx_planescreen_close_null;

	return (REPLY_OK);
}


//--- rx_screen_slice_FMS_1x3r ----------------------------------------------------------------------
int rx_screen_slice_FMS_1x3r(SSLiceInfo *inplane, SSLiceInfo *outplane, void * epplaneScreenConfig)
{
	SPlaneScreenConfig* pplaneScreenConfig = epplaneScreenConfig;
	UINT32 l;
	int i, modl, taindex;
	BYTE Dst, *pSrc, *pDst;
	UINT16 WSrc;
	STaConfig *pta = pplaneScreenConfig->TA;
	UINT16 *lut16 = pplaneScreenConfig->lut16;
	UINT16 *lut16_M = pplaneScreenConfig->lut16_M;
	UINT16 *lut16_L = pplaneScreenConfig->lut16_L;

	if (rx_screen_outslice_init(inplane, outplane, pplaneScreenConfig))
		return REPLY_ERROR;

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
						Dst = 1;
						WSrc = lut16_M[pSrc[i] * 257];
						if ((WSrc >= pta->ta16[taindex])) {// 1 or bigger				
							Dst = 2;
							WSrc = lut16_L[pSrc[i] * 257];
							if ((WSrc >= pta->ta16[taindex])) {// 2 or bigger				
								Dst = 3;
							}
						}

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



//--- rx_planescreen_init_FMS_1x3r ----------------------------------------------------------------------
int rx_planescreen_init_FMS_1x3r(SPlaneScreenConfig* pplaneScreenConfig)
{
	float totR, mediumR, largerR, totminR, totmaxR, mediumminR, mediummaxR, largerminR, largermaxR;
	int j, r1Input, r2Input, r3Input;

	sprintf(pplaneScreenConfig->typeName, "FMS_1x3r");
	pplaneScreenConfig->fctplane = &rx_screen_slice_FMS_1x3r;
	pplaneScreenConfig->fctclose = &rx_planescreen_close_null;

	// check to avoid input=0
	if (pplaneScreenConfig->ratio.r1.input == 0)
		pplaneScreenConfig->ratio.r3 = pplaneScreenConfig->ratio.r2 = pplaneScreenConfig->ratio.r1 = pplaneScreenConfig->ratio.r100;

	r1Input = 65536 * pplaneScreenConfig->ratio.r1.input / 100;
	r2Input = 65536 * pplaneScreenConfig->ratio.r2.input / 100;
	r3Input = 65536 * pplaneScreenConfig->ratio.r3.input / 100;

	if (pplaneScreenConfig->ratio.r1.input) { // 1st slice
		largermaxR = (float)pplaneScreenConfig->ratio.r1.largeD / pplaneScreenConfig->ratio.r1.input; // init 
		if (largermaxR > 1.0) largermaxR = 1.0;
		largerR = largerminR = largermaxR;

		mediummaxR = (float)(pplaneScreenConfig->ratio.r1.mediumD + pplaneScreenConfig->ratio.r1.largeD) / pplaneScreenConfig->ratio.r1.input;
		if (mediummaxR > 1.0) mediummaxR = 1.0;
		mediumR = mediumminR = mediummaxR;

		totmaxR = (float)(pplaneScreenConfig->ratio.r1.smallD + pplaneScreenConfig->ratio.r1.mediumD + pplaneScreenConfig->ratio.r1.largeD) / pplaneScreenConfig->ratio.r1.input;
		if (totmaxR > 1.0) totmaxR = 1.0;
		totR = totminR = totmaxR;

		for (j = 0; j < r1Input; j++) { // 1st slice from 0 to r1
			pplaneScreenConfig->lut16_L[j] = (UINT16)(largerR * pplaneScreenConfig->lut16[j]);	//  lut for larger dot
			pplaneScreenConfig->lut16_M[j] = (UINT16)(mediumR * pplaneScreenConfig->lut16[j]);	//  lut for medium dot
			pplaneScreenConfig->lut16[j] = (UINT16)(totR * pplaneScreenConfig->lut16[j]);		// lut for any dot
		}
	}

	if (pplaneScreenConfig->ratio.r2.input > pplaneScreenConfig->ratio.r1.input) { // 2nd slice
		largermaxR = (float)pplaneScreenConfig->ratio.r2.largeD / pplaneScreenConfig->ratio.r2.input; // init 
		if (largermaxR > 1.0) largermaxR = 1.0;
		largerminR = largerR;

		mediummaxR = (float)(pplaneScreenConfig->ratio.r2.mediumD + pplaneScreenConfig->ratio.r2.largeD) / pplaneScreenConfig->ratio.r2.input;
		if (mediummaxR > 1.0) mediummaxR = 1.0;
		mediumminR = mediumR;

		totmaxR = (float)(pplaneScreenConfig->ratio.r2.smallD + pplaneScreenConfig->ratio.r2.mediumD + pplaneScreenConfig->ratio.r2.largeD) / pplaneScreenConfig->ratio.r2.input;
		if (totmaxR > 1.0) totmaxR = 1.0;

		for (j = r1Input; j < r2Input; j++) { // 2nd slice from r1 to r2
			largerR = largerminR + ((largermaxR - largerminR) * ((float)(j - r1Input) / (r2Input - r1Input)));
			pplaneScreenConfig->lut16_L[j] = (UINT16)(largerR * pplaneScreenConfig->lut16[j]);	//  lut for larger dot
			mediumR = mediumminR + ((mediummaxR - mediumminR) * ((float)(j - r1Input) / (r2Input - r1Input)));
			pplaneScreenConfig->lut16_M[j] = (UINT16)(mediumR * pplaneScreenConfig->lut16[j]);	//  lut for medium dot
			totR = totminR + ((totmaxR - totminR) * ((float)(j - r1Input) / (r2Input - r1Input)));
			pplaneScreenConfig->lut16[j] = (UINT16)(totR * pplaneScreenConfig->lut16[j]);		// lut for any dot
		}
		totminR = totR;
		mediumminR = mediumR;
		largerminR = largerR;
	}
	else
		pplaneScreenConfig->ratio.r2 = pplaneScreenConfig->ratio.r1;

	if (pplaneScreenConfig->ratio.r3.input > pplaneScreenConfig->ratio.r2.input) { // 3rd slice
		largermaxR = (float)pplaneScreenConfig->ratio.r3.largeD / pplaneScreenConfig->ratio.r3.input; // init 
		if (largermaxR > 1.0) largermaxR = 1.0;
		largerminR = largerR;

		mediummaxR = (float)(pplaneScreenConfig->ratio.r3.mediumD + pplaneScreenConfig->ratio.r3.largeD) / pplaneScreenConfig->ratio.r3.input;
		if (mediummaxR > 1.0) mediummaxR = 1.0;
		mediumminR = mediumR;

		totmaxR = (float)(pplaneScreenConfig->ratio.r3.smallD + pplaneScreenConfig->ratio.r3.mediumD + pplaneScreenConfig->ratio.r3.largeD) / pplaneScreenConfig->ratio.r3.input;
		if (totmaxR > 1.0) totmaxR = 1.0;
		totminR = totR;

		for (j = r2Input; j < r3Input; j++) { // 3rd slice from r2 to r3
			largerR = largerminR + ((largermaxR - largerminR) * ((float)(j - r2Input) / (r3Input - r2Input)));
			pplaneScreenConfig->lut16_L[j] = (UINT16)(largerR * pplaneScreenConfig->lut16[j]);	//  lut for larger dot
			mediumR = mediumminR + ((mediummaxR - mediumminR) * ((float)(j - r2Input) / (r3Input - r2Input)));
			pplaneScreenConfig->lut16_M[j] = (UINT16)(mediumR * pplaneScreenConfig->lut16[j]);	//  lut for medium dot
			totR = totminR + ((totmaxR - totminR) * ((float)(j - r2Input) / (r3Input - r2Input)));
			pplaneScreenConfig->lut16[j] = (UINT16)(totR * pplaneScreenConfig->lut16[j]);		// lut for any dot
		}
		totminR = totR;
		mediumminR = mediumR;
		largerminR = largerR;
	}
	else
		pplaneScreenConfig->ratio.r3 = pplaneScreenConfig->ratio.r2;

	if (100 > pplaneScreenConfig->ratio.r3.input) { // last slice
		largermaxR = (float)pplaneScreenConfig->ratio.r100.largeD / 100; // init
		if (largermaxR > 1.0) largermaxR = 1.0;
		largerminR = largerR;

		mediummaxR = (float)(pplaneScreenConfig->ratio.r100.mediumD + pplaneScreenConfig->ratio.r100.largeD) / 100;
		if (mediummaxR > 1.0) mediummaxR = 1.0;
		mediumminR = mediumR;

		totmaxR = (float)(pplaneScreenConfig->ratio.r100.smallD + pplaneScreenConfig->ratio.r100.mediumD + pplaneScreenConfig->ratio.r100.largeD) / 100;
		if (totmaxR > 1.0) totmaxR = 1.0;
		totminR = totR;

		for (j = r3Input; j < 65536; j++) { // last slice from r3 to 65536
			largerR = largerminR + ((largermaxR - largerminR) * ((float)(j - r3Input) / (65536 - r3Input)));
			pplaneScreenConfig->lut16_L[j] = (UINT16)(largerR * pplaneScreenConfig->lut16[j]);	//  lut for larger dot
			mediumR = mediumminR + ((mediummaxR - mediumminR) * ((float)(j - r3Input) / (65536 - r3Input)));
			pplaneScreenConfig->lut16_M[j] = (UINT16)(mediumR * pplaneScreenConfig->lut16[j]);	//  lut for medium dot
			totR = totminR + ((totmaxR - totminR) * ((float)(j - r3Input) / (65536 - r3Input)));
			pplaneScreenConfig->lut16[j] = (UINT16)(totR * pplaneScreenConfig->lut16[j]);		// lut for any dot
		}
	}

	return (REPLY_OK);
}
