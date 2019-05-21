// ****************************************************************************
//
//	
//
// ****************************************************************************
//
//	Copyright 2013 by Radex AG, Switzerland. All rights reserved.
//	Written by Marcel Galliker 
//
// ****************************************************************************

#pragma once

#include "export.h"
#include "rx_xml.h"
#include "rx_embrip_def.h"


typedef struct
{
	INT32 width;
	INT32 height;
	INT32 bitsPerPixel;
	INT32 lineLen;
} SEmbRipCfg;

#ifdef __cplusplus
extern "C"{
#endif

EXPORT HANDLE embrip_create_xml(void);
EXPORT int    embrip_save_xml(HANDLE file, const char *path);

EXPORT HANDLE embrip_open_xml(const char *path);
EXPORT int    embrip_close_xml(HANDLE file, int save);

EXPORT int    embrip_screening_cfg(HANDLE file, int write, SScreeningCfg *cfg);
EXPORT int    embrip_ripping_cfg  (HANDLE file, int write, SRippingCfg *cfg);

EXPORT int	  embrip_ScreeningModeStr(EScreeningMode mode, char *str);

#ifdef __cplusplus
}
#endif