// ****************************************************************************
//
//	rx_tas.h	Threshold arrays
//
// ****************************************************************************
//
//	Copyright 2017 by BM-Printing SA, Switzerland. All rights reserved.
//	Written by Gilbert Therville
//
// ****************************************************************************


// --- STaConfig --------------------------------------------
typedef struct
{
	int			width;
	int			heigth;
	BYTE		*ta;
	UINT16		*ta16;
} STaConfig;


#ifdef __cplusplus
extern "C"{
#endif

	int rx_tas_load(const char * path, STaConfig **pta);
	int rx_tas_rotate(STaConfig *ptasrc, int idx, STaConfig **pta);
	int rx_tas_shift(STaConfig *ptasrc, STaConfig **pta);
	int rx_tas_addnoise(STaConfig *ptasrc, int noise);
	int rx_tas_free(STaConfig *pta);

#ifdef __cplusplus
}
#endif