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
/** \file crc.c
 **
 ** A detailed description is available at 
 ** @link CrcGroup CRC Module description @endlink
 **
 ** History:
 **   - 2012-04-02  1.0  NT   First version.
 **
 ******************************************************************************/

/******************************************************************************/
/* Include files                                                              */
/******************************************************************************/
#include "crc.h"

#if (defined(PDL_PERIPHERAL_CRC_ACTIVE))

/**
 ******************************************************************************
 ** \addtogroup CrcGroup
 ******************************************************************************/
//@{

/******************************************************************************/
/* Local pre-processor symbols/macros ('#define')                             */
/******************************************************************************/

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

/******************************************************************************/
/* Function implementation - global ('extern') and local ('static')           */
/******************************************************************************/


/**
 ******************************************************************************
 ** \brief Initialisation of a CRC module.
 **
 ** \param [in]  pstcConfig    CRC module configuration 
 **
 ** \retval Ok                     Initializiation of CRC module successfully 
 **                                done.
 ** \retval ErrorInvalidParameter  pstcConfig == NULL,
 **                                parameter out of range
 ******************************************************************************/
en_result_t Crc_Init(stc_crc_config_t* pstcConfig)
{
    en_result_t enResult;
    stc_crc_crccr_field_t stcCRCCR; /* local preset of CRCCR */

    /* Check for valid pointers */
    if (NULL == pstcConfig)
    {
        enResult = ErrorInvalidParameter;
    }
    else
    {
        enResult = Ok;
        /* Check CRC mode select and setting */
        switch (pstcConfig->enMode)
        {
            case Crc16:
                /* Set CRC16 */
                stcCRCCR.CRC32 = FALSE;
                /* Mask the initial value for 16 bits */
                pstcConfig->u32CrcInitValue &= 0x0000FFFF;
                break;
            case Crc32:
                /* Set CRC32 */
                stcCRCCR.CRC32 = TRUE;
                break;
            default:
                enResult = ErrorInvalidParameter;
                break;
        }

        if (Ok == enResult)
        {
            /* XOR is executed finaly or not */
            if (TRUE == pstcConfig->bFinalXor)
            {
                stcCRCCR.FXOR = TRUE;
            }
            else
            {
                stcCRCCR.FXOR = FALSE;
            }
            /* Bit order for CRC result */
            if (TRUE == pstcConfig->bResultLsbFirst)
            {
                stcCRCCR.CRCLSF = TRUE;
            }
            else
            {
                stcCRCCR.CRCLSF = FALSE;
            }
            /* Byte order for CRC result */
            if (TRUE == pstcConfig->bResultLittleEndian)
            {
                stcCRCCR.CRCLTE = TRUE;
            }
            else
            {
                stcCRCCR.CRCLTE = FALSE;
            }
            /* Bit order for DATA */
            if (TRUE == pstcConfig->bDataLsbFirst)
            {
                stcCRCCR.LSBFST = TRUE;
            }
            else
            {
                stcCRCCR.LSBFST = FALSE;
            }
            /* Byte order for DATA */
            if (TRUE == pstcConfig->bDataLittleEndian)
            {
                stcCRCCR.LTLEND = TRUE;
            }
            else
            {
                stcCRCCR.LTLEND = FALSE;
            }
            /* Initialize CRC configuration */
            stcCRCCR.INIT = TRUE;

            /* Set the initial value */
            FM4_CRC->CRCINIT = pstcConfig->u32CrcInitValue;
            /* Write setting to CRC control register */
            FM4_CRC->CRCCR_f = stcCRCCR;
        }
    }

    return (enResult);
} /* Crc_Init */


/**
 ******************************************************************************
 ** \brief De-Initialisation of a CRC module.
 **
 ******************************************************************************/
void Crc_DeInit(void)
{
    /* clear hardware */
    FM4_CRC->CRCIN = 0;
    FM4_CRC->CRCCR = 0;

} /* Crc_DeInit */

/**
 ******************************************************************************
 ** \brief Push 8-bit integer data to a CRC module with if no DMA is used.
 **
 ** \param [in]  u8DataToPush  8-Bit data to be pushed to CRC
 **
 ******************************************************************************/
void Crc_Push8(uint8_t u8DataToPush)
{
    /* Caluculate CRC (Push 8bit data) */
    FM4_CRC->CRCINLL = u8DataToPush;

} /* Crc_Push8 */

/**
 ******************************************************************************
 ** \brief Push 16-bit integer data to a CRC module with if no DMA is used.
 **
 ** \note Be careful with the endianess. Byte swapping might have to be
 **       performed before pushing 16-bit data.
 **
 ** \param [in]  u16DataToPush  16-Bit data to be pushed to CRC
 **
 ******************************************************************************/
void Crc_Push16(uint16_t u16DataToPush)
{
    /* Caluculate CRC (Push 16bit data) */
    FM4_CRC->CRCINL = u16DataToPush;

} /* Crc_Push16 */

/**
 ******************************************************************************
 ** \brief Push 32-bit integer data to a CRC module with if no DMA is used.
 **
 ** \param [in]  u32DataToPush  32-Bit data to be pushed to CRC
 **
 ******************************************************************************/
void Crc_Push32(uint32_t u32DataToPush)
{
    /* Caluculate CRC (Push 32bit data) */
    FM4_CRC->CRCIN = u32DataToPush;

} /* Crc_Push32 */

/**
 ******************************************************************************
 ** \brief Read CRC result register
 **
 ** \note This function returns a 32-bit value regardless of a valid
 **       pointer to the CRC instance anyhow.
 **
 ** \retval value of CRC result
 **
 ******************************************************************************/
uint32_t Crc_ReadResult(void)
{
    /* Return value of CRC result register */
    return (FM4_CRC->CRCR);
} /* Crc_ReadResult */


//@} // CrcGroup

#endif /* #if (defined(PDL_PERIPHERAL_CRC_ACTIVE)) */
/******************************************************************************/
/* EOF (not truncated)                                                        */
/******************************************************************************/
