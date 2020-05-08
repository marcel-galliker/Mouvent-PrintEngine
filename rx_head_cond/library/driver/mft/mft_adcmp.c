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
/** \file mft_adcmp.c
 **
 ** A detailed description is available at 
 ** @link AdcmpGroup ADCMP Module description @endlink
 **
 ** History:
 **   - 2013-04-23  0.1  Chamber  First version.
 **
 ******************************************************************************/

/******************************************************************************/
/* Include files                                                              */
/******************************************************************************/
#include "mft_adcmp.h"

#if (defined(PDL_PERIPHERAL_MFT_ADCMP_ACTIVE))
/**
 ******************************************************************************
 ** \addtogroup AdcmpGroup
 ******************************************************************************/
//@{
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
 ** \brief Device dependent initialization of Mft adcmp module
 **       
 ** \param [in] pstcMftAdcmp    Pointer to Mft instance
 ** \arg        structure of Mft_Adcmp
 ** \param [in] u8Ch            Mft adcmp channel
 ** \arg        MFT_ADCMP_CH0 ~ MFT_ADCMP_CH5
 ** \param [in] pstcConfig      Pointer to Mft adcmp config
 ** \arg        structure of Mft Adcmp configuration
 ** \return Ok
 ** \return ErrorInvalidParameter
 ******************************************************************************/
en_result_t Mft_Adcmp_Init(volatile stc_mftn_adcmp_t* pstcMftAdcmp, 
                            uint8_t u8Ch, 
                            stc_mft_adcmp_config_t* pstcConfig)
{
    stc_mft_adcmp_acfs10_field_t* stcACFS10 ;
    stc_mft_adcmp_acfs32_field_t* stcACFS32 ;
    stc_mft_adcmp_acfs54_field_t* stcACFS54 ;
    
    stc_mft_adcmp_acsc_field_t*   stcACSCx ;
    stc_mft_adcmp_acsd_field_t*   stcACSDx ;
     
    if ((pstcMftAdcmp == NULL)||(pstcConfig == NULL)||(u8Ch > MFT_ADCMP_CH_MAX))
    {
        return ErrorInvalidParameter ;
    }

    // Get actual address of register list of current channel
    stcACFS10 = (stc_mft_adcmp_acfs10_field_t*)(&pstcMftAdcmp->ACFS10);
    stcACFS32 = (stc_mft_adcmp_acfs32_field_t*)(&pstcMftAdcmp->ACFS32);
    stcACFS54 = (stc_mft_adcmp_acfs54_field_t*)(&pstcMftAdcmp->ACFS54);
    
    stcACSCx = (stc_mft_adcmp_acsc_field_t*)((uint8_t*)(&pstcMftAdcmp->ACSC0) + 0x4*u8Ch);
    stcACSDx = (stc_mft_adcmp_acsd_field_t*)((uint8_t*)(&pstcMftAdcmp->ACSD0) + 0x4*u8Ch);

    // Configure parameter
    stcACSDx->DE = 0;
    stcACSDx->PE = 0;
    stcACSDx->UE = 0;
    stcACSDx->ZE = 0;
    
    pstcMftAdcmp->ACSA &= 0x0000;
	
    // Select FRT channel : FRTx -> ADCMPx
    switch(u8Ch)
    {
        case 0:
            stcACFS10->FSA0 = pstcConfig->enFrt;
            break;
        case 1:
            stcACFS10->FSA1 = pstcConfig->enFrt;
            break;
        case 2:
            stcACFS32->FSA2 = pstcConfig->enFrt;
            break;
        case 3:
            stcACFS32->FSA3 = pstcConfig->enFrt;
            break;
        case 4:
            stcACFS54->FSA4 = pstcConfig->enFrt;
            break;
        case 5:
            stcACFS54->FSA5 = pstcConfig->enFrt;
            break;
        default:
            return ErrorInvalidParameter;
    }
    
    // configure buffer function
    if(pstcConfig->enBuf > AdcmpBufFrtZeroPeak)
    {
        return ErrorInvalidParameter;
    }
    stcACSCx->BUFE = pstcConfig->enBuf;
	
    // configure start trigger output channel number
    if(pstcConfig->enTrigSel > AdcmpTrigAdc2Prio)
    {
        return ErrorInvalidParameter;
    }
    stcACSCx->ADSEL = pstcConfig->enTrigSel;
	
    // Select ADCMP running mode
    if(pstcConfig->enMode > AdcmpOffsetMode)
    {
        return ErrorInvalidParameter;
    }
    stcACSDx->AMOD = pstcConfig->enMode;
	
    // select OCU OCCP register: OCCP(x)
    if(pstcConfig->enOccpSel > AdcmpSelOccp1)
    {
        return ErrorInvalidParameter;
    }
    stcACSDx->OCUS = pstcConfig->enOccpSel;

    return Ok;
}
/**
 ******************************************************************************
 ** \brief Enable Mft Adcmp operations
 **
 ** \param [in] pstcMftAdcmp  Pointer to Mft instance
 ** \arg        structure of Mft_Adcmp
 ** \param [in] u8Ch          Mft Adcmp channel
 ** \arg        MFT_ADCMP_CH0 ~ MFT_ADCMP_CH5
 ** \param [in] pstcFunc      Mft Adcmp function
 **         
 ** \return Ok
 ** \return ErrorInvalidParameter
 ******************************************************************************/
en_result_t Mft_Adcmp_EnableOperation(volatile stc_mftn_adcmp_t *pstcMftAdcmp, 
                                      uint8_t u8Ch, 
                                      stc_mft_adcmp_func_t* pstcFunc)
{
    stc_mft_adcmp_acsd_field_t*   stcACSDx   = { 0 };
	
    // Check for NULL pointer and channel parameter
    if ((pstcMftAdcmp == NULL)||(pstcFunc == NULL)||(u8Ch > MFT_ADCMP_CH_MAX))
    {
        return ErrorInvalidParameter ;
    }  
    // Get actual address of register list of current channel
    stcACSDx = (stc_mft_adcmp_acsd_field_t*)((uint8_t*)(&pstcMftAdcmp->ACSD0) + 0x4*u8Ch);
	
    stcACSDx->PE = pstcFunc->bPeakEn;
    stcACSDx->ZE = pstcFunc->bZeroEn;
    stcACSDx->UE = pstcFunc->bUpEn;
    stcACSDx->DE = pstcFunc->bDownEn;
	
    return Ok;
}
/**
 ******************************************************************************
 ** \brief Disable Mft Adcmp operations
 **
 ** \param [in] pstcMftAdcmp  Pointer to Mft instance
 ** \arg        structure of Mft_Adcmp
 ** \param [in] u8Ch          Mft Adcmp channel
 ** \arg        MFT_ADCMP_CH0 ~ MFT_ADCMP_CH5
 ** \param [in] pstcFunc      Mft Adcmp function
 ** \arg
 ** \return Ok    
 ** \return ErrorInvalidParameter
 ******************************************************************************/
en_result_t Mft_Adcmp_DisableOperation(volatile stc_mftn_adcmp_t *pstcMftAdcmp, 
                                       uint8_t u8Ch, 
                                       stc_mft_adcmp_func_t* pstcFunc)
{
    stc_mft_adcmp_acsd_field_t*   stcACSDx   = { 0 };
	
    // Check for NULL pointer and channel parameter
    if ((pstcMftAdcmp == NULL)||(pstcFunc == NULL)||(u8Ch > MFT_ADCMP_CH_MAX))
    {
        return ErrorInvalidParameter ;
    }  
    // Get actual address of register list of current channel
    stcACSDx = (stc_mft_adcmp_acsd_field_t*)((uint8_t*)(&pstcMftAdcmp->ACSD0) + 0x4*u8Ch);
	
    stcACSDx->PE = 0;
    stcACSDx->UE = 0;
    stcACSDx->ZE = 0;
    stcACSDx->DE = 0;
	
    return Ok;
}
/**
 ******************************************************************************
 ** \brief Write compare and offset value to ACMP
 **
 ** \param [in] pstcMftAdcmp        Pointer to Mft instance
 ** \arg        structure of Mft_Adcmp
 ** \param [in] u8Ch                Channel
 ** \param [in] u16AcmpVal          Mft Adcmp channel
 ** \arg        MFT_ADCMP_CH0 ~ MFT_ADCMP_CH5
 **
 ** \return Ok    
 ** \return ErrorInvalidParameter
 ******************************************************************************/
en_result_t Mft_Adcmp_WriteAcmp(volatile stc_mftn_adcmp_t *pstcMftAdcmp, uint8_t u8Ch, 
                                uint16_t u16AcmpVal)
{
    uint16_t*   u16AdcmpAcpmReg   = { 0 };
	
    // Check for NULL pointer and channel parameter
    if ((pstcMftAdcmp == NULL)||(u8Ch > MFT_ADCMP_CH_MAX))
    {
        return ErrorInvalidParameter ;
    }
    u16AdcmpAcpmReg = (uint16_t*)((uint8_t*)(&pstcMftAdcmp->ACMP0) + 0x4*u8Ch);
    *u16AdcmpAcpmReg =  u16AcmpVal;
	
    return Ok;
}
/**
 ******************************************************************************
 ** \brief Read compare and offset value of ACMP
 **
 ** \param [in] pstcMftAdcmp Pointer to Mft instance
 ** \arg        structure of Mft_Adcmp
 ** \param [in] u8Ch         Mft Adcmp channel
 ** \arg        MFT_ADCMP_CH0 ~ MFT_ADCMP_CH5
 ** \return uint16_t  return value of register ACMP
 **
 ******************************************************************************/
uint16_t Mft_Adcmp_ReadAcmp(volatile stc_mftn_adcmp_t *pstcMftAdcmp, uint8_t u8Ch)
{
    uint16_t*   u16AdcmpAcpmReg ;
	
    // Check for NULL pointer and channel parameter
    if ((pstcMftAdcmp == NULL)||(u8Ch > MFT_ADCMP_CH_MAX))
    {
        return ErrorInvalidParameter ;
    }  
    u16AdcmpAcpmReg =  (uint16_t*)((uint8_t*)(&pstcMftAdcmp->ACMP0) + 0x4*u8Ch);

    // return data value of ACMP register
    return *u16AdcmpAcpmReg;
}

/**
 ******************************************************************************
 ** \brief Mft Adcmp fm3 compatible initialization
 **
 ** \param [in] pstcMftAdcmp       Pointer to Mft instance
 ** \arg        structure of       Mft_Adcmp
 ** \param [in] u8CoupleCh         Mft Adcmp channel
 ** \arg        MFT_ADCMP_CH10,
 **             MFT_ADCMP_CH32,
 **             MFT_ADCMP_CH54
 ** \param [in] pstcConfig         Mft Adcmp configuration parameter
 **
 ** \return Ok
 ** \return ErrorInvalidParameter
 ******************************************************************************/
en_result_t Mft_Adcmp_Fm3_Init(volatile stc_mftn_adcmp_t *pstcMftAdcmp, 
                               uint8_t u8CoupleCh,
                               stc_mft_adcmp_fm3_config_t *pstcConfig)
{
    stc_mft_adcmp_acsa_field_t*   stcACSAx ;
    stc_mft_adcmp_acsc_field_t*   stcACSCx ;
    stc_mft_adcmp_acfs10_field_t* stcACFS10 ;
    stc_mft_adcmp_acfs32_field_t* stcACFS32;
    stc_mft_adcmp_acfs54_field_t* stcACFS54 ;

    // Get actual address of register list of current channel
    stcACSAx = (stc_mft_adcmp_acsa_field_t*)((uint16_t*)(&pstcMftAdcmp->ACSA));
    stcACSCx = (stc_mft_adcmp_acsc_field_t*)((uint8_t*)(&pstcMftAdcmp->ACSC0) + 0x4*u8CoupleCh);
    stcACFS10 = (stc_mft_adcmp_acfs10_field_t*)((uint16_t*)&pstcMftAdcmp->ACFS10);
    stcACFS32 = (stc_mft_adcmp_acfs32_field_t*)((uint16_t*)&pstcMftAdcmp->ACFS32);
    stcACFS54 = (stc_mft_adcmp_acfs54_field_t*)((uint16_t*)&pstcMftAdcmp->ACFS54);

    switch(u8CoupleCh)
    {
        case MFT_ADCMP_CH10:
            stcACFS10->FSA0 = pstcConfig->enFrt;
            stcACFS10->FSA1 = pstcConfig->enFrt;
            break;
        case MFT_ADCMP_CH32:
            stcACFS32->FSA2 = pstcConfig->enFrt;
            stcACFS32->FSA3 = pstcConfig->enFrt;
            break;
        case MFT_ADCMP_CH54:
            stcACFS54->FSA4 = pstcConfig->enFrt;
            stcACFS54->FSA5 = pstcConfig->enFrt;
            break;
        default:
            return ErrorInvalidParameter;
    }

    // configure buffer transfer type
    if(pstcConfig->enBuf > AdcmpBufFrtZeroPeak)
    {
        return ErrorInvalidParameter;
    }
    stcACSCx->BUFE = pstcConfig->enBuf;

    // configure start trigger output channel number
    if(pstcConfig->enTrigSel > AdcmpTrigAdc2Prio)
    {
        return ErrorInvalidParameter;
    }
    stcACSCx->ADSEL = pstcConfig->enTrigSel;

    // configure Adcmp Fm3 trig mode
    switch(u8CoupleCh)
    {
        case MFT_ADCMP_CH10:
            stcACSAx->CE10 = 0x00;
            stcACSAx->SEL10 = pstcConfig->enMode;
            break;
        case MFT_ADCMP_CH32:
            stcACSAx->CE32 = 0x00;
            stcACSAx->SEL32 = pstcConfig->enMode;
            break;
        case MFT_ADCMP_CH54:
            stcACSAx->CE54 = 0x00;
            stcACSAx->SEL54 = pstcConfig->enMode;
            break;
        default:
            return ErrorInvalidParameter;
    }
    return Ok;
}
/**
 ******************************************************************************
 ** \brief Mft Adcmp fm3 compatible mode configuration and enable operation
 **
 ** \param [in] pstcMftAdcmp Pointer to Mft instance
 ** \arg        structure of Mft_Adcmp
 ** \param [in] u8CoupleCh   Mft Adcmp channel
 ** \arg        MFT_ADCMP_CH10,
 **             MFT_ADCMP_CH32,
 **             MFT_ADCMP_CH54
 ** \return Ok
 ** \return ErrorInvalidParameter
 ******************************************************************************/
en_result_t Mft_Adcmp_Fm3_EnableOperation(volatile stc_mftn_adcmp_t *pstcMftAdcmp, uint8_t u8CoupleCh)
{
    stc_mft_adcmp_acsa_field_t*   stcACSAx ;

    // Check for NULL pointer and channel parameter
    if ((pstcMftAdcmp == NULL)||(u8CoupleCh > MFT_ADCMP_CPCH_MAX))
    {
        return ErrorInvalidParameter ;
    }
    // Get actual address of register list of current channel
    stcACSAx = (stc_mft_adcmp_acsa_field_t*)(&pstcMftAdcmp->ACSA);
    switch(u8CoupleCh)
    {
        case MFT_ADCMP_CH10:
            stcACSAx->CE10 = 0x01;
            break;
        case MFT_ADCMP_CH32:
            stcACSAx->CE32 = 0x01;
            break;
        case MFT_ADCMP_CH54:
            stcACSAx->CE54 = 0x01;
            break;
        default:
            return ErrorInvalidParameter;
    }
    return Ok;
}
/**
 ******************************************************************************
 ** \brief Dis-configurate Mft Adcmp fm3 compatible fucntion and disable operation
 **
 ** \param [in] pstcMftAdcmp Pointer to Mft instance
 ** \arg        structure of Mft_Adcmp
 ** \param [in] u8CoupleCh   Mft Adcmp channel
 ** \arg        MFT_ADCMP_CH10,
 **             MFT_ADCMP_CH32,
 **             MFT_ADCMP_CH54
 ** \return Ok
 ** \return ErrorInvalidParameter
 ******************************************************************************/
en_result_t Mft_Adcmp_Fm3_DisableOperation(volatile stc_mftn_adcmp_t *pstcMftAdcmp, uint8_t u8CoupleCh)
{
    stc_mft_adcmp_acsa_field_t*   stcACSAx;

    // Check for NULL pointer and channel parameter
    if ((pstcMftAdcmp == NULL)||(u8CoupleCh > MFT_ADCMP_CPCH_MAX))
    {
        return ErrorInvalidParameter ;
    }
    // Get actual address of register list of current channel
    stcACSAx = (stc_mft_adcmp_acsa_field_t*)(&pstcMftAdcmp->ACSA);
    switch(u8CoupleCh)
    {
        case MFT_ADCMP_CH10:
            stcACSAx->CE10 = 0x00;
            break;
        case MFT_ADCMP_CH32:
            stcACSAx->CE32 = 0x00;
            break;
        case MFT_ADCMP_CH54:
            stcACSAx->CE54 = 0x00;
            break;
        default:
            return ErrorInvalidParameter;
    }
    return Ok;
}
/**
 ******************************************************************************
 ** \brief Mft Adcmp fm3 compatible mode, write Accp register
 **
 ** \param [in] pstcMftAdcmp Pointer to Mft instance
 ** \arg        structure of Mft_Adcmp
 ** \param [in] u8CoupleCh   Mft Adcmp channel
 ** \arg        MFT_ADCMP_CH10,
 **             MFT_ADCMP_CH32,
 **             MFT_ADCMP_CH54
 ** \param [in] u16AccpVal      Mft Adcmp configuration parameter
 ** \arg        16-bit data value
 ** \return Ok
 ** \return ErrorInvalidParameter
 ******************************************************************************/
en_result_t Mft_Adcmp_Fm3_WriteAccp(volatile stc_mftn_adcmp_t *pstcMftAdcmp, uint8_t u8CoupleCh, uint16_t u16AccpVal)
{
    uint16_t*   u16AdcmpAcpmReg;

    // Check for NULL pointer and channel parameter
    if ((pstcMftAdcmp == NULL)||(u8CoupleCh > MFT_ADCMP_CPCH_MAX))
    {
        return ErrorInvalidParameter ;
    }
    u16AdcmpAcpmReg = (uint16_t*)((uint8_t*)&pstcMftAdcmp->ACMP0 + 0x4*u8CoupleCh);
    *u16AdcmpAcpmReg =  u16AccpVal;

    return Ok;
}
/**
 ******************************************************************************
 ** \brief Mft Adcmp fm3 compatible mode, read Accp register stored value
 **
 ** \param [in] pstcMftAdcmp Pointer to Mft instance
 ** \arg        structure of Mft_Adcmp
 ** \param [in] u8CoupleCh   Mft Adcmp channel
 ** \arg        MFT_ADCMP_CH10,
 **             MFT_ADCMP_CH32,
 **             MFT_ADCMP_CH54
 ** \return uint16_t         Data stored in Accp register
 **
 ******************************************************************************/
uint16_t Mft_Adcmp_Fm3_ReadAccp(volatile stc_mftn_adcmp_t *pstcMftAdcmp, uint8_t u8CoupleCh)
{
    uint16_t*   u16AdcmpAcpmReg   = { 0 };

    // Check for NULL pointer and channel parameter
    if ((pstcMftAdcmp == NULL)||(u8CoupleCh > MFT_ADCMP_CPCH_MAX))
    {
        return ErrorInvalidParameter ;
    }
    u16AdcmpAcpmReg = (uint16_t*)((uint8_t*)&pstcMftAdcmp->ACMP0 + 0x4*u8CoupleCh);

    // return data value of ACMP register
    return *u16AdcmpAcpmReg;
}
/**
 ******************************************************************************
 ** \brief Mft Adcmp fm3 compatible mode, write Accpdn register
 **
 ** \param [in] pstcMftAdcmp Pointer to Mft instance
 ** \arg        structure of Mft_Adcmp
 ** \param [in] u8CoupleCh   Mft Adcmp channel
 ** \arg        MFT_ADCMP_CH10,
 **             MFT_ADCMP_CH32,
 **             MFT_ADCMP_CH54
 ** \param [in] u16AccpdnVal    data value of Accpdn
 **
 ** \return Ok
 ** \return ErrorInvalidParameter
 ******************************************************************************/
en_result_t Mft_Adcmp_Fm3_WriteAccpdn(volatile stc_mftn_adcmp_t *pstcMftAdcmp, uint8_t u8CoupleCh, uint16_t u16AccpdnVal)
{
    uint16_t*   u16AdcmpAcpmReg;

    // Check for NULL pointer and channel parameter
    if ((pstcMftAdcmp == NULL)||(u8CoupleCh > MFT_ADCMP_CPCH_MAX))
    {
        return ErrorInvalidParameter ;
    }
    u16AdcmpAcpmReg = (uint16_t*)((uint8_t*)&pstcMftAdcmp->ACMP1 + 0x4*u8CoupleCh);
    *u16AdcmpAcpmReg =  u16AccpdnVal;

    return Ok;
}
/**
 ******************************************************************************
 ** \brief Mft Adcmp fm3 compatible mode, read Accpdn register stored value
 **
 ** \param [in] pstcMftAdcmp Pointer to Mft instance
 ** \arg        structure of Mft_Adcmp
 ** \param [in] u8CoupleCh   Mft Adcmp channel
 ** \arg        MFT_ADCMP_CH10,
 **             MFT_ADCMP_CH32,
 **             MFT_ADCMP_CH54
 ** \return uint16_t         Data stored in Accpdn register
 **
 ******************************************************************************/
uint16_t Mft_Adcmp_Fm3_ReadAccpdn(volatile stc_mftn_adcmp_t *pstcMftAdcmp, uint8_t u8CoupleCh)
{
    uint16_t*   u16AdcmpAcpmReg   = { 0 };

    // Check for NULL pointer and channel parameter
    if ((pstcMftAdcmp == NULL)||(u8CoupleCh > MFT_ADCMP_CPCH_MAX))
    {
        return ErrorInvalidParameter ;
    }
    u16AdcmpAcpmReg = ((uint16_t*)&pstcMftAdcmp->ACMP1 + 0x4*u8CoupleCh);

    // return data value of ACMP register
    return *u16AdcmpAcpmReg;
}

//@} // AdcmpGroup

#endif

/******************************************************************************/
/* EOF (not truncated)                                                        */
/******************************************************************************/


