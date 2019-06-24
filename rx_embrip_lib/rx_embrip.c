// ****************************************************************************
//
//	rx_embrip.c
//
// ****************************************************************************
//
//	Copyright 2017 by Radex AG, Switzerland. All rights reserved.
//	Written by Gilbert Therville 
//
// ****************************************************************************

//--- includes ---------------------------------------------------------------------

#include "rx_error.h"
#include "rx_trace.h"
#include "rx_mem.h"
#include "rx_embrip.h"
#include "rx_luts.h"
#include "rx_screen_ed12.h"
#include "rx_screen_th12.h"
#include "rx_tif.h"

/* --- Ghostscript --- */
#include "ierrors.h"
#include "iapi.h"

#ifdef linux
	#include "errno.h"
#endif

//--- Defines -------------------------------------------------------------------------
#define IMAGE_SCALE_BASE  1588635683 /* prime number for scaling */

//--- Globals -------------------------------------------------------------------------
static SBmpInfo			g_pdfbmp;
static PBYTE			g_buffer[MAX_COLORS];
static SScreenConfig	g_screenConfig;
static char *			g_outputpath;
static char *			g_jobName;
static int				g_time;
static int				g_rotation;
static  int				g_columns;
static  int				g_colDist;
static  int				g_page;

// --- Local prototypes --------------------------------------------
static	int GSDLLCALL	_embrip_stdin	(void *instance, char *buf, int len);
static	int GSDLLCALL	_embrip_stdout	(void *instance, const char *buf, int len);
static	int GSDLLCALL	_embrip_stderr	(void *instance, const char *buf, int len);

static	int GSDLLCALL	_embrip_planealloc	(void *instance, int planes, int width, int heigth, char ***buf);
static	int GSDLLCALL	_embrip_planedone	(void *instance);


//--- rx_bmp_scale ----------------------------------------------------------------------
int rx_bmp_scale(SBmpInfo *pinfo, SPoint NewDim, SPoint NewResol)
{
	UINT32 PelNum;
	UINT32 iCntPel, iCntLine, BaseCntXStepSrc, BaseCntXStepDest, BaseCntYStepSrc, BaseCntYStepDest;
	SHORT iCntPlane;
	BYTE  *pSrc = NULL;
	BYTE *pDst, *pImage = NULL;
	UINT32 SrcLineNum, DstLineCnt, LastSrcLineNum;
	//	int i;

	BaseCntXStepSrc = IMAGE_SCALE_BASE / pinfo->srcWidthPx;
	BaseCntXStepDest = IMAGE_SCALE_BASE / NewDim.x;
	BaseCntYStepSrc = IMAGE_SCALE_BASE / pinfo->lengthPx;
	BaseCntYStepDest = IMAGE_SCALE_BASE / NewDim.y;

	for (iCntPlane = 0; iCntPlane < pinfo->planes; iCntPlane++)
	{
		LastSrcLineNum = 0xFFFFFFFF;
		DstLineCnt = 0;
		iCntLine = BaseCntYStepDest;
		pImage = rx_mem_alloc(NewDim.x*NewDim.y);
		TrPrintfL(1, "data_malloc buffer: %p, size=%u MB, free=%u MB", pImage, (NewDim.x*NewDim.y) / 1024 / 1024, rx_mem_get_freeMB());
		if (pImage == NULL) return Error(ERR_CONT, 0, "No buffer for scaling: %d");

		while (iCntLine <= (UINT32)(IMAGE_SCALE_BASE))
		{
			pDst = &pImage[DstLineCnt * NewDim.x];
			SrcLineNum = iCntLine / BaseCntYStepSrc;
			if (SrcLineNum >= (UINT32)pinfo->lengthPx) SrcLineNum = pinfo->lengthPx - 1;
			if (SrcLineNum == LastSrcLineNum)
			{
				pSrc = &pImage[(DstLineCnt - 1) * NewDim.x];
				memcpy(pDst, pSrc, NewDim.x);
				//				for (i = 0; i < NewWidth; i++) {
				//					if (*(pSrc + i))
				//					TrPrintf(TRUE, "byte not empty >>%x<< at line %d row %d", *(pSrc + i), DstLineCnt, i);
				//				}
			}
			else
			{
				LastSrcLineNum = SrcLineNum;
				pSrc = *pinfo->buffer[iCntPlane] + (SrcLineNum * pinfo->srcWidthPx);
				iCntPel = BaseCntXStepDest;
				while (iCntPel <= (UINT32)(IMAGE_SCALE_BASE))
				{
					PelNum = iCntPel / BaseCntXStepSrc;
					if (PelNum >= (UINT32)pinfo->srcWidthPx) PelNum = pinfo->srcWidthPx - 1;
					*pDst = pSrc[PelNum];
					pDst++;
					iCntPel += BaseCntXStepDest;
				}
			}
			iCntLine += BaseCntYStepDest;
			DstLineCnt++;
		}
		if (pinfo->buffer[iCntPlane]) rx_mem_free(pinfo->buffer[iCntPlane]);
		pinfo->buffer[iCntPlane] = &pImage;
		pImage = NULL;
	}

	pinfo->srcWidthPx = NewDim.x;
	pinfo->lengthPx = NewDim.y;
	pinfo->lineLen = pinfo->srcWidthPx;
	pinfo->dataSize = pinfo->lineLen*pinfo->lengthPx;
	pinfo->resol = NewResol;

	return (REPLY_OK);
}


//--- rx_bmp_rotate ----------------------------------------------------------------------
int rx_bmp_rotate(SBmpInfo *pinfo, int rotation)
{
	int iLineCnt;
	int iLineNum;
	int iColCnt;
	int iColNum;
	SHORT iCntPlane;
	BYTE *pSrc = NULL;;
	BYTE *pDes, *pImage = NULL;
	UINT32 NewWidthPx;
	UINT32 NewlengthPx;

	switch (rotation)
	{
	default:
	case 0:
		return (REPLY_OK);
	case 90:
		NewWidthPx = pinfo->lengthPx;
		NewlengthPx = pinfo->srcWidthPx;
		break;
	case 180:
		NewWidthPx = pinfo->srcWidthPx;
		NewlengthPx = pinfo->lengthPx;
		break;
	case 270:
		NewWidthPx = pinfo->lengthPx;
		NewlengthPx = pinfo->srcWidthPx;
		break;
	}

	for (iCntPlane = 0; iCntPlane < pinfo->planes; iCntPlane++)
	{
		pSrc = *pinfo->buffer[iCntPlane];
		iLineNum = pinfo->lengthPx;
		iColNum = pinfo->srcWidthPx;

		pDes= pImage = rx_mem_alloc(pinfo->dataSize);
		TrPrintfL(1, "data_malloc buffer: %p, size=%u MB, free=%u MB", pImage, (pinfo->dataSize) / 1024 / 1024, rx_mem_get_freeMB());
		if (pImage == NULL) return Error(ERR_CONT, 0, "No buffer for scaling: %d");

		switch (rotation)
		{
		default:
		case 0:
			memcpy(pDes, pSrc, pinfo->dataSize);
			break;
		case 90:
			pDes += iLineNum - 1;
			for (iLineCnt = 0; iLineCnt < iLineNum; iLineCnt++)
			{
				for (iColCnt = 0; iColCnt < iColNum; iColCnt++)
				{
					*pDes = *pSrc;
					pSrc++;
					pDes += iLineNum;
				}

				pDes -= (iColNum * iLineNum) + 1;
			}
			break;
		case 180:
			pDes += (iLineNum * iColNum) - 1;
			for (iLineCnt = 0; iLineCnt < iLineNum; iLineCnt++)
			{
				for (iColCnt = 0; iColCnt < iColNum; iColCnt++)
				{
					*pDes = *pSrc;
					pSrc++;
					pDes -= 1;
				}
			}
			break;
		case 270:
			pDes += (iColNum - 1) * iLineNum;
			for (iLineCnt = 0; iLineCnt < iLineNum; iLineCnt++)
			{
				for (iColCnt = 0; iColCnt < iColNum; iColCnt++)
				{
					*pDes = *pSrc;
					pSrc++;
					pDes -= iLineNum;
				}

				pDes += (iColNum * iLineNum) + 1;
			}

			break;
		}
		if (pinfo->buffer[iCntPlane]) rx_mem_free(pinfo->buffer[iCntPlane]);
		pinfo->buffer[iCntPlane] = &pImage;
		pImage = NULL;
	}

	pinfo->srcWidthPx = NewWidthPx;
	pinfo->lengthPx = NewlengthPx;

	return (REPLY_OK);
}


//--- rx_bmp_place ----------------------------------------------------------------------
int rx_bmp_place(SBmpInfo *pinfo, int columns, int colDist)
{
	UINT32 Gap, NewWidth, NewX, NewLinelen, l;
	int iCntPlane, c;
	BYTE* pImage, *pDes, *pSrc;

	if ((columns == 1) && (colDist== 0))
		return (REPLY_OK);

	Gap = colDist * pinfo->resol.x  / 25400 ;
	NewX = (Gap + pinfo->srcWidthPx);
	NewLinelen = NewWidth = columns * NewX;

	if (pinfo->bitsPerPixel == 4) {
		Gap = Gap + 1 / 2;
		NewX = NewX + 1 / 2;
		NewLinelen = columns * NewX;
		NewWidth = NewLinelen * 2;
	}
	else if (pinfo->bitsPerPixel == 2) {
		Gap = (Gap + 3) / 4;
		NewX = (NewX + 3) / 4;
		NewLinelen = columns * NewX;
		NewWidth = NewLinelen * 4;
	}
	else if (pinfo->bitsPerPixel == 1) {
		Gap = (Gap + 7) / 8;
		NewX = (NewX + 7) / 8;
		NewLinelen = columns * NewX;
		NewWidth = NewLinelen * 8;
	}

	for (iCntPlane = 0; iCntPlane < pinfo->planes; iCntPlane++)
	{
		pImage = rx_mem_alloc(NewLinelen*pinfo->lengthPx);
		TrPrintfL(1, "data_malloc buffer: %p, size=%u MB, free=%u MB", pImage, (NewLinelen*pinfo->lengthPx) / 1024 / 1024, rx_mem_get_freeMB());
		if (pImage == NULL) return Error(ERR_CONT, 0, "No buffer for placing: %d");

		for (l = 0; l < pinfo->lengthPx; l++)
		{
			pSrc = *pinfo->buffer[iCntPlane] + (l*pinfo->lineLen);
			for (c = 0; c < columns; c++)
			{
				pDes = pImage + (l*NewLinelen) + (c * NewX);
				memset(pDes, 0, Gap);
				pDes += Gap;
				memcpy(pDes, pSrc, pinfo->lineLen);
			}
		}
		if (pinfo->buffer[iCntPlane]) rx_mem_free(pinfo->buffer[iCntPlane]);
		pinfo->buffer[iCntPlane] = &pImage;
		pImage = NULL;
	}

	pinfo->srcWidthPx = NewWidth;
	pinfo->lineLen = NewLinelen;
	pinfo->dataSize = pinfo->lineLen*pinfo->lengthPx;

	return (REPLY_OK);
}


//--- rx_bmp_screen ----------------------------------------------------------------------
int rx_bmp_screen(SBmpInfo *pinfo, SScreenConfig* pscreenConfig)
{
	int iCntPlane;

	for (iCntPlane = 0; iCntPlane < pinfo->planes; iCntPlane++)
	{
		bmp_plane_screen(iCntPlane, pinfo, pscreenConfig);
	}

	pinfo->srcWidthPx = pinfo->srcWidthPx * 2;
	pinfo->lengthPx = pinfo->lengthPx * 2;
	pinfo->bitsPerPixel = 2;
	pinfo->lineLen = ((pinfo->srcWidthPx * 2) + 7) / 8;
	pinfo->dataSize = pinfo->lineLen*pinfo->lengthPx;
	pinfo->resol = pscreenConfig->outputResol;

	return (REPLY_OK);
}


//--- _bmp_plane_screen ----------------------------------------------------------------------
int bmp_plane_screen(SHORT plane, SBmpInfo *pinfo, SScreenConfig* pscreenConfig)
{
	switch (pscreenConfig->type)
	{
	case 0:
		return (bmp_plane_ED12screen(plane, pinfo, pscreenConfig));
	case 1:
		return (bmp_plane_TH12screen(plane, pinfo, pscreenConfig));

	default:
		Error(ERR_CONT, 0, "Screen type %d: <%s> not supported", pscreenConfig->type, pscreenConfig->typeName);
		return (REPLY_ERROR);
	}
}


//--- rx_screen_init ----------------------------------------------------------------------
int rx_screen_init(char * printEnv, SScreenConfig* pscreenConfig)
{
	char filepath [MAX_PATH];
	
	memset(pscreenConfig, 0, sizeof (SScreenConfig));

	sprintf(filepath, "%s%s/profile.ted", PATH_EMBRIP_PRENV, printEnv);

	// init lut tables
	if (rx_luts_load(filepath, RX_Color, SIZEOF(RX_Color), pscreenConfig->lut))
		Error(ERR_CONT, 0, "Lut tables maybe inconsistent, see PrintEnv file <%s/profile.ted>", printEnv);

	// TODO read parameters
	pscreenConfig->type = 1;

	switch (pscreenConfig->type)
	{
	case 0:
		return (rx_screen_ED12init(pscreenConfig));
	case 1:
		return (rx_screen_TH12init(pscreenConfig));

	default:
		Error(ERR_CONT, 0, "Screen type %d: <%s> not supported", pscreenConfig->type, pscreenConfig->typeName);
		return (REPLY_ERROR);
	}
}


//--- rx_tiff_rip --------------------------------------------------------------------------------------
int rx_tiff_rip(char * printEnv, char * inputpath, int rotation, int columns, int colDist, char * outputpath, char * jobName)
{

	SBmpInfo	BmpInfo;
	int		ret = REPLY_OK, i;
	char capture[MAX_PATH];
	SScreenConfig screenConfig;
	int	 time;

	time = rx_get_ticks();
	if (rx_screen_init(printEnv, &screenConfig)) return (REPLY_ERROR);
	TrPrintfL(1, "screen init time=%d", rx_get_ticks() - time);

	time = rx_get_ticks();
	memset(&BmpInfo, 0, sizeof(SBmpInfo));


	ret = tif_load_full(inputpath, RX_Color, SIZEOF(RX_Color), &BmpInfo);
	if (FALSE && !ret) {
		sprintf(capture, "%s.read", jobName);
		ret = tif_write(outputpath, capture, &BmpInfo);
	}
	TrPrintfL(1, "tiff load time=%d", rx_get_ticks() - time);

	//TODO other files than tif

	time = rx_get_ticks();
	if ((!ret) && (rotation))
	{
		ret = rx_bmp_rotate(&BmpInfo, rotation);
		TrPrintfL(1, "rotate time=%d", rx_get_ticks() - time);
		if (FALSE && !ret) {
			sprintf(capture, "%s.rotate", jobName);
			ret = tif_write(outputpath, capture, &BmpInfo);
		}
	}

	time = rx_get_ticks();
	if ((!ret) && BmpInfo.resol.x && BmpInfo.resol.y && ((BmpInfo.resol.x != screenConfig.inputResol.x) || (BmpInfo.resol.y != screenConfig.inputResol.y)))
	{
		SPoint NewDim;
		NewDim.x = BmpInfo.srcWidthPx*screenConfig.inputResol.x / BmpInfo.resol.x;
		NewDim.y = BmpInfo.lengthPx*screenConfig.inputResol.x / BmpInfo.resol.y;
		ret = rx_bmp_scale(&BmpInfo, NewDim, screenConfig.inputResol);
		TrPrintfL(1, "scale time=%d", rx_get_ticks() - time);
		if (FALSE && !ret) {
			sprintf(capture, "%s.scale", jobName);
			ret = tif_write(outputpath, capture, &BmpInfo);
		}
	}

	time = rx_get_ticks();
	if (!ret) { ret = rx_bmp_screen(&BmpInfo, &screenConfig); }
	TrPrintfL(1, "screen time=%d", rx_get_ticks() - time);

	time = rx_get_ticks();
	if ((!ret) && (colDist || (columns>1)))
	{
		ret = rx_bmp_place(&BmpInfo, columns, colDist);
		TrPrintfL(1, "place %d columns time=%d", columns, rx_get_ticks() - time);
		if (FALSE && !ret) {
			sprintf(capture, "%s.place", jobName);
			ret = tif_write(outputpath, capture, &BmpInfo);
		}
	}

	time = rx_get_ticks();
	if (!ret) { ret = tif_write(outputpath, jobName, &BmpInfo); }
	TrPrintfL(1, "write time=%d", rx_get_ticks() - time);

	for (i = 0; i<MAX_COLORS; i++)
		rx_mem_free(BmpInfo.buffer[i]);
	return ret;
}


//--- rx_pdf_init ----------------------------------------------------------------------
int rx_pdf_init(char * printEnv, SPdfConfig* ppdfConfig)
{
	memset(ppdfConfig, 0, sizeof (SPdfConfig));
	// Todo: fill struct  from xml file in printEnv
	return REPLY_OK;

}


//--- rx_pdf_rip --------------------------------------------------------------------------------------
int rx_pdf_rip (char * printEnv, char * inputpath, int rotation, int columns, int colDist, char * outputpath, char * jobName)
{
	SPdfConfig pdfConfig;
	int		ret = REPLY_OK, i;
	int argc = 0;
	char * argv[64];
	char outarg[256+32];
	char resol[32];
	void* instance;

	g_rotation = rotation;
	g_columns = columns;
	g_colDist = colDist;
	g_outputpath = outputpath;
	g_jobName = jobName;
	g_page = 1;

	rx_pdf_init(printEnv, &pdfConfig);


	rx_screen_init(printEnv, &g_screenConfig);

	memset(&g_pdfbmp, 0, sizeof (SBmpInfo));
	for (i = 0; i < 4; i++)
	{
		if (RX_Color[i].color.name[0] && RX_Color[i].lastLine)
		{
			g_pdfbmp.colorCode[i] = RX_Color[i].color.colorCode;
		}
		else
		{
			ret = Error(ERR_CONT, 0, "File %s: No color define for sample: %d", inputpath, i);
			return (REPLY_ERROR); 
		}
	}

	argv[argc++] = "rx_embrip";
	argv[argc++] = "-dNOPAUSE";
	argv[argc++] = "-dBATCH";
	argv[argc++] = "-q";
//	argv[argc++] = "-sDEVICE=bmpsep8";
	argv[argc++] = "-sDEVICE=tiff32nc";
//	argv[argc++] = "-sDEVICE=tiffsep";
	argv[argc++] = "-dPrinted";
	argv[argc++] = "-dNumRenderingThreads=8";
	sprintf(resol, "-r%dx%d", g_screenConfig.inputResol.x, g_screenConfig.inputResol.x);
	argv[argc++] = "-r600x600";
	sprintf(outarg, "-sOutputFile=%s%s.tif", PATH_TEMP, jobName);
	argv[argc++] = outarg;
	argv[argc++] = inputpath;

	instance = 0;
	if (gsapi_new_instance(&instance, 0) < 0)	{
		Error(ERR_CONT, 0, "Could not create ghostscript instance"); 
		return (REPLY_ERROR);
	}

	gsapi_set_stdio(instance, _embrip_stdin, _embrip_stdout, _embrip_stderr);
	gsapi_set_planeio(instance, _embrip_planealloc, _embrip_planedone);

	g_time = rx_get_ticks();
	ret = gsapi_init_with_args(instance, argc, argv);

	if ((ret != 0) && (ret != gs_error_Quit))
		Error(ERR_CONT, 0, "Could not start ghostscript instance");

	ret = gsapi_exit(instance);
	if ((ret != 0) && (ret != gs_error_Quit))
		Error(ERR_CONT, 0, "Error waiting ghostscript");
	gsapi_delete_instance(instance);

	// Free if not done
	for (i = 0; i < MAX_COLORS; i++)
		rx_mem_free(g_pdfbmp.buffer[i]);

	return (ret);
}


/* --- _embrip_stdin ------------------------------------------------------------ */
int GSDLLCALL _embrip_stdin(void *instance, char *buf, int len)
{
	if ((!buf) || (len <= 0))
		return (REPLY_ERROR);

	Error(ERR_CONT, 0, "No input in ghoscript expected");

	return (REPLY_OK);
}


/* --- _embrip_stdout ----------------------------------------------------------- */
static int GSDLLCALL _embrip_stdout(void *instance, const char *buf, int len)
{
	static char buffer[201];

	if ((!buf) || (len <= 0))
		return (REPLY_ERROR);

	memset(buffer, 0, 201);
	if (len < 201)
		memcpy(buffer, buf, len);
	else
		memcpy(buffer, buf, 200);

	TrPrintfL(1, "ghostscript output: %s", buffer);

	return (len);
}


/* --- _embrip_stderr ----------------------------------------------------------- */
static int GSDLLCALL _embrip_stderr(void *instance, const char *buf, int len)
{
	static char buffer[201];

	if ((!buf) || (len <= 0))
		return (REPLY_ERROR);

	memset(buffer, 0, 201);
	if (len < 201)
		memcpy(buffer, buf, len);
	else
		memcpy(buffer, buf, 200);

	TrPrintfL(1, "ghostscript error: %s", buffer);

	return (len);
}


/* --- _embrip_planealloc ----------------------------------------------------------- */
static int GSDLLCALL _embrip_planealloc(void *instance, int planes, int width, int heigth, char ***buf)
{
	int iCntPlane;
	if ((planes <= 0) || (width <= 0) || (heigth <= 0))
		return (REPLY_ERROR);

	// Clean previous if exists
	for (iCntPlane = 0; iCntPlane < MAX_COLORS; iCntPlane++)
		rx_mem_free(g_pdfbmp.buffer[iCntPlane]);

	// Alloc	
	for (iCntPlane = 0; iCntPlane < planes; iCntPlane++)
	{
		g_buffer[iCntPlane] = rx_mem_alloc((long long)width*(long long)heigth);
		g_pdfbmp.buffer[iCntPlane] = &g_buffer[iCntPlane];
		TrPrintfL(1, "data_malloc buffer: %p, width=%d, heigth=%d, size=%u MB, free=%u MB", g_buffer[iCntPlane], width, heigth, ((long long)width*(long long)heigth) / 1024 / 1024, rx_mem_get_freeMB());
		if (g_buffer[iCntPlane] == NULL) return Error(ERR_CONT, 0, "No buffer for pdf load: %d");
	}

	// Fill info	
	g_pdfbmp.bitsPerPixel = 8;
	g_pdfbmp.srcWidthPx = width;
	g_pdfbmp.lengthPx = heigth;
	g_pdfbmp.lineLen = width;
	g_pdfbmp.dataSize = g_pdfbmp.lineLen*g_pdfbmp.lengthPx;
	g_pdfbmp.aligment = 8;
	g_pdfbmp.planes = planes;
	g_pdfbmp.resol = g_screenConfig.inputResol;

	*buf = g_buffer;

	return (REPLY_OK);
}


/* --- _embrip_planedone ----------------------------------------------------------- */
static int GSDLLCALL _embrip_planedone(void *instance)
{
	int ret=0, i;
	char outname[MAX_PATH];


	if (FALSE && !ret) {
		sprintf(outname, "%s.ripped", g_jobName);
		ret = tif_write(g_outputpath, outname, &g_pdfbmp);
	}
	TrPrintfL(1, "pdf ripping time=%d", rx_get_ticks() - g_time);

	//TODO other files than tif

	g_time = rx_get_ticks();
	if (g_rotation)
	{
		ret = rx_bmp_rotate(&g_pdfbmp, g_rotation);
		TrPrintfL(1, "rotate time=%d", rx_get_ticks() - g_time);
		if (FALSE && !ret) {
			sprintf(outname, "%s.rotate", g_jobName);
			ret = tif_write(g_outputpath, outname, &g_pdfbmp);
		}
	}


	g_time = rx_get_ticks();
	if (!ret) { ret = rx_bmp_screen(&g_pdfbmp, &g_screenConfig); }
	TrPrintfL(1, "screen time=%d", rx_get_ticks() - g_time);

	g_time = rx_get_ticks();
	if ((!ret) && (g_colDist || (g_columns>1)))
	{
		ret = rx_bmp_place(&g_pdfbmp, g_columns, g_colDist);
		TrPrintfL(1, "place %d columns time=%d", g_columns, rx_get_ticks() - g_time);
		if (FALSE && !ret) {
			sprintf(outname, "%s.place", g_jobName);
			ret = tif_write(g_outputpath, outname, &g_pdfbmp);
		}
	}

	g_time = rx_get_ticks();
	sprintf(outname, "%s_p%.6d", g_jobName, g_page);

	if (!ret) { ret = tif_write(g_outputpath, outname, &g_pdfbmp); }
	TrPrintfL(1, "write time=%d", rx_get_ticks() - g_time);
	
	g_page++;
	for (i = 0; i<MAX_COLORS; i++)
		rx_mem_free(g_pdfbmp.buffer[i]);

	g_time = rx_get_ticks();
	return (REPLY_OK);
}
