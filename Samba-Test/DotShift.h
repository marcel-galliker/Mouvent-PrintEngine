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

int shift_bmp(const wchar_t *in_path, const wchar_t *out_path, const wchar_t *dotGeneration, const wchar_t *bitsPerPixel, int pixel0, int backwards, int rightToLeft, int version);
int shift_bmp_reverse(const wchar_t *in_path, const wchar_t *out_path, int version);
int udp_data(const wchar_t *in_path, const wchar_t *out_path, const wchar_t *info_path, int pixel0, const wchar_t *bitsPerPixel);