/******************************************************************************/
/** \file work_flash_operation.c
 ** 
 ** read and write to work flash section
 **
 **	Copyright 2016 by radex AG, Switzerland. All rights reserved.
 **	Written by Stefan Weber
 **
 ******************************************************************************/

/******************************************************************************/
/* Include files                                                              */
/******************************************************************************/
#include "cond_def_head.h"
#include "work_flash_operation.h"

/**
 ******************************************************************************
 **  \brief       write actual settings to work flash 						
 ******************************************************************************/
void write_data_to_flash(void)
{		
	if (RX_Status.pump_minutes!=INVALID_VALUE)
	{
		// erase flash, write flash
		if(WFlash_SectorErase((uint16_t*)work_flash_base_adr)!=0)
		{
	//		RX_Status.cond_error.bitset.flash_write_error=1;
		}
		else
		{
			
			if(WFlash_Write((uint16_t*)work_flash_base_adr, (uint16_t*)&RX_Status.pump_minutes, sizeof(&RX_Status.pump_minutes)/sizeof(uint16_t))!=0)
			{
	//			RX_Status.cond_error.flash_write_error=0;			
			}
		}
	}
}

/**
 ******************************************************************************
 **  \brief       read back settings saved on work flash
 ******************************************************************************/
void read_data_from_flash(void)
{
	//--- read pump time ----------------------
	uint32_t pump_time=0;
	uint16_t *ptr=(uint16_t*)pump_time;
	
	ptr[0] =  Flash_Read(work_flash_base_adr);
	ptr[1] =  Flash_Read(work_flash_base_adr+2);
		
	if(pump_time==0x0000FFFF) pump_time=0;	// first use of flash!
	RX_Status.pump_minutes=pump_time;

	//--- read unique id ------------------------
	//reads out UIDR0 and shifts it by 4 (LSB aligned)
	RX_Status.unique_id[0]=((FM4_UNIQUE_ID->UIDR0)>>4);
	//reads out UIDR1 and masks the upper 19 bits to '0'
	RX_Status.unique_id[1]=((FM4_UNIQUE_ID->UIDR1)& 0x00001FFFu);
}

/**
 ******************************************************************************
 **  \brief       work flash test
 ******************************************************************************/
void work_flash_test(void)
{
    uint16_t aArray[4] = {0x1122, 0x3344, 0x5566, 0x7788};
    uint16_t bArray[4] = {0x99aa, 0xbbcc, 0xddee, 0xff00};
    /* test sector 0 */
    WFlash_SectorErase((uint16_t*)0x200C0000);
    WFlash_Write((uint16_t*)0x200C0000, aArray, sizeof(aArray)/sizeof(uint16_t));
    /* test sector 3 */
    WFlash_SectorErase((uint16_t*)0x200C6000);
    WFlash_Write((uint16_t*)0x200C6000, bArray, sizeof(bArray)/sizeof(uint16_t));
    
    /* Erase whole Work Flash */
    WFlash_ChipErase();	
}
