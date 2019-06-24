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

int rx_luts_load(char * path, SColorSplitCfg *psplit, int splitCnt, BYTE pluts[MAX_COLORS][256]);

#ifdef __cplusplus
}
#endif