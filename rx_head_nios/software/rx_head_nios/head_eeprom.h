/*
 * head_eeprom.h
 *
 *  Created on: 29.09.2016
 *      Author: stefan
 */

#ifndef HEAD_EEPROM_H_
#define HEAD_EEPROM_H_

int read_eeprom(alt_u8 * eeprom_data, char chip_adr, char eeprom_adr);
int head_eeprom_read(void);



#endif /* HEAD_EEPROM_H_ */
