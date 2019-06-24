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

static UINT32 _PrintingTime = INVALID_VALUE;	// value saved in flash
	
/**
 ******************************************************************************
 **  \brief       write actual settings to work flash 						
 ******************************************************************************/
void flash_write_data(UINT32 pumptime)
{		
	if (_PrintingTime != pumptime)
	{
		// erase flash, write flash
		if (WFlash_SectorErase((uint16_t*)CONDITIONER_FLASH_USER_ADDR) == WFLASH_RET_OK
            && WFlash_Write((uint16_t*)CONDITIONER_FLASH_USER_ADDR, 
            (uint16_t*)&RX_Status.pumptime, sizeof(&RX_Status.pumptime)/sizeof(uint16_t)) 
            == WFLASH_RET_OK)
		{		
			_PrintingTime = RX_Status.pumptime;
		}
	}
}

/**
 ******************************************************************************
 **  \brief       read back settings saved on work flash
 ******************************************************************************/
UINT32 flash_read_data(void)
{   
    #define EMPTY_FLASH 0x0000FFFF
    
	//--- read pump time ----------------------
	uint16_t *ptr=(uint16_t*)&_PrintingTime;
	
	ptr[0] =  Flash_Read(CONDITIONER_FLASH_USER_ADDR);
	ptr[1] =  Flash_Read(CONDITIONER_FLASH_USER_ADDR+2);

	if (_PrintingTime == EMPTY_FLASH) 
        _PrintingTime = 0;	// first use of flash!
	
	//--- read unique id ------------------------
	//reads out UIDR0 and shifts it by 4 (LSB aligned)
//	RX_Status.unique_id[0]=((FM4_UNIQUE_ID->UIDR0)>>4);
	//reads out UIDR1 and masks the upper 19 bits to '0'
//	RX_Status.unique_id[1]=((FM4_UNIQUE_ID->UIDR1)& 0x00001FFFu);
    
    return _PrintingTime;
}

/**
 ******************************************************************************
 **  \brief       work flash test
 ******************************************************************************/
void flash_test(void)
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
