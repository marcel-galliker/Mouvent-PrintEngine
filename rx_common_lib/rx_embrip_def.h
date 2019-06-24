// ****************************************************************************
//
//	rx_rip_def.h
//
// ****************************************************************************
//
//	Copyright 2017 by Radex AG, Switzerland. All rights reserved.
//	Written by Marcel Galliker 
//
// ****************************************************************************

#pragma once

#include "rx_common.h"

#ifdef __cplusplus
extern "C"{
#endif

#pragma pack(1)

char *EBoolStr(int value);

//--- EScreeningMode -------------------------------------------
typedef enum EScreeningMode
{
    sm_undef,       // 00
    sm_1,           // 01
    sm_2,           // 02
    sm_3,           // 03
    sm_4,           // 04
	end,
} EScreeningMode;
char *EScreeningModeStr(EScreeningMode);

//--- ERenderIntent ---------------------------------------------
typedef enum ERenderIntent
{
    ri_undef,           //  00
    ri_perceptual,      //  01
    ri_colorimetric,    //  02
    ri_saturation,      //  03
    ri_absolute,        //  04
} ERenderIntent;
char *ERenderIntentStr(ERenderIntent);

//--- EBlackPreserve ----------------------------------------------
typedef enum EBlackPreserve
{
    bp_undef,           //  00
    bp_no,              //  01
    bp_black_only,      //  02
    bp_black_plane,     //  03
} EBlackPreserve;
char *EBlackPreserveStr(EBlackPreserve);

//--- SScreeningCfg ----------------------------------------------
typedef struct SScreeningCfg
{
    BYTE	mode;	// EScreeningMode
    char	profileName[256];
} SScreeningCfg;

//--- SRippingCfg ---------------------------------------------
typedef struct SRippingCfg
{
    char			outputProfileName[256];
    BYTE		    renderIntent;	// ERenderIntent
    BYTE			blackPreserve;	// EBlackPreserve
    BYTE            blackPtComp;
    BYTE            simuOverprint;
} SRippingCfg;

//--- SPrintEnv ----------------------------------------------
typedef struct SPrintEnv
{
    char			name[256];        
    SScreeningCfg   screening;
    SRippingCfg     ripping;
} SPrintEnv;    
	
#ifdef __cplusplus
}
#endif
