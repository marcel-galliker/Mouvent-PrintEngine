// ****************************************************************************
//
//	Basics
//
// ****************************************************************************
//
//	Copyright 2014 by Radex AG, Switzerland. All rights reserved.
//	Written by Marcel Galliker 
//
// ****************************************************************************

#pragma once

#include "rx_xml.h"

void lc_get_symbols(void);
void lc_reset_error(void);

int lc_save(rx_doc doc, char *filter);
int lc_load(rx_doc doc);

