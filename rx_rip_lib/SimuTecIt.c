
// #define SIMU_TECIT

#ifdef SIMU_TECIT

#include "tbarcode.h"

void BCGetErrorTextA(ERRCODE eCode, LPSTR szText, size_t nSize){};
ERRCODE BCAlloc(t_BarCode** pBarCode){return 0;};
ERRCODE TECIT_STDCALL BCFree(t_BarCode* pBarCode){return 0;};
ERRCODE TECIT_STDCALL BCLicenseMeA(LPCSTR lpszLicensee, e_licKind eKindOfLicense, DWORD dwNoOfLicenses, LPCSTR lpszKey, e_licProduct eProductID){return 0;};
ERRCODE TECIT_STDCALL BCCreate(t_BarCode* pBarCode){return 0;};
ERRCODE TECIT_STDCALL BCSetBCType(t_BarCode* pBarCode, e_BarCType eType){return 0;};
LPCSTR TECIT_STDCALL  BCGetTextA(t_BarCode* pBarCode){return 0;};
ERRCODE TECIT_STDCALL BCSetTextA(t_BarCode* pBarCode, LPCSTR szText, LONG nLen){return 0;};
LPCWSTR TECIT_STDCALL BCGetTextW(t_BarCode* pBarCode){return 0;};
ERRCODE TECIT_STDCALL BCSetTextW(t_BarCode* pBarCode, LPCWSTR szText, LONG nLen){return 0;};
e_EncodingMode TECIT_STDCALL BCGetEncodingMode(t_BarCode* pBarCode){return (e_EncodingMode)0;};
ERRCODE TECIT_STDCALL BCSetEncodingMode(t_BarCode* pBarCode, e_EncodingMode eMode){return 0;};
ERRCODE TECIT_STDCALL BCSetCodePage(t_BarCode* pBarCode, e_CodePage eCodePage){return 0;};
ERRCODE TECIT_STDCALL BCSetCodePageCustom(t_BarCode* pBarCode, UINT nCodePage){return 0;};
LPCSTR TECIT_STDCALL BCGetFormatA(t_BarCode* pBarCode){return NULL;};
ERRCODE TECIT_STDCALL BCSetFormatA(t_BarCode* pBarCode, LPCSTR szFormat){return 0;};
ERRCODE TECIT_STDCALL BCCheck(t_BarCode* pBarCode){return 0;};
e_Degree TECIT_STDCALL BCGetRotation(t_BarCode* pBarCode){return (e_Degree)0;};
ERRCODE TECIT_STDCALL BCSetRotation(t_BarCode* pBarCode, e_Degree eRotation){return 0;};
ERRCODE TECIT_STDCALL BCSetRatioA(t_BarCode* pBarCode, LPCSTR szRatio){return 0;};
LPCSTR TECIT_STDCALL BCGetRatioString(e_BarCType eType){return NULL;};
LONG TECIT_STDCALL BCGetCountBars(e_BarCType eBarCType){return 0;};
LONG TECIT_STDCALL BCGetCountSpaces(e_BarCType eBarCType){return 0;};
ERRCODE TECIT_STDCALL BCSetLogFont(t_BarCode*  pBarCode, const LOGFONTA* lf){return 0;};
ERRCODE TECIT_STDCALL BCSetPrintText(t_BarCode* pBarCode, BOOL bReadable, BOOL bAbove){return 0;};
ERRCODE TECIT_STDCALL BCSetCDMethod(t_BarCode* pBarCode, e_CDMethod eMethod){return 0;};
ERRCODE TECIT_STDCALL BCCalcCD(t_BarCode* pBarCode){return 0;};
LPCSTR TECIT_STDCALL BCGetOptionsA(t_BarCode* pBarCode, LPCSTR szFilter){return NULL;};
DOUBLE TECIT_STDCALL BCGet2D_XRows(const t_BarCode* pBarCode){return 0;};
LONG TECIT_STDCALL BCGet2D_XCols(const t_BarCode* pBarCode){return 0;};
LONG TECIT_STDCALL BCGetCountRows(const t_BarCode* pBarCode){return 0;};
ERRCODE TECIT_STDCALL BCSetOptionsA(t_BarCode* pBarCode, LPCSTR szOption){return 0;};
ERRCODE TECIT_STDCALL BCSetModWidthN(t_BarCode* pBarCode, DOUBLE moduleWidth){return 0;};
ERRCODE TECIT_STDCALL BCSetOptResolution(t_BarCode* pBarCode, BOOL bOpt){return 0;};
fn_DrawRow TECIT_STDCALL BCGetFuncDrawRow(t_BarCode* pBarCode){return (fn_DrawRow)0;};
ERRCODE TECIT_STDCALL BCSetFuncDrawRow(t_BarCode* pBarCode, fn_DrawRow fn){return 0;};
fn_DrawTextA TECIT_STDCALL BCGetFuncDrawTextA(t_BarCode* pBarCode){return (fn_DrawTextA)0;};
void TECIT_STDCALL BCSetCBData(t_BarCode* pBarCode, LPVOID pData);
ERRCODE TECIT_STDCALL BCDrawCB(t_BarCode* pBarCode, HDC hDC, RECT* pRect){return 0;};
LPCSTR TECIT_STDCALL BCGetMetaDataA(t_BarCode* pBarCode){return NULL;};
ERRCODE TECIT_STDCALL BCSet_PDF417_Rows(t_BarCode* pBarCode, LONG nRows){return 0;};
ERRCODE TECIT_STDCALL BCSet_PDF417_Columns(t_BarCode* pBarCode, LONG nRows){return 0;};
e_PDFMode TECIT_STDCALL BCGet_PDF417_EncodingMode(t_BarCode* pBarCode){return (e_PDFMode)0;};
ERRCODE TECIT_STDCALL BCSet_MPDF417_Mode(t_BarCode* pBarCode, e_MPDFMode eMode){return 0;};
ERRCODE TECIT_STDCALL BCSet_MPDF417_Version(t_BarCode* pBarCode, e_MPDFVers eVersion){return 0;};
ERRCODE TECIT_STDCALL BCSet_Maxi_Mode(t_BarCode* pBarCode, LONG nMode){return 0;};
ERRCODE TECIT_STDCALL BCSet_Maxi_UnderCut(t_BarCode* pBarCode, LONG nUndercut){return 0;};
ERRCODE TECIT_STDCALL BCSet_DM_Size(t_BarCode* pBarCode, e_DMSizes eSize){return 0;};
ERRCODE TECIT_STDCALL BCSet_DM_Rectangular(t_BarCode* pBarCode, BOOL bRect){return 0;};
ERRCODE TECIT_STDCALL BCSet_DM_Format(t_BarCode* pBarCode, e_DMFormat eFormat){return 0;};
ERRCODE TECIT_STDCALL BCSet_Aztec_Size(t_BarCode* pBarCode, e_AztecSizes eSize){return 0;};
ERRCODE TECIT_STDCALL BCSet_Aztec_EnforceBinaryEncoding(t_BarCode* pBarCode, BOOL bEnforceBinary){return 0;};
ERRCODE TECIT_STDCALL BCSet_Aztec_FormatW(t_BarCode* pBarCode, e_AztecFormat eFormat, LPCWSTR szFormatSpecifier){return 0;};
ERRCODE TECIT_STDCALL BCSet_Aztec_ErrorCorrection(t_BarCode* pBarCode, LONG nErrorCorrection){return 0;};
ERRCODE TECIT_STDCALL BCSet_Aztec_RuneMode(t_BarCode* pBarCode, BOOL bRuneMode){return 0;};
ERRCODE TECIT_STDCALL BCSet_QR_Version(t_BarCode* pBarCode, e_QRVersion eVersion){return 0;};
ERRCODE TECIT_STDCALL BCSet_QR_Format(t_BarCode* pBarCode, e_QRFormat eFormat){return 0;};
ERRCODE TECIT_STDCALL BCSet_QR_ECLevel(t_BarCode* pBarCode, e_QRECLevel eECLevel){return 0;};
ERRCODE TECIT_STDCALL BCSet_QR_Mask(t_BarCode* pBarCode, e_QRMask eMask){return 0;};
ERRCODE TECIT_STDCALL BCSet_MQR_Version(t_BarCode* pBarCode, e_MQRVersion eVersion){return 0;};
ERRCODE TECIT_STDCALL BCSet_MQR_Mask(t_BarCode* pBarCode, e_MQRMask eMask){return 0;};
ERRCODE TECIT_STDCALL BCSet2DCompositeComponent(t_BarCode* pBarCode, e_CCType eCCType){return 0;};
BOOL TECIT_STDCALL BCIsCompositeAllowed(e_BarCType barcodeType){return 0;};
ERRCODE TECIT_STDCALL BCSet_RSS_SegmPerRow(t_BarCode* pBarCode, LONG nSegmPerRow){return 0;};

ERRCODE TECIT_STDCALL BCSetFuncDrawTextA(t_BarCode* pBarCode, fn_DrawTextA fn){return 0;};
VOID TECIT_STDCALL BCSetCBData(t_BarCode* pBarCode, LPVOID pData){};
ERRCODE TECIT_STDCALL BCSet_PDF417_EncodingMode(t_BarCode* pBarCode, e_PDFMode eMode){return 0;};
#endif












