#ifndef EEPROM_H
#define EEPROM_H

#include "rx_ink_common.h"

// EEPROM Settings API
extern const BYTE EE_ADDR_PUMPTIME;
//extern const BYTE EE_ADDR_PIN_0_POUT; // -> unused
extern const BYTE EE_ADDR_PIN_FACTORY_OFFSET;
extern const BYTE EE_ADDR_PIN_USER_OFFSET;
extern const BYTE EE_ADDR_POUT_FACTORY_OFFSET;
extern const BYTE EE_ADDR_POUT_USER_OFFSET;        

extern const UINT32 EE_SETTINGS_PAGE;
extern const UINT32 EE_EMPTY_LONG;

// function prototypes
int eeprom_write_byte(const UINT32 pagenum, 
                      const BYTE byteaddress, const BYTE data);
int eeprom_write_long(const UINT32  pagenum,
                      const BYTE byteaddress, const UINT32 data);
int eeprom_read_byte(const UINT32 pagenum, 
                     const BYTE byteaddress,
                     BYTE *data);

#if DEBUG                   
int eeprom_fill_page_dummy(const UINT32 pagenum);
int eeprom_fill_page(const UINT32 pagenum, const BYTE *page_data_256bytes);
int eeprom_read_page(const UINT32 pagenum, BYTE *page_data_256bytes);
int eeprom_clear_page(const UINT32 pagenum);
int eeprom_print_page(const UINT32 pagenum);
#endif
                     
void eeprom_write_setting32(const BYTE address, const UINT32 value);
UINT32 eeprom_read_setting32(const BYTE address);
void eeprom_write_setting16(const BYTE address, const UINT16 value);
int eeprom_read_setting16(const BYTE address, UINT16 *value);
                     
void eeprom_delay(void);
//void eeprom_ack_polling(void);
                       
#endif // EEPROM_H
