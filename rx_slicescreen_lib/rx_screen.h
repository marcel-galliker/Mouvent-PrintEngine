// ****************************************************************************
//
//	rx_screen.h
//
// ****************************************************************************
//
//	Copyright 2017 by Radex AG, Switzerland. All rights reserved.
//	Written by Gilbert Therville 
//
// ****************************************************************************

#pragma once

#include "rx_def.h"


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


// --- SScreenConfig --------------------------------------------
typedef struct
{
	int			type;
	char		typeName[16];
	int			method;		/* error diffusion, bayer, threshold */
	int			dropsize;		/* 1, 2, 3, 12, 123 */
	int			fillmethod;		/* fill then grow, fill and grow , ratio mode*/
	int			limit[MAX_COLORS][2]; /* ratio (0 to 100) Small to Medium and Medium to Large 
									  (default: 50 for 2 drops, 33 and 67 for 3 drops) */
	SRatioCurve	ratio[MAX_COLORS]; /* ratio (0 to 100) ratio according input grey, last one is for input 100% */
					 
	int			missinglinecpt; /* to simulate missing line default:0 max 32 */
	int			missingline[32][2]; /* plane number, line number for each missing line */
	int			dropcount;
	int			screenByPrinter; /* 1: the screening is made by printer */
	int			output;			/* 0 compressed, 1 uncompressed, 2 none */
	int			totalInkLimit;
	int			minPenetrationFluid; /* minimum fluid default:0 */
	int			noise;	// noise to add to TA: from 0: no to 100: full
	STaConfig*	TA[8];
	SPoint		inputResol;
	SPoint		outputResol;
	int			overSampling;	// 0: No, 1: Normal, 2: Light, 3: Bold
	int			antiBleeding;	// 0: No, 1: std, 2:stronger
	int			outputbitsPerPixel; // 1 or 2
	int			inputbitsPerPixel; // 8 or 16
	int			inMaxInk[MAX_COLORS]; // colorplane to compute in totalInkLimit(+x)
	int			penetrationFluid[MAX_COLORS]; // colorplane if penetration fluid or CMYK(+x)
	BYTE		lut[MAX_COLORS][256]; // 8 bits per pixel lut table 256 bytes
	UINT16		lut16[MAX_COLORS][65536];// 16 bits per pixel lut table 65536 short for small 
	UINT16		lut16_M[MAX_COLORS][65536];// 16 bits per pixel lut table 65536 short for medium
	UINT16		lut16_L[MAX_COLORS][65536];// 16 bits per pixel lut table 65536 short for large
	int(*fctplane)	(SPlaneInfo *inplane, SPlaneInfo *outplane, void* pscreenConfig);
	int			(*fctclose)	(void* pscreenConfig);
} SScreenConfig;




// --- Prototypes --------------------------------------------
#ifdef __cplusplus
extern "C"{
#endif

	int rx_screen_load		(const char * path, SScreenConfig* pscreenConfig);
	int rx_screen_init		(const char * jobSettingsDir, const char * printEnv, SColorSpaceConfig *pcs, int pattern, SScreenConfig* pscreenConfig);
	int rx_screen_bmp		(SBmpInfo *pinfo, SScreenConfig* pscreenConfig);
	int rx_screen_plane		(SPlaneInfo *inplane, SPlaneInfo *outplane, SScreenConfig* pscreenConfig);
	int rx_screen_close		(SScreenConfig* pscreenConfig);
	int rx_screen_close_null(void* pscreenConfig);
	int rx_screen_outplane_init(SPlaneInfo *inplane, SPlaneInfo *outplane, SScreenConfig* pscreenConfig);
	int rx_lut_plane		(SPlaneInfo *inplane, SScreenConfig* pscreenConfig);
	int rx_lut_bmp			(SBmpInfo *pinfo, SScreenConfig* pscreenConfig);
	int rx_inklimit_bmp(SBmpInfo *pinfo, SScreenConfig* pscreenConfig);
	int rx_missinglines_add(SBmpInfo *pinfo, SScreenConfig* pscreenConfig);
	int rx_screen_plane_Printer(SPlaneInfo *inplane, SPlaneInfo *outplane, void* pscreenConfig);
	int rx_screen_plane_Printerx2(SPlaneInfo *inplane, SPlaneInfo *outplane, void* pscreenConfig);
	int rx_screen_plane_Printerx4(SPlaneInfo *inplane, SPlaneInfo *outplane, void* pscreenConfig);


#ifdef __cplusplus
}
#endif
