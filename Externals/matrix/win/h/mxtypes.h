/***************************************************************************/
/*  MXTYPES.H   Definition of data types.                                  */
/*                                                                         */
/*  (C) TDi GmbH                                                           */
/*                                                                         */
/*  Include File for C/C++ (32-Bit and 64-Bit)                             */
/***************************************************************************/
#ifndef _MATRIXAPI_DATA_TYPES
#define _MATRIXAPI_DATA_TYPES

#include <limits.h>

#ifdef __cplusplus
extern "C" {
#endif

#if ULONG_MAX == 0xffffffff
  #define  _mxINT32  long
  #define  _mxUINT32 unsigned long
  #define  _mxINT16  short
  #define  _mxUINT16 unsigned short
#elif UINT_MAX == 0xffffffff
  #define  _mxINT32  int
  #define  _mxUINT32 unsigned int
  #define  _mxINT16  short
  #define  _mxUINT16 unsigned short 
#else
  #error can not define Matrix-API data types
#endif


#ifdef __cplusplus
}
#endif

#endif  // _MATRIXAPI_DATA_TYPES

