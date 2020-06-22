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
#include "rx_trace.h"
#include "rx_luts.h"
#include "rx_setup_file.h"

#define __STDC_FORMAT_MACROS
#include <inttypes.h>
// #include <io.h>


//--- rx_luts_load ------------------------------------------------------------
int rx_luts_load(const char * path, SColorSpaceConfig *pcs, BYTE pluts[MAX_COLORS][256], UINT16 pluts16[MAX_COLORS][65536])
{
	int c, i, j, p;

	TiXmlDocument *pDoc = new TiXmlDocument;
	CRX_XmlElement	*transferSet = NULL;
	CRX_XmlElement	*transferCurve;
	int error = REPLY_OK;
	int filledLut = 0;

	// Init lut tables
	for (c = 0; c < MAX_COLORS && c < pcs->count; c++)
	for (i = 0; i < 256; i++)
		pluts[c][i] = (BYTE)i;

	// Init lut16 tables
	for (c = 0; c < MAX_COLORS && c < pcs->count; c++)
	for (i = 0; i < 65536; i++)
		pluts16[c][i] = (UINT16)i;


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
		UINT16 tmpLut16[65536];
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
		p = 0;
		token = strtok(curve, " \t");
		while (token) {
			sscanf(token, "%f", &CurveXY[p].x);
			token = strtok(NULL, " \t");
			if (!token) break;
			sscanf(token, "%f", &CurveXY[p].y);
			p++;
			token = strtok(NULL, " \t");
		}

		if (p < 2) {
			error = REPLY_ERROR;
			break;							// at least 2 points
		}

		// Move point x,y to the temp lut table
		memset(tmpLut, 0, sizeof(tmpLut));
		for (c = 0; c < p; c++) {
			BYTE x, y;
			x = (BYTE)((((CurveXY[c].x * 255) + 0.5) > 255) ? 255 : ((CurveXY[c].x * 255) + 0.5));
			y = (BYTE)((((CurveXY[c].y * 255) + 0.5) > 255) ? 255 : ((CurveXY[c].y * 255) + 0.5));
			tmpLut[x] = y;
		}

		// Fill the last point if not
		if (tmpLut[255] == 0) tmpLut[255] = 255;

		// Fill the interval
		i = 0;
		while (i < 255) {
			float coef;
			c = 1;
			while (tmpLut[i + c] == 0) c++;
			coef = (float)(tmpLut[i + c] - tmpLut[i]);
			coef /= c;
			for (j = 1; j < c; j++)
				tmpLut[i + j] = (tmpLut[i] + (BYTE)((j*coef) + 0.5) > 255) ? 255 : tmpLut[i] + (BYTE)((j*coef) + 0.5);
			i += c;
		}


		// Move point x,y to the temp lut table
		memset(tmpLut16, 0, sizeof(tmpLut16));
		for (c = 0; c < p; c++) {
			UINT16 x, y;
			x = (UINT16)((((CurveXY[c].x * 65535) + 0.5) > 65535) ? 65535 : ((CurveXY[c].x * 65535) + 0.5));
			y = (UINT16)((((CurveXY[c].y * 65535) + 0.5) > 65535) ? 65535 : ((CurveXY[c].y * 65535) + 0.5));
			tmpLut16[x] = y;
		}

		// Fill the last point if not
		if (tmpLut16[65535] == 0) tmpLut16[65535] = 65535;

		// Fill the interval
		i = 0;
		while (i < 65535) {
			float coef;
			c = 1;
			while (tmpLut16[i + c] == 0) c++;
			coef = (float)(tmpLut16[i + c] - tmpLut16[i]);
			coef /= c;
			for (j = 1; j < c; j++) {
				tmpLut16[i + j] = (tmpLut16[i] + (UINT16)((j*coef) + 0.5) > 65535) ? 65535 : tmpLut16[i] + (UINT16)((j*coef) + 0.5);
			}

			i += c;
		}

		// Copy temp lut table to the good lut table, fill the not specified with default value
		for (c = 0; c < MAX_COLORS && c < pcs->count; c++) {
			if (strncmp(separation, "All", strlen(separation)) == 0) {
				if (!(filledLut & (1 << c))) {
					memcpy(pluts[c], tmpLut, sizeof(tmpLut));
					memcpy(pluts16[c], tmpLut16, sizeof(tmpLut16));
				}
			}
			else if (pcs->color[c].name[0]) {
				char *name = strdup(pcs->color[c].name);
				for (token = name; *token; token++) *token = tolower(*token);

				if (strncmp(separation, name, strlen(separation)) == 0) {
					memcpy(pluts[c], tmpLut, sizeof(tmpLut));
					memcpy(pluts16[c], tmpLut16, sizeof(tmpLut16));
					filledLut |= (1 << c);
					free(name);
					break;
				}
				free(name);
			}
		}
	}

end:
	delete pDoc;
	return error;
}


//--- rx_adjust_load ------------------------------------------------------------
int rx_adjust_load(const char * path, SColorSpaceConfig *pcs, float padjust[MAX_COLORS])
{
	HANDLE  file;
	char	name[64];
	int i;
	unsigned int count;
	int error = REPLY_OK;

	// Init adjust values
	for (i = 0; i < MAX_COLORS && i < pcs->count; i++)
		padjust[i] = 1.0;

	file = setup_create();
	if (setup_load(file, path))
		{ error = REPLY_ERROR;	goto end; }

	if (setup_chapter(file, "LastAdjust", -1, READ) != REPLY_OK)
		{ error = REPLY_ERROR;	goto end; }

	if (setup_chapter(file, "LA", -1, READ) != REPLY_OK)
		{ error = REPLY_ERROR;	goto end;	}

	setup_str(file, "Name", READ, name, sizeof(name), "Unknown");
	setup_uint32(file, "Count", READ, &count, 0);

	if (count)
	{
		setup_chapter(file, "..", -1, READ);
		if (setup_chapter(file, "Adjust", -1, READ) != REPLY_OK) return REPLY_ERROR;
		for (i = 0; i<(int)count; i++)
		{
			int plane;
			int ratio, cpt;
			float adjust;
			setup_int32(file, "Plane", READ, &plane, 0);
			if ((plane < -1) || (plane >= MAX_COLORS))
				{	error = REPLY_ERROR;	goto end;	}
			if ((int)plane < pcs->count) {
				setup_int32(file, "Ratio", READ, &ratio, 0);
				if ((ratio < -100) || (ratio >= 100))
					{ error = REPLY_ERROR;	goto end; }
				adjust = (float) (1.0 + ratio /  100.0);
				if (plane < 0) {
					for (cpt=0; cpt <pcs->count; cpt++)
						padjust[cpt] = adjust;
				}
				else
					padjust[plane] = adjust;
			}
			setup_chapter_next(file, READ, name, sizeof(name));
			if (strcmp(name, "Adjust")) break;
		}
	}

end:
	setup_destroy(file);
	return error;
}


//--- rx_luts_adjust ------------------------------------------------------------
int rx_luts_adjust(SColorSpaceConfig *pcs, float padjust[MAX_COLORS], BYTE pluts[MAX_COLORS][256], UINT16 pluts16[MAX_COLORS][65536])
{
	int c, i;
	BYTE tmpLut[256];
	UINT16 tmpLut16[65536];

	for (c = 0; c < MAX_COLORS && c < pcs->count; c++) {
		for (i = 0; i<256; i++) {
			float f_lut;
			BYTE b_lut;
			f_lut = padjust[c] * i;
			if (f_lut <0) f_lut = 0;
			if (f_lut > 255.0) f_lut = 255.0;
			b_lut = (BYTE) (f_lut + 0.5);
			tmpLut[i] = pluts[c][b_lut];
		}
		memcpy(pluts[c], tmpLut, sizeof(tmpLut));
	}

	for (c = 0; c < MAX_COLORS && c < pcs->count; c++) {
		for (i = 0; i<65536; i++) {
			float f_lut;
			UINT16 w_lut;
			f_lut = padjust[c] * i;
			if (f_lut <0) f_lut = 0;
			if (f_lut > 65535.0) f_lut = 65535.0;
			w_lut = (UINT16)(f_lut + 0.5);
			tmpLut16[i] = pluts16[c][w_lut];
		}
		memcpy(pluts16[c], tmpLut16, sizeof(tmpLut16));
	}

	return REPLY_OK;
}
