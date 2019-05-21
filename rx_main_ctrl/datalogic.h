// ****************************************************************************
//
//	datalogic.h	
//
// ****************************************************************************
//
//	Copyright 2015 by Radex AG, Switzerland. All rights reserved.
//	Written by Marcel Galliker 
//
// ****************************************************************************

#pragma once

void dl_init(void);
void dl_reset(int isNo);
void dl_identify(int isNo);
void dl_trigger(int isNo);

void dl_get_barcode(int isNo, char *scannerSN, char *barcode);