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

static int _seq_read_eeprom(alt_u8 * eeprom_data, alt_u32 size, char head, alt_u32 addr);
static int _seq_write_eeprom(alt_u8 * eeprom_data, alt_u32 size, char head, alt_u32 addr);

//--- head_eeprom_read --------------------------------------
// Fuji data, read byte by byte with one single I2C read command => fast
int head_eeprom_read(alt_u32 head, alt_u8 * eeprom_data, alt_u32 size)
{
	// Read Data from Head eeprom
	//0x50, 0x52, 0x54, 0x56 for head 0-4

	//_seq_read_eeprom(alt_u8 * eeprom_data, alt_u32 number_of_byte_to_read, char chip_adr, char eeprom_adr0, char eeprom_adr1)
	return _seq_read_eeprom(eeprom_data, size, head, 0x00)==0;
}

//--- _seq_read_eeprom --------------------------------------
// read byte by byte with one single I2C read command => fast
static int _seq_read_eeprom(alt_u8 * eeprom_data, alt_u32 size, char head, alt_u32 addr)
{
	char chip_adr=0x50+(head*2);

	if(addr+size>=EEPROM_MAX_ADR)
	{
		return (-1);	// adr. to high
	}

	if (I2C_start(I2C_MASTER_0_BASE,chip_adr,WRITE)==I2C_ACK)					// set chip address and set to write(0)
	{
		if(I2C_write(I2C_MASTER_0_BASE,(addr>>8)&0xff,!LAST_BYTE)==I2C_ACK)		// set address1 to eeprom_adr0
		{
			if(I2C_write(I2C_MASTER_0_BASE,addr&0xff,LAST_BYTE)==I2C_ACK)		// set address2 to eeprom_adr1
			{
				if(I2C_start(I2C_MASTER_0_BASE, chip_adr,READ)==I2C_ACK)		// set chip address in read mode)
				{
					while(size--)
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
int head_eeprom_read_user_data(alt_u32 head, alt_u32 addr, alt_u8 *eeprom_data, alt_u32 size)
{
	// Read Data from Head eeprom
	//0x50, 0x52, 0x54, 0x56 for head 0-4

	if((EEPROM_USER_DATA_START+addr+size)>=EEPROM_MAX_ADR)
	{
		memset(eeprom_data, 0x00, size);
		return FALSE;		// number_of_byte_to_read extend EEPROM Size
	}

	if(_seq_read_eeprom(eeprom_data, size, head, EEPROM_USER_DATA_START+addr)==0)
	{
		return TRUE;
	}
	memset(eeprom_data, 0x00, size);
	return FALSE;
}

//--- head_eeprom_write_user_data --------------------------------------
// write only page wise (32Byte) but with a single I2C write command => fast
int head_eeprom_write_user_data(alt_u32 head, alt_u32 addr, alt_u8 *eeprom_data, alt_u32 size)
{
	// Write Data to Head eeprom
	//0x50, 0x52, 0x54, 0x56 for head 0-4

	if(EEPROM_USER_DATA_START+addr+size>=EEPROM_MAX_ADR)
	{
		return(-1);		// number_of_byte_to_read extend EEPROM Size
	}

	int ret=0;
	while(size>0)
	{
		int len=size;
		if (len>32) len=32;
		ret=_seq_write_eeprom(eeprom_data, len, head, EEPROM_USER_DATA_START+addr);
		if (ret) return ret;
		addr+=len;
		size-=len;
		eeprom_data+=len;
	}
	return 0;
}

//--- head_eeprom_change_user_data ---------------------------------------------------------
int head_eeprom_change_user_data(alt_u32 head, alt_u32 addr, alt_u8 *act_data, alt_u8 * new_data, alt_u32 cnt)
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
			ret=_seq_write_eeprom(&new_data[idx], l, head, EEPROM_USER_DATA_START+addr+idx);
			idx += l;
			len -= l;
		}
	}
	if (change)
		_seq_read_eeprom(act_data, cnt, head, EEPROM_USER_DATA_START+addr);
	return ret;
}

//--- _seq_write_eeprom --------------------------------------
// writes up to 32 bytes at ones to the eeprom but those
// 32 bytes have to be in the same (32 Byte)page as the EEPROM
// internal Address Increment only increment Adr. Bit 0-4!
//  waits till written into EEPROM!
static int _seq_write_eeprom(alt_u8 * eeprom_data, alt_u32 size, char head, alt_u32 addr)
{
	char chip_adr=0x50+(head*2);
	int timeout;
	if(addr+size>=EEPROM_MAX_ADR)
	{
		return (-1);	// adr. to high
	}
	if (addr/32 != (addr+size-1)/32)
	{
		return (-2);	// writing into multiple pages is not supported in a single seq. write!
	}

	if (I2C_start(I2C_MASTER_0_BASE,chip_adr,WRITE)==I2C_ACK)					// set chip address and set to write(0)
	{
		if(I2C_write(I2C_MASTER_0_BASE,(addr>>8)&0xff,!LAST_BYTE)==I2C_ACK)		// set eeprom_adr0
		{
			if(I2C_write(I2C_MASTER_0_BASE,addr&0xff,!LAST_BYTE)==I2C_ACK)		// set address2 to eeprom_adr
			{
				while(--size)
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
				for(timeout=50; (I2C_start(I2C_MASTER_0_BASE,chip_adr,WRITE)==I2C_NOACK);)
				{
					if(!--timeout) return (-5);
				}
				return 0;
			}
		}
	}
	return (-6);
}

