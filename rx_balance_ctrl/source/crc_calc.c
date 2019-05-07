/******************************************************************************/
/** \file crc_calc.c
 ** 
 ** crc calc Module
 **
 **	Copyright 2016 by radex AG, Switzerland. All rights reserved.
 **	Written by Stefan Weber
 **
 ******************************************************************************/

/******************************************************************************/
/* Include files                                                              */
/******************************************************************************/
#include "mcu.h"
#include <string.h>
#include "IOMux.h" 
#include <stdio.h>

//defines --------------------------------------

// Global variabels

/*****************************************************************************/
/* Function prototype                                                        */
/*****************************************************************************/

//CRC
#define POLYNOMIAL 0x131 //P(x)=x^8+x^5+x^4+1 = 100110001
//============================================================

//============================================================
//calculates checksum for n bytes of data
//and compares it with expected checksum
//input: data[] checksum is built based on this data
// nbrOfBytes checksum is built for n bytes of data
// checksum expected checksum
//return: error: CHECKSUM_ERROR = checksum does not match
// 0 = checksum matches
//============================================================
uint8_t MakeCRC (uint8_t data[], uint8_t nbrOfBytes, uint8_t checksum)
{
	uint8_t crc = 0;
	uint8_t byteCtr;
	//uint8_t data_temp=0;
	//calculates 8-Bit checksum with given polynomial
	for (byteCtr = 1; byteCtr <= nbrOfBytes; byteCtr++)
	{ 
		//data_temp=data[nbrOfBytes-byteCtr];
		crc ^= (data[nbrOfBytes-byteCtr]);
		for (uint8_t bit = 8; bit > 0; --bit)
		{ 
			if (crc & 0x80) crc = (crc << 1) ^ POLYNOMIAL;
			else crc = (crc << 1);
		}
	}
	//data_temp=data[byteCtr];
	if(crc!=checksum) return 1;
	else return 0;
}
