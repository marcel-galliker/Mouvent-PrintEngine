// ----------------------------------------------------------------------------
// file name: TECDataTypes.h
//    author: Dominik Seichter (TEC-IT)
// copyright: (c) TEC-IT Datenverarbeitung GmbH., 2004-2006
// ----------------------------------------------------------------------------
//   content: 
// ----------------------------------------------------------------------------
// history: 
// (DS 2004-07-19) : creation
// ----------------------------------------------------------------------------

#ifndef _TECDataTypes_h_
#define _TECDataTypes_h_

// ----------------------------------------------------------------------
// define types if no windows include is here                                                                
// ----------------------------------------------------------------------
#ifndef TEC_WIN
  #ifndef _stdcall
    #define _stdcall
  #endif

  #ifndef NEAR
    #define NEAR
  #endif

  #ifndef FAR
    #define FAR
  #endif

  #ifndef VOID
    #define VOID			void
  #endif

  #ifndef UBYTE
    #define UBYTE			unsigned char
  #endif

  #ifndef SBYTE
    #define SBYTE			signed char
  #endif

  #ifndef BYTE
    #define BYTE			UBYTE
  #endif

  #ifndef SHORT
    #define SHORT			short
  #endif

  #ifndef USHORT
    #define USHORT		unsigned SHORT
  #endif

  #ifndef INT
    #define INT			int
  #endif
  
  #ifndef _TINT
    #define _TINT int
  #endif

  #ifndef UINT
    #define UINT			unsigned INT
  #endif

  #ifndef LONG
    #define LONG			long
  #endif

  #ifndef MAXLONG
    #define MAXLONG 0x7fffffff 
  #endif

  #ifndef LONGLONG
    #define LONGLONG	long long
  #endif

  #ifndef MAXLONGLONG
    #define MAXLONGLONG (0x7fffffffffffffffll)
  #endif

  #ifndef ULONG
    #define ULONG			unsigned LONG
  #endif

  #ifndef ULONGLONG
    #define ULONGLONG unsigned long long
  #endif

  #ifndef WORD
    #define WORD			USHORT
  #endif

  #ifndef DWORD
    // DWORD must be 32 bit long.
    // UINT ... 32 bit (on 32-bit and 64-bit-platforms)
    // On 16-bit-platforms DWORD needs to be defined as ULONG.
    #define DWORD			UINT
  #endif

  #ifndef DWORDLONG
    #define DWORDLONG unsigned long long
  #endif

  #ifndef FLOAT
    #define FLOAT     float
  #endif

  #ifndef DOUBLE
    #define DOUBLE		double
  #endif

  #ifndef CHAR
    #define CHAR			char
  #endif

  #ifndef UCHAR
    #define UCHAR			unsigned char
  #endif

  #ifndef WCHAR
    #define WCHAR			wchar_t
  /*
    #ifndef _TEC_UNIX
      #define WCHAR			unsigned short
    #else
      #define WCHAR			wchar_t
    #endif    
  */  
  #endif

  #ifndef TCHAR
    #ifdef _UNICODE
      #define TCHAR			WCHAR
    #else
      #define TCHAR			CHAR
    #endif
  #endif

  #ifndef BOOL
    #define BOOL			INT
  #endif

  #ifndef BOOLEAN
    #define BOOLEAN   BYTE
  #endif

  #ifndef LPTSTR
    #define LPTSTR		TCHAR*
  #endif  

  #ifndef LPCTSTR
    #define LPCTSTR		const TCHAR*
  #endif

  // Ansi Strings
  #ifndef LPSTR    
    #define LPSTR CHAR*
  #endif

  #ifndef LPCSTR    
    #define LPCSTR const LPSTR
  #endif

  // Unicode Strings
  #ifndef LPWSTR    
    #define LPWSTR WCHAR*
  #endif

  #ifndef LPCWSTR    
    #define LPCWSTR const LPWSTR
  #endif

  // Datatype Ptrs
  #ifndef LPBOOL
    #define LPBOOL		BOOL*
  #endif

  #ifndef LPINT
    #define LPINT     INT*
  #endif

  #ifndef LPWORD
    #define LPWORD		WORD*
  #endif

  #ifndef LPDWORD
    #define LPDWORD		DWORD*
  #endif

  #ifndef LPBYTE
    #define LPBYTE		BYTE*
  #endif

  #ifndef LPLONG
    #define LPLONG		LONG*
  #endif

  #ifndef LPVOID
    #define LPVOID		VOID*
  #endif

  #ifndef LPCVOID
    #define LPCVOID		const VOID*
  #endif
  
  #ifndef LPARAM
    #define LPARAM		LONG*
    #define _LPARAM_DEFINED
  #endif
  
  #ifndef LONG_PTR
    #define LONG_PTR long
    #define PLONG_PTR long*
  #endif 

  #ifndef ULONG_PTR
    #define ULONG_PTR   unsigned long
    #define PULONG_PTR  unsigned long*
  #endif

  #ifndef NULL
    #ifdef __cplusplus
      #define NULL    0
    #else
      #define NULL    ((void *)0)
    #endif
  #endif

  #ifndef CONST
    #define CONST     const
  #endif

  #ifndef CALLBACK
    #define CALLBACK
  #endif

#else // TEC_WIN is defined
  #ifndef UBYTE
    #define UBYTE			unsigned char
  #endif
#endif 

// Macro for Unicode String define
#ifndef _TU
  #ifdef _WIN32
    #define _TU(x) L ## x
  #elif defined _TEC_UNIX
    #define _TU(x) L##x
  #else 
    #define _TU(x) x
  #endif    
#endif

#ifndef _T
  #ifdef _UNICODE
    #define _T(x)      _TU(x)
  #else
    #define _T(x)      x
  #endif
#endif

// ----------------------------------------------------------------------
// Bool Values defines                                                                     
// ----------------------------------------------------------------------
#ifndef FALSE
  #define FALSE			0
#endif

#ifndef TRUE
  #define TRUE			!FALSE
#endif

// ----------------------------------------------------------------------
// ErrCode defines                                                                     
// ----------------------------------------------------------------------
#ifndef ERRCODE
  #define	ERRCODE		LONG  
#endif

#ifndef ErrOk
  #define	ErrOk		  S_OK
#endif

#ifndef _WIN32
  #ifndef HANDLE  
		#ifdef QT_CORE_LIB
			// replaced macro with typedef, in order to fix the conflict 
			// with the QT framework variable Qt::HANDLE 
			typedef VOID* HANDLE;
		#else
			#define HANDLE VOID*
		#endif
  #endif

  #ifndef HDC
    #define HDC HANDLE
  #endif

  #ifndef HINSTANCE
    #define HINSTANCE HANDLE
  #endif

  #ifndef HMODULE
    #define HMODULE HANDLE
  #endif

  #ifndef HGDIOBJ
    #define HGDIOBJ HANDLE
  #endif

  #ifndef HPEN
    #define HPEN HANDLE
  #endif

  #ifndef HFONT
    #define HFONT HANDLE
  #endif

  #ifndef HBRUSH
    #define HBRUSH HANDLE
  #endif

  #ifndef INVALID_HANDLE_VALUE
    #define INVALID_HANDLE_VALUE   ((HANDLE)0xFFFFFF)
  #endif

	#ifndef UNUSED
		#define UNUSED(x)
  #endif
#endif

// ----------------------------------------------------------------------
// WindowsStructs and various non simple datatypes
// ----------------------------------------------------------------------
#ifdef _WIN32
  #define	TIMEB	struct	_timeb
#endif

#if defined TEC_SOLARIS
typedef unsigned long u_int32_t;
#endif 

#ifndef _WIN32

  #define TIMEB	struct	timeb
  
  typedef struct tagSIZE { 
    LONG cx; 
    LONG cy; 
  } SIZE, *LPSIZE;
  
  typedef struct tagPOINT 
  {
    LONG   x;
    LONG   y;
  } POINT, *LPPOINT; 
  
  typedef struct tagRECT 
  {
    LONG left;
    LONG top;
    LONG right;
    LONG bottom;
  } RECT, *LPRECT;  

  typedef const RECT*    LPCRECT;
  typedef const SIZE*    LPCSIZE;
  typedef const POINT*   LPCPOINT;
    
  #define LF_FACESIZE 32
  typedef struct tagLOGFONTA
  {
      LONG      lfHeight;
      LONG      lfWidth;
      LONG      lfEscapement;
      LONG      lfOrientation;
      LONG      lfWeight;
      BYTE      lfItalic;
      BYTE      lfUnderline;
      BYTE      lfStrikeOut;
      BYTE      lfCharSet;
      BYTE      lfOutPrecision;
      BYTE      lfClipPrecision;
      BYTE      lfQuality;
      BYTE      lfPitchAndFamily;
      CHAR      lfFaceName[LF_FACESIZE];
  } LOGFONTA, *PLOGFONTA, NEAR *NPLOGFONTA, FAR *LPLOGFONTA;

  typedef struct tagLOGFONTW {
    LONG lfHeight;
    LONG lfWidth;
    LONG lfEscapement;
    LONG lfOrientation;
    LONG lfWeight;
    BYTE lfItalic;
    BYTE lfUnderline;
    BYTE lfStrikeOut;
    BYTE lfCharSet;
    BYTE lfOutPrecision;
    BYTE lfClipPrecision;
    BYTE lfQuality;
    BYTE lfPitchAndFamily;
    WCHAR lfFaceName[LF_FACESIZE];
   } LOGFONTW, *PLOGFONTW, NEAR *NPLOGFONTW, FAR *LPLOGFONTW;

#ifdef _UNICODE
  #define LOGFONT LOGFONTW
  #define LPLOGFONT LPLOGFONTW
#else
  #define LOGFONT LOGFONTA
  #define LPLOGFONT LPLOGFONTA
#endif

  // color
  typedef DWORD     COLORREF;
  typedef COLORREF* LPCOLORREF;
  
  /* Logical Brush (or Pattern) */
  typedef struct tagLOGBRUSH
  {
    UINT        lbStyle;
    COLORREF    lbColor;
    ULONG_PTR   lbHatch;    // Sundown: lbHatch could hold a HANDLE
  } LOGBRUSH, *PLOGBRUSH, NEAR *NPLOGBRUSH, FAR *LPLOGBRUSH;

  #define _XFORM_
  typedef struct  tagXFORM
  {
    FLOAT   eM11;
    FLOAT   eM12;
    FLOAT   eM21;
    FLOAT   eM22;
    FLOAT   eDx;
    FLOAT   eDy;
  } XFORM, *PXFORM, FAR *LPXFORM;

  // error handling
  typedef LONG      HRESULT;

  // systemtime      
  typedef struct _SYSTEMTIME {
    WORD wYear;
    WORD wMonth;
    WORD wDayOfWeek;
    WORD wDay;
    WORD wHour;
    WORD wMinute;
    WORD wSecond;
    WORD wMilliseconds;
  } SYSTEMTIME, *PSYSTEMTIME, *LPSYSTEMTIME;

#else // _ Win32 is defined

  typedef const RECT*    LPCRECT;
  typedef const SIZE*    LPCSIZE;
  typedef const POINT*   LPCPOINT;

#endif // !defined(TEC_WIN)

// ------------------------------------------------------------------------------
// some strange datatypes from old TECITStd
struct _RPC_ASYNC_STATE;		/* to avoid warnings                         */

#endif // _TECDataTypes_h_
