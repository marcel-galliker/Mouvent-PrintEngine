// ****************************************************************************
//
//	rx_luts.h	Lut Tables
//
// ****************************************************************************
//
//	Copyright 2017 by BM-Printing SA, Switzerland. All rights reserved.
//	Written by Hubert Zimmermann
//
// ****************************************************************************

#ifdef __cplusplus
extern "C"{
#endif

	int rx_luts_load(const char * path, SColorSpaceConfig *pcs, BYTE pluts[MAX_COLORS][256], UINT16 pluts16[MAX_COLORS][65536]);
	int rx_adjust_load(const char * path, SColorSpaceConfig *pcs, float padjust[MAX_COLORS]);
	int rx_luts_adjust(SColorSpaceConfig *pcs, float padjust[MAX_COLORS], BYTE pluts[MAX_COLORS][256], UINT16 pluts16[MAX_COLORS][65536]);

#ifdef __cplusplus
}
#endif