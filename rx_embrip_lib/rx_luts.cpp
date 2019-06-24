// ****************************************************************************
//
// rx_luts.cpp	Lut Tables
//
// ****************************************************************************
//
//	Copyright 2017 by BM-Printing SA, Switzerland. All rights reserved.
//	Written by Hubert Zimmermann
//
// ****************************************************************************
//
//
////#include "stdafx.h"
#include "rx_def.h"
#include "rx_xml.h"
#include "fastlz.h"
#include "rx_common.h"
#include "rx_file.h"
#include "rx_luts.h"

#define __STDC_FORMAT_MACROS
#include <inttypes.h>
// #include <io.h>


//--- rx_luts_load ------------------------------------------------------------
int rx_luts_load(char * path, SColorSplitCfg *psplit, int splitCnt, BYTE pluts[MAX_COLORS][256])
{
	int c, i, j;

	TiXmlDocument *pDoc = new TiXmlDocument;
	CRX_XmlElement	*transferSet = NULL;
	CRX_XmlElement	*transferCurve;
	int error = REPLY_OK;
	int filledLut = 0;

	// Init lut tables
	for (c = 0; c < MAX_COLORS && c < splitCnt; c++)
		for (i = 0; i < 256; i++)
			pluts[c][i] = (BYTE)i;

	// Check file and load
	if (!pDoc->LoadFile(path)) {
		error = REPLY_ERROR;
		goto end;
	}
	transferSet = (CRX_XmlElement*)pDoc->FirstChild("TransferCurveSet");
	if (transferSet == NULL) {
		error = REPLY_ERROR;
		goto end;
	}

	// Fill lut tables
	for (transferCurve = (CRX_XmlElement *)transferSet->FirstChild("TransferCurve"); transferCurve; transferCurve = (CRX_XmlElement *)transferCurve->NextSibling("TransferCurve"))
	{
		BYTE tmpLut[256];
		char curve[1024];
		char separation[256];
		struct {
			float x;
			float y;
		} CurveXY[256];
		char *token = NULL;

		// Get info from xml file
		transferCurve->Attribute("Separation", separation, SIZEOF(separation), "All");
		transferCurve->Attribute("Curve", curve, SIZEOF(curve), "0.0 0.0   1.0 1.0");
		for (token = separation; *token; token++) *token = tolower(*token);

		// Split Curve to i point x,y
		i = 0;
		token = strtok(curve, " \t");
		while (token) {
			sscanf(token, "%f", &CurveXY[i].x);
			token = strtok(NULL, " \t");
			if (!token) break;
			sscanf(token, "%f", &CurveXY[i].y);
			i++;
			token = strtok(NULL, " \t");
		}

		if (i < 2) {
			error = REPLY_ERROR;
			break;							// at least 2 points
		}

		// Move point x,y to the temp lut table
		memset(tmpLut, 0, SIZEOF(tmpLut));
		for (c = 0; c < i; c++) {
			BYTE x, y;
			x = (BYTE) ((CurveXY[c].x * 255) + 0.5);
			y = (BYTE) ((CurveXY[c].y * 255) + 0.5);
			if (x>255) x = 255;
			if (y>255) y = 255;
			tmpLut[x] = y;
		}
		if (tmpLut[255] == 0) tmpLut[255] = 255;

		// Fill the interval
		i = 0;
		while (i < 255) {
			float coef;
			c = 1;
			while (tmpLut[i+c] == 0) c++;
			coef = (float) (tmpLut[i + c] - tmpLut[i]);
			coef /= c;
			for (j = 1; j < c; j++)
				tmpLut[i + j] = tmpLut[i] + (BYTE) ((j*coef)+0.5);
			i += c;
		}

		// Copy temp lut table to the good lut table, fill the not specified with default value
		for (c = 0; c < MAX_COLORS && c < splitCnt; c++) {
			if (strncmp(separation, "all", strlen(separation)) == 0) {
				if (!(filledLut & (1 << c)))
					memcpy(pluts[c], tmpLut, SIZEOF(tmpLut));
			} else if (psplit[c].color.name[0]) {
				char *name = strdup(psplit[c].color.name);
				for (token = name; *token; token++) *token = tolower(*token);

				if (strncmp(separation, name, strlen(separation)) == 0) {
					memcpy(pluts[c], tmpLut, SIZEOF(tmpLut));
					filledLut |= (1 << c);
					break;
				}
			}
		}
	}
	
end:
	delete pDoc;
	return error;
}