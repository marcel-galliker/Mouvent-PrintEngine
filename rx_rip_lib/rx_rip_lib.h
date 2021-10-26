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

#include "export.h"
#include "rx_xml.h"
#include "rx_bitmap.h"

//--------- layout -------------------------------------

#define MAX_BOXES			64

#define LAYOUT_CNT			1
#define BOX_TYPE_TEXT		0
#define BOX_TYPE_BARCODE	1
#define BOX_TYPE_BITMAP		2

#define BOX_MIRROR_OFF		0
#define BOX_MIRROR_HORIZ	1
#define BOX_MIRROR_VERT		2

#define DPIX	1200
#define DPIY	1200

typedef struct
{
	INT32		boxType;
	INT32		x;
	INT32		y;
	INT32		width;
	INT32		height;
	INT32		orientation;
	INT32		mirror;
	char		content[256];
} SBoxHeaderDef;

#define TA_NEW_LINE		32

typedef struct
{
	char		font[32];
	INT32		size;
	INT32		lineSpace;
	INT32		alignment;	// TA_LEFT | TA_RIGHT | TA_CENTER | TA_NEW_LINE
//	INT32		extraSpace;
} STextBoxDef;

typedef struct
{
	//--- general -----
	INT32		bcType;			// e_BarCType
	INT16		bar[8];			// bar width
	INT16		space[8];		// space width
	INT16		stretch;		// module for Matrix codes
	INT16		filter;
	INT16		codePage;

	//--- Paramaters for TEC-It Library ---------------------------------------------------------------------------------
	//-------------------------	DM			QR			MicroQR			CodeBlock	MicroPDF	PDF			Aztec
	INT16		check;		//				e_QRECLevel	
	INT16		size;		//	e_DMSizes	e_QRVersion								e_MPDFVers
	INT16		format;		//	e_DMFormat	e_QRFormat	e_MQRVersion	e_CBFFormat	e_MPDFMode	e_PDFMode	e_AztecFormat
	INT16		mask;		//	Encoding Mode	x
	INT16		rows;		//																	x
	INT16		cols;		//																	x

	INT16		ccType;
	INT16		segPerRow;	// RSS
	INT16		hex;

	//--- human readable ------------------
	char		font[32];
	INT16		fontSize;
	INT16		fontAbove;
	INT16		fontDist;
} SBarcodeBoxDef; 

typedef struct
{
	SBoxHeaderDef	hdr;
} SBitmapBoxDef;

typedef struct
{
	SBoxHeaderDef	hdr;
	STextBoxDef		text;
	SBarcodeBoxDef	bc;
} SLayoutBoxDef;

typedef struct
{
	INT32	size;
	INT32	id;
	INT32	width;		// in �m
	INT32	height;		// in �m
	INT32	webWidth;	// in �m
	INT32	columns;
	INT32	columnDist;	// in �m

	char	label[256];
	char	colorLayer[256];
	INT32	boxCnt;
	SLayoutBoxDef	box[MAX_BOXES];
} SLayoutDef;

typedef struct
{
	INT32 width;
	INT32 height;
	INT32 bitsPerPixel;
	INT32 lineLen;
} SRipBmpInfo;

#ifdef __cplusplus
extern "C"{
#endif

EXPORT int  rip_init();
EXPORT int  rip_add_fonts(char *path);

EXPORT rx_doc rip_open_xml_local(const char *path, const char *tempPath);

typedef void (*font_callback)		(const char *fontname);
EXPORT void  rip_enum_fonts(rx_doc *doc, font_callback callback);

EXPORT int  rip_load_layout(void *doc, char *tempPath, SLayoutDef *pLayout);
EXPORT int  rip_save_layout(void *doc, char *tempPath, SLayoutDef *pLayout);
EXPORT int  rip_copy_files (char *path, SLayoutDef *pLayout);

EXPORT int  rip_load_files(void* doc, char *dstFolder, char *filename);
	
EXPORT int  rip_layout_is_equal	(SLayoutDef *pLayout1, SLayoutDef *pLayout2);
EXPORT int  rip_set_layout		(SLayoutDef *pLayout, SRipBmpInfo *pInfo);
EXPORT int  rip_data			(SLayoutDef *pLayout, INT32 x, INT32 y, void *prxBmp, void *prxBmpLabel, void *prxBmpColor, int black);

EXPORT int  rip_get_bitmap	(int *width, int *height, int *bitsperpixel, int **data, int *datasize, int *stride);

EXPORT int  rip_bc_states	(int bc_type);

EXPORT int  rip_get_font_name	(int nr, char *fontname);

EXPORT int  rip_get_print_env	(const char *path, char *printEnv);

EXPORT int  rip_bmp_copy		(int *srcBmp, int srcWidth, int srcHeight, int srcStride, int srcBppx, int dist, int cnt, int *dstBmp, int dstStride);

EXPORT int  rip_test_data		(RX_Bitmap *pBmp, int testImage, int y, char *text);

#ifdef __cplusplus
}
#endif