// ****************************************************************************
//
//	rx_csv.h
//
// ****************************************************************************
//
//	Copyright 2016 by Radex AG, Switzerland. All rights reserved.
//	Written by Marcel Galliker 
//
// ****************************************************************************

#pragma once

int  csv_get_int(const char *str, int col);
void csv_put(const char *instr, int col, const char *value, char *outstr);