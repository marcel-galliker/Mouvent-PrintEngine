// ****************************************************************************
//
//	DIGITAL PRINTING - crc_fuji.c
//
//	Communication between ARM processor and FPGA
//
// ****************************************************************************
//
//	Copyright 2021 by radex AG, Switzerland. All rights reserved.
//	Written by Marcel Galliker
//
// ****************************************************************************

#include "rx_common.h"

// according to fuji the EEProm is using the CRC-8/Maxim algorithm
// source:  https://stackoverflow.com/questions/29214301/ios-how-to-calculate-crc-8-dallas-maxim-of-nsdata

static UCHAR crc_bits(UCHAR data)
{
    unsigned char crc = 0;
    if(data & 1)     crc ^= 0x5e;
    if(data & 2)     crc ^= 0xbc;
    if(data & 4)     crc ^= 0x61;
    if(data & 8)     crc ^= 0xc2;
    if(data & 0x10)  crc ^= 0x9d;
    if(data & 0x20)  crc ^= 0x23;
    if(data & 0x40)  crc ^= 0x46;
    if(data & 0x80)  crc ^= 0x8c;
    return crc;
}

//--- fuji_crc -------------------------------------------------------
UCHAR fuji_crc(const UCHAR * data, int size)
{
    unsigned char crc = 0;
    for ( unsigned int i = 0; i < size; ++i )
    {
         crc = crc_bits(data[i] ^ crc);
    }
    return crc;
}