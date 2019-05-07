/*
 * head_eeprom.h
 *
 *  Created on: 29.09.2016
 *      Author: stefan
 */

#ifndef HEAD_EEPROM_H_
#define HEAD_EEPROM_H_

int head_eeprom_read(void);
// int head_eeprom_read_seq(void);
int head_eeprom_read_user_data(alt_u32 head, alt_u8 * eeprom_data, alt_u32 number_of_byte_to_read, alt_u32 addr);
// int head_eeprom_write_user_data(alt_u32 head, alt_u8 * eeprom_data, alt_u32 number_of_byte_to_write, alt_u32 addr);
int head_eeprom_write_user_data(alt_u32 head, alt_u8 * eeprom_data, alt_u32 number_of_byte_to_write, alt_u32 addr);
int head_eeprom_change_user_data(alt_u32 head, alt_u8 *act_data, alt_u8 * new_data, alt_u32 number_of_byte_to_write, alt_u32 addr);


#endif /* HEAD_EEPROM_H_ */
