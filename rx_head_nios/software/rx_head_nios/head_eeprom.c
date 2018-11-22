/*
 * head_eeprom.c
 *
 *  Created on: 29.09.2016
 *      Author: stefan
 */
#include "i2c_master.h"
#include "nios_def_head.h"
#include "system.h"
#include "head_eeprom.h"


#define LAST_BYTE 1
#define READ 1
#define WRITE 0

int head_eeprom_read(void)
{
	alt_u32 	head=0;
	alt_u32		count_temp=0;
	alt_u32		head_eeprom_ok=0;
	// Read Data from Head eeprom
	//0x50, 0x52, 0x54, 0x56 for head 0-4

	for(head=0;head<MAX_HEADS_BOARD;head++)
	{
		for(count_temp=0;count_temp<EEPROM_DATA_SIZE; count_temp++)
		{
			if(read_eeprom(&pRX_Status->head_eeprom[head][count_temp], 0x50+(head*2), count_temp)==0)
			{
				head_eeprom_ok+=1;
			}
			else
			{
				return(-1);
			}
		}
	}
	if(head_eeprom_ok==MAX_HEADS_BOARD*EEPROM_DATA_SIZE)
	{
		return (0);
	}

	return(-1);
}

int read_eeprom(alt_u8 * eeprom_data, char chip_adr, char eeprom_adr)
{
	int head_data=0;
	if (I2C_start(I2C_MASTER_0_BASE,(chip_adr),WRITE)==I2C_ACK)					// set chip address and set to write(0)
	{
		if(I2C_write(I2C_MASTER_0_BASE,0x00,!LAST_BYTE)==I2C_ACK)				// set address1 to 0x00
		{
			if(I2C_write(I2C_MASTER_0_BASE,eeprom_adr,LAST_BYTE)==I2C_ACK)		// set address2 to 0x00
			{
				if(I2C_start(I2C_MASTER_0_BASE,(chip_adr),READ)==I2C_ACK)		// set chip address in read mode)
				{
					head_data=  I2C_read(I2C_MASTER_0_BASE,LAST_BYTE);  		// read
					//alt_printf("0x%x: 0x%x %c\n", eeprom_adr, head_data, head_data);
					*eeprom_data=head_data;
					return (0);
				}
			}
		}
	}
	return (-1);
}
