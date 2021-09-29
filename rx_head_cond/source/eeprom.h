#ifndef EEPROM_H
#define EEPROM_H

#include "rx_ink_common.h"

// EEPROM Settings API
#define EE_ADDR_PUMPTIME            0x00
#define EE_ADDR_04          		0x04
#define EE_ADDR_PIN_FACTORY_OFFSET  0x08
#define EE_ADDR_PIN_USER_OFFSET     0x0c
#define EE_ADDR_POUT_FACTORY_OFFSET 0x10
#define EE_ADDR_POUT_USER_OFFSET    0x14
#define EE_ADDR_18          		0x18
#define EE_ADDR_1c          		0x1c
#define EE_ADDR_CLUSTER_NO          0x20
#define EE_ADDR_CLUSTER_TIME        0x24
#define __EE_ADDR_MACHINE_METERS	0x28
#define EE_ADDR_PIN2_FACTORY_OFFSET 0x32

// function prototypes
int eeprom_write_byte(const UINT32 pagenum, const BYTE byteaddress, const BYTE data);
int eeprom_write_long(const UINT32 pagenum, const BYTE byteaddress, const UINT32 data);
int eeprom_read_byte (const UINT32 pagenum, const BYTE byteaddress, BYTE *data);

#if DEBUG                   
int eeprom_fill_page_dummy(const UINT32 pagenum);
int eeprom_fill_page (const UINT32 pagenum, const BYTE *page_data_256bytes);
int eeprom_read_page (const UINT32 pagenum, BYTE *page_data_256bytes);
int eeprom_clear_page(const UINT32 pagenum);
int eeprom_print_page(const UINT32 pagenum);
#endif
      
void eeprom_init(void);
int  eeprom_write_setting32(const BYTE address, const UINT32 value);
int  eeprom_read_setting32 (const BYTE address, INT32 *value);
int  eeprom_write_setting16(const BYTE address, const INT16 value);
int  eeprom_read_setting16 (const BYTE address, INT32 *value);
                     
void eeprom_delay(void);
//void eeprom_ack_polling(void);
                       
#endif // EEPROM_H
