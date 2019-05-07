// ****************************************************************************
//	FreeForm.h
//	
//
// ****************************************************************************
//
//	Copyright 2014 by Radex AG, Switzerland. All rights reserved.
//	Written by Marcel Galliker 
//
// ****************************************************************************

#pragma once

#include "rx_common.h"
#include "rx_bitmap.h"
#include <ft2build.h>
#include FT_FREETYPE_H

#ifdef linux
	//--- source: wingdi.h ------------------------
	#define TA_LEFT                     0
	#define TA_RIGHT                    2
	#define TA_CENTER                   6
#endif

void	ft_init					(int dpiX, int dpiY, int monoDotSize);
void	ft_end					(void );
void	ft_enum_fonts			(char *path);
int		ft_get_font_name		(int nr, char *fontname);
char*	ft_get_font_file_path	(const char *fontname);

void	ft_set_color			(RX_Bitmap *pColor);

FT_Face ft_load_font			(char *fontname);
void    ft_free_font			(FT_Face font);
int		ft_height				(FT_Face font, int size);
int		ft_char_width			(FT_Face font);
void	ft_text_out				(RX_Bitmap *pBmp, int x, int y, FT_Face font, int size, int orientation, UINT16 *text, int len, int black);
void	ft_text_out				(RX_Bitmap *pBmp, FT_Vector origin, FT_Pos box_width, FT_Pos box_height, FT_Face face, USHORT alignment, int size, int rotation, UINT16 *text, size_t max_text_length, int black);