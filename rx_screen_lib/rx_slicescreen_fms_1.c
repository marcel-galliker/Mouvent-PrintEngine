// ****************************************************************************
//
//	rx_slicescreen_fms_1.c
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
#include "rx_slicescreen_fms_1.h"

#ifdef linux
	#include "errno.h"
#endif

static BYTE			Val4[4];



//--- rx_screen_slice_FMS_1x1 ----------------------------------------------------------------------
int rx_screen_slice_FMS_1x1(SSLiceInfo *inplane, SSLiceInfo *outplane, void * epplaneScreenConfig)
{
	SPlaneScreenConfig* pplaneScreenConfig = epplaneScreenConfig;
	UINT32 l;
	int i, modl, taindex;
	BYTE *pSrc, *pDst;
	UINT16  WSrc;
	STaConfig *pta = pplaneScreenConfig->TA;
	int ratio = 257;

	if (rx_screen_outslice_init(inplane, outplane, pplaneScreenConfig))
		return REPLY_ERROR;

	if (pplaneScreenConfig->fillmethod == 2) // ratio mode
	{
		ratio = ratio * (pplaneScreenConfig->ratio.r100.smallD + pplaneScreenConfig->ratio.r100.mediumD + pplaneScreenConfig->ratio.r100.largeD) / 100; // max 257
	}


	for (l = 0; l < inplane->lengthPx; l++)
	{
		pSrc = inplane->buffer + (l * inplane->lineLen);
		modl = l % pta->heigth;
		pDst = outplane->buffer + (l * outplane->lineLen);
		for (i = 0; i < (int)inplane->WidthPx; i++)
		{
			if (pSrc[i])
			{
				WSrc = pSrc[i] * ratio;
				if (WSrc) {
					taindex = (modl * pta->width) + ((i+ inplane->Xoffset) % pta->width);
					if (WSrc >= pta->ta16[taindex]) {
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
	return (REPLY_OK);
}


//--- rx_planescreen_init_FMS_1x1 ----------------------------------------------------------------------
int rx_planescreen_init_FMS_1x1(SPlaneScreenConfig* pplaneScreenConfig)
{
	sprintf(pplaneScreenConfig->typeName, "FMS_1x1");
	pplaneScreenConfig->fctplane = &rx_screen_slice_FMS_1x1;
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
	return (REPLY_OK);
}


//--- rx_planescreen_init_FMS_1x1r ----------------------------------------------------------------------
int rx_planescreen_init_FMS_1x1r(SPlaneScreenConfig* pplaneScreenConfig)
{
	float totR;
	int  j;

	rx_planescreen_init_FMS_1x1(pplaneScreenConfig);
	sprintf(pplaneScreenConfig->typeName, "FMS_1x1r");


	totR = (float)(pplaneScreenConfig->ratio.r100.smallD + pplaneScreenConfig->ratio.r100.mediumD + pplaneScreenConfig->ratio.r100.largeD) / 100; // max 100
	if (totR > 1.0) totR = 1.0;

	for (j = 0; j < 65536; j++) {
		pplaneScreenConfig->lut16[j] = (UINT16)(totR * pplaneScreenConfig->lut16[j]); // used smallD ratio for any drop size
	}

	return (REPLY_OK);
}
