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
/** \file work_flash.c
 **
 ** A detailed description is available at 
 ** @link WorkFlashGroup WorkFlash Module description @endlink
 **
 ** History:
 **   - 2013-10-09  0.1  Edison Zhang  First version.
 **
 ******************************************************************************/
/*---------------------------------------------------------------------------*/
/* include files                                                             */
/*---------------------------------------------------------------------------*/
#include "work_flash.h"

#if defined(PDL_PERIPHERAL_WORK_FLASH_ACTIVE) 

/**
 ******************************************************************************
 ** \addtogroup WorkFlashGroup
 ******************************************************************************/
//@{   
   

/*---------------------------------------------------------------------------*/
/* Local functions                                                           */
/*---------------------------------------------------------------------------*/
static uint8_t WFlash_CheckToggle( uint16_t* pAddr );
static void WFlash_ReadResetCmd(uint16_t* pu16ResetSecAddr);

/*!
 ******************************************************************************
 ** \brief Issue read/reset command
 **
 ** \param pu16ResetSecAddr address of reset sector  
 **
 ** \return None
 **
 ******************************************************************************
 */
static void WFlash_ReadResetCmd(uint16_t* pu16ResetSecAddr)
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
 ** \return Operation status
 ** \retval MFLASH_RET_OK
 ** \retval MFLASH_RET_ABNORMAL
 ** \retval MFLASH_RET_INVALID_PARA
 **
 ******************************************************************************
 */
uint8_t WFlash_ChipErase(void)
{
    uint8_t  u8Dummy;
    uint8_t u8RetValue = WFLASH_RET_OK;
    
    FM4_WORKFLASH_IF->WFASZR = 0x00;
    u8Dummy = FM4_WORKFLASH_IF->WFASZR;
    if(u8Dummy) ;
        
    Flash_Write((WFLASH_CODE1 | WFLASH_BASE_ADDR), 0x00AA);
    Flash_Write((WFLASH_CODE2 | WFLASH_BASE_ADDR), 0x0055);
    Flash_Write((WFLASH_CODE1 | WFLASH_BASE_ADDR), 0x0080);
    Flash_Write((WFLASH_CODE1 | WFLASH_BASE_ADDR), 0x00AA);
    Flash_Write((WFLASH_CODE2| WFLASH_BASE_ADDR), 0x0055);
    Flash_Write((WFLASH_CODE1 | WFLASH_BASE_ADDR), 0x0010);

     /*  if execution result of the automatic algorithm of flash memory is abnormally completed  */
    if( WFlash_CheckToggle((uint16_t*)WFLASH_BASE_ADDR) == WFLASH_CHK_TOGG_ABNORMAL)
    {
        /*  sending the read/reset command to the reset sector  */
        WFlash_ReadResetCmd((uint16_t*)WFLASH_BASE_ADDR) ;

        /*  return flash operation abnormally   */
        u8RetValue  = WFLASH_RET_ABNORMAL ;
    }
    
    /*  CPU ROM mode setting    */
    FM4_WORKFLASH_IF->WFASZR = 0x01; 
    u8Dummy = FM4_WORKFLASH_IF->WFASZR;

    return u8RetValue;
}

/*!
 ******************************************************************************
 ** \brief Flash sector erase       
 **
 ** \param pu16SecAddr address of flash sector  
 **
 ** \return Operation status
 ** \retval MFLASH_RET_OK
 ** \retval MFLASH_RET_ABNORMAL
 ** \retval MFLASH_RET_INVALID_PARA
 **
 ******************************************************************************
 */ 
uint8_t WFlash_SectorErase(uint16_t* pu16SecAddr)
{
    uint8_t  Dummy;
    uint8_t u8Ret = WFLASH_RET_OK;
    if(pu16SecAddr == (uint16_t*)0 )
        u8Ret = WFLASH_RET_INVALID_PARA;
    else
    {
        FM4_WORKFLASH_IF->WFASZR = 0x00;
        Dummy = FM4_WORKFLASH_IF->WFASZR;
        if(Dummy) ;
       
        Flash_Write((WFLASH_CODE1 | WFLASH_BASE_ADDR), 0x00AA);
        Flash_Write((WFLASH_CODE2 | WFLASH_BASE_ADDR), 0x0055);
        Flash_Write((WFLASH_CODE1 | WFLASH_BASE_ADDR), 0x0080);
        Flash_Write((WFLASH_CODE1 | WFLASH_BASE_ADDR), 0x00AA);
        Flash_Write((WFLASH_CODE2 | WFLASH_BASE_ADDR), 0x0055);
        Flash_Write(pu16SecAddr, 0x0030);

         /*  if execution result of the automatic algorithm of flash memory is abnormally completed  */
        if( WFlash_CheckToggle(pu16SecAddr) == WFLASH_CHK_TOGG_ABNORMAL)
        {
            /*  sending the read/reset command to the reset sector  */
            WFlash_ReadResetCmd(pu16SecAddr) ;

            /*  return flash operation abnormally   */
            u8Ret  = WFLASH_RET_ABNORMAL ;
        }

        /*  CPU ROM mode setting    */
        FM4_WORKFLASH_IF->WFASZR = 0x01; 
        Dummy = FM4_WORKFLASH_IF->WFASZR;
        
    }
    return u8Ret;
}

/*!
 ******************************************************************************
 ** \brief Flash half-word write with ECC  
 **
 ** \param pu16WriteAddr address of flash data
 ** \param pu16WriteData pointer to write data
 ** \param u32Size data size, 1 indicates 1 16-bit data, always set it to even
 **
 ** \return Operation status
 ** \retval MFLASH_RET_OK
 ** \retval MFLASH_RET_ABNORMAL
 ** \retval MFLASH_RET_INVALID_PARA
 **
 ******************************************************************************
 */ 
uint8_t WFlash_Write(uint16_t*  pu16WriteAddr, 
                     uint16_t* pu16WriteData, 
                     uint32_t u32Size)
{
    uint8_t    u8RetValue  = WFLASH_RET_OK ;
    uint32_t   u32Cnt;
    uint8_t    u8Dummy;
    
    if((pu16WriteAddr == (uint16_t*)0)
       || ((u32Size%2) != 0))
    {
        /*  return parameter invalid    */
        u8RetValue  = WFLASH_RET_INVALID_PARA ;
    }
    else
    { 
        /*  CPU programming mode setting    */
        FM4_WORKFLASH_IF->WFASZR = 0x00;
        u8Dummy = FM4_WORKFLASH_IF->WFASZR;
        if(u8Dummy) ;
        
        for(u32Cnt=u32Size;u32Cnt;u32Cnt--)
        {
            /*  issue write command   */
            Flash_Write((WFLASH_CODE1 | WFLASH_BASE_ADDR), 0x00AA) ;
            Flash_Write((WFLASH_CODE2 | WFLASH_BASE_ADDR), 0x0055) ;
            Flash_Write((WFLASH_CODE1 | WFLASH_BASE_ADDR), 0x00A0) ;
            Flash_Write(pu16WriteAddr, (uint16_t)*pu16WriteData);
            
            /*  execution result of the automatic algorithm of flash memory is abnormally complete or verify error  */
            if(( WFlash_CheckToggle(pu16WriteAddr) == WFLASH_CHK_TOGG_ABNORMAL) ||
              ( Flash_Read(pu16WriteAddr) != *pu16WriteData))
            {
                /*  issue read/reset command to reset sector    */
                WFlash_ReadResetCmd(pu16WriteAddr) ;
    
                /*  return flash operation abnormally   */
                u8RetValue  = WFLASH_RET_ABNORMAL ;
            }      
            /* Prepare next h-word write */
            pu16WriteAddr++;
            pu16WriteData++; 
        }
        
        /*  CPU ROM mode setting    */
        FM4_WORKFLASH_IF->WFASZR = 0x01; 
        u8Dummy = FM4_WORKFLASH_IF->WFASZR;
    }

    return u8RetValue ;
}

/*!
 ******************************************************************************
 ** \brief automatic algorithm of flash memory execution 
 **
 ** \param pu16Addr address of flash data
 **
 ** \return Operation status
 ** \retval MFLASH_RET_OK
 ** \retval MFLASH_RET_ABNORMAL
 ** \retval MFLASH_RET_INVALID_PARA
 **
 ******************************************************************************
 */
static uint8_t WFlash_CheckToggle( uint16_t* pu16Addr )
{
    uint16_t   u16SequenceFlag1, u16SequenceFlag2 ;  /*  hardware sequence flag */
    uint8_t   u8RetValue  = WFLASH_CHK_TOGG_NORMAL ;

    /* set hardware sequence flag */
    u16SequenceFlag1 = Flash_Read(pu16Addr) ;
    u16SequenceFlag2 = Flash_Read(pu16Addr) ;
    /*  if automatic algorithm is executing */
    while((( u16SequenceFlag1 ^ u16SequenceFlag2) & WFLASH_CHK_TOGG_MASK) == WFLASH_CHK_TOGG_MASK)
    {
        /*  if exceeds the timing limit */
        if(( u16SequenceFlag1 & WFLASH_CHK_TLOV_MASK) == WFLASH_CHK_TLOV_MASK)
        {
            /* set hardware sequence flag */
            u16SequenceFlag1 = Flash_Read(pu16Addr) ;
            u16SequenceFlag2 = Flash_Read(pu16Addr) ;

            /*  if automatic algorithm is executing */
            if((( u16SequenceFlag1 ^ u16SequenceFlag2) & WFLASH_CHK_TOGG_MASK) == WFLASH_CHK_TOGG_MASK)
            {
                /*  abnormally complete */
                u8RetValue  = WFLASH_CHK_TOGG_ABNORMAL ;

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
