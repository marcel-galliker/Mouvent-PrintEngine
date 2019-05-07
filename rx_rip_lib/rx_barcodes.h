#ifndef _TEC_IT_BARCODES_H
#define _TEC_IT_BARCODES_H

// ****************************************************************************
//
//	Foreground: -- MIC NT --  file: TEC_ITBarcodes_defh  
//	
// ****************************************************************************
//
//	Copyright 2013 by Radex AG, Switzerland. All rights reserved.
//	Written by Marcel Gallliker
//
// ****************************************************************************

#include "rx_bitmap.h"
#include "TCP_IP_OEM.h"
// #include "WTypes.h"
#include "tbarcode.h"

//--------- DEFINES -----------------------------------------------------------------------------

typedef HANDLE	HBarCode;

//--- bc_init_box -------------------------------
HBarCode*	bc_init_box(SBarcodeBox *pBox, int boxNo, int resx, int resy);
//	allocates and initializes barcode object

//--- bc_release_box --------------------------
void		bc_release_box(HBarCode pBarCode);
// releases a barcode object

int			bc_states(e_BarCType bcType);


//--- bc_draw_barcode ------------------
int			bc_draw_code	(RX_Bitmap *pBmp, int x0, int y0, SBarcodeBox *pBox, HBarCode pBarCode, RECT *rect, const UTF16 *code, int codelen);
void		bc_draw_bitmap	(RX_Bitmap *pBmp, int x0, int y0, SBarcodeBox *pBox, HBarCode pBarCode, int dropSize, RX_Bitmap *pColor);

// draws the barcode

#endif
