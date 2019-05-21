/*=====================================================================
  ===========     MXAPI.H  -  Matrix-API header file       ============
  =====================================================================
  ===============       Copyright (c) TDi GmbH        =================
  =====================================================================*/

#include "mxtypes.h"

typedef struct
{
  _mxINT16 LPT_Nr;
  _mxINT16 LPT_Adr;
  _mxINT16 DNG_Cnt;
} DNGINFO;

#ifdef __cplusplus
extern "C"
{
#endif

_mxINT16  Init_MatrixAPI(void);
_mxINT16  Release_MatrixAPI(void);
_mxINT32   GetVersionAPI(void);
_mxINT32   GetVersionDRV(void);
_mxINT32   GetVersionDRV_USB(void);
_mxINT16  GetPortAdr(_mxINT16);
_mxINT16  Dongle_WriteData(_mxINT32, _mxINT32*, _mxINT16, _mxINT16, _mxINT16);
_mxINT16  Dongle_WriteDataEx(_mxINT32, _mxINT32*, _mxINT16, _mxINT16, _mxINT16, _mxINT16);
_mxINT16  Dongle_ReadData(_mxINT32, _mxINT32*, _mxINT16, _mxINT16, _mxINT16);
_mxINT16  Dongle_ReadDataEx(_mxINT32, _mxINT32*, _mxINT16, _mxINT16, _mxINT16, _mxINT16);
_mxINT32   Dongle_ReadSerNr(_mxINT32, _mxINT16, _mxINT16);
_mxINT16  Dongle_Exit(void);
_mxINT16  Dongle_Find(void);
_mxINT16  Dongle_FindEx(DNGINFO *);
_mxINT32   Dongle_Version(_mxINT16, _mxINT16);
_mxINT32   Dongle_Model(_mxINT16, _mxINT16);
_mxINT16  Dongle_MemSize(_mxINT16, _mxINT16);
_mxINT16  Dongle_Count(_mxINT16);
_mxINT16  Dongle_SetLedFlag(_mxINT32, _mxINT16, _mxINT16, _mxINT16);

_mxINT16  Dongle_WriteKey(_mxINT32 UserCode, 
                       _mxINT32 *KeyData, 
                       _mxINT16 DongleNr, 
                       _mxINT16 PortNr);


_mxINT16  Dongle_GetKeyFlag(_mxINT32 UserCode, _mxINT16 DongleNr, _mxINT16 PortNr);

_mxINT16  Dongle_EncryptData(_mxINT32 UserCode, _mxINT32 *DataBlock, _mxINT16 DongleNr, _mxINT16 PortNr);

_mxINT16  Dongle_DecryptData(_mxINT32 UserCode, _mxINT32 *DataBlock, _mxINT16 DongleNr, _mxINT16 PortNr);

_mxINT16  Dongle_SetDriverFlag(_mxINT32 UserCode, _mxINT16 DriverFlag, _mxINT16 DongleNr, _mxINT16 PortNr);
_mxINT16  Dongle_GetDriverFlag(_mxINT32 UserCode, _mxINT16 DongleNr, _mxINT16 PortNr);



#ifdef __cplusplus
}
#endif

