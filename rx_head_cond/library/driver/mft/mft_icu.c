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
/** \file mft_icu.c
 **
 ** A detailed description is available at 
 ** @link IcuGroup ICU Module description @endlink
 **
 ** History:
 **   - 2013-04-23  1.0  Chamber      First version.
 **   - 2013-10-31  1.1  Edison Zhang Change the "MftIcuGetInternDataPtr" 
 **                                   location to avoid warning when it is unused.
 **   - 2014-01-15  1.2  Edison Zhang Correct the instance judgement in MftIcu_InitIrq().
 **
 ******************************************************************************/

/******************************************************************************/
/* Include files                                                              */
/******************************************************************************/
#include "mft_icu.h"

#if (defined(PDL_PERIPHERAL_MFT_ICU_ACTIVE))
   
/**
 ******************************************************************************
 ** \addtogroup IcuGroup
 ******************************************************************************/
//@{
   
/******************************************************************************/
/* Local pre-processor symbols/macros ('#define')                             */
/******************************************************************************/

/******************************************************************************/
/* Global variable definitions (declared in header file with 'extern')        */
/******************************************************************************/
stc_mft_icu_instance_data_t m_astcMftIcuInstanceDataLut[MFT_ICU_INSTANCE_COUNT] =
{
#if (PDL_PERIPHERAL_ENABLE_MFT0_ICU == PDL_ON)
    { 
        &MFT0_ICU,    // pstcInstance
        NULL          // stcInternData (not initialized yet)
    },
#endif
#if (PDL_PERIPHERAL_ENABLE_MFT1_ICU == PDL_ON)
    { 
        &MFT1_ICU,    // pstcInstance
        NULL          // stcInternData (not initialized yet)
    },
#endif
#if (PDL_PERIPHERAL_ENABLE_MFT2_ICU == PDL_ON)
    { 
        &MFT2_ICU,    // pstcInstance
        NULL          // stcInternData (not initialized yet)
    },
#endif
};

/******************************************************************************/
/* Local type definitions ('typedef')                                         */
/******************************************************************************/

/******************************************************************************/
/* Local function prototypes ('static')                                       */
/******************************************************************************/
#if (PDL_INTERRUPT_ENABLE_MFT0_ICU == PDL_ON) || \
    (PDL_INTERRUPT_ENABLE_MFT1_ICU == PDL_ON) || \
    (PDL_INTERRUPT_ENABLE_MFT2_ICU == PDL_ON)
static stc_mft_icu_intern_data_t* MftIcuGetInternDataPtr(volatile stc_mftn_icu_t *pstcMftIcu);
static en_result_t MftIcu_InitIrq( volatile stc_mftn_icu_t* pstcMftIcu );
static en_result_t MftIcu_DeInitIrq( volatile stc_mftn_icu_t* pstcMftIcu );
#endif
/******************************************************************************/
/* Local variable definitions ('static')                                      */
/******************************************************************************/

/******************************************************************************/
/* Function implementation - global ('extern') and local ('static')           */
/******************************************************************************/

#if (PDL_INTERRUPT_ENABLE_MFT0_ICU == PDL_ON) || \
    (PDL_INTERRUPT_ENABLE_MFT1_ICU == PDL_ON) || \
    (PDL_INTERRUPT_ENABLE_MFT2_ICU == PDL_ON)
/**
 *****************************************************************************
 ** \brief Return the internal data for a certain MFT_ICU instance.
 **
 ** \param pstcMftIcu Pointer to MftIcu instance
 **
 ** \return Pointer to internal data or NULL if instance is not enabled (or not known)
 **
 *****************************************************************************/
static stc_mft_icu_intern_data_t* MftIcuGetInternDataPtr(volatile stc_mftn_icu_t *pstcMftIcu) 
{
    uint32_t u32Instance;
   
    for (u32Instance = 0u; u32Instance < MFT_ICU_INSTANCE_COUNT; u32Instance++)
    {
        if (pstcMftIcu == m_astcMftIcuInstanceDataLut[u32Instance].pstcInstance)
        {
            return &m_astcMftIcuInstanceDataLut[u32Instance].stcInternData;
        }
    }

    return NULL;
}		
		
/**
 ******************************************************************************
 ** \brief Device Interrupt handler
 **
 ** \param [in] pstcMftIcu            Pointer to ICU instance
 ** \param [in] pstcMftIcuInternData  Pointer to intern data
 **
 ** \return none
 ******************************************************************************/
void Mft_Icu_IrqHandler( volatile stc_mftn_icu_t* pstcMftIcu, 
                         stc_mft_icu_intern_data_t* pstcMftIcuInternData)
{
    // ICU interrupt 0
    if(TRUE == pstcMftIcu->ICSA10_f.ICP0)
    {
        pstcMftIcu->ICSA10_f.ICP0 = 0u;
        if (pstcMftIcuInternData->pfnIcu0Callback != NULL)
        {
             pstcMftIcuInternData->pfnIcu0Callback();
        }
    }
    // ICU interrupt 1
    if(TRUE == pstcMftIcu->ICSA10_f.ICP1)
    {
        pstcMftIcu->ICSA10_f.ICP1 = 0u;
        if (pstcMftIcuInternData->pfnIcu1Callback != NULL)
        {
             pstcMftIcuInternData->pfnIcu1Callback();
        }
    }
    // ICU interrupt 2
    if(TRUE == pstcMftIcu->ICSA32_f.ICP0)
    {
        pstcMftIcu->ICSA32_f.ICP0 = 0u;
        if (pstcMftIcuInternData->pfnIcu2Callback != NULL)
        {
             pstcMftIcuInternData->pfnIcu2Callback();
        }
    }
    // ICU interrupt 3
    if(TRUE == pstcMftIcu->ICSA32_f.ICP1)
    {
        pstcMftIcu->ICSA32_f.ICP1 = 0u;
        if (pstcMftIcuInternData->pfnIcu3Callback != NULL)
        {
             pstcMftIcuInternData->pfnIcu3Callback();
        }
    }
}

/**
 ******************************************************************************
 ** \brief Device dependent initialization of interrupts according CMSIS with
 **        level defined in l3.h
 **
 ** \param  pstcMftIcu Pointer to ICU instance
 **
 ** \return Ok    Successful initialization
 **
 ******************************************************************************/
static en_result_t MftIcu_InitIrq( volatile stc_mftn_icu_t* pstcMftIcu ) 
{
#if (PDL_INTERRUPT_ENABLE_MFT0_ICU == PDL_ON)
    if (pstcMftIcu == (volatile stc_mftn_icu_t*)(&MFT0_ICU))
    {
        NVIC_ClearPendingIRQ(ICU0_IRQn);
        NVIC_EnableIRQ(ICU0_IRQn);
        NVIC_SetPriority(ICU0_IRQn, PDL_IRQ_LEVEL_MFT0_ICU);   
    }
#endif
#if (PDL_INTERRUPT_ENABLE_MFT1_ICU == PDL_ON)
    if (pstcMftIcu == (volatile stc_mftn_icu_t*)(&MFT1_ICU))
    {
        NVIC_ClearPendingIRQ(ICU1_IRQn);
        NVIC_EnableIRQ(ICU1_IRQn);
        NVIC_SetPriority(ICU1_IRQn, PDL_IRQ_LEVEL_MFT1_ICU);      
    }
#endif
#if (PDL_INTERRUPT_ENABLE_MFT2_ICU == PDL_ON)
    if (pstcMftIcu == (volatile stc_mftn_icu_t*)(&MFT2_ICU))
    {
        NVIC_ClearPendingIRQ(ICU2_IRQn);
        NVIC_EnableIRQ(ICU2_IRQn);
        NVIC_SetPriority(ICU2_IRQn, PDL_IRQ_LEVEL_MFT2_ICU);      
    }  
#endif
    return Ok;
}

/**
 ******************************************************************************
 ** \brief Device dependent de-initialization of interrupts according CMSIS 
 **
 ** \param [in] pstcMftIcu Pointer to ICU instance
 **
 ** \return Ok    Successful initialization
 **
 ******************************************************************************/
static en_result_t MftIcu_DeInitIrq( volatile stc_mftn_icu_t* pstcMftIcu ) 
{
    if((pstcMftIcu->ICSA10_f.ICE0 == 0u) &&
       (pstcMftIcu->ICSA10_f.ICE1 == 0u) &&
       (pstcMftIcu->ICSA32_f.ICE0 == 0u) &&
       (pstcMftIcu->ICSA32_f.ICE1 == 0u) )
    {  
    #if (PDL_INTERRUPT_ENABLE_MFT0_ICU == PDL_ON)
        if (pstcMftIcu == (volatile stc_mftn_icu_t*)(&MFT0_ICU))
        {
            NVIC_ClearPendingIRQ(ICU0_IRQn);
            NVIC_DisableIRQ(ICU0_IRQn);
            NVIC_SetPriority(ICU0_IRQn, PDL_DEFAULT_INTERRUPT_LEVEL);	 
        }
    #endif
    #if (PDL_INTERRUPT_ENABLE_MFT1_ICU == PDL_ON)
        if (pstcMftIcu == (volatile stc_mftn_icu_t*)(&MFT1_ICU))
        {
            NVIC_ClearPendingIRQ(ICU1_IRQn);
            NVIC_DisableIRQ(ICU1_IRQn);
            NVIC_SetPriority(ICU1_IRQn, PDL_DEFAULT_INTERRUPT_LEVEL);		
        }
    #endif
    #if (PDL_INTERRUPT_ENABLE_MFT2_ICU == PDL_ON)
        if (pstcMftIcu == (volatile stc_mftn_icu_t*)(&MFT2_ICU))
        {
            NVIC_ClearPendingIRQ(ICU2_IRQn);
            NVIC_DisableIRQ(ICU2_IRQn);
            NVIC_SetPriority(ICU2_IRQn, PDL_DEFAULT_INTERRUPT_LEVEL);		
        }  
    #endif
    }
    return Ok;
}

#endif

/**
 ******************************************************************************
 ** \brief  Select FRTx channel to connect to ICUx
 **
 ** \param [in] pstcMftIcu Pointer to Mft instance
 ** \arg        structure of Mft_Icu
 ** \param [in] u8Ch       Mft Icu channel
 ** \arg        MFT_ICU_CH0 ~ MFT_ICU_CH3
 ** \param [in] enFrt      Frt channel number
 ** \arg        Frt0ToIcu,
 **             Frt1ToIcu,
 **             Frt2ToIcu,
 **             IcuFrtToExt0,
 **             IcuFrtToExt1
 ** \return Ok    
 ** \return ErrorInvalidParameter
 ******************************************************************************/
en_result_t Mft_Icu_SelFrt( volatile stc_mftn_icu_t* pstcMftIcu, uint8_t u8Ch, en_mft_icu_frt_t enFrt)
{
    stc_mft_icu_icfs10_field_t* stcICFS10  = { 0u };
    stc_mft_icu_icfs32_field_t* stcICFS32  = { 0u };

    // Check for NULL pointer and channel parameter
    boolean_t bAvoidSideEffects  = ((NULL == pstcMftIcu) ? 1u : 0u);
              bAvoidSideEffects |= ((u8Ch > MFT_ICU_CHx_MAX) ? 1u : 0u);
    if (TRUE == bAvoidSideEffects)
    {
        return ErrorInvalidParameter ;
    }

    // Get actual address of register list of current channel
    stcICFS10 = (stc_mft_icu_icfs10_field_t*)(&(pstcMftIcu->ICFS10));
    stcICFS32 = (stc_mft_icu_icfs32_field_t*)(&(pstcMftIcu->ICFS32));
    
    // FRT channel select: FRTx -> ICUx
    if(enFrt > IcuFrtToExt1)
    {
        return ErrorInvalidParameter;
    }
    // configure the Frt channel to connect to Icu
    switch(u8Ch)
    {
        // ICU channel 0
        case MFT_ICU_CH0: 
            stcICFS10->FSI00 = (((uint8_t)enFrt >> 0u) & 0x01u);
            stcICFS10->FSI01 = (((uint8_t)enFrt >> 1u) & 0x01u);
            stcICFS10->FSI02 = (((uint8_t)enFrt >> 2u) & 0x01u);
            stcICFS10->FSI03 = (((uint8_t)enFrt >> 3u) & 0x01u);
            break;
        // ICU channel 1
        case MFT_ICU_CH1:
            stcICFS10->FSI10 = (((uint8_t)enFrt >> 0u) & 0x01u);
            stcICFS10->FSI11 = (((uint8_t)enFrt >> 1u) & 0x01u);
            stcICFS10->FSI12 = (((uint8_t)enFrt >> 2u) & 0x01u);
            stcICFS10->FSI13 = (((uint8_t)enFrt >> 3u) & 0x01u);
            break;
        // ICU channel 2
        case MFT_ICU_CH2:
            stcICFS32->FSI00 = (((uint8_t)enFrt >> 0u) & 0x01u);
            stcICFS32->FSI01 = (((uint8_t)enFrt >> 1u) & 0x01u);
            stcICFS32->FSI02 = (((uint8_t)enFrt >> 2u) & 0x01u);
            stcICFS32->FSI03 = (((uint8_t)enFrt >> 3u) & 0x01u);
            break;
        // ICU channel 3
        case MFT_ICU_CH3:
            stcICFS32->FSI10 = (((uint8_t)enFrt >> 0u) & 0x01u);
            stcICFS32->FSI11 = (((uint8_t)enFrt >> 1u) & 0x01u);
            stcICFS32->FSI12 = (((uint8_t)enFrt >> 2u) & 0x01u);
            stcICFS32->FSI13 = (((uint8_t)enFrt >> 3u) & 0x01u);
            break;
        default:
            return ErrorInvalidParameter;
    }
    
    return Ok;
}

/**
 ******************************************************************************
 ** \brief  Configure ICU module detection mode(different edge)
 **
 ** \param  [in] pstcMftIcu Pointer to Mft instance
 ** \arg        structure of Mft_Icu
 ** \param  [in] u8Ch       Mft Icu channel
 ** \arg        MFT_ICU_CH0 ~ MFT_ICU_CH3
 ** \param  [in] enMode     Icu detect mode
 ** \arg        IcuDisable,
 **             IcuRisingDetect,
 **             IcuFallingDetect,
 **             IcuBothDetect
 ** \return Ok    
 ** \return ErrorInvalidParameter
 ******************************************************************************/

en_result_t Mft_Icu_ConfigDetectMode( volatile stc_mftn_icu_t* pstcMftIcu, uint8_t u8Ch, 
                                      en_mft_icu_mode_t enMode)
{
    // Check for NULL pointer and channel parameter
    boolean_t bAvoidSideEffects  = ((NULL == pstcMftIcu) ? 1u : 0u);
              bAvoidSideEffects |= ((u8Ch > MFT_ICU_CHx_MAX) ? 1u : 0u);
    if (bAvoidSideEffects)
    {
        return ErrorInvalidParameter ;
    }

    // Check configuration of Mft Icu mode
    if(enMode > IcuBothDetect)
    {
        return ErrorInvalidParameter;
    }
    // Configure Icu detection mode
    switch (u8Ch)
    {
        // ICU channel 0
        case MFT_ICU_CH0:
            pstcMftIcu->ICSA10_f.EG00 = (((uint8_t)enMode >> 0u) & 0x01u);
            pstcMftIcu->ICSA10_f.EG01 = (((uint8_t)enMode >> 1u) & 0x01u);
            break;
        // ICU channel 1
        case MFT_ICU_CH1:
            pstcMftIcu->ICSA10_f.EG10 = (((uint8_t)enMode >> 0u) & 0x01u);
            pstcMftIcu->ICSA10_f.EG11 = (((uint8_t)enMode >> 1u) & 0x01u);
            break;
        // ICU channel 2
        case MFT_ICU_CH2:
            pstcMftIcu->ICSA32_f.EG00 = (((uint8_t)enMode >> 0u) & 0x01u);
            pstcMftIcu->ICSA32_f.EG01 = (((uint8_t)enMode >> 1u) & 0x01u);
            break;
        // ICU channel 3
        case MFT_ICU_CH3:
            pstcMftIcu->ICSA32_f.EG10 = (((uint8_t)enMode >> 0u) & 0x01u);
            pstcMftIcu->ICSA32_f.EG11 = (((uint8_t)enMode >> 1u) & 0x01u);
            break;
        // ICU channel number error
        default:
            return ErrorInvalidParameter;
    }
    return Ok;
}

#if (PDL_INTERRUPT_ENABLE_MFT0_ICU == PDL_ON) || \
    (PDL_INTERRUPT_ENABLE_MFT1_ICU == PDL_ON) || \
    (PDL_INTERRUPT_ENABLE_MFT2_ICU == PDL_ON)
/**
 ******************************************************************************
 ** \brief  Enable Mft Icu interrupt
 **
 ** \param  [in] pstcMftIcu   Pointer to Mft instance
 ** \arg         structure of Mft_Icu
 ** \param  [in] u8Ch         Mft Icu channel
 ** \arg        MFT_ICU_CH0 ~ MFT_ICU_CH3
 ** \param  [in] pfnCallback  register callback function
 ** \arg        user interrupt application function
 ** \return Ok    
 ** \return ErrorInvalidParameter
 ******************************************************************************/

en_result_t Mft_Icu_EnableInt( volatile stc_mftn_icu_t*pstcMftIcu, uint8_t u8Ch, 
                               func_ptr_t pfnCallback)
{
    // Pointer to internal data
    stc_mft_icu_intern_data_t* pstcMftIcuInternData ;  
    volatile stc_mftn_icu_t* pstcOrgMftIcu = pstcMftIcu;

    // Check for NULL pointer and channel parameter
    boolean_t bAvoidSideEffects  = ((NULL == pstcMftIcu) ? 1u : 0u);
    bAvoidSideEffects |= ((u8Ch > MFT_ICU_CHx_MAX) ? 1u : 0u);
    if (bAvoidSideEffects)
    {
        return ErrorInvalidParameter ;
    }

    // Get pointer to internal data structure
    pstcMftIcuInternData = MftIcuGetInternDataPtr( pstcOrgMftIcu );
    
    switch (u8Ch)
    {
        // ICU channel 0
        case MFT_ICU_CH0:
            pstcMftIcuInternData->pfnIcu0Callback = pfnCallback;
            pstcMftIcu->ICSA10_f.ICE0 = 1u;
            break;
        // ICU channel 1
        case MFT_ICU_CH1:
            pstcMftIcuInternData->pfnIcu1Callback = pfnCallback;
            pstcMftIcu->ICSA10_f.ICE1 = 1u;
            break;
        // ICU channel 2
        case MFT_ICU_CH2:
            pstcMftIcuInternData->pfnIcu2Callback = pfnCallback;
            pstcMftIcu->ICSA32_f.ICE0 = 1u;
            break;
        // ICU channel 3
        case MFT_ICU_CH3:
            pstcMftIcuInternData->pfnIcu3Callback = pfnCallback;
            pstcMftIcu->ICSA32_f.ICE1 = 1u;
            break;
        // ICU channel number error
        default:
            return ErrorInvalidParameter;
    }

    // initialize interrupt
    MftIcu_InitIrq(pstcOrgMftIcu);
    return Ok;
}

/**
 ******************************************************************************
 ** \brief  Disable Mft Icu interrupt and release callback function
 **
 ** \param  [in] pstcMftIcu   Pointer to Mft instance
 ** \arg         structure of Mft_Icu
 ** \param  [in] u8Ch         Mft Icu channel
 ** \arg        MFT_ICU_CH0 ~ MFT_ICU_CH3
 **
 ** \return Ok    
 ** \return ErrorInvalidParameter
 ******************************************************************************/

en_result_t Mft_Icu_DisableInt( volatile stc_mftn_icu_t*pstcMftIcu, uint8_t u8Ch)
{
    // Pointer to internal data
    stc_mft_icu_intern_data_t* pstcMftIcuInternData ;  
    volatile stc_mftn_icu_t* pstcOrgMftIcu = pstcMftIcu;

    // Check for NULL pointer and channel parameter
    boolean_t bAvoidSideEffects  = ((NULL == pstcMftIcu) ? 1u : 0u);
              bAvoidSideEffects |= ((u8Ch > MFT_ICU_CHx_MAX) ? 1u : 0u);
    if (bAvoidSideEffects)
    {
        return ErrorInvalidParameter ;
    }

    // Get pointer to internal data structure
    pstcMftIcuInternData = MftIcuGetInternDataPtr( pstcOrgMftIcu );
    // Dis-register callback function and disable interrupt operation
    switch (u8Ch)
    {
        // ICU channel 0
        case MFT_ICU_CH0:
            pstcMftIcuInternData->pfnIcu0Callback = NULL;
            pstcMftIcu->ICSA10_f.ICE0 = 0u;
            break;
        // ICU channel 1
        case MFT_ICU_CH1:
            pstcMftIcuInternData->pfnIcu1Callback = NULL;
            pstcMftIcu->ICSA10_f.ICE1 = 0u;
            break;
        // ICU channel 2
        case MFT_ICU_CH2:
            pstcMftIcuInternData->pfnIcu2Callback = NULL;
            pstcMftIcu->ICSA32_f.ICE0 = 0u;
            break;
        // ICU channel 3
        case MFT_ICU_CH3:
            pstcMftIcuInternData->pfnIcu3Callback = NULL;
            pstcMftIcu->ICSA32_f.ICE1 = 0u;
            break;
        // ICU channel number error
        default:
            return ErrorInvalidParameter;
    }
   
    MftIcu_DeInitIrq(pstcOrgMftIcu);
    return Ok;
}

#endif

/**
 ******************************************************************************
 ** \brief  Get interrupt flag
 **
 ** \param  [in] pstcMftIcu    Pointer to Mft instance
 ** \arg        structure of Mft_Icu
 ** \param  [in] u8Ch          Mft Icu channel
 ** \arg        MFT_ICU_CH0 ~ MFT_ICU_CH3
 **
 ** \return en_int_flag_t
 ** \arg        interrupt flag
 ******************************************************************************/

en_int_flag_t Mft_Icu_GetIntFlag(volatile stc_mftn_icu_t *pstcMftIcu, uint8_t u8Ch)
{
    en_int_flag_t enIntFlagBuf;

    // Read interrupt flag
    switch (u8Ch)
    {
        // ICU channel 0
        case MFT_ICU_CH0:
            enIntFlagBuf = (en_int_flag_t)(pstcMftIcu->ICSA10_f.ICP0);
            break;
        // ICU channel 1
        case MFT_ICU_CH1:
            enIntFlagBuf = (en_int_flag_t)(pstcMftIcu->ICSA10_f.ICP1);
            break;
        // ICU channel 2
        case MFT_ICU_CH2:
            enIntFlagBuf = (en_int_flag_t)(pstcMftIcu->ICSA32_f.ICP0);
            break;
        // ICU channel 3
        case MFT_ICU_CH3:
            enIntFlagBuf = (en_int_flag_t)(pstcMftIcu->ICSA32_f.ICP1);
            break;
        // ICU channel number error
        default:
            break;
    }
    // return interrupt flag
    return (en_int_flag_t)enIntFlagBuf;
}
/**
 ******************************************************************************
 ** \brief  Clear interrupt flag
 **
 ** \param  [in] pstcMftIcu    Pointer to Mft instance
 ** \arg        structure of Mft_Icu
 ** \param  [in] u8Ch          Mft Icu channel
 ** \arg        MFT_ICU_CH0 ~ MFT_ICU_CH3
 **
 ** \return Ok
 ** \return ErrorInvalidParameter
 ******************************************************************************/

en_result_t Mft_Icu_ClrIntFlag( volatile stc_mftn_icu_t *pstcMftIcu, uint8_t u8Ch)
{
    // Check for NULL pointer and channel parameter
    boolean_t bAvoidSideEffects  = ((NULL == pstcMftIcu) ? 1u : 0u);
              bAvoidSideEffects |= ((u8Ch > MFT_ICU_CHx_MAX) ? 1u : 0u);
    if (bAvoidSideEffects)
    {
        return ErrorInvalidParameter ;
    }

	// Clear interrupt flag
    switch (u8Ch)
    {
        // ICU channel 0
        case MFT_ICU_CH0:
            pstcMftIcu->ICSA10_f.ICP0 = 0u;
            break;
        // ICU channel 1
        case MFT_ICU_CH1:
            pstcMftIcu->ICSA10_f.ICP1 = 0u;
            break;
        // ICU channel 2
        case MFT_ICU_CH2:
            pstcMftIcu->ICSA32_f.ICP0 = 0u;
            break;
        // ICU channel 3
        case MFT_ICU_CH3:
            pstcMftIcu->ICSA32_f.ICP1 = 0u;
            break;
        // ICU channel number error
        default:
            return ErrorInvalidParameter;
    }
    
    return Ok;
}
/**
 ******************************************************************************
 ** \brief  Get the latest captured edge type
 **
 ** \param  [in] pstcMftIcu    Pointer to Mft instance
 ** \arg        structure of Mft_Icu
 ** \param  [in] u8Ch          Mft Icu channel
 ** \arg        MFT_ICU_CH0 ~ MFT_ICU_CH3
 **
 ** \return en_mft_icu_edge_t  detected edge type
 ******************************************************************************/
en_mft_icu_edge_t Mft_Icu_GetLastEdge( volatile stc_mftn_icu_t *pstcMftIcu, uint8_t u8Ch)
{
    en_mft_icu_edge_t enEdgeTypeBuf;

    // Read last Edge type
    switch (u8Ch)
    {
        // ICU channel 0
        case MFT_ICU_CH0:
            enEdgeTypeBuf = (en_mft_icu_edge_t)(pstcMftIcu->ICSB10_f.IEI0);
            break;
        // ICU channel 1
        case MFT_ICU_CH1:
            enEdgeTypeBuf = (en_mft_icu_edge_t)(pstcMftIcu->ICSB10_f.IEI1);
            break;
        // ICU channel 2
        case MFT_ICU_CH2:
            enEdgeTypeBuf = (en_mft_icu_edge_t)(pstcMftIcu->ICSB32_f.IEI0);
            break;
        // ICU channel 3
        case MFT_ICU_CH3:
            enEdgeTypeBuf = (en_mft_icu_edge_t)(pstcMftIcu->ICSB32_f.IEI1);
            break;
        // ICU channel number error
        default:
            break;
    }
    // return last edge type
    return (en_mft_icu_edge_t)enEdgeTypeBuf;
}
/**
 ******************************************************************************
 ** \brief  Readout captured data value
 **
 ** \param  [in] pstcMftIcu    Pointer to Mft instance
 ** \arg        structure of Mft_Icu
 ** \param  [in] u8Ch          Mft Icu channel
 ** \arg        MFT_ICU_CH0 ~ MFT_ICU_CH3
 **
 ** \return uint16_t   captured data value
 ******************************************************************************/
uint16_t Mft_Icu_GetCaptureData(volatile stc_mftn_icu_t *pstcMftIcu, uint8_t u8Ch)
{
    uint16_t u16CaptureVal;
    // Check for NULL pointer and channel parameter
    boolean_t bAvoidSideEffects  = ((NULL == pstcMftIcu) ? 1u : 0u);
              bAvoidSideEffects |= ((u8Ch > MFT_ICU_CHx_MAX) ? 1u : 0u);
    if (bAvoidSideEffects)
    {
        return ErrorInvalidParameter ;
    }
    // Read captured value
    switch (u8Ch)
    {
        // ICU channel 0
        case MFT_ICU_CH0:
            u16CaptureVal = (uint16_t)(pstcMftIcu->ICCP0);
            break;
        // ICU channel 1
        case MFT_ICU_CH1:
            u16CaptureVal = (uint16_t)(pstcMftIcu->ICCP1);
            break;
        // ICU channel 2
        case MFT_ICU_CH2:
            u16CaptureVal = (uint16_t)(pstcMftIcu->ICCP2);
            break;
        // ICU channel 3
        case MFT_ICU_CH3:
            u16CaptureVal = (uint16_t)(pstcMftIcu->ICCP3);
            break;
        // ICU channel number error
        default:
            return ErrorInvalidParameter;
    }
    
    return (uint16_t)u16CaptureVal;
}

//@} IcuGroup

#endif // #if (defined(PDL_PERIPHERAL_MFT_ICU_ACTIVE))

/******************************************************************************/
/* EOF (not truncated)                                                        */
/******************************************************************************/
