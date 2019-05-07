#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "crc_calc.h"

#define                 P_CCITT     0x1021


static int              crc_tabccitt_init = 0x0;
static unsigned short   crc_tabccitt[256];

static unsigned short 	_update_crc_ccitt(unsigned short crc, char c);
static void 			_init_crcccitt_tab( void );
// copied from http://www.lammertbies.nl/download/lib_crc.zip

unsigned short crc_calc(char * input_string, int size)
{
	//char input_string[5]={0x10,0xE2,0x03,0x20,0xF1};//working
    unsigned short crc_ccitt_0000; //16bit => 2 Byte !
    int i;

    if ( !crc_tabccitt_init ) _init_crcccitt_tab();

    crc_ccitt_0000 = 0;
    for (i=0; i<size; i++, input_string++)
    {
    	crc_ccitt_0000 = _update_crc_ccitt(crc_ccitt_0000, *input_string);
    };
    return crc_ccitt_0000;
}

/*******************************************************************\
*                                                                   *
*   unsigned short update_crc_ccitt( unsigned long crc, char c );   *
*                                                                   *
*   The function update_crc_ccitt calculates  a  new  CRC-CCITT     *
*   value  based  on the previous value of the CRC and the next     *
*   byte of the data to be checked.                                 *
*                                                                   *
\*******************************************************************/

static unsigned short _update_crc_ccitt( unsigned short crc, char c )
{
    unsigned short tmp, short_c;

    short_c  = 0x00ff & (unsigned short) c;

    tmp = (crc >> 8) ^ short_c;
    crc = (crc << 8) ^ crc_tabccitt[tmp];

    return crc;

}  /* update_crc_ccitt */

/*******************************************************************\
*                                                                   *
*   static void init_crcccitt_tab( void );                          *
*                                                                   *
*   The function init_crcccitt_tab() is used to fill the  array     *
*   for calculation of the CRC-CCITT with values.                   *
*                                                                   *
\*******************************************************************/

static void _init_crcccitt_tab( void )
{
	int i, j;
	unsigned short crc, c;

	for (i=0; i<256; i++)
	{
		crc = 0;
		c   = ((unsigned short) i) << 8;

		for (j=0; j<8; j++)
		{
			if ( (crc ^ c) & 0x8000 )
			{
				crc = ( crc << 1 ) ^ P_CCITT;
			}
			else
			{
				crc =   crc << 1;
			}
			c = c << 1;
		}
		crc_tabccitt[i] = crc;
	}
	crc_tabccitt_init = 1;

}  /* init_crcccitt_tab */
