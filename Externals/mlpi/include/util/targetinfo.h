#ifndef __TARGETINFO_H__
#define __TARGETINFO_H__
/**
==============================================================================
@file 
@copyright  Bosch Rexroth Corporation http://www.boschrexroth.com
@date       2012
------------------------------------------------------------------------------
This header tries to detect the used compiler, operating system and cpu
of the target platform.
The introduced defines for CPU, OS and COMPILER are mutual exclusive.

At the moment, following defines are supported and will be set:
 - TARGET_COMPILER_INTEL
 - TARGET_COMPILER_GCC
 - TARGET_COMPILER_MSVC
 - TARGET_COMPILER_BORLAND
 - TARGET_OS_VXWORKS
 - TARGET_OS_WINAPP
 - TARGET_OS_WINNT64
 - TARGET_OS_WINNT32
 - TARGET_OS_WINCE32
 - TARGET_OS_IOS
 - TARGET_OS_IOS_SIM
 - TARGET_OS_MAC
 - TARGET_OS_ANDROID
 - TARGET_OS_LINUX
 - TARGET_OS_QNX
 - TARGET_OS_SYMBIAN
 - TARGET_OS_UNIX
 - TARGET_OS_POSIX
 - TARGET_OS_MINGW
 - TARGET_CPU_PPC64
 - TARGET_CPU_PPC32
 - TARGET_CPU_SH3
 - TARGET_CPU_SH4
 - TARGET_CPU_ARM
 - TARGET_CPU_MIPS
 - TARGET_CPU_IA64
 - TARGET_CPU_X64
 - TARGET_CPU_X86
==============================================================================
*/


/* 
------------------------------------------------------------------------------
 Detect compiler
------------------------------------------------------------------------------
*/
#if defined(__ECC) || defined(__ICC) || defined(__INTEL_COMPILER)
  #define TARGET_COMPILER_STRING "Intel C/C++"
  #define TARGET_COMPILER_INTEL 1

#elif defined(__GNUC__)
  #define TARGET_COMPILER_STRING "Gnu GCC"
  #define TARGET_COMPILER_GCC 1

#elif defined(_MSC_VER)
  #define TARGET_COMPILER_STRING "Microsoft Visual C++"
  #define TARGET_COMPILER_MSVC 1

#elif defined(__BORLANDC__)
  #define TARGET_COMPILER_STRING "Borland C/C++"
  #define TARGET_COMPILER_BORLAND 1

#else
  #define TARGET_COMPILER_STRING "Unknown compiler"
  #error failed to detect compiler! 
#endif




/* 
------------------------------------------------------------------------------
 Detect operating system
------------------------------------------------------------------------------
*/
#if defined(__VXWORKS__)
  #define TARGET_OS_STRING "VxWorks"
  #define TARGET_OS_VXWORKS 1

#elif defined(WINAPI_FAMILY) && (WINAPI_FAMILY == WINAPI_FAMILY_APP)
  #define TARGET_OS_STRING "Windows App"
  #define TARGET_OS_WINAPP 1

#elif defined(_WIN64)
  #define TARGET_OS_STRING "WindowsNT 64bit"
  #define TARGET_OS_WINNT64 1

#elif defined(_WIN32_WINNT) || defined(_WINDLL) || defined (_WIN32)
  #define TARGET_OS_STRING "WindowsNT 32bit"
  #define TARGET_OS_WINNT32 1
  
#elif defined(_WIN32_WCE)
  #define TARGET_OS_STRING "WindowsCE 32bit"
  #define TARGET_OS_WINCE32 1

#elif defined(__APPLE__)
  #include "TargetConditionals.h"
  #if TARGET_OS_IPHONE    
    #define TARGET_OS_STRING "Apple iOS"
    #define TARGET_OS_IOS 1

  #elif TARGET_IPHONE_SIMULATOR
    #define TARGET_OS_STRING "Apple iOS Simulator"
    #define TARGET_OS_IOS_SIM 1

  #elif TARGET_OS_MAC
    #define TARGET_OS_STRING "Apple Macintosh"
    #define TARGET_OS_MAC 1

  #else
    #define TARGET_OS_STRING "Apple Unknown"
  #endif

#elif defined(__ANDROID__)
  #define TARGET_OS_STRING "Android"
  #define TARGET_OS_ANDROID 1

#elif defined(__linux__) ||  defined(__linux) || defined(linux)
  #define TARGET_OS_STRING "Linux"
  #define TARGET_OS_LINUX 1

#elif defined(__QNX__) || defined(__QNXNTO__)
  #define TARGET_OS_STRING "QNX"
  #define TARGET_OS_QNX 1

#elif defined(__SYMBIAN32__)
  #define TARGET_OS_STRING "Symbian"
  #define TARGET_OS_SYMBIAN 1

#elif defined(__unix) 
  #define TARGET_OS_STRING "Unix"
  #define TARGET_OS_UNIX 1

#elif defined(__posix)
  #define TARGET_OS_STRING "Posix"
  #define TARGET_OS_POSIX 1

#elif defined(__MINGW32__)
  #define TARGET_OS_STRING "MinGW"
  #define TARGET_OS_MINGW 1

#else
  #define TARGET_OS_STRING "Unknown operating system"
  #error failed to detect operating system!
#endif




/* 
------------------------------------------------------------------------------
 Detect CPU
------------------------------------------------------------------------------
*/
#if defined(__PPC__) || defined(__POWERPC__) || defined(_POWER) || defined(__ppc__) || defined(__powerpc__)
  #if defined(__powerpc64__)
    #define TARGET_CPU_STRING "PowerPC64"
    #define TARGET_CPU_PPC64 1

  #else
    #define TARGET_CPU_STRING "PowerPC"
    #define TARGET_CPU_PPC32 1

  #endif
#elif defined(_SH3)
  #define TARGET_CPU_STRING "Hitachi SH3"  
  #define TARGET_CPU_SH3 1

#elif defined(__sh4__) || defined(__SH4__)
  #define TARGET_CPU_STRING "Hitachi SH4"
  #define TARGET_CPU_SH4 1

#elif  defined(__arm__) || defined(_ARM)
  #define TARGET_CPU_STRING "ARM"
  #define TARGET_CPU_ARM 1

#elif defined(__mips__) || defined(__MIPS__) || defined(_MIPS)
  #define TARGET_CPU_STRING "MIPS"
  #define TARGET_CPU_MIPS 1 

#elif defined(__ia64) || defined(_M_IA64) || defined(__ia64__)
  #define TARGET_CPU_STRING "IA64"
  #define TARGET_CPU_IA64 1

#elif defined(__X86__) || defined(__i386__) || defined(i386) || defined(_M_IX86) || defined(__386__) || defined(__x86_64__) || defined(_M_X64)
  #if defined(__x86_64__) || defined(_M_X64)
    #define TARGET_CPU_STRING "AMD x86-64"
    #define TARGET_CPU_X64 1
  #else
    #define TARGET_CPU_STRING "Intel x86"
    #define TARGET_CPU_X86 1
  #endif

#else
  #define TARGET_CPU_STRING "Unknown CPU"
  #error failed to detect CPU!
#endif




/*
==============================================================================
History
------------------------------------------------------------------------------
10-Sep-2012 SK
  - first version
11-Sep-2012 SK Subject: MSC: cleanup of globaltype
  - corrected and reformated defines
==============================================================================
*/

#endif /* __TARGETINFO_H__ */



