// ****************************************************************************
//
//	ink_def.h	
//
// ****************************************************************************
//
//	Copyright 2014 by Radex AG, Switzerland. All rights reserved.
//	Written by Marcel Galliker 
//
// ****************************************************************************

#pragma once

#include "rx_def.h"

int ink_read(const char *filepath,  SInkDefinition *pink);
int ink_write(const char *filepath, SInkDefinition *pink);
