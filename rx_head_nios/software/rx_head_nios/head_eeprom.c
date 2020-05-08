/*
 * head_eeprom.c
 *
 *  Created on: 29.09.2016
 *      Author: stefan
 */
#include "i2c_master.h"
#include "nios_def_head.h"
#include "system.h"
#include <string.h>
#include "head_eeprom.h"


#define LAST_BYTE 1
#define READ 1
#define WRITE 0
#define USER_DATA_ADR_START

static int _seq_read_eeprom(alt_u8 * eeprom_data, alt_u32 number_of_byte_to_read, char chip_adr, alt_u32 addr);
static int _seq_write_eeprom(alt_u8 * eeprom_data, alt_u32 number_of_byte_to_write, char chip_adr, alt_u32 addr);

//--- head_eeprom_read --------------------------------------
// Fuji data, read byte by byte with one single I2C read command => fast
int head_eeprom_read(void)
{
	alt_u32 	head=0;
	// Read Data from Head eeprom
	//0x50, 0x52, 0x54, 0x56 for head 0-4

	for(head=0;head<MAX_HEADS_BOARD;head++)
	{
		//_seq_read_eeprom(alt_u8 * eeprom_data, alt_u32 number_of_byte_to_read, char chip_adr, char eeprom_adr0, char eeprom_adr1)
		if(_seq_read_eeprom(pRX_Status->head_eeprom[head], EEPROM_DATA_SIZE, 0x50+(head*2), 0x00)!=0)
			return(-1);
	}
	return (0);
}

//--- _seq_read_eeprom --------------------------------------
// read byte by byte with one single I2C read command => fast
static int _seq_read_eeprom(alt_u8 * eeprom_data, alt_u32 number_of_byte_to_read, char chip_adr, alt_u32 addr)
{
	int read_count=0;

	if(addr+(number_of_byte_to_read-1)>EEPROM_MAX_ADR)
	{
		return (-1);	// adr. to high
	}

	if (I2C_start(I2C_MASTER_0_BASE,(chip_adr),WRITE)==I2C_ACK)					// set chip address and set to write(0)
	{
		if(I2C_write(I2C_MASTER_0_BASE,(addr>>8)&0xff,!LAST_BYTE)==I2C_ACK)		// set address1 to eeprom_adr0
		{
			if(I2C_write(I2C_MASTER_0_BASE,addr&0xff,LAST_BYTE)==I2C_ACK)		// set address2 to eeprom_adr1
			{
				if(I2C_start(I2C_MASTER_0_BASE,(chip_adr),READ)==I2C_ACK)		// set chip address in read mode)
				{
					for(read_count=0; read_count<(number_of_byte_to_read-1);read_count++)
					{
						*eeprom_data = I2C_read(I2C_MASTER_0_BASE,!LAST_BYTE);  	// read
						eeprom_data++;	//inc. adr. of pointer
					}

					*eeprom_data =  I2C_read(I2C_MASTER_0_BASE,LAST_BYTE);  		// read last Byte
					//alt_printf("0x%x: 0x%x %c\n", eeprom_adr, head_data, head_data);

					return (0);
				}
			}
		}
	}
	return (-2);
}


//--- head_eeprom_read_user_data --------------------------------------
// read user data
int head_eeprom_read_user_data(alt_u32 head, alt_u8 * eeprom_data, alt_u32 number_of_byte_to_read, alt_u32 addr)
{
	// Read Data from Head eeprom
	//0x50, 0x52, 0x54, 0x56 for head 0-4

	if((EEPROM_USER_DATA_START+addr+number_of_byte_to_read)>EEPROM_MAX_ADR)
	{
		return(-1);		// number_of_byte_to_read extend EEPROM Size
	}

	if(_seq_read_eeprom(eeprom_data, number_of_byte_to_read, 0x50+(head*2), EEPROM_USER_DATA_START+addr)==0)
	{
		return (0);
	}

	return(-1);
}

//--- head_eeprom_write_user_data --------------------------------------
// write only page wise (32Byte) but with a single I2C write command => fast
int head_eeprom_write_user_data(alt_u32 head, alt_u8 * eeprom_data, alt_u32 number_of_byte_to_write, alt_u32 addr)
{
	// Write Data to Head eeprom
	//0x50, 0x52, 0x54, 0x56 for head 0-4

	if(EEPROM_USER_DATA_START+addr+number_of_byte_to_write>=EEPROM_MAX_ADR)
	{
		return(-1);		// number_of_byte_to_read extend EEPROM Size
	}

	return _seq_write_eeprom(eeprom_data, number_of_byte_to_write, 0x50+(head*2), EEPROM_USER_DATA_START+addr);
}

//--- head_eeprom_change_user_data ---------------------------------------------------------
int head_eeprom_change_user_data(alt_u32 head, alt_u8 *act_data, alt_u8 * new_data, alt_u32 cnt, alt_u32 addr)
{
	int idx;
	int len, l, end;
	int ret=REPLY_OK;
	int change=FALSE;

	for (idx=0; idx<cnt; idx++)
	{
		if (new_data[idx] == act_data[idx]) continue;
		for (len=0; idx+len<cnt && new_data[idx+len] != act_data[idx+len]; len++)
		{

		}
		// write in blocks that end in 32-Byte borders!
		while (ret==REPLY_OK && len)
		{
			change=TRUE;
			if (len>32) l=32;
			else		l=len;
			end=(idx/32) * 32 + 32;
			if (idx+l>=end) l=end-idx;
			ret=_seq_write_eeprom(&new_data[idx], l, 0x50+(head*2), EEPROM_USER_DATA_START+addr+idx);
			idx += l;
			len -= l;
		}
	}
	if (change)
		_seq_read_eeprom(act_data, cnt, 0x50+(head*2), EEPROM_USER_DATA_START+addr);
	return ret;
}


//--- _seq_write_eeprom --------------------------------------
// writes up to 32 bytes at ones to the eeprom but those
// 32 bytes have to be in the same (32 Byte)page as the EEPROM
// internal Address Increment only increment Adr. Bit 0-4!
//  waits till written into EEPROM!
static int _seq_write_eeprom(alt_u8 * eeprom_data, alt_u32 number_of_byte_to_write, char chip_adr, alt_u32 addr)
{
	int timeout;
	int write_count=0;
	if(addr+number_of_byte_to_write>=EEPROM_MAX_ADR)
	{
		return (-1);	// adr. to high
	}
	if (addr/32 != (addr+number_of_byte_to_write-1)/32)
	{
		return (-2);	// writing into multiple pages is not supported in a single seq. write!
	}

	if (I2C_start(I2C_MASTER_0_BASE,(chip_adr),WRITE)==I2C_ACK)						// set chip address and set to write(0)
	{
		if(I2C_write(I2C_MASTER_0_BASE,(addr>>8)&0xff,!LAST_BYTE)==I2C_ACK)			// set eeprom_adr0
		{
			if(I2C_write(I2C_MASTER_0_BASE,addr&0xff,!LAST_BYTE)==I2C_ACK)		// set address2 to eeprom_adr
			{
				for(write_count=0; write_count<(number_of_byte_to_write-1);write_count++)
				{
					if(I2C_write(I2C_MASTER_0_BASE,*eeprom_data,!LAST_BYTE)!=I2C_ACK)	// write data to eeprom_adr
					{
						return (-3);
					}
					eeprom_data++;	//inc. adr. of pointer
				}
				if(I2C_write(I2C_MASTER_0_BASE,*eeprom_data,LAST_BYTE)!=I2C_ACK)	// write Last Byte to eeprom_adr
				{
					return (-4);
				}
				// wait till data is written into EEPROM (max. 5ms per Page)
				for(timeout=50; (I2C_start(I2C_MASTER_0_BASE,(chip_adr),WRITE)==I2C_NOACK);)
				{
					if(!--timeout) return (-5);
				}
				return 0;
			}
		}
	}
	return (-6);
}

