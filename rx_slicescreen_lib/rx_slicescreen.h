// ****************************************************************************
//
//	rx_slicescreen.h
//
// ****************************************************************************
//
//	Copyright 2017 by Radex AG, Switzerland. All rights reserved.
//	Written by Gilbert Therville 
//
// ****************************************************************************

#pragma once

#include "rx_def.h"
#include "rx_tas.h"

//--- SSliceInfo ---------------------------------------------
typedef struct
{
	UINT32 planeNumber;		// plane number
	UINT32 Xoffset;			// X offset of the slice
	UINT32 widthPx;
	UINT32 lengthPx;
	UINT32 bitsPerPixel;
	UINT32 lineLen;		// in bytes
	UINT32 aligment;	// 8, 16, 32 ,....
	SPoint resol;

	UINT64 dataSize;
	PBYTE buffer;
} SSLiceInfo;

// --- SRatio --------------------------------------------
typedef struct
{
	int			input;
	int			smallD;
	int			mediumD;
	int			largeD;
} SRatio;

// --- SRatioCurve --------------------------------------------
typedef struct
{
	SRatio		r1; // 1st ratio (not used if 1 drop size)
	SRatio		r2; // 2nd ratio (not used if 1 drop size)
	SRatio		r3; //  3 rd ratio (not used if 1 drop or 2 drops size)
	SRatio		r100; // last ratio (input is 100%)
} SRatioCurve;

#define JET_CNT	(2048+128)

// --- SPlaneScreenConfig --------------------------------------------
typedef struct
{
	char *		ressourcePath; // path where to find ressource files. (Example: TA file)
	int			planeNumber; // 0 to 15
	int			type;
	char		typeName[16];
	int			method;		/* error diffusion, bayer, threshold */
	int			dropsize;		/* 1, 2, 3, 12, 123 */
	int			fillmethod;		/* fill then grow, fill and grow , ratio mode*/
	int			limit[2]; /* ratio (0 to 100) Small to Medium and Medium to Large
									  (default: 50 for 2 drops, 33 and 67 for 3 drops) */
	SRatioCurve	ratio; /* ratio (0 to 100) ratio according input grey, last one is for input 100% */

	int			noise;	// noise to add to TA: from 0: no to 100: full
	STaConfig*	TA;
	int			outputbitsPerPixel; // 1 or 2
	UINT16		lut16[65536];// 16 bits per pixel lut table 65536 short for small 
	UINT16		lut16_M[65536];// 16 bits per pixel lut table 65536 short for medium
	UINT16		lut16_L[65536];// 16 bits per pixel lut table 65536 short for large
	UINT16		densityFactor[JET_CNT];
	int(*fctplane)	(SSLiceInfo *inplane, SSLiceInfo *outplane, void* SPlaneScreenConfig);
	int(*fctclose)	(void* SPlaneScreenConfig);
} SPlaneScreenConfig;



// --- Prototypes --------------------------------------------
#ifdef __cplusplus
extern "C"{
#endif

	int rx_planescreen_load(int planeNumber, const char * settingsPath, const char * ressourcesPath, SPlaneScreenConfig* pplaneScreenConfig);
	int rx_planescreen_init(int planeNumber, const char * settingsPath, const char * ressourcesPath, const double fact[JET_CNT], SPlaneScreenConfig* pplaneScreenConfig);
	int rx_planescreen_close(SPlaneScreenConfig* pplaneScreenConfig);
	int rx_screen_slice(SSLiceInfo *inplane, SSLiceInfo *outplane, SPlaneScreenConfig* pplaneScreenConfig);
	int rx_screen_outslice_init(SSLiceInfo *inplane, SSLiceInfo *outplane, SPlaneScreenConfig* pplaneScreenConfig);

	int rx_planescreen_close_null(void* pplaneScreenConfig);

#ifdef __cplusplus
}
#endif
