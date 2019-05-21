// ****************************************************************************
//
//	rx_embrip_def.c
//
// ****************************************************************************
//
//	Copyright 2013 by Radex AG, Switzerland. All rights reserved.
//	Written by Marcel Galliker 
//
// ****************************************************************************

#include "rx_embrip_def.h"

//--- Global variables -----------------------------------------------------

char *EScreeningModeStr(EScreeningMode mode)
{
	switch (mode)
	{
	case sm_1: return "sm_1";
	case sm_2: return "sm_2";
	case sm_3: return "sm_3";
	case sm_4: return "sm_4";
	default:	return "undef";
	}	
};


char *ERenderIntentStr(ERenderIntent intent)
{
	switch (intent)
	{
	case ri_perceptual:		return "perceptual";
	case ri_colorimetric:	return "colorimetric";
	case ri_saturation:		return "saturation";
	case ri_absolute:		return "absolute";
	default: return "undef";
	}
};


char *EBlackPreserveStr(EBlackPreserve preserve)
{
	switch(preserve)
	{
	case bp_no:				return "no";
	case bp_black_only:		return  "black only";
	case bp_black_plane:	return "plane";
	default: return "undef";
	}
}

char *EBoolStr(int value)
{
	return value? "True" : "False";
}
