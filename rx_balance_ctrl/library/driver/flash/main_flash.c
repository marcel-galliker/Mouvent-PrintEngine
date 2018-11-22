/*******************************************************************************
* Copyright (C) 2013 Spansion LLC. All Rights Reserved. 
*
* This software is owned and published by: 
* Spansion LLC, 915 DeGuigne Dr. Sunnyvale, CA  94088-3453 ("Spansion").
*
* BY DOWNLOADING, INSTALLING OR USING THIS SOFTWARE, YOU AGREE TO BE BOUND 
* BY ALL THE TERMS AND CONDITIONS OF THIS AGREEMENT.
*
* This software contains source code for use with Spansion 
* components. This software is licensed by Spansion to be adapted only 
* for use in systems utilizing Spansion components. Spansion shall not be 
* responsible for misuse or illegal use of this software for devices not 
* supported herein.  Spansion is providing this software "AS IS" and will 
* not be responsible for issues arising from incorrect user implementation 
* of the software.  
*
* SPANSION MAKES NO WARRANTY, EXPRESS OR IMPLIED, ARISING BY LAW OR OTHERWISE,
* REGARDING THE SOFTWARE (INCLUDING ANY ACOOMPANYING WRITTEN MATERIALS), 
* ITS PERFORMANCE OR SUITABILITY FOR YOUR INTENDED USE, INCLUDING, 
* WITHOUT LIMITATION, THE IMPLIED WARRANTY OF MERCHANTABILITY, THE IMPLIED 
* WARRANTY OF FITNESS FOR A PARTICULAR PURPOSE OR USE, AND THE IMPLIED 
* WARRANTY OF NONINFRINGEMENT.  
* SPANSION SHALL HAVE NO LIABILITY (WHETHER IN CONTRACT, WARRANTY, TORT, 
* NEGLIGENCE OR OTHERWISE) FOR ANY DAMAGES WHATSOEVER (INCLUDING, WITHOUT 
* LIMITATION, DAMAGES FOR LOSS OF BUSINESS PROFITS, BUSINESS INTERRUPTION, 
* LOSS OF BUSINESS INFORMATION, OR OTHER PECUNIARY LOSS) ARISING FROM USE OR 
* INABILITY TO USE THE SOFTWARE, INCLUDING, WITHOUT LIMITATION, ANY DIRECT, 
* INDIRECT, INCIDENTAL, SPECIAL OR CONSEQUENTIAL DAMAGES OR LOSS OF DATA, 
* SAVINGS OR PROFITS, 
* EVEN IF SPANSION HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES. 
* YOU ASSUME ALL RESPONSIBILITIES FOR SELECTION OF THE SOFTWARE TO ACHIEVE YOUR
* INTENDED RESULTS, AND FOR THE INSTALLATION OF, USE OF, AND RESULTS OBTAINED 
* FROM, THE SOFTWARE.  
*
* This software may be replicated in part or whole for the licensed use, 
* with the restriction that this Disclaimer and Copyright notice must be 
* included with each copy of this software, whether used in part or whole, 
* at all times.  
*/

/******************************************************************************/
/** \file main_flash.c
 **
 ** A detailed description is available at 
 ** @link MainFlashGroup MainFlash Module description @endlink
 **
 ** History:
 **   - 2013-10-09  0.1  Edison Zhang  First version.
 **
 ******************************************************************************/
/*---------------------------------------------------------------------------*/
/* include files                                                             */
/*---------------------------------------------------------------------------*/
#include "main_flash.h"

#if defined(PDL_PERIPHERAL_MAIN_FLASH_ACTIVE) 

/**
 ******************************************************************************
 ** \addtogroup MainFlashGroup
 ******************************************************************************/
//@{   
   
/*!
 ******************************************************************************
 ** \brief Read a half word data from Flash
 ** \param addr Pointer to read data address
 ******************************************************************************
 */
#define Flash_Read(addr)        *(volatile uint16_t*)((uint32_t)(addr))

/*!
 ******************************************************************************
 ** \brief Wirte a half word data into Flash
 ** \param addr Pointer to read data address
 ** \param data Write data
 ******************************************************************************
 */
#define Flash_Write(addr, data) *(volatile uint16_t*)((uint32_t)(addr)) = ( uint16_t)(data)

#if defined ( __ICCARM__ )
__ramfunc 
#endif
static uint8_t MFlash_CheckToggle( uint16_t* pAddr );

#if defined ( __ICCARM__ )
__ramfunc 
#endif 
static void MFlash_ReadResetCmd(uint16_t* pResetSecAddr);

/*!
 ******************************************************************************
 ** \brief Issue read/reset command
 **
 ** \param [in] pu16ResetSecAddr  Address of reset sector  
 **
 ** \return None
 **
 ******************************************************************************
 */
#if defined ( __ICCARM__ )
__ramfunc 
#endif 
static void MFlash_ReadResetCmd(uint16_t* pu16ResetSecAddr)
{
    uint8_t  u8Dummy;

    /*  issue read/reset command    */
    Flash_Write(0x0000, 0xF0U) ;
    u8Dummy = Flash_Read(pu16ResetSecAddr) ;
    if(u8Dummy)  /* avoid warning */  
        ;
    return ;
}

/*!
 ******************************************************************************
 ** \brief Flash chip erase       
 **
 ** \param bCrRemain CR remaim flag  
 **
 ** \return Operation status
 ** \retval MFLASH_RET_OK
 ** \retval MFLASH_RET_ABNORMAL
 ** \retval MFLASH_RET_INVALID_PARA
 **
 ******************************************************************************
 */
#if defined ( __ICCARM__ )
__ramfunc 
#endif 
uint8_t MFlash_ChipErase(boolean_t bCrRemain)
{
    uint8_t  u8Dummy, u8Cnt;
    uint32_t u32CrData, u32CrAddr;
    uint16_t u16WriteData;
    uint8_t u8RetValue = MFLASH_RET_OK;
    if(bCrRemain > 1)
        u8RetValue = MFLASH_RET_INVALID_PARA;
    else
    {
        /* 
         * It can not be interrupted when set to 16-bit operation mode, 
         * as code may switch to 32-bit operation before write is completed.
         */
        __disable_irq(); 
        /* Save CR value */
        if (bCrRemain == TRUE)
        {
            u32CrData = *(uint32_t*)CR_DATA_ADDR;
        }
        /* CPU programming mode setting */
        FM4_FLASH_IF->FASZR = 0x01;
        u8Dummy = FM4_FLASH_IF->FASZR;
        if(u8Dummy) ;
        Flash_Write(MFLASH_CODE1, 0x00AA);
        Flash_Write(MFLASH_CODE2, 0x0055);
        Flash_Write(MFLASH_CODE1, 0x0080);
        Flash_Write(MFLASH_CODE1, 0x00AA);
        Flash_Write(MFLASH_CODE2, 0x0055);
        Flash_Write(MFLASH_CODE1, 0x0010);

         /*  if execution result of the automatic algorithm of flash memory is abnormally completed  */
        if( MFlash_CheckToggle((uint16_t*)0) == MFLASH_CHK_TOGG_ABNORMAL)
        {
            /*  sending the read/reset command to the reset sector  */
            MFlash_ReadResetCmd((uint16_t*)0) ;

            /*  return flash operation abnormally   */
            u8RetValue  = MFLASH_RET_ABNORMAL ;
        }

        /* restore CR data in Flash */
        if (bCrRemain == TRUE)
        {
            for(u8Cnt=2,u16WriteData=(uint16_t)u32CrData, u32CrAddr = CR_DATA_ADDR;u8Cnt;u8Cnt--)
            {
                /*  issue write command   */
                Flash_Write(MFLASH_CODE1, 0x00AA) ;
                Flash_Write(MFLASH_CODE2, 0x0055) ;
                Flash_Write(MFLASH_CODE1, 0x00A0) ;
                Flash_Write((uint16_t*)u32CrAddr, u16WriteData);
                
                /*  execution result of the automatic algorithm of flash memory is abnormally complete or verify error  */
                if(( MFlash_CheckToggle((uint16_t*)u32CrAddr) == MFLASH_CHK_TOGG_ABNORMAL) ||
                  ( Flash_Read((uint16_t*)u32CrAddr) != u16WriteData))
                {
                    /*  issue read/reset command to reset sector    */
                    MFlash_ReadResetCmd((uint16_t*)u32CrAddr) ;
        
                    /*  return flash operation abnormally   */
                    u8RetValue  = MFLASH_RET_ABNORMAL ;
                }      
                u16WriteData = (uint16_t)(u32CrData>>16);
                u32CrAddr += 2;
            }
        }
        
        /*  CPU ROM mode setting    */
        FM4_FLASH_IF->FASZR = 2; 
        u8Dummy = FM4_FLASH_IF->FASZR;
        
        /* Recover IRQ  */
        __enable_irq();
    }
    return u8RetValue;
}

/*!
 ******************************************************************************
 ** \brief Flash sector erase       
 **
 ** \param pu16SecAddr   Address of flash sector  
 **
 ** \return Operation status
 ** \retval MFLASH_RET_OK
 ** \retval MFLASH_RET_ABNORMAL
 ** \retval MFLASH_RET_INVALID_PARA
 **
 ******************************************************************************
 */
#if defined ( __ICCARM__ )
__ramfunc 
#endif 
uint8_t MFlash_SectorErase(uint16_t* pu16SecAddr)
{
    uint8_t  Dummy;
    uint8_t u8Ret = MFLASH_RET_OK;
    if(pu16SecAddr == (uint16_t*)0 )
        u8Ret = MFLASH_RET_INVALID_PARA;
    else
    {
        /* 
         * It can not be interrupted when set to 16-bit operation mode, 
         * as code may switch to 32-bit operation before write is completed.
         */
        __disable_irq(); 
        /* CPU programming mode setting */
        FM4_FLASH_IF->FASZR = 0x01;
        Dummy = FM4_FLASH_IF->FASZR;
        if(Dummy) ;
        Flash_Write(MFLASH_CODE1, 0x00AA);
        Flash_Write(MFLASH_CODE2, 0x0055);
        Flash_Write(MFLASH_CODE1, 0x0080);
        Flash_Write(MFLASH_CODE1, 0x00AA);
        Flash_Write(MFLASH_CODE2, 0x0055);
        Flash_Write(pu16SecAddr, 0x0030);

         /*  if execution result of the automatic algorithm of flash memory is abnormally completed  */
        if( MFlash_CheckToggle(pu16SecAddr) == MFLASH_CHK_TOGG_ABNORMAL)
        {
            /*  sending the read/reset command to the reset sector  */
            MFlash_ReadResetCmd(pu16SecAddr) ;

            /*  return flash operation abnormally   */
            u8Ret  = MFLASH_RET_ABNORMAL ;
        }

        /*  CPU ROM mode setting    */
        FM4_FLASH_IF->FASZR = 2; 
        Dummy = FM4_FLASH_IF->FASZR;
        /* Recover IRQ  */
        __enable_irq();
        
    }
    return u8Ret;
}

/*!
 ******************************************************************************
 ** \brief Flash half-word write with ECC  
 **
 ** \param [in] pu16WriteAddr       Address of flash data
 ** \param [in] pu16WriteData       Pointer to write data
 ** \param [in] u32EvenSize         Data size, 1 indicates 1 16-bit data, always set it to even
 ** \param [in] bVerifyAndEccCheck  TRUE: Verify and check ECC, FALSE: No verification
 **
 ** \return Operation status
 ** \retval MFLASH_RET_OK
 ** \retval MFLASH_RET_ABNORMAL
 ** \retval MFLASH_RET_INVALID_PARA
 **
 ******************************************************************************
 */
#if defined ( __ICCARM__ )
__ramfunc 
#endif 
uint8_t MFlash_Write(uint16_t*  pu16WriteAddr, 
                     uint16_t* pu16WriteData, 
                     uint32_t u32EvenSize,
                     boolean_t bVerifyAndEccCheck)
{
    uint8_t   u8RetValue  = MFLASH_RET_OK ;
    uint32_t   u32Cnt;
    uint8_t    u8Dummy;
    uint16_t*  pu16OrgWriteAddr;
    uint16_t*  pu16OrgWriteData;
    pu16OrgWriteAddr = pu16WriteAddr;
    pu16OrgWriteData = pu16WriteData;
    
    if((pu16WriteAddr == (uint16_t*)0)
       || ((u32EvenSize%2) != 0))
    {
        /*  return parameter invalid    */
        u8RetValue  = MFLASH_RET_INVALID_PARA ;
    }
    else
    {
        /* 
         * It can not be interrupted when set to 16-bit operation mode, 
         * as code may switch to 32-bit operation before write is completed.
         */
        __disable_irq(); 
        /*  CPU programming mode setting    */
        FM4_FLASH_IF->FASZR = 1; 
        u8Dummy = FM4_FLASH_IF->FASZR;
        if(u8Dummy)   /* avoid warning */  
            ;  
        for(u32Cnt=u32EvenSize;u32Cnt;u32Cnt--)
        {
            /*  issue write command   */
            Flash_Write(MFLASH_CODE1, 0x00AA) ;
            Flash_Write(MFLASH_CODE2, 0x0055) ;
            Flash_Write(MFLASH_CODE1, 0x00A0) ;
            Flash_Write(pu16WriteAddr, (uint16_t)*pu16WriteData);
            
            /*  execution result of the automatic algorithm of flash memory is abnormally complete or verify error  */
            if(( MFlash_CheckToggle(pu16WriteAddr) == MFLASH_CHK_TOGG_ABNORMAL) ||
              ( Flash_Read(pu16WriteAddr) != *pu16WriteData))
            {
                /*  issue read/reset command to reset sector    */
                MFlash_ReadResetCmd(pu16WriteAddr) ;
    
                /*  return flash operation abnormally   */
                u8RetValue  = MFLASH_RET_ABNORMAL ;
            }      
            /* Prepare next h-word write */
            pu16WriteAddr++;
            pu16WriteData++; 
        }
        
        /*  CPU ROM mode setting    */
        FM4_FLASH_IF->FASZR = 2; 
        u8Dummy = FM4_FLASH_IF->FASZR;
        
        /* Recover IRQ  */
        __enable_irq();
        
        /* Verify the readback data */
        if (bVerifyAndEccCheck)
        {
            for(u32Cnt=u32EvenSize;u32Cnt;u32Cnt--)
            {
                if(*pu16OrgWriteAddr == *pu16OrgWriteData)
                {
                    pu16OrgWriteAddr++;
                    pu16OrgWriteData++;
                }
                else
                {
                    return MFLASH_RET_ABNORMAL;
                }
            }
            
            /* Check ECC error */
            if(bFM4_FLASH_IF_FSTR_ERR)
            {
                bFM4_FLASH_IF_FSTR_ERR = 0;
                return MFLASH_RET_ABNORMAL;
            }
        }
    }

    return u8RetValue ;
}

/*!
 ******************************************************************************
 ** \brief automatic algorithm of flash memory execution 
 **
 ** \param [in] pu16Addr  Address of flash data
 **
 ** \return Operation status
 ** \retval MFLASH_RET_OK
 ** \retval MFLASH_RET_ABNORMAL
 ** \retval MFLASH_RET_INVALID_PARA
 **
 ******************************************************************************
 */
#if defined ( __ICCARM__ )
__ramfunc 
#endif 
static uint8_t MFlash_CheckToggle( uint16_t* pu16Addr )
{
    uint16_t   u16SequenceFlag1, u16SequenceFlag2 ;  /*  hardware sequence flag */
    uint8_t   u8RetValue  = MFLASH_CHK_TOGG_NORMAL ;

    /* set hardware sequence flag */
    u16SequenceFlag1 = Flash_Read(pu16Addr) ;
    u16SequenceFlag2 = Flash_Read(pu16Addr) ;
    /*  if automatic algorithm is executing */
    while((( u16SequenceFlag1 ^ u16SequenceFlag2) & MFLASH_CHK_TOGG_MASK) == MFLASH_CHK_TOGG_MASK)
    {
        /*  if exceeds the timing limit */
        if(( u16SequenceFlag1 & MFLASH_CHK_TLOV_MASK) == MFLASH_CHK_TLOV_MASK)
        {
            /* set hardware sequence flag */
            u16SequenceFlag1 = Flash_Read(pu16Addr) ;
            u16SequenceFlag2 = Flash_Read(pu16Addr) ;

            /*  if automatic algorithm is executing */
            if((( u16SequenceFlag1 ^ u16SequenceFlag2) & MFLASH_CHK_TOGG_MASK) == MFLASH_CHK_TOGG_MASK)
            {
                /*  abnormally complete */
                u8RetValue  = MFLASH_CHK_TOGG_ABNORMAL ;

                break;
            }
        }

        /* set hardware sequence flag */
        u16SequenceFlag1 = Flash_Read(pu16Addr) ;
        u16SequenceFlag2 = Flash_Read(pu16Addr) ;
    }

    return u8RetValue ;
}

//@}

#endif

/******************************************************************************/
/* EOF (not truncated)                                                        */
/******************************************************************************/
