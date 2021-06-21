// ****************************************************************************
//
//	rx_bitmap.c
//
// ****************************************************************************
//
//	Copyright 2014 by Radex AG, Switzerland. All rights reserved.
//	Written by Marcel Galliker 
//
// ****************************************************************************

#include "rx_bitmap.h"

//--- rxbmp_create -----------------------------------------------
int bmp_create(RX_Bitmap *pbmp, int width, int height, int bppx)
{
	pbmp->bppx		= bppx;
	pbmp->width		= width;
	pbmp->height	= height;
	pbmp->lineLen	= ((width*bppx+31)/32)*4;
	pbmp->sizeUsed	= height*pbmp->lineLen;
	if (pbmp->sizeUsed>pbmp->sizeAlloc)
	{
		if (pbmp->buffer) free(pbmp->buffer);
		pbmp->sizeAlloc = pbmp->sizeUsed;
		pbmp->buffer	= (BYTE*)malloc(pbmp->sizeAlloc);
	}
	return pbmp->buffer?REPLY_OK:REPLY_ERROR;
}

//--- bmp_clear -----------------------------------------------------------
int bmp_clear  (RX_Bitmap *pbmp, int x, int y, int width, int height)
{
	BYTE *dst;
	int pxPerByte = 8/pbmp->bppx;		

	x = ((x+pxPerByte-1)/pxPerByte)*pxPerByte;
	if (x+width>pbmp->width) width=pbmp->width-x;
	width /= pxPerByte;
	if (y+height>pbmp->height) height=pbmp->height-y;
	dst = &pbmp->buffer[y*pbmp->lineLen+x/pxPerByte];
	for (; height>0; y++, height--, dst+=pbmp->lineLen)
	{
		memset(dst, 0x00, width);
	}
	return REPLY_OK;
}

//--- rxbmp_destroy ----------------------------------------
int bmp_destroy(RX_Bitmap *pbmp)
{
	free(pbmp->buffer);
	memset(pbmp, 0,  sizeof(RX_Bitmap));
	return REPLY_OK;
}