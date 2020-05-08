// ****************************************************************************
//
//	DIGITAL PRINTING - rx_hash.c
//
//	calculates hash-code on memory or file
// 
// ****************************************************************************
//
//	Copyright 2015 by Radex AG, Switzerland. All rights reserved.
//	Written by Marcel Galliker
//
// ****************************************************************************

#include "rx_common.h"
#include "polarssl/sha1.h"

//--- rx_hash_file ---------------------------
int rx_hash_file( const char *path, unsigned char output[20] )
{
	return sha1_file(path, output);
}

//--- rx_hash_mem -------------------------------
int rx_hash_mem( const unsigned char *input, size_t ilen, unsigned char output[20] )
{
	sha1(input, ilen, output);
	return REPLY_OK;
}


//--- rx_hash_mem_str ---------------------------------------------------------------
int rx_hash_mem_str ( const unsigned char *input, size_t ilen, unsigned char output[41] )
{
	unsigned char hash[20];
	int i;
	sha1(input, ilen, hash);
	for (i=0; i<20; i++) sprintf(&output[2*i], "%02X", hash[i]);
	return REPLY_OK;	
}
