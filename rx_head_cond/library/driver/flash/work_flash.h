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
/** \file work_flash.h
 **
 ** Headerfile for Work Flash operaiton
 **  
 ** History:
 **   - 2013-10-09  0.1  Edison Zhang First version.
 **
 ******************************************************************************/

#ifndef _WORK_FLASH_H_
#define _WORK_FLASH_H_

/**
 ******************************************************************************
 ** \defgroup WorkFlashGroup Work Flash
 **
 ** Provided functions of Work Flash operation:
 ** - WFlash_ChipErase()
 ** - WFlash_SectorErase()
 ** - WFlash_Write() 
 **
 ** \brief What is Work Flash?
 ** Work Flash have independent area, which is sperated from Main Flash, 
 ** thus the Flash operation API can be called derectly from Main Flash. 
 **
 ** \brief How to use Work Flash module?
 ** WFlash_ChipErase() can erase whole chip space of Work Flash.
 ** WFlash_SectorErase() can erase one selected sector.
 ** WFlash_Write() writes data into Flash area with half-word align.
 ** 
 ******************************************************************************/ 

/*---------------------------------------------------------------------------*/
/* include files                                                             */
/*---------------------------------------------------------------------------*/
#include "pdl.h"

#if defined(PDL_PERIPHERAL_WORK_FLASH_ACTIVE) 

//@{
 
#define     WFLASH_CODE1                 0x0AA8
#define     WFLASH_CODE2                 0x0554

#define     WFLASH_RET_OK                0
#define     WFLASH_RET_INVALID_PARA      1
#define     WFLASH_RET_ABNORMAL          2

#define     WFLASH_CHK_TOGG_NORMAL       0
#define     WFLASH_CHK_TOGG_ABNORMAL     1

#define     WFLASH_CHK_DPOL_NORMAL       0
#define     WFLASH_CHK_DPOL_ABNORMAL     1

#define     WFLASH_CHK_DPOL_MASK         (uint16_t)0x0080
#define     WFLASH_CHK_TOGG_MASK         (uint16_t)0x0040
#define     WFLASH_CHK_TLOV_MASK         (uint16_t)0x0020
   
#define     WFLASH_BASE_ADDR             (0x200C0000)
   
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
	 
	 
/*---------------------------------------------------------------------------*/
/* function prototypes                                                       */
/*---------------------------------------------------------------------------*/
#ifdef __cplusplus
extern "C" {
#endif
uint8_t WFlash_ChipErase(void);
uint8_t WFlash_SectorErase(uint16_t* pu16SecAddr);
uint8_t WFlash_Write(uint16_t*  pWriteAddr, 
                     uint16_t* pWriteData, 
                     uint32_t Size);
uint8_t WFlash_SectorErase(uint16_t* pu16SecAddr);
	

#ifdef __cplusplus
}
#endif

//@}

#endif /* PERIPHERAL_AVAILABLE_WORKFLASH */

#endif /* _WORKFLASH_FM3_H_ */
