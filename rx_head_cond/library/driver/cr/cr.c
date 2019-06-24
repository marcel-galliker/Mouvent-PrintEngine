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
/** \file cr.c
 **
 ** A detailed description is available at 
 ** @link CrGroup CR Module description @endlink
 **
 ** History:
 **   - 2013-10-11  0.1  Edison Zhang   First version.
 **
 ******************************************************************************/

/******************************************************************************/
/* Include files                                                              */
/******************************************************************************/
#include "cr.h"

#if (defined(PDL_PERIPHERAL_CR_ACTIVE))

/**
 ******************************************************************************
 ** \addtogroup CrGroup
 ******************************************************************************/
//@{

/******************************************************************************/
/* Local pre-processor symbols/macros ('#define')                             */
/******************************************************************************/
#define CR_REG_UNLOCK_CODE        0x1ACCE554
#define CR_REG_LOCK_CODE          0  
   
/******************************************************************************/
/* Global variable definitions (declared in header file with 'extern')        */
/******************************************************************************/

/******************************************************************************/
/* Local type definitions ('typedef')                                         */
/******************************************************************************/

/******************************************************************************/
/* Local function prototypes ('static')                                       */
/******************************************************************************/

/******************************************************************************/
/* Local variable definitions ('static')                                      */
/******************************************************************************/

/**
 ******************************************************************************
 ** \brief   Set the frequency division of CR output to Base timer
 **
 ** \param   [in]  enCrDiv            CR division
 ** \arg           CrFreqDivBy4
 ** \arg           CrFreqDivBy8
 ** \arg           CrFreqDivBy16
 ** \arg           CrFreqDivBy32
 ** \arg           CrFreqDivBy64
 ** \arg           CrFreqDivBy128
 ** \arg           CrFreqDivBy256
 ** \arg           CrFreqDivBy512
 **
 ** \retval  Ok                       Set the frequency division successfully
 ** \retval  ErrorInvalidParameter    enCrDiv > CrFreqDivBy512
 **                                     
 ******************************************************************************/
en_result_t Cr_SetFreqDiv(en_cr_freq_div_t enCrDiv)
{
    if(enCrDiv > CrFreqDivBy512)
    {
        return ErrorInvalidParameter;
    }
    
    FM4_CRTRIM->MCR_PSR = enCrDiv;
    
    return Ok;
}

/**
 ******************************************************************************
 ** \brief   Set CR temperature trimming register
 **
 ** \param   [in]  u8Data             temperature trimming value, only Bit[4:0]
 **                                   is valid.
 **
 ** \retval  Ok                       Set the frequency division successfully
 **                                     
 ******************************************************************************/
en_result_t Cr_SetTempTrimmingData(uint8_t u8Data)
{
    FM4_CRTRIM->MCR_RLR = CR_REG_UNLOCK_CODE;
    FM4_CRTRIM->MCR_TTRM = u8Data & 0x1F;
    FM4_CRTRIM->MCR_RLR = CR_REG_LOCK_CODE;
    return Ok;
}

/**
 ******************************************************************************
 ** \brief   Get CR temperature trimming register       
 **
 ** \retval  temperature trimming value, only Bit[4:0] is valid.                       
 **                                     
 ******************************************************************************/
uint8_t Cr_GetTempTrimmingData(void)
{
    uint8_t u8CrTempData;
    u8CrTempData = FM4_CRTRIM->MCR_TTRM & 0x1F;
    return u8CrTempData;
}

/**
 ******************************************************************************
 ** \brief   Set CR frequency trimming register
 **
 ** \param   [in]  u16Data             temperature trimming value, only Bit[9:0]
 **                                    is valid.
 **
 ** \retval  Ok                        Set CR frequency trimming successfully
 **                                     
 ******************************************************************************/
en_result_t Cr_SetFreqTrimmingData(uint16_t u16Data)
{
    FM4_CRTRIM->MCR_RLR = CR_REG_UNLOCK_CODE;
    FM4_CRTRIM->MCR_FTRM = u16Data & 0x3FF;
    FM4_CRTRIM->MCR_RLR = CR_REG_LOCK_CODE;
    return Ok;
}

/**
 ******************************************************************************
 ** \brief   Get frequency trimming register   
 **
 ** \retval  temperature trimming value, only Bit[9:0]
 **          is valid.
 **                                     
 ******************************************************************************/
uint16_t Cr_GetFreqTrimmingData(void)
{
    uint16_t u16CrFreqData;
    u16CrFreqData = FM4_CRTRIM->MCR_FTRM & 0x3FF;
    return u16CrFreqData;
}

//@} // CrGroup

#endif /* #if (defined(PDL_PERIPHERAL_CR_ACTIVE)) */
/******************************************************************************/
/* EOF (not truncated)                                                        */
/******************************************************************************/
