#include <atmel_start.h>
#include "component\pio.h"
#include "spi.h"

static struct io_descriptor *SPI_0_io;

//--- _ps_spi_transfer ----------------------------------------

void spi0_init(void)
{
	spi_m_sync_get_io_descriptor(&SPI_0, &SPI_0_io);
	spi_m_sync_enable(&SPI_0);
}
uint32_t _ps_spi_transfer(uint8_t dir, uint8_t appCmd, uint8_t size, uint32_t val)
{
	uint8_t count;
	uint8_t cs_delay;

	union
	{
		uint32_t	val;
		uint8_t		valArr[4];
	}data;

	data.val = val;
	
	gpio_set_pin_level(SM_CS,false);
	io_write(SPI_0_io,&appCmd,1);
	gpio_set_pin_level(SM_CS,true);

	if(dir==SPI_WRITE)
	{
		for(count = 0; count < size ; count++)
		{
			for(cs_delay = 0; cs_delay < 0x1f ; cs_delay++)
			;
			gpio_set_pin_level(SM_CS,false);
			io_write(SPI_0_io, &data.valArr[size-count-1],1);
			gpio_set_pin_level(SM_CS,true);
		}
	}
	else
	{
		for(count = 0; count < size ; count++)
		{
			for(cs_delay = 0; cs_delay < 0x1f ; cs_delay++)
			;
			gpio_set_pin_level(SM_CS,false);
			io_read(SPI_0_io, &data.valArr[size-count-1],1);
			gpio_set_pin_level(SM_CS,true);
		}
	}
	return data.val;
}
