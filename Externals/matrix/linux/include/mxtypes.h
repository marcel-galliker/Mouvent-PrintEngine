/*****************************************************************************/
/*  MXTYPES.H   MS-WINDOWS Win32 and Win64 - definition of data types        */
/*                                                                           */
/*  (C) TDi GmbH                                                             */
/*                                                                           */
/*  Include File for Visual C/C++ (32-Bit and 64 -Bit)                       */
/*****************************************************************************/
#ifndef _MATRIXAPI_DATA_TYPES
#define _MATRIXAPI_DATA_TYPES

#define __STDC_FORMAT_MACROS
#include <stdint.h>
#include <inttypes.h>

#ifdef __cplusplus
extern "C" {
#endif

#define  _mxUINT8   uint8_t
#define  _mxINT8    int8_t
#define  _mxINT32   int32_t
#define  _mxINT16   int16_t
#define  _mxUINT32  uint32_t
#define  _mxUINT16  uint16_t 

#ifdef __cplusplus
}
#endif

#endif  // _MATRIXAPI_DATA_TYPES

