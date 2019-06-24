//----------------------------------------------------------------------------
/// \file 
/// Header File for TBarCode Library.
/// Include this header file in your modules to access the TBarCode API.
/// In Windows: 
/// \code
/// #include "tbarcode.h"
/// \endcode
/// In Linux/Unix:
/// \code
/// #include <libTBarCode11/tbarcode.h>
/// \endcode
/// \n
/// The TBarCode Library for Windows and TBarCode Library for UNIX have the same
/// API except for some functions:
/// - Functions denoted with "Windows only" are only available for Windows platforms.
/// - Functions denoted with "UNIX only" are only available in UNIX/Linux environments.
/// \n
/// \author TEC-IT Datenverarbeitung GmbH \n
///   Hans-Wagner-Str. 6, A-4400 Steyr, AUSTRIA/EUROPE \n
///   http://www.tec-it.com \n
///   Copyright &copy; 1996 - 2014 by TEC-IT Datenverarbeitung GmbH 
/// \version 11.0
/// \note This product is supported. Please send an email to support@tec-it.com
//----------------------------------------------------------------------------
#ifndef TBARCODE_HEADER
#  define TBARCODE_HEADER

//
// Define TEC_WIN symbols on Windows platforms OR TEC_UNIX on Linux/Unix platforms.
//
#if defined(WIN32) || defined(_WIN32) || defined(_WIN32_WCE) || defined(WIN64) || defined(_WIN64)
  /// Defined if platform is Windows.
  /// \internal
  #define TEC_WIN 1
  #if defined(UNDER_CE) || defined(_WIN32_WCE)
    /// Defined if platform is Windows CE.
    /// \internal
    #define TEC_WINCE 1
  #elif defined(WIN64) || defined(_WIN64)
    /// Defined if platform is a 32-bit Windows compatible.
    /// \internal   
    #define TEC_WIN32 1
    /// Defined if platform is a 64-bit Windows.
    /// \internal   
    #define TEC_WIN64 1
  #else
    /// Defined if platform is a 32-bit Windows compatible.
    /// \internal
    #define TEC_WIN32 1
  #endif
#else
  /// Defined if platform is Windows.
  /// \internal
  #define TEC_UNIX 1
#endif


#ifdef TEC_WIN  
  #include <windows.h>
#else
  #include <wchar.h>
  #include "TECITStd/TECDataTypes.h"
  #include "TECITStd/TECWinError.h"
#endif

/// Current TBarCode version.
#define TBARCODE_VERSION "11.0.0"
/// Symbol that identifies TBarCode 11.0.0.
#define TBARCODE_11_0_0

//
// Set symbol TECIT_STDCALL to appropriate calling convention statement.
//
#ifdef TEC_WIN
  #undef TECIT_STDCALL
  /// Defines calling convention.
  /// \internal
  #define TECIT_STDCALL _stdcall
#else
  #undef TECIT_STDCALL
  /// Defines calling convention.
  /// \internal
  #define TECIT_STDCALL
#endif

//
// Set symbol TBARCODEDLL_API to appropriate DLL export or import statement.
//
#if defined(TEC_WIN) && defined (TBARCODE_DLLEXPORT)
  /// Exports/imports DLL interface.
  /// \internal
	#define TBARCODEDLL_API 
  // Since we use a .def file, we don't need __declspec(dllexport) (Warning in VC++ 64-bit).
#elif defined(TEC_WIN) && !defined(TBARCODE_STATICLIB)
  /// Exports/imports DLL interface.
  /// \internal
	#define TBARCODEDLL_API  __declspec(dllimport)
#elif defined(TEC_MAC) && !defined(TBARCODE_STATICLIB)
  /// Exports/imports DLL interface.
  /// \internal
	#define TBARCODEDLL_API __attribute__((visibility("default")))
#else
  /// Exports/imports DLL interface.
  /// \internal
	#define TBARCODEDLL_API
#endif


#define TEC_MAX_INPUT 10000

#include "tecbcenum.h"


#ifdef __cplusplus
  extern "C"
  {
#endif


//____________________________________________________________________________
//
// Constants, Structures, Types
//____________________________________________________________________________
//

#ifndef S_OK
  #define S_OK      ((HRESULT) 0x00000000L)
  #define S_FALSE   ((HRESULT) 0x00000001L)
#endif

/// Constant indicating a default value.
/// Used for determining or setting the default value of most relevant parameters.
#define BC_USEDEFAULT   -1

/// Forward declaration of the internal used barcode structure.
/// The exact layout of this structure is hidden.
typedef struct tag_BarCode  t_BarCode;


/// Rectangle structure for double types.
typedef struct tagRECTD 
{
  DOUBLE left;
  DOUBLE top;
  DOUBLE right;
  DOUBLE bottom;
} RECTD, *LPRECTD;  


#ifndef ERRCODE
/// Data type for error codes.
/// Error code 0 indicates successful operation
#define ERRCODE LONG
#define ErrOk S_OK
#endif

#ifndef _BC_T_FUNCTIONS_
#define _BC_T_FUNCTIONS_
  #ifdef _UNICODE
    #define BCGetLibraryVersion BCGetLibraryVersionW
    #define BCInitLibrary BCInitLibraryW
    #define BCLicenseMe BCLicenseMeW
    #define BCGetBCList BCGetBCListW
    #define BCGetText BCGetTextW
    #define BCSetText BCSetTextW
    #define BCGetFormat BCGetFormatW
    #define BCSetFormat BCSetFormatW
    #define BCSetBarShape BCSetBarShapeW
    #define BCGetBarShape_ImageFileName BCGetBarShape_ImageFileNameW
    #define BCGetRatio BCGetRatioW
    #define BCSetRatio BCSetRatioW
    #define BCGetFontName BCGetFontNameW
    #define BCSetFontName BCSetFontNameW
    #define BCGetDisplayText BCGetDisplayTextW
    #define BCSetDisplayText BCSetDisplayTextW
    #define BCGetCheckDigits BCGetCheckDigitsW
    #define BCCalcCDExt BCCalcCDExtW
    #define BCGetNameFromEnum BCGetNameFromEnumW
    #define BCGetCDList BCGetCDListW
    #define BCGetModWidth BCGetModWidthW
    #define BCSetModWidth BCSetModWidthW
    #define BCCopyToClipboardEx BCCopyToClipboardExW
    #define BCSaveImage BCSaveImageW
    #define BCSaveImageEx BCSaveImageExW
    #define BCBitmapToFile BCBitmapToFileW
    #define BCBitmapToMemory BCBitmapToMemoryW
    #define BCPCLToFile BCPCLToFileW
    #define BCPCLToFileExt BCPCLToFileExtW
    #define BCPDFToFile BCPDFToFileW
    #define BCPostscriptToFile BCPostscriptToFileW
    #define BCGetPostscriptMacros BCGetPostscriptMacrosW
    #define BCSVGToFile BCSVGToFileW
    #define BCAIToFile BCAIToFileW
    #define BCGetMetaData BCGetMetaDataW
    #define BCGetErrorText BCGetErrorTextW
    #define BCGetErrorTextExt BCGetErrorTextExtW
    #define BCGet_PDF417_RowColRatio BCGet_PDF417_RowColRatioW
    #define BCSet_PDF417_RowColRatio BCSet_PDF417_RowColRatioW
    #define BCGet_PDF417_FileID BCGet_PDF417_FileIDW
    #define BCSet_PDF417_FileID BCSet_PDF417_FileIDW
    #define BCGet_PDF417_FileName BCGet_PDF417_FileNameW
    #define BCSet_PDF417_FileName BCSet_PDF417_FileNameW
    #define BCGet_PDF417_Sender BCGet_PDF417_SenderW
    #define BCSet_PDF417_Sender BCSet_PDF417_SenderW
    #define BCGet_PDF417_Addressee BCGet_PDF417_AddresseeW
    #define BCSet_PDF417_Addressee BCSet_PDF417_AddresseeW
    #define BCGet_MPDF417_Versions BCGet_MPDF417_VersionsW
    #define BCSet_Maxi_UsePreamble BCSet_Maxi_UsePreambleW
    #define BCGet_Maxi_PreambleDate BCGet_Maxi_PreambleDateW
    #define BCGet_Maxi_SCMServClass BCGet_Maxi_SCMServClassW
    #define BCGet_Maxi_SCMCountryCode BCGet_Maxi_SCMCountryCodeW
    #define BCGet_Maxi_SCMPostalCode BCGet_Maxi_SCMPostalCodeW
    #define BCSet_Maxi_SCM BCSet_Maxi_SCMW
    #define BCGetDataMatrixSizes BCGetDataMatrixSizesW
    #define BCGet_Aztec_FormatSpecifier BCGet_Aztec_FormatSpecifierW
    #define BCSet_Aztec_Format BCSet_Aztec_FormatW
    #define BCGet_Aztec_AppendTotal BCGet_Aztec_AppendTotalW
    #define BCGet_Aztec_AppendIndex BCGet_Aztec_AppendIndexW
    #define BCGet_Aztec_AppendMessageID BCGet_Aztec_AppendMessageIDW
    #define BCSet_Aztec_AppendData BCSet_Aztec_AppendDataW
    #define BCGetAztecSizes BCGetAztecSizesW
    #define BCGet_DotCode_Size BCGet_DotCode_SizeW
    #define BCSet_DotCode_Size BCSet_DotCode_SizeW
    #define BCGet_DotCode_DefaultSizeForMode BCGet_DotCode_DefaultSizeForModeW
    #define BCGet_DotCode_FormatSpecifier BCGet_DotCode_FormatSpecifierW
    #define BCSet_DotCode_Format BCSet_DotCode_FormatW
    #define BCGet_QR_FmtAppIndicator BCGet_QR_FmtAppIndicatorW
    #define BCSet_QR_FmtAppIndicator BCSet_QR_FmtAppIndicatorW
    #define BCCalcStructApp_Parity BCCalcStructApp_ParityW
    #define BCGetQRCodeVersions BCGetQRCodeVersionsW
    #define BCGetMQRCodeVersions BCGetMQRCodeVersionsW
    #define BCGetHanXinVersions BCGetHanXinVersionsW
    #define BCGetDefaultDataByType BCGetDefaultDataByTypeW
    #define fn_DrawText fn_DrawTextW
    #define BCGetFuncDrawText BCGetFuncDrawTextW
    #define BCSetFuncDrawText BCSetFuncDrawTextW
    #define BCSetOptions BCSetOptionsW
    #define BCGetOptions BCGetOptionsW
  #else
    #define BCGetLibraryVersion BCGetLibraryVersionA
    #define BCInitLibrary BCInitLibraryA
    #define BCLicenseMe BCLicenseMeA
    #define BCGetBCList BCGetBCListA
    #define BCGetText BCGetTextA
    #define BCSetText BCSetTextA
    #define BCGetFormat BCGetFormatA
    #define BCSetFormat BCSetFormatA
    #define BCSetBarShape BCSetBarShapeA
    #define BCGetBarShape_ImageFileName BCGetBarShape_ImageFileNameA
    #define BCGetRatio BCGetRatioA
    #define BCSetRatio BCSetRatioA
    #define BCGetFontName BCGetFontNameA
    #define BCSetFontName BCSetFontNameA
    #define BCGetDisplayText BCGetDisplayTextA
    #define BCSetDisplayText BCSetDisplayTextA
    #define BCGetCheckDigits BCGetCheckDigitsA
    #define BCCalcCDExt BCCalcCDExtA
    #define BCGetNameFromEnum BCGetNameFromEnumA
    #define BCGetCDList BCGetCDListA
    #define BCGetModWidth BCGetModWidthA
    #define BCSetModWidth BCSetModWidthA
    #define BCCopyToClipboardEx BCCopyToClipboardExA
    #define BCSaveImage BCSaveImageA
    #define BCSaveImageEx BCSaveImageExA
    #define BCBitmapToFile BCBitmapToFileA
    #define BCBitmapToMemory BCBitmapToMemoryA
    #define BCPCLToFile BCPCLToFileA
    #define BCPCLToFileExt BCPCLToFileExtA
    #define BCPDFToFile BCPDFToFileA
    #define BCPostscriptToFile BCPostscriptToFileA
    #define BCGetPostscriptMacros BCGetPostscriptMacrosA
    #define BCSVGToFile BCSVGToFileA
    #define BCAIToFile BCAIToFileA
    #define BCGetMetaData BCGetMetaDataA
    #define BCGetErrorText BCGetErrorTextA
    #define BCGetErrorTextExt BCGetErrorTextExtA
    #define BCGet_PDF417_RowColRatio BCGet_PDF417_RowColRatioA
    #define BCSet_PDF417_RowColRatio BCSet_PDF417_RowColRatioA
    #define BCGet_PDF417_FileID BCGet_PDF417_FileIDA
    #define BCSet_PDF417_FileID BCSet_PDF417_FileIDA
    #define BCGet_PDF417_FileName BCGet_PDF417_FileNameA
    #define BCSet_PDF417_FileName BCSet_PDF417_FileNameA
    #define BCGet_PDF417_Sender BCGet_PDF417_SenderA
    #define BCSet_PDF417_Sender BCSet_PDF417_SenderA
    #define BCGet_PDF417_Addressee BCGet_PDF417_AddresseeA
    #define BCSet_PDF417_Addressee BCSet_PDF417_AddresseeA
    #define BCGet_MPDF417_Versions BCGet_MPDF417_VersionsA
    #define BCSet_Maxi_UsePreamble BCSet_Maxi_UsePreambleA
    #define BCGet_Maxi_PreambleDate BCGet_Maxi_PreambleDateA
    #define BCGet_Maxi_SCMServClass BCGet_Maxi_SCMServClassA
    #define BCGet_Maxi_SCMCountryCode BCGet_Maxi_SCMCountryCodeA
    #define BCGet_Maxi_SCMPostalCode BCGet_Maxi_SCMPostalCodeA
    #define BCSet_Maxi_SCM BCSet_Maxi_SCMA
    #define BCGetDataMatrixSizes BCGetDataMatrixSizesA
    #define BCGet_Aztec_FormatSpecifier BCGet_Aztec_FormatSpecifierA
    #define BCSet_Aztec_Format BCSet_Aztec_FormatA
    #define BCGet_Aztec_AppendTotal BCGet_Aztec_AppendTotalA
    #define BCGet_Aztec_AppendIndex BCGet_Aztec_AppendIndexA
    #define BCGet_Aztec_AppendMessageID BCGet_Aztec_AppendMessageIDA
    #define BCSet_Aztec_AppendData BCSet_Aztec_AppendDataA
    #define BCGetAztecSizes BCGetAztecSizesA
    #define BCGet_DotCode_Size BCGet_DotCode_SizeA
    #define BCSet_DotCode_Size BCSet_DotCode_SizeA
    #define BCGet_DotCode_DefaultSizeForMode BCGet_DotCode_DefaultSizeForModeA
    #define BCGet_DotCode_FormatSpecifier BCGet_DotCode_FormatSpecifierA
    #define BCSet_DotCode_Format BCSet_DotCode_FormatA
    #define BCGet_QR_FmtAppIndicator BCGet_QR_FmtAppIndicatorA
    #define BCSet_QR_FmtAppIndicator BCSet_QR_FmtAppIndicatorA
    #define BCCalcStructApp_Parity BCCalcStructApp_ParityA
    #define BCGetQRCodeVersions BCGetQRCodeVersionsA
    #define BCGetMQRCodeVersions BCGetMQRCodeVersionsA
    #define BCGetHanXinVersions BCGetHanXinVersionsA
    #define BCGetDefaultDataByType BCGetDefaultDataByTypeA
    #define fn_DrawText fn_DrawTextA
    #define BCGetFuncDrawText BCGetFuncDrawTextA
    #define BCSetFuncDrawText BCSetFuncDrawTextA
    #define BCSetOptions BCSetOptionsA
    #define BCGetOptions BCGetOptionsA
  #endif  // _UNICODE
#endif    // _BC_T_FUNCTIONS_

//____________________________________________________________________________
//
// Misc Function Calls
//____________________________________________________________________________
//
//@{
TBARCODEDLL_API ERRCODE TECIT_STDCALL BCGetLibraryVersionA(INT* pMajor, INT* pMinor, INT* pRelease, INT* pRevision, LPCSTR* pszVersion, LPCSTR* pszBuildDate);
TBARCODEDLL_API ERRCODE TECIT_STDCALL BCGetLibraryVersionW(INT* pMajor, INT* pMinor, INT* pRelease, INT* pRevision, LPCWSTR* pszVersion, LPCWSTR* pszBuildDate);
//@}



/**
 * Callback-function which is called instead of drawing a barcode row.
 * This callback function can be set with BCSetFuncDrawRow(). When set the drawing 
 * functions such as BCDraw(), BCPostscriptToFile, BCPCLToFile() etc. do not generate 
 * any output. Instead they call the active callback function.\n
 * When Windows-GDI drawing is started (with BCDraw()) then drawDC, targetDC contain the 
 * Windows device contexts (if set). \n
 * The parameters x, y, width, and height specify the extent
 * of the barcode row. If drawDC and targetDC are not set then these variables are given in millimeters,
 * where the origin of the coordinate system is the upper, left coordinate of the barcode. The positive X-axis
 * points to the right and the positive Y-axis points downwards.\n
 * If drawDC or targetDC are set then the parameters x, y, width, and height use the current logical 
 * coordinate system and the logical units of these device contexts.\n
 * In your callback function you can use BCGetMetaData() to query the bars and spaces of the current row.
 * \note A barcode row in this case is a single row of bars. Linear (1-dimensional) barcodes 
 * have exactly one row of bars. 2-dimensional barcodes can have more than one row of bars: In this 
 * case this callback function is called once for each row.
 * \note The user-defined data (customData) is shared by all callback functions.
 * \note The TBarCode Callback functions can be used only with a developer license. Sample code is available on request: support@tec-it.com
 * \param[in,out]  customData  User-defined data.
 * \param[in,out]  barcode     The handle of the barcode.
 * \param[in,out]  drawDC      The device context.
 * \param[in,out]  targetDC    The target device context.
 * \param[in]  x       The left coordinate of the row.
 * \param[in]  y       The top coordinate of the row.
 * \param[in]  width   The width of the row.
 * \param[in]  height  The height of the row.
 * \retval     ErrOk   Everything went fine, and the drawing continues.
 * \retval     !=ErrOk Something went wrong, the drawing function returns with the returned error value.
 * \see BCGetFuncDrawRow(), BCSetFuncDrawRow(), BCDrawCB(), BCGetMetaData()
 */
typedef ERRCODE (CALLBACK *fn_DrawRow) (VOID* customData, t_BarCode* barcode, HDC drawDC, HDC targetDC, DOUBLE x, DOUBLE y, DOUBLE width, DOUBLE height);


/**
 * Data type for custom bar-drawing callback-function.
 * This callback function can be set with BCSetFuncDrawBar(). When set the drawing 
 * functions such as BCDraw(), BCPostscriptToFile, BCPCLToFile() etc. do not generate 
 * any output. Instead they call the active callback function. This function is called once for each bar.\n
 * When Windows-GDI drawing is started (with BCDraw()) then drawDC contains the 
 * Windows device contexts (if set). The variables x, y, width, and height specify the extent
 * of the bar. If drawDC and targetDC are not set then these variables are given in millimeters,
 * where the origin of the coordinate system is the upper, left coordinate of the barcode. The positive X-axis
 * points to the right and the positive Y-axis points downwards.\n
 * If drawDC or targetDC are set then the parameters x, y, width, and height use the current logical 
 * coordinate system and the logical units of these device contexts.\n
 * \note The user-defined data (customData) is shared by all callback functions.
 * \note The TBarCode Callback functions can be used only with a developer license. Sample code is available on request: support@tec-it.com
 * \param[in,out]  customData  User-defined data.
 * \param[in,out]  drawDC      The device context.
 * \param[in]  x       The left coordinate of the row.
 * \param[in]  y       The top coordinate of the row.
 * \param[in]  width   The width of the row.
 * \param[in]  height  The height of the row.
 * \retval     ErrOk   Everything went fine, and the drawing continues.
 * \retval     !=ErrOk Something went wrong, the drawing function returns with the returned error value.
 * \see BCGetFuncDrawBar(), BCSetFuncDrawBar(), BCDrawCB()
 */
typedef ERRCODE (CALLBACK *fn_DrawBar)  (VOID* customData, HDC drawDC, DOUBLE x, DOUBLE y, DOUBLE width, DOUBLE height);


/**
 * Data type for custom human readable text drawing callback-function.
 * This callback function can be set with BCSetFuncDrawText(). When set the drawing 
 * functions such as BCDraw(), BCPostscriptToFile, BCPCLToFile() etc. do not generate 
 * any output. Instead they call the active callback function.\n
 * When Windows-GDI drawing is started (with BCDraw()) then drawDC contains the 
 * Windows device contexts (if set). The variables x, and y specify the position
 * of the barcode text. If drawDC and targetDC are not set then these variables are given in millimeters,
 * where the origin of the coordinate system is the upper, left coordinate of the barcode. The positive X-axis
 * points to the right and the positive Y-axis points downwards.\n
 * If drawDC or targetDC are set then the parameters x and y use the current logical 
 * coordinate system and the logical units of these device contexts.\n
 * \note The user-defined data (customData) is shared by all callback functions.
 * \param[in,out]  customData  User-defined data.
 * \param[in,out]  drawDC      The device context.
 * \param[in]  font    The LOGFONTA structure that defines the font.
 * \param[in]  x       The x-coordinate of the text. Depending on the alignment, the x specifies either the left, the center or the right coordinate of the text.
 * \param[in]  y       The top coordinate of the text.
 * \param[in]  alignment The horizontal alignment of the text. (The vertical alignment is always 'top'.)
 * \param[in]  text    The output string.
 * \param[in]  numberOfCharacters   The number of characters in text.
 * \param[in]  drawArea   The draw area for the text.
 * \param[in]  textIndex  The index of given text.
 *                        0  ... means that given text is the <i>human readable text</i>.
 *                        >0 ... means that given text is the <i>extra text</i> with given index [1 .. n].
 * \retval     ErrOk   Everything went fine, and the drawing continues.
 * \retval     !=ErrOk Something went wrong, the drawing function returns with the returned error value.
 * \see BCGetFuncDrawText(), BCSetFuncDrawText(), BCDrawCB()
 */
typedef ERRCODE (CALLBACK *fn_DrawTextA) (VOID* customData, HDC drawDC, LPLOGFONTA font, DOUBLE x, DOUBLE y, e_BCAlign alignment, LPCSTR text, INT numberOfCharacters, LPRECTD drawArea, LONG textIndex);
typedef ERRCODE (CALLBACK *fn_DrawTextW) (VOID* customData, HDC drawDC, LPLOGFONTW font, DOUBLE x, DOUBLE y, e_BCAlign alignment, LPCWSTR text, INT numberOfCharacters, LPRECTD drawArea, LONG textIndex);

/**
 * Data type for custom MaxiCode finder drawing callback-function.
 * This callback function can be set with BCGetFuncDrawRing(). When set the MaxiCode
 * finder drawing functions do not generate any output. Instead they call the active callback function.\n
 * When Windows-GDI drawing is started (with BCDraw()) then drawDC contains the 
 * Windows device contexts (if set). The variables x, and y specify the position
 * of center of the ring. If drawDC and targetDC are not set then these variables are given in millimeters,
 * where the origin of the coordinate system is the upper, left coordinate of the barcode. The positive X-axis
 * points to the right and the positive Y-axis points downwards.\n
 * If drawDC or targetDC are set then the parameters x and y use the current logical 
 * coordinate system and the logical units of these device contexts.\n
 * \note The user-defined data (customData) is shared by all callback functions.
 * \param[in,out]  customData  User-defined data.
 * \param[in,out]  drawDC      The device context.
 * \param[in]  x       The x-coordinate (=horizontal) of the ring center.
 * \param[in]  y       The y-coordinate (=vertical) of the ring center.
 * \param[in]  rInner  The radius of the inner margin of the ring.
 * \param[in]  rOuter  The radius of the outer margin of the ring.
 * \retval     ErrOk   Everything went fine, and the drawing continues.
 * \retval     !=ErrOk Something went wrong, the drawing function returns with the returned error value.
 * \see BCGetFuncDrawRing(), BCSetFuncDrawRing(), BCDrawCB()
 */
typedef ERRCODE (CALLBACK *fn_DrawRing) (VOID* customData, HDC drawDC, DOUBLE x, DOUBLE y, DOUBLE rInner, DOUBLE rOuter);

/**
 * Data type for custom MaxiCode hexagon drawing callback-function.
 * This callback function can be set with BCSetFuncDrawHexagon(). When set the MaxiCode
 * hexagon drawing functions do not generate any output. Instead they call the active callback function.\n
 * When Windows-GDI drawing is started (with BCDraw()) then drawDC contains the 
 * Windows device contexts (if set). The variables x, and y specify the position
 * of the center of the hexagon. If drawDC and targetDC are not set then these variables are given in millimeters,
 * where the origin of the coordinate system is the upper, left coordinate of the barcode. The positive X-axis
 * points to the right and the positive Y-axis points downwards.\n
 * If drawDC or targetDC are set then the parameters x and y use the current logical 
 * coordinate system and the logical units of these device contexts.\n
 * \note The user-defined data (customData) is shared by all callback functions.
 * \param[in,out]  customData  User-defined data.
 * \param[in,out]  drawDC      The device context.
 * \param[in]  points_x  An array of the 6 x-positions of the hexagon corners (clockwise).
 * \param[in]  points_y  An array of the 6 y-positions of the hexagon corners (clockwise).
 * \retval     ErrOk   Everything went fine, and the drawing continues.
 * \retval     !=ErrOk Something went wrong, the drawing function returns with the returned error value.
 * \see BCGetFuncDrawHexagon(), BCSetFuncDrawHexagon(), BCDrawCB()
 */
typedef ERRCODE (CALLBACK *fn_DrawHexagon) (VOID* customData, HDC drawDC, DOUBLE points_x[6], DOUBLE points_y[6]);


//____________________________________________________________________________
//
// Required Function Calls
//____________________________________________________________________________
//
#if defined(TEC_WIN32) || defined(TEC_WINCE)
TBARCODEDLL_API BOOL TECIT_STDCALL BCAttach();
TBARCODEDLL_API BOOL TECIT_STDCALL BCAttachInstance(HINSTANCE hInstance);
TBARCODEDLL_API BOOL TECIT_STDCALL BCDetach();
#endif 

#if defined(TEC_UNIX) || defined(TBARCODE_STATICLIB)
//@{
  TBARCODEDLL_API ERRCODE TECIT_STDCALL BCInitLibraryA(LPCSTR szPath);
  TBARCODEDLL_API ERRCODE TECIT_STDCALL BCInitLibraryW(LPCWSTR szPath);
  TBARCODEDLL_API ERRCODE TECIT_STDCALL _BCInitLibraryT(LPCTSTR szPath);
//@}
  TBARCODEDLL_API ERRCODE TECIT_STDCALL BCDeInitLibrary();
#endif 

TBARCODEDLL_API ERRCODE TECIT_STDCALL BCAlloc(t_BarCode** pBarCode);
TBARCODEDLL_API ERRCODE TECIT_STDCALL BCClone(t_BarCode** ppBarCodeDest, const t_BarCode* pBarCodeSrc);
TBARCODEDLL_API ERRCODE TECIT_STDCALL BCFree(t_BarCode* pBarCode);

//@{
TBARCODEDLL_API ERRCODE TECIT_STDCALL BCLicenseMeA(LPCSTR lpszLicensee, e_licKind eKindOfLicense, 
                                                DWORD dwNoOfLicenses, LPCSTR lpszKey, e_licProduct eProductID);
TBARCODEDLL_API ERRCODE TECIT_STDCALL BCLicenseMeW(LPCWSTR lpszLicensee, e_licKind eKindOfLicense, 
                                                DWORD dwNoOfLicenses, LPCWSTR lpszKey, e_licProduct eProductID);
//@}

TBARCODEDLL_API ERRCODE TECIT_STDCALL BCCreate(t_BarCode* pBarCode);



//____________________________________________________________________________
//
// Barcode Settings
//____________________________________________________________________________
//

//
// ----- Barcode > Barcode Type
//
TBARCODEDLL_API e_BarCType TECIT_STDCALL BCGetBCType(t_BarCode* pBarCode);
TBARCODEDLL_API ERRCODE TECIT_STDCALL BCSetBCType(t_BarCode* pBarCode, e_BarCType eType);

//@{
TBARCODEDLL_API LPCSTR* TECIT_STDCALL BCGetBCListA();
TBARCODEDLL_API LPCWSTR* TECIT_STDCALL BCGetBCListW();
//@}
TBARCODEDLL_API LONG TECIT_STDCALL BCGetBCCount();

//
// ----- Barcode > Barcode Data
//
//@{
TBARCODEDLL_API LPCSTR TECIT_STDCALL BCGetTextA(t_BarCode* pBarCode);
TBARCODEDLL_API LPCWSTR TECIT_STDCALL BCGetTextW(t_BarCode* pBarCode);
//@}
//@{
TBARCODEDLL_API ERRCODE TECIT_STDCALL BCSetTextA(t_BarCode* pBarCode, LPCSTR szText, LONG nLen);
TBARCODEDLL_API ERRCODE TECIT_STDCALL BCSetTextW(t_BarCode* pBarCode, LPCWSTR szText, LONG nLen);
//@}
TBARCODEDLL_API BOOL TECIT_STDCALL BCIsTextUnicode(t_BarCode* pBarCode);
TBARCODEDLL_API LONG TECIT_STDCALL BCGetTextLen(t_BarCode* pBarCode);
TBARCODEDLL_API LONG TECIT_STDCALL BCGetTextLenCompressed(t_BarCode* pBarCode);

TBARCODEDLL_API e_EncodingMode TECIT_STDCALL BCGetEncodingMode(t_BarCode* pBarCode);
TBARCODEDLL_API ERRCODE TECIT_STDCALL BCSetEncodingMode(t_BarCode* pBarCode, e_EncodingMode eMode);
TBARCODEDLL_API e_CodePage TECIT_STDCALL BCGetCodePage(t_BarCode* pBarCode);
TBARCODEDLL_API UINT TECIT_STDCALL BCGetCodePageCustom(t_BarCode* pBarCode);
TBARCODEDLL_API ERRCODE TECIT_STDCALL BCSetCodePage(t_BarCode* pBarCode, e_CodePage eCodePage);
TBARCODEDLL_API ERRCODE TECIT_STDCALL BCSetCodePageCustom(t_BarCode* pBarCode, UINT nCodePage);

TBARCODEDLL_API e_Compression TECIT_STDCALL BCGetCompression(t_BarCode* pBarCode);
TBARCODEDLL_API ERRCODE TECIT_STDCALL BCSetCompression(t_BarCode* pBarCode, e_Compression eCompression);

//@{
TBARCODEDLL_API LPCSTR TECIT_STDCALL BCGetFormatA(t_BarCode* pBarCode);
TBARCODEDLL_API LPCWSTR TECIT_STDCALL BCGetFormatW(t_BarCode* pBarCode);
//@}
//@{
TBARCODEDLL_API ERRCODE TECIT_STDCALL BCSetFormatA(t_BarCode* pBarCode, LPCSTR szFormat);
TBARCODEDLL_API ERRCODE TECIT_STDCALL BCSetFormatW(t_BarCode* pBarCode, LPCWSTR szFormat);
//@}

TBARCODEDLL_API BOOL TECIT_STDCALL BCGetTranslateEsc(t_BarCode* pBarCode);
TBARCODEDLL_API ERRCODE TECIT_STDCALL BCSetTranslateEsc(t_BarCode* pBarCode, BOOL bTranslate);

TBARCODEDLL_API ERRCODE TECIT_STDCALL BCCheck(t_BarCode* pBarCode);
TBARCODEDLL_API LONG TECIT_STDCALL BCGetMaxLenOfData(e_BarCType eType);

TBARCODEDLL_API BOOL TECIT_STDCALL BCGetAutoCorrect(t_BarCode* pBarCode);
TBARCODEDLL_API ERRCODE TECIT_STDCALL BCSetAutoCorrect(t_BarCode* pBarCode, BOOL bAutoCorrect);

TBARCODEDLL_API LPCSTR TECIT_STDCALL BCGetDefaultDataByTypeA(e_BarCType eBCType);
TBARCODEDLL_API LPCWSTR TECIT_STDCALL BCGetDefaultDataByTypeW(e_BarCType eBCType);

//
// ----- Barcode > Appearance
//
TBARCODEDLL_API LONG TECIT_STDCALL BCGetBkMode(t_BarCode* pBarCode);
TBARCODEDLL_API ERRCODE TECIT_STDCALL BCSetBkMode(t_BarCode* pBarCode, LONG nMode);

TBARCODEDLL_API LONG TECIT_STDCALL BCGetBearerBarWidth(t_BarCode* pBarCode);
TBARCODEDLL_API ERRCODE TECIT_STDCALL BCSetBearerBarWidth(t_BarCode* pBarCode, LONG nBearerWidth);

TBARCODEDLL_API e_BearerBar TECIT_STDCALL BCGetBearerBarType(t_BarCode* pBarCode);
TBARCODEDLL_API ERRCODE TECIT_STDCALL BCSetBearerBarType(t_BarCode* pBarCode, e_BearerBar eType);

TBARCODEDLL_API BOOL TECIT_STDCALL BCGetMirror(t_BarCode* pBarCode);
TBARCODEDLL_API ERRCODE TECIT_STDCALL BCSetMirror(t_BarCode* pBarCode, BOOL bMirror);

TBARCODEDLL_API LONG TECIT_STDCALL BCGetNotchHeight(t_BarCode* pBarCode);
TBARCODEDLL_API ERRCODE TECIT_STDCALL BCSetNotchHeight(t_BarCode* pBarCode, LONG nHeight);

TBARCODEDLL_API e_Degree TECIT_STDCALL BCGetRotation(t_BarCode* pBarCode);
TBARCODEDLL_API ERRCODE TECIT_STDCALL BCSetRotation(t_BarCode* pBarCode, e_Degree eRotation);

TBARCODEDLL_API e_BarShape TECIT_STDCALL BCGetBarShape(t_BarCode* pBarCode);
TBARCODEDLL_API LPCSTR TECIT_STDCALL BCGetBarShape_ImageFileNameA(t_BarCode* pBarCode);
TBARCODEDLL_API LPCWSTR TECIT_STDCALL BCGetBarShape_ImageFileNameW(t_BarCode* pBarCode);
TBARCODEDLL_API ERRCODE TECIT_STDCALL BCSetBarShapeA(t_BarCode* pBarCode, e_BarShape eShape, LPCSTR szImageFile);
TBARCODEDLL_API ERRCODE TECIT_STDCALL BCSetBarShapeW(t_BarCode* pBarCode, e_BarShape eShape, LPCWSTR szImageFile);
TBARCODEDLL_API e_BarShape TECIT_STDCALL BCGetDefaultShapeForBCType(t_BarCode* pBarCode);

//
// ----- Barcode > Colors
//
TBARCODEDLL_API COLORREF TECIT_STDCALL BCGetColorBC(t_BarCode* pBarCode);
TBARCODEDLL_API ERRCODE TECIT_STDCALL BCSetColorBC(t_BarCode* pBarCode, COLORREF color);

TBARCODEDLL_API COLORREF TECIT_STDCALL BCGetColorFont(t_BarCode* pBarCode);
TBARCODEDLL_API ERRCODE TECIT_STDCALL BCSetColorFont(t_BarCode* pBarCode, COLORREF color);

TBARCODEDLL_API COLORREF TECIT_STDCALL BCGetColorBk(t_BarCode* pBarCode);
TBARCODEDLL_API ERRCODE TECIT_STDCALL BCSetColorBk(t_BarCode* pBarCode, COLORREF color);

TBARCODEDLL_API e_ColorMode TECIT_STDCALL BCGetColorMode(const t_BarCode* pBarcode);
TBARCODEDLL_API VOID TECIT_STDCALL BCSetColorMode(t_BarCode* pBarcode, e_ColorMode mode);

TBARCODEDLL_API ERRCODE TECIT_STDCALL BCSetColorBC(t_BarCode* pBarcode, COLORREF color);
TBARCODEDLL_API COLORREF TECIT_STDCALL BCGetColorBC(t_BarCode* pBarCode);

TBARCODEDLL_API VOID TECIT_STDCALL BCSetBarcodeColorCMYK(t_BarCode* pBarcode, DOUBLE dCyan, DOUBLE dMagenta, DOUBLE dYellow, DOUBLE dBlack);
TBARCODEDLL_API VOID TECIT_STDCALL BCGetBarcodeColorCMYK(const t_BarCode* pBarcode, DOUBLE* pdCyan, DOUBLE* pdMagenta, DOUBLE* pdYellow, DOUBLE* pdBlack);

TBARCODEDLL_API VOID TECIT_STDCALL BCSetBarcodeColorGDI(t_BarCode* pBarcode, COLORREF color);
TBARCODEDLL_API VOID TECIT_STDCALL BCGetBarcodeColorGDI(const t_BarCode* pBarcode, COLORREF* pColor);

TBARCODEDLL_API VOID TECIT_STDCALL BCSetBarcodeColorGray(t_BarCode* pBarcode, DOUBLE dGray);
TBARCODEDLL_API VOID TECIT_STDCALL BCGetBarcodeColorGray(const t_BarCode* pBarcode, DOUBLE* pdGray);

TBARCODEDLL_API VOID TECIT_STDCALL BCSetBarcodeColorRGB(t_BarCode* pBarcode, DOUBLE dRed, DOUBLE dGreen, DOUBLE dBlue);
TBARCODEDLL_API VOID TECIT_STDCALL BCGetBarcodeColorRGB(const t_BarCode* pBarcode, DOUBLE* pdRed, DOUBLE* pdGreen, DOUBLE* pdBlue);

TBARCODEDLL_API VOID TECIT_STDCALL BCSetBackgroundColorCMYK(t_BarCode* pBarcode, DOUBLE dCyan, DOUBLE dMagenta, DOUBLE dYellow, DOUBLE dBlack);
TBARCODEDLL_API VOID TECIT_STDCALL BCGetBackgroundColorCMYK(const t_BarCode* pBarcode, DOUBLE* pdCyan, DOUBLE* pdMagenta, DOUBLE* pdYellow, DOUBLE* pdBlack);

TBARCODEDLL_API VOID TECIT_STDCALL BCSetBackgroundColorGDI(t_BarCode* pBarcode, COLORREF color);
TBARCODEDLL_API VOID TECIT_STDCALL BCGetBackgroundColorGDI(const t_BarCode* pBarcode, COLORREF* pColor);

TBARCODEDLL_API VOID TECIT_STDCALL BCSetBackgroundColorGray(t_BarCode* pBarcode, DOUBLE dGray);
TBARCODEDLL_API VOID TECIT_STDCALL BCGetBackgroundColorGray(const t_BarCode* pBarcode, DOUBLE* pdGray);

TBARCODEDLL_API VOID TECIT_STDCALL BCSetBackgroundColorRGB(t_BarCode* pBarcode, DOUBLE dRed, DOUBLE dGreen, DOUBLE dBlue);
TBARCODEDLL_API VOID TECIT_STDCALL BCGetBackgroundColorRGB(const t_BarCode* pBarcode, DOUBLE* pdRed, DOUBLE* pdGreen, DOUBLE* pdBlue);

TBARCODEDLL_API VOID TECIT_STDCALL BCSetFontColorCMYK(t_BarCode* pBarcode, DOUBLE dCyan, DOUBLE dMagenta, DOUBLE dYellow, DOUBLE dBlack);
TBARCODEDLL_API VOID TECIT_STDCALL BCGetFontColorCMYK(const t_BarCode* pBarcode, DOUBLE* pdCyan, DOUBLE* pdMagenta, DOUBLE* pdYellow, DOUBLE* pdBlack);

TBARCODEDLL_API VOID TECIT_STDCALL BCSetFontColorGDI(t_BarCode* pBarcode, COLORREF color);
TBARCODEDLL_API VOID TECIT_STDCALL BCGetFontColorGDI(const t_BarCode* pBarcode, COLORREF* pColor);

TBARCODEDLL_API VOID TECIT_STDCALL BCSetFontColorGray(t_BarCode* pBarcode, DOUBLE dGray);
TBARCODEDLL_API VOID TECIT_STDCALL BCGetFontColorGray(const t_BarCode* pBarcode, DOUBLE* pdGray);

TBARCODEDLL_API VOID TECIT_STDCALL BCSetFontColorRGB(t_BarCode* pBarcode, DOUBLE dRed, DOUBLE dGreen, DOUBLE dBlue);
TBARCODEDLL_API VOID TECIT_STDCALL BCGetFontColorRGB(const t_BarCode* pBarcode, DOUBLE* pdRed, DOUBLE* pdGreen, DOUBLE* pdBlue);


//
// ----- Barcode > Print Ratio
//
//@{
TBARCODEDLL_API LPCSTR TECIT_STDCALL BCGetRatioA(t_BarCode* pBarCode);
TBARCODEDLL_API LPCWSTR TECIT_STDCALL BCGetRatioW(t_BarCode* pBarCode);
//@}
//@{
TBARCODEDLL_API ERRCODE TECIT_STDCALL BCSetRatioA(t_BarCode* pBarCode, LPCSTR szRatio);
TBARCODEDLL_API ERRCODE TECIT_STDCALL BCSetRatioW(t_BarCode* pBarCode, LPCWSTR szRatio);
//@}

TBARCODEDLL_API LPCSTR TECIT_STDCALL BCGetRatioHint(e_BarCType eType);
TBARCODEDLL_API LPCSTR TECIT_STDCALL BCGetRatioString(e_BarCType eType);

TBARCODEDLL_API LONG TECIT_STDCALL BCGetCountBars(e_BarCType eBarCType);

TBARCODEDLL_API LONG TECIT_STDCALL BCGetCountSpaces(e_BarCType eBarCType);

//
// ----- Barcode > Quiet Zone
//
TBARCODEDLL_API ERRCODE TECIT_STDCALL BCSetQuietZone(t_BarCode* pBarCode, LPRECTD prQuietZone, e_MUnit eQZMUnit);
TBARCODEDLL_API const LPRECTD TECIT_STDCALL BCGetQuietZone(t_BarCode* pBarCode);
TBARCODEDLL_API e_MUnit TECIT_STDCALL BCGetQuietZoneUnit(t_BarCode* pBarCode);

TBARCODEDLL_API BOOL TECIT_STDCALL BCGetUseQuietZoneDefaultColor(t_BarCode* pBarCode, e_QuietZone eZone);

TBARCODEDLL_API VOID TECIT_STDCALL BCSetQuietZoneColorCMYK(t_BarCode* pBarcode, e_QuietZone eZone, BOOL bUseDefault, DOUBLE dCyan, DOUBLE dMagenta, DOUBLE dYellow, DOUBLE dBlack);
TBARCODEDLL_API VOID TECIT_STDCALL BCGetQuietZoneColorCMYK(const t_BarCode* pBarcode, e_QuietZone eZone, DOUBLE* pdCyan, DOUBLE* pdMagenta, DOUBLE* pdYellow, DOUBLE* pdBlack);

TBARCODEDLL_API VOID TECIT_STDCALL BCSetQuietZoneColorGDI(t_BarCode* pBarcode, e_QuietZone eZone, BOOL bUseDefault, COLORREF color);
TBARCODEDLL_API VOID TECIT_STDCALL BCGetQuietZoneColorGDI(const t_BarCode* pBarcode, e_QuietZone eZone, COLORREF* pColor);

TBARCODEDLL_API VOID TECIT_STDCALL BCSetQuietZoneColorGray(t_BarCode* pBarcode, e_QuietZone eZone, BOOL bUseDefault, DOUBLE dGray);
TBARCODEDLL_API VOID TECIT_STDCALL BCGetQuietZoneColorGray(const t_BarCode* pBarcode, e_QuietZone eZone, DOUBLE* pdGray);

TBARCODEDLL_API VOID TECIT_STDCALL BCSetQuietZoneColorRGB(t_BarCode* pBarcode, e_QuietZone eZone, BOOL bUseDefault, DOUBLE dRed, DOUBLE dGreen, DOUBLE dBlue);
TBARCODEDLL_API VOID TECIT_STDCALL BCGetQuietZoneColorRGB(const t_BarCode* pBarcode, e_QuietZone eZone, DOUBLE* pdRed, DOUBLE* pdGreen, DOUBLE* pdBlue);


//
// ----- Barcode > Human Readable Text 
//
TBARCODEDLL_API LONG TECIT_STDCALL BCGetMaximumTextIndex();
TBARCODEDLL_API LONG TECIT_STDCALL BCGetActiveTextIndex(t_BarCode* pBarCode);
TBARCODEDLL_API ERRCODE TECIT_STDCALL BCSetActiveTextIndex(t_BarCode* pBarCode, LONG nIndex);

TBARCODEDLL_API LOGFONTA* TECIT_STDCALL BCGetLogFont(t_BarCode*  pBarCode);
TBARCODEDLL_API ERRCODE TECIT_STDCALL BCSetLogFont(t_BarCode*  pBarCode, const LOGFONTA* lf);

//@{
TBARCODEDLL_API CHAR* TECIT_STDCALL BCGetFontNameA(t_BarCode* pBarCode);
TBARCODEDLL_API WCHAR* TECIT_STDCALL BCGetFontNameW(t_BarCode* pBarCode);
//@}
//@{
TBARCODEDLL_API ERRCODE TECIT_STDCALL BCSetFontNameA(t_BarCode* pBarCode, const CHAR* szName);
TBARCODEDLL_API ERRCODE TECIT_STDCALL BCSetFontNameW(t_BarCode* pBarCode, const WCHAR* szName);
//@}

TBARCODEDLL_API INT TECIT_STDCALL BCGetFontHeight(t_BarCode* pBarCode);
TBARCODEDLL_API ERRCODE TECIT_STDCALL  BCSetFontHeight(t_BarCode* pBarCode, INT nHeight);

TBARCODEDLL_API ERRCODE TECIT_STDCALL BCSetPrintText(t_BarCode* pBarCode, BOOL bReadable, BOOL bAbove);

TBARCODEDLL_API BOOL TECIT_STDCALL BCGetTextAbove(t_BarCode* pBarCode);
TBARCODEDLL_API BOOL TECIT_STDCALL BCGetPrintText(t_BarCode* pBarCode);

TBARCODEDLL_API LONG TECIT_STDCALL BCGetTextDist(t_BarCode* pBarCode);
TBARCODEDLL_API ERRCODE TECIT_STDCALL BCSetTextDist(t_BarCode* pBarCode, LONG nTextDist);

//@{
TBARCODEDLL_API LPCSTR TECIT_STDCALL BCGetDisplayTextA(t_BarCode* pBarCode);
TBARCODEDLL_API LPCWSTR TECIT_STDCALL BCGetDisplayTextW(t_BarCode* pBarCode);
//@}
//@{
TBARCODEDLL_API ERRCODE TECIT_STDCALL BCSetDisplayTextA(t_BarCode* pBarCode, LPCSTR szText);
TBARCODEDLL_API ERRCODE TECIT_STDCALL BCSetDisplayTextW(t_BarCode* pBarCode, LPCWSTR szText);
//@}

TBARCODEDLL_API e_BCAlign TECIT_STDCALL BCGetTextAlignment(t_BarCode* pBarCode);
TBARCODEDLL_API ERRCODE TECIT_STDCALL BCSetTextAlignment(t_BarCode* pBarCode, e_BCAlign eAlign);

TBARCODEDLL_API RECT TECIT_STDCALL BCGetTextPosition(t_BarCode* pBarCode);
TBARCODEDLL_API BOOL TECIT_STDCALL BCGetTextClipping(t_BarCode* pBarCode);
TBARCODEDLL_API BOOL TECIT_STDCALL BCIsWordWrappingEnabled(t_BarCode* pBarCode);
TBARCODEDLL_API ERRCODE TECIT_STDCALL BCSetTextPosition(t_BarCode* pBarCode, LPRECT pRect, BOOL bClipping, BOOL bWordWrap);

TBARCODEDLL_API e_Degree TECIT_STDCALL BCGetTextRotation(t_BarCode* pBarCode);
TBARCODEDLL_API ERRCODE TECIT_STDCALL BCSetTextRotation(t_BarCode* pBarCode, e_Degree eRotation);

TBARCODEDLL_API DOUBLE TECIT_STDCALL BCGetCharacterSpacing(t_BarCode* pBarCode);
TBARCODEDLL_API ERRCODE TECIT_STDCALL BCSetCharacterSpacing(t_BarCode* pBarCode, DOUBLE dSpacing);

//
// ----- Barcode > Check-Digits
//
TBARCODEDLL_API e_CDMethod TECIT_STDCALL BCGetCDMethod(t_BarCode* pBarCode);
TBARCODEDLL_API ERRCODE TECIT_STDCALL BCSetCDMethod(t_BarCode* pBarCode, e_CDMethod eMethod);

//@{
TBARCODEDLL_API LONG TECIT_STDCALL BCGetCheckDigitsA(t_BarCode* pBarCode, LPSTR lpszCDText, LONG nSize);
TBARCODEDLL_API LONG TECIT_STDCALL BCGetCheckDigitsW(t_BarCode* pBarCode, LPWSTR lpszCDText, LONG nSize);
//@}
TBARCODEDLL_API ERRCODE TECIT_STDCALL BCCalcCD(t_BarCode* pBarCode);
//@{
TBARCODEDLL_API ERRCODE TECIT_STDCALL BCCalcCDExtA(t_BarCode* pBarCode, LPCSTR lpszData, e_CDMethod eCDMethod, LPSTR lpszChecksum, LONG *nChecksum );
TBARCODEDLL_API ERRCODE TECIT_STDCALL BCCalcCDExtW(t_BarCode* pBarCode, LPCWSTR lpszData, e_CDMethod eCDMethod, LPWSTR lpszChecksum, LONG *nChecksum );
//@}

TBARCODEDLL_API e_CDMethod* TECIT_STDCALL BCGetCDListByType(e_BarCType eBCType);
//@{
TBARCODEDLL_API LPCSTR TECIT_STDCALL BCGetNameFromEnumA(e_CDMethod eCDMethod);
TBARCODEDLL_API LPCWSTR TECIT_STDCALL BCGetNameFromEnumW(e_CDMethod eCDMethod);
//@}
//@{
TBARCODEDLL_API LPCSTR* TECIT_STDCALL BCGetCDListA();
TBARCODEDLL_API LPCWSTR* TECIT_STDCALL BCGetCDListW();
//@}
TBARCODEDLL_API LONG TECIT_STDCALL BCGetCDCount();

//
// ----- Barcode > Generic Options/Properties
//
//@{
TBARCODEDLL_API LPCSTR TECIT_STDCALL BCGetOptionsA(t_BarCode* pBarCode, LPCSTR szFilter);
TBARCODEDLL_API LPCWSTR TECIT_STDCALL BCGetOptionsW(t_BarCode* pBarCode, LPCWSTR szFilter);
//@}
//@{
TBARCODEDLL_API ERRCODE TECIT_STDCALL BCSetOptionsA(t_BarCode* pBarCode, LPCSTR szOption);
TBARCODEDLL_API ERRCODE TECIT_STDCALL BCSetOptionsW(t_BarCode* pBarCode, LPCWSTR szOption);
//@}

//
// ----- Barcode > Size Queries
//
TBARCODEDLL_API DOUBLE TECIT_STDCALL BCGet2D_XRows(const t_BarCode* pBarCode);
TBARCODEDLL_API LONG TECIT_STDCALL BCGet2D_XCols(const t_BarCode* pBarCode);
TBARCODEDLL_API DOUBLE TECIT_STDCALL BCGetCountModules(const t_BarCode* pBarCode);
TBARCODEDLL_API LONG TECIT_STDCALL BCGetCountRows(const t_BarCode* pBarCode);

TBARCODEDLL_API DOUBLE TECIT_STDCALL BCGetModuleWidth(t_BarCode* pBarCode, LPRECT pRect, HDC hDC, e_MUnit eUnit);
TBARCODEDLL_API DOUBLE TECIT_STDCALL BCGetBarcodeWidth(t_BarCode* pBarCode, LPRECT pRect, HDC hDC, e_MUnit eUnit);
TBARCODEDLL_API DOUBLE TECIT_STDCALL BCGetBarcodeHeight(t_BarCode* pBarCode, LPRECT pRect, HDC hDC, e_MUnit eUnit);
TBARCODEDLL_API ERRCODE TECIT_STDCALL BCGetBarcodeSize(t_BarCode* pBarCode, LPRECT pRect, HDC hDC, e_MUnit eUnit, DOUBLE* pdWidth, DOUBLE* pdHeight);
TBARCODEDLL_API DOUBLE TECIT_STDCALL BCGetTextAreaHeight(t_BarCode* pBarCode, LPRECT pRect, HDC hDC, e_MUnit eUnit);

TBARCODEDLL_API ERRCODE TECIT_STDCALL BCGetOptimalBitmapSize(t_BarCode* pBarCode, LPRECT pRect, LONG nScaleX, LONG nScaleY);

//
// ----- Barcode > Module Width and Barcode Quality 
//
TBARCODEDLL_API DOUBLE TECIT_STDCALL BCGetDPIHorz(t_BarCode* pBarCode);
TBARCODEDLL_API DOUBLE TECIT_STDCALL BCGetDPIVert(t_BarCode* pBarCode);
TBARCODEDLL_API ERRCODE TECIT_STDCALL BCSetDPI(t_BarCode* pBarCode, DOUBLE dDPIHorz, DOUBLE dDPIVert);

TBARCODEDLL_API BOOL TECIT_STDCALL BCGetIgnoreDpiFromDC(t_BarCode* pBarCode);
TBARCODEDLL_API ERRCODE TECIT_STDCALL BCSetIgnoreDpiFromDC(t_BarCode* pBarCode, BOOL bIgnore);

//@{
TBARCODEDLL_API LPCSTR TECIT_STDCALL BCGetModWidthA(t_BarCode* pBarCode);
TBARCODEDLL_API LPCWSTR TECIT_STDCALL BCGetModWidthW(t_BarCode* pBarCode);
//@}
//@{
TBARCODEDLL_API ERRCODE TECIT_STDCALL BCSetModWidthA(t_BarCode* pBarCode, LPCSTR szModWidth);
TBARCODEDLL_API ERRCODE TECIT_STDCALL BCSetModWidthW(t_BarCode* pBarCode, LPCWSTR szModWidth);
//@}

TBARCODEDLL_API DOUBLE TECIT_STDCALL BCGetModWidthN(t_BarCode* pBarCode);
TBARCODEDLL_API ERRCODE TECIT_STDCALL BCSetModWidthN(t_BarCode* pBarCode, DOUBLE moduleWidth);

TBARCODEDLL_API BOOL TECIT_STDCALL BCGetMustFit(t_BarCode* pBarCode);
TBARCODEDLL_API ERRCODE TECIT_STDCALL BCSetMustFit(t_BarCode* pBarCode, BOOL bMustFit);

TBARCODEDLL_API BOOL TECIT_STDCALL BCGetOptResolution(t_BarCode* pBarCode);
TBARCODEDLL_API ERRCODE TECIT_STDCALL BCSetOptResolution(t_BarCode* pBarCode, BOOL bOpt);

TBARCODEDLL_API e_SizeMode TECIT_STDCALL BCGetSizeMode(const t_BarCode* pBarCode);
TBARCODEDLL_API ERRCODE TECIT_STDCALL BCSetSizeMode(t_BarCode* pBarCode, e_SizeMode mode);

TBARCODEDLL_API e_Decoder TECIT_STDCALL BCGetDecoder(t_BarCode* pBarCode);
TBARCODEDLL_API ERRCODE TECIT_STDCALL BCSetDecoder(t_BarCode* pBarCode, e_Decoder eDecoder);

TBARCODEDLL_API DOUBLE TECIT_STDCALL BCGetBarWidthReduction(t_BarCode* pBarCode);
TBARCODEDLL_API e_MUnit TECIT_STDCALL BCGetBarWidthReductionUnit(t_BarCode* pBarCode);
TBARCODEDLL_API ERRCODE TECIT_STDCALL BCSetBarWidthReduction(t_BarCode* pBarCode, DOUBLE dValue, e_MUnit eUnit);

TBARCODEDLL_API LONG TECIT_STDCALL BCGetQuality(t_BarCode* pBarCode, HDC hDC, RECT* pRect);



//____________________________________________________________________________
//
// Drawing and Saving Barcodes  
//____________________________________________________________________________
//

//
// ----- Drawing > GDI
//
TBARCODEDLL_API ERRCODE TECIT_STDCALL BCSetDCConversion(t_BarCode* pBarCode, BOOL bConvertDC);
TBARCODEDLL_API BOOL TECIT_STDCALL BCGetDCConversion(t_BarCode* pBarCode);

TBARCODEDLL_API BOOL TECIT_STDCALL BCGetDrawMode(t_BarCode* pBarCode);
TBARCODEDLL_API ERRCODE TECIT_STDCALL BCSetDrawMode(t_BarCode* pBarCode, BOOL bUseGDIRect);

TBARCODEDLL_API e_DrawMode TECIT_STDCALL BCGetDrawModeExt(t_BarCode* pBarCode);
TBARCODEDLL_API ERRCODE TECIT_STDCALL BCSetDrawModeExt(t_BarCode* pBarCode, e_DrawMode eMode);

TBARCODEDLL_API ERRCODE TECIT_STDCALL BCDraw(t_BarCode* pBarCode, HDC hDC, RECT* pRect);




#if defined(TEC_WIN32) || defined(TEC_WINCE)
//
// ----- Drawing > GDI > Clipboard
//
TBARCODEDLL_API ERRCODE TECIT_STDCALL BCCopyToClipboard(t_BarCode* pBarCode, LONG nWidth, LONG nHeight);
//@{
TBARCODEDLL_API ERRCODE TECIT_STDCALL BCCopyToClipboardExA(t_BarCode* pBarCode, HDC hDC, LONG nWidth, LONG nHeight, 
                                                        BOOL fTransparent, LPCSTR szFileName);
TBARCODEDLL_API ERRCODE TECIT_STDCALL BCCopyToClipboardExW(t_BarCode* pBarCode, HDC hDC, LONG nWidth, LONG nHeight, 
                                                        BOOL fTransparent, LPCWSTR szFileName);
//@}

//
// ----- Drawing > GDI > Enhanced Meta file
//
TBARCODEDLL_API ERRCODE TECIT_STDCALL BCGetEnhMetaFile(t_BarCode* pBarCode, HENHMETAFILE* phEMF, LONG nWidth, LONG nHeight);  
TBARCODEDLL_API ERRCODE TECIT_STDCALL BCGetEnhMetaFileEx(t_BarCode* pBarCode, HENHMETAFILE* phEMF, HDC hDC, 
                                                       LONG nWidth, LONG nHeight, BOOL fTransparent);
//
// ----- Drawing > GDI > Image
//
//@{
TBARCODEDLL_API ERRCODE TECIT_STDCALL BCSaveImageA(t_BarCode* pBarCode, LPCSTR lpszFileName, 
                         e_IMType eImageType, LONG lXSize, LONG lYSize, DOUBLE dXRes, DOUBLE dYRes);
TBARCODEDLL_API ERRCODE TECIT_STDCALL BCSaveImageW(t_BarCode* pBarCode, LPCWSTR lpszFileName, 
                         e_IMType eImageType, LONG lXSize, LONG lYSize, DOUBLE dXRes, DOUBLE dYRes);
//@}
//@{
TBARCODEDLL_API ERRCODE TECIT_STDCALL BCSaveImageExA(t_BarCode* pBarCode, HDC hDC, LPCSTR lpszFileName, e_IMType eImageType, 
                                                  LONG lQuality, LONG lXSize, LONG lYSize, DOUBLE dXRes, DOUBLE dYRes);
TBARCODEDLL_API ERRCODE TECIT_STDCALL BCSaveImageExW(t_BarCode* pBarCode, HDC hDC, LPCWSTR lpszFileName, e_IMType eImageType, 
                                                  LONG lQuality, LONG lXSize, LONG lYSize, DOUBLE dXRes, DOUBLE dYRes);
//@}
TBARCODEDLL_API ERRCODE TECIT_STDCALL BCSaveImageToBuffer(t_BarCode* pBarCode, LPBYTE* lpBuffer, e_IMType eImageType, 
                        LONG lXSize, LONG lYSize, DOUBLE dXRes, DOUBLE dYRes);
TBARCODEDLL_API ERRCODE TECIT_STDCALL BCSaveImageToBufferEx(t_BarCode* pBarCode, HDC hDC, LPBYTE* lpBuffer, 
                                                          e_IMType eImageType, LONG lQuality, LONG lXSize, LONG lYSize, 
                                                          DOUBLE dXRes, DOUBLE dYRes);
#endif 


//
// ----- Drawing > Bitmap
//
TBARCODEDLL_API BOOL TECIT_STDCALL BCIsImageMagickAvailable();
//@{
TBARCODEDLL_API ERRCODE TECIT_STDCALL BCBitmapToFileA(t_BarCode* 	pBarCode, LPCSTR szFilename, LPCRECT pRect);
TBARCODEDLL_API ERRCODE TECIT_STDCALL BCBitmapToFileW(t_BarCode* 	pBarCode, LPCWSTR szFilename, LPCRECT pRect);
//@}
//@{
TBARCODEDLL_API ERRCODE TECIT_STDCALL BCBitmapToMemoryA(t_BarCode* 	pBarCode, VOID* pHandle, LPCSTR pszFormat, 
                                                     INT* pnSize, LPCRECT pRect);
TBARCODEDLL_API ERRCODE TECIT_STDCALL BCBitmapToMemoryW(t_BarCode* 	pBarCode, VOID* pHandle, LPCWSTR pszFormat, 
                                                     INT* pnSize, LPCRECT pRect);
//@}


//
// ----- Drawing > PCL
//
TBARCODEDLL_API e_PCLMode TECIT_STDCALL BCGetPCLMode(const t_BarCode* pBarcode);
TBARCODEDLL_API VOID TECIT_STDCALL BCSetPCLMode(t_BarCode* pBarcode, e_PCLMode mode);

//@{
TBARCODEDLL_API ERRCODE TECIT_STDCALL BCPCLToFileA(t_BarCode* pBarCode, LPCSTR szFilename, LPCRECT pRect);
TBARCODEDLL_API ERRCODE TECIT_STDCALL BCPCLToFileW(t_BarCode* pBarCode, LPCWSTR szFilename, LPCRECT pRect);
//@}
//@{
TBARCODEDLL_API ERRCODE TECIT_STDCALL BCPCLToFileExtA(t_BarCode* pBarCode, LPCSTR szFilename, LPCRECT pRect, 
                                                   BOOL bRelPos);
TBARCODEDLL_API ERRCODE TECIT_STDCALL BCPCLToFileExtW(t_BarCode* pBarCode, LPCWSTR szFilename, LPCRECT pRect, 
                                                   BOOL bRelPos);
//@}

TBARCODEDLL_API ERRCODE TECIT_STDCALL BCPCLToMemory(t_BarCode* pBarCode, VOID* pHandle, INT nSize, LPCRECT pRect);  
TBARCODEDLL_API ERRCODE TECIT_STDCALL BCPCLToMemoryExt(t_BarCode* pBarCode, VOID* pHandle, INT* pnSize, 
                                                     LPCRECT pRect, BOOL bRelPos);


//
// ----- Drawing > PDF
//
//@{
TBARCODEDLL_API ERRCODE TECIT_STDCALL BCPDFToFileA(t_BarCode* pBarCode, LPCSTR szFilename, LPCRECT pRect);
TBARCODEDLL_API ERRCODE TECIT_STDCALL BCPDFToFileW(t_BarCode* pBarCode, LPCWSTR szFilename, LPCRECT pRect);
//@}
TBARCODEDLL_API ERRCODE TECIT_STDCALL BCPDFToMemory(t_BarCode* pBarCode, VOID* pHandle, INT* pnSize, 
                                                  LPCRECT pRect);
TBARCODEDLL_API ERRCODE TECIT_STDCALL BCPDFFragmentToMemory(t_BarCode* pBarCode, VOID* pHandle, INT* pnSize, 
                                                    LPCRECT pRect);
//
// ----- Drawing > Postscript
//
//@{
TBARCODEDLL_API ERRCODE TECIT_STDCALL BCPostscriptToFileA(t_BarCode* pBarCode, LPCSTR szFilename, LPCRECT pRect);  
TBARCODEDLL_API ERRCODE TECIT_STDCALL BCPostscriptToFileW(t_BarCode* pBarCode, LPCWSTR szFilename, LPCRECT pRect);  
//@}
TBARCODEDLL_API ERRCODE TECIT_STDCALL BCPostscriptToMemory(t_BarCode* pBarCode, VOID* pHandle, INT nSize, LPCRECT pRect);  
TBARCODEDLL_API ERRCODE TECIT_STDCALL BCPostscriptToMemoryExt(t_BarCode* pBarCode, VOID* pHandle, INT* pnSize, 
                                                            LPCRECT pRect, BOOL bIncludeMacros);

//@{
TBARCODEDLL_API ERRCODE TECIT_STDCALL BCGetPostscriptMacrosA(LPSTR pszMacros, INT* pnSize);
TBARCODEDLL_API ERRCODE TECIT_STDCALL BCGetPostscriptMacrosW(LPWSTR pszMacros, INT* pnSize);
//@}

TBARCODEDLL_API BOOL TECIT_STDCALL BCGetEpsSubstwDeviceFonts(t_BarCode* pBarCode);
TBARCODEDLL_API ERRCODE TECIT_STDCALL BCSetEpsSubstwDeviceFonts(t_BarCode* pBarCode, BOOL bSubwDevFnts);


//
// ----- Drawing > SVG
//
//@{
TBARCODEDLL_API ERRCODE TECIT_STDCALL BCSVGToFileA(t_BarCode* pBarCode, LPCSTR szFilename, LPCRECT pRect);
TBARCODEDLL_API ERRCODE TECIT_STDCALL BCSVGToFileW(t_BarCode* pBarCode, LPCWSTR szFilename, LPCRECT pRect);
//@}
TBARCODEDLL_API ERRCODE TECIT_STDCALL BCSVGToMemory(t_BarCode* pBarCode, VOID* pHandle, INT* pnSize, 
                                                    LPCRECT pRect);


//
// ----- Drawing > Adobe Illustrator Format
//
//@{
TBARCODEDLL_API ERRCODE TECIT_STDCALL BCAIToFileA(t_BarCode* pBarCode, LPCSTR szFilename, LPCRECT pRect);
TBARCODEDLL_API ERRCODE TECIT_STDCALL BCAIToFileW(t_BarCode* pBarCode, LPCWSTR szFilename, LPCRECT pRect);
//@}
TBARCODEDLL_API ERRCODE TECIT_STDCALL BCAIToMemory(t_BarCode* pBarCode, VOID* pHandle, INT* pnSize, 
                                                   LPCRECT pRect);


//
// ----- Drawing > Callback
//
TBARCODEDLL_API fn_DrawRow TECIT_STDCALL BCGetFuncDrawRow(t_BarCode* pBarCode);
TBARCODEDLL_API ERRCODE TECIT_STDCALL BCSetFuncDrawRow(t_BarCode* pBarCode, fn_DrawRow fn);

TBARCODEDLL_API fn_DrawBar TECIT_STDCALL BCGetFuncDrawBar(t_BarCode* pBarCode);
TBARCODEDLL_API ERRCODE TECIT_STDCALL BCSetFuncDrawBar(t_BarCode* pBarCode, fn_DrawBar fn);

//@{
TBARCODEDLL_API fn_DrawTextA TECIT_STDCALL BCGetFuncDrawTextA(t_BarCode* pBarCode);
TBARCODEDLL_API fn_DrawTextW TECIT_STDCALL BCGetFuncDrawTextW(t_BarCode* pBarCode);
//@}
//@{
TBARCODEDLL_API ERRCODE TECIT_STDCALL BCSetFuncDrawTextA(t_BarCode* pBarCode, fn_DrawTextA fn);
TBARCODEDLL_API ERRCODE TECIT_STDCALL BCSetFuncDrawTextW(t_BarCode* pBarCode, fn_DrawTextW fn);
//@}

TBARCODEDLL_API fn_DrawRing TECIT_STDCALL BCGetFuncDrawRing(t_BarCode* pBarCode);
TBARCODEDLL_API ERRCODE TECIT_STDCALL BCSetFuncDrawRing(t_BarCode* pBarCode, fn_DrawRing fn);

TBARCODEDLL_API fn_DrawHexagon TECIT_STDCALL BCGetFuncDrawHexagon(t_BarCode* pBarCode);
TBARCODEDLL_API ERRCODE TECIT_STDCALL BCSetFuncDrawHexagon(t_BarCode* pBarCode, fn_DrawHexagon fn);

TBARCODEDLL_API LPVOID TECIT_STDCALL BCGetCBData(t_BarCode* pBarCode);
TBARCODEDLL_API VOID TECIT_STDCALL BCSetCBData(t_BarCode* pBarCode, LPVOID pData);

TBARCODEDLL_API ERRCODE TECIT_STDCALL BCDrawCB(t_BarCode* pBarCode, HDC hDC, RECT* pRect);

//@{
TBARCODEDLL_API LPCSTR TECIT_STDCALL BCGetMetaDataA(t_BarCode* pBarCode);
TBARCODEDLL_API LPCWSTR TECIT_STDCALL BCGetMetaDataW(t_BarCode* pBarCode);
//@}

TBARCODEDLL_API COLORREF TECIT_STDCALL BCGetColorFromMetaData(t_BarCode* pBarCode, INT chMeta);

//____________________________________________________________________________
//
// Error Handling
//____________________________________________________________________________
//

//@{
TBARCODEDLL_API VOID TECIT_STDCALL BCGetErrorTextA(ERRCODE eCode, LPSTR szText, size_t nSize);
TBARCODEDLL_API VOID TECIT_STDCALL BCGetErrorTextW(ERRCODE eCode, LPWSTR szText, size_t nSize);
//@}
//@{
TBARCODEDLL_API VOID TECIT_STDCALL BCGetErrorTextExtA(t_BarCode* pBarCode, ERRCODE eCode, LPSTR szText, size_t nSize);
TBARCODEDLL_API VOID TECIT_STDCALL BCGetErrorTextExtW(t_BarCode* pBarCode, ERRCODE eCode, LPWSTR szText, size_t nSize);
//@}
TBARCODEDLL_API INT TECIT_STDCALL BCGetInvalidDataPos(t_BarCode* pBarCode);



//____________________________________________________________________________
//
// 2D Barcodes and Special Symbologies
//___________________________________________________________________________
//

//
// ----- Special Barcodes > PDF417 and MicroPDF 
//
TBARCODEDLL_API LONG TECIT_STDCALL BCGet_PDF417_Rows(t_BarCode* pBarCode);
TBARCODEDLL_API ERRCODE TECIT_STDCALL BCSet_PDF417_Rows(t_BarCode* pBarCode, LONG nRows);

TBARCODEDLL_API LONG TECIT_STDCALL BCGet_PDF417_Columns(t_BarCode* pBarCode);
TBARCODEDLL_API ERRCODE TECIT_STDCALL BCSet_PDF417_Columns(t_BarCode* pBarCode, LONG nColumns);

TBARCODEDLL_API LONG TECIT_STDCALL BCGet_PDF417_ECLevel(t_BarCode* pBarCode);
TBARCODEDLL_API ERRCODE TECIT_STDCALL BCSet_PDF417_ECLevel(t_BarCode* pBarCode, LONG nLevel);

TBARCODEDLL_API LONG TECIT_STDCALL BCGet_PDF417_RowHeight(t_BarCode* pBarCode);
TBARCODEDLL_API ERRCODE TECIT_STDCALL BCSet_PDF417_RowHeight(t_BarCode* pBarCode, LONG nHeight);

//@{
TBARCODEDLL_API LPCSTR TECIT_STDCALL BCGet_PDF417_RowColRatioA(t_BarCode* pBarCode);
TBARCODEDLL_API LPCWSTR TECIT_STDCALL BCGet_PDF417_RowColRatioW(t_BarCode* pBarCode);
//@}
//@{
TBARCODEDLL_API ERRCODE TECIT_STDCALL BCSet_PDF417_RowColRatioA(t_BarCode* pBarCode, LPCSTR szRatio);
TBARCODEDLL_API ERRCODE TECIT_STDCALL BCSet_PDF417_RowColRatioW(t_BarCode* pBarCode, LPCWSTR szRatio);
//@}

TBARCODEDLL_API LONG TECIT_STDCALL BCGet_PDF417_CodeWords(t_BarCode* pBarCode);

TBARCODEDLL_API e_PDFMode TECIT_STDCALL BCGet_PDF417_EncodingMode(t_BarCode* pBarCode);
TBARCODEDLL_API ERRCODE TECIT_STDCALL BCSet_PDF417_EncodingMode(t_BarCode* pBarCode, e_PDFMode eMode);

//
// ----- Special Barcodes > PDF417 and Micro PDF > Macro PDF
//
TBARCODEDLL_API LONG TECIT_STDCALL BCGet_PDF417_SegIndex(t_BarCode* pBarCode);
TBARCODEDLL_API ERRCODE TECIT_STDCALL BCSet_PDF417_SegIndex(t_BarCode* pBarCode, LONG nSegInx);

//@{
TBARCODEDLL_API LPCSTR TECIT_STDCALL BCGet_PDF417_FileIDA(t_BarCode* pBarCode);
TBARCODEDLL_API LPCWSTR TECIT_STDCALL BCGet_PDF417_FileIDW(t_BarCode* pBarCode);
//@}
//@{
TBARCODEDLL_API ERRCODE TECIT_STDCALL BCSet_PDF417_FileIDA(t_BarCode* pBarCode, LPCSTR szFileID);
TBARCODEDLL_API ERRCODE TECIT_STDCALL BCSet_PDF417_FileIDW(t_BarCode* pBarCode, LPCWSTR szFileID);
//@}

TBARCODEDLL_API BOOL TECIT_STDCALL BCGet_PDF417_SegLast(t_BarCode* pBarCode);
TBARCODEDLL_API ERRCODE TECIT_STDCALL BCSet_PDF417_SegLast(t_BarCode* pBarCode, BOOL bSegLast);

//@{
TBARCODEDLL_API LPCSTR TECIT_STDCALL BCGet_PDF417_FileNameA(t_BarCode* pBarCode);
TBARCODEDLL_API LPCWSTR TECIT_STDCALL BCGet_PDF417_FileNameW(t_BarCode* pBarCode);
//@}
//@{
TBARCODEDLL_API ERRCODE TECIT_STDCALL BCSet_PDF417_FileNameA(t_BarCode* pBarCode, LPCSTR szFileName);
TBARCODEDLL_API ERRCODE TECIT_STDCALL BCSet_PDF417_FileNameW(t_BarCode* pBarCode, LPCWSTR szFileName);
//@}

TBARCODEDLL_API LONG TECIT_STDCALL BCGet_PDF417_SegCount(t_BarCode* pBarCode);
TBARCODEDLL_API ERRCODE TECIT_STDCALL BCSet_PDF417_SegCount(t_BarCode* pBarCode, LONG nSegCount);

TBARCODEDLL_API LONG TECIT_STDCALL BCGet_PDF417_TimeStamp(t_BarCode* pBarCode);
TBARCODEDLL_API ERRCODE TECIT_STDCALL BCSet_PDF417_TimeStamp(t_BarCode* pBarCode, LONG nTimeStamp);

//@{
TBARCODEDLL_API LPCSTR TECIT_STDCALL BCGet_PDF417_SenderA(t_BarCode* pBarCode);
TBARCODEDLL_API LPCWSTR TECIT_STDCALL BCGet_PDF417_SenderW(t_BarCode* pBarCode);
//@}
//@{
TBARCODEDLL_API ERRCODE TECIT_STDCALL BCSet_PDF417_SenderA(t_BarCode* pBarCode, LPCSTR szSender);
TBARCODEDLL_API ERRCODE TECIT_STDCALL BCSet_PDF417_SenderW(t_BarCode* pBarCode, LPCWSTR szSender);
//@}

//@{
TBARCODEDLL_API LPCSTR TECIT_STDCALL BCGet_PDF417_AddresseeA(t_BarCode* pBarCode);
TBARCODEDLL_API LPCWSTR TECIT_STDCALL BCGet_PDF417_AddresseeW(t_BarCode* pBarCode);
//@}
//@{
TBARCODEDLL_API ERRCODE TECIT_STDCALL BCSet_PDF417_AddresseeA(t_BarCode* pBarCode, LPCSTR szAddressee);
TBARCODEDLL_API ERRCODE TECIT_STDCALL BCSet_PDF417_AddresseeW(t_BarCode* pBarCode, LPCWSTR szAddressee);
//@}

TBARCODEDLL_API LONG TECIT_STDCALL BCGet_PDF417_FileSize(t_BarCode* pBarCode);
TBARCODEDLL_API ERRCODE TECIT_STDCALL BCSet_PDF417_FileSize(t_BarCode* pBarCode, LONG nFileSize);

TBARCODEDLL_API LONG TECIT_STDCALL BCGet_PDF417_CheckSum(t_BarCode* pBarCode);
TBARCODEDLL_API ERRCODE TECIT_STDCALL BCSet_PDF417_CheckSum(t_BarCode* pBarCode, LONG nCheckSum);


//
// ----- Special Barcodes > PDF417 and Micro PDF > Micro PDF
//
TBARCODEDLL_API e_MPDFMode TECIT_STDCALL BCGet_MPDF417_Mode(t_BarCode* pBarCode);
TBARCODEDLL_API ERRCODE TECIT_STDCALL BCSet_MPDF417_Mode(t_BarCode* pBarCode, e_MPDFMode eMode);

TBARCODEDLL_API e_MPDFVers TECIT_STDCALL BCGet_MPDF417_Version(t_BarCode* pBarCode);
TBARCODEDLL_API ERRCODE TECIT_STDCALL BCSet_MPDF417_Version(t_BarCode* pBarCode, e_MPDFVers eVersion);

//@{
TBARCODEDLL_API LPCSTR* TECIT_STDCALL BCGet_MPDF417_VersionsA();
TBARCODEDLL_API LPCWSTR* TECIT_STDCALL BCGet_MPDF417_VersionsW();
//@}
TBARCODEDLL_API LONG TECIT_STDCALL BCGet_MPDF417_VersionCount();



//
// ----- Special Barcodes > MaxiCode
//
TBARCODEDLL_API LONG TECIT_STDCALL BCGet_Maxi_Mode(t_BarCode* pBarCode);
TBARCODEDLL_API ERRCODE TECIT_STDCALL BCSet_Maxi_Mode(t_BarCode* pBarCode, LONG nMode);

TBARCODEDLL_API LONG TECIT_STDCALL BCGet_Maxi_AppendSum(t_BarCode* pBarCode);
TBARCODEDLL_API ERRCODE TECIT_STDCALL BCSet_Maxi_Append(t_BarCode* pBarCode, LONG nSum, LONG nIndex);

TBARCODEDLL_API LONG TECIT_STDCALL BCGet_Maxi_AppendIndex(t_BarCode* pBarCode);

TBARCODEDLL_API LONG TECIT_STDCALL BCGet_Maxi_UnderCut(t_BarCode* pBarCode);
TBARCODEDLL_API ERRCODE TECIT_STDCALL BCSet_Maxi_UnderCut(t_BarCode* pBarCode, LONG nUndercut);

TBARCODEDLL_API BOOL TECIT_STDCALL BCGet_Maxi_UsePreamble(t_BarCode* pBarCode);
//@{
TBARCODEDLL_API ERRCODE TECIT_STDCALL BCSet_Maxi_UsePreambleA(t_BarCode* pBarCode, BOOL bUse, LPCSTR szDate);
TBARCODEDLL_API ERRCODE TECIT_STDCALL BCSet_Maxi_UsePreambleW(t_BarCode* pBarCode, BOOL bUse, LPCWSTR szDate);
//@}

//@{
TBARCODEDLL_API LPCSTR TECIT_STDCALL BCGet_Maxi_PreambleDateA(t_BarCode* pBarCode);
TBARCODEDLL_API LPCWSTR TECIT_STDCALL BCGet_Maxi_PreambleDateW(t_BarCode* pBarCode);
//@}
//@{
TBARCODEDLL_API LPCSTR TECIT_STDCALL BCGet_Maxi_SCMServClassA(t_BarCode* pBarCode);
TBARCODEDLL_API LPCWSTR TECIT_STDCALL BCGet_Maxi_SCMServClassW(t_BarCode* pBarCode);
//@}
//@{
TBARCODEDLL_API LPCSTR TECIT_STDCALL BCGet_Maxi_SCMCountryCodeA(t_BarCode* pBarCode);
TBARCODEDLL_API LPCWSTR TECIT_STDCALL BCGet_Maxi_SCMCountryCodeW(t_BarCode* pBarCode);
//@}
//@{
TBARCODEDLL_API LPCSTR TECIT_STDCALL BCGet_Maxi_SCMPostalCodeA(t_BarCode* pBarCode);
TBARCODEDLL_API LPCWSTR TECIT_STDCALL BCGet_Maxi_SCMPostalCodeW(t_BarCode* pBarCode);
//@}
//@{
TBARCODEDLL_API ERRCODE TECIT_STDCALL BCSet_Maxi_SCMA(t_BarCode* pBarCode, LPCSTR szServiceClass,
                                                   LPCSTR szCountryCode, LPCSTR szPostalCode);
TBARCODEDLL_API ERRCODE TECIT_STDCALL BCSet_Maxi_SCMW(t_BarCode* pBarCode, LPCWSTR szServiceClass,
                                                   LPCWSTR szCountryCode, LPCWSTR szPostalCode);
//@}

//
// ----- Special Barcodes > Data Matrix 
//
#ifndef TEC_DISABLE_DATAMATRIX
TBARCODEDLL_API e_DMSizes TECIT_STDCALL BCGet_DM_Size(t_BarCode* pBarCode);
TBARCODEDLL_API ERRCODE TECIT_STDCALL BCSet_DM_Size(t_BarCode* pBarCode, e_DMSizes eSize);

TBARCODEDLL_API BOOL TECIT_STDCALL BCGet_DM_Rectangular(t_BarCode* pBarCode);
TBARCODEDLL_API ERRCODE TECIT_STDCALL BCSet_DM_Rectangular(t_BarCode* pBarCode, BOOL bRect);

TBARCODEDLL_API e_DMFormat TECIT_STDCALL BCGet_DM_Format(t_BarCode* pBarCode);
TBARCODEDLL_API ERRCODE TECIT_STDCALL BCSet_DM_Format(t_BarCode* pBarCode, e_DMFormat eFormat);

TBARCODEDLL_API BOOL TECIT_STDCALL BCGet_DM_EnforceBinaryEncoding(t_BarCode* pBarCode);
TBARCODEDLL_API ERRCODE TECIT_STDCALL BCSet_DM_EnforceBinaryEncoding(t_BarCode* pBarCode, BOOL bEnforceBinary);

TBARCODEDLL_API LONG TECIT_STDCALL BCGet_DM_AppendSum(t_BarCode* pBarCode);
TBARCODEDLL_API LONG TECIT_STDCALL BCGet_DM_AppendIndex(t_BarCode* pBarCode);
TBARCODEDLL_API LONG TECIT_STDCALL BCGet_DM_AppendFileID(t_BarCode* pBarCode);
TBARCODEDLL_API ERRCODE TECIT_STDCALL BCSet_DM_Append(t_BarCode* pBarCode, LONG nSum, LONG nIndex, LONG nFileID);
TBARCODEDLL_API BOOL TECIT_STDCALL BC_DM_IsAppendAllowed(e_DMFormat eFormat);

//@{
TBARCODEDLL_API LPCSTR* TECIT_STDCALL BCGetDataMatrixSizesA();
TBARCODEDLL_API LPCWSTR* TECIT_STDCALL BCGetDataMatrixSizesW();
//@}
TBARCODEDLL_API LONG TECIT_STDCALL BCGetDataMatrixSizesCount();

TBARCODEDLL_API e_DMSizes* TECIT_STDCALL BCGetDataMatrixSizeListForType(e_BarCType eBCType);
TBARCODEDLL_API LONG TECIT_STDCALL BCGetDataMatrixSizesCountForType(e_BarCType eBCType);
#endif // TEC_DISABLE_DATAMATRIX


//
// ----- Special Barcodes > Aztec Code
//

TBARCODEDLL_API e_AztecSizes TECIT_STDCALL BCGet_Aztec_Size(t_BarCode* pBarCode);
TBARCODEDLL_API ERRCODE TECIT_STDCALL BCSet_Aztec_Size(t_BarCode* pBarCode, e_AztecSizes eSize);

TBARCODEDLL_API BOOL TECIT_STDCALL BCGet_Aztec_EnforceBinaryEncoding(t_BarCode* pBarCode);
TBARCODEDLL_API ERRCODE TECIT_STDCALL BCSet_Aztec_EnforceBinaryEncoding(t_BarCode* pBarCode, BOOL bEnforceBinary);

TBARCODEDLL_API e_AztecFormat TECIT_STDCALL BCGet_Aztec_Format(t_BarCode* pBarCode);
//@{
TBARCODEDLL_API LPCSTR TECIT_STDCALL BCGet_Aztec_FormatSpecifierA(t_BarCode* pBarCode);
TBARCODEDLL_API LPCWSTR TECIT_STDCALL BCGet_Aztec_FormatSpecifierW(t_BarCode* pBarCode);
//@}
//@{
TBARCODEDLL_API ERRCODE TECIT_STDCALL BCSet_Aztec_FormatA(t_BarCode* pBarCode, e_AztecFormat eFormat, LPCSTR szFormatSpecifier);
TBARCODEDLL_API ERRCODE TECIT_STDCALL BCSet_Aztec_FormatW(t_BarCode* pBarCode, e_AztecFormat eFormat, LPCWSTR szFormatSpecifier);
//@}

TBARCODEDLL_API LONG TECIT_STDCALL BCGet_Aztec_ErrorCorrection(t_BarCode* pBarCode);
TBARCODEDLL_API ERRCODE TECIT_STDCALL BCSet_Aztec_ErrorCorrection(t_BarCode* pBarCode, LONG nErrorCorrection);

TBARCODEDLL_API BOOL TECIT_STDCALL BCGet_Aztec_RuneMode(t_BarCode* pBarCode);
TBARCODEDLL_API ERRCODE TECIT_STDCALL BCSet_Aztec_RuneMode(t_BarCode* pBarCode, BOOL bRuneMode);

TBARCODEDLL_API BOOL TECIT_STDCALL BCGet_Aztec_AppendActive(t_BarCode* pBarCode);
TBARCODEDLL_API ERRCODE TECIT_STDCALL BCSet_Aztec_AppendActive(t_BarCode* pBarCode, BOOL bActivate);

//@{
TBARCODEDLL_API CHAR TECIT_STDCALL BCGet_Aztec_AppendTotalA(t_BarCode* pBarCode);
TBARCODEDLL_API WCHAR TECIT_STDCALL BCGet_Aztec_AppendTotalW(t_BarCode* pBarCode);
//@}
//@{
TBARCODEDLL_API CHAR TECIT_STDCALL BCGet_Aztec_AppendIndexA(t_BarCode* pBarCode);
TBARCODEDLL_API WCHAR TECIT_STDCALL BCGet_Aztec_AppendIndexW(t_BarCode* pBarCode);
//@}
//@{
TBARCODEDLL_API LPCSTR TECIT_STDCALL BCGet_Aztec_AppendMessageIDA(t_BarCode* pBarCode);
TBARCODEDLL_API LPCWSTR TECIT_STDCALL BCGet_Aztec_AppendMessageIDW(t_BarCode* pBarCode);
//@}
//@{
TBARCODEDLL_API ERRCODE TECIT_STDCALL BCSet_Aztec_AppendDataA(t_BarCode* pBarCode, CHAR cTotal, CHAR cIndex, LPCSTR szMessageID);
TBARCODEDLL_API ERRCODE TECIT_STDCALL BCSet_Aztec_AppendDataW(t_BarCode* pBarCode, WCHAR cTotal, WCHAR cIndex, LPCWSTR szMessageID);
//@}

//@{
TBARCODEDLL_API LPCSTR* TECIT_STDCALL BCGetAztecSizesA();
TBARCODEDLL_API LPCWSTR* TECIT_STDCALL BCGetAztecSizesW();
//@}
TBARCODEDLL_API LONG TECIT_STDCALL BCGetAztecSizesCount();

//
// ----- Special Barcodes > DotCode
//

TBARCODEDLL_API e_DCSizeMode TECIT_STDCALL BCGet_DotCode_SizeMode(t_BarCode* pBarCode);
//@{
TBARCODEDLL_API LPCSTR TECIT_STDCALL BCGet_DotCode_SizeA(t_BarCode* pBarCode);
TBARCODEDLL_API LPCWSTR TECIT_STDCALL BCGet_DotCode_SizeW(t_BarCode* pBarCode);
//@}
//@{
TBARCODEDLL_API LPCSTR TECIT_STDCALL BCGet_DotCode_DefaultSizeForModeA(e_DCSizeMode eMode);
TBARCODEDLL_API LPCWSTR TECIT_STDCALL BCGet_DotCode_DefaultSizeForModeW(e_DCSizeMode eMode);
//@}
//@{
TBARCODEDLL_API ERRCODE TECIT_STDCALL BCSet_DotCode_SizeA(t_BarCode* pBarCode, e_DCSizeMode eMode, LPCSTR szSize);
TBARCODEDLL_API ERRCODE TECIT_STDCALL BCSet_DotCode_SizeW(t_BarCode* pBarCode, e_DCSizeMode eMode, LPCWSTR szSize);
//@}

TBARCODEDLL_API e_DCPrintDirection TECIT_STDCALL BCGet_DotCode_PrintDirection(t_BarCode* pBarCode);
TBARCODEDLL_API ERRCODE TECIT_STDCALL BCSet_DotCode_PrintDirection(t_BarCode* pBarCode, e_DCPrintDirection eDirection);

TBARCODEDLL_API BOOL TECIT_STDCALL BCGet_DotCode_EnforceBinaryEncoding(t_BarCode* pBarCode);
TBARCODEDLL_API ERRCODE TECIT_STDCALL BCSet_DotCode_EnforceBinaryEncoding(t_BarCode* pBarCode, BOOL bEnforceBinary);

TBARCODEDLL_API e_DCFormat TECIT_STDCALL BCGet_DotCode_Format(t_BarCode* pBarCode);
//@{
TBARCODEDLL_API LPCSTR TECIT_STDCALL BCGet_DotCode_FormatSpecifierA(t_BarCode* pBarCode);
TBARCODEDLL_API LPCWSTR TECIT_STDCALL BCGet_DotCode_FormatSpecifierW(t_BarCode* pBarCode);
//@}
//@{
TBARCODEDLL_API ERRCODE TECIT_STDCALL BCSet_DotCode_FormatA(t_BarCode* pBarCode, e_DCFormat eFormat, LPCSTR szFormatSpecifier);
TBARCODEDLL_API ERRCODE TECIT_STDCALL BCSet_DotCode_FormatW(t_BarCode* pBarCode, e_DCFormat eFormat, LPCWSTR szFormatSpecifier);
//@}

TBARCODEDLL_API e_DCMask TECIT_STDCALL BCGet_DotCode_Mask(t_BarCode* pBarCode);
TBARCODEDLL_API ERRCODE TECIT_STDCALL BCSet_DotCode_Mask(t_BarCode* pBarCode, e_DCMask eMask);

TBARCODEDLL_API BOOL TECIT_STDCALL BCGet_DotCode_AppendActive(t_BarCode* pBarCode);
TBARCODEDLL_API ERRCODE TECIT_STDCALL BCSet_DotCode_AppendActive(t_BarCode* pBarCode, BOOL bActivate);
TBARCODEDLL_API LONG TECIT_STDCALL BCGet_DotCode_AppendTotal(t_BarCode* pBarCode);
TBARCODEDLL_API LONG TECIT_STDCALL BCGet_DotCode_AppendIndex(t_BarCode* pBarCode);
TBARCODEDLL_API ERRCODE TECIT_STDCALL BCSet_DotCode_Append(t_BarCode* pBarCode, LONG nTotal, LONG nIndex);

//
// ----- Special Barcodes > QR-Code + Micro QR Code
//
TBARCODEDLL_API e_QRVersion TECIT_STDCALL BCGet_QR_Version(t_BarCode* pBarCode);
TBARCODEDLL_API ERRCODE TECIT_STDCALL BCSet_QR_Version(t_BarCode* pBarCode, e_QRVersion eVersion);

TBARCODEDLL_API e_QRFormat TECIT_STDCALL BCGet_QR_Format(t_BarCode* pBarCode);
TBARCODEDLL_API ERRCODE TECIT_STDCALL BCSet_QR_Format(t_BarCode* pBarCode, e_QRFormat eFormat);

//@{
TBARCODEDLL_API LPCSTR TECIT_STDCALL BCGet_QR_FmtAppIndicatorA(t_BarCode* pBarCode);
TBARCODEDLL_API LPCWSTR TECIT_STDCALL BCGet_QR_FmtAppIndicatorW(t_BarCode* pBarCode);
//@}
//@{
TBARCODEDLL_API ERRCODE TECIT_STDCALL BCSet_QR_FmtAppIndicatorA(t_BarCode* pBarCode, LPCSTR szIndicator);
TBARCODEDLL_API ERRCODE TECIT_STDCALL BCSet_QR_FmtAppIndicatorW(t_BarCode* pBarCode, LPCWSTR szIndicator);
//@}

TBARCODEDLL_API e_QRECLevel TECIT_STDCALL BCGet_QR_ECLevel(t_BarCode* pBarCode);
TBARCODEDLL_API ERRCODE TECIT_STDCALL BCSet_QR_ECLevel(t_BarCode* pBarCode, e_QRECLevel eECLevel);

TBARCODEDLL_API e_QRMask TECIT_STDCALL BCGet_QR_Mask(t_BarCode* pBarCode);
TBARCODEDLL_API ERRCODE TECIT_STDCALL BCSet_QR_Mask(t_BarCode* pBarCode, e_QRMask eMask);

TBARCODEDLL_API e_QRMBCompaction TECIT_STDCALL BCGet_QR_KanjiChineseCompaction (t_BarCode* pBarCode);
TBARCODEDLL_API ERRCODE TECIT_STDCALL BCSet_QR_KanjiChineseCompaction (t_BarCode* pBarCode, e_QRMBCompaction eCompact);

TBARCODEDLL_API LONG TECIT_STDCALL BCGet_QR_AppendSum(t_BarCode* pBarCode);
TBARCODEDLL_API LONG TECIT_STDCALL BCGet_QR_AppendIndex(t_BarCode* pBarCode);
TBARCODEDLL_API BYTE TECIT_STDCALL BCGet_QR_AppendParity(t_BarCode* pBarCode);
TBARCODEDLL_API ERRCODE TECIT_STDCALL BCSet_QR_Append(t_BarCode* pBarCode, LONG nSum, LONG nIndex, BYTE bParity);

//@{
TBARCODEDLL_API BYTE TECIT_STDCALL BCCalcStructApp_ParityA(LPCSTR szIntData, LONG nIntData);
TBARCODEDLL_API BYTE TECIT_STDCALL BCCalcStructApp_ParityW(LPCWSTR szIntData, LONG nIntData);
//@}

//@{
TBARCODEDLL_API LPCSTR* TECIT_STDCALL BCGetQRCodeVersionsA();
TBARCODEDLL_API LPCWSTR* TECIT_STDCALL BCGetQRCodeVersionsW();
//@}
TBARCODEDLL_API LONG TECIT_STDCALL BCGetQRCodeVersionCount();

//
// ----- Special Barcodes > QR-Code + Micro QR Code > Micro QR Code
//
TBARCODEDLL_API e_MQRVersion TECIT_STDCALL BCGet_MQR_Version(t_BarCode* pBarCode);
TBARCODEDLL_API ERRCODE TECIT_STDCALL BCSet_MQR_Version(t_BarCode* pBarCode, e_MQRVersion eVersion);

TBARCODEDLL_API e_MQRMask TECIT_STDCALL BCGet_MQR_Mask(t_BarCode* pBarCode);
TBARCODEDLL_API ERRCODE TECIT_STDCALL BCSet_MQR_Mask(t_BarCode* pBarCode, e_MQRMask eMask);

//@{
TBARCODEDLL_API LPCSTR* TECIT_STDCALL BCGetMQRCodeVersionsA();
TBARCODEDLL_API LPCWSTR* TECIT_STDCALL BCGetMQRCodeVersionsW();
//@}
TBARCODEDLL_API LONG TECIT_STDCALL BCGetMQRCodeVersionCount();

//
// ----- Special Barcodes > Han Xin Code
//

TBARCODEDLL_API e_HanXinSizes TECIT_STDCALL BCGet_HanXin_Version(t_BarCode* pBarCode);
TBARCODEDLL_API ERRCODE TECIT_STDCALL BCSet_HanXin_Version(t_BarCode* pBarCode, e_HanXinSizes eVersion);

TBARCODEDLL_API e_HanXinECLevel TECIT_STDCALL BCGet_HanXin_ECLevel(t_BarCode* pBarCode);
TBARCODEDLL_API ERRCODE TECIT_STDCALL BCSet_HanXin_ECLevel(t_BarCode* pBarCode, e_HanXinECLevel eECLevel);

TBARCODEDLL_API e_HanXinMask TECIT_STDCALL BCGet_HanXin_Mask(t_BarCode* pBarCode);
TBARCODEDLL_API ERRCODE TECIT_STDCALL BCSet_HanXin_Mask(t_BarCode* pBarCode, e_HanXinMask eMask);

TBARCODEDLL_API BOOL TECIT_STDCALL BCGet_HanXin_EnforceBinaryEncoding(t_BarCode* pBarCode);
TBARCODEDLL_API ERRCODE TECIT_STDCALL BCSet_HanXin_EnforceBinaryEncoding(t_BarCode* pBarCode, BOOL bEnforceBinary);

//@{
TBARCODEDLL_API LPCSTR* TECIT_STDCALL BCGetHanXinVersionsA();
TBARCODEDLL_API LPCWSTR* TECIT_STDCALL BCGetHanXinVersionsW();
//@}
TBARCODEDLL_API LONG TECIT_STDCALL BCGetHanXinVersionCount();

//
// ----- Special Barcodes > Codablock-F 
//
TBARCODEDLL_API e_CBFFormat TECIT_STDCALL BCGet_CBF_Format(t_BarCode* pBarCode);
TBARCODEDLL_API ERRCODE TECIT_STDCALL BCSet_CBF_Format(t_BarCode* pBarCode, e_CBFFormat eFormat);

TBARCODEDLL_API LONG TECIT_STDCALL BCGet_CBF_Columns(t_BarCode* pBarCode);
TBARCODEDLL_API ERRCODE TECIT_STDCALL BCSet_CBF_Columns(t_BarCode* pBarCode, LONG nColumns);

TBARCODEDLL_API LONG TECIT_STDCALL BCGet_CBF_Rows(t_BarCode* pBarCode);
TBARCODEDLL_API ERRCODE TECIT_STDCALL BCSet_CBF_Rows(t_BarCode* pBarCode, LONG nRows);

TBARCODEDLL_API LONG TECIT_STDCALL BCGet_CBF_RowHeight(t_BarCode* pBarCode);
TBARCODEDLL_API ERRCODE TECIT_STDCALL BCSet_CBF_RowHeight(t_BarCode* pBarCode, LONG nHeight);

TBARCODEDLL_API LONG TECIT_STDCALL BCGet_CBF_RowSeparatorHeight(t_BarCode* pBarCode);
TBARCODEDLL_API ERRCODE TECIT_STDCALL BCSet_CBF_RowSeparatorHeight(t_BarCode* pBarCode, LONG nHeight);

//
// ----- Special Barcodes > EAN / UCC Composite Symbology
//
TBARCODEDLL_API e_CCType TECIT_STDCALL BCGet2DCompositeComponent(t_BarCode* pBarCode);
TBARCODEDLL_API ERRCODE TECIT_STDCALL BCSet2DCompositeComponent(t_BarCode* pBarCode, e_CCType eCCType);

TBARCODEDLL_API BOOL TECIT_STDCALL BCGet_CompSym_IsCCPresent(t_BarCode* pBarCode);
TBARCODEDLL_API BOOL TECIT_STDCALL BCGet_CompSym_IsCCAvailable(t_BarCode* pBarCode, e_CCType eCCType);
TBARCODEDLL_API BOOL TECIT_STDCALL BCGet_CompSym_ErrorInCC(t_BarCode* pBarCode);

TBARCODEDLL_API BOOL TECIT_STDCALL BCIsCompositeAllowed(e_BarCType barcodeType);
TBARCODEDLL_API BOOL TECIT_STDCALL BCIsCompositeComponentAllowed(e_BarCType barcodeType, e_CCType componentType);


//
// ----- Special Barcodes > RSS (Reduced Space Symbology)
//
TBARCODEDLL_API LONG TECIT_STDCALL BCGet_RSS_SegmPerRow(t_BarCode* pBarCode);
TBARCODEDLL_API ERRCODE TECIT_STDCALL BCSet_RSS_SegmPerRow(t_BarCode* pBarCode, LONG nSegmPerRow);

#ifdef TEC_USE_BC_EXTENDED_HEADER
#include "tbarcode_ext.h"
#endif // TEC_USE_BC_EXTENDED_HEADER

#ifdef __cplusplus
  }
#endif

#endif // TBARCODE_HEADER
