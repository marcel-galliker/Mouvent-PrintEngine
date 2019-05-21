//--- Include -----------------------------------------------------------------
#include "stdafx.h"
#include <string.h>
#include "rx_common.h"
#include "tec_it_test.h"

#ifndef DOUBLE
	#define DOUBLE double
#endif

#include "tbarcode.h"

//--- LICENSING --------------------------------------
#define TEC_IT_LICENSEE		"Mem: Radex AG CH-2540"
#ifdef WIN32
	#define TEC_IT_KEY_11		"654B7D6225F04C44BA52DAF657F3E047"
#else
	#define TEC_IT_KEY_11		"654B7D6225F04CA45A523AF65713E0A7"
	#define TEC_IT_LIB_PATH		"/usr/local/share/tbarcode11"
#endif


ERRCODE CALLBACK CustomDrawRow_test (VOID* customData, t_BarCode* barcode, HDC drawDC, HDC targetDC, DOUBLE x, DOUBLE y, DOUBLE width, DOUBLE height);
ERRCODE CALLBACK CustomDrawText_test(VOID *customData, HDC drawDC, LPLOGFONTA font, DOUBLE x, DOUBLE y, e_BCAlign alignment, LPCSTR text, INT numberOfCharacters, LPRECTD drawArea, LONG textIndex);

static int CustomData = 1234;

void TEC_IT_Test(void)
{
	t_BarCode	*pTBarCode;
	ERRCODE		eCode;
	LOGFONTA    font;
	int ret;

	//--- initaliation -------------------------------------------------------
	#ifdef linux
		BCInitLibrary(TEC_IT_LIB_PATH); 
	#endif
	BCLicenseMeA (TEC_IT_LICENSEE, eLicKindDeveloper, 1, TEC_IT_KEY_11, eLicProd2D);

	BCAlloc(&pTBarCode);
	eCode = BCSetBCType			(pTBarCode, eBC_EAN13);
	eCode = BCSetRotation		(pTBarCode, deg0);
	eCode = BCSetOptResolution	(pTBarCode, FALSE);

	memset(&font, 0, sizeof(font));
	font.lfHeight = 320;
	font.lfWeight = 400;
	font.lfCharSet = 0x01;
	strcpy(font.lfFaceName, "OCRB");

	ret = sizeof(font);
	ret = sizeof(LONGLONG);
	ret = sizeof(LONG);
	ret = sizeof(SHORT);
	ret = sizeof(BYTE);
	ret = sizeof(CHAR);

	eCode = BCSetLogFont		(pTBarCode, &font);
	eCode = BCSetPrintText		(pTBarCode, TRUE, FALSE);
	eCode = BCSetFuncDrawText	(pTBarCode, CustomDrawText_test);
	eCode = BCSetCDMethod		(pTBarCode, eCDNone);
	eCode = BCSetModWidthN		(pTBarCode, 265);
	eCode = BCSetRatioA			(pTBarCode, "8:16:24:32:8:16:24:32");
	
	eCode = BCSetFuncDrawRow			(pTBarCode, CustomDrawRow_test);
		    BCSetCBData					(pTBarCode, (void*)&CustomData);
	eCode = BCSetCodePage				(pTBarCode, eCodePage_Default);
	eCode = BCSetEncodingMode			(pTBarCode, eEncMode_CodePage);
	eCode = BCSet2DCompositeComponent	(pTBarCode, eCC_None);

	//--- drawing --------------------------------
	RECT rect_draw; // values in pixels
	rect_draw.left	=	2584;
	rect_draw.right	=	3434;
	rect_draw.top	=	245;
	rect_draw.bottom=	1143;

	eCode = BCSetTextW(pTBarCode, L"1234567890123", 13);

	eCode = BCCheck		(pTBarCode);
	eCode = BCCalcCD	(pTBarCode);
	eCode = BCCreate	(pTBarCode);
	eCode = BCDrawCB(pTBarCode, NULL, &rect_draw);
}

ERRCODE CALLBACK CustomDrawRow_test(VOID* customData, t_BarCode* barcode, HDC drawDC, HDC targetDC, DOUBLE x, DOUBLE y, DOUBLE width, DOUBLE height)
{
	return ErrOk;
}

ERRCODE CALLBACK CustomDrawText_test(VOID *customData, HDC drawDC, LPLOGFONTA font, DOUBLE x, DOUBLE y, e_BCAlign alignment, LPCSTR text, INT numberOfCharacters, LPRECTD drawArea, LONG textIndex)
{
	return ErrOk;	
}
