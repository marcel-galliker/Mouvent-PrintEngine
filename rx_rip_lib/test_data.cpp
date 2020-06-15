// ****************************************************************************
//
//	test_data.cpp
//
// ****************************************************************************
//
//	Copyright 2017 by Radex AG, Switzerland. All rights reserved.
//	Written by Marcel Galliker 
//
// ****************************************************************************
 
//--- Include -----------------------------------------------------------------
#include "stdafx.h"
#include "rx_common.h"
#include "rx_def.h"
#include <stdlib.h>
#include <malloc.h>
#include <wchar.h>
// #include "ErrorRip.h"
#include "rx_barcodes.h"
#include "rx_free_type.h"
#include "TCP_IP_OEM.h"
#include "rip.h"
#include "rx_rip_lib.h"


//--- defines -----------------------------------------------------------------
#define FONT_NAME	"Telex"
#define FONT_SIZE	450
#define LINE_DIST	150
#define FONT_SIZE_SMALL	250
#define LINE_DIST_SMALL	100
#define X			130

 //--- rip_test_data ----------------------------------------------------------
int  rip_test_data	(RX_Bitmap *pBmp, int testImage, char *text)
{
	FT_Face font;
	int y, n, len;
	char *line;
	UINT16 wstr[100];

	ft_init(DPIX, DPIY, 1);
	ft_enum_fonts((char*)PATH_BIN_SPOOLER);

	font = ft_load_font((char*)FONT_NAME);
	y=0;
	n=0;
	line=text;
	len=0;
	while (*line)
	{
		if (!line[len]) break;
		if (line[len]=='\n')
		{
			bmp_clear(pBmp, X, y, 2048+128, LINE_DIST);

			n++;
			len++;
			#ifdef linux
//				char_to_wchar32((const char*)line, (wchar_t*)wstr, len);
				char_to_wchar((const char*)line, (wchar_t*)wstr, len);
			#else
				char_to_wchar((const char*)line, (wchar_t*)wstr, len);
			#endif
			if (testImage==PQ_TEST_DENSITY && n>6)
			{
				ft_text_out(pBmp, X, y, font, FONT_SIZE_SMALL, 0, wstr, len, 1);	// changed interface if wstr from wchar_t to UINT16
				y += LINE_DIST_SMALL;
			}
			else 
			{
			ft_text_out(pBmp, X, y, font, FONT_SIZE, 0, wstr, len, 1);	// changed interface if wstr from wchar_t to UINT16
			y += LINE_DIST;
			}
			line += len;
			len=0;
		}
		else len++;
	}
	return REPLY_OK;
}

