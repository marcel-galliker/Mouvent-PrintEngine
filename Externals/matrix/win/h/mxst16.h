/*****************************************************************************/
/*  MXST16.H   MS-WINDOWS Win16 (3.x)                                        */
/*                                                                           */
/*  (C) TDi GmbH                                                             */
/*                                                                           */
/*  Include File for Visual C/C++ (16-Bit LIB)                               */
/*****************************************************************************/
#ifndef _MATRIXAPI_H
#define _MATRIXAPI_H

#define IW_DRIVER    1
#define IW_NODRIVER  2

typedef struct
{
  short LPT_Nr;
  short LPT_Adr;
  short DNG_Cnt;
} DNGINFO;

short Init_MatrixAPI(void);
short Release_MatrixAPI(void);
long  GetVersionAPI(void);
long  GetVersionAPI32(void);
long  GetVersionDRV(void);
long  GetVersionDRV_USB(void);
short GetPortAdr(short);
short Dongle_WriteData(long, long*, short, short, short);
short Dongle_WriteDataEx(long, long*, short, short, short, short);
short Dongle_ReadData(long, long*, short, short, short);
short Dongle_ReadDataEx(long, long*, short, short, short, short);
long  Dongle_ReadSerNr(long, short, short);
short Dongle_Exit(void);
short Dongle_Find(void);
short Dongle_FindEx(DNGINFO *);
long  Dongle_Version(short, short);
long  Dongle_Model(short, short);
short Dongle_MemSize(short, short);
void  SetW95Access(short);
short Dongle_Count(short);
short PausePrinterActivity(void);
short ResumePrinterActivity(void);
short SetConfig_MatrixNet(short, char*);
short LogIn_MatrixNet(long, short, short);
short LogOut_MatrixNet(long, short, short);
long  GetConfig_MatrixNet(short Category);

short Dongle_WriteKey(long UserCode, long *KeyData, 
                                     short DongleNr, 
                                     short PortNr);
short Dongle_GetKeyFlag(long UserCode, short DongleNr, short PortNr);

short Dongle_EncryptData(long UserCode, long *DataBlock, short DongleNr, short PortNr);
short Dongle_DecryptData(long UserCode, long *DataBlock, short DongleNr, short PortNr);

short Dongle_SetDriverFlag(long UserCode, short DriverFlag, short DongleNr, short PortNr);
short Dongle_GetDriverFlag(long UserCode, short DongleNr, short PortNr);

#endif  // _MATRIXAPI_H

