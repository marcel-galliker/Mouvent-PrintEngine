#ifndef SPI_H_
#define SPI_H_

#define SPI_READ		0
#define SPI_WRITE		1

void		spi0_init(void);
uint32_t	_ps_spi_transfer(uint8_t dir, uint8_t appCmd, uint8_t size, uint32_t val);

#endif /* SPI_H_ */