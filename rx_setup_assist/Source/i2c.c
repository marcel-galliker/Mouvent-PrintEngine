#include <atmel_start.h>
#include "component\pio.h"

static struct io_descriptor *I2C_0_io;

void i2c0_init(void)
{
	i2c_m_sync_get_io_descriptor(&I2C_0, &I2C_0_io);
	i2c_m_sync_enable(&I2C_0);
	i2c_m_sync_set_slaveaddr(&I2C_0, 0x54, I2C_M_SEVEN);
}

void I2C_0_eeprom_w(void)
{
//	struct io_descriptor *I2C_0_io;
	uint8_t writebuf[32];
	uint8_t *buf;
	
	buf = writebuf;

	writebuf[0] = 0;
	writebuf[1] = 0;
	writebuf[2] = 0x55;
	writebuf[3] = 0xAA;
	writebuf[4] = 0xA5;
	
//	i2c_m_sync_get_io_descriptor(&I2C_0, &I2C_0_io);
//	i2c_m_sync_enable(&I2C_0);
//	i2c_m_sync_set_slaveaddr(&I2C_0, 0x54, I2C_M_SEVEN);
	io_write(I2C_0_io, buf, 5);
}

void I2C_0_eeprom_r(void)
{
	uint8_t writebuf[32];
	uint8_t *buf;
	
	buf = writebuf;

	writebuf[0] = 0;
	writebuf[1] = 0;

	io_write(I2C_0_io, buf, 2);

	io_read(I2C_0_io, buf, 3);
}
