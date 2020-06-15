// ****************************************************************************
//	RX-FreeForm.cpp
//	
//
// ****************************************************************************
//
//	Copyright 2014 by Radex AG, Switzerland. All rights reserved.
//	Written by Marcel Galliker 
//
// ****************************************************************************
//
//	http://www.freetype.org/freetype2/documentation.html
//
// ****************************************************************************

// #include <io.h>

#include <math.h>
#include "rx_common.h"
// #include "ErrorRip.h"
#include "rx_def.h"
#include "rx_error.h"
#include "rx_file.h"
#include "rx_rip_lib.h"
#include "rx_free_type.h"

#include <ft2build.h>
#include FT_FREETYPE_H
#include "ftrender.h"

//--- types -----------------------------------
typedef struct SFontDef
{
	char *filepath;
	char *fontname;
	FT_Face		font;
	SFontDef	*next;
} SFontDef;

//--- globals -------------------------------
static int			_DpiX;
static int			_DpiY;

static FT_Library   _Library;
static FT_Matrix	_Matrix[4];
static int			_Init=FALSE;

static SFontDef		*_Fonts=NULL;
static int			 _FontCnt=0;

static int			 _MonoDotSize;
static RX_Bitmap	*_ColorBmp;

//--- prototypes ---------------------------------------------------
static void _copy_bmp1(RX_Bitmap *rxbmp, int x, int y, FT_Bitmap *ftbmp, int black);
static void _copy_bmp2(RX_Bitmap *rxbmp, int x, int y, FT_Bitmap *ftbmp, int black);
static void _copy_bmp8(RX_Bitmap *rxbmp, int x, int y, FT_Bitmap *ftbmp, int black);

//--- _load_library ---------------------------------------
static void _load_library(void)
{
	if (_Library==NULL)
	{
		int error = FT_Init_FreeType( &_Library );
		if ( error ) Error(ERR_CONT, 0, "Could not load FreeType Library");		
	}		
}

//--- ft_init --------------------------------------------------
void ft_init(int dpiX, int dpiY, int monoDotSize)
{
	if (!_Init)
	{
		_Init = TRUE;

		_DpiX = dpiX;
		_DpiY = dpiY;

		_MonoDotSize = monoDotSize;

		//--- load library ---------------------------------------------------
		_load_library();
		
		#ifdef WIN32
			{
				char pathFonts[64];

				GetWindowsDirectory(pathFonts, sizeof(pathFonts));
				strcat(pathFonts, "/Fonts/");
		
				ft_enum_fonts(pathFonts);				
			}
		#endif

		//--- angles --------------
		{
			int i;
			double  angle, orientation=0;
			for (i=0; i<4; i++, orientation+=900.0)
			{
				angle = orientation*PI/1800;
				_Matrix[i].xx = (FT_Fixed)( cos( angle ) * 0x10000L );
				_Matrix[i].xy = (FT_Fixed)(-sin( angle ) * 0x10000L );
				_Matrix[i].yx = (FT_Fixed)( sin( angle ) * 0x10000L );
				_Matrix[i].yy = (FT_Fixed)( cos( angle ) * 0x10000L );
			}
		}
	}
	_ColorBmp = NULL;
}

//--- ft_end ------------------------------
void ft_end(void )
{
	while (_Fonts!=NULL)
	{
		SFontDef *font = _Fonts;
		_Fonts=_Fonts->next;
		free(font->filepath);
		free(font->fontname);
		free(font);
	}
}

//--- ft_enum_fonts -----------------------------------------------------
void ft_enum_fonts(char *path)
{
	SEARCH_Handle search;
	char			filePath[MAX_PATH];
	UINT64			time;
	UINT32			filesize, isDir;
	int				len, error;
	char			str[MAX_PATH];
	SFontDef		*item, *last;
	FT_Face			font;

	_load_library();

	search=rx_search_open(path, "*.ttf");
	len = sprintf(filePath, "%s/", path);
	while (rx_search_next(search, &filePath[len], sizeof(filePath)-len, &time, &filesize, &isDir))
	{		
		error = FT_New_Face(_Library, filePath, 0, &font);
		if (error==0)
		{
			if (font->style_flags==0)
			{
				if (strcmp(font->style_name, "Regular"))
					sprintf(str, "%s %s", font->family_name, font->style_name);
				else strcpy(str, font->family_name);

				for (item=_Fonts; item; item=item->next)
				{
					last=item;
					if (!strcmp(item->fontname, str))
					{
						if (!strncmp(item->filepath, path, len)) 
						{
							printf("Fontmatch by name");
							break;
						}
						// found
						if (item->font) FT_Done_Face(item->font);
						item->font = 0;
						free(item->filepath);
						item->filepath = strdup(filePath);
						break;
					}
				}

				if (item==NULL)
				{
					//---- new font ---
					SFontDef *fontDef = (SFontDef*)malloc(sizeof(SFontDef));
					if (fontDef==NULL) 
					{
						printf("malloc Error\n");
					}
					memset(fontDef, 0, sizeof(SFontDef));
					fontDef->filepath = strdup(filePath);
					fontDef->fontname = strdup(str);
					if (_Fonts==NULL) _Fonts=fontDef;
					else last->next = fontDef;
					_FontCnt ++;
				}
			}
			FT_Done_Face(font);
		}
	}

	rx_search_close(search);
}

//--- ft_get_font_name ---------------------------------------------
int  ft_get_font_name(int nr, char *fontname)
{
	int i;
	SFontDef *font;

	if (nr>=0 && nr<_FontCnt)
	{
		for (i=0, font=_Fonts; i<nr; i++)
		{
			font = font->next;
		}
		strcpy(fontname, font->fontname);
		return (int)strlen(font->fontname);
	}
	return 0;
}

/*
//--- FT_GetFontFileName ------------------------------------------
char *FT_GetFontFileName(char *fontname)
{
	int i=0;
	SFontDef *font;
	for (i=0, font=_Fonts; font; i++)
	{
		if (!strcmp(font->fontname, fontname))
		{
			return font->filepath;
		}
		font = font->next;
	}
	return NULL;
}
*/

//--- ft_get_font_file_path -----------------------------------
char *ft_get_font_file_path(const char *fontname)
{
	SFontDef *font;
	for (font=_Fonts; font; font = font->next)
	{
		if (!strcmp(font->fontname, fontname)) return font->filepath;
	}
	return NULL;
}

//--- ft_set_color -------------------------------
void ft_set_color(RX_Bitmap *pColor)
{
	_ColorBmp = pColor;
}

//-----------------------------------
static void _copy_bmp1(RX_Bitmap *rxbmp, int x, int y, FT_Bitmap *ftbmp, int black)
{
	int 	h, width, x0, xx;
	int     lineWidth;
	BYTE	*src;
	BYTE	*dst;
	int		w;

	if (x>=rxbmp->width) return;
	if (y>rxbmp->height) return;

	lineWidth = rxbmp->lineLen;
	src = ftbmp->buffer;	
	h=ftbmp->rows;
	if (y<0) 
	{
		src-=y*ftbmp->pitch;
		h+=y;
		y=0;
	}
	dst = rxbmp->buffer+y*lineWidth;
	if (y+h>rxbmp->height) h=rxbmp->height-y;
	if (x+ftbmp->pitch*8>rxbmp->width)	width=rxbmp->width-x;
	else								width=ftbmp->width;
	if (h<=0) return;
	x0=(x<0)?-x:0;
	for (; h; h--)
	{
		for (w=x0, xx=x+x0; w<width; w++, xx++)
		{
			if (src[w>>3] & (0x80>>(w&0x07))) 
			{
				if (black) dst[xx >> 3] |= 0x80 >> (xx & 0x07);		// black
				else	   dst[xx >> 3] &= ~(0x80 >> (xx & 0x07));	// white
			}
		}
		/*
		memcpy(dst, buf, bmp->pitch);
		buf+=bmp->pitch;
		*/
		src+=ftbmp->pitch;
		dst += lineWidth;
	}
}

//--- copy_bmp2 -------------------------------------------------------------
static void _copy_bmp2(RX_Bitmap *rxbmp, int x, int y, FT_Bitmap *ftbmp, int black)
{
	int 	h, width, x0, xx;
	int     lineWidth;
	int		dotSize;
	BYTE	*src;
	BYTE	*dst;
	int w;

	if (x>=rxbmp->width) return;
	if (y>rxbmp->height) return;

	lineWidth = rxbmp->lineLen;
	src = ftbmp->buffer;	
	h=ftbmp->rows;
	if (y<0) 
	{
		src-=y*ftbmp->pitch;
		h+=y;
		y=0;
	}
	dst = rxbmp->buffer+y*lineWidth;
	if (y+h>rxbmp->height) h=rxbmp->height-y;
	if (x+ftbmp->pitch*8>rxbmp->width)	width=rxbmp->width-x;
	else								width=ftbmp->width;
	if (h<=0) return;
	x0=(x<0)?-x:0;
	if (_ColorBmp && _ColorBmp->buffer)
	{
		dotSize = 0x03<<6;
		BYTE *color;
		BYTE *d;
		BYTE mask;		
		
		color = dst-rxbmp->buffer+_ColorBmp->buffer;
		if (h<=0) return;
		for (; h; h--)
		{
			for (w=x0, xx=x+x0; w<width; w++, xx++)
			{
				if (src[w>>3] & (0x80>>(w&0x07))) 
				{	
					mask  = dotSize >> (2*(xx&0x03));
					d  = &dst[xx>>2];
					*d = (*d&~mask) | (color[xx>>2] & mask);
				}
			}
			/*
			memcpy(dst, buf, bmp->pitch);
			buf+=bmp->pitch;
			*/
			src   += ftbmp->pitch;
			dst   += lineWidth;
			color += lineWidth; // _ColorBmp->lineLen;
		}
	}
	else
	{
		dotSize = _MonoDotSize<<6;
		for (; h; h--)
		{
			for (w=x0, xx=x+x0; w<width; w++, xx++)
			{
				if (src[w >> 3] & (0x80 >> (w & 0x07)))
				{
					if (black) dst[xx >> 2] |= dotSize >> (2*(xx & 0x03));		// black
					else	   dst[xx >> 2] &= ~(dotSize >> (2*(xx & 0x03)));	// white
				}
			}
			/*
			memcpy(dst, buf, bmp->pitch);
			buf+=bmp->pitch;
			*/
			src+=ftbmp->pitch;
			dst += lineWidth;
		}
	}
}

//--- copy_bmp8 -------------------------------------------------------------
static void _copy_bmp8(RX_Bitmap *rxbmp, int x, int y, FT_Bitmap *ftbmp, int black)
{
	int 	h, width, x0, xx;
	int     lineWidth;
	int		dotSize;
	BYTE	*src;
	BYTE	*dst;
	int w;

	if (x>=rxbmp->width) return;
	if (y>rxbmp->height) return;

	lineWidth = rxbmp->lineLen;
	src = ftbmp->buffer;	
	h=ftbmp->rows;
	if (y<0) 
	{
		src-=y*ftbmp->pitch;
		h+=y;
		y=0;
	}
	dst = rxbmp->buffer+y*lineWidth;
	if (y+h>rxbmp->height) h=rxbmp->height-y;
	if (x+ftbmp->pitch*8>rxbmp->width)	width=rxbmp->width-x;
	else								width=ftbmp->width;
	if (h<=0) return;
	x0=(x<0)?-x:0;
	if (_ColorBmp && _ColorBmp->buffer)
	{
		dotSize = 0x03<<6;
		BYTE *color;
		BYTE *d;
		BYTE mask;		
		
		Error(ERR_ABORT, 0, "_copy_bmp8: Not Implemented");

		color = dst-rxbmp->buffer+_ColorBmp->buffer;
		if (h<=0) return;
		for (; h; h--)
		{
			for (w=x0, xx=x+x0; w<width; w++, xx++)
			{
				if (src[w>>3] & (0x80>>(w&0x07))) 
				{	
					mask  = dotSize >> (2*(xx&0x03));
					d  = &dst[xx>>2];
					*d = (*d&~mask) | (color[xx>>2] & mask);
				}
			}
			/*
			memcpy(dst, buf, bmp->pitch);
			buf+=bmp->pitch;
			*/
			src   += ftbmp->pitch;
			dst   += lineWidth;
			color += lineWidth; // _ColorBmp->lineLen;
		}
	}
	else
	{
		dotSize = 0xff;
		for (; h; h--)
		{
			for (w=x0, xx=x+x0; w<width; w++, xx++)
			{
				if (src[w >> 3] & (0x80 >> (w & 0x07)))
				{
					if (black) dst[xx] = dotSize;	// black
					else	   dst[xx] = 0;			// white
				}
			}
			/*
			memcpy(dst, buf, bmp->pitch);
			buf+=bmp->pitch;
			*/
			src+=ftbmp->pitch;
			dst += lineWidth;
		}
	}
}

//--- ft_load_font -------------------------------------------
FT_Face ft_load_font(char *fontname)
{
	int			error;

	SFontDef *fnt = _Fonts;
	while(TRUE)
	{
		if (fnt==NULL) 
		{
			Error(ERR_CONT, 0, "Font >>%s<< not found", fontname);
			return NULL;
		}
		if (!strcmp(fontname, fnt->fontname)) break;
		fnt=fnt->next;
	}

	if (!fnt->font) 
	{
		error = FT_New_Face(_Library, fnt->filepath, 0, &fnt->font);
		if (error == FT_Err_Unknown_File_Format) Error(ERR_CONT, 0, "Unknown Font File Format >>%s<<", fontname);
		else if (error)						     Error(ERR_CONT, 0, "Cound not load font >>%s<<", fontname);
	}
	return fnt->font;
}


//--- ft_free_font --------------------------------------------------
void ft_free_font(FT_Face font)
{
	
}

//--- ft_height ---------------------------------------------------
int ft_height(FT_Face font, int size)
{
	if (size!=0)
	{
		FT_Set_Char_Size( font,		// handle to face object
							0,			// char_width in 1/64th of points
							size,		// char_height in 1/64th of points
							_DpiX,		// horizontal device resolution
							_DpiY );	// vertical device
	}
	if (font==NULL) return 0;
	return font->size->metrics.ascender/64;
}

//--- ft_char_width ---------------------------------------------
int  ft_char_width(FT_Face font)
{
	if (font) return font->size->metrics.max_advance/64;
	else return 0;
}

//--- ft_text_out -------------------------------------------
void ft_text_out(RX_Bitmap *pBmp, int x, int y, FT_Face font, int size, int orientation, UINT16 *text, int len, int black)
{
	int error=0;
	if (font==NULL) { Error(ERR_CONT, 0, "Font not defined"); return;}

	FT_GlyphSlot  	slot = font->glyph;
	FT_UInt  		glyph_index;
	int				idx1=-1;
	int				ascender;
	FT_Matrix		*matrix = &_Matrix[orientation/900];

	FT_Set_Char_Size( font,		// handle to face object
						0,			// char_width in 1/64th of points
						size,		// char_height in 1/64th of points
						_DpiX,		// horizontal device resolution
						_DpiY );	// vertical device

	FT_Set_Transform(font, matrix, 0);

	ascender=font->size->metrics.ascender;
	//--- compensate baseline ---
	x -= (int)(ascender*matrix->xy/64/0x10000L);
	y += (int)(ascender*matrix->yy/64/0x10000L);
	
	for (; *text && len--; text++)
	{
		//--- load character -------------------------------
		glyph_index = FT_Get_Char_Index(font, *text );

		error = FT_Load_Glyph(font, glyph_index, FT_LOAD_DEFAULT );
		if ( error ) continue;

		error = FT_Render_Glyph(font->glyph, FT_RENDER_MODE_MONO );
		if ( error ) continue;

		//--- kerning -------------------------------------------
		{
			FT_Vector  		kerning;
			if (idx1>=0 && !FT_Get_Kerning(font, idx1, glyph_index, FT_KERNING_DEFAULT, &kerning))
			{
				x += (kerning.x>>6)*matrix->xx/0x10000L;
				y -= (kerning.x>>6)*matrix->yx/0x10000L;
			}
			idx1 = glyph_index;
		}

		//--- draw the bitmap -------------------------------
		if (pBmp->bppx==1) _copy_bmp1(pBmp, x + slot->bitmap_left, y - slot->bitmap_top, &slot->bitmap, black);
		else if (pBmp->bppx==2)	_copy_bmp2(pBmp, x + slot->bitmap_left, y - slot->bitmap_top, &slot->bitmap, black);
		else if (pBmp->bppx==8) _copy_bmp8(pBmp, x + slot->bitmap_left, y - slot->bitmap_top, &slot->bitmap, black);
		
		//--- increment position ----------------------------
		x += slot->advance.x >> 6; 
		y -= slot->advance.y >> 6; 
	}
}

//--- ft_text_out -------------------------------------------
///
/// Custom method to handle text alignment
/// Create by Samuel de Santis
///
void ft_text_out(
	RX_Bitmap *pBmp,
	FT_Vector  origin,
	FT_Pos     box_width,
	FT_Pos     box_height,
	FT_Face    face,
	USHORT     alignment,
	int        size,
	int        rotation,		// rotation applied to the text in deca-degree (0, 900, 1800, 2700)
	UINT16    *text,			// pointer to a null-terminated wide character array
	size_t     max_text_length, // maximum number of wide characters to copy
	int        black)
{
	if (face == NULL)
	{
		Error(ERR_CONT, 0, "Face not defined"); return;
	}

	FT_Error     error = 0;
	FT_Pos       x     = origin.x;
	FT_Pos       y     = origin.y;
	FT_GlyphSlot slot  = face->glyph;
	FT_UInt  		glyph_index;
	int				idx1 = -1;
	int				ascender;
	FT_Matrix		*matrix = &_Matrix[rotation / 900];

	FT_Set_Char_Size(
		face,		// handle to face object
		0,			// char_width in 1/64th of points
		size,		// char_height in 1/64th of points
		_DpiX,		// horizontal device resolution
		_DpiY);		// vertical device

	FT_Set_Transform(face, matrix, 0);

	ascender = face->size->metrics.ascender;

	//--- compensate baseline ---
	x -= (int)(ascender*matrix->xy / 64 / 0x10000L);
	y += (int)(ascender*matrix->yy / 64 / 0x10000L);

	// Handle alignement.
	// We have to compute the new origin (x,y) of the text depending
	// on the alignment and rotation applied.
	// By default the text is aligned to the (top-)left corner.
	if (alignment != TA_LEFT)
	{
		FT_UInt   glyph_index;
		FT_Pos    text_width = 0;
		UINT16   *text_it = text;

		for (size_t i = 0; (*text_it != L'\0') && (i < max_text_length); ++i, ++text_it)
		{
			glyph_index = FT_Get_Char_Index(face, *text_it);

			// Load the glyph image in the normal way into the face's glyph slot.
			error = FT_Load_Glyph(face, glyph_index, FT_LOAD_DEFAULT);
			if (error) { continue; }

			// Compute kerning.
			{
				FT_Vector		kerning;
				if (idx1 >= 0 && !FT_Get_Kerning(face, idx1, glyph_index, FT_KERNING_DEFAULT, &kerning))
				{
					x += (kerning.x >> 6)*matrix->xx / 0x10000L;
					y -= (kerning.x >> 6)*matrix->yx / 0x10000L;
				}
				idx1 = glyph_index;
			}

			FT_Pos char_width = rotation == 900 || rotation == 2700 ? slot->advance.y >> 6 : slot->advance.x >> 6;
			text_width += char_width;
		}

		// If the text direction is right to left the advance is negative.
		// So, we end up
		text_width = abs(text_width);

		switch (rotation)
		{
		case 900:
			if (alignment == TA_RIGHT)
			{
				y -= (box_height - text_width);
			}
			else if (alignment == TA_CENTER)
			{
				y -= (box_height - text_width) / 2;
			}
			break;
		case 1800:
			if (alignment == TA_RIGHT)
			{
				x -= (box_width - text_width);
			}
			else if (alignment == TA_CENTER)
			{
				x -= (box_width - text_width) / 2;
			}
			break;
		case 2700:
			if (alignment == TA_RIGHT)
			{
				y += (box_height - text_width);
			}
			else if (alignment == TA_CENTER)
			{
				y += (box_height - text_width) / 2;
			}
			break;
		default:
			if (alignment == TA_RIGHT)
			{
				x += (box_width - text_width);
			}
			else if (alignment == TA_CENTER)
			{
				x += (box_width - text_width) / 2;
			}
			break;
		}
	}
	///////////////////////////////////////////////////////////////////////////////////////////



	for (; *text && max_text_length--; text++)
	{
		//--- load character -------------------------------
		glyph_index = FT_Get_Char_Index(face, *text);

		error = FT_Load_Glyph(face, glyph_index, FT_LOAD_DEFAULT);
		if (error) continue;

		error = FT_Render_Glyph(face->glyph, FT_RENDER_MODE_MONO);
		if (error) continue;

		//--- kerning -------------------------------------------
		{
			FT_Vector  		kerning;
			if (idx1 >= 0 && !FT_Get_Kerning(face, idx1, glyph_index, FT_KERNING_DEFAULT, &kerning))
			{
				x += (kerning.x >> 6)*matrix->xx / 0x10000L;
				y -= (kerning.x >> 6)*matrix->yx / 0x10000L;
			}
			idx1 = glyph_index;
		}

		//--- draw the bitmap -------------------------------
		if (pBmp->bppx == 1) _copy_bmp1(pBmp, x + slot->bitmap_left, y - slot->bitmap_top, &slot->bitmap, black);
		else if (pBmp->bppx == 2)	_copy_bmp2(pBmp, x + slot->bitmap_left, y - slot->bitmap_top, &slot->bitmap, black);
		else if (pBmp->bppx == 8)	_copy_bmp8(pBmp, x + slot->bitmap_left, y - slot->bitmap_top, &slot->bitmap, black);

		//--- increment position ----------------------------
		x += slot->advance.x >> 6;
		y -= slot->advance.y >> 6;
	}
}
