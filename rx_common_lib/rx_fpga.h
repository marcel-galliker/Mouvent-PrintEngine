// ****************************************************************************
//
//	rx_fpga.h
//
// ****************************************************************************
//
//	Copyright 2016 by Radex AG, Switzerland. All rights reserved.
//	Written by Marcel Galliker 
//
// ****************************************************************************

#pragma once

#include "rx_common.h"

int rx_fpga_load(char *path_rbf);
int rx_fpga_running(void);
void* rx_fpga_map_page(int memId, UINT64 addr, UINT32 size, UINT32 size_expected);
