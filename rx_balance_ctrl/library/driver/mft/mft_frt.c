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
/** \file mft_frt.c
 **
 ** A detailed description is available at 
 ** @link FrtGroup FRT Module description @endlink
 **  
 ** History:
 **   - 2013-03-21  0.1  Frank  First version.
 **
 *****************************************************************************/


/******************************************************************************/
/* Include files                                                              */
/******************************************************************************/
#include "mft_frt.h"

#if (defined(PDL_PERIPHERAL_MFT_FRT_ACTIVE))

/**
 ******************************************************************************
 ** \addtogroup FrtGroup
 ******************************************************************************/
//@{

/******************************************************************************/
/* Local pre-processor symbols/macros ('#define')                             */
/******************************************************************************/
///  Macro to return the number of enabled MFT instances
#define FRT_INSTANCE_COUNT (uint32_t)(sizeof(m_astcMftFrtInstanceDataLut) \
                                      / sizeof(m_astcMftFrtInstanceDataLut[0]))

/******************************************************************************/
/* Global variable definitions (declared in header file with 'extern')        */
/******************************************************************************/
/// Look-up table for all enabled FRT of MFT  instances and their internal data
stc_mft_frt_instance_data_t m_astcMftFrtInstanceDataLut[] =
{
#if (PDL_PERIPHERAL_ENABLE_MFT0_FRT == PDL_ON)
    { 
        &MFT0_FRT,  // pstcInstance
        NULL        // stcInternData (not initialized yet)
    },
#endif
#if (PDL_PERIPHERAL_ENABLE_MFT1_FRT == PDL_ON)
    { 
        &MFT1_FRT,  // pstcInstance
        NULL        // stcInternData (not initialized yet)
    },
#endif
#if (PDL_PERIPHERAL_ENABLE_MFT2_FRT == PDL_ON)
    { 
        &MFT2_FRT,  // pstcInstance
        NULL        // stcInternData (not initialized yet)
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
 ** \brief Return the internal data for a certain FRT instance.
 **
 ** \param pstcFrt  Pointer to FRT instance
 **
 ** \return Pointer to internal data or NULL if instance is not enabled 
 **         (or not known)
 **
 *****************************************************************************/
static stc_mft_frt_intern_data_t* MftGetInternDataPtr(volatile stc_mftn_frt_t* pstcFrt)
{
    uint32_t u32Instance;
   
    for (u32Instance = 0; u32Instance < FRT_INSTANCE_COUNT; u32Instance++)
    {
        if (pstcFrt == m_astcMftFrtInstanceDataLut[u32Instance].pstcInstance)
        {
            return &m_astcMftFrtInstanceDataLut[u32Instance].stcInternData;
        }
    }

    return NULL;
}

#if (PDL_INTERRUPT_ENABLE_MFT0_FRT == PDL_ON) || \
    (PDL_INTERRUPT_ENABLE_MFT1_FRT == PDL_ON) || \
    (PDL_INTERRUPT_ENABLE_MFT2_FRT == PDL_ON)
/*!
 ******************************************************************************
 ** \brief  FRT interrupt handler sub function
 **
 ** \param [in] pstcMft MFT register definition
 ** \arg    structure of MFT register
 **
 ** \param [in]  pstcMftFrtInternData  structure of frt callback function
 ** \arg    structure of stc_mft_intern_data
 **
 **
 ** \retval void
 **
 *****************************************************************************
*/
void Mft_Frt_IrqHandler( volatile stc_mftn_frt_t*pstcMft,
                         stc_mft_frt_intern_data_t* pstcMftFrtInternData)
{

    static func_ptr_t funCallBack;
    static uint32_t* ptemp;
    uint8_t ch;

    /* Peak match Interrupt */
    for(ch=0;ch<3;ch++)
    {
        if(PdlSet == Mft_Frt_GetIntFlag(pstcMft,ch,enFrtPeakMatchInt))
        {
            /* Clear Interrupt */
            Mft_Frt_ClrIntFlag(pstcMft,ch,enFrtPeakMatchInt);

            //get peak match callback address of each channel
            ptemp = (uint32_t*)&(pstcMftFrtInternData->pfnFrt0PeakCallback);
            funCallBack = (func_ptr_t)(*(uint32_t*)((uint32_t)ptemp + 
                                          (sizeof(func_ptr_t) * ch)));
            
            if(funCallBack != NULL)
            {
               funCallBack();
            }
        }
    }

    /* Zero match Interrupt */
    for(ch=0;ch<3;ch++)
    {
        if(PdlSet == Mft_Frt_GetIntFlag(pstcMft,ch,enFrtZeroMatchInt))
        {
            /* Clear Interrupt */
            Mft_Frt_ClrIntFlag(pstcMft,ch,enFrtZeroMatchInt);

            //get Zero match callback address of each channel
            ptemp = (uint32_t*)&(pstcMftFrtInternData->pfnFrt0ZeroCallback);
            funCallBack = (func_ptr_t)(*(uint32_t*)((uint32_t)ptemp + 
                                          (sizeof(func_ptr_t) * ch)));
            

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
 ** \param [in] pstcMft     Pointer to FRT instance
 ** \param [in] pstcIntSel  Interrupt selection
 **
 ** \return Ok    Successful initialization
 **
 ******************************************************************************/
static en_result_t Mft_Frt_InitIrq( volatile stc_mftn_frt_t* pstcMft,
                                    stc_frt_int_sel_t* pstcIntSel )
{
#if (PDL_INTERRUPT_ENABLE_MFT0_FRT == PDL_ON)
    if (pstcMft == (stc_mftn_frt_t*)(&MFT0_FRT))
    {
        if(pstcIntSel->bFrtZeroMatchInt == 1)
        {
            NVIC_ClearPendingIRQ(FRT0_ZERO_IRQn);
            NVIC_EnableIRQ(FRT0_ZERO_IRQn);
            NVIC_SetPriority(FRT0_ZERO_IRQn, PDL_IRQ_LEVEL_MFT0_FRT);
        }
        if(pstcIntSel->bFrtPeakMatchInt == 1)
        {
            NVIC_ClearPendingIRQ(FRT0_PEAK_IRQn);
            NVIC_EnableIRQ(FRT0_PEAK_IRQn);
            NVIC_SetPriority(FRT0_PEAK_IRQn, PDL_IRQ_LEVEL_MFT0_FRT);
        }
    }
#endif
#if (PDL_INTERRUPT_ENABLE_MFT1_FRT == PDL_ON)
     if (pstcMft == (stc_mftn_frt_t*)(&MFT1_FRT))
    {
        if(pstcIntSel->bFrtZeroMatchInt == 1)
        {
            NVIC_ClearPendingIRQ(FRT1_ZERO_IRQn);
            NVIC_EnableIRQ(FRT1_ZERO_IRQn);
            NVIC_SetPriority(FRT1_ZERO_IRQn, PDL_IRQ_LEVEL_MFT1_FRT);
        }
        if(pstcIntSel->bFrtPeakMatchInt == 1)
        {
            NVIC_ClearPendingIRQ(FRT1_PEAK_IRQn);
            NVIC_EnableIRQ(FRT1_PEAK_IRQn);
            NVIC_SetPriority(FRT1_PEAK_IRQn, PDL_IRQ_LEVEL_MFT1_FRT);
        }
    }
#endif
#if (PDL_INTERRUPT_ENABLE_MFT2_FRT == PDL_ON)
    if (pstcMft == (stc_mftn_frt_t*)(&MFT2_FRT))
    {
        if(pstcIntSel->bFrtZeroMatchInt == 1)
        {
            NVIC_ClearPendingIRQ(FRT2_ZERO_IRQn);
            NVIC_EnableIRQ(FRT2_ZERO_IRQn);
            NVIC_SetPriority(FRT2_ZERO_IRQn, PDL_IRQ_LEVEL_MFT2_FRT);
        }
        if(pstcIntSel->bFrtPeakMatchInt == 1)
        {
            NVIC_ClearPendingIRQ(FRT2_PEAK_IRQn);
            NVIC_EnableIRQ(FRT2_PEAK_IRQn);
            NVIC_SetPriority(FRT2_PEAK_IRQn, PDL_IRQ_LEVEL_MFT2_FRT);
        }
    }
#endif
    return Ok;
}


/**
 ******************************************************************************
 ** \brief Device dependent initialization of interrupts according CMSIS with
 **        level defined in l3.h
 **
 ** \param [in] pstcMft     Pointer to FRT instance
 ** \param [in] pstcIntSel  Interrupt selection
 **
 ** \return Ok    Successful initialization
 **
 ******************************************************************************/
static en_result_t Mft_Frt_DeInitIrq( volatile stc_mftn_frt_t* pstcMft,
                                      stc_frt_int_sel_t* pstcIntSel )
{
    // Check for NULL pointer
    if ((pstcMft == NULL))
    {
        return ErrorInvalidParameter ;
    }
#if (PDL_INTERRUPT_ENABLE_MFT0_FRT == PDL_ON)
    if (pstcMft == (stc_mftn_frt_t*)(&MFT0_FRT))
    {
        if(pstcIntSel->bFrtZeroMatchInt == 1)
        {
            if((pstcMft->TCSA0_f.IRQZE == 0) && (pstcMft->TCSA1_f.IRQZE == 0) && (pstcMft->TCSA2_f.IRQZE == 0)) 
            {
                NVIC_ClearPendingIRQ(FRT0_ZERO_IRQn);
                NVIC_DisableIRQ(FRT0_ZERO_IRQn);
                NVIC_SetPriority(FRT0_ZERO_IRQn, PDL_DEFAULT_INTERRUPT_LEVEL);
            }
        }
        if(pstcIntSel->bFrtPeakMatchInt == 1)
        {
            if((pstcMft->TCSA0_f.ICRE == 0) && (pstcMft->TCSA1_f.ICRE == 0) && (pstcMft->TCSA2_f.ICRE == 0))
            {
                NVIC_ClearPendingIRQ(FRT0_PEAK_IRQn);
                NVIC_DisableIRQ(FRT0_PEAK_IRQn);
                NVIC_SetPriority(FRT0_PEAK_IRQn, PDL_DEFAULT_INTERRUPT_LEVEL);
            }
        }
    }
#endif
#if (PDL_INTERRUPT_ENABLE_MFT1_FRT == PDL_ON)
    if (pstcMft == (stc_mftn_frt_t*)(&MFT1_FRT))
    {
        if(pstcIntSel->bFrtZeroMatchInt == 1)
        {
            if((pstcMft->TCSA0_f.IRQZE == 0) && (pstcMft->TCSA1_f.IRQZE == 0) && (pstcMft->TCSA2_f.IRQZE == 0))
            {
                NVIC_ClearPendingIRQ(FRT1_ZERO_IRQn);
                NVIC_DisableIRQ(FRT1_ZERO_IRQn);
                NVIC_SetPriority(FRT1_ZERO_IRQn, PDL_DEFAULT_INTERRUPT_LEVEL);
            }
        }
        if(pstcIntSel->bFrtPeakMatchInt == 1)
        {
            if((pstcMft->TCSA0_f.ICRE == 0) && (pstcMft->TCSA1_f.ICRE == 0) && (pstcMft->TCSA2_f.ICRE == 0))
            {
                NVIC_ClearPendingIRQ(FRT1_PEAK_IRQn);
                NVIC_DisableIRQ(FRT1_PEAK_IRQn);
                NVIC_SetPriority(FRT1_PEAK_IRQn, PDL_DEFAULT_INTERRUPT_LEVEL);
            }
        }
    }

#endif
#if (PDL_INTERRUPT_ENABLE_MFT2_FRT == PDL_ON)
    if (pstcMft == (stc_mftn_frt_t*)(&MFT2_FRT))
    {
        if(pstcIntSel->bFrtZeroMatchInt == 1)
        {
            if((pstcMft->TCSA0_f.IRQZE == 0) && (pstcMft->TCSA1_f.IRQZE == 0) && (pstcMft->TCSA2_f.IRQZE == 0))
            {
                NVIC_ClearPendingIRQ(FRT2_ZERO_IRQn);
                NVIC_DisableIRQ(FRT2_ZERO_IRQn);
                NVIC_SetPriority(FRT2_ZERO_IRQn, PDL_DEFAULT_INTERRUPT_LEVEL);
            }
        }
        if(pstcIntSel->bFrtPeakMatchInt == 1)
        {
            if((pstcMft->TCSA0_f.ICRE == 0) && (pstcMft->TCSA1_f.ICRE == 0) && (pstcMft->TCSA2_f.ICRE == 0))
            {
                NVIC_ClearPendingIRQ(FRT2_PEAK_IRQn);
                NVIC_DisableIRQ(FRT2_PEAK_IRQn);
                NVIC_SetPriority(FRT2_PEAK_IRQn, PDL_DEFAULT_INTERRUPT_LEVEL);
            }
        }
    }
#endif
    return Ok;
}

#endif

/******************************************************************************/
/* Local Functions                                                            */
/******************************************************************************/

/*!
 ******************************************************************************
 ** \brief  Init FRT module
 **
 ** \param [in] pstcMft MFT register definition
 ** \arg    structure of MFT register
 ** \param [in]  u8Ch  channel of Free run timer 
 ** \arg    MFT_FRT_CH0 ~ MFT_FRT_CH2
 ** \param [in] pstcFrtConfig configure of FRT
 ** \arg    structure of FRT cofnigure
 **
 ** \retval Ok                    Internal data has been setup
 ** \retval ErrorInvalidParameter pstcMft == NULL
 **                               pstcFrtConfig == NULL
 **                               Other invalid configuration
 **
 *****************************************************************************
*/
en_result_t Mft_Frt_Init(volatile stc_mftn_frt_t *pstcMft, uint8_t u8Ch, 
                         stc_mft_frt_config_t*  pstcFrtConfig)
{
    // Pointer to internal data
    stc_mft_frt_intern_data_t* pstcFrtInternData ;
    stc_mft_frt_tcsa0_field_t* pstcTCSA;

    // Check for NULL pointer and configuration parameter
    if ((pstcMft == NULL) || 
        (pstcFrtConfig == NULL) || 
        (pstcFrtConfig->enFrtMode > FrtUpDownCount) ||  
        (pstcFrtConfig->enFrtClockDiv > FrtPclkDiv1024) || 
        (pstcFrtConfig->bEnBuffer > 1) || 
        (pstcFrtConfig->bEnExtClock > 1) )
    {
        return ErrorInvalidParameter ;
    }  
    
    // Get pointer to internal data structure ...
    pstcFrtInternData = MftGetInternDataPtr( pstcMft ) ;
    // ... and check for NULL
    if ( pstcFrtInternData != NULL )
    {
        pstcFrtInternData->pfnFrt0PeakCallback = NULL;
        pstcFrtInternData->pfnFrt0ZeroCallback = NULL;
        pstcFrtInternData->pfnFrt1PeakCallback = NULL;
        pstcFrtInternData->pfnFrt1ZeroCallback = NULL;
        pstcFrtInternData->pfnFrt2PeakCallback = NULL;
        pstcFrtInternData->pfnFrt2ZeroCallback = NULL;
    }

    //Get pointer of current channel frt register address
    pstcTCSA = (stc_mft_frt_tcsa0_field_t*)((uint8_t*)pstcMft + 0x48 + u8Ch*0x0C);
    
    //set count clock cycle of FRT counter
    pstcTCSA->CLK0 = (pstcFrtConfig->enFrtClockDiv >> 0) & 0x01;
    pstcTCSA->CLK1 = (pstcFrtConfig->enFrtClockDiv >> 1) & 0x01;
    pstcTCSA->CLK2 = (pstcFrtConfig->enFrtClockDiv >> 2) & 0x01;
    pstcTCSA->CLK3 = (pstcFrtConfig->enFrtClockDiv >> 3) & 0x01;

    //set frt mode
    pstcTCSA->MODE = pstcFrtConfig->enFrtMode;

    //set buffer enable bit
    pstcTCSA->BFE = pstcFrtConfig->bEnBuffer;

    //set external clock enable bit
    pstcTCSA->ECKE = pstcFrtConfig->bEnExtClock;

    return Ok;
}

/*!
 ******************************************************************************
 ** \brief  set mask zero times
 **
 ** \param [in] pstcMft MFT register definition
 ** \arg    structure of MFT register
 ** \param [in]  u8Ch  channel of Free run timer 
 ** \arg    MFT_FRT_CH0 ~ MFT_FRT_CH2
 ** \param [in] u8Times mask times
 ** \arg    value of 0~15
 **
 ** \retval Ok                    Internal data has been setup
 ** \retval ErrorInvalidParameter pstcMft == NULL
 **                               u8Ch out of range
 **                               Other invalid configuration
 **
 *****************************************************************************
*/
en_result_t Mft_Frt_SetMaskZeroTimes(volatile stc_mftn_frt_t*pstcMft,
		                     uint8_t u8Ch, uint8_t u8Times)
{
      stc_mft_frt_tcsc0_field_t* pstcTCSC;

      // Check for NULL pointer
      if ( (pstcMft == NULL) || 
           (u8Ch >= MFT_FRT_MAX_CH) || 
           (u8Times > 15) )
      {
          return ErrorInvalidParameter ;
      }

      //Get pointer of current channel frt register address
      pstcTCSC=(stc_mft_frt_tcsc0_field_t*)((uint8_t*)pstcMft+0x4AUL+u8Ch*0x0C);

      pstcTCSC->MSZI = u8Times;

      return Ok;
}

/*!
 ******************************************************************************
 ** \brief  get mask zero  times
 **
 ** \param [in] pstcMft MFT register definition
 ** \arg    structure of MFT register
 ** \param [in]  u8Ch  channel of Free run timer 
 ** \arg    MFT_FRT_CH0 ~ MFT_FRT_CH2
 **
 ** \retval Ok                    Internal data has been setup
 ** \retval ErrorInvalidParameter pstcMft == NULL
 **                               u8Ch out of range
 **                               Other invalid configuration
 **
 *****************************************************************************
*/
uint8_t Mft_Frt_GetMaskZeroTimes(volatile stc_mftn_frt_t*pstcMft, uint8_t u8Ch)
{
    stc_mft_frt_tcsc0_field_t* pstcTCSC;

    // Check for NULL pointer
    if ( (pstcMft == NULL) 
         || (u8Ch >= MFT_FRT_MAX_CH))
    {
        return 0 ;
    }
		
    //Get pointer of current channel frt register address
    pstcTCSC = (stc_mft_frt_tcsc0_field_t*)((uint8_t*)pstcMft+0x4AUL+u8Ch*0x0C);

    return (uint8_t)pstcTCSC->MSZC;
}
/*!
 ******************************************************************************
 ** \brief  set mask peak  times
 **
 ** \param [in] pstcMft MFT register definition
 ** \arg    structure of MFT register
 ** \param [in]  u8Ch  channel of Free run timer 
 ** \arg    MFT_FRT_CH0 ~ MFT_FRT_CH2
 ** \param [in] u8Times mask times
 ** \arg    value of 0~15
 **
 ** \retval Ok                    Internal data has been setup
 ** \retval ErrorInvalidParameter pstcMft == NULL
 **                               u8Ch out of range
 **                               Other invalid configuration
 **
 *****************************************************************************
*/
en_result_t Mft_Frt_SetMaskPeakTimes(volatile stc_mftn_frt_t*pstcMft,
                                     uint8_t u8Ch, uint8_t u8Times)
{
    stc_mft_frt_tcsc0_field_t* pstcTCSC;

    // Check for NULL pointer
      if ( (pstcMft == NULL) || 
           (u8Ch >= MFT_FRT_MAX_CH) || 
           (u8Times > 15) )
      {
          return ErrorInvalidParameter ;
      }
		
    //Get pointer of current channel frt register address
    pstcTCSC = (stc_mft_frt_tcsc0_field_t*)((uint8_t*)pstcMft+0x4AUL+u8Ch*0x0C);

    pstcTCSC->MSPI = u8Times;

    return Ok;
}

/*!
 ******************************************************************************
 ** \brief  get mask peak  times
 **
 ** \param [in] pstcMft MFT register definition
 ** \arg    structure of MFT register
 ** \param [in]  u8Ch  channel of Free run timer 
 ** \arg    MFT_FRT_CH0 ~ MFT_FRT_CH2
 **
 ** \retval Ok                    Internal data has been setup
 ** \retval ErrorInvalidParameter pstcMft == NULL
 **                               u8Ch out of range
 **                               Other invalid configuration
 **
 *****************************************************************************
*/
uint8_t Mft_Frt_GetMaskPeakTimes(volatile stc_mftn_frt_t*pstcMft, uint8_t u8Ch)
{
    stc_mft_frt_tcsc0_field_t* pstcTCSC;

    // Check for NULL pointer
    if ( (pstcMft == NULL) 
         || (u8Ch >= MFT_FRT_MAX_CH))
    {
        return 0 ;
    }
		
    //Get pointer of current channel frt register address
    pstcTCSC = (stc_mft_frt_tcsc0_field_t*)((uint8_t*)pstcMft+0x4AUL+u8Ch*0x0C);

    return (uint8_t)pstcTCSC->MSPC;
}

/*!
 ******************************************************************************
 ** \brief  set frt start
 **
 ** \param [in] pstcMft MFT register definition
 ** \arg    structure of MFT register
 ** \param [in]  u8Ch  channel of Free run timer 
 ** \arg    MFT_FRT_CH0 ~ MFT_FRT_CH2
 **
 ** \retval Ok                    Internal data has been setup
 ** \retval ErrorInvalidParameter pstcMft == NULL
 **                               u8Ch out of range
 **                               Other invalid configuration
 **
 *****************************************************************************
*/
en_result_t Mft_Frt_Start(volatile stc_mftn_frt_t*pstcMft, uint8_t u8Ch)
{
    stc_mft_frt_tcsa0_field_t* pstcTCSA;

    // Check for NULL pointer
    if ( (pstcMft == NULL) 
         || (u8Ch >= MFT_FRT_MAX_CH))
    {
        return ErrorInvalidParameter ;
    }
		
    //Get pointer of current channel frt register address
    pstcTCSA = (stc_mft_frt_tcsa0_field_t*)((uint8_t*)pstcMft+0x48UL+u8Ch*0x0C);
    pstcTCSA->STOP = 0;
    pstcTCSA->SCLR = 0;	
    return Ok;
}

/*!
 ******************************************************************************
 ** \brief  set frt stop
 **
 ** \param [in] pstcMft MFT register definition
 ** \arg    structure of MFT register
 ** \param [in]  u8Ch  channel of Free run timer 
 ** \arg    MFT_FRT_CH0 ~ MFT_FRT_CH2
 **
 ** \retval Ok                    Internal data has been setup
 ** \retval ErrorInvalidParameter pstcMft == NULL
 **                               u8Ch out of range
 **                               Other invalid configuration
 **
 *****************************************************************************
*/
en_result_t Mft_Frt_Stop(volatile stc_mftn_frt_t*pstcMft, uint8_t u8Ch)
{
    stc_mft_frt_tcsa0_field_t* pstcTCSA;

    // Check for NULL pointer
    if ( (pstcMft == NULL) 
         || (u8Ch >= MFT_FRT_MAX_CH))
    {
        return ErrorInvalidParameter ;
    }
		
    //Get pointer of current channel frt register address
    pstcTCSA = (stc_mft_frt_tcsa0_field_t*)((uint8_t*)pstcMft+0x48UL+u8Ch*0x0C);

    pstcTCSA->STOP = 1;
    pstcTCSA->SCLR = 1;
    return Ok;
}

#if (PDL_INTERRUPT_ENABLE_MFT0_FRT == PDL_ON) || \
    (PDL_INTERRUPT_ENABLE_MFT1_FRT == PDL_ON) || \
    (PDL_INTERRUPT_ENABLE_MFT2_FRT == PDL_ON)
/*!
 ******************************************************************************
 ** \brief  enable frt interrupt
 **
 ** \param [in] pstcMft MFT register definition
 ** \arg    structure of MFT register
 **
 ** \param [in]  u8Ch  Channel of Free run timer 
 ** \arg    MFT_FRT_CH0 ~ MFT_FRT_CH2
 **
 ** \param [in]  pstcIntSel  Tpe of interrupt
 ** \arg     enFrtZeroMatchInt and  enFrtPeakMatchInt,
 **
 ** \param [in]  pstcFrtIntCallback  Callback function structure of frt
 ** \arg    structure of func_ptr_t
 **
 ** \retval Ok                    Internal data has been setup
 ** \retval ErrorInvalidParameter pstcMft == NULL
 **                               u8Ch out of range
 **                               Other invalid configuration
 **
 *****************************************************************************
*/
en_result_t Mft_Frt_EnableInt(volatile stc_mftn_frt_t*pstcMft, 
                              uint8_t u8Ch, 
                              stc_frt_int_sel_t* pstcIntSel,
                              stc_frt_int_cb_t* pstcFrtIntCallback)
{
     stc_mft_frt_tcsa0_field_t* pstcTCSA;
     stc_mft_frt_intern_data_t* pstcFrtInternData ;

    // Check for NULL pointer
    if ( (pstcMft == NULL) 
         || (u8Ch >= MFT_FRT_MAX_CH)
         || (pstcIntSel->bFrtPeakMatchInt > 1)
         || (pstcIntSel->bFrtZeroMatchInt > 1) )
    {
        return ErrorInvalidParameter ;
    }
    
    // Get pointer to internal data structure ...
    pstcFrtInternData = MftGetInternDataPtr( pstcMft ) ;
    
    //Get pointer of current channel frt register address
    pstcTCSA = (stc_mft_frt_tcsa0_field_t*)((uint8_t*)pstcMft+0x48UL+u8Ch*0x0C);

    if(pstcIntSel->bFrtZeroMatchInt == 1) //zero detect interrupt
    {
        pstcTCSA->IRQZE = 1;
    }
    if(pstcIntSel->bFrtPeakMatchInt == 1) //peak detect interrupt
    {
        pstcTCSA->ICRE = 1;
    }
		
    //set callback function
    switch(u8Ch)
    {
        case MFT_FRT_CH0: //channel 0
            if(pstcIntSel->bFrtZeroMatchInt == 1) //zero detect interrupt
            {
                pstcFrtInternData->pfnFrt0ZeroCallback = pstcFrtIntCallback->pfnFrtZeroCallback;
            }
            if(pstcIntSel->bFrtPeakMatchInt == 1) //peak detect interrupt
            {
                pstcFrtInternData->pfnFrt0PeakCallback = pstcFrtIntCallback->pfnFrtPeakCallback;
            }
            break;
                
        case MFT_FRT_CH1: //channel 1
            if(pstcIntSel->bFrtZeroMatchInt == 1) //zero detect interrupt
            {
                pstcFrtInternData->pfnFrt1ZeroCallback = pstcFrtIntCallback->pfnFrtZeroCallback;
            }
            if(pstcIntSel->bFrtPeakMatchInt == 1)  //peak detect interrupt
            {
                pstcFrtInternData->pfnFrt1PeakCallback = pstcFrtIntCallback->pfnFrtPeakCallback;
            }
            break;
        case MFT_FRT_CH2: //channel 2
            if(pstcIntSel->bFrtZeroMatchInt == 1)  //zero detect interrupt
            {
                pstcFrtInternData->pfnFrt2ZeroCallback = pstcFrtIntCallback->pfnFrtZeroCallback;
            }
            if(pstcIntSel->bFrtPeakMatchInt == 1)  //peak detect interrupt
            {
                pstcFrtInternData->pfnFrt2PeakCallback = pstcFrtIntCallback->pfnFrtPeakCallback;
            }
            break;
        default :
            return ErrorInvalidParameter;
    }
    Mft_Frt_InitIrq(pstcMft,pstcIntSel);
  
    return Ok;
}

/*!
 ******************************************************************************
 ** \brief  disable frt interrupt
 **
 ** \param [in] pstcMft MFT register definition
 ** \arg    structure of MFT register
 **
 ** \param [in]  u8Ch  channel of Free run timer 
 ** \arg    MFT_FRT_CH0 ~ MFT_FRT_CH2
 **
 ** \param [in]  pstcIntSel  the type of interrupt
 ** \arg     enFrtZeroMatchInt and  enFrtPeakMatchInt,
 **
 ** \retval Ok                    Internal data has been setup
 ** \retval ErrorInvalidParameter pstcMft == NULL
 **                               u8Ch out of range
 **                               Other invalid configuration
 **
 *****************************************************************************
*/
en_result_t Mft_Frt_DisableInt(volatile stc_mftn_frt_t*pstcMft,uint8_t u8Ch,
                               stc_frt_int_sel_t* pstcIntSel)
{
    stc_mft_frt_tcsa0_field_t* pstcTCSA;

    // Check for NULL pointer
    if ( (pstcMft == NULL) 
         || (u8Ch >= MFT_FRT_MAX_CH)
         || (pstcIntSel->bFrtPeakMatchInt > 1) 
         || (pstcIntSel->bFrtZeroMatchInt > 1))
    {
        return ErrorInvalidParameter ;
    }

    //Get pointer of current channel frt register address
    pstcTCSA = (stc_mft_frt_tcsa0_field_t*)((uint8_t*)pstcMft+0x48UL+u8Ch*0x0C);
  		
    if(pstcIntSel->bFrtZeroMatchInt == 1) //zero detect interrupt
    {
        pstcTCSA->IRQZE = 0;
    }
    if(pstcIntSel->bFrtPeakMatchInt == 1) //peak detect interrupt
    {
        pstcTCSA->ICRE = 0;
    }  
    Mft_Frt_DeInitIrq(pstcMft, pstcIntSel);     
    return Ok;
}

#endif  

/*!
 ******************************************************************************
 ** \brief  get frt interrupt falg
 **
 ** \param [in] pstcMft MFT register definition
 ** \arg    structure of MFT register
 **
 ** \param [in]  u8Ch  channel of Free run timer 
 ** \arg    MFT_FRT_CH0 ~ MFT_FRT_CH2
 **
 ** \param [in]  enIntType  the type of interrupt
 ** \arg     enFrtZeroMatchInt and  enFrtPeakMatchInt,
 **
 ** \retval Ok                    Internal data has been setup
 ** \retval ErrorInvalidParameter pstcMft == NULL
 **                               u8Ch out of range
 **                               Other invalid configuration
 **
 *****************************************************************************
*/
en_int_flag_t Mft_Frt_GetIntFlag(volatile stc_mftn_frt_t*pstcMft, 
                                 uint8_t u8Ch, 
                                 en_mft_frt_int_t enIntType)
{
    stc_mft_frt_tcsa0_field_t* pstcTCSA;
    en_int_flag_t retval;

    //Get pointer of current channel frt register address
    pstcTCSA = (stc_mft_frt_tcsa0_field_t*)((uint8_t*)pstcMft+0x48UL+u8Ch*0x0C);
  		
    if(enFrtZeroMatchInt == enIntType) //zero detect interrupt
    {
        retval = (en_int_flag_t)pstcTCSA->IRQZF;
    }
    else //peak detect interrupt
    {
        retval = (en_int_flag_t)pstcTCSA->ICLR;
    }
		
    return retval;
}

/*!
 ******************************************************************************
 ** \brief  clear frt interrupt falg
 **
 ** \param [in] pstcMft MFT register definition
 ** \arg    structure of MFT register
 **
 ** \param [in]  u8Ch  channel of Free run timer 
 ** \arg    MFT_FRT_CH0 ~ MFT_FRT_CH2
 **
 ** \param [in]  enIntType  the type of interrupt
 ** \arg     enFrtZeroMatchInt and  enFrtPeakMatchInt,
 **
 ** \retval Ok                    Internal data has been setup
 ** \retval ErrorInvalidParameter pstcMft == NULL
 **                               u8Ch out of range
 **                               Other invalid configuration
 **
 *****************************************************************************
*/
en_result_t Mft_Frt_ClrIntFlag(volatile stc_mftn_frt_t*pstcMft, 
                               uint8_t u8Ch, 
                               en_mft_frt_int_t enIntType)
{
    stc_mft_frt_tcsa0_field_t* pstcTCSA;

    // Check for NULL pointer
    if ( (pstcMft == NULL) 
         || (u8Ch >= MFT_FRT_MAX_CH))
    {
        return ErrorInvalidParameter ;
    }

    //Get pointer of current channel frt register address
    pstcTCSA = (stc_mft_frt_tcsa0_field_t*)((uint8_t*)pstcMft+0x48UL+u8Ch*0x0C);
  		
    if(enFrtZeroMatchInt == enIntType) //zero detect interrupt
    {
        pstcTCSA->IRQZF = 0;
    }
    else //peak detect interrupt
    {
        pstcTCSA->ICLR = 0;
    }
		
    return Ok;
}

/*!
 ******************************************************************************
 ** \brief  set frt cycle value
 **
 ** \param [in] pstcMft MFT register definition
 ** \arg    structure of MFT register
 **
 ** \param [in]  u8Ch  channel of Free run timer 
 ** \arg    MFT_FRT_CH0 ~ MFT_FRT_CH2
 **
 ** \param [in]  u16Cycle  the cycle value 
 ** \arg     number of 16bit
 **
 ** \retval Ok                    Internal data has been setup
 ** \retval ErrorInvalidParameter pstcMft == NULL
 **                               u8Ch out of range
 **                               Other invalid configuration
 **
 *****************************************************************************
*/
en_result_t Mft_Frt_SetCountCycle(volatile stc_mftn_frt_t*pstcMft, 
                                  uint8_t u8Ch, uint16_t u16Cycle)
{
    uint16_t* pstcTCCP;
		
    // Check for NULL pointer
    if ( (pstcMft == NULL) 
         || (u8Ch >= MFT_FRT_MAX_CH))
    {
        return ErrorInvalidParameter ;
    }

    //Get pointer of current channel frt register address
    pstcTCCP = (uint16_t*)( (uint8_t*)pstcMft + 0x42UL + u8Ch*0x0C);

    *pstcTCCP = u16Cycle;

    return Ok;
}

/*!
 ******************************************************************************
 ** \brief  set frt count value
 **
 ** \param [in] pstcMft MFT register definition
 ** \arg    structure of MFT register
 **
 ** \param [in]  u8Ch  channel of Free run timer 
 ** \arg    MFT_FRT_CH0 ~ MFT_FRT_CH2
 **
 ** \param [in]  u16Count  the count value 
 ** \arg     number of 16bit
 **
 ** \retval Ok                    Internal data has been setup
 ** \retval ErrorInvalidParameter pstcMft == NULL
 **                               Other invalid configuration
 **
 *****************************************************************************
*/
en_result_t Mft_Frt_SetCountVal(volatile stc_mftn_frt_t*pstcMft, 
                                uint8_t u8Ch,uint16_t u16Count)
{
    uint16_t* pstcTCDT;
    
    // Check for NULL pointer
    if ( (pstcMft == NULL) 
         || (u8Ch >= MFT_FRT_MAX_CH))
    {
        return ErrorInvalidParameter ;
    }

    //Get pointer of current channel frt register address
    pstcTCDT = (uint16_t*)( (uint8_t*)pstcMft + 0x46UL + u8Ch*0x0C);

    *pstcTCDT = u16Count;

    return Ok;
}

/*!
 ******************************************************************************
 ** \brief  get frt current count 
 **
 ** \param [in] pstcMft MFT register definition
 ** \arg    structure of MFT register
 **
 ** \param [in]  u8Ch  channel of Free run timer 
 ** \arg    MFT_FRT_CH0 ~ MFT_FRT_CH2
 **
 ** \retval current count of frt
 **
 *****************************************************************************
*/
uint16_t Mft_Frt_GetCurCount(volatile stc_mftn_frt_t*pstcMft, uint8_t u8Ch)
{
    uint16_t retTCDT;
    uint16_t* pstcTCDT;
    
    // Check for NULL pointer
    if ( (pstcMft == NULL) 
         || (u8Ch >= MFT_FRT_MAX_CH))
    {
        return ErrorInvalidParameter ;
    }

    //Get pointer of current channel frt register address
    pstcTCDT = (uint16_t*)( (uint8_t*)pstcMft + 0x46UL + u8Ch*0x0C);

    retTCDT = *pstcTCDT;

    return retTCDT;
}

//@}

#endif // #if (defined(PDL_PERIPHERAL_$$X_ACTIVE))

/******************************************************************************/
/* EOF (not truncated)                                                        */
/******************************************************************************/
