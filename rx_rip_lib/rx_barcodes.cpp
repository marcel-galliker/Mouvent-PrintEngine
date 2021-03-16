// ****************************************************************************
//
//	Foreground: -- MIC NT --  file: rx_barcodes.c  
//	
// ****************************************************************************
//
//	Copyright 2013 by Radex AG, Switzerland. All rights reserved.
//	Written by Marcel Gallliker
//
// ****************************************************************************

//--- Include -----------------------------------------------------------------
#include "stdafx.h"
#include <locale.h>
#include <malloc.h>
#include <stdio.h>
#include "math.h"
#include "rx_common.h"

#include "rx_free_type.h"
#include "rx_barcodes.h"
#ifdef JOB_COMPOSER
	#include "ErrorRip.h"
#else
	#include "rx_error.h"
#endif

//--- LICENSING --------------------------------------
#define TEC_IT_LICENSEE		"Mem: Radex AG CH-2540"
#ifdef WIN32
	#define TEC_IT_KEY_11		"654B7D6225F04C44BA52DAF657F3E047"
#else
	#define TEC_IT_KEY_11		"654B7D6225F04CA45A523AF65713E0A7"
	#define TEC_IT_LIB_PATH		"/usr/local/share/tbarcode11"
#endif

typedef struct
{
	int			boxNo;
	t_BarCode*	pTBarCode;
	double		stretch;
	RECT		rect;
	RECT		rect_draw;
	int			orient;
	int			mirror;
	int			code_2d;
	int			states;
	int			fontHeightPx;
	int			row;
	int			rows;
	int			cols;
	int			dataSize;
	char		*mdata;
	FT_Face		font;
	RX_Bitmap	*pBmp;
} SBarCode;

static int		_ErrorSet;
static int		_DpiX;
static int		_DpiY;

//--- Functions ---------------------------------------------------------------
static ERRCODE CALLBACK _custom_draw_row (VOID* customData, t_BarCode* barcode, HDC drawDC, HDC targetDC, DOUBLE x, DOUBLE y, DOUBLE width, DOUBLE height);
static ERRCODE CALLBACK _custom_draw_text(VOID *customData, HDC drawDC, LPLOGFONTA font, DOUBLE x, DOUBLE y, e_BCAlign alignment, LPCSTR text, INT numberOfCharacters, LPRECTD drawArea, LONG textIndex);
static int _bc_is2D_scaling(e_BarCType type);
static void _draw_bitmap_2D     (RX_Bitmap *pBmp, int par_x, int par_y, SBarcodeBox *pBox, HBarCode hBarCode, int dropSize, RECT *prect);
static void _draw_bitmap_1D_000 (RX_Bitmap *pBmp, int par_x, int par_y, SBarcodeBox *pBox, HBarCode hBarCode, int dropSize);
static void _draw_bitmap_1D_090 (RX_Bitmap *pBmp, int par_x, int par_y, SBarcodeBox *pBox, HBarCode hBarCode, int dropSize);
static void _draw_bitmap_1D_180 (RX_Bitmap *pBmp, int par_x, int par_y, SBarcodeBox *pBox, HBarCode hBarCode, int dropSize);
static void _draw_bitmap_1D_270 (RX_Bitmap *pBmp, int par_x, int par_y, SBarcodeBox *pBox, HBarCode hBarCode, int dropSize);
static void _colorize_box			(RX_Bitmap *pBmp, SBarcodeBox *pBox, RX_Bitmap *pColor, RECT *prect);

//---bc_init_box -------------------------------------------
HBarCode* bc_init_box(SBarcodeBox *pBox, int boxNo, int resx, int resy)
{
	static int init = FALSE;

	SBarCode* pBarCode;
	int		ret;
	char	ratio [64];
	char	mode[64];
	int		i, len;
	ERRCODE	eCode;

	#ifdef linux
	if (sizeof(LOGFONTA)!=80)
	{
		Error(ERR_STOP, 0, "Sizeof(DECDataTypes.LOGFONTA) must be 80!!!!");
		return NULL;
	}
	if (sizeof(RECT)!=32)
	{
		Error(ERR_STOP, 0, "Sizeof(DECDataTypes.RECT) must be 32!!!!");
		return NULL;
	}
	#endif

	if (!init)
	{
		#ifdef linux
			BCInitLibrary(TEC_IT_LIB_PATH); 
		#endif
		eCode = BCLicenseMeA (TEC_IT_LICENSEE, eLicKindDeveloper, 1, TEC_IT_KEY_11, eLicProd2D);
		if (eCode) 
		{	
			Error(ERR_CONT, 600, "TEC-IT: Licence Key Error"); 
			return NULL;
		}
	}
	init = TRUE;
	_ErrorSet = FALSE;
	
	if (pBox->stretch && pBox->filter >= pBox->stretch && pBox->bcType!=eBC_MAXICODE) 
	{
		Error(ERR_CONT, 600, "Barcode Box %d: Filter larger than stretch factor", boxNo+1); 
		return NULL;
	}
	_DpiX = resx;
	_DpiY = resy;
	pBarCode = (SBarCode*)malloc(sizeof(SBarCode));
	if (pBarCode)
	{
		memset(pBarCode, 0, sizeof(SBarCode));
		pBarCode->boxNo = boxNo;
		BCAlloc (&pBarCode->pTBarCode);
		if (pBarCode)
		{
			ret = BCSetBCType		(pBarCode->pTBarCode, (e_BarCType)pBox->bcType);
			ret = BCSetRotation		(pBarCode->pTBarCode, (e_Degree)pBox->orientation);
			ret = BCSetOptResolution(pBarCode->pTBarCode, FALSE);
			if (pBox->font.lfHeight)
			{
				ret = BCSetLogFont		(pBarCode->pTBarCode, &pBox->font);
				ret = BCSetPrintText	(pBarCode->pTBarCode, TRUE, pBox->above);
				ret = BCSetFuncDrawText	(pBarCode->pTBarCode, _custom_draw_text);
			}
			else BCSetPrintText	(pBarCode->pTBarCode, FALSE, 0);

			ret=BCSetCDMethod  (pBarCode->pTBarCode, (e_CDMethod)pBox->check);
		
			//--- ratio string ---
			if (pBox->stretch>1)
			{
				ret = BCSetModWidthN (pBarCode->pTBarCode, 265*pBox->stretch);
			}
			else
			{
				len = 0;
				memset(ratio, 0, sizeof(ratio));
				for (i=0; i<BCGetCountBars((e_BarCType)pBox->bcType); i++)
				{
					len += sprintf(&ratio[len], "%d:", pBox->bar[i]);
				}
				for (i=0; i<BCGetCountSpaces((e_BarCType)pBox->bcType); i++)
				{
					len += sprintf(&ratio[len], "%d:", pBox->space[i]);
				}
				if (len>0)
				{
					ratio[len-1]=0;
					ret=BCSetModWidthN (pBarCode->pTBarCode, 265);
					ret=BCSetRatioA (pBarCode->pTBarCode, ratio);
				}
			}

			pBarCode->orient		= pBox->orientation;
			pBarCode->mirror		= pBox->hdr.mirror;
			switch (pBox->bcType) {
			case eBC_PDF417:	
			case eBC_PDF417Trunc:		if (pBox->rows > 0)	BCSet_PDF417_Rows	 (pBarCode->pTBarCode, pBox->rows);
										else				BCSet_PDF417_Rows	 (pBarCode->pTBarCode, -1);	// default: calculated
										if (pBox->cols > 0)	BCSet_PDF417_Columns (pBarCode->pTBarCode, pBox->cols);
										else				BCSet_PDF417_Columns (pBarCode->pTBarCode, -1);
										BCSet_PDF417_EncodingMode (pBarCode->pTBarCode,(e_PDFMode)pBox->format);
										break;

			case eBC_QRCode:		//	SetMapMode(*dc, MM_HIMETRIC);
										BCSet_QR_Version		(pBarCode->pTBarCode, (e_QRVersion)pBox->size);
										BCSet_QR_Format			(pBarCode->pTBarCode, (e_QRFormat)pBox->format);
										BCSetCDMethod			(pBarCode->pTBarCode, eCDStandard);
										BCSet_QR_ECLevel		(pBarCode->pTBarCode, (e_QRECLevel)pBox->check);
										BCSet_QR_Mask			(pBarCode->pTBarCode, (e_QRMask)pBox->mask);
										BCSetFuncDrawRow		(pBarCode->pTBarCode, _custom_draw_row);
										BCSetCBData				(pBarCode->pTBarCode, pBarCode);
										BCSetRotation			(pBarCode->pTBarCode, deg0);
										break;

			case eBC_MicroQRCode:		BCSet_MQR_Version		(pBarCode->pTBarCode, (e_MQRVersion)pBox->size);
										BCSet_MQR_Mask			(pBarCode->pTBarCode,  (e_MQRMask)pBox->mask);
										BCSetFuncDrawRow		(pBarCode->pTBarCode, _custom_draw_row);
										BCSetCBData				(pBarCode->pTBarCode, pBarCode);
										BCSetRotation			(pBarCode->pTBarCode, deg0);
										break;

			case eBC_Aztec:				ret=BCSet_Aztec_RuneMode			 (pBarCode->pTBarCode, 0);
										ret=BCSet_Aztec_EnforceBinaryEncoding(pBarCode->pTBarCode, 0);
										ret=BCSet_Aztec_FormatW	( pBarCode->pTBarCode, (e_AztecFormat)pBox->format, L"");
										if ( pBox->check > 0)	ret=BCSet_Aztec_ErrorCorrection( pBarCode->pTBarCode, pBox->check);
										else					ret=BCSet_Aztec_ErrorCorrection( pBarCode->pTBarCode, 23);
										ret=BCSet_Aztec_Size	(pBarCode->pTBarCode, (e_AztecSizes)pBox->size);
										BCSetFuncDrawRow		(pBarCode->pTBarCode, _custom_draw_row);
										BCSetCBData				(pBarCode->pTBarCode, pBarCode);
										BCSetRotation			(pBarCode->pTBarCode, deg0);
										break;

			case eBC_DataMatrix:		BCSet_DM_Rectangular	(pBarCode->pTBarCode, pBox->size > eDMSz_144x144);
										BCSet_DM_Size			(pBarCode->pTBarCode, (e_DMSizes)pBox->size);
										BCSet_DM_Format			(pBarCode->pTBarCode, (e_DMFormat)pBox->format);
										BCSetFuncDrawRow		(pBarCode->pTBarCode, _custom_draw_row);
										BCSetCBData				(pBarCode->pTBarCode, pBarCode);
										BCSetRotation			(pBarCode->pTBarCode, deg0);
										sprintf(mode, "DataMatrix_EncodingMode=%d", pBox->mask);
										BCSetOptionsA(pBarCode->pTBarCode, mode);
										break;

			case eBC_MicroPDF417:		BCSet_MPDF417_Version	(pBarCode->pTBarCode, (e_MPDFVers)pBox->size);
										BCSet_MPDF417_Mode		(pBarCode->pTBarCode, (e_MPDFMode)pBox->format);
										break;

			case eBC_MAXICODE:			BCSet_Maxi_Mode(pBarCode->pTBarCode, pBox->format);
										BCSet_Maxi_UnderCut(pBarCode->pTBarCode, pBox->filter);
										break;

			default:					break;
			}
			pBarCode->states = bc_states((e_BarCType)pBox->bcType);

			BCSetFuncDrawRow		(pBarCode->pTBarCode, _custom_draw_row);
			BCSetCBData				(pBarCode->pTBarCode, pBarCode);

			if (pBox->wchar)
			{
				if (pBox->codePage>0)
				{
					BCSetCodePageCustom(pBarCode->pTBarCode, pBox->codePage);
					BCSetCodePage(pBarCode->pTBarCode, eCodePage_Custom);
					BCSetEncodingMode(pBarCode->pTBarCode, eEncMode_CodePage);
				}
				else
				{
					BCSetCodePage(pBarCode->pTBarCode, eCodePage_Default);
					BCSetEncodingMode(pBarCode->pTBarCode, eEncMode_CodePage);
				}
			}
			else
			{
				BCSetEncodingMode(pBarCode->pTBarCode, eEncMode_LowByte);	// this was HEX!
			}

			ret = BCIsCompositeAllowed ((e_BarCType)pBox->bcType);
			if (ret)	BCSet2DCompositeComponent (pBarCode->pTBarCode, (e_CCType)pBox->ccType);
			else		BCSet2DCompositeComponent (pBarCode->pTBarCode, eCC_None);

			if (pBox->bcType==eBC_RSSExpStacked)
				BCSet_RSS_SegmPerRow(pBarCode->pTBarCode, pBox->segPerRow);

			//--- special: asymetric 2D codes --------------------------------------------
			strcpy(ratio, BCGetRatioString((e_BarCType)pBox->bcType));
			pBarCode->stretch = 1;
			pBarCode->rect    = pBox->hdr.rect;
			/*
			if (resx!=resy && _bc_is2D_scaling((e_BarCType)pBox->bcType)) //(!ratio[0] || !strcmp(ratio, "1:1") || pBox->bcType==eBC_MAXICODE) && !BC_is4State(pBox->bcType))
			{
				pBarCode->stretch		= (double)resy/(double)resx;
				pBarCode->rect.top		= 0;
				pBarCode->rect.left		= 0;
				pBarCode->rect.right	= (int)(pBarCode->stretch*(pBox->hdr.rect.right-pBox->hdr.rect.left));
				pBarCode->rect.bottom	= pBox->hdr.rect.bottom-pBox->hdr.rect.top;
				if (Rect.right	<pBarCode->rect.right)	Rect.right	= pBarCode->rect.right;
				if (Rect.bottom <pBarCode->rect.bottom) Rect.bottom = pBarCode->rect.bottom;
			}
			*/
		}
	}
	return (HBarCode*)pBarCode;
}

//--- bc_release_box --------------------------
void bc_release_box(HBarCode hBarCode)
{
	SBarCode* pBarCode = (SBarCode*)hBarCode;
	if (pBarCode)
	{
		BCFree(pBarCode->pTBarCode);
		if (pBarCode->mdata) free(pBarCode->mdata);
		if (pBarCode->font)	 ft_free_font(pBarCode->font);
		free(pBarCode);
	}
}

//--- bc_states ------------------------------
int	bc_states(e_BarCType bcType)
{
	switch (bcType)
	{
		case eBC_USPSPostNet5:
		case eBC_USPSPostNet6:
		case eBC_USPSPostNet9:
		case eBC_USPSPostNet10:
		case eBC_USPSPostNet11:
		case eBC_USPSPostNet12:
		case eBC_Plessey:
		case eBC_CEPNet:
		case eBC_Planet12:
		case eBC_Planet14:			return 2;

		case eBC_Pharma2:			return 3;

		case eBC_AusPostCustom:
		case eBC_AusPostCustom2:
		case eBC_AusPostCustom3:
		case eBC_AusPostReplyPaid:
		case eBC_AusPostRouting:
		case eBC_AusPostRedirect:
		case eBC_RM4SCC:
		case eBC_KIX:
		case eBC_USPSIMPackage:
		case eBC_USPSOneCode4CB:
	//	case eBC_USPSIntelligentMail:
		case eBC_DAFT:
		case eBC_JapanesePostal:	return 4;

		default:					return 0;
	}
}

//--- _bc_is2D_scaling ---------------------------------------------------------------------
static int _bc_is2D_scaling(e_BarCType type)
{
	int bars = BCGetCountBars(type);
	int spaces = BCGetCountSpaces(type);
	if (BCGetCountBars(type)<2 && BCGetCountSpaces(type)<2)
	{
		switch (type)
		{
		case eBC_Flattermarken:
		case eBC_USPSPostNet5:
		case eBC_USPSPostNet6:
		case eBC_USPSPostNet10:
		case eBC_USPSPostNet11:
		case eBC_USPSPostNet12:
		case eBC_Pharma2:
		case eBC_CEPNet:
		case eBC_AusPostCustom:
		case eBC_AusPostCustom2:
		case eBC_AusPostCustom3:
		case eBC_AusPostReplyPaid:
		case eBC_AusPostRouting:
		case eBC_AusPostRedirect:
		case eBC_RM4SCC:
		case eBC_JapanesePostal:
		case eBC_Planet12:
		case eBC_Planet14:
		case eBC_USPSOneCode4CB:
		case eBC_KIX:
		case eBC_DAFT:
		// eBC_DotCode: is 2D:
		// eBC_HanXin: is 2D
		return FALSE;
		}
		return TRUE;
	}
	return FALSE;
}

//--- bc_draw_code ---------------------------------------------------
ERRCODE bc_draw_code(RX_Bitmap *pBmp, int x0, int y0, SBarcodeBox *pBox, HBarCode hBarCode, RECT *rect, const UTF16 *code, int codeLen)
{
	SBarCode*	pBarCode = (SBarCode*)hBarCode;
	ERRCODE		eCode;

	eCode = S_OK;

	pBarCode->rect_draw.left	= x0+rect->left;
	pBarCode->rect_draw.right	= x0+rect->right;
	pBarCode->rect_draw.top		= y0+rect->top;
	pBarCode->rect_draw.bottom	= y0+rect->bottom;

	// Error(LOG, 0, L"Barcode >>%s<<, len=%d", code, codeLen);
	if (codeLen && *code)
	{
		wchar_t *wcode = (wchar_t*)malloc((codeLen/2+1)*sizeof(wchar_t));

		if (pBox->bcType==eBC_Code128 || pBox->bcType==eBC_Code128A || pBox->bcType==eBC_Code128B || pBox->bcType==eBC_Code128C)
		{
			const UTF16 *ch, *end;
			char	str[64];
			char	format[64];
			int		manual=FALSE;
			int		l, fl;

			memset(format, 0, sizeof(format));
			fl=l = 0;
			for(ch=code, end=&code[codeLen/2]; ch<end; ch++)
			{
				if (*ch==(UTF16)'/' && (ch[1]==(UTF16)'A' || ch[1]==(UTF16)'a'))		
				{
					ch++;
					manual = TRUE;
					format[fl++]='A';
				}
				else if (*ch==(UTF16)'/' && (ch[1]==(UTF16)'B' || ch[1]==(UTF16)'b'))	
				{
					ch++;
					manual = TRUE;
					format[fl++]='B';
				}
				else if (*ch==(UTF16)'/' && (ch[1]==(UTF16)'C' || ch[1]==(UTF16)'c')) 
				{
					ch++;
					manual = TRUE;
					format[fl++]='C';
				}
				else
				{ 
					format[fl++]='#';
					str[l++]=(char)*ch;
				}
			}
			if (manual)
			{
				str[l] = 0;
				BCSetFormatA(pBarCode->pTBarCode, format);
				eCode=BCSetTextA(pBarCode->pTBarCode, str, l);
			}
			else
			{
				BCSetFormatA(pBarCode->pTBarCode, NULL);
				utf16_to_wchar(code, wcode, codeLen/2+1);
				eCode=BCSetTextW(pBarCode->pTBarCode, wcode, codeLen/2);
			}
		}
		else
		{
			if (pBox->wchar)
			{
				utf16_to_wchar(code, wcode, codeLen/2+1);
				eCode = BCSetTextW(pBarCode->pTBarCode, wcode, codeLen/2);
			}
			else eCode = BCSetTextA(pBarCode->pTBarCode, (char*)code, codeLen);
		}

		if (eCode == S_OK) eCode = BCCheck	(pBarCode->pTBarCode);     // check bar code parameters (MUST)
		if (eCode == S_OK) eCode = BCCalcCD	(pBarCode->pTBarCode);     // calculate check digit (MUST)
		if (eCode == S_OK) eCode = BCCreate	(pBarCode->pTBarCode);     // create internal bar code structure (MUST)

		if (eCode == S_OK)
		{
			if (BCGetFuncDrawRow(pBarCode->pTBarCode))	
			{
				BCSetModWidthN  (pBarCode->pTBarCode, 265*1);
				pBarCode->pBmp = pBmp;
				eCode = BCDrawCB(pBarCode->pTBarCode, NULL, &pBarCode->rect_draw);
			}
		}
		free(wcode);
	}
	return (eCode);
}

//--- _custom_draw_row -------------------------------------------------------------
ERRCODE CALLBACK _custom_draw_row(VOID* customData, t_BarCode* barcode, HDC drawDC, HDC targetDC, DOUBLE x, DOUBLE y, DOUBLE width, DOUBLE height)
{
	SBarCode*	pBarCode = (SBarCode*)customData;
	char *src, *dst;
	int test;

	src = (char*)BCGetMetaDataA(barcode);
	if (pBarCode->row==0)
	{
		int rows, cols;
		rows = (int)BCGet2D_XRows(barcode);
		if (rows>0) 
		{
			cols = BCGet2D_XCols(barcode);	
			pBarCode->code_2d=TRUE;
		}
		else
		{
			pBarCode->code_2d=FALSE;
			rows = BCGetCountRows(barcode);
			cols = (int)strlen(src);
		}

		if (pBarCode->orient==deg0 || pBarCode->orient==deg180)
		{
			pBarCode->rows = rows;
			pBarCode->cols = cols;
		}
		else
		{
			pBarCode->rows = cols;
			pBarCode->cols = rows;
		}
		
		if (pBarCode->rows * pBarCode->cols>pBarCode->dataSize) 
		{
			free(pBarCode->mdata);
			pBarCode->mdata=NULL;
		}
		if (!pBarCode->mdata) 
		{
			pBarCode->dataSize=2*pBarCode->rows * pBarCode->cols;
			pBarCode->mdata = (char*)malloc (pBarCode->dataSize);
		}
		memset(pBarCode->mdata, 0, pBarCode->dataSize);
	}

	if (pBarCode->code_2d)
	{
		switch(pBarCode->orient)
		{
		case deg90:		dst=pBarCode->mdata+(pBarCode->rows-1)*(pBarCode->cols+1)+pBarCode->row;
						while (*src)
						{
							*dst = *src++;
							dst-=(pBarCode->cols+1);
						}
						break;

		case deg180:	test = (int)strlen(src);
						dst=pBarCode->mdata+(pBarCode->rows-pBarCode->row-1)*(pBarCode->cols+1)+pBarCode->cols-1;
						while (*src) 
						{
							*dst-- = *src++;
							test--;
						}
						test=(int)strlen(pBarCode->mdata);
						break;

		case deg270:	dst=pBarCode->mdata+pBarCode->rows-pBarCode->row-1;
						while (*src) 
						{
							*dst = *src++;
							dst += (pBarCode->cols+1);
						}
						break;

		default:		memcpy(pBarCode->mdata+pBarCode->row*(pBarCode->cols+1), src, pBarCode->cols+1);
		}

	}
	else memcpy(pBarCode->mdata, src, pBarCode->dataSize);
	pBarCode->row++;

	return ErrOk;
}

//--- _custom_draw_text --------------------------------------------------------------------------------
ERRCODE CALLBACK _custom_draw_text(VOID *customData, HDC drawDC, LPLOGFONTA font, DOUBLE x, DOUBLE y, e_BCAlign alignment, LPCSTR text, INT numberOfCharacters, LPRECTD drawArea, LONG textIndex)
{
	SBarCode	*pBarCode = (SBarCode*)customData;
	RX_Bitmap	*pBmp	  = pBarCode->pBmp;
	int			i;
	int			black=TRUE;
	UINT16		wStr[64];
//	char		test[64];

	for (i=0; i<numberOfCharacters && i<SIZEOF(wStr)-1; i++)
	{
//		test[i] = (UCHAR)text[textIndex+i];
		wStr[i] = (UCHAR)text[textIndex+i];
	}
//	test[i]=0;
	wStr[i]=0;
	
//	printf("_custom_draw_text font:(lfFaceName=\"%s\", lfHeight=%d), x=%f, y=%f, text=\"%s\"\n", font->lfFaceName, font->lfHeight, x, y, test);

	if (pBarCode->font==NULL) pBarCode->font=ft_load_font(font->lfFaceName);

	pBarCode->fontHeightPx = ft_height(pBarCode->font, font->lfHeight);

//	x = pBarCode->rect_draw.right+x*3.8;
//	y = pBarCode->rect_draw.top-y*7.2;

	switch(pBarCode->orient)
	{
	case deg90:  x = pBarCode->rect_draw.right-pBarCode->fontHeightPx;
				 y = pBarCode->rect_draw.bottom+y*3.8;
//				 if (x>pBarCode->rect_draw.left+pBarCode->fontHeightPx) x-=pBarCode->fontHeightPx;
				 ft_text_out(pBmp, (int)x, (int)y+ft_char_width(pBarCode->font)*numberOfCharacters/2, pBarCode->font, font->lfHeight, font->lfOrientation, wStr, numberOfCharacters, black);
				 break;

	case deg180: // if (y<pBarCode->rect_draw.top+pBarCode->fontHeightPx) y+=pBarCode->fontHeightPx;
				 x=pBarCode->rect_draw.right+x*3.8;
				 y=pBarCode->rect_draw.top+pBarCode->fontHeightPx;
				 ft_text_out(pBmp, (int)x+ft_char_width(pBarCode->font)*numberOfCharacters/2, (int)y, pBarCode->font, font->lfHeight, font->lfOrientation, wStr, numberOfCharacters, black);
				 break;
				 
	case deg270: x = pBarCode->rect_draw.left+pBarCode->fontHeightPx;//+x*7.2;
				 y = pBarCode->rect_draw.top+y*3.8;
//				 if (x<pBarCode->rect_draw.left+pBarCode->fontHeightPx) x+=pBarCode->fontHeightPx;
				 ft_text_out(pBmp, (int)x, (int)y-ft_char_width(pBarCode->font)*numberOfCharacters/2, pBarCode->font, font->lfHeight, font->lfOrientation, wStr, numberOfCharacters, black);
				 break;
				 
	default:	 x=pBarCode->rect_draw.left+x*3.8; 
				 y=pBarCode->rect_draw.bottom-pBarCode->fontHeightPx;
				 // if (y>pBarCode->rect_draw.top+pBarCode->fontHeightPx) y-=pBarCode->fontHeightPx;
				 ft_text_out(pBmp, (int)x-ft_char_width(pBarCode->font)*numberOfCharacters/2, (int)y, pBarCode->font, font->lfHeight, font->lfOrientation, wStr, numberOfCharacters, black);
	}
	return ErrOk;	
}

//--- _draw_bitmap_2D ------------------------------------------------
static void _draw_bitmap_2D (RX_Bitmap *pBmp, int par_x, int par_y, SBarcodeBox *pBox, HBarCode hBarCode, int dropSize, RECT *prect)
{
	SBarCode*	pBarCode = (SBarCode*)hBarCode;
	int			row, x, y, width, widthCode;
	int			dy, w, boxWidth;
	int			x0, x1, i, f, stretch;
	int			xoffset, l;
	char		*src, *src1;
	BYTE		dot[8];
	BYTE		*dst, *d1;
	UCHAR		*s, *d;
	double		pos;
	int			shift, mod;
	BYTE		*bufEnd;

	if (pBarCode->row)
	{
		// TRACE(L"Size(%d x %d)\n", pBarCode->cols, pBarCode->rows);
//		ASSERT(pBarCode->stretch!=0.0);
//		widthPx = (pBmp->width+31) & ~31;
		width	= pBmp->lineLen; //widthPx*pBmp->bppx/8;
		w		= pBox->hdr.rect.right-pBox->hdr.rect.left;
		boxWidth= (w*pBmp->bppx+7)/8;
		xoffset = 0;
		dy		= 1;
		switch(pBarCode->orient)
		{
		case deg90:		if (pBarCode->mirror==BOX_MIRROR_HORIZ)	x  = pBox->hdr.rect.right-pBarCode->cols*pBox->stretch;
						else									x  = pBox->hdr.rect.left;
						if (pBarCode->mirror==BOX_MIRROR_VERT)	y  = pBox->hdr.rect.top+pBarCode->rows*pBox->stretch;
						else									y  = pBox->hdr.rect.bottom-pBarCode->rows*pBox->stretch;
						break;

		case deg180:	if (pBarCode->mirror==BOX_MIRROR_HORIZ)	x  = pBox->hdr.rect.left;
						else									x  = (int)(pBox->hdr.rect.right-pBarCode->cols*pBox->stretch/pBarCode->stretch);
						if (pBarCode->mirror==BOX_MIRROR_VERT)	y  = pBox->hdr.rect.top+pBarCode->rows*pBox->stretch;
						else									y  = (int)(pBox->hdr.rect.bottom-pBarCode->rows*pBox->stretch);
						break;

		case deg270:	if (pBarCode->mirror==BOX_MIRROR_HORIZ)	x  = pBox->hdr.rect.left;
						else									x  = (int)(pBox->hdr.rect.right-pBarCode->cols*pBox->stretch/pBarCode->stretch);
						if (pBarCode->mirror==BOX_MIRROR_VERT)	y  = pBox->hdr.rect.bottom;
						else									y  = pBox->hdr.rect.top;
						break;

		case deg0:		if (pBarCode->mirror==BOX_MIRROR_HORIZ) x  = pBox->hdr.rect.right-pBarCode->cols*pBox->stretch;
						else									x  = pBox->hdr.rect.left;
						if (pBarCode->mirror==BOX_MIRROR_VERT)  y  = pBox->hdr.rect.bottom;
						else									y  = pBox->hdr.rect.top;
						break;
		}
		
		x += par_x;
		y += par_y;

		if (pBarCode->mirror==BOX_MIRROR_VERT)
		{
			dy= -1;
			width = -width;
		}

		x0 = x%(8/pBmp->bppx);
		widthCode = (x0+(int)((pBarCode->cols*pBox->stretch)/pBarCode->stretch)*pBmp->bppx+7)/8; 
		if (pBmp->bppx==1)
		{
			dropSize = (dropSize)?1:0; 
			for (i=0; i<8; i++) dot[7-i] = dropSize<<i;
			shift = 3;		
			mod   = 0x07;
		}
		else if (pBmp->bppx==2) 
		{
			for (i=0; i<4; i++) dot[3-i] = dropSize<<(2*i); 
			shift = 2;		
			mod   = 0x03;
		}
		else return; // Error(ERR_STOP, 0, "Barcode does not support %d bits per pixel bitmaps", pBmp->bppx);

		prect->left   = x;
		prect->right  = x+pBarCode->cols*pBox->stretch;
		prect->top    = y;
		prect->bottom = y+pBarCode->rows*pBox->stretch;

		src = pBarCode->mdata;
		dst = &pBmp->buffer[y*pBmp->lineLen+x*pBmp->bppx/8];
		bufEnd = pBmp->buffer+pBmp->sizeUsed;
		for (row=0; row<pBarCode->rows; row++)
		{
			if (dst<pBmp->buffer)
			{
				src += strlen(src)+1;
				dst += width*pBox->stretch;
				continue;
			}

			// TRACE("Row %02d: %s\n", row, src);
			x=x0;
			pos = x;
			l = (int)strlen(src);
			src1 = &src[l-1];
			memset(dst, 0x00, pBox->stretch*strlen(src)*pBmp->bppx/8);
			while (*src)
			{
				stretch = (int)pos;
				pos += (pBox->stretch/pBarCode->stretch);
				stretch = (int)pos - stretch;
				if (stretch<pBox->filter) 
				{
					stretch=pBox->filter;
					if (!_ErrorSet) Error(ERR_CONT, 600, "Barcode Box %d: Filter larger than stretch factor", pBarCode->boxNo+1);
					_ErrorSet=TRUE;
				}
				if (pBarCode->mirror==BOX_MIRROR_HORIZ)
				{
					if (*src1--=='A')
					{
						if (*src1=='A') for (x1=x, i=stretch;			  i; i--, x1++) dst[x1>>shift] |= dot[x1&mod];
						else			for (x1=x, i=stretch-pBox->filter;i; i--, x1++) dst[x1>>shift] |= dot[x1&mod];
					}
					src++;
				}
				else
				{
					if (*src++=='A')
					{
						if (*src=='A')  for (x1=x, i=stretch;				i; i--, x1++) dst[x1>>shift] |= dot[x1&mod];
						else			for (x1=x, i=stretch-pBox->filter;	i; i--, x1++) dst[x1>>shift] |= dot[x1&mod];
					}
				}
				x=(int)pos;
			}

			// TRACE("widthCode=%d, x/8=%d\n", widthCode, (x+7)/8);
			//--- filter rows ---
			// TRACE("dst=%d\n", (dst-buffer)/width);
			if (row && pBox->filter)
			{
				d1 = dst;
				for (i=pBox->filter; i; i--)
				{
					s=dst+xoffset;
					d1-=width;
					d=d1;
					for (f=widthCode; f; f--) *d++ &= *s++;
				}
			}
			if (row+1==pBarCode->rows)
			{
				for (i=pBox->stretch-1-pBox->filter; i>0; --i)
				{
					memcpy(dst+width+xoffset, dst+xoffset, widthCode);
					dst += width;
					if (dst>bufEnd) break;
				}
			}
			else
			{
				for (i=pBox->stretch-1; i>0; --i)
				{
					memcpy(dst+width+xoffset, dst+xoffset, widthCode);
					dst += width;
					if (dst>bufEnd) break;
				}
			}
			src++;
			dst += width;
			if (dst>bufEnd) break;
		}
	}

	pBarCode->row=0;
}

//--- _draw_bitmap_1D_000 ------------------------------------------------
static void _draw_bitmap_1D_000 (RX_Bitmap *pBmp, int par_x, int par_y, SBarcodeBox *pBox, HBarCode hBarCode, int dropSize)
{
	SBarCode*	pBarCode = (SBarCode*)hBarCode;
	int			x, y, w;
	int			boxWidth;
	int			height, h, h2, fh;
	int			i, l, lineLen;
	char		*src;
	BYTE		dot[8];
	BYTE		*dst;
	BYTE		*mdata;
	BYTE		*s, *d;
	int			shift, mod;
	BYTE		*bufEnd;
	int			adjust;
	//--- top bars (without addon) --- 
	int			addon;
	BYTE		*tdata;
	//--- long bars ----
	int			longBar;
	BYTE		*bdata;

	if (pBarCode->row)
	{
		// TRACE(L"Size(%d x %d)\n", pBarCode->cols, pBarCode->rows);
//		ASSERT(pBarCode->stretch!=0.0);
		boxWidth= 1+(pBox->hdr.rect.right*pBmp->bppx+7)/8 - (pBox->hdr.rect.left*pBmp->bppx+7)/8;
		x  = pBox->hdr.rect.left;
		y  = pBox->hdr.rect.top;
		w  = boxWidth*8/pBmp->bppx;

		tdata = (BYTE*)malloc(boxWidth);
		mdata = (BYTE*)malloc(boxWidth);
		bdata = (BYTE*)malloc(boxWidth);
		if (tdata==NULL || mdata==NULL || bdata==NULL) return;
		memset(tdata, 0, boxWidth);
		memset(mdata,  0, boxWidth);
		memset(bdata, 0, boxWidth);
				
		if (pBmp->bppx==1)
		{
			dropSize = (dropSize)?1:0; 
			for (i=0; i<8; i++) dot[7-i] = dropSize<<i;
			shift = 3;		
			mod   = 0x07;
		}
		else if (pBmp->bppx==2) 
		{
			for (i=0; i<4; i++) dot[3-i] = dropSize<<(2*i); 
			shift = 2;		
			mod   = 0x03;
		}
		else return; // Error(ERR_STOP, 0, "Barcode does not support %d bits per pixel bitmaps", pBmp->bppx);

		if (TRUE)
		{
			src = pBarCode->mdata;

			longBar  = FALSE;
			addon    = FALSE;
			adjust   = 0;
			x		 -= ((pBox->hdr.rect.left*pBmp->bppx)/8) * (8/pBmp->bppx);
			while (*src)
			{
					 if ((UCHAR)*src==249) adjust=0;				// End bar width decrement 
				else if ((UCHAR)*src==250) adjust=-pBox->bar[0]/13;	// Begin bar width decrement 
				else if ((UCHAR)*src==251) adjust=0;				// End bar width increment 
				else if ((UCHAR)*src==252) adjust=+pBox->bar[0]/13;	// Begin bar width increment 		
				else if ((UCHAR)*src==253) addon=TRUE;				// Begin of an add-on section which last until the end of the code 
				else if ((UCHAR)*src==254) longBar=TRUE;	// Begin of section with long (=protruding) bars 
				else if ((UCHAR)*src==255) longBar=FALSE;	// End of section with long (=protruding) bars 
				else if (*src=='0') // long bar 
				{
					l=pBox->bar[0];
					for (i=l; i && x<w; i--, x++)
					{
						tdata[x>>shift] |= dot[x&mod];
						mdata[x>>shift] |= dot[x&mod];
						bdata[x>>shift] |= dot[x&mod];
					}
					x += pBox->space[0];
				}
				else if (*src=='1') // short bar, growing top down 
				{
					l=pBox->bar[0];
					switch(pBarCode->states)
					{
					case 2:	for (i=l; i && x<w; i--, x++) mdata[x>>shift] |= dot[x&mod]; break;

					case 3: for (i=l; i && x<w; i--, x++) bdata[x>>shift] |= dot[x&mod]; break;

					case 4:	for (i=l; i && x<w; i--, x++)
							{
								tdata[x>>shift] |= dot[x&mod];
								mdata[x>>shift] |= dot[x&mod];
							}
							break;
					}
					x += pBox->space[0];
				}
				else if (*src=='2') // short bar, growing bottom up
				{
					l=pBox->bar[0];
					for (i=l; i && x<w; i--, x++)
					{
						if (pBarCode->states==3) tdata[x>>shift] |= dot[x&mod];
						else
						{
							mdata[x>>shift] |= dot[x&mod];
							bdata[x>>shift] |= dot[x&mod];
						} 
					}
					x += pBox->space[0];
				}
				else if (*src=='3') // short bar (Tracker) 
				{
					l=pBox->bar[0];
					for (i=l; i && x<w; i--, x++)
					{
						mdata[x>>shift] |= dot[x&mod];
					}
					x += pBox->space[0];
				}
				else if (*src>='a')
				{
					x+=pBox->space[*src-'a']-adjust;
				} 
				else if (*src>='A')
				{
					l = pBox->bar[*src-'A']+adjust;
					if (x+l<0) x += l;
					else 
					{
						if (x<0)
						{
							l+=x;
							x=0;
						}
						for (i=l; i && x<w; i--, x++)
						{
							if (!addon)  tdata[x>>shift] |= dot[x&mod];
										 mdata[x>>shift] |= dot[x&mod];
							if (longBar) bdata[x>>shift] |= dot[x&mod];
						}
					}
				}
				src++;
			}

			fh = pBarCode->fontHeightPx+(int)(pBox->dist*25.4/_DpiY);
			switch(pBarCode->states)
			{
			case 2:		addon  = TRUE;
						height = pBox->bar[1]*2540/_DpiY;
						h      = height - pBox->bar[2]*2540/_DpiY;
						h2     = 0;
						break;
			
			case 3:		addon  = TRUE;
						height	 = pBox->hdr.rect.bottom-pBox->hdr.rect.top-fh;
						h = h2 = height/2;
						height -= h2;
						break;

			case 4:		addon  = TRUE;
						height = pBox->bar[1]*2540/_DpiY;
						h	   = pBox->bar[2]*2540/_DpiY;
						h = h2 = (height-h)/2;
						height -= h2;
						break;

			default:	h		 = fh;
						height	 = pBox->hdr.rect.bottom-pBox->hdr.rect.top -fh;
						h2		 = h/2;
			}

			lineLen = pBmp->lineLen;

			x = (par_x + pBox->hdr.rect.left)*pBmp->bppx/8;
			y += par_y;
			if (y>=0) dst = &pBmp->buffer[y*pBmp->lineLen+x];
			else
			{
				height += y;
				dst    = &pBmp->buffer[x];
			}

			bufEnd = pBmp->buffer+pBmp->sizeUsed;
			if (addon)
			{
				for (i=h; i>0; --i, dst+=lineLen)
				{
					if (dst>bufEnd) break;
					for (l=boxWidth, d=dst, s=tdata; l; l--)
						*d++ |= *s++;
				}
				height-=h;
			}
			for (i=height; i>0; --i, dst+=lineLen)
			{
				if (dst>bufEnd) break;
				memcpy(dst, mdata, boxWidth);
			}
			for (i=h2; i>0; --i, dst+=lineLen)
			{
				if (dst>bufEnd) break;
				for (l=boxWidth, d=dst, s=bdata; l; l--)
					*d++ |= *s++;
			}
		}
		free(tdata);
		free(mdata);
		free(bdata);
	}

	pBarCode->row=0;
}

//--- _draw_bitmap_1D_090 ------------------------------------------------
static void _draw_bitmap_1D_090 (RX_Bitmap *pBmp, int par_x, int par_y, SBarcodeBox *pBox, HBarCode hBarCode, int dropSize)
{
	SBarCode*	pBarCode = (SBarCode*)hBarCode;
	int			x, y;
	int			height, h, h2, fh;
	int			i, l, lineLen, n;
	char		*src;
	BYTE		dot[8];
	BYTE		*dst;
	int			start[4];
	int			len[4];
	int			shift, mod;
	int			adjust;

	if (pBarCode->row)
	{				
		if (pBmp->bppx==1)
		{
			dropSize = (dropSize)?1:0; 
			for (i=0; i<8; i++) dot[7-i] = dropSize<<i;
			shift = 3;		
			mod   = 0x07;
		}
		else if (pBmp->bppx==2) 
		{
			for (i=0; i<4; i++) dot[3-i] = dropSize<<(2*i); 
			shift = 2;		
			mod   = 0x03;
		}
		else return; // Error(ERR_STOP, 0, "Barcode does not support %d bits per pixel bitmaps", pBmp->bppx);

		memset(start, 0, sizeof(start));
		memset(len, 0, sizeof(len));

		fh = pBarCode->fontHeightPx+(int)(pBox->dist*25.4/_DpiY);			
		switch(pBarCode->states)
		{
		case 2:		height = pBox->bar[1]*2540/_DpiY;
					h	   = pBox->bar[2]*2540/_DpiY;
					// long bar (full)
					start[0] = pBox->hdr.rect.left;
					len[0]   = height;

					// short bar
					start[1] = pBox->hdr.rect.left+height-h;
					len[1]   = h;
					break;
			
		case 3:		height = height   = pBox->hdr.rect.right-pBox->hdr.rect.left-fh;
					h2	   = height/2;

					// long bar (full)
					start[0] = pBox->hdr.rect.left;
					len[0]   = height;

					// bottom bar
					start[1] = pBox->hdr.rect.left+h2;
					len[1]   = h2;

					// top bar
					start[2] = pBox->hdr.rect.left;
					len[2]   = h2;
					break;

		case 4:		height = pBox->bar[1]*2540/_DpiY;
					h	   = pBox->bar[2]*2540/_DpiY;
					h2 = (height-h)/2;

					// long bar (full)
					start[0] = pBox->hdr.rect.left;
					len[0]   = height;

					// ascender
					start[1] = pBox->hdr.rect.left;
					len[1]   = height-h2;

					// descender
					start[2] = pBox->hdr.rect.left+h2;
					len[2]   = height-h2;

					// tracker
					start[3] = pBox->hdr.rect.left+h2;
					len[3]   = h;
					break;

		default:	h		 = fh;
					height   = pBox->hdr.rect.right-pBox->hdr.rect.left;
					h2		 = h/2;
					// short bar
					start[0] = pBox->hdr.rect.left;
					len[0]	 = height-h;

					// long bar
					start[1] = pBox->hdr.rect.left;
					len[1]	 = height-h2;

					// add on
					start[2] = pBox->hdr.rect.left+h;
					len[2]	 = height-h;
		}
	
		for (i=0; i<4; i++)
		{	
			if (start[i] < 0) {len[i]+=start[i]; start[i]=0;}
			if (start[i]+len[i] > pBox->hdr.rect.right) len[i] = pBox->hdr.rect.right-start[i];
		}
			
		lineLen = pBmp->lineLen;
		y		= par_y + pBox->hdr.rect.bottom;
		dst		= &pBmp->buffer[y*lineLen];
		src		= pBarCode->mdata;
		adjust  = 0;
		n		= 0;
		while (*src)	
		{
					if ((UCHAR)*src==249) adjust=0;				// End bar width decrement 
			else if ((UCHAR)*src==250) adjust=-pBox->bar[0]/13;	// Begin bar width decrement 
			else if ((UCHAR)*src==251) adjust=0;				// End bar width increment 
			else if ((UCHAR)*src==252) adjust=+pBox->bar[0]/13;	// Begin bar width increment 		
			else if ((UCHAR)*src==253) n=2;						// Begin of an add-on section which last until the end of the code 
			else if ((UCHAR)*src==254) n=1;						// Begin of section with long (=protruding) bars 
			else if ((UCHAR)*src==255) n=0;						// End of section with long (=protruding) bars 
			else if (*src>='0' && *src<='3')
			{
				n=*src-'0';
				for (i=pBox->bar[0]; i && y>=0; i--, y--, dst-=lineLen)
				{
					for (x=par_x+start[n], l=len[n]; l>0; l--, x++) dst[x>>shift] |= dot[x&mod];
				}
				l    = pBox->space[0]; 
				y   -= l;
				dst -= l*lineLen;
			}
			else if (*src>='a')
			{
				l = pBox->space[*src-'a']-adjust;
				y   -= l;
				dst -= l*lineLen;
			} 
			else if (*src>='A')
			{
				for (i=pBox->bar[*src-'A']+adjust; i && y>=0; i--, y--, dst-=lineLen)
				{
					for (x=par_x+start[n], l=len[n]; l>0; l--, x++) dst[x>>shift] |= dot[x&mod];
				}
			}
			src++;
		}
	}

	pBarCode->row=0;
}

//--- _draw_bitmap_1D_180 ------------------------------------------------
static void _draw_bitmap_1D_180 (RX_Bitmap *pBmp, int par_x, int par_y, SBarcodeBox *pBox, HBarCode hBarCode, int dropSize)
{
	SBarCode*	pBarCode = (SBarCode*)hBarCode;
	int			x, y, w;
	int			boxWidth;
	int			height, h, fh,  h2;
	int			i, l, lineLen;
	char		*src;
	BYTE		dot[8];
	BYTE		*dst;
	BYTE		*mdata;
	BYTE		*s, *d;
	int			shift, mod;
	int			adjust;
	//--- top bars (without addon) --- 
	int			addon;
	BYTE		*tdata;
	//--- long bars ----
	int			longBar;
	BYTE		*bdata;

	if (pBarCode->row)
	{
		boxWidth= 1+(pBox->hdr.rect.right*pBmp->bppx+7)/8 - (pBox->hdr.rect.left*pBmp->bppx+7)/8;
		x  = pBox->hdr.rect.right;
		y  = pBox->hdr.rect.bottom;
		w = boxWidth*8/pBmp->bppx;

		tdata = (BYTE*)malloc(boxWidth);
		mdata  = (BYTE*)malloc(boxWidth);
		bdata = (BYTE*)malloc(boxWidth);
		if (tdata==NULL || mdata==NULL || bdata==NULL) return;
		memset(tdata, 0, boxWidth);
		memset(mdata,  0, boxWidth);
		memset(bdata, 0, boxWidth);
		
		if (pBmp->bppx==1)
		{
			dropSize = (dropSize)?1:0; 
			for (i=0; i<8; i++) dot[7-i] = dropSize<<i;
			shift = 3;		
			mod   = 0x07;
		}
		else if (pBmp->bppx==2) 
		{
			for (i=0; i<4; i++) dot[3-i] = dropSize<<(2*i); 
			shift = 2;		
			mod   = 0x03;
		}
		else return; // Error(ERR_STOP, 0, "Barcode does not support %d bits per pixel bitmaps", pBmp->bppx);

		if (TRUE)
		{
			src = pBarCode->mdata;

			longBar  = FALSE;
			addon    = FALSE;
			adjust   = 0;
			if (pBox->hdr.rect.left>0) x -= ((pBox->hdr.rect.left*pBmp->bppx)/8) * (8/pBmp->bppx);
			while (*src && x>=0)
			{
					 if ((UCHAR)*src==249) adjust=0;				// End bar width decrement 
				else if ((UCHAR)*src==250) adjust=-pBox->bar[0]/13;	// Begin bar width decrement 
				else if ((UCHAR)*src==251) adjust=0;				// End bar width increment 
				else if ((UCHAR)*src==252) adjust=+pBox->bar[0]/13;	// Begin bar width increment 		
				else if ((UCHAR)*src==253) addon=TRUE;				// Begin of an add-on section which last until the end of the code 
				else if ((UCHAR)*src==254) longBar=TRUE;	// Begin of section with long (=protruding) bars 
				else if ((UCHAR)*src==255) longBar=FALSE;	// End of section with long (=protruding) bars 
				else if (*src=='0') // long bar 
				{
					l=pBox->bar[0];
					for (i=l; i && x>=0; i--, x--)
					{
						tdata[x>>shift] |= dot[x&mod];
						mdata[x>>shift] |= dot[x&mod];
						bdata[x>>shift] |= dot[x&mod];
					}
					x -= pBox->space[0];
				}
				else if (*src=='1') // short bar, growing top down 
				{
					l=pBox->bar[0];
					switch(pBarCode->states)
					{
					case 2:	for (i=l; i && x>=0; i--, x--) mdata[x>>shift] |= dot[x&mod]; break;
					case 3: for (i=l; i && x>=0; i--, x--) bdata[x>>shift] |= dot[x&mod]; break;
					case 4:	for (i=l; i && x>=0; i--, x--)
							{
								tdata[x>>shift] |= dot[x&mod];
								mdata[x>>shift] |= dot[x&mod];
							}
							break;
					}
					x -= pBox->space[0];
				}
				else if (*src=='2') // short bar, growing bottom up
				{
					l=pBox->bar[0];
					for (i=l; i && x>=0; i--, x--)
					{
						if (pBarCode->states==3) tdata[x>>shift] |= dot[x&mod];
						else
						{
							mdata[x>>shift] |= dot[x&mod];
							bdata[x>>shift] |= dot[x&mod];
						} 
					}
					x -= pBox->space[0];
				}
				else if (*src=='3') // short bar (Tracker) 
				{
					l=pBox->bar[0];
					for (i=l; i && x>=0; i--, x--)
					{
						mdata[x>>shift] |= dot[x&mod];
					}
					x -= pBox->space[0];
				}
				else if (*src>='a')
				{
					x-=pBox->space[*src-'a']-adjust;
				} 
				else if (*src>='A')
				{
					for (i=pBox->bar[*src-'A']+adjust; i && x>=0; i--, x--)
					{
						if (!addon)  tdata[x>>shift] |= dot[x&mod];
									 mdata[x>>shift] |= dot[x&mod];
						if (longBar) bdata[x>>shift] |= dot[x&mod];
					}
				}
				src++;
			}

			fh = pBarCode->fontHeightPx+(int)(pBox->dist*25.4/_DpiY);
			switch(pBarCode->states)
			{
			case 2:		addon  = TRUE;
						height = pBox->bar[1]*2540/_DpiY;
						h      = height-pBox->bar[2]*2540/_DpiY;
						h2     = 0;
						break;
			
			case 3:		addon  = TRUE;
						height	 = pBox->hdr.rect.bottom-pBox->hdr.rect.top-fh;
						h = h2 = height/2;
						height -= h2;
						break;

			case 4:		addon  = TRUE;
						height = pBox->bar[1]*2540/_DpiY;
						h	   = pBox->bar[2]*2540/_DpiY;
						h = h2 = (height-h)/2;
						height -= h2;
						break;

			default:	h		 = fh;
						height	 = pBox->hdr.rect.bottom-pBox->hdr.rect.top -fh;
						h2		 = h/2;
			}
			lineLen = pBmp->lineLen;
			y = par_y + pBox->hdr.rect.bottom;
			x = par_x + pBox->hdr.rect.left;
			if (pBox->hdr.rect.left<0) dst = &pBmp->buffer[y*pBmp->lineLen];
			else dst = &pBmp->buffer[y*pBmp->lineLen+x*pBmp->bppx/8];
			if (addon)
			{
				for (i=h; i>0 && y>0; --i, dst-=lineLen, y--)
				{
					if (y<pBmp->height)
						for (l=0, d=dst, s=tdata; l<boxWidth; l++)
							*d++ |= *s++;
				}
				height-=h;
			}
			for (i=height; i>0 && y>0; --i, dst-=lineLen, y--)
			{
				if (y<pBmp->height) memcpy(dst, mdata, boxWidth);
			}
			for (i=h2; i>0 && y>0; --i, dst-=lineLen, y--)
			{
				if (y<pBmp->height)
					for (l=boxWidth, d=dst, s=bdata; l; l--)
						*d++ |= *s++;
			}
		}
		free(tdata);
		free(mdata);
		free(bdata);
	}

	pBarCode->row=0;
}

//--- _draw_bitmap_1D_270 ------------------------------------------------
static void _draw_bitmap_1D_270 (RX_Bitmap *pBmp, int par_x, int par_y, SBarcodeBox *pBox, HBarCode hBarCode, int dropSize)
{
	SBarCode*	pBarCode = (SBarCode*)hBarCode;
	int			x, y;
	int			height, h, h2, fh;
	int			i, l, lineLen, n;
	char		*src;
	BYTE		dot[8];
	BYTE		*dst;
	int			start[4];
	int			len[4];
	int			shift, mod;
	int			adjust;

	if (pBarCode->row)
	{				
		if (pBmp->bppx==1)
		{
			dropSize = (dropSize)?1:0; 
			for (i=0; i<8; i++) dot[7-i] = dropSize<<i;
			shift = 3;		
			mod   = 0x07;
		}
		else if (pBmp->bppx==2) 
		{
			for (i=0; i<4; i++) dot[3-i] = dropSize<<(2*i); 
			shift = 2;		
			mod   = 0x03;
		}
		else return; // Error(ERR_STOP, 0, "Barcode does not support %d bits per pixel bitmaps", pBmp->bppx);

		memset(start, 0, sizeof(start));
		memset(len, 0, sizeof(len));

		fh = pBarCode->fontHeightPx+(int)(pBox->dist*25.4/_DpiY);			
		switch(pBarCode->states)
		{
		case 2:		height = pBox->bar[1]*2540/_DpiY;
					h	   = pBox->bar[2]*2540/_DpiY;

					// long bar (full)
					len[0]   = height;
					start[0] = pBox->hdr.rect.right-height;

					// short bar
					len[1]   = h;
					start[1] = pBox->hdr.rect.right-height;
					break;
			
		case 3:		height = height   = pBox->hdr.rect.right-pBox->hdr.rect.left-fh;
					h2	   = height/2;

					// long bar (full)
					start[0] = pBox->hdr.rect.right-height;
					len[0]   = height;

					// bottom bar
					start[1] = pBox->hdr.rect.right-height;
					len[1]   = h2;

					// top bar
					start[2] = pBox->hdr.rect.right-height+h2;
					len[2]   = h2;
					break;

		case 4:		height = pBox->bar[1]*2540/_DpiY;
					h	   = pBox->bar[2]*2540/_DpiY;
					h2 = (height-h)/2;

					// long bar (full)
					start[0] = pBox->hdr.rect.left;
					len[0]   = height;

					// ascender
					start[1] = pBox->hdr.rect.left;
					len[1]   = height-h2;

					// descender
					start[2] = pBox->hdr.rect.left+h2;
					len[2]   = height-h2;

					// tracker
					start[3] = pBox->hdr.rect.left+h2;
					len[3]   = h;
					break;

		default:	h		 = fh;
					height   = pBox->hdr.rect.right-pBox->hdr.rect.left;
					h2		 = h/2;

					// short bar
					len[0]	 = height-h;
					start[0] = pBox->hdr.rect.right-len[0];

					// long bar
					len[1]	 = height-h2;
					start[1] = pBox->hdr.rect.right-len[1];

					// add on
					len[2]	 = height-h;
					start[2] = pBox->hdr.rect.right-len[2]-h;
		}
	
		for (i=0; i<4; i++)
		{	
			if (start[i] < 0) {len[i]+=start[i]; start[i]=0;}
			if (start[i]+len[i] > pBox->hdr.rect.right) len[i] = pBox->hdr.rect.right-start[i];
		}
			
		lineLen = pBmp->lineLen;
		y		= pBox->hdr.rect.top;
		dst		= &pBmp->buffer[y*lineLen];
		src		= pBarCode->mdata;
		adjust  = 0;
		n		= 0;

		while (*src)
		{
				 if ((UCHAR)*src==249) adjust=0;				// End bar width decrement 
			else if ((UCHAR)*src==250) adjust=-pBox->bar[0]/13;	// Begin bar width decrement 
			else if ((UCHAR)*src==251) adjust=0;				// End bar width increment 
			else if ((UCHAR)*src==252) adjust=+pBox->bar[0]/13;	// Begin bar width increment 		
			else if ((UCHAR)*src==253) n=2;						// Begin of an add-on section which last until the end of the code 
			else if ((UCHAR)*src==254) n=1;						// Begin of section with long (=protruding) bars 
			else if ((UCHAR)*src==255) n=0;						// End of section with long (=protruding) bars 
			else if (*src>='0' && *src<='3')
			{
				n=*src-'0';
				for (i=pBox->bar[0]; i && dst>pBmp->buffer; i--, y++, dst+=lineLen)
				{
					for (x=par_x+start[n], l=len[n]; l>0; l--, x++) dst[x>>shift] |= dot[x&mod];
				}
				l    = pBox->space[0]; 
				y   += l;
				dst += l*lineLen;
			}
			else if (*src>='a')
			{
				l = pBox->space[*src-'a']-adjust;
				y   += l;
				dst += l*lineLen;
			} 
			else if (*src>='A')
			{
				for (i=pBox->bar[*src-'A']+adjust; i && dst>pBmp->buffer; i--, y++, dst+=lineLen)
				{
					for (x=par_x+start[n], l=len[n]; l>0; l--, x++) dst[x>>shift] |= dot[x&mod];
				}
			}
			src++;
		}
	}

	pBarCode->row=0;
}

//--- _colorize_box -----------------------------------------------------------------------------
static void _colorize_box(RX_Bitmap *pBmp, SBarcodeBox *pBox, RX_Bitmap *pColor, RECT *prect)
{
	if  (pColor->bppx==2)
	{
		BYTE *dst, *d;
		BYTE *color;
		int x, h;
		BYTE mask;

		dst   = pBmp->buffer   + prect->top*pBmp->lineLen;
		color = pColor->buffer + prect->top*pColor->lineLen;
		
		//--- TEST ---
		/*
		for (h=prect->top; h<prect->bottom; h++)
		{
			for (x=prect->left; x<prect->right; x++)
			{
				mask = 0xc0 >> (2*(x&0x03));
				d = &dst[x>>2];
				if (*d & ~mask) *d |= mask;
				else            *d |= 0x04>>(x&3); 
			}
			dst += pBmp->lineLen;
		}
		*/
		for (h=prect->top; h<prect->bottom; h++)
		{
			for (x=prect->left; x<prect->right; x++)
			{
				mask = 0xc0 >> (2*(x&0x03));
				d = &dst[x>>2];
				if (*d & mask) *d = *d&~mask | color[x>>2]&mask;
			}
			dst   += pBmp->lineLen;
			color += pColor->lineLen; 
		}

	}
}

//--- bc_draw_bitmap ------------------------------------------------------------------
void bc_draw_bitmap(RX_Bitmap *pBmp, int x0, int y0, SBarcodeBox *pBox, HBarCode hBarCode, int dropSize, RX_Bitmap *pColor)
{
	SBarCode*	pBarCode = (SBarCode*)hBarCode;
	RECT		rect;
	rect.bottom = rect.top = rect.left = rect.right = 0;

	if (pBarCode->code_2d) 
	{
		_draw_bitmap_2D(pBmp, x0, y0, pBox, hBarCode, dropSize, &rect);
		if (pColor && pColor->buffer) _colorize_box(pBmp, pBox, pColor, &rect);
	}
	else
	{
		switch(pBox->orientation)
		{
		case deg90:  _draw_bitmap_1D_090(pBmp, x0, y0, pBox, hBarCode, dropSize); break;
		case deg180: _draw_bitmap_1D_180(pBmp, x0, y0, pBox, hBarCode, dropSize); break;
		case deg270: _draw_bitmap_1D_270(pBmp, x0, y0, pBox, hBarCode, dropSize); break;
		default:	 _draw_bitmap_1D_000(pBmp, x0, y0, pBox, hBarCode, dropSize);
		}
		if (pColor && pColor->buffer) 
		{
			rect.left   = x0+pBox->hdr.rect.left;
			rect.right  = x0+pBox->hdr.rect.right;
			rect.top    = y0+pBox->hdr.rect.top;
			rect.bottom = y0+pBox->hdr.rect.bottom+1;
			_colorize_box(pBmp, pBox, pColor, &rect);
		}
	}
}
