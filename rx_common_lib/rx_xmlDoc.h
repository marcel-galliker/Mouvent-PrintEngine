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

#include "rx_common.h"

typedef  void* rx_doc;

DLL_EXPORT rx_doc rx_doc_load(char *path);
DLL_EXPORT rx_doc rx_doc_new ();
DLL_EXPORT int    rx_doc_save(char *path, rx_doc doc);
DLL_EXPORT int    rx_doc_free(rx_doc doc);


