/*****************************************************************************/
/* MXTEA.H  -  XTEA Encryption/Decryption Algorithm                          */
/*                                                                           */
/* (C) TDi GmbH                                                              */
/*                                                                           */
/* Include File for C/C++                                                    */
/*****************************************************************************/
/* The 'Extended Tiny Encryption Algorithm' (XTEA) by David Wheeler and      */
/* Roger Needham of the Cambridge Computer Laboratory.                       */
/* XTEA is a Feistel cipher with XOR and AND addition as the non-linear      */
/* mixing functions.                                                         */
/* Takes 64 bits (8 Bytes block) of data in Data[0] and Data[1].             */
/* Returns 64 bits of encrypted data in Data[0] and Data[1].                 */
/* Takes 128 bits of key in Key[0] - Key[3].                                 */
/*****************************************************************************/

#include "mxtypes.h"

_mxINT16 MxApp_Encrypt(_mxUINT32 *DataBlock, _mxUINT32 *Key);
_mxINT16 MxApp_Decrypt(_mxUINT32 *DataBlock, _mxUINT32 *Key);

//------------------------------------------------------------------------------
 _mxINT16 MxApp_Encrypt(_mxUINT32 *Data, _mxUINT32 *Key)
//------------------------------------------------------------------------------
{
   register _mxUINT32 delta, sum;
   _mxINT16 cnt;
   sum = 0;
   delta = 0x9E3779B9;
   cnt = 32;
   while(cnt-- > 0)
   {
      Data[0] += (Data[1]<<4 ^ Data[1]>>5) + Data[1] ^ sum + Key[sum&3];
      sum += delta;
      Data[1] += (Data[0]<<4 ^ Data[0]>>5) + Data[0] ^ sum + Key[sum>>11 & 3];
   }
   return 0;
}

//-----------------------------------------------------------------------------
 _mxINT16 MxApp_Decrypt(_mxUINT32 *Data, _mxUINT32 *Key)
//-----------------------------------------------------------------------------
{
   register _mxUINT32 delta, sum;
   _mxINT16 cnt;
   sum   = 0xC6EF3720;
   delta = 0x9E3779B9;
   cnt   = 32;
   while(cnt-- > 0)
   {
      Data[1] -= ((Data[0]<<4 ^ Data[0]>>5) + Data[0]) ^ (sum + Key[sum>>11 & 3]);
      sum -= delta;
      Data[0] -= ((Data[1]<<4 ^ Data[1]>>5) + Data[1]) ^ (sum + Key[sum&3]);
   }
   return 0;
}

