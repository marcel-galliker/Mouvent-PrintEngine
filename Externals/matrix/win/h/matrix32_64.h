/*****************************************************************************/
/*  MXAPI.H   MS-WINDOWS Win32 & Win64 (95/98/NT/2K/XP/XP64)                 */
/*                                                                           */
/*  (C) TDi GmbH                                                             */
/*                                                                           */
/*  Include File for Visual C/C++ (32- and 64-Bit)                           */
/*****************************************************************************/
#ifndef _MATRIXAPI_H
#define _MATRIXAPI_H

#include "mxtypes.h"     // platform independent data types

#define IW_DRIVER    1
#define IW_NODRIVER  2

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
  _mxINT16 LPT_Nr;
  _mxINT16 LPT_Adr;
  _mxINT16 DNG_Cnt;
} DNGINFO;

_mxINT16  __stdcall Init_MatrixAPI(void);
_mxINT16  __stdcall Release_MatrixAPI(void);
_mxINT32  __stdcall GetVersionAPI(void);
_mxINT32  __stdcall GetVersionDRV(void);
_mxINT32  __stdcall GetVersionDRV_USB(void);
void      __stdcall SetW95Access(_mxINT16);
_mxINT16  __stdcall GetPortAdr(_mxINT16);
_mxINT16  __stdcall Dongle_WriteData(_mxINT32, _mxINT32*, _mxINT16, _mxINT16, _mxINT16);
_mxINT16  __stdcall Dongle_WriteDataEx(_mxINT32, _mxINT32*, _mxINT16, _mxINT16, _mxINT16, _mxINT16);
_mxINT16  __stdcall Dongle_ReadData(_mxINT32, _mxINT32*, _mxINT16, _mxINT16, _mxINT16);
_mxINT16  __stdcall Dongle_ReadDataEx(_mxINT32, _mxINT32*, _mxINT16, _mxINT16, _mxINT16, _mxINT16);
_mxINT32  __stdcall Dongle_ReadSerNr(_mxINT32, _mxINT16, _mxINT16);
_mxINT16  __stdcall Dongle_Exit(void);
_mxINT16  __stdcall Dongle_Find(void);
_mxINT16  __stdcall Dongle_FindEx(DNGINFO *);
_mxINT32  __stdcall Dongle_Version(_mxINT16, _mxINT16);
_mxINT32  __stdcall Dongle_Model(_mxINT16, _mxINT16);
_mxINT16  __stdcall Dongle_MemSize(_mxINT16, _mxINT16);
_mxINT16  __stdcall Dongle_Count(_mxINT16);
_mxINT16  __stdcall PausePrinterActivity(void);
_mxINT16  __stdcall ResumePrinterActivity(void);
_mxINT16  __stdcall SetConfig_MatrixNet(_mxINT16, char*);
_mxINT16  __stdcall LogIn_MatrixNet(_mxINT32, _mxINT16, _mxINT16);
_mxINT16  __stdcall LogOut_MatrixNet(_mxINT32, _mxINT16, _mxINT16);
_mxINT32  __stdcall GetConfig_MatrixNet(_mxINT16 Category);

_mxINT16  __stdcall Dongle_WriteKey(_mxINT32 UserCode, 
                                    _mxINT32 *KeyData, 
                                    _mxINT16 DongleNr, 
                                    _mxINT16 PortNr);

_mxINT16  __stdcall Dongle_GetKeyFlag(_mxINT32 UserCode, _mxINT16 DongleNr, _mxINT16 PortNr);

_mxINT16  __stdcall Dongle_EncryptData(_mxINT32 UserCode, _mxINT32 *DataBlock, _mxINT16 DongleNr, _mxINT16 PortNr);
_mxINT16  __stdcall Dongle_DecryptData(_mxINT32 UserCode, _mxINT32 *DataBlock, _mxINT16 DongleNr, _mxINT16 PortNr);

_mxINT16  __stdcall Dongle_SetDriverFlag(_mxINT32 UserCode, _mxINT16 DriverFlag, _mxINT16 DongleNr, _mxINT16 PortNr);
_mxINT16  __stdcall Dongle_GetDriverFlag(_mxINT32 UserCode, _mxINT16 DongleNr, _mxINT16 PortNr);


#ifdef __cplusplus
}
#endif

#endif  // _MATRIXAPI_H

