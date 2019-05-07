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


#include "stdafx.h"


// ----------------------------------------------------------------------------
// Fuction:		BMP_write
// Return:		
// Description:	
// ----------------------------------------------------------------------------
int BMP_write(const wchar_t *filename, unsigned char *pBuffer, int bitsPerPixel, int width, int height)
{
// write Print Memory to file
	BITMAPFILEHEADER	header;
	BITMAPINFOHEADER	bmpInfo;
	ULONG				bmiColors[16];
	int					bmpSize;
	int					len;
	int					bbpx;
	FILE *file=NULL;

	if (pBuffer==NULL) return 0;

	_wfopen_s(&file, filename, L"wb");
	if (file)
	{		
		bbpx    =  bitsPerPixel;
		if (bitsPerPixel==2) bitsPerPixel=4;
		width   = (width+63) & ~63;
//		height  = 664;

		bmpSize = (width*height)/8*bitsPerPixel;

		memset(&header, 0, sizeof(header));

		memcpy(&header.bfType, &"BM", 2);
		header.bfOffBits		= sizeof(header)+sizeof(bmpInfo)+sizeof(bmiColors);
		header.bfSize			= header.bfOffBits+bmpSize;
		
		fwrite(&header, 1, sizeof(header), file);

		memset(&bmpInfo, 0, sizeof(bmpInfo));
		bmpInfo.biSize			= sizeof(bmpInfo);
		bmpInfo.biWidth			= width;
		bmpInfo.biHeight		= -height;
		bmpInfo.biPlanes		= 1;
		bmpInfo.biBitCount		= bitsPerPixel;
		bmpInfo.biCompression	= 0;
		bmpInfo.biSizeImage		= bmpSize;
		bmpInfo.biClrImportant	= 0;
		bmpInfo.biClrUsed		= 0;
		bmpInfo.biXPelsPerMeter = 0; // ResX*10000/254;
		bmpInfo.biYPelsPerMeter = 0; // ResY*10000/254;

		fwrite(&bmpInfo, 1, sizeof(bmpInfo), file);

		memset(bmiColors, 0, sizeof(bmiColors));
		if (bitsPerPixel==1)
		{
			if (FALSE)
			{
				bmiColors[0] = RGB(0x00,0x00,0x00);
				bmiColors[1] = RGB(0xff,0xff,0xff);
			}
			else
			{
				bmiColors[0] = RGB(0xff,0xff,0xff);
				bmiColors[1] = RGB(0x00,0x00,0x00);
			}
		}
		else
		{
			bmiColors[0] = RGB(~0x00,~0x00,~0x00);
			bmiColors[1] = RGB(~0x10,~0x10,~0x10);
			bmiColors[2] = RGB(~0x20,~0x20,~0x20);
			bmiColors[3] = RGB(~0x30,~0x30,~0x30);
			bmiColors[4] = RGB(~0x40,~0x40,~0x40);
		}

		len=fwrite(&bmiColors, 1, sizeof(bmiColors), file);
		len=fwrite(pBuffer, 1, bmpSize, file);
		fclose(file);

		return TRUE;
	}
	return FALSE;
}

