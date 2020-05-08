/*
 * crc_calc.h
 *
 *  Created on: 25.02.2016
 *      Author: stefan
 */

#ifndef CRC_CALC_H_
#define CRC_CALC_H_

#define                 P_CCITT     0x1021

static int              crc_tabccitt_init = 0x0;

static unsigned short   crc_tabccitt[256];
static void             init_crcccitt_tab(void);
unsigned short update_crc_ccitt(unsigned short crc, char c);
unsigned short crc_calc(char * input_string, int size);

#endif /* CRC_CALC_H_ */
