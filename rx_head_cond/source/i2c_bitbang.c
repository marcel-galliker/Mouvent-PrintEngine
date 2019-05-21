/**************************************************************************/ /**
 \file i2c_bitbang.c

 I2C BitBanging Library

 \author    Cyril Andreatta

 \copyright 2017 by radex AG, Switzerland. All rights reserved.
 ******************************************************************************/

#include "i2c_bitbang.h"
#include "gpio/gpio.h"

#define STANDARD 26u // ~5us ~= 100kHz
#define FAST 1u      // ~1.25us ~= 400kHz

const unsigned int DELAY = FAST;

#define EEPROM_ADR (0xA8) // Last 3 bits: A17 A16 R/W
#define WRITE 0
#define READ 1
#define ACK 0
#define NACK 1

// clang-format off
#define SDA(bit_value)                       \
do {                                         \
    bFM4_GPIO_PDOR5_P5 = (bit_value & 0x01); \
} while (0)

#define SCL(bit_value)                       \
do {                                         \
    bFM4_GPIO_PDOR6_P6 = (bit_value & 0x01); \
} while (0)
// clang-format on

/*******************************************************************************
 * PRIVATE FUNCTIONS
 ******************************************************************************/
static void i2c_bb_start(void);
static void i2c_bb_stop(void);
static int i2c_bb_write_bit(const BYTE bit);
static BYTE i2c_bb_read_bit(void);
static BYTE i2c_bb_read_byte(const BYTE ack);
static BYTE i2c_bb_write_byte(BYTE byte);
static void delay(void);

/**
 * \brief Initialize I2C, define both SCL and SDA as OUTPUT and set level HIGH.
 *
 * \return 0 on success
 **/
int i2c_bb_init(void)
{
    // SDA
    bFM4_GPIO_PDOR5_P5 = 1; // set high
    bFM4_GPIO_DDR5_P5 = 1;  // output
    bFM4_GPIO_PFR5_P5 = 0;  // gpio
    bFM4_GPIO_PZR5_P5 = 1;  // open-drain

    // SCL
    bFM4_GPIO_PDOR6_P6 = 1; // set high
    bFM4_GPIO_DDR6_P6 = 1;  // outout
    bFM4_GPIO_PFR6_P6 = 0;  // gpio
    bFM4_GPIO_PZR6_P6 = 1;  // open-drain
    // disable use of ADC is necessary for this pin
    bFM4_GPIO_ADE_AN09 = 0; // no ADC

    return 1;
}

/**
 * \brief Read single byte from current (last accessed) address
 *
 * \return byte read from given address
 **/
BYTE i2c_bb_read_current_address(const BYTE ack)
{
    BYTE ret = 0;

    i2c_bb_start();
    // A17/A16 are don't care
    i2c_bb_write_byte(EEPROM_ADR | READ);
    ret = i2c_bb_read_byte(ack);

    i2c_bb_stop();

    return ret;
}

/**
 * \brief Read single byte from random address
 *
 * \return byte read from given address
 **/
BYTE i2c_bb_read_random_address(const UINT32u address)
{
    BYTE ret = 0;
    BYTE read_addr = EEPROM_ADR | WRITE;
    read_addr |= (address.byte[2] & 0x03) << 1;

    i2c_bb_start();
    i2c_bb_write_byte(read_addr);
    i2c_bb_write_byte(address.byte[1]);
    i2c_bb_write_byte(address.byte[0]);

    i2c_bb_start();
    // Address bits A17/A16 don't care
    i2c_bb_write_byte(EEPROM_ADR | READ);
    ret = i2c_bb_read_byte(NACK);
    i2c_bb_stop();

    return ret;
}

/**
 * \brief Read multiple bytes starting from given random address
 *
 * \return 0 on success
 **/
BYTE i2c_bb_read_sequential_address(const UINT32u address, BYTE *ret_values, BYTE len)
{
    BYTE read_addr = EEPROM_ADR | WRITE;
    read_addr |= (address.byte[2] & 0x03) << 1;

    // start with a normal random read and store to answer to first byte
    *(ret_values++) = i2c_bb_read_random_address(address);
    --len;
    
    // initialize read sequence
    i2c_bb_start();
    i2c_bb_write_byte(EEPROM_ADR | READ);

    // read multiple bytes
    while (--len)
        *(ret_values++) = i2c_bb_read_byte(ACK);

    // Respond with NACK to end read cycle
    *ret_values = i2c_bb_read_byte(NACK);

    i2c_bb_stop();

    return 0;
}

/**
 * \brief Write single byte to random address
 *
 * \return 0 on success
 **/
BYTE i2c_bb_write_address(const UINT32u address, const BYTE data)
{
    BYTE write_addr = EEPROM_ADR | WRITE;
    write_addr |= (address.byte[2] & 0x03) << 1;

    i2c_bb_start();
    i2c_bb_write_byte(write_addr);
    i2c_bb_write_byte(address.byte[1]);
    i2c_bb_write_byte(address.byte[0]);
    i2c_bb_write_byte(data);
    i2c_bb_stop();

    return 0;
}

/**
 * \brief Write multiple data bytes to random address
 *
 * \return 0 on success
 **/
BYTE i2c_bb_write_page(const UINT32u address, const BYTE *data, BYTE len)
{
    BYTE write_addr = EEPROM_ADR | WRITE;
    write_addr |= (address.byte[2] & 0x03) << 1;

    i2c_bb_start();
    i2c_bb_write_byte(write_addr);
    i2c_bb_write_byte(address.byte[1]);
    i2c_bb_write_byte(address.byte[0]);

    /// \todo check for page roll-over
    while (len--)
        i2c_bb_write_byte(*(data++));

    i2c_bb_stop();

    return 0;
}

BOOL i2c_bb_read_ack(void)
{
    BYTE ack = 0;

    i2c_bb_start();
    // A17/A16 are don't care
    i2c_bb_write_byte(EEPROM_ADR | READ);
    ack = i2c_bb_read_byte(NACK);
    
    return ack;
}

/*******************************************************************************
 * PRIVATE FUNCTIONS
 ******************************************************************************/

/**
 * \brief Send one bit
 *
 * \return 0 on success
 **/
static int i2c_bb_write_bit(const BYTE bit)
{
    SDA(bit & 0x01);
    delay();

    // clock data out
    SCL(1);
    delay();
    SCL(0);

    return 1;
}

/**
 * \brief Iitialize I2C, set both SCL and SDA as OUT and HIGH.
 *
 * \return bit read from bus
 **/
static BYTE i2c_bb_read_bit(void)
{
    BYTE readbit = 0;

    SDA(1);
    delay();

    SCL(1);
    delay();

    // SDA must be set as INPUT
    readbit = bFM4_GPIO_PDIR5_P5;

    SCL(0);

    return readbit;
}

/**
 * \brief Iitialize I2C, set both SCL and SDA as OUT and HIGH.
 *
 * \return ACK from slave
 **/
static BYTE i2c_bb_write_byte(BYTE byte)
{
    BYTE ack = NACK;
    unsigned int timeout = 1000;

    for (int i = 0; i < 8; ++i)
    {
        // take first bit (MSB)
        i2c_bb_write_bit((byte & 0x80) != 0);
        byte <<= 1;
    }

    // get ACK
    bFM4_GPIO_DDR5_P5 = 0; // input

    do
        ack = i2c_bb_read_bit();
    while (NACK == ack && --timeout);

    bFM4_GPIO_DDR5_P5 = 1;  // output
    bFM4_GPIO_PDOR5_P5 = 1; // set high

    return ack;
}

/**
 * \brief Iitialize I2C, set both SCL and SDA as OUT and HIGH.
 *
 * \return byte read from bus
 **/
static BYTE i2c_bb_read_byte(const BYTE ack)
{
    BYTE ret = 0;

    // set SDA as input
    bFM4_GPIO_DDR5_P5 = 0; // input

    for (int i = 0; i < 8; ++i)
    {
        ret <<= 1;
        ret |= i2c_bb_read_bit();
    }

    // restore SDA to output
    bFM4_GPIO_DDR5_P5 = 1;  // output
    bFM4_GPIO_PDOR5_P5 = 1; // set high

    i2c_bb_write_bit(ack);

    return ret;
}

/**
 * \brief Reset I2C bus
 **/
void i2c_bb_reset(void)
{
    i2c_bb_start();
    i2c_bb_write_byte(0xff);
    i2c_bb_start();
    i2c_bb_stop();
}

/**
 * \brief Send start bit sequence for initializing new communication
 **/
static void i2c_bb_start(void)
{
    SDA(1);
    delay();

    SCL(1);
    delay();

    SDA(0);
    delay();

    SCL(0);
    delay();
}

/**
 * \brief Send stop bit sequence to terminate communication
 **/
static void i2c_bb_stop(void)
{
    SDA(0);
    delay();

    SCL(1);
    delay();

    SDA(1);
    delay();

    SCL(0);
    delay();

    SCL(1);
}
    
/**
 * \brief Crude delay function for I2C clock.
 **/
static void delay(void)
{
    for (unsigned int i = 0; i < DELAY; ++i)
        ;
}
