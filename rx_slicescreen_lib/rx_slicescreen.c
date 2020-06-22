// ****************************************************************************
//
//	rx_slicescreen.c
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
#include "rx_file.h"
#include "rx_setup_file.h"
#include "rx_slicescreen.h"

#include "rx_slicescreen_fme_4x3.h"
#include "rx_slicescreen_fms_1.h"
#include "rx_slicescreen_fms_1x2.h"
#include "rx_slicescreen_fms_1x3.h"

#ifdef linux
	#include "errno.h"
#endif


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

int rx_planescreen_load(int planeNumber, const char * settingsPath, const char * ressourcesPath, SPlaneScreenConfig* pplaneScreenConfig)
{
	int lplanenumber, rationumber, first, second;
	char Name[32];
	HANDLE  file;

	pplaneScreenConfig->planeNumber = planeNumber;
	pplaneScreenConfig->ressourcePath = (char *)ressourcesPath;
	
	file = setup_create();
	setup_load(file, settingsPath);
	if (setup_chapter(file, "PrinterEnvSet", -1, READ) != REPLY_OK)
		return Error(ERR_CONT, 0, "Cannot find PrinterEnvSet item in file %s", settingsPath);

	if (setup_chapter(file, "Screen", -1, READ) != REPLY_OK)
		return Error(ERR_CONT, 0, "Cannot find Screen item in file %s", settingsPath);
	setup_uint32(file, "Type", READ, (unsigned int *)&pplaneScreenConfig->type, pplaneScreenConfig->type);
	switch (pplaneScreenConfig->type)
	{
	case 0://FME_1x3x2
		pplaneScreenConfig->method = 0;			/* error diffusion */
		pplaneScreenConfig->dropsize = 123;		/* 123 */
		pplaneScreenConfig->fillmethod = 1;		/* fill and grow */
		pplaneScreenConfig->outputbitsPerPixel = 2;
		break;

	default:
	case 2://FMS_1x1
		pplaneScreenConfig->method = 2;			/* Threshold */
		pplaneScreenConfig->dropsize = 1;		/* 1 */
		pplaneScreenConfig->fillmethod = 1;		/* fill and grow */
		pplaneScreenConfig->outputbitsPerPixel = 2;
		break;
	case 3:// FMS_1x3s
		pplaneScreenConfig->method = 2;			/* Threshold */
		pplaneScreenConfig->dropsize = 123;		/* 123 */
		pplaneScreenConfig->fillmethod = 0;		/* fill then grow */
		pplaneScreenConfig->outputbitsPerPixel = 2;
		break;

	case 1://FMB_4x3
		// not supported
	case 4:// FMS_1x3g
		pplaneScreenConfig->method = 2;			/* Threshold */
		pplaneScreenConfig->dropsize = 123;		/* 123 */
		pplaneScreenConfig->fillmethod = 1;		/* fill and grow */
		pplaneScreenConfig->outputbitsPerPixel = 2;
		break;
	case 5:// FMS_1x3sx2
		pplaneScreenConfig->method = 2;			/* Threshold */
		pplaneScreenConfig->dropsize = 123;		/* 123 */
		pplaneScreenConfig->fillmethod = 0;		/* fill then grow */
		pplaneScreenConfig->outputbitsPerPixel = 2;
		break;
	case 6:// FMS_1x3gx2
		pplaneScreenConfig->method = 2;			/* Threshold */
		pplaneScreenConfig->dropsize = 123;		/* 123 */
		pplaneScreenConfig->fillmethod = 1;		/* fill and grow */
		pplaneScreenConfig->outputbitsPerPixel = 2;
		break;
	case 7:// FMS_1x2s
		pplaneScreenConfig->method = 2;			/* Threshold */
		pplaneScreenConfig->dropsize = 12;		/* 12 */
		pplaneScreenConfig->fillmethod = 0;		/* fill then grow */
		pplaneScreenConfig->outputbitsPerPixel = 2;
		break;
	case 8:// FMS_1x2g
		pplaneScreenConfig->method = 2;			/* Threshold */
		pplaneScreenConfig->dropsize = 12;		/* 12 */
		pplaneScreenConfig->fillmethod = 0;		/* fill then grow */
		pplaneScreenConfig->outputbitsPerPixel = 2;
		break;
	case 9:// FMS_1x2gx2
		pplaneScreenConfig->method = 2;			/* Threshold */
		pplaneScreenConfig->dropsize = 12;		/* 12 */
		pplaneScreenConfig->fillmethod = 0;		/* fill then grow */
		pplaneScreenConfig->outputbitsPerPixel = 2;
		break;
	case 10:// FMS_1x2gx2
		pplaneScreenConfig->method = 2;			/* Threshold */
		pplaneScreenConfig->dropsize = 12;		/* 12 */
		pplaneScreenConfig->fillmethod = 1;		/* fill and grow */
		pplaneScreenConfig->outputbitsPerPixel = 2;
		break;
	case 11:// FMS_1x1sx2
		pplaneScreenConfig->method = 2;			/* Threshold */
		pplaneScreenConfig->dropsize = 1;		/* 1 */
		pplaneScreenConfig->fillmethod = 0;		/* fill then grow */
		pplaneScreenConfig->outputbitsPerPixel = 2;
		break;
	case 12:///FMS_1x1gx2
		pplaneScreenConfig->method = 2;			/* Threshold */
		pplaneScreenConfig->dropsize = 1;		/* 1 */
		pplaneScreenConfig->fillmethod = 0;		/* fill and grow */
		pplaneScreenConfig->outputbitsPerPixel = 2;
		break;
	}
	setup_uint32(file, "Method", READ, (unsigned int *)&pplaneScreenConfig->method, pplaneScreenConfig->method);
	if ((pplaneScreenConfig->method != 0) && (pplaneScreenConfig->method != 1) && (pplaneScreenConfig->method != 2) && (pplaneScreenConfig->method != 3))
		pplaneScreenConfig->method = 2;
	setup_uint32(file, "DropSize", READ, (unsigned int *)&pplaneScreenConfig->dropsize, pplaneScreenConfig->dropsize);
	if ((pplaneScreenConfig->dropsize != 1) && (pplaneScreenConfig->dropsize != 2) && (pplaneScreenConfig->dropsize != 3) && (pplaneScreenConfig->dropsize != 12) && (pplaneScreenConfig->dropsize != 13) && (pplaneScreenConfig->dropsize != 23) && (pplaneScreenConfig->dropsize != 123))
		pplaneScreenConfig->dropsize = 1;
	setup_uint32(file, "FillMethod", READ, (unsigned int *)&pplaneScreenConfig->fillmethod, 1);
	if ((pplaneScreenConfig->fillmethod < 0) && (pplaneScreenConfig->fillmethod > 2))
		pplaneScreenConfig->fillmethod = 1;
	setup_uint32(file, "Noise", READ, (unsigned int *)&pplaneScreenConfig->noise, 0);
	if ((pplaneScreenConfig->noise < 0) || (pplaneScreenConfig->noise > 100))
		pplaneScreenConfig->noise = 0;

	if (pplaneScreenConfig->dropsize == 123) {
		pplaneScreenConfig->limit[0] = 33;
		pplaneScreenConfig->limit[1] = 67;
	}
	else {
		pplaneScreenConfig->limit[0] = 50; //will be used if 12
		pplaneScreenConfig->limit[1] = 50; // not used
	}



	setup_chapter(file, "..", -1, READ);
	if (setup_chapter(file, "ScreenLimitSet", -1, READ) != REPLY_OK) goto Next1;
	if (setup_chapter(file, "ScreenLimit", -1, READ) != REPLY_OK) {
		setup_chapter(file, "..", -1, READ);
		goto Next1;
	}
	while (1) {
		setup_int32(file, "PlaneNumber", READ, &lplanenumber, -1);
		setup_uint32(file, "First", READ, &first, pplaneScreenConfig->limit[0]);
		if (first < 1) first = 1;
		if (first > 98) first = 98;
		setup_uint32(file, "Second", READ, &second, pplaneScreenConfig->limit[1]);
		if (second < first + 1) second = first + 1;
		if (second > 99) second = 99;
		if ((lplanenumber < 0) || (lplanenumber== pplaneScreenConfig->planeNumber)) {
			pplaneScreenConfig->limit[0] = first;
			pplaneScreenConfig->limit[1] = second;
		}

		setup_chapter_next(file, READ, Name, sizeof(Name));
		if (strcmp(Name, "ScreenLimit")) {
			setup_chapter(file, "..", -1, READ);
			break;
		}
	}

	setup_chapter(file, "..", -1, READ);


Next1: // Init ratios

	if (pplaneScreenConfig->dropsize < 4) { // 1 drop size config only r100
		pplaneScreenConfig->ratio.r100.input = 100;
		if (pplaneScreenConfig->dropsize == 1)
			pplaneScreenConfig->ratio.r100.smallD = 100;
		else  if (pplaneScreenConfig->dropsize == 2)
			pplaneScreenConfig->ratio.r100.mediumD = 100;
		else // 3
			pplaneScreenConfig->ratio.r100.largeD = 100;
		pplaneScreenConfig->ratio.r3 = pplaneScreenConfig->ratio.r2 = pplaneScreenConfig->ratio.r1 = pplaneScreenConfig->ratio.r100;
	}

	else if (pplaneScreenConfig->dropsize < 24) { // 2 drop sizes: config R100 then R1 and R2
		pplaneScreenConfig->ratio.r100.input = 100;
		if (pplaneScreenConfig->dropsize == 12)
			pplaneScreenConfig->ratio.r100.mediumD = 100;
		else // 13 or 23
			pplaneScreenConfig->ratio.r100.largeD = 100;

		pplaneScreenConfig->ratio.r1.input = 25; // 1st slice 25% 
		if (pplaneScreenConfig->dropsize == 23)
			pplaneScreenConfig->ratio.r1.mediumD = 25;
		else // 12 or 13
			pplaneScreenConfig->ratio.r1.smallD = 25;

		pplaneScreenConfig->ratio.r2.input = 75; // 2nd slice 25% to 75% 
		if (pplaneScreenConfig->dropsize == 23)
			pplaneScreenConfig->ratio.r2.mediumD = 25;
		else // 12 or 13
			pplaneScreenConfig->ratio.r2.smallD = 25;
		if (pplaneScreenConfig->dropsize == 12)
			pplaneScreenConfig->ratio.r2.mediumD = 50;
		else // 13 or 23
			pplaneScreenConfig->ratio.r2.largeD = 50;
		pplaneScreenConfig->ratio.r3 = pplaneScreenConfig->ratio.r2; // r3 no effect
	}
	else { // 3 drop sizes: config R100 then R1, R2 and R3

		pplaneScreenConfig->ratio.r1.input = 25; // 1st slice 25% 
		pplaneScreenConfig->ratio.r1.smallD = 25;

		pplaneScreenConfig->ratio.r2.input = 50; // 2nd slice 25% to 50% 
		pplaneScreenConfig->ratio.r2.smallD = 25;
		pplaneScreenConfig->ratio.r2.mediumD = 25;

		pplaneScreenConfig->ratio.r2.input = 75; // 3rd slice 50% to 75% 
		pplaneScreenConfig->ratio.r2.smallD = 25;
		pplaneScreenConfig->ratio.r2.mediumD = 25;
		pplaneScreenConfig->ratio.r100.largeD = 25;

		pplaneScreenConfig->ratio.r100.input = 100; // last slice 75% to 100%
		pplaneScreenConfig->ratio.r100.largeD = 75;
		pplaneScreenConfig->ratio.r100.mediumD = 25;
	}

	// Fill ratio
	if (setup_chapter(file, "RatioSet", -1, READ) != REPLY_OK) goto End;
	if (setup_chapter(file, "RatioForPlane", -1, READ) != REPLY_OK) {
		setup_chapter(file, "..", -1, READ);
		goto End;
	}

	while (1) {
		setup_int32(file, "PlaneNumber", READ, &lplanenumber, -1);
		setup_int32(file, "RatioNumber", READ, &rationumber, 100);
		if ((lplanenumber < 0) || (lplanenumber == pplaneScreenConfig->planeNumber )) {
			if (rationumber == 100) {
				_RatioFill(file, &pplaneScreenConfig->ratio.r100);
			}
			else if (rationumber == 1) {
				_RatioFill(file, &pplaneScreenConfig->ratio.r1);
			}
			else if (rationumber == 2) {
				_RatioFill(file, &pplaneScreenConfig->ratio.r2);
			}
			else if (rationumber == 3) {
				_RatioFill(file, &pplaneScreenConfig->ratio.r3);
			}
		}


		setup_chapter_next(file, READ, Name, sizeof(Name));
		if (strcmp(Name, "RatioForPlane")) {
			setup_chapter(file, "..", -1, READ);
			break;
		}
	}

	if (pplaneScreenConfig->dropsize < 4) {
		pplaneScreenConfig->ratio.r3 = pplaneScreenConfig->ratio.r2 = pplaneScreenConfig->ratio.r1 = pplaneScreenConfig->ratio.r100;
	}
	else {
		if (pplaneScreenConfig->ratio.r1.input == 0) // r1 not set
			pplaneScreenConfig->ratio.r3 = pplaneScreenConfig->ratio.r2 = pplaneScreenConfig->ratio.r1 = pplaneScreenConfig->ratio.r100;
		if (pplaneScreenConfig->ratio.r2.input == 0) // r2 not set
			pplaneScreenConfig->ratio.r3 = pplaneScreenConfig->ratio.r2 = pplaneScreenConfig->ratio.r1;
		if (pplaneScreenConfig->ratio.r3.input == 0) // r2 not set
			pplaneScreenConfig->ratio.r3 = pplaneScreenConfig->ratio.r2;
	}
	setup_chapter(file, "..", -1, READ);

End:
	setup_destroy(file);
	return (REPLY_OK);
}


int rx_planescreen_init(int planeNumber, const char * settingsPath, const char * ressourcesPath, const double fact[JET_CNT], SPlaneScreenConfig* pplaneScreenConfig)
{
	char filepath[MAX_PATH];
	STaConfig*	lTA, *lTARot;
	int i;
	
	memset(pplaneScreenConfig, 0, sizeof(SPlaneScreenConfig));
	if (rx_planescreen_load(planeNumber, settingsPath, ressourcesPath, pplaneScreenConfig))
		Error(WARN, 0, "Screen parameters file <%s> maybe inconsistent", settingsPath);

	// Init lut16 tables
	for (i = 0; i < 65536; i++)
		pplaneScreenConfig->lut16[i] = (UINT16)i;

	for (i=0; i<JET_CNT; i++)
		pplaneScreenConfig->densityFactor[i] = (UINT16)(fact[i]*257);
 
	if (pplaneScreenConfig->method >= 2) // need ta ? 
	{
		sprintf(filepath, "%sTA.dat", ressourcesPath);
		if (!rx_file_exists(filepath)) {
			sprintf(filepath, "%s../../TA/TA.dat", ressourcesPath); // find generic
			if (!rx_file_exists(filepath)) {
				Error(ERR_CONT, 0, "Threshold arrays file <%s> not found", filepath);
				return (REPLY_ERROR);
			}
		}

		// init threshold array
		if (rx_tas_load(filepath, &lTA)) {
			Error(ERR_CONT, 0, "Threshold arrays file <%s> are inconsistent", filepath);
			return (REPLY_ERROR);
		}

		if (planeNumber < 4) { // 0,1,2,3
			if (rx_tas_rotate(lTA, planeNumber, &pplaneScreenConfig->TA))
				return (REPLY_ERROR);
		}
		else {
			if ((planeNumber < 8) || (planeNumber > 11)) { // 4,5,6,7 and 12,13,14,15
				if (rx_tas_rotate(lTA, planeNumber % 4, &lTARot))
					return (REPLY_ERROR);
				if (rx_tas_shift(lTARot, &pplaneScreenConfig->TA))
				return (REPLY_ERROR);
				rx_tas_free(lTARot);
			}
			else { // 8,9,10, 11
				if (rx_tas_rotate(lTA, planeNumber % 4, &pplaneScreenConfig->TA))
					return (REPLY_ERROR);
			}

		}
		rx_tas_free(lTA);

		if (pplaneScreenConfig->noise > 0)
			if (rx_tas_addnoise(pplaneScreenConfig->TA, pplaneScreenConfig->noise)) {
				Error(ERR_CONT, 0, "Threshold arrays file <%s> are inconsistent", filepath);
				return (REPLY_ERROR);
			}
	}


	if (pplaneScreenConfig->method == 0) {
		if (pplaneScreenConfig->dropsize < 12) {
			rx_planescreen_init_FME_1x1(pplaneScreenConfig); // error diffusion, 1200, 1, 2 or 3
		}
		else if (pplaneScreenConfig->dropsize < 123)
		{
			rx_planescreen_init_FME_1x2g(pplaneScreenConfig);// error diffusion, 1200, 12, fill and grow 1 and 2
		}
		else {
			rx_planescreen_init_FME_1x3g(pplaneScreenConfig);// error diffusion, 1200, fill and grow 1, 2 and 3
		}
	}
	else if (pplaneScreenConfig->method == 1) {// Bayer 600x4not supported use FMG
		rx_planescreen_init_FMS_1x3g(pplaneScreenConfig);// Threshold, 1200, fill and grow 1, 2 and 3
	}
	else
	{
		if (pplaneScreenConfig->dropsize < 12)
		{
			if (pplaneScreenConfig->fillmethod < 2)
				rx_planescreen_init_FMS_1x1(pplaneScreenConfig); // Threshold, 1200, 1, 2 or 3
			else
				rx_planescreen_init_FMS_1x1r(pplaneScreenConfig); // Threshold, 1200, ratio mode, 1, 2 or 3

		}
		else if (pplaneScreenConfig->dropsize < 123)
		{
			if (pplaneScreenConfig->fillmethod < 1) {
				rx_planescreen_init_FMS_1x2s(pplaneScreenConfig);// Threshold, 1200, fill then grow 1 and 2
			}
			else 	if (pplaneScreenConfig->fillmethod < 2) {
				rx_planescreen_init_FMS_1x2g(pplaneScreenConfig); // Threshold, 1200, fill and grow 1 and 2
			}
			else {
				rx_planescreen_init_FMS_1x2r(pplaneScreenConfig); // Threshold, 1200, ratio mode 1 and 2
			}
		}
		else
		{
			if (pplaneScreenConfig->fillmethod < 1) {
				rx_planescreen_init_FMS_1x3s(pplaneScreenConfig);// Threshold, 1200, fill then grow 1, 2 and 3
			}
			else 	if (pplaneScreenConfig->fillmethod < 2) {
				rx_planescreen_init_FMS_1x3g(pplaneScreenConfig);// Threshold, 1200, fill and grow 1, 2 and 3
			}
			else {
				rx_planescreen_init_FMS_1x3r(pplaneScreenConfig);// Threshold, 1200, ratio mode 1, 2 and 3
			}
		}
	}
	return (REPLY_OK);
}

//--- rx_planescreen_close ----------------------------------------------------------------------
int rx_planescreen_close(SPlaneScreenConfig* pplaneScreenConfig)
{
	if (pplaneScreenConfig->TA) // need ta ? 
	{
		rx_tas_free((STaConfig *)pplaneScreenConfig->TA);
	}

	if (pplaneScreenConfig->fctclose)
		return(pplaneScreenConfig->fctclose(pplaneScreenConfig));
	else
	{
		Error(ERR_CONT, 0, "Screen type %d: <%s> not supported", pplaneScreenConfig->type, pplaneScreenConfig->typeName);
		return (REPLY_ERROR);
	}

}

//--- rx_screen_slice ----------------------------------------------------------------------
int rx_screen_slice(SSLiceInfo *inplane, SSLiceInfo *outplane, SPlaneScreenConfig* pplaneScreenConfig)
{

	if (inplane->buffer == NULL) return Error(ERR_CONT, 0, "No output buffer for rx_screen_plane: %d", pplaneScreenConfig->planeNumber);

	if (pplaneScreenConfig->fctplane)
		return(pplaneScreenConfig->fctplane(inplane, outplane, pplaneScreenConfig));
	else
	{
		Error(ERR_CONT, 0, "Screen type %d: <%s> not supported", pplaneScreenConfig->type, pplaneScreenConfig->typeName);
		return (REPLY_ERROR);
	}

}

//--- rx_screen_outslice_init ----------------------------------------------------------------------
int rx_screen_outslice_init(SSLiceInfo *inplane, SSLiceInfo *outplane, SPlaneScreenConfig* pplaneScreenConfig)
{
	outplane->widthPx = inplane->widthPx;
	outplane->lengthPx = inplane->lengthPx;
	outplane->bitsPerPixel = pplaneScreenConfig->outputbitsPerPixel;
	outplane->aligment = 8;
	outplane->lineLen = ((outplane->widthPx * outplane->bitsPerPixel) + outplane->aligment - 1) / outplane->aligment;
	outplane->planeNumber = inplane->planeNumber;
	outplane->Xoffset = inplane->Xoffset;

	if ((outplane->buffer == NULL) && (outplane->dataSize==0)) {
		outplane->dataSize = (UINT64)outplane->lineLen*outplane->lengthPx;
		outplane->buffer = rx_mem_alloc(outplane->dataSize);
		TrPrintfL(60, "data_malloc buffer: %p, size=%u MB, free=%u MB", outplane->buffer, outplane->dataSize / 1024 / 1024, rx_mem_get_freeMB());
		if (outplane->buffer == NULL) return Error(ERR_CONT, 0, "No buffer for screening: %d");
	}
	else {
		if (outplane->dataSize < (UINT64)outplane->lineLen*outplane->lengthPx)
			return Error(ERR_CONT, 0, "Output buffer too small : %d", outplane->dataSize);
	}
	memset(outplane->buffer, 0, (INT32)outplane->dataSize);

	return REPLY_OK;
}




//--- rx_planescreen_close_null ----------------------------------------------------------------------
int rx_planescreen_close_null(void* pplaneScreenConfig)
{

	return (REPLY_OK);
}
