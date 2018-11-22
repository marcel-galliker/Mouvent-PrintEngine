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
/** \file mft_ocu.c
 **
 ** A detailed description is available at 
 ** @link OcuGroup OCU Module description @endlink
 **  
 ** History:
 **   - 2013-03-21  0.1  Frank  First version.
 **   - 2013-08-20  0.2  Edison Add a structure for OCSE register setting
 **   - 2014-01-15  0.3  Edison Correct typo error
 **
 *****************************************************************************/


/******************************************************************************/
/* Include files                                                              */
/******************************************************************************/
#include "mft_ocu.h"

#if (defined(PDL_PERIPHERAL_MFT_OCU_ACTIVE))

/**
 ******************************************************************************
 ** \addtogroup OcuGroup
 ******************************************************************************/
//@{

/******************************************************************************/
/* Local pre-processor symbols/macros ('#define')                             */
/******************************************************************************/
///  Macro to return the number of enabled MFT instances
#define OCU_INSTANCE_COUNT (uint32_t)(sizeof(m_astcMftOcuInstanceDataLut) \
                                      / sizeof(m_astcMftOcuInstanceDataLut[0]))

/******************************************************************************/
/* Global variable definitions (declared in header file with 'extern')        */
/******************************************************************************/
/// Look-up table for all enabled FRT of MFT  instances and their internal data
stc_mft_ocu_instance_data_t m_astcMftOcuInstanceDataLut[] =
{
#if (PDL_PERIPHERAL_ENABLE_MFT0_OCU == PDL_ON)
    {
        &MFT0_OCU,  // pstcInstance
        NULL    // stcInternData (not initialized yet)
    },
#endif
#if (PDL_PERIPHERAL_ENABLE_MFT1_OCU == PDL_ON)
    {
        &MFT1_OCU,  // pstcInstance
        NULL    // stcInternData (not initialized yet)
    },
#endif
#if (PDL_PERIPHERAL_ENABLE_MFT2_OCU == PDL_ON)
    {
        &MFT2_OCU,  // pstcInstance
        NULL    // stcInternData (not initialized yet)
    },
#endif 
};

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
 *****************************************************************************
 ** \brief Return the internal data for a certain OCU instance.
 **
 ** \param pstcOcu Pointer to OCU instance
 **
 ** \return Pointer to internal data or NULL if instance is not enabled
 **         (or not known)
 **
 *****************************************************************************/
static stc_mft_ocu_intern_data_t* MftGetInternDataPtr(volatile stc_mftn_ocu_t* pstcOcu)
{
    uint32_t u32Instance;

    for (u32Instance = 0; u32Instance < OCU_INSTANCE_COUNT; u32Instance++)
    {
        if (pstcOcu == m_astcMftOcuInstanceDataLut[u32Instance].pstcInstance)
        {
            return &m_astcMftOcuInstanceDataLut[u32Instance].stcInternData;
        }
    }

    return NULL;
}

#if (PDL_INTERRUPT_ENABLE_MFT0_OCU == PDL_ON) || \
    (PDL_INTERRUPT_ENABLE_MFT1_OCU == PDL_ON) || \
    (PDL_INTERRUPT_ENABLE_MFT2_OCU == PDL_ON)  
/*!
 ******************************************************************************
 ** \brief  OCU module interrupt handler
 **
 ** \param [in] pstcMft MFT register definition
 ** \arg    structure of MFT register
 ** \param [in]  pstcMftOcuInternData  callback function of OCU
 ** \arg    stc_mft_ocu_intern_data_t
 **
 ** \retval none
 **
 *****************************************************************************
*/
void Mft_Ocu_IrqHandler( volatile stc_mftn_ocu_t*pstcMft,
		         stc_mft_ocu_intern_data_t* pstcMftOcuInternData)
{
    func_ptr_t funCallBack;
    uint32_t* ptemp;
    uint8_t Ch;

    //lookup the handler
    for(Ch=0;Ch<MFT_OCU_MAXCH;Ch++)
    {
        if(PdlSet == Mft_Ocu_GetIntFlag(pstcMft,Ch))
        {
            //clear the interrupt flag
            Mft_Ocu_ClrIntFlag(pstcMft,Ch);

            //pointer callback function address
            ptemp = (uint32_t*)&(pstcMftOcuInternData->pfnOcu0Callback);
            funCallBack = (func_ptr_t)(*(uint32_t*)((uint32_t)ptemp + 
                                          (sizeof(func_ptr_t) * Ch)));
            
            if(funCallBack != NULL)
            {
               funCallBack();
            }
        }
    }
}

/**
 ******************************************************************************
 ** \brief Device dependent initialization of interrupts according CMSIS with
 **        level defined in l3.h
 **
 ** \param pstcMft Pointer to OCU instance
 **
 ** \return Ok    Successful initialization
 **
 ******************************************************************************/
static en_result_t Mft_Ocu_InitIrq( volatile stc_mftn_ocu_t* pstcMft )
{

#if (PDL_INTERRUPT_ENABLE_MFT0_OCU == PDL_ON)
    if (pstcMft == (stc_mftn_ocu_t*)(&MFT0_OCU))
    {
        NVIC_ClearPendingIRQ(OCU0_IRQn);
        NVIC_EnableIRQ(OCU0_IRQn);
        NVIC_SetPriority(OCU0_IRQn, PDL_IRQ_LEVEL_MFT0_OCU);
    }
#endif
#if (PDL_INTERRUPT_ENABLE_MFT1_OCU == PDL_ON)
    if (pstcMft == (stc_mftn_ocu_t*)(&MFT1_OCU))
    {
        NVIC_ClearPendingIRQ(OCU1_IRQn);
        NVIC_EnableIRQ(OCU1_IRQn);
        NVIC_SetPriority(OCU1_IRQn, PDL_IRQ_LEVEL_MFT1_OCU);
    }
#endif
#if (PDL_INTERRUPT_ENABLE_MFT2_OCU == PDL_ON)
    if (pstcMft == (stc_mftn_ocu_t*)(&MFT2_OCU))
    {
        NVIC_ClearPendingIRQ(OCU2_IRQn);
        NVIC_EnableIRQ(OCU2_IRQn);
        NVIC_SetPriority(OCU2_IRQn, PDL_IRQ_LEVEL_MFT2_OCU);
    }
#endif
    return Ok;
}

/**
 ******************************************************************************
 ** \brief Device dependent de-initialization of interrupts according CMSIS
 **
 ** \param pstcMft Pointer to OCU instance
 **
 ** \return Ok    Successful initialization
 **
 ******************************************************************************/
static en_result_t Mft_Ocu_DeInitIrq( volatile stc_mftn_ocu_t* pstcMft )
{
    if((pstcMft->OCSA10_f.IOE0 == 0) && (pstcMft->OCSA10_f.IOE1 == 0) &&
       (pstcMft->OCSA32_f.IOE0 == 0) && (pstcMft->OCSA10_f.IOE1 == 0) &&
       (pstcMft->OCSA54_f.IOE0 == 0) && (pstcMft->OCSA10_f.IOE1 == 0) )
    {  
    #if (PDL_INTERRUPT_ENABLE_MFT0_OCU == PDL_ON)      
        if (pstcMft == (stc_mftn_ocu_t*)(&MFT0_OCU))
        {
            NVIC_ClearPendingIRQ(OCU0_IRQn);
            NVIC_DisableIRQ(OCU0_IRQn);
            NVIC_SetPriority(OCU0_IRQn, PDL_DEFAULT_INTERRUPT_LEVEL);

        }
    #endif
    #if (PDL_INTERRUPT_ENABLE_MFT1_OCU == PDL_ON)
        if (pstcMft == (stc_mftn_ocu_t*)(&MFT1_OCU))
        {
            NVIC_ClearPendingIRQ(OCU1_IRQn);
            NVIC_DisableIRQ(OCU1_IRQn);
            NVIC_SetPriority(OCU1_IRQn, PDL_DEFAULT_INTERRUPT_LEVEL);
        }
    #endif
    #if (PDL_INTERRUPT_ENABLE_MFT2_OCU == PDL_ON)
        if (pstcMft == (stc_mftn_ocu_t*)(&MFT2_OCU))
        {
            NVIC_ClearPendingIRQ(OCU2_IRQn);
            NVIC_DisableIRQ(OCU2_IRQn);
            NVIC_SetPriority(OCU2_IRQn, PDL_DEFAULT_INTERRUPT_LEVEL);
        }
    #endif
    }
    return Ok;
}

#endif

/******************************************************************************/
/* Local Functions                                                            */
/******************************************************************************/

/*!
 ******************************************************************************
 ** \brief  Init OCU module
 **
 ** \param [in] pstcMft MFT register definition
 ** \arg    structure of MFT register
 ** \param [in]  u8Ch  channel of OCU
 ** \arg    MFT_OCU_CH0 ~ MFT_OCU_CH5
 ** \param [in] pstcOcuConfig configure of OCU
 ** \arg    structure of OCU cofnigure
 **
 ** \retval Ok                    Internal data has been setup
 ** \retval ErrorInvalidParameter pstcMft == NULL
 **                               pstcOcuConfig == NULL
 **                               Other invalid configuration
 **
 *****************************************************************************
*/
en_result_t Mft_Ocu_Init( volatile stc_mftn_ocu_t*pstcMft, 
                          uint8_t u8Ch, stc_mft_ocu_config_t*  pstcOcuConfig)
{
    stc_mft_ocu_intern_data_t* pstcOcuInternData;
    uint8_t* p32OCFS;
    stc_mft_ocu_ocsb10_field_t* pstcOCSB;
    uint8_t* p8OCSC;
    stc_mft_ocu_ocsd10_field_t* pstcOCSD;

    // Check for NULL pointer and configuration parameter
    if ( (pstcMft == NULL) || 
         (u8Ch >= MFT_OCU_MAXCH) ||
         (pstcOcuConfig->enFrtConnect > OcuFrtToExt) ||
         (pstcOcuConfig->bFm4 > 1) ||
         (pstcOcuConfig->bCmod > 1) || 
         (pstcOcuConfig->enOccpBufMode > OccpBufTrsfByFrtZeroPeak ) || 
         (pstcOcuConfig->enOcseBufMode > OcseBufTrsfByFrtZeroPeak ) )
    {
        return ErrorInvalidParameter ;
    }
    // Get pointer to internal data structure ...
    pstcOcuInternData = MftGetInternDataPtr( pstcMft ) ;
    // ... and check for NULL
    if ( pstcOcuInternData != NULL )
    {
    	pstcOcuInternData->pfnOcu0Callback = NULL;
    	pstcOcuInternData->pfnOcu1Callback = NULL;
    	pstcOcuInternData->pfnOcu2Callback = NULL;
    	pstcOcuInternData->pfnOcu3Callback = NULL;
    	pstcOcuInternData->pfnOcu4Callback = NULL;
    	pstcOcuInternData->pfnOcu5Callback = NULL;
    }

    //Get pointer of current channel OCU register address
    p32OCFS = (uint8_t*)((uint8_t*)pstcMft + 0x68UL + u8Ch/2);
    pstcOCSB = (stc_mft_ocu_ocsb10_field_t*)((uint8_t*)pstcMft + 0x19UL + (u8Ch/2)*4);
    p8OCSC = (uint8_t*)((uint8_t*)pstcMft + 0x25UL);
    pstcOCSD = (stc_mft_ocu_ocsd10_field_t*)((uint8_t*)pstcMft + 0x1AUL + (u8Ch/2)*4);

    //set OCSB register, config ocu's operation
    pstcOCSB->CMOD = pstcOcuConfig->bCmod;
    pstcOCSB->FM4 = pstcOcuConfig->bFm4;

    //set OCSC register, when enable FM3 compatible mode
    *p8OCSC = pstcOcuConfig->u8Mod;

    //set OCSD register and RT output level state
    if(0 == (u8Ch % 2)) //channel 0, 2, 4
    {
        pstcOCSD->OCCP0BUFE = pstcOcuConfig->enOccpBufMode;
        pstcOCSD->OCSE0BUFE = pstcOcuConfig->enOcseBufMode;

        *p32OCFS |= pstcOcuConfig->enFrtConnect;
        pstcOCSB->OTD0 = pstcOcuConfig->enStatePin;
    }
    else//channel 1, 3, 5
    {
        pstcOCSD->OCCP1BUFE = pstcOcuConfig->enOccpBufMode;
        pstcOCSD->OCSE1BUFE = pstcOcuConfig->enOcseBufMode;

        *p32OCFS |= (pstcOcuConfig->enFrtConnect << 4);
        pstcOCSB->OTD1 = pstcOcuConfig->enStatePin;
    }

    return Ok;
}

/*!
 ******************************************************************************
 ** \brief  Compare congifuration of even OCU channel
 **
 ** \param [in] pstcMft MFT register definition
 ** \arg    structure of MFT register
 ** \param [in]  EvenCh  even channel of OCU
 ** \arg    MFT_OCU_CH0, MFT_OCU_CH2, MFT_OCU_CH4
 ** \param [in] pstcConfig pointer to structure of compare mode
 **
 ** \retval Ok                    Internal data has been setup
 ** \retval ErrorInvalidParameter pstcMft == NULL
 **                               EvenCh out of range
 **                               Other invalid configuration
 **
 *****************************************************************************
*/
en_result_t Mft_Ocu_SetEvenChCompareMode(volatile stc_mftn_ocu_t*pstcMft,
                                         uint8_t EvenCh,
                                         stc_even_compare_config_t* pstcConfig)
{
    uint16_t* p16OCSE0;

    // Check for NULL pointer
    if ( (pstcMft == NULL)
         || (EvenCh%2 != 0) 
         || (pstcConfig == NULL) 
         || (pstcConfig->enFrtZeroEvenMatchEvenChRtStatus > RtOutputReverse)  
         || (pstcConfig->enFrtZeroEvenNotMatchEvenChRtStatus > RtOutputReverse)  
         || (pstcConfig->enFrtUpCntEvenMatchEvenChRtStatus > RtOutputReverse) 
         || (pstcConfig->enFrtPeakEvenMatchEvenChRtStatus > RtOutputReverse) 
         || (pstcConfig->enFrtPeakEvenNotMatchEvenChStatus > RtOutputReverse)  
         || (pstcConfig->enFrtDownCntEvenMatchEvenChRtStatus > RtOutputReverse)  
         || (pstcConfig->enIopFlagWhenFrtZeroEvenMatch > IopFlagSet)  
         || (pstcConfig->enIopFlagWhenFrtUpCntEvenMatch > IopFlagSet) 
         || (pstcConfig->enIopFlagWhenFrtPeakEvenMatch > IopFlagSet)  
         || (pstcConfig->enIopFlagWhenFrtDownCntEvenMatch > IopFlagSet)  )
    {
        return ErrorInvalidParameter ;
    }
    
    p16OCSE0 = (uint16_t*)((uint8_t*)pstcMft + 0x28UL + EvenCh*4);
    
    *p16OCSE0 = (pstcConfig->enFrtZeroEvenMatchEvenChRtStatus    << 10)     |
                (pstcConfig->enFrtZeroEvenNotMatchEvenChRtStatus << 14)     |
                (pstcConfig->enFrtUpCntEvenMatchEvenChRtStatus   << 8)      |
                (pstcConfig->enFrtPeakEvenMatchEvenChRtStatus    << 6)      |
                (pstcConfig->enFrtPeakEvenNotMatchEvenChStatus   << 12)     |
                (pstcConfig->enFrtDownCntEvenMatchEvenChRtStatus << 4)      |
                (pstcConfig->enIopFlagWhenFrtZeroEvenMatch       << 3)      |
                (pstcConfig->enIopFlagWhenFrtUpCntEvenMatch      << 2)      |
                (pstcConfig->enIopFlagWhenFrtPeakEvenMatch       << 1)      |
                (pstcConfig->enIopFlagWhenFrtDownCntEvenMatch    << 0);
    return Ok;
}

/*!
 ******************************************************************************
 ** \brief  Compare congifuration of odd OCU channel
 **
 ** \param [in] pstcMft MFT register definition
 ** \arg    structure of MFT register
 ** \param [in]  OddCh  odd channel of OCU
 ** \arg    MFT_OCU_CH1, MFT_OCU_CH3, MFT_OCU_CH5
 ** \param [in] pstcConfig pointer to structure of compare mode
 **
 ** \retval Ok                    Internal data has been setup
 ** \retval ErrorInvalidParameter pstcMft == NULL
 **                               OddCh out of range
 **                               Other invalid configuration
 **
 *****************************************************************************
*/
en_result_t Mft_Ocu_SetOddChCompareMode (volatile stc_mftn_ocu_t*pstcMft,
                                         uint8_t OddCh,
                                         stc_odd_compare_config_t* pstcConfig)
{
    uint32_t* p32OCSE1;
    
    // Check for NULL pointer
    if ( (pstcMft == NULL)
         || (OddCh%2 == 0) 
         || (pstcConfig->enFrtZeroOddMatchEvenMatchOddChRtStatus > RtOutputReverse)     
         || (pstcConfig->enFrtZeroOddMatchEvenNotMatchOddChRtStatus > RtOutputReverse)     
         || (pstcConfig->enFrtZeroOddNotMatchEvenMatchOddChRtStatus > RtOutputReverse)     
         || (pstcConfig->enFrtZeroOddNotMatchEvenNotMatchOddChRtStatus > RtOutputReverse)       
         || (pstcConfig->enFrtUpCntOddMatchEvenMatchOddChRtStatus  > RtOutputReverse)    
         || (pstcConfig->enFrtUpCntOddMatchEvenNotMatchOddChRtStatus > RtOutputReverse)      
         || (pstcConfig->enFrtUpCntOddNotMatchEvenMatchOddChRtStatus > RtOutputReverse)       
         || (pstcConfig->enFrtPeakOddMatchEvenMatchOddChRtStatus     > RtOutputReverse)     
         || (pstcConfig->enFrtPeakOddMatchEvenNotMatchOddChRtStatus  > RtOutputReverse)      
         || (pstcConfig->enFrtPeakOddNotMatchEvenMatchOddChRtStatus  > RtOutputReverse)     
         || (pstcConfig->enFrtPeakOddNotMatchEvenNotMatchOddChRtStatus > RtOutputReverse)      
         || (pstcConfig->enFrtDownOddMatchEvenMatchOddChRtStatus       > RtOutputReverse)     
         || (pstcConfig->enFrtDownOddMatchEvenNotMatchOddChRtStatus    > RtOutputReverse)     
         || (pstcConfig->enFrtDownOddNotMatchEvenMatchOddChRtStatus    > RtOutputReverse)      
         || (pstcConfig->enIopFlagWhenFrtZeroOddMatch    > IopFlagSet)      
         || (pstcConfig->enIopFlagWhenFrtUpCntOddMatch   > IopFlagSet)      
         || (pstcConfig->enIopFlagWhenFrtPeakOddMatch    > IopFlagSet)     
         || (pstcConfig->enIopFlagWhenFrtDownCntOddMatch > IopFlagSet) )
    {
        return ErrorInvalidParameter ;
    }
    
    p32OCSE1 = (uint32_t*)((uint8_t*)pstcMft + 0x28UL + OddCh*4);
    
    *p32OCSE1 = (pstcConfig->enFrtZeroOddMatchEvenMatchOddChRtStatus        << 26)     |
                (pstcConfig->enFrtZeroOddMatchEvenNotMatchOddChRtStatus     << 10)     |
                (pstcConfig->enFrtZeroOddNotMatchEvenMatchOddChRtStatus     << 30)     |
                (pstcConfig->enFrtZeroOddNotMatchEvenNotMatchOddChRtStatus  << 14)     |  
                (pstcConfig->enFrtUpCntOddMatchEvenMatchOddChRtStatus       << 24)     |
                (pstcConfig->enFrtUpCntOddMatchEvenNotMatchOddChRtStatus    << 8)      |
                (pstcConfig->enFrtUpCntOddNotMatchEvenMatchOddChRtStatus    << 18)     |  
                (pstcConfig->enFrtPeakOddMatchEvenMatchOddChRtStatus        << 22)     |
                (pstcConfig->enFrtPeakOddMatchEvenNotMatchOddChRtStatus     << 6)      |
                (pstcConfig->enFrtPeakOddNotMatchEvenMatchOddChRtStatus     << 28)     |
                (pstcConfig->enFrtPeakOddNotMatchEvenNotMatchOddChRtStatus  << 12)     | 
                (pstcConfig->enFrtDownOddMatchEvenMatchOddChRtStatus        << 20)     |
                (pstcConfig->enFrtDownOddMatchEvenNotMatchOddChRtStatus     << 4)      |
                (pstcConfig->enFrtDownOddNotMatchEvenMatchOddChRtStatus     << 16)     |  
                (pstcConfig->enIopFlagWhenFrtZeroOddMatch                   << 3)      |
                (pstcConfig->enIopFlagWhenFrtUpCntOddMatch                  << 2)      |
                (pstcConfig->enIopFlagWhenFrtPeakOddMatch                   << 1)      |
                (pstcConfig->enIopFlagWhenFrtDownCntOddMatch                << 0);
    
    return Ok;
}

/*!
 ******************************************************************************
 ** \brief  enable ocu operation
 **
 ** \param [in] pstcMft MFT register definition
 ** \arg    structure of MFT register
 ** \param [in]  u8Ch  channel of OCU
 ** \arg    MFT_OCU_CH0 ~ MFT_OCU_CH5
 **
 ** \retval Ok                    Internal data has been setup
 ** \retval ErrorInvalidParameter pstcMft == NULL
 **                               u8Ch out of range
 **                               Other invalid configuration
 **
 *****************************************************************************
*/
en_result_t Mft_Ocu_EnableOperation(volatile stc_mftn_ocu_t*pstcMft, uint8_t u8Ch)
{
    stc_mft_ocu_ocsa10_field_t* pstcOCSA;

    // Check for NULL pointer
    if ( (pstcMft == NULL)
         || (u8Ch >= MFT_OCU_MAXCH))
    {
        return ErrorInvalidParameter ;
    }

    //Get pointer of current channel OCU register address
    pstcOCSA = (stc_mft_ocu_ocsa10_field_t*)((uint8_t*)pstcMft + 0x18UL + (u8Ch/2)*4);

    //set register
    if(0 == (u8Ch%2)) //channel 0,2,4
    {
    	pstcOCSA->CST0 = 1;
    }
    else //channel 1,3,5
    {
    	pstcOCSA->CST1 = 1;
    }

    return Ok;
}

/*!
 ******************************************************************************
 ** \brief  Disable ocu operation
 **
 ** \param [in] pstcMft MFT register definition
 ** \arg    structure of MFT register
 ** \param [in]  u8Ch  channel of OCU
 ** \arg    MFT_OCU_CH0 ~ MFT_OCU_CH5
 **
 ** \retval Ok                    Internal data has been setup
 ** \retval ErrorInvalidParameter pstcMft == NULL
 **                               u8Ch out of range
 **                               Other invalid configuration
 **
 *****************************************************************************
*/
en_result_t Mft_Ocu_DisableOperation(volatile stc_mftn_ocu_t*pstcMft, uint8_t u8Ch)
{
    stc_mft_ocu_ocsa10_field_t* pstcOCSA;

    // Check for NULL pointer
    if ( (pstcMft == NULL)
         || (u8Ch >= MFT_OCU_MAXCH))
    {
        return ErrorInvalidParameter ;
    }

    //Get pointer of current channel OCU register address
    pstcOCSA = (stc_mft_ocu_ocsa10_field_t*)((uint8_t*)pstcMft + 0x18UL + (u8Ch/2)*4);

    //set register
    if(0 == (u8Ch%2)) //channel 0,2,4
    {
    	pstcOCSA->CST0 = 0;
    }
    else //channel 1,3,5
    {
    	pstcOCSA->CST1 = 0;
    }

    return Ok;
}

#if (PDL_INTERRUPT_ENABLE_MFT0_OCU == PDL_ON) || \
    (PDL_INTERRUPT_ENABLE_MFT1_OCU == PDL_ON) || \
    (PDL_INTERRUPT_ENABLE_MFT2_OCU == PDL_ON)
/*!
 ******************************************************************************
 ** \brief  enable ocu interrupt
 **
 ** \param [in] pstcMft MFT register definition
 ** \arg    structure of MFT register
 ** \param [in]  u8Ch  channel of OCU
 ** \arg    MFT_OCU_CH0 ~ MFT_OCU_CH5
 ** \param [in]  pfnCallback  callback function
 ** \arg
 **
 ** \retval Ok                    Internal data has been setup
 ** \retval ErrorInvalidParameter pstcMft == NULL
 **                               u8Ch out of range
 **                               Other invalid configuration
 **
 *****************************************************************************
*/
en_result_t Mft_Ocu_EnableInt(volatile stc_mftn_ocu_t*pstcMft,
                              uint8_t u8Ch, func_ptr_t  pfnCallback)
{
    stc_mft_ocu_intern_data_t* pstcOcuInternData;
    stc_mft_ocu_ocsa10_field_t* pstcOCSA;
    func_ptr_t* fnCallback;
    // Check for NULL pointer
    if ( (pstcMft == NULL)
         || (u8Ch >= MFT_OCU_MAXCH))
    {
        return ErrorInvalidParameter ;
    }

    // Get pointer to internal data structure ...
    pstcOcuInternData = MftGetInternDataPtr( pstcMft ) ;

    //Get pointer of current channel OCU register address
    pstcOCSA = (stc_mft_ocu_ocsa10_field_t*)((uint8_t*)pstcMft + 0x18UL + (u8Ch/2)*4);
//    pstcOcuInternData = pstcOcuInternData + (sizeof(func_ptr_t)) * u8Ch;

    //set register
    if(0 == (u8Ch%2)) //channel 0,2,4
    {
        pstcOCSA->IOE0 = 1;
    }
    else //channel 1,3,5
    {
        pstcOCSA->IOE1 = 1;
    }

    //set callback function
    fnCallback = (func_ptr_t*)((uint8_t*)&(pstcOcuInternData->pfnOcu0Callback) 
                               + (sizeof(func_ptr_t) * u8Ch));
    *fnCallback = pfnCallback;
      
    Mft_Ocu_InitIrq(pstcMft);

    return Ok;
}

/*!
 ******************************************************************************
 ** \brief  enable ocu interrupt
 **
 ** \param [in] pstcMft MFT register definition
 ** \arg    structure of MFT register
 ** \param [in]  u8Ch  channel of OCU
 ** \arg    MFT_OCU_CH0 ~ MFT_OCU_CH5
 **
 ** \retval Ok                    Internal data has been setup
 ** \retval ErrorInvalidParameter pstcMft == NULL
 **                               u8Ch out of range
 **                               Other invalid configuration
 **
 *****************************************************************************
*/
en_result_t Mft_Ocu_DisableInt(volatile stc_mftn_ocu_t*pstcMft,uint8_t u8Ch)
{
    stc_mft_ocu_intern_data_t* pstcOcuInternData;
    stc_mft_ocu_ocsa10_field_t* pstcOCSA;

    // Check for NULL pointer
    if ( (pstcMft == NULL)
         || (u8Ch >= MFT_OCU_MAXCH))
    {
        return ErrorInvalidParameter ;
    }

    // Get pointer to internal data structure ...
    pstcOcuInternData = MftGetInternDataPtr( pstcMft ) ;

    //Get pointer of current channel OCU register address
    pstcOCSA = (stc_mft_ocu_ocsa10_field_t*)((uint8_t*)pstcMft + 0x18UL + (u8Ch/2)*4);
    pstcOcuInternData = pstcOcuInternData + (sizeof(pstcOcuInternData)) * u8Ch;

    //set register
    if(0 == (u8Ch%2)) //channel 0,2,4
    {
        pstcOCSA->IOE0 = 0;
    }
    else //channel 1,3,5
    {
        pstcOCSA->IOE1 = 0;
    }

    //set callback function
    pstcOcuInternData = NULL;
    
    Mft_Ocu_DeInitIrq(pstcMft);
    
    return Ok;
}

#endif

/*!
 ******************************************************************************
 ** \brief  get ocu interrupt flag
 **
 ** \param [in] pstcMft MFT register definition
 ** \arg    structure of MFT register
 ** \param [in]  u8Ch  channel of OCU
 ** \arg    MFT_OCU_CH0 ~ MFT_OCU_CH5
 **
 ** \retval en_int_flag_t  return result
 ** \retval PdlClr and PdlSet
 **
 *****************************************************************************
*/
en_int_flag_t Mft_Ocu_GetIntFlag(volatile stc_mftn_ocu_t*pstcMft, uint8_t u8Ch)
{
    stc_mft_ocu_ocsa10_field_t* pstcOCSA;
    en_int_flag_t retval;

    //Get pointer of current channel OCU register address
    pstcOCSA = (stc_mft_ocu_ocsa10_field_t*)((uint8_t*)pstcMft + 0x18UL + (u8Ch/2)*4);

    //set return value
    if(0 == (u8Ch%2)) //channel 0,2,4
    {
    	retval = (en_int_flag_t)pstcOCSA->IOP0;
    }
    else //channel 1,3,5
    {
    	retval = (en_int_flag_t)pstcOCSA->IOP1;
    }

    return retval;
}

/*!
 ******************************************************************************
 ** \brief  clear ocu interrupt flag
 **
 ** \param [in] pstcMft MFT register definition
 ** \arg    structure of MFT register
 ** \param [in]  u8Ch  channel of OCU
 ** \arg    MFT_OCU_CH0 ~ MFT_OCU_CH5
 **
 ** \retval Ok                    Internal data has been setup
 ** \retval ErrorInvalidParameter pstcMft == NULL
 **                               u8Ch out of range
 **                               Other invalid configuration
 **
 *****************************************************************************
*/
en_result_t Mft_Ocu_ClrIntFlag(volatile stc_mftn_ocu_t*pstcMft, uint8_t u8Ch)
{
    stc_mft_ocu_ocsa10_field_t* pstcOCSA;

    // Check for NULL pointer
    if ( (pstcMft == NULL)
         || (u8Ch >= MFT_OCU_MAXCH))
    {
        return ErrorInvalidParameter ;
    }

    //Get pointer of current channel OCU register address
    pstcOCSA = (stc_mft_ocu_ocsa10_field_t*)((uint8_t*)pstcMft + 0x18UL + (u8Ch/2)*4);

    //clear register
    if(0 == (u8Ch%2)) //channel 0,2,4
    {
        pstcOCSA->IOP0 = 0;
    }
    else //channel 1,3,5
    {
        pstcOCSA->IOP1 = 0;
    }

    return Ok;
}

/*!
 ******************************************************************************
 ** \brief  Get RT pin level of OCU
 **
 ** \param [in] pstcMft MFT register definition
 ** \arg    structure of MFT register
 ** \param [in]  u8Ch  channel of OCU
 ** \arg    MFT_OCU_CH0 ~ MFT_OCU_CH5
 **
 ** \retval en_ocu_rt_out_state_t  return level
 ** \retval RtLowlevel and RtHighlevel
 **
 *****************************************************************************
*/
en_ocu_rt_out_state_t Mft_Ocu_GetRtPinLevel(volatile stc_mftn_ocu_t*pstcMft, uint8_t u8Ch)
{
    stc_mft_ocu_ocsb10_field_t* pstcOCSB;
    en_ocu_rt_out_state_t enLevel;
    
    // Get the OCSB address
    pstcOCSB = (stc_mft_ocu_ocsb10_field_t*)((uint8_t*)pstcMft + 0x19UL + (u8Ch/2)*4);
    if((u8Ch%2) == 0)
    {
        enLevel = (en_ocu_rt_out_state_t)pstcOCSB->OTD0; 
    }
    else
    {
        enLevel = (en_ocu_rt_out_state_t)pstcOCSB->OTD1; 
    }
    
    return enLevel;
}

/*!
 ******************************************************************************
 ** \brief  writ occp register
 **
 ** \param [in] pstcMft MFT register definition
 ** \arg    structure of MFT register
 ** \param [in]  u8Ch  channel of OCU
 ** \arg    MFT_OCU_CH0 ~ MFT_OCU_CH5
 ** \param [in]  u16Occp  the value of occp
 ** \arg    16bit value
 **
 ** \retval Ok                    Internal data has been setup
 ** \retval ErrorInvalidParameter pstcMft == NULL
 **                               u8Ch out of range
 **                               Other invalid configuration
 **
 *****************************************************************************
*/
en_result_t Mft_Ocu_WriteOccp(volatile stc_mftn_ocu_t*pstcMft,
		              uint8_t u8Ch, uint16_t u16Occp)
{
	uint16_t* p16OCCP;

    // Check for NULL pointer
    if ( (pstcMft == NULL)
         || (u8Ch >= MFT_OCU_MAXCH))
    {
        return ErrorInvalidParameter ;
    }
    //Get pointer of current channel OCU register address
    p16OCCP = (uint16_t*)((uint8_t*)pstcMft + 0x02 + u8Ch*4);

    //set register
    *p16OCCP = u16Occp;

    return Ok;
}

/*!
 ******************************************************************************
 ** \brief  get occp register value
 **
 ** \param [in] pstcMft MFT register definition
 ** \arg    structure of MFT register
 ** \param [in]  u8Ch  channel of OCU
 ** \arg    MFT_OCU_CH0 ~ MFT_OCU_CH5
 **
 ** \retval 16bit   occp value
 **
 *****************************************************************************
*/
uint16_t Mft_Ocu_ReadOccp(volatile stc_mftn_ocu_t*pstcMft, uint8_t u8Ch)
{
    uint16_t* p16OCCP;

    // Check for NULL pointer
    if ( (pstcMft == NULL)
         || (u8Ch >= MFT_OCU_MAXCH))
    {
        return 0 ;
    }
    //Get pointer of current channel OCU register address
    p16OCCP = (uint16_t*)((uint8_t*)pstcMft + 0x02 + u8Ch*4);

    return (uint16_t)((*p16OCCP) & 0xFFFF);
}

//@}

#endif // #if (defined(PDL_PERIPHERAL_$$X_ACTIVE))
/******************************************************************************/
/* EOF (not truncated)                                                        */
/******************************************************************************/
