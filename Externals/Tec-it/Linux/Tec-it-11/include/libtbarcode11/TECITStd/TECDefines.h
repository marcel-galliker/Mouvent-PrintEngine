// ----------------------------------------------------------------------------
// file name: TECDefines.h
//    author: Dominik Seichter (TEC-IT)
// copyright: (c) TEC-IT Datenverarbeitung GmbH., 2004-2006
// ----------------------------------------------------------------------------
//   content: 
// ----------------------------------------------------------------------------
// history: 
// (DS 2004-07-19) : creation
// ----------------------------------------------------------------------------

#ifndef _TEC_DEFINES_H_
#define _TEC_DEFINES_H_

// Min, Max Macros
#ifndef __max
  #define __max(a,b)  (((a) > (b)) ? (a) : (b))
#endif    

#ifndef __min
  #define __min(a,b)  (((a) < (b)) ? (a) : (b))
#endif

///////////////////////////////////////////////////////////////////////////////
// TEC-IT SIZEOF macros
///////////////////////////////////////////////////////////////////////////////
#define	BYTESIZEOF(v)			(sizeof (v))
#define	CHARSIZEOF(v)			(sizeof (v) / sizeof (v[0]))
#define	ELEMSIZEOF(v)			CHARSIZEOF(v)
#define ARRAYSIZEOF(nLen,type)	((nLen) * sizeof (type))


///////////////////////////////////////////////////////////////////////////////
// TEC-IT macros for #pragma message 
// Sample: #pragma message (__LOC__ "** Some Message")
// will output file (lineNr): ** Some Message to the build window
// dblClick the line  VC will open the specific file at the given lineNr
///////////////////////////////////////////////////////////////////////////////
#define __LOC_STR2__(x) #x
#define __LOC_STR1__(x) __LOC_STR2__(x)
#define __LOC__ __FILE__ "(" __LOC_STR1__(__LINE__) ") : "

///////////////////////////////////////////////////////////////////////////////
// TEC-IT memory allocation (SINGLE BYTE / UNICODE enabled)
///////////////////////////////////////////////////////////////////////////////
#define	BYTEALLOC(nLen)			(malloc (nLen))
#define	CHARALLOC(nLen)			((LPTSTR)calloc (nLen, sizeof (TCHAR)))
#define	CHARALLOCA(nLen)	    ((LPSTR)calloc (nLen, sizeof (CHAR)))
#define	CHARALLOCW(nLen)	    ((LPWSTR)calloc (nLen, sizeof (WCHAR)))
#define BYTEREALLOC(p, nLen)	(realloc (p, nLen))
#define CHARREALLOC(p, nLen)	((LPTSTR)realloc (p, nLen * sizeof (TCHAR)))
#define CHARREALLOCA(p, nLen)	((LPSTR)realloc (p, nLen * sizeof (CHAR)))
#define CHARREALLOCW(p, nLen)	((LPSTR)realloc (p, nLen * sizeof (WCHAR)))

#define SAVE_CPY(dest,src) { _tcsncpy (dest, src, sizeof (dest) / sizeof (dest[0]) - 1); dest[sizeof (dest) / sizeof (dest[0]) - 1] = _T('\0'); }
#define SAVE_CPY_A(dest,src) { strncpy (dest, src, sizeof (dest) / sizeof (dest[0]) - 1); dest[sizeof (dest) / sizeof (dest[0]) - 1] = '\0'; }
#define SAVE_CPY_W(dest,src) { wcsncpy (dest, src, sizeof (dest) / sizeof (dest[0]) - 1); dest[sizeof (dest) / sizeof (dest[0]) - 1] = L'\0'; }
#define SAVE_CAT(dest,src) { _tcsncat (dest, src, (sizeof (dest) / sizeof (dest[0])) - _tcslen (dest) - 1); dest[sizeof (dest) / sizeof (dest[0]) - 1] = _T('\0'); }
#define SAVE_CAT_A(dest,src) { strncat (dest, src, (sizeof (dest) / sizeof (dest[0])) - strslen (dest) - 1); dest[sizeof (dest) / sizeof (dest[0]) - 1] = '\0'; }
#define SAVE_CAT_W(dest,src) { wcsncat (dest, src, (sizeof (dest) / sizeof (dest[0])) - wcslen (dest) - 1); dest[sizeof (dest) / sizeof (dest[0]) - 1] = L'\0'; }

///////////////////////////////////////////////////////////////////////////////
// Type-Safe memory allocation (for arrays of type)
///////////////////////////////////////////////////////////////////////////////
#ifdef __cplusplus
  #define	ELEMALLOC(nLen,type)		(static_cast<type*>(malloc ((nLen) * sizeof (type))))
  #define	ELEMREALLOC(p,nLen,type)	(static_cast<type*>(realloc (p, (nLen) * sizeof (type))))
#endif

// COLORREF Macros 
#ifndef RGB
  #define RGB(r,g,b)          ((COLORREF)(((UBYTE)(r)|((WORD)((UBYTE)(g))<<8))|(((DWORD)(UBYTE)(b))<<16)))
#endif

#ifndef RGBGetRValue
  #define RGBGetRValue(rgb)      ((UBYTE)(rgb))
#endif

#ifndef RGBGetGValue
  #define RGBGetGValue(rgb)      ((UBYTE)(((WORD)(rgb)) >> 8))
#endif

#ifndef RGBGetBValue
  #define RGBGetBValue(rgb)      ((UBYTE)((rgb)>>16))
#endif

#ifndef RGBGetAValue
  #define RGBGetAValue(rgb)      ((UBYTE)((rgb)>>24))
#endif

#ifndef GetRValue
  #define GetRValue(rgb)         ((UBYTE)(rgb))
#endif

#ifndef GetGValue
  #define GetGValue(rgb)         ((UBYTE)(((WORD)(rgb)) >> 8))
#endif

#ifndef GetBValue
  #define GetBValue(rgb)         ((UBYTE)((rgb)>>16))
#endif

// Calculates Luminance (0.0..255.0) Double value
// Official NTSC conversion formula for calculating the effective luminance of a pixel
#ifndef RGBGetLuminance
  #define RGBGetLuminance(rgb)   (0.2989*(DOUBLE)RGBGetRValue(rgb) + 0.5870*(DOUBLE)RGBGetGValue(rgb) + 0.1140*(DOUBLE)RGBGetBValue(rgb))
#endif

// Calculates Gray (0.0..255.0) Double value
#ifndef RGBGetGray
  #define RGBGetGray             RGBGetLuminance
#endif

// Converts given RGB Value to Black or White
#ifndef RGBGetBW
  //#define RGBGetBW(rgb)          ((rgb == RGB(255,255,255)) ? rgb : RGB(0,0,0))
  #define RGBGetBW(rgb)          (((RGBGetLuminance(rgb)/255.0) > 0.5) ? RGB(255,255,255) : RGB(0,0,0))
#endif

#ifndef LOBYTE
  #define LOBYTE(b)                ((BYTE)(WORD)(b))
#endif 

#ifndef HIBYTE
  #define HIBYTE(b)                ((BYTE)((WORD)(b) >> 8))
#endif 

//
// ASSERT defines
// -------------------------------------------------------------------
#if !(defined ASSERT) && !(defined NDEBUG)
  #include <assert.h>
  #define ASSERT(x)  assert(x)
#elif !defined ASSERT  
  #define ASSERT(expr)
#endif

// Error Macros
#define W32toTEC	HRESULT_FROM_WIN32

// Ansi & Unicode defines    
/*    
    #define iswalpha(_c)	( isalpha(_c) )
    #define iswupper(_c)	( isupper(_c) )
    #define iswlower(_c)	( islower(_c) )
    #define iswdigit(_c)	( isdigit(_c) )
    #define iswxdigit(_c)	( isxdigit(_c))
    #define iswspace(_c)	( isspace(_c) )
    #define iswpunct(_c)	( ispunct(_c) )
    #define iswalnum(_c)	( isalnum(_c) )
    #define iswprint(_c)	( isprint(_c) )
    #define iswgraph(_c)	( isgraph(_c) )
    #define iswcntrl(_c)	( iscntrl(_c) )
    #define iswascii(_c)	( isascii(_c) )
    #define towupper(_c)	( toupper(_c) )
    #define towlower(_c)	( tolower(_c) )
*/    

#ifndef _INC_TCHAR // if tchar is not included

  #ifdef _TEC_UNIX
    
    #include <stdio.h>

    #ifdef TEC_EAGRTOS
      #include <string.h>
      #define _stricmp stricmp
      #define isascii(c)   (!((c) & ~0x7f))
    #else
      #include <strings.h>
      #define stricmp    strcasecmp
      #define _stricmp   strcasecmp
      #define strnicmp   strncasecmp
      #define _strnicmp  strncasecmp
      #define _itot_s    itoa
    #endif

	#ifdef TEC_SOLARIS
	  #undef __XOPEN_OR_POSIX 
	  #include <widec.h>
	  #define __XOPEN_OR_POSIX
	  #define wcsicmp    wscasecmp
	  #define _wcsicmp   wscasecmp
	  #define wcsnicmp   wsncasecmp
	#else
		#include <wchar.h>
	  #define wcsicmp    wcscasecmp
	  #define _wcsicmp   wcscasecmp
	  #define wcsnicmp   wcsncasecmp
	#endif // TEC_SOLARIS

    #define _snprintf  snprintf
	#define _splitpath _tsplitpath
	#define __iscsym(c) (isalnum(c) || ((c) == '_'))

	// those are needed with _UNICODE
    // and without _UNICODE and are defined
    // in stdlib.h on UNIX
    #include <stdlib.h>
  #endif

  #define _strdup strdup
  #define _strlwr strlwr
  #define _strupr strupr

  #ifdef _UNICODE    
    #define _tcscat     wcscat
    #define _tcschr     wcschr
    #define _tcscpy     wcscpy
    #define _tcscspn    wcscspn
    #define _tcslen     wcslen
    #define _tcsncat    wcsncat
    #define _tcsncpy    wcsncpy
    #define _tcspbrk    wcspbrk
    #define _tcsrchr    wcsrchr
    #define _tcsspn     wcsspn
    #define _tcsstr     wcsstr
    #define _tcstok     wcstok
    #define _tcsftime   wcsftime

    #define _tcsdup     _wcsdup
    #define _tcsnset    _wcsnset
    #define _tcsrev     _wcsrev
    #define _tcsset     _wcsset
      
    #define _tcscmp     wcscmp
    #define _tcsicmp    _wcsicmp
    #define _tcsnccmp   wcsncmp
    #define _tcsncmp    wcsncmp
    #define _tcsncicmp  _wcsnicmp
    
    #define _tcsnicmp   wcsnicmp
    
    #define _tcsupr     wcsupr
    #define _tcslwr     wcslwr
    
	#define _tgetenv    _wgetenv

    #define _stprintf   swprintf
    #define _vsntprintf vswprintf  
    #define _sntprintf  _snwprintf
    #define _ftprinf    _fwprintf
    #define _tprintf    wprintf
    
    #define _stscanf    swscanf
    #define _ttol       _wtol
    #define _ttof       tcl::_wtof
    #define _ttoi       _wtoi
    
    #define _ltot       _ltow
    #define _itot       _itow
    #define _ultot      _ultow
    #define _i64tot     _i64tow
    #define _ui64tot    _ui64tow
        
    #define _tcstol     wcstol
    #define _tcstod     wcstod    
        
    #define _totlower    towlower
    #define _totupper    towupper
    #define _istupper    iswupper
    #define _istlower    iswlower
    #define _istspace    iswspace
    #define _istdigit    iswdigit
    #define _istalpha    iswalpha
    #define _istalnum    iswalnum
    #define _istascii    iswascii
    #define _istxdigit   iswxdigit
    
    // some functions that seem to be not available under linux
    #ifdef _WIN32        
      #define _tcsspnp    _wcsspnp
      #define _tcsinc     _wcsinc
    #endif

    #define _tfopen     _wfopen
    #define _fputts     fputws
    #define _fgetts     fgetws
    #define _fgettc     fgetwc
    #define _ungettc    ungetwc
    
  #else  
    #define _tcscat     strcat
    #define _tcschr     strchr
    #define _tcscpy     strcpy
    #define _tcscspn    strcspn
    #define _tcslen     strlen
    #define _tcsncat    strncat
    #define _tcsncpy    strncpy
    #define _tcspbrk    strpbrk
    #define _tcsrchr    strrchr
    #define _tcsspn     strspn
    #define _tcsstr     strstr
    #define _tcstok     strtok
	#define _tcsftime   strftime

    #define _tcsdup     strdup
    #define _tcsnset    strnset
    #define _tcsrev     strrev
    #define _tcsset     strset
      
    #define _tcscmp     strcmp
        
    #define _tcsicmp    stricmp    
    
    #define _tcsnccmp   strncmp
    #define _tcsncmp    strncmp
    #define _tcsncicmp  strnicmp   
    #define _tcsnicmp   strnicmp
    
    #define _ltot       _ltoa
    #define _itot       _itoa
    #define _ultot      _ultoa
    #define _i64tot     _i64toa
    #define _ui64tot    _ui64toa
       
    #define _tcsupr     strupr
    #define _tcslwr     strlwr
    
	#define _tgetenv    getenv
    
    #define _stprintf   sprintf
    #define _ftprintf   fprintf
    #define _vsntprintf vsnprintf
    #define _sntprintf  snprintf
    #define _tprintf    printf
  
    #define _stscanf    sscanf
    #define _ttol       atol
    #define _ttof       atof
    #define _ttoi       atoi
          
    #define _tcstol     strtol
    #define _tcstod     strtod      
      
    #define _totlower    tolower
    #define _totupper    toupper
    #define _istupper    isupper
    #define _istlower    islower
    #define _istspace    isspace
    #define _istdigit    isdigit    
    #define _istalpha    isalpha
    #define _istascii    isascii
    #define _istalnum    isalnum
    #define _istxdigit   isxdigit
    
    // some functions that seem to be not available under linux
    #ifdef _WIN32  
      #define _tcsspnp    strspnp
      #define _tcsinc     strinc
    #endif

		#define _fgettc     fgetc
		#define _ungettc    ungetc
    #define _tfopen     fopen
    #define _fputts     fputs 
    #define _fgetts     fgets   
    #define _trmdir     rmdir
  #endif // _UNICODE
#else // some defines always needed

  #ifndef _ttof
    #ifdef _UNICODE
      #define _ttof       tcl::_wtof
    #else
      #define _ttof       atof
    #endif
  #endif
#endif // if tchar is not included

#ifdef TEC_WINCE
	#define strnicmp  _strnicmp 
	#define stricmp   _stricmp  
	#define strupr    _strupr
	#define strlwr    _strlwr
#endif

#ifdef _WIN32
  // ----------------------------------------------------------------------
  // wtof is normally not available so lets define it as inline here                                                                     
  // ----------------------------------------------------------------------
// causes problems as inline here
// probably make new lib TECWindows so that callee has to link either TECWindows or TECUnix
// currently moved to TECBase

// HP did not find it in TECBase but needed for UNICODE port
// so reactivated this one
#ifndef _WTOF_DEFINED
#define _WTOF_DEFINED
/*  
  DOUBLE _wtof (LPCWSTR pstVal)
  {
    LPWSTR pEnd;
    DOUBLE  dRes = wcstod (pstVal, &pEnd);
    if (pEnd && pEnd[0])
      return 0.0;
  
    return dRes;
  }
*/
#endif // _WTOF_DEFINED
#elif defined _TEC_UNIX
  // Defined in TECUnix.h because gcc does not like inline functions in headers
  // without library at linking time
#endif
  
// -------------------------------------------------------------------
//  Some GDI defines, which might be unavailable on non Windows systems
// -------------------------------------------------------------------
#ifndef TRANSPARENT  
  #define TRANSPARENT 1
#endif

#ifndef OPAQUE
  #define OPAQUE      2
#endif

#ifndef UNUSED
  #ifdef _DEBUG
    #define UNUSED(x)
  #else
    #define UNUSED(x) x
  #endif
#endif

#ifndef UNUSED_ALWAYS
  #define UNUSED_ALWAYS(x) x
#endif

#ifndef BS_SOLID
  #define BS_SOLID            0
  #define BS_NULL             1
  #define BS_HOLLOW           BS_NULL
  #define BS_HATCHED          2
  #define BS_PATTERN          3
  #define BS_INDEXED          4
  #define BS_DIBPATTERN       5
  #define BS_DIBPATTERNPT     6
  #define BS_PATTERN8X8       7
  #define BS_DIBPATTERN8X8    8
  #define BS_MONOPATTERN      9
#endif

#ifndef HS_HORIZONTAL
  #define HS_HORIZONTAL       0
  #define HS_VERTICAL         1
  #define HS_FDIAGONAL        2
  #define HS_BDIAGONAL        3
  #define HS_CROSS            4
  #define HS_DIAGCROSS        5
#endif 

#ifndef PS_SOLID
  #define PS_SOLID            0
  #define PS_DASH             1       /* -------  */
  #define PS_DOT              2       /* .......  */
  #define PS_DASHDOT          3       /* _._._._  */
  #define PS_DASHDOTDOT       4       /* _.._.._  */
  #define PS_NULL             5
  #define PS_INSIDEFRAME      6
  #define PS_USERSTYLE        7
  #define PS_ALTERNATE        8
  #define PS_STYLE_MASK       0x0000000F
#endif

#ifndef DT_PLOTTER
  #define DT_PLOTTER          0
  #define DT_RASDISPLAY       1
  #define DT_RASPRINTER       2
  #define DT_RASCAMERA        3
  #define DT_CHARSTREAM       4
  #define DT_METAFILE         5
  #define DT_DISPFILE         6
#endif 

#ifndef ETO_OPAQUE
  #define ETO_OPAQUE                   0x0002
  #define ETO_CLIPPED                  0x0004
#endif

#ifndef TA_LEFT
  #define TA_LEFT                      0
  #define TA_RIGHT                     2
  #define TA_CENTER                    6
  #define TA_TOP                       0
  #define TA_BOTTOM                    8
  #define TA_BASELINE                  24
#endif 

#ifndef GM_COMPATIBLE
  #define GM_COMPATIBLE       1
  #define GM_ADVANCED         2
  #define GM_LAST             2
#endif

#ifndef FW_NORMAL
  #define FW_DONTCARE   0
  #define FW_THIN       100
  #define FW_EXTRALIGHT 200
  #define FW_LIGHT      300
  #define FW_NORMAL     400
  #define FW_MEDIUM     500
  #define FW_SEMIBOLD   600
  #define FW_BOLD       700
  #define FW_EXTRABOLD  800
  #define FW_HEAVY      900
  #define FW_ULTRALIGHT FW_EXTRALIGHT
  #define FW_REGULAR    FW_NORMAL
  #define FW_DEMIBOLD   FW_SEMIBOLD
  #define FW_ULTRABOLD  FW_EXTRABOLD
  #define FW_BLACK      FW_HEAVY
#endif

#define CLR_INVALID     0xFFFFFFFF

#ifndef LongToHandle
#define LongToHandle( h )   ((HANDLE)(LONG_PTR) (h) )
#endif

#define GDI_ERROR (0xFFFFFFFFL)
#ifndef HGDI_ERROR
  #define HGDI_ERROR (LongToHandle(0xFFFFFFFFL))
#endif

// -------------------------------------------------------------------
// Defines that are needed on UNIX for Windows portability
// -------------------------------------------------------------------
#if defined _TEC_UNIX 
  #if defined TEC_AIX
  #include <unistd.h>
  #endif
  #define _tsetlocale     setlocale
  
  #define _tunlink       unlink  
  #define _trename       rename

  #ifndef MAX_PATH 
    #define MAX_PATH   PATH_MAX
  #endif

  #define __stdcall
    
  #ifdef __GNUC__
    // malloc_usable_size is only available with GNU libc and might cause
    // problems on other unixes. We need as a _msize replacement, though.
    # define _msize malloc_usable_size
  #endif
#endif // _TEC_UNIX


#if defined(TEC_UNIX)
  #ifndef _MAX_DRIVE
    #define _MAX_DRIVE   MAX_PATH
  #endif
  
  #ifndef _MAX_DIR
    #define _MAX_DIR   MAX_PATH
  #endif
  
  #ifndef _MAX_FNAME
    #define _MAX_FNAME MAX_PATH
  #endif

  #ifndef _MAX_EXT
    #define _MAX_EXT   MAX_PATH   
  #endif
#endif

#ifdef TEC_WINCE
  #ifndef _MAX_DRIVE
    #define _MAX_DRIVE  3
  #endif
  
  #ifndef _MAX_DIR
    #define _MAX_DIR    256
  #endif
  
  #ifndef _MAX_FNAME
    #define _MAX_FNAME  256
  #endif

  #ifndef _MAX_EXT
    #define _MAX_EXT    256
  #endif
#endif

// -------------------------------------------------------------------
// Defines that allow cross platform plug-in systems
// -------------------------------------------------------------------
#ifdef _WIN32
  #define LoadDLL(x)      LoadLibrary(x)
  #define LoadFunction    GetProcAddress
  #define FreeDLL(x)      FreeLibrary(x)
#elif defined _TEC_UNIX
  // TODO: add an extra if section for HP-UX here.
  // HP-UX uses AFAIK shl_load() instead of dlopen()
  #define LoadDLL      dlopen
  #define LoadFunction dlsym
  #define FreeDLL      dlclose
#endif

// -------------------------------------------------------------------
// Defines that specifiy different linux distributions and UNIX Versions
// Those defines should get passed to the compiler during built
// -------------------------------------------------------------------
#ifdef _TEC_UNIX
// #define _TEC_UNIX_SUSE_72
// #define _TEC_UNIX_REDHAT_9
#endif // _TEC_UNIX


// -------------------------------------------------------------------
// Byte-Order: Little-Endian or Big-Endian
//
// In Unix projects the endianes is detected by the configure-script,
// which sets the symbol WORDS_BIGENDIAN.
//
// If the Windows project requires big-endian the symbol
// WORDS_BIGENDIAN needs to be set in the VS project.
// -------------------------------------------------------------------
#ifndef WORDS_BIGENDIAN
// no automake!
#if defined TEC_HPUX && defined _TEC_BIGENDIAN
#define WORDS_BIGENDIAN 1
#endif // TEC_HPUX

#ifdef TEC_AIX
#define WORDS_BIGENDIAN 1
#endif // TEC_AIX

#if defined TEC_SOLARIS && defined __BIG_ENDIAN__
#define WORDS_BIGENDIAN 1
#endif // TEC_SOLARIS

#if defined TEC_MAC && defined __BIG_ENDIAN__
#define WORDS_BIGENDIAN 1
#endif // TEC_SOLARIS
#endif // WORDS_BIGENDIAN

#ifdef WORDS_BIGENDIAN
    #define _TEC_BIGENDIAN
  #else
#define WORDS_LITTLEENDIAN
#endif

// -------------------------------------------------------------------
// BIT SWAP
// 16 bits = 2 bytes
// 32 bits = 4 bytes
// 64 bits = 8 bytes
// -------------------------------------------------------------------
#define TEC_BSWAP16C(x) (((x) << 8 & 0xff00)  | ((x) >> 8 & 0x00ff))
#define TEC_BSWAP32C(x) (TEC_BSWAP16C(x) << 16 | TEC_BSWAP16C((x) >> 16))
#define TEC_BSWAP64C(x) (TEC_BSWAP32C(x) << 32 | TEC_BSWAP32C((x) >> 32))

#define TEC_BSWAP16(x) x = TEC_BSWAP16C(x)
#define TEC_BSWAP32(x) x = TEC_BSWAP32C(x)
#define TEC_BSWAP64(x) x = TEC_BSWAP64C(x)

// -------------------------------------------------------------------
// Macro to compare floating point values using a given 
// epsilon-interval.
// -------------------------------------------------------------------
#ifndef COMPARE_FLOATINGPOINT
  #define COMPARE_FLOATINGPOINT(a,b,epsilon) (-(epsilon) <= ((a)-(b)) && ((a)-(b)) <= (epsilon))
#endif 

// Define tcout, tcerr etc.so we can safly use these in UNICODE and Ansi builds
#ifdef UNICODE
  #define tcout wcout
  #define tcerr wcerr  
  #define tcin  wcin
  #define tclog wclog
#else
  #define tcout cout
  #define tcerr cerr  
  #define tcin  cin
  #define tclog clog
#endif



#endif // _TEC_DEFINES_H_

