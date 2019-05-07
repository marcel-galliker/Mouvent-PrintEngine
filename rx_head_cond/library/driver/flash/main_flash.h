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
/** \file main_flash.h
 **
 ** Headerfile for Main Flash operaiton
 **  
 ** History:
 **   - 2013-10-09  0.1  Edison Zhang  First version.
 **   - 2014-05-28  1.0  MWi           Doxygen comments corrected.
 **
 ******************************************************************************/

#ifndef _MAIN_FLASH_H_
#define _MAIN_FLASH_H_

/**
 ******************************************************************************
 ** \defgroup MainFlashGroup Main Flash
 **
 ** Provided functions of Main Flash operation:
 ** 
 ** - MFlash_ChipErase()
 ** - MFlash_SectorErase()
 ** - MFlash_Write() 
 **
 ** \brief What should be done before using these APIs?
 ** Before using the Main Flash operation APIs, please make sure the code is 
 ** operated in RAM area. 
 ** In the IAR environment, the key word "__ramfunc" makes sure that the code 
 ** is implemented from RAM. 
 ** In the Keil environment, user has to place the code to RAM area manually.
 ** 1) Right click main_flash.c
 ** 2) [Options for main_flash.c]->[code/const]->select a RAM area
 **
 ** \brief How to use Main Flash module?
 ** MFlash_ChipErase() can erase whole chip space of Main Flash, whether CR 
 ** data remains after chip erase depends on the parameter bCrRemain.
 ** MFlash_SectorErase() can erase one selected sector.
 ** MFlash_Write() writes data into Flash area with word align, as ECC is 
 ** equipped in the Flash module. Whether data verify and ECC check is done
 ** depends on the parameter bVerifyAndEccCheck.
 ** 
 ** 
 ******************************************************************************/ 

/*---------------------------------------------------------------------------*/
/* include files                                                             */
/*---------------------------------------------------------------------------*/
#include "pdl.h"

#if defined(PDL_PERIPHERAL_MAIN_FLASH_ACTIVE) 

//@{
 
#define     MFLASH_CODE1                 0x0AA8
#define     MFLASH_CODE2                 0x0554

#define     MFLASH_RET_OK                0
#define     MFLASH_RET_INVALID_PARA      1
#define     MFLASH_RET_ABNORMAL          2

#define     MFLASH_CHK_TOGG_NORMAL       0
#define     MFLASH_CHK_TOGG_ABNORMAL     1

#define     MFLASH_CHK_DPOL_NORMAL       0
#define     MFLASH_CHK_DPOL_ABNORMAL     1

#define     MFLASH_CHK_DPOL_MASK         (uint16_t)0x0080
#define     MFLASH_CHK_TOGG_MASK         (uint16_t)0x0040
#define     MFLASH_CHK_TLOV_MASK         (uint16_t)0x0020
   
#define     CR_DATA_ADDR                 (0x00402000)

/*---------------------------------------------------------------------------*/
/* function prototypes                                                       */
/*---------------------------------------------------------------------------*/
#ifdef __cplusplus
extern "C" {
#endif
uint8_t MFlash_ChipErase(boolean_t bCrRemain);
uint8_t MFlash_SectorErase(uint16_t* pu16SecAddr);
uint8_t MFlash_Write(uint16_t*  pWriteAddr, 
                     uint16_t* pWriteData, 
                     uint32_t EvenSize,
                     boolean_t bVerifyAndEccCheck);

#ifdef __cplusplus
}
#endif

//@}

#endif /* PERIPHERAL_AVAILABLE_MAINFLASH */

#endif /* _MAINFLASH_FM3_H_ */
