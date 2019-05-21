// ****************************************************************************
//
//	rx_embrip.h
//
// ****************************************************************************
//
//	Copyright 2017 by Radex AG, Switzerland. All rights reserved.
//	Written by Gilbert Therville 
//
// ****************************************************************************

#pragma once

#include "rx_def.h"

// --- SPdfConfig --------------------------------------------
typedef struct
{
	char		outputProfile [256];
	int			renderIntent;
	int			blackPreserve;
	int			blackPtComp;
	int			simuOverprint;
} SPdfConfig;


// --- SScreenConfig --------------------------------------------
typedef struct
{
	int			type;
	char		typeName[16];
	void*		extra;
	SPoint		inputResol;
	SPoint		outputResol;
	BYTE		lut[MAX_COLORS][256];
} SScreenConfig;

// --- Prototypes --------------------------------------------
int rx_bmp_scale		(SBmpInfo *pinfo, SPoint NewDim, SPoint NewResol);

int rx_screen_init		(char * printEnv, SScreenConfig* pscreenConfig);
int rx_bmp_screen		(SBmpInfo *pinfo, SScreenConfig* pscreenConfig);
int bmp_plane_screen	(SHORT plane, SBmpInfo *pinfo, SScreenConfig* pscreenConfig);

int rx_bmp_rotate		(SBmpInfo *pinfo, int rotation);
int rx_bmp_place		(SBmpInfo *pinfo, int columns, int colDist);

int rx_tiff_rip			(char * printEnv, char * inputpath, int rotation, int columns, int colDist, char * outputpath, char * jobName);
int rx_pdf_init			(char * printEnv, SPdfConfig* ppdfConfig);
int rx_pdf_rip			(char * printEnv, char * inputpath, int rotation, int columns, int colDist, char * outputpath, char * jobName);

