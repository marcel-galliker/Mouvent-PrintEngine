// ****************************************************************************
//
//	tec_it.c
//
// ****************************************************************************
//
//	Copyright 2014 by Radex AG, Switzerland. All rights reserved.
//	Written by Marcel Galliker 
//
// ****************************************************************************

//--- includes ----------------------------------------------------------------
#include "rx_common.h"
#include "tec_it.h"
#include "tbarcode.h"
 
 //--- defines --------------------------------------------
#define TEC_IT_LX_LICENSEE			"TEMP:0914 Graph-Tech AG"
#define TEC_IT_LX_LICENSE_KEY		"F5D7EF49FAD4AA5F8B05D3AFCF3B4145"
#define TEC_IT_LX_LICENSE_PRODUCT	2025

#define TEC_IT_WIN_LICENSEE			"Mem: Graph-Tech AG 4800"
#define TEC_IT_WIN_LICENSE_KEY		"C52EF14426FE7C146FA50C89E54A29EE"
#define TEC_IT_WIN_LICENSE_PRODUCT	eLicProd2D

//--- ti_init ---------------------------------
void ti_init(void)
{
#ifdef linux
	BCInitLibrary("/usr/local/share/tbarcode11");  
	BCLicenseMe(TEC_IT_LX_LICENSEE, eLicKindDeveloper, 1, TEC_IT_LX_LICENSE_KEY, TEC_IT_LX_LICENSE_PRODUCT);//eLicProd2D);  
#else
	BCLicenseMe(TEC_IT_WIN_LICENSEE, eLicKindDeveloper, 1, TEC_IT_WIN_LICENSE_KEY, TEC_IT_WIN_LICENSE_PRODUCT); 
#endif
}

//--_ ti_end --------------------------------
void ti_end(void)
{
#ifdef linux
	BCDeInitLibrary();
#endif	
}

//--- ti_test -------------------------------------------------------
void ti_test(void)
{
	ti_init();

	int ret;

	// Allocate memory and retrieve barcode handle (pointer) 
	t_BarCode* pBC; 
	BCAlloc(&pBC);  

	// (Optional:) Set font type and height for the human readable text 
	BCSetFontName(pBC, "Helvetica"); 
	BCSetFontHeight(pBC, 10);           // 10 points  

	// Adjust symbology 
	BCSetBCType(pBC, eBC_Code128);  

	// Set barcode data 
	char* demo = "12345678"; 
	BCSetText(pBC, demo, strlen(demo));  

	// Create barcode pattern (bars, spaces) 
	BCCreate(pBC);  

	// Set barcode size (PostScript bounding rectangle) 
	// Units are [0.001 mm] 
	RECT rect;
	rect.left = 0;        // 0 mm 
	rect.bottom = 0;      // 0 mm 
	rect.right = 50000;   // 50 mm 
	rect.top = 30000;     // 30 mm  

	// Draw to device context 
	// not supported in Linux/UNIX, because only the Windows GDI uses a "device context"  
	// Save to Postscript file 
	
	ret = BCPostscriptToFile(pBC, (void *) "/tmp/barcode.eps", &rect);  
	
	// Save barcode image to buffer 
	// Unit is [0.001mm] for Postscript and PCL 
	void* pPSBuffer = malloc(0xffff); 
	BCPostscriptToMemory(pBC, pPSBuffer, 0xffff, &rect);  
	if (pPSBuffer)     
		free(pPSBuffer); 
		
	// Release allocated memory after use  
	// Release memory for barcode structure 
	BCFree(pBC);  
	
	ti_end(); 
}

