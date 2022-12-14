/*===========================================================================*/
/*                        Copyright (C) 1996-2006 by                         */
/*                                                                           */
/*                    -- TEC-IT Datenverarbeitung GmbH --                    */
/*   -- team for engineering and consulting in information technologies --   */
/*                                                                           */
/*                           All rights reserved.                            */
/*                                                                           */
/*              This is a part of the TEC-IT Standard Software.              */
/*                                                                           */
/*    This source code is only intended as a supplement to the References    */
/*      and related electronic documentation provided with the product.      */
/*                                                                           */
/*     See these sources for detailed information regarding this product     */
/*===========================================================================*/

#ifndef __TECIT_STD_H__
#define __TECIT_STD_H__

#undef TECIT_DLLSPEC
#ifdef _WIN32
  #ifdef TECIT_DLLEXPORT
    #define TECIT_DLLSPEC _declspec (dllexport)
  #endif
  #ifdef TECIT_DLLIMPORT
    #define TECIT_DLLSPEC _declspec (dllimport)
  #endif
#endif // _WIN32

#ifndef TECIT_DLLSPEC
  #define TECIT_DLLSPEC
#endif

#ifdef _WIN32
/* #pragma's for WIN32 should be defined as such,
   so that they get only used on WIN32.
   Normally a compiler should ignore them, if he does not 
   know such a pragma - the old SuSE 7.2 compiler
   produced strange results because of them.
*/

  #pragma warning (disable : 4786)	/* 'identifier' : identifier was truncated to 'number' characters in the debug information		  */
#ifdef _WIN32_WCE
  #pragma warning (disable : 4201) /* nonstandard extension used : nameless struct/union */
#else
  #pragma warning (3 : 4201)
#endif

  //#pragma warning (disable : 4251)	/* needs to have dll-interface																	  */
  #pragma warning (3 : 4032 4268 4220 4665 4673 4709 4674 4504 4706 4213 4245 4125 4061 4202 4208)
  #pragma warning (3 : 4211 4222 4209 4239 4671 4507 4670 4134 4207 4212 4057 4238 4130 4206)
  #pragma warning (3 : 4092 4727 4128 4705 4232 4127 4517 4505 4516 4152 4121 4189 4672 4701)
  #pragma warning (3 : 4611 4131 4236 4132)
#endif // _WIN32

///////////////////////////////////////////////////////////////////////////////
// TECITStd.h
// Standard header file for TEC-IT projects 
//
// ATTENTION!
// If used with MFC Applications this File must not be included before any 
// MFC-include-file!!
//
///////////////////////////////////////////////////////////////////////////////

// will be defined via included "TECDataTypes.h" below
/*struct _RPC_ASYNC_STATE;*/		/* to avoid warning                          */


/* HAVE_CONFIG_H and config.h is generated by Automake/Autoconf (Linux).
   config.h is used to define useful symbols (TEC_LINUX, TEC_AIX, 
   WORDS_BIGENDIAN, ...).
*/
#ifdef HAVE_CONFIG_H
  #include <config.h>
#endif


/*
   OS symbols
   ==========
  
   To distinguish different platforms, the following symbols are used:
  
   TEC_UNIX .... Unix platform (Linux, AIX, HPUX, Solaris, ...)
   TEC_WIN ..... Microsoft Windows platform (Win32, WinCE, ...)

   TEC_LINUX ..... GNU Linux
   TEC_FREEBSD ... FreeBSD
   TEC_AIX ....... IBM AIX
   TEC_HPUX ...... HP-UX
   TEC_SOLARIS ... Sun Solaris
   TEC_WIN32 ..... Microsoft Windows 32-bit Desktop platforms (i.e. NOT WinCE) AND compatible platforms like Windows 64-bit
   TEC_WIN64 ..... Microsoft Windows 64-bit Desktop platforms (i.e. NOT WinCE)
   TEC_WINCE ..... Microsoft Windows Embedded platforms (i.e. WinCE)
   TEC_OS400 ..... IBM OS400
   TEC_SCO ....... SCO UNIX (OpenServer, OpenUNIX, UnixWare)
   TEC_MAC ....... Mac OS
  
   TEC_LINUX, TEC_FREEBSD, TEC_AIX, TEC_HPUX, TEC_SOLARIS, TEC_OS400, TEC_SCO must be set in project files (Makefiles or config.h).
   The other symbols are set automatically.
*/

//
// Check for Windows platform
//
#if defined(WIN32) || defined(_WIN32) || defined(_WIN32_WCE) || defined(WIN64) || defined(_WIN64)
  #define TEC_WIN 1
  #if defined(UNDER_CE) || defined(_WIN32_WCE)
    #define TEC_WINCE 1
  #elif defined(WIN64) || defined(_WIN64)
    #define TEC_WIN32 1
    #define TEC_WIN64 1    
  #else
    #define TEC_WIN32 1
  #endif
#endif

//
// Check for UNIX platform
//
#if defined(TEC_LINUX) || defined(TEC_FREEBSD) || defined(TEC_AIX) || defined(TEC_HPUX) || defined(TEC_SOLARIS) || defined(TEC_OS400) || defined(TEC_SCO) || defined(TEC_MAC)
  #define TEC_UNIX 1
  #define _TEC_UNIX 1
  #ifdef TEC_MAC
	#define TEC_FREEBSD
  #endif
#endif

// 
// Perform some checks to see whether defines are correct.
//
#if defined(TEC_WIN) && defined (TEC_UNIX)
  #error TEC_WIN and TEC_UNIX must not be defined simultaneously!
#endif
#if !defined(TEC_WIN) && !defined (TEC_UNIX)
  #error Either TEC_WIN or TEC_UNIX must be defined!
#endif
#if defined(TEC_WINCE) && defined(TEC_WIN32)
  #error TEC_WINCE and TEC_WIN32 must not be defined simultaneously!
#endif
#if defined(TEC_WINCE) && defined(TEC_WIN64)
  #error TEC_WINCE and TEC_WIN64 must not be defined simultaneously!
#endif



#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
/** malloc.h was replaced by stdlib.h on Unix systems
  * FreeBSD does even refuse to compile if this header
  * is used.
  * Win32 and linux allow malloc.h and require it for 
  * _msize (malloc_usable_size)
  */
#if defined _WIN32 || defined _TEC_UNIX
  #ifndef TEC_FREEBSD                   // malloc.h has been replaced with stdlib.h on FreeBSD
                                        // malloc.h causes error in FreeBSD
   #include <malloc.h>
  #endif
#endif // _WIN32 || _TEC_UNIX

#include <memory.h>
#include <math.h>
#include <limits.h>
#include <stdarg.h>

// WINCE and EAGRTOS don't support errno.h
#if !defined(TEC_WINCE) && (!defined TEC_EAGRTOS)  /* Windows CE */
  #include <errno.h>			// Most Windows CE SDKs don't have this header
#endif

#ifdef _WIN32
  #include <tchar.h>
  #include <stddef.h>
  #include "WTypes.h"
  #include "limits.h"

  #ifndef _WIN32_WCE   /* Windows CE                                          */
    #include <new.h>	
    #if _MSC_VER >= 1300 /* VC 7.1 */
      // Not available and it seems as not needed also
      //#include <iostream.h>
    #else
      #include <iostream.h>
    #endif
  #endif // _WIN32_WCE
  
  #ifndef __AFX_H__			/* MFC Application ?							 */
    #include <windows.h>          
  #endif
  
#else 
  #include <stddef.h>
  #include <wchar.h>
#endif	

//////////////////////////////////////////////////////////////////////////////
// STL includes
//////////////////////////////////////////////////////////////////////////////
#include "TECStl.h"

///////////////////////////////////////////////////////////////////////////////
//	Typ-Definitionen
///////////////////////////////////////////////////////////////////////////////
#include "TECDataTypes.h"

///////////////////////////////////////////////////////////////////////////////
// TECWinErrors
///////////////////////////////////////////////////////////////////////////////
#include "TECWinError.h"

///////////////////////////////////////////////////////////////////////////////
// TECDefines
///////////////////////////////////////////////////////////////////////////////
#include "TECDefines.h"


///////////////////////////////////////////////////////////////////////////////
// TECDebug is not included by default
///////////////////////////////////////////////////////////////////////////////
// #include "TECDebug.h"

///////////////////////////////////////////////////////////////////////////////
// Additional UNIX stuff
///////////////////////////////////////////////////////////////////////////////
#if defined(_TEC_UNIX) || defined(TEC_WINCE)
  /** TEC_UNIX_BUILD
    * is only defined when the TECUnix library is build
    * TECUnix includes TECITStd because it needs access
    * to the datatypes defined here. Of course it is not 
    * possible and required to include the TECUnix/TECUnix.h
    * header here, when TECUnix is build for the first time
    * on a system.
    */
  #ifndef TEC_UNIX_BUILD
    #include <TECUnix/TECUnix.h>
  #endif // TEC_UNIX_BUILD
#endif // _TEC_UNIX

#endif					/* #ifndef __TECIT_STD_H__	*/
