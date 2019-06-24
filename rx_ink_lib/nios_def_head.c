
// ****************************************************************************
//
//	DIGITAL PRINTING - nios_def_head.h
//
//	Communication between ARM processor and FPGA
//
// ****************************************************************************
//
//	Copyright 2013 by Radex AG, Switzerland. All rights reserved.
//	Written by Marcel Galliker
//
// ****************************************************************************

#include "nios_def_head.h"

char *PowerStateStr(EnPowerState state)
{
	switch(state)
	{
	case power_off:				return "off";
	case power_wait_all_off:	return "wait_all_off";
	case power_wait_amp_on:		return "wait_amp_on";
	case power_wait_all_on:		return "wait_all_on";
	case power_all_on:			return "all_on";
	case power_sd:				return "sd";
	case power_sd_3v3:			return "sd_3v3";
	case power_sd_amp:			return "sd_amp";
	case power_down:			return "down";
	default:					break;
	};
	return "unknown";
};
