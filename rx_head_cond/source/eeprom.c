/**************************************************************************/ /**
 \file eeprom.c

 EEPROM used with I2C BitBanging Library

 \author 	Cyril Andreatta

 \copyright 2017 by radex AG, Switzerland. All rights reserved.
 ******************************************************************************/

#include "eeprom.h"
#include "cond_def_head.h"
#include "debug_printf.h"
#include "i2c_bitbang.h"
#include "pres.h"

#define MAX_NUM_PAGE 1023u
#define MAX_PAGE_SIZE 255u

// return values
#define EEPROM_SUCCESS 0

/// Location to store settings in EEPROM
const UINT32 EE_SETTINGS_PAGE = 0;
const UINT32 EE_EMPTY_LONG    = 0xFFFFFFFF;

const BYTE EE_ADDR_PUMPTIME            = 0x00;
//const BYTE EE_ADDR_PIN_0_POUT          = 0x04; // -> unused
const BYTE EE_ADDR_PIN_FACTORY_OFFSET  = 0x08;
const BYTE EE_ADDR_PIN_USER_OFFSET     = 0x0c;
const BYTE EE_ADDR_POUT_FACTORY_OFFSET = 0x10;
const BYTE EE_ADDR_POUT_USER_OFFSET    = 0x14;

/**
 * \brief Write one byte in specified page at specified address
 *
 * This takes around 120us with FAST SPEED I2C (400kHz)
 *
 * \param pagenum Number of page [0..255] to write to
 * \param byteaddress address to write to
 * \param data that should be written
 *
 * \return 0 on success
 **/
int eeprom_write_byte(const UINT32 pagenum, const BYTE byteaddress,
                      const BYTE data)
{
    ASSERT(pagenum <= MAX_NUM_PAGE);

    // calculate EEPROM address
    UINT32u addr = {0};
    addr.value = pagenum * (MAX_PAGE_SIZE + 1);
    addr.value += byteaddress;

    i2c_bb_write_address(addr, data);

    return EEPROM_SUCCESS;
}

/**
 * \brief Write four bytes in specified page at specified address
 *
 * \param pagenum Number of page [0..255] to write to
 * \param byteaddress starting address to write to
 * \param data that should be written
 *
 * \return 0 on success
 **/
int eeprom_write_long(const UINT32 pagenum, const BYTE byteaddress,
                      const UINT32 data)
{
    ASSERT(pagenum <= MAX_NUM_PAGE);

    // calculate EEPROM address
    UINT32u addr = {0};
    addr.value = pagenum * (MAX_PAGE_SIZE + 1);
    addr.value += byteaddress;

    BYTE *dp = (BYTE *)&data;
    i2c_bb_write_page(addr, dp, 4);

    return EEPROM_SUCCESS;
}

/**
 * \brief Read one byte from specified page at specified address
 *
 * This takes around 150us with FAST SPEED I2C (400kHz)
 *
 * \param pagenum Number of page [0..255] to read from
 * \param byteaddress starting address to read from
 * \param data Pointer to store data read from given address
 *
 * \return 0 on success
 **/
int eeprom_read_byte(const UINT32 pagenum, const BYTE byteaddress, BYTE *data)
{
    ASSERT(pagenum <= MAX_NUM_PAGE);

    // calculate EEPROM address
    UINT32u addr = {0};
    addr.value = pagenum * (MAX_PAGE_SIZE + 1);
    addr.value += byteaddress;

    *data = i2c_bb_read_random_address(addr);

    return EEPROM_SUCCESS;
}

/**
 * \brief Read one byte from specified page at specified address
 *
 * This takes around 150us with FAST SPEED I2C (400kHz)
 *
 * \param pagenum Number of page [0..255] to read from
 * \param byteaddress starting address to read from
 * \param data Pointer to store data read from given address
 *
 * \return 0 on success
 **/
int eeprom_read_long(const UINT32 pagenum, const BYTE byteaddress, UINT32u *data)
{
    ASSERT(pagenum <= MAX_NUM_PAGE);

    // calculate EEPROM address
    UINT32u addr = {0};
    addr.value = pagenum * (MAX_PAGE_SIZE + 1);
    addr.value += byteaddress;

    // Assume a UINT32 has always 4 bytes, which is true for this platform!
    i2c_bb_read_sequential_address(addr, &(data->byte[0]), 4);

    return EEPROM_SUCCESS;
}

/**
 * \brief Fill whole page with dummy data (counting up, same as address)
 *
 * \param pagenum Number of page [0..255] to fill
 *
 * \return 0 on success
 **/
#if DEBUG
int eeprom_fill_page_dummy(const UINT32 pagenum)
{
    ASSERT(pagenum <= MAX_NUM_PAGE);

    BYTE wdatas[MAX_PAGE_SIZE + 1] = {0};

    // calculate EEPROM address
    UINT32u addr = {0};
    addr.value = pagenum * (MAX_PAGE_SIZE + 1);

    for (UINT32 i = 0; i <= MAX_PAGE_SIZE; ++i)
        wdatas[i] = i;

    i2c_bb_write_page(addr, wdatas, MAX_PAGE_SIZE);
    return EEPROM_SUCCESS;
}

/**
 * \brief Fill whole page with given data
 *
 * \param pagenum Number of page [0..255] to fill
 * \param page_data_256bytes Pointer to a 256 byte array, to be stored in EEPROM
 *
 * \return 0 on success
 **/
int eeprom_fill_page(const UINT32 pagenum, const BYTE *page_data_256bytes)
{
    ASSERT(pagenum <= MAX_NUM_PAGE);

    // calculate EEPROM address
    UINT32u addr = {0};
    addr.value = pagenum * (MAX_PAGE_SIZE + 1);

    i2c_bb_write_page(addr, page_data_256bytes, MAX_PAGE_SIZE);
    return EEPROM_SUCCESS;
}

/**
 * \brief Read whole page
 *
 * \param pagenum Number of page [0..255] to read
 * \param page_data_256bytes Pointer to a 256 byte array, save retrieved data
 *from EEPROM
 *
 * \return 0 on success
 **/
int eeprom_read_page(const UINT32 pagenum, BYTE *page_data_256bytes)
{
    ASSERT(pagenum <= MAX_NUM_PAGE);

    // calculate EEPROM address
    UINT32u addr = {0};
    addr.value = pagenum * (MAX_PAGE_SIZE + 1);

    i2c_bb_read_sequential_address(addr, page_data_256bytes, MAX_PAGE_SIZE);

    return EEPROM_SUCCESS;
}

/**
 * \brief Clear whole page (fill with 0xFF)
 *
 * \param pagenum Number of page [0..255] to clear
 *
 * \return 0 on success
 **/
int eeprom_clear_page(const UINT32 pagenum)
{
    ASSERT(pagenum <= MAX_NUM_PAGE);

    BYTE wdatas[MAX_PAGE_SIZE + 1];

    for (UINT32 i = 0; i <= MAX_PAGE_SIZE; ++i)
        wdatas[i] = 0xff;

    eeprom_fill_page(pagenum, wdatas);

    return EEPROM_SUCCESS;
}

/**
 * \brief Printout data from a page (only for DEBUG)
 *
 * \param pagenum Number of page [0..255] to print
 *
 * \return 0 on success
 **/
int eeprom_print_page(const UINT32 pagenum)
{
    ASSERT(pagenum <= MAX_NUM_PAGE);

    BYTE rdatas[256] = {0};
    eeprom_read_page(pagenum, rdatas);

    // header
    for (int i = 0; i < 8; ++i)
        DBG_PRINTF("0x%02d\t", i);
    DBG_PRINTF("\n");
    for (int i = 0; i < 8; ++i)
        DBG_PRINTF("--------");

    // EEPROM data
    for (UINT32 i = 0; i <= 0xff; ++i)
    {
        if (i % 8 == 0) DBG_PRINTF("\n");

        DBG_PRINTF("0x%02X\t", rdatas[i]);
    }
    DBG_PRINTF("\n");

    return EEPROM_SUCCESS;
}
#endif

void eeprom_write_setting32(const BYTE address, const UINT32 value)
{
    eeprom_write_long(EE_SETTINGS_PAGE, address, value);
    eeprom_delay();
    //eeprom_ack_polling();
}

UINT32 eeprom_read_setting32(const BYTE address)
{
    UINT32u ret = {0};
    eeprom_read_long(EE_SETTINGS_PAGE, address, &ret);

    if (ret.value != EE_EMPTY_LONG)
        return ret.value;
    else
        return 0;
}

void eeprom_write_setting16(const BYTE address, const UINT16 value)
{
    UINT32 data = value;
    data |= (~value) << 16;
    
    eeprom_write_setting32(address, data);
}

int eeprom_read_setting16(const BYTE address, UINT16 *value)
{
    #define MASK_16 0xffff    
    UINT32 data = eeprom_read_setting32(address);
    
    if ((data & MASK_16) == (~(data >> 16) & MASK_16))
    {
        *value = data & MASK_16;
        return (0);
    }
    else
    {
        *value = 0;
        return (-1);
    }
}

/*
void eeprom_ack_polling(void)
{
    #define TIMEOUT_10MS 400000
    static UINT32 timeout = 0;
    
    while (1)
    {
        if (0 == i2c_bb_read_ack())
            break;
            
        if (++timeout < TIMEOUT_10MS)
            break;
    }
    
    timeout = 0;
    return;
}
*/

void eeprom_delay(void)
{
    for (volatile unsigned int x = 0; x < 30000; x++)
        ;
}
