// ****************************************************************************
//
//	DIGITAL PRINTING - rx_hash.h
//
//	calculates hash-code on memory or file
// 
// ****************************************************************************
//
//	Copyright 2015 by Radex AG, Switzerland. All rights reserved.
//	Written by Marcel Galliker
//
// ****************************************************************************

#pragma once

int rx_hash_file	( const char *path,                        unsigned char output[20] );
int rx_hash_mem		( const unsigned char *input, size_t ilen, unsigned char output[20] );
int rx_hash_mem_str ( const unsigned char *input, size_t ilen, unsigned char output[41] );
