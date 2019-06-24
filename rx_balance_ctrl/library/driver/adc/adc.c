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
/** \file adc.c
 **
 ** A detailed description is available at 
 ** @link AdcGroup ADC description @endlink
 **
 ** History:
 **   - 2013-03-26  1.0  MWi  First version.
 **   - 2013-07-22  1.1  MWi  func_ptr_adc_arg32_t changed to
 **                           func_ptr_adc_parg32_t for pointing to FIFO result
 **                           register
 **   - 2013-11-22  1.2  EZ   Correct Priority FIFO length location in the 
 **                           structure of stc_adc_config_t
 **                           Add the software trigger function of Priority 
 **                           Conversion
 **   - 2014-01-15  1.3  MWi  Fifio typo corrected to Fifo in 
 **                           Adc_ScanFifoStatus(), Adc_ScanFifoClear(),
 **                           Adc_PrioFifoStatus(), Adc_PrioFifoClear()
 **
 ******************************************************************************/

/******************************************************************************/
/* Include files                                                              */
/******************************************************************************/
#include "adc.h"

#if (defined(PDL_PERIPHERAL_ADC_ACTIVE))

/**
 ******************************************************************************
 ** \addtogroup AdcGroup
 ******************************************************************************/
//@{

/******************************************************************************/
/* Local pre-processor symbols/macros ('#define')                             */
/******************************************************************************/

/******************************************************************************/
/* Global variable definitions (declared in header file with 'extern')        */
/******************************************************************************/

/// Look-up table for all enabled ADC instances and their internal data
stc_adc_instance_data_t m_astcAdcInstanceDataLut[ADC_INSTANCE_COUNT] =
{
  #if (PDL_PERIPHERAL_ENABLE_ADC0 == PDL_ON)
  { 
     &ADC0,  // pstcInstance
     {0, 0 ,0, 0, 0 ,0}           // stcInternData (not initialized yet)
  },
  #endif
  #if (PDL_PERIPHERAL_ENABLE_ADC1 == PDL_ON)
  {  
     &ADC1,  // pstcInstance
     {0, 0 ,0, 0, 0 ,0}            // stcInternData (not initialized yet)
  },
  #endif
  #if (PDL_PERIPHERAL_ENABLE_ADC2 == PDL_ON)
  {  
     &ADC2,  // pstcInstance
     {0, 0 ,0, 0, 0 ,0}            // stcInternData (not initialized yet)
  }
  #endif
};

/******************************************************************************/
/* Local type definitions ('typedef')                                         */
/******************************************************************************/
typedef union
{
	uint8_t              u8AdcSccrReadout;
	stc_adc_sccr_field_t stcAdcSccrReadout;
	
}un_adc_sccr_t;

typedef union
{
	stc_adc_pccr_field_t stcAdcPccrReadout;
	uint8_t              u8AdcPccrReadout;
	
}un_adc_pccr_t;

/******************************************************************************/
/* Local function prototypes ('static')                                       */
/******************************************************************************/
static stc_adc_intern_data_t* AdcGetInternDataPtr(volatile stc_adcn_t* pstcAdc);

#if (PDL_INTERRUPT_ENABLE_ADC0 == PDL_ON) || \
    (PDL_INTERRUPT_ENABLE_ADC1 == PDL_ON) || \
    (PDL_INTERRUPT_ENABLE_ADC2 == PDL_ON)
static void Adc_InitInterrupt(volatile stc_adcn_t* pstcAdc);
static void Adc_DeInitInterrupt(volatile stc_adcn_t* pstcAdc);
#endif

/******************************************************************************/
/* Local variable definitions ('static')                                      */
/******************************************************************************/

/******************************************************************************/
/* Function implementation - global ('extern') and local ('static')           */
/******************************************************************************/

/******************************************************************************/
/* Local Functions                                                            */
/******************************************************************************/

/**
 ******************************************************************************
 ** \brief Return the internal data for a certain ADC instance.
 **
 ** \param pstcAdc Pointer to ADC instance
 **
 ** \return Pointer to internal data or NULL if instance is not enabled (or not known)
 **
 ******************************************************************************/
static stc_adc_intern_data_t* AdcGetInternDataPtr(volatile stc_adcn_t* pstcAdc) 
{
    uint8_t u8Instance;
   
    for (u8Instance = 0u; u8Instance < ADC_INSTANCE_COUNT; u8Instance++)
    {
        if (pstcAdc == m_astcAdcInstanceDataLut[u8Instance].pstcInstance)
        {
            return &m_astcAdcInstanceDataLut[u8Instance].stcInternData;
        }
    }

    return NULL;
}

#if (PDL_INTERRUPT_ENABLE_ADC0 == PDL_ON) || \
    (PDL_INTERRUPT_ENABLE_ADC1 == PDL_ON) || \
    (PDL_INTERRUPT_ENABLE_ADC2 == PDL_ON)
      
/**
 ******************************************************************************
 ** \brief ADC instance interrupt service routine
 **
 ** ADC instance interrupt service routine, clear interrupt cause and
 **
 ** \param  pstcAdc              Pointer to ADC instance
 ** \param  pstcAdcInternData    Pointer to ADC intern data
 **        
 ******************************************************************************/
void AdcIrqHandler( volatile stc_adcn_t*   pstcAdc,
                    stc_adc_intern_data_t* pstcAdcInternData
                  )
{
  if (pstcAdc->SCCR_f.SOVR)   // Error case (FIFO overflow)
  {
    pstcAdc->SCCR_f.SOVR = 0u;
    
    if (pstcAdcInternData->pfnErrorCallbackAdc != NULL)
    {
       pstcAdcInternData->pfnErrorCallbackAdc();
    }
  }
  
  if (pstcAdc->PCCR_f.POVR)  // Error case (Priority FIFO overflow)
  {
    pstcAdc->PCCR_f.POVR = 0u;
    
    if (pstcAdcInternData->pfnPrioErrorCallbackAdc != NULL)
    {
       pstcAdcInternData->pfnPrioErrorCallbackAdc();
    }
  }
  
  if (pstcAdc->ADCR_f.PCIF)  // Priority Conversion
  {
    pstcAdc->ADCR_f.PCIF = 0u;
    
    if (pstcAdcInternData->pfnPrioConversion != NULL)
    {
      // Callback argument points to instance scan FIFO. User has to empty it in
      //   the callback function, because the FIFO depth is known.
      pstcAdcInternData->pfnPrioConversion((uint32_t*)&pstcAdc->PCFD);
    }
  }
  
  if (pstcAdc->ADCR_f.SCIF)   // Scan conversion interrupt request?
  {
    pstcAdc->ADCR_f.SCIF = 0u;
    
    if (pstcAdcInternData->pfnScanConversion != NULL)
    {
      // Callback argument points to instance FIFO. User has to empty it in
      //   the callback function, because the FIFO depth is known. 
      pstcAdcInternData->pfnScanConversion((uint32_t*)&pstcAdc->SCFD);
    }
  }
  
  if (pstcAdc->ADCR_f.CMPIF)  // Compare result interrupt request?
  {
    pstcAdc->ADCR_f.CMPIF = 0u;
    
    if (pstcAdcInternData->pfnComparisonCallback != NULL)
    {
       pstcAdcInternData->pfnComparisonCallback();
    }
  }

  if (pstcAdc->WCMRCIF_f.RCINT)  // Range result interrupt request?
  {
    pstcAdc->WCMRCIF_f.RCINT = 0u;
    
    if (pstcAdcInternData->pfnRangeCallback != NULL)
    {
       pstcAdcInternData->pfnRangeCallback();
    }
  }
  
  return;
} // AdcIrqHandler

/**
 ******************************************************************************
 ** \brief Set NVIC Interrupt depending on ADC instance
 **
 ** \param pstcAdc Pointer to ADC instance
 **
 ******************************************************************************/
static void Adc_InitInterrupt(volatile stc_adcn_t* pstcAdc)
{
  if (pstcAdc == (volatile stc_adcn_t*)(&ADC0))
  {
    NVIC_ClearPendingIRQ(ADC0_IRQn);
    NVIC_EnableIRQ(ADC0_IRQn);
    NVIC_SetPriority(ADC0_IRQn, PDL_IRQ_LEVEL_ADC0);   
  }
  else if (pstcAdc == (volatile stc_adcn_t*)(&ADC1))
  {
    NVIC_ClearPendingIRQ(ADC1_IRQn);
    NVIC_EnableIRQ(ADC1_IRQn);
    NVIC_SetPriority(ADC1_IRQn, PDL_IRQ_LEVEL_ADC1);      
  }
  else if (pstcAdc == (volatile stc_adcn_t*)(&ADC2))
  {
    NVIC_ClearPendingIRQ(ADC2_IRQn);
    NVIC_EnableIRQ(ADC2_IRQn);
    NVIC_SetPriority(ADC2_IRQn, PDL_IRQ_LEVEL_ADC2);      
  }  
} // Adc_InitInterrupt

/**
 ******************************************************************************
 ** \brief Clear NVIC Interrupt depending on ADC instance
 **
 ** \param pstcAdc Pointer to ADC instance
 **
 ******************************************************************************/
static void Adc_DeInitInterrupt(volatile stc_adcn_t* pstcAdc)
{
  if (pstcAdc == (volatile stc_adcn_t*)(&ADC0))
  {
    NVIC_ClearPendingIRQ(ADC0_IRQn);
    NVIC_DisableIRQ(ADC0_IRQn);
    NVIC_SetPriority(ADC0_IRQn, PDL_DEFAULT_INTERRUPT_LEVEL);   
  }
  else if (pstcAdc == (volatile stc_adcn_t*)(&ADC1))
  {
    NVIC_ClearPendingIRQ(ADC1_IRQn);
    NVIC_DisableIRQ(ADC1_IRQn);
    NVIC_SetPriority(ADC1_IRQn, PDL_DEFAULT_INTERRUPT_LEVEL);      
  }
  else if (pstcAdc == (volatile stc_adcn_t*)(&ADC2))
  {
    NVIC_ClearPendingIRQ(ADC2_IRQn);
    NVIC_DisableIRQ(ADC2_IRQn);
    NVIC_SetPriority(ADC2_IRQn, PDL_DEFAULT_INTERRUPT_LEVEL);      
  }  
} // Adc_DeInitInterrupt
#endif // #if (PDL_INTERRUPT_ENABLE_ADC0 == PDL_ON) || (PDL_INTERRUPT_ENABLE_ADC1 == PDL_ON) || (PDL_INTERRUPT_ENABLE_ADC2 == PDL_ON)

/******************************************************************************/
/* Global Functions                                                           */
/******************************************************************************/

/**
 ******************************************************************************
 ** \brief Initialize ADC
 **
 ** This function initializes an ADC module and sets up the internal
 ** data structures
 **
 ** \param [in]  pstcAdc          ADC instance
 ** \param [in]  pstcConfig       ADC module configuration 
 **
 ** \retval Ok                    Internal data has been setup
 ** \retval ErrorInvalidParameter pstcAdc == NULL
 **                               pstcConfig == NULL
 **                               Other invalid configuration
 ******************************************************************************/
en_result_t Adc_Init( volatile stc_adcn_t* pstcAdc,
                      stc_adc_config_t*    pstcConfig )
{
  stc_adc_sccr_field_t  stcSCCR;
  stc_adc_pccr_field_t  stcPCCR;
  stc_adc_adst0_field_t stcADST0;
  stc_adc_adst1_field_t stcADST1;
  stc_adc_pcis_field_t  stcPCIS;
  stc_adc_cmpcr_field_t stcCMPCR;
	// Pointer to internal data
  stc_adc_intern_data_t* pstcAdcInternData ;  
  
  PDL_ZERO_STRUCT(stcSCCR);
  PDL_ZERO_STRUCT(stcPCCR);
  PDL_ZERO_STRUCT(stcADST0);
  PDL_ZERO_STRUCT(stcADST1);
  PDL_ZERO_STRUCT(stcPCIS);
  PDL_ZERO_STRUCT(stcCMPCR);
  
  
  // Check for NULL pointer
  if ((pstcAdc    == NULL) ||
      (pstcConfig == NULL)
     )
  {
    return ErrorInvalidParameter ;
  }  
  
  // Disable ADC in any case first
  pstcAdc->ADCEN_f.ENBL = 0u;
  
  // Clear all relevant registers
  pstcAdc->ADCR = 0u;
  pstcAdc->ADSR = 0u;
  pstcAdc->SCCR = 0u;
  
  // Get pointer to internal data structure ...
  pstcAdcInternData = AdcGetInternDataPtr( pstcAdc ) ;
  // ... and check for NULL
  if ( pstcAdcInternData != NULL )
  {
#if (PDL_INTERRUPT_ENABLE_ADC0 == PDL_ON) || \
    (PDL_INTERRUPT_ENABLE_ADC1 == PDL_ON) || \
    (PDL_INTERRUPT_ENABLE_ADC2 == PDL_ON)
      pstcAdcInternData->pfnScanConversion       = NULL;
      pstcAdcInternData->pfnPrioConversion       = NULL;
      pstcAdcInternData->pfnErrorCallbackAdc     = NULL;
      pstcAdcInternData->pfnPrioErrorCallbackAdc = NULL;
      pstcAdcInternData->pfnComparisonCallback   = NULL;
      pstcAdcInternData->pfnRangeCallback        = NULL;
#endif // #if (PDL_INTERRUPT_ENABLE_ADC0 == PDL_ON) || (PDL_INTERRUPT_ENABLE_ADC1 == PDL_ON) || (PDL_INTERRUPT_ENABLE_ADC2 == PDL_ON)
  }  
  
  // Set ADC input pin configuration
  FM4_GPIO->ADE = (pstcConfig->u32CannelSelect.u32AD_CHn);
  
  // Set Scan Conversion Input Selection Register
  pstcAdc->SCIS01 = (uint16_t)(0x0000FFFFul & pstcConfig->u32CannelSelect.u32AD_CHn);
  pstcAdc->SCIS23 = (uint16_t)(0x0000FFFFul & (pstcConfig->u32CannelSelect.u32AD_CHn >> 16u));
  
  // Set Sampling Time Selection Register
  pstcAdc->ADSS01 = (uint16_t)(0x0000FFFFul & pstcConfig->u32SamplingTimeSelect.u32AD_CHn);
  pstcAdc->ADSS23 = (uint16_t)(0x0000FFFFul & pstcConfig->u32SamplingTimeSelect.u32AD_CHn >> 16u);
  
  // Set Sampling Times Config. 0
  switch (pstcConfig->enSamplingTimeN0)
  {
    case Value1:
      stcADST0.STX0 = 0u;
      break;
    case Value4:
      stcADST0.STX0 = 1u;
      break;
    case Value8:
      stcADST0.STX0 = 2u;
      break;
    case Value16:
      stcADST0.STX0 = 3u;
      break;
    case Value32:
      stcADST0.STX0 = 4u;
      break;
    case Value64:
      stcADST0.STX0 = 5u;
      break;
    case Value128:
      stcADST0.STX0 = 6u;
      break;
    case Value256:
      stcADST0.STX0 = 7u;
      break;
    default:
      return ErrorInvalidParameter;
  }
  
  if (pstcConfig->u8SamplingTime0 > 31u)
  {
      return ErrorInvalidParameter;
  }
      
  stcADST0.ST0 = pstcConfig->u8SamplingTime0;
  pstcAdc->ADST0_f = stcADST0;

  // Set Sampling Times Config. 1
  switch (pstcConfig->enSamplingTimeN1)
  {
    case Value1:
      stcADST1.STX1 = 0u;
      break;
    case Value4:
      stcADST1.STX1 = 1u;
      break;
    case Value8:
      stcADST1.STX1 = 2u;
      break;
    case Value16:
      stcADST1.STX1 = 3u;
      break;
    case Value32:
      stcADST1.STX1 = 4u;
      break;
    case Value64:
      stcADST1.STX1 = 5u;
      break;
    case Value128:
      stcADST1.STX1 = 6u;
      break;
    case Value256:
      stcADST1.STX1 = 7u;
      break;
    default:
      return ErrorInvalidParameter;
  }
  
  if (pstcConfig->u8SamplingTime1 > 31u)
  {
      return ErrorInvalidParameter;
  }
      
  stcADST1.ST1 = pstcConfig->u8SamplingTime1;
  pstcAdc->ADST1_f = stcADST1;
  
  // Frequency Division for ADC Instance
  pstcAdc->ADCT = pstcConfig->u8SamplingMultiplier;
  
  // MSB, LSB alignment
  pstcAdc->ADSR_f.FDAS = (TRUE == pstcConfig->bLsbAlignment) ? 1u : 0u;
  
  // Conversion mode single, repeated
  switch (pstcConfig->enScanMode)
  {
    case ScanSingleConversion:
      stcSCCR.RPT = 0u;
      break;
    case ScanRepeatConversion:
      stcSCCR.RPT = 1u;
      break;
    default:
      return ErrorInvalidParameter;
  }
  
  // Scan Conversion Timer Start Enable and Timer Source
  if (TRUE == pstcConfig->bScanTimerStartEnable)
  {
    stcSCCR.SHEN = 1u;
    
    pstcAdc->SCTSL = (uint8_t) pstcConfig->enScanConversionTimerTrigger;
  }
  
  // Update hardware
  pstcAdc->SCCR_f = stcSCCR;
  
  // Scan Conversion FIFO Depth
  if (pstcConfig->u8ScanFifoDepth > 15u)
  {
    return ErrorInvalidParameter;
  }
  
  pstcAdc->SFNS = pstcConfig->u8ScanFifoDepth;
  
  // Priority external Trigger Analog Input
  stcPCCR.ESCE = (TRUE == pstcConfig->bPrioExtTrigEnable) ? 1u : 0u;
  
  // Priority external Trigger Start Enable
  stcPCCR.PEEN = (TRUE == pstcConfig->bPrioExtTrigStartEnable) ? 1u : 0u;
  
  // Priority Conversion Timer Start Enable and Timer Source
  if (TRUE == pstcConfig->bPrioTimerStartEnable)
  {
    stcPCCR.PHEN = 1u;
    
    pstcAdc->PRTSL = (uint8_t) pstcConfig->enPrioConversionTimerTrigger;
  }
  
  // Update Hardware
  pstcAdc->PCCR_f = stcPCCR;
      
  // Priority Conversion FIFO Stage Count Setup
  pstcAdc->PFNS = pstcConfig->u8PrioFifoDepth;

  // Priority Conversion Input Selection
  if (pstcConfig->u8PrioLevel1AnalogChSel > 7u)
  {
    return ErrorInvalidParameter;
  }
  stcPCIS.P1A = pstcConfig->u8PrioLevel1AnalogChSel;
  
  if (pstcConfig->u8PrioLevel2AnalogChSel > 31u)
  {
    return ErrorInvalidParameter;
  }
  stcPCIS.P2A = pstcConfig->u8PrioLevel2AnalogChSel;
  
  pstcAdc->PCIS_f = stcPCIS;
  
  if (TRUE == pstcConfig->bComparisonEnable)
  {
    // Comparison Value
    pstcAdc->CMPD = ((0x0FFCu & pstcConfig->u16ComparisonValue) << 6u);
    
    // Comparison Control
    stcCMPCR.CMPEN = (TRUE == pstcConfig->bComparisonEnable)    ? 1u : 0u;
    
#if (PDL_INTERRUPT_ENABLE_ADC0 == PDL_ON) || \
    (PDL_INTERRUPT_ENABLE_ADC1 == PDL_ON) || \
    (PDL_INTERRUPT_ENABLE_ADC2 == PDL_ON)
    stcCMPCR.CMD1  = (TRUE == pstcConfig->bCompIrqEqualGreater) ? 1u : 0u;
#endif
    
    stcCMPCR.CMD0  = (TRUE == pstcConfig->bCompareAllChannels)  ? 1u : 0u;
    
    if (pstcConfig->u8ComapreChannel > 31u)
    {
      return ErrorInvalidParameter;
    }
    stcCMPCR.CCH = pstcConfig->u8ComapreChannel;
    
    pstcAdc->CMPCR_f = stcCMPCR;
  }
  
  // Set enable time
  pstcAdc->ADCEN_f.ENBLTIME = pstcConfig->u8EnableTime;
  
  if (TRUE == pstcConfig->bRangeComparisonEnable)
  {
    pstcAdc->WCMPDH = ((0x0FFCu & pstcConfig->u16UpperLimitRangeValue) << 6u);
    pstcAdc->WCMPDL = ((0x0FFCu & pstcConfig->u16LowerLimitRangeValue) << 6u);
    
    if (pstcConfig->u8RangeCountValue > 7u)
    {
      return ErrorInvalidParameter;
    }
    pstcAdc->WCMPCR_f.RCOCD = pstcConfig->u8RangeCountValue;
    
    pstcAdc->WCMPCR_f.RCOIRRS = pstcConfig->bOutOfRange;
    pstcAdc->WCMPSR_f.WCMD = pstcConfig->bRangeCompareAllChannels;
    
    if (pstcConfig->u8RangeComapreChannel > 31u)
    {
      return ErrorInvalidParameter;
    }
    pstcAdc->WCMPSR_f.WCCH = pstcConfig->u8RangeComapreChannel;
    
    pstcAdc->WCMPCR_f.RCOE = pstcConfig->bRangeComparisonEnable;
  }
  
  // Set Interrupt Configuration, if ADC Interrupts are enabled in PDL
#if (PDL_INTERRUPT_ENABLE_ADC0 == PDL_ON) || \
    (PDL_INTERRUPT_ENABLE_ADC1 == PDL_ON) || \
    (PDL_INTERRUPT_ENABLE_ADC2 == PDL_ON)
      
  // Interrupt enable bits
  pstcAdc->ADCR_f.SCIE    = (TRUE == pstcConfig->bScanConversionIrqEnable)  ? 1u : 0u;
  pstcAdc->ADCR_f.PCIE    = (TRUE == pstcConfig->bPrioConversionIrqEnable)  ? 1u : 0u;
  pstcAdc->ADCR_f.CMPIE   = (TRUE == pstcConfig->bConversionCompIrqEnable)  ? 1u : 0u;
  pstcAdc->ADCR_f.OVRIE   = (TRUE == pstcConfig->bFifoOverrunIrqEnable)     ? 1u : 0u;
  pstcAdc->WCMPCR_f.RCOIE = (TRUE == pstcConfig->bRangeComparisonIrqEnable) ? 1u : 0u;
  
  // Callback pointers
  pstcAdcInternData->pfnScanConversion       = pstcConfig->pfnScanConversion;
  pstcAdcInternData->pfnPrioConversion       = pstcConfig->pfnPrioConversion;
  pstcAdcInternData->pfnErrorCallbackAdc     = pstcConfig->pfnErrorCallbackAdc;
  pstcAdcInternData->pfnPrioErrorCallbackAdc = pstcConfig->pfnPrioErrorCallbackAdc;
  pstcAdcInternData->pfnComparisonCallback   = pstcConfig->pfnComparisonCallback;
  pstcAdcInternData->pfnRangeCallback        = pstcConfig->pfnRangeCallback;

  Adc_InitInterrupt(pstcAdc);
#endif // #if (PDL_INTERRUPT_ENABLE_ADC0 == PDL_ON) || (PDL_INTERRUPT_ENABLE_ADC1 == PDL_ON) || (PDL_INTERRUPT_ENABLE_ADC2 == PDL_ON)

  return Ok;
} // Adc_Init

/**
 ******************************************************************************
 ** \brief De-Initialize ADC
 **
 ** This function deinitializes an ADC module
 **
 ** \param [in]  pstcAdc          ADC instance
 **
 ** \retval Ok                    ADC instance fully disabled and reset
 ** \retval ErrorInvalidParameter pstcAdc == NULL
 ******************************************************************************/
en_result_t Adc_DeInit( volatile stc_adcn_t* pstcAdc )
{
  // Check for NULL pointer
  if (pstcAdc  == NULL)
  {
    return ErrorInvalidParameter ;
  }
  
  pstcAdc->ADCEN   = 0u;   // Diasble ADC (including ENBL bit)
  
#if (PDL_INTERRUPT_ENABLE_ADC0 == PDL_ON) || \
    (PDL_INTERRUPT_ENABLE_ADC1 == PDL_ON) || \
    (PDL_INTERRUPT_ENABLE_ADC2 == PDL_ON)
      
  Adc_DeInitInterrupt(pstcAdc); // Disable Interrupts
  
#endif // #if (PDL_INTERRUPT_ENABLE_ADC0 == PDL_ON) || (PDL_INTERRUPT_ENABLE_ADC1 == PDL_ON) || (PDL_INTERRUPT_ENABLE_ADC2 == PDL_ON)
  
  pstcAdc->ADCR    = 0u;
  pstcAdc->ADSR    = 0u;
  pstcAdc->SCCR    = 0u;
  pstcAdc->SFNS    = 0u;
  pstcAdc->SCIS01  = 0u;
  pstcAdc->SCIS23  = 0u;
  pstcAdc->PCCR    = 0u;
  pstcAdc->PFNS    = 0u;
  pstcAdc->PCFD    = 0u;
  pstcAdc->PCIS    = 0u;
  pstcAdc->CMPD    = 0u;
  pstcAdc->CMPCR   = 0u;
  pstcAdc->ADSS01  = 0u;
  pstcAdc->ADSS23  = 0u;
  pstcAdc->ADST0   = 0u;
  pstcAdc->ADST1   = 0u;
  pstcAdc->ADCEN   = 0u;
  pstcAdc->WCMPDH  = 0u;
  pstcAdc->WCMPCR  = 0u;
  pstcAdc->WCMPDL  = 0u;
  pstcAdc->WCMPSR  = 0u;
  pstcAdc->WCMRCOT = 0u;
  pstcAdc->WCMRCIF = 0u;
  pstcAdc->SCTSL   = 0u;
  pstcAdc->PRTSL   = 0u;
  
  return Ok;
} // Adc_DeInit

/**
 ******************************************************************************
 ** \brief Enable ADC and wait for Readiness
 **
 ** This function enables an ADC instance and waits for readiness
 **
 ** \param [in]  pstcAdc          ADC instance
 **
 ** \retval Ok                    ADC instance enabled and ready
 ** \retval ErrorTimeout          ADC instance not ready
 ** \retval ErrorInvalidParameter pstcAdc == NULL
 ******************************************************************************/
en_result_t Adc_EnableWaitReady( volatile stc_adcn_t* pstcAdc )
{
  uint32_t u32TimeOutCounter = PDL_ADC_READY_WAIT_COUNT;
  
  // Check for NULL pointer
  if (pstcAdc  == NULL)
  {
    return ErrorInvalidParameter ;
  }
  
  // Enable ADC
  pstcAdc->ADCEN_f.ENBL = 1u;
  
  // Polling for readiness
  while (u32TimeOutCounter)
  {
    if (TRUE == pstcAdc->ADCEN_f.READY)
    {
      return Ok;
    }
    
    u32TimeOutCounter--;
  }
  
  return ErrorTimeout;
} // Adc_EnableWaitReady

/**
 ******************************************************************************
 ** \brief Enable ADC
 **
 ** This function enables an ADC instance (Does not wait for readiness)
 **
 ** \param [in]  pstcAdc          ADC instance
 **
 ** \retval Ok                    ADC enable bit set (does not indicate readiness!)
 ** \retval ErrorInvalidParameter pstcAdc == NULL
 ******************************************************************************/
en_result_t Adc_Enable( volatile stc_adcn_t* pstcAdc )
{
  // Check for NULL pointer
  if (pstcAdc  == NULL)
  {
    return ErrorInvalidParameter ;
  }
  
  // Enable ADC
  pstcAdc->ADCEN_f.ENBL = 1u;
  
  return Ok;
} // Adc_Enable

/**
 ******************************************************************************
 ** \brief Check ADC Readiness
 **
 ** This function checks for ADC instance readiness
 **
 ** \param [in]  pstcAdc          ADC instance
 **
 ** \retval Ok                    ADC instance ready
 ** \retval ErrorNotReady         ADC instance not ready
 ** \retval ErrorInvalidParameter pstcAdc == NULL
 ******************************************************************************/
en_result_t Adc_Ready( volatile stc_adcn_t* pstcAdc )
{
  // Check for NULL pointer
  if (pstcAdc  == NULL)
  {
    return ErrorInvalidParameter ;
  }
  
  if (FALSE == pstcAdc->ADCEN_f.READY)
  {
      return ErrorNotReady;
  }
  
  return Ok;
} // Adc_Ready

/**
 ******************************************************************************
 ** \brief ADC Scan Software Start Trigger
 **
 ** This function starts an AD Scan Conversion by Software trigger
 **
 ** \param [in]  pstcAdc          ADC instance
 **
 ** \retval Ok                    ADC instance triggered (or re-triggered)
 ** \retval ErrorInvalidParameter pstcAdc == NULL
 ******************************************************************************/
en_result_t Adc_ScanSwTrigger( volatile stc_adcn_t* pstcAdc )
{
  // Check for NULL pointer
  if (pstcAdc  == NULL)
  {
    return ErrorInvalidParameter ;
  }
  
  pstcAdc->SCCR_f.SSTR = 1u;     // Trigger AD Conversion
  
  return Ok;
} // Adc_SwStart

/**
 ******************************************************************************
 ** \brief ADC Priority Software Start Trigger
 **
 ** This function starts an AD Priority Conversion by Software trigger
 **
 ** \param [in]  pstcAdc          ADC instance
 **
 ** \retval Ok                    ADC instance triggered (or re-triggered)
 ** \retval ErrorInvalidParameter pstcAdc == NULL
 ******************************************************************************/
en_result_t Adc_PrioSwTrigger( volatile stc_adcn_t* pstcAdc )
{
  // Check for NULL pointer
  if (pstcAdc  == NULL)
  {
    return ErrorInvalidParameter ;
  }
  
  pstcAdc->PCCR_f.PSTR = 1u;     // Trigger AD Conversion
  
  return Ok;
}
                      
/**
 ******************************************************************************
 ** \brief ADC Software Stop
 **
 ** This function request a stop of the ADC
 **
 ** \param [in]  pstcAdc          ADC instance
 **
 ** \retval Ok                    ADC instance stop request
 ** \retval ErrorInvalidParameter pstcAdc == NULL
 ******************************************************************************/
en_result_t Adc_ForceStop( volatile stc_adcn_t* pstcAdc )
{
  // Check for NULL pointer
  if (pstcAdc  == NULL)
  {
    return ErrorInvalidParameter ;
  }
  
  pstcAdc->ADSR_f.ADSTP = 1u;
  
  return Ok;
} // Adc_SwStop

/**
 ******************************************************************************
 ** \brief Read Scan Conversion Status of ADC
 **
 ** This function reads out the Scan Conversion Status odf the ADC
 **
 ** \param [in]  pstcAdc          ADC instance
 **
 ** \retval Ok                    ADC instance ready
 ** \retval OperationInProgress   ADC instance not ready
 ** \retval ErrorInvalidParameter pstcAdc == NULL
 ******************************************************************************/
en_result_t Adc_ScanStatus( volatile stc_adcn_t* pstcAdc )
{
  // Check for NULL pointer
  if (pstcAdc  == NULL)
  {
    return ErrorInvalidParameter ;
  }
  
  if (1u == pstcAdc->ADSR_f.SCS)
  {
    return OperationInProgress;
  }
  
  return Ok;
} // Adc_ScanStatus

/**
 ******************************************************************************
 ** \brief Read Priority Conversion Status of ADC
 **
 ** This function reads out the Priority Conversion Status odf the ADC
 **
 ** \param [in]  pstcAdc          ADC instance
 **
 ** \retval Ok                    ADC instance no priority conversion ongoing
 ** \retval OperationInProgress   ADC instance priority conversion ongoing
 ** \retval ErrorInvalidParameter pstcAdc == NULL
 ******************************************************************************/
en_result_t Adc_PrioStatus( volatile stc_adcn_t* pstcAdc )
{
  // Check for NULL pointer
  if (pstcAdc  == NULL)
  {
    return ErrorInvalidParameter ;
  }
  
  if (1u == pstcAdc->ADSR_f.PCS)
  {
    return OperationInProgress;
  }
  
  return Ok;
} // Adc_PrioStatus

/**
 ******************************************************************************
 ** \brief Read Priority Level 2 Conversion Status of ADC
 **
 ** This function reads out the Priority Level 2 Conversion Status odf the ADC
 **
 ** \param [in]  pstcAdc          ADC instance
 **
 ** \retval Ok                    ADC instance no priority level 2 conversion
 **                               ongoing
 ** \retval OperationInProgress   ADC instance priority level 2 conversion
 **                               ongoing
 ** \retval ErrorInvalidParameter pstcAdc == NULL
 ******************************************************************************/
en_result_t Adc_Prio2Status( volatile stc_adcn_t* pstcAdc )
{
  // Check for NULL pointer
  if (pstcAdc  == NULL)
  {
    return ErrorInvalidParameter ;
  }
  
  if (1u == pstcAdc->ADSR_f.PCNS)
  {
    return OperationInProgress;
  }
  
  return Ok;
} // Adc_Prio2Status

/**
 ******************************************************************************
 ** \brief Read Scan Conversion FIFO state
 **
 ** This function reads out the state of the Scan Conversion FIFO
 **
 ** \param [in]  pstcAdc          ADC instance
 **
 ** \retval AdcFifoEmpty          Scan Conversion FIFO empty (no data)
 ** \retval AdcFifoFilled         Scan Conversion FIFO (partly) filled
 ** \retval AdcFifoFull           Scan Conversion FIFO full
 ** \retval AdcFifoOverrun        Scan Conversion FIFO overrun
 ** \retval AdcFifoError          Unknown error or pstcAdc == NULL
 ******************************************************************************/
en_adc_fifo_status_t Adc_ScanFifoStatus( volatile stc_adcn_t* pstcAdc )
{ 
	un_adc_sccr_t unAdcSccr;	
  // Check for NULL pointer
  if (pstcAdc  == NULL)
  {
    return AdcFifoError;
  }
  
  unAdcSccr.u8AdcSccrReadout = pstcAdc->SCCR;     // Get state of FIFO
  
  // Analyze read-out state
  if (1u == unAdcSccr.stcAdcSccrReadout.SOVR)
  {
    return AdcFifoOverrun;
  }
  
  if (1u == unAdcSccr.stcAdcSccrReadout.SFUL)
  {
    return AdcFifoFull;
  }

  if (0u == unAdcSccr.stcAdcSccrReadout.SEMP)
  {
    return AdcFifoFilled;
  }
  
  if (1u == unAdcSccr.stcAdcSccrReadout.SEMP)
  {
    return AdcFifoEmpty;
  }
  
  return AdcFifoError;  // Unknown error
} // Adc_ScanFifioStatus

/**
 ******************************************************************************
 ** \brief Clears the Scan Conversion FIFO
 **
 ** This function clears the Scan Conversion FIFO
 **
 ** \param [in]  pstcAdc          ADC instance
 **
 ** \retval Ok                    Scan Conversion FIFO cleared
 ** \retval ErrorInvalidParameter pstcAdc == NULL
 ******************************************************************************/
en_result_t Adc_ScanFifoClear( volatile stc_adcn_t* pstcAdc )
{
  // Check for NULL pointer
  if (pstcAdc  == NULL)
  {
    return ErrorInvalidParameter;
  }
  
  pstcAdc->SCCR_f.SFCLR = 1u;
  
  return Ok;
} // Adc_ScanFifioClear

/**
 ******************************************************************************
 ** \brief Read Priority Conversion FIFO state
 **
 ** This function reads out the state of the Priority Conversion FIFO
 **
 ** \param [in]  pstcAdc          ADC instance
 **
 ** \retval AdcFifoEmpty          Priority Conversion FIFO empty (no data)
 ** \retval AdcFifoFilled         Priority Conversion FIFO (partly) filled
 ** \retval AdcFifoFull           Priority Conversion FIFO full
 ** \retval AdcFifoOverrun        Priority Conversion FIFO overrun
 ** \retval AdcFifoError          Unknown error or pstcAdc == NULL
 ******************************************************************************/
en_adc_fifo_status_t Adc_PrioFifoStatus( volatile stc_adcn_t* pstcAdc )
{
  un_adc_pccr_t unAdcPccr;
  
  // Check for NULL pointer
  if (pstcAdc  == NULL)
  {
    return AdcFifoError;
  }
  
  unAdcPccr.u8AdcPccrReadout = pstcAdc->PCCR;     // Get state of FIFO
  
  // Analyze read-out state
  if (1u == unAdcPccr.stcAdcPccrReadout.POVR)
  {
    return AdcFifoOverrun;
  }
  
  if (1u == unAdcPccr.stcAdcPccrReadout.PFUL)
  {
    return AdcFifoFull;
  }

  if (0u == unAdcPccr.stcAdcPccrReadout.PEMP)
  {
    return AdcFifoFilled;
  }
  
  if (1u == unAdcPccr.stcAdcPccrReadout.PEMP)
  {
    return AdcFifoEmpty;
  }
  
  return AdcFifoError;  // Unknown error
} // Adc_PrioFifioStatus

/**
 ******************************************************************************
 ** \brief Clears the Priority Conversion FIFO
 **
 ** This function clears the Priority Conversion FIFO
 **
 ** \param [in]  pstcAdc          ADC instance
 **
 ** \retval Ok                    Priority Conversion FIFO cleared
 ** \retval ErrorInvalidParameter pstcAdc == NULL
 ******************************************************************************/
en_result_t Adc_PrioFifoClear( volatile stc_adcn_t* pstcAdc )
{
  // Check for NULL pointer
  if (pstcAdc  == NULL)
  {
    return ErrorInvalidParameter;
  }
  
  pstcAdc->PCCR_f.PFCLR = 1u;
  
  return Ok;
} // Adc_PrioFifioClear

/**
 ******************************************************************************
 ** \brief Reads out Scan Conversion FIFO
 **
 ** This function reads out the Scan Conversion FIFO.
 **
 ** \pre Adc_ScanFifioStatus() should be called before
 **
 ** \param [in]  pstcAdc        ADC instance
 **
 ** \return                     Recent Scan Conversion FIFO value including
 **                             INVL, RS1, RS0, and Channel data as is.
 **                             If pstcAdc == NULL 0xFFFFFFFF is returned.
 ******************************************************************************/
uint32_t Adc_ReadScanFifo( volatile stc_adcn_t* pstcAdc )
{
  // Check for NULL pointer
  if (pstcAdc  == NULL)
  {
    return 0xFFFFFFFFu;
  }
  
  return pstcAdc->SCFD;
} // Adc_ReadScanFifo

/**
 ******************************************************************************
 ** \brief Get Channel Number from FIFO data
 **
 ** This function returns the Channel data from input data from Scan Conversion
 ** FIFO
 **
 ** \pre Adc_ReadScanFifo() must be called before
 **
 ** \param [in]  u32FifoData    FIFO data
 **
 ** \return                     Recent Scan Conversion Channel value.
 ******************************************************************************/
uint8_t Adc_GetScanChannel( uint32_t u32FifoData )
{
  return (uint8_t) (0x0000001Fu & u32FifoData);
} // Adc_GetScanChannel

/**
 ******************************************************************************
 ** \brief Get Valid Data Flag from FIFO data
 **
 ** This function returns the Channel data from input data from FIFO
 **
 ** \pre Adc_ReadScanFifo() must be called before
 **
 ** \param [in]  u32FifoData      FIFO data
 **
 ** \retval  AdcFifoDataValid     Recent FIFO data valid
 ** \retval  AdcFifoDataInvalid   Recent FIFO data invalid
 ******************************************************************************/
en_adc_fifo_data_valid_t Adc_GetScanDataValid( uint32_t u32FifoData )
{
  if (0u == (0x00001000u & u32FifoData))
  {
    return AdcFifoDataValid;
  }
  
  return AdcFifoDataInvalid;
} // Adc_GetScanDataValid

/**
 ******************************************************************************
 ** \brief Get Scan Conversion Start Cause from FIFO data
 **
 ** This function returns the Scan Conversion Start Cause from FIFO Data
 **
 ** \pre Adc_ReadScanFifo() must be called before
 **
 ** \param [in]  u32FifoData      FIFO data
 **
 ** \retval  AdcFifoSoftwareStart   Recent FIFO data cuased by Software Start
 ** \retval  AdcFifoTimerStart      Recent FIFO data caused by Timer Start
 ** \retval  AdcFifoErrorStart      Recent FIFO data caused by unknown factor
 ******************************************************************************/
en_adc_fifo_start_cause_t Adc_GetScanDataCause( uint32_t u32FifoData )
{
  if (0x00000100u == (0x00000300u & u32FifoData))
  {
    return AdcFifoSoftwareStart;
  }

  if (0x00000200u == (0x00000300u & u32FifoData))
  {
    return AdcFifoTimerStart;
  }
  
  return AdcFifoErrorStart;
} // Adc_GetPDataCause

/**
 ******************************************************************************
 ** \brief Reads out Priority Conversion FIFO
 **
 ** This function reads out the Priority Conversion FIFO.
 **
 ** \pre Adc_PrioFifioStatus() should be called before
 **
 ** \param [in]  pstcAdc        ADC instance
 **
 ** \return                     Recent Priority Conversion FIFO value including
 **                             INVL, RS2, RS1, RS0, and Channel data as is.
 **                             If pstcAdc == NULL 0xFFFFFFFF is returned.
 ******************************************************************************/
uint32_t Adc_ReadPrioFifo( volatile stc_adcn_t* pstcAdc )
{
  // Check for NULL pointer
  if (pstcAdc  == NULL)
  {
    return 0xFFFFFFFFu;
  }
  
  return pstcAdc->PCFD;
} // Adc_ReadPrioFifo

/**
 ******************************************************************************
 ** \brief Get Channel Number from Priority FIFO data
 **
 ** This function returns the Channel data from input data from Priority
 ** Conversion FIFO
 **
 ** \pre Adc_ReadPrioFifo() must be called before
 **
 ** \param [in]  u32FifoData    Priority FIFO data
 **
 ** \return                     Recent Priority Conversion Channel value.
 ******************************************************************************/
uint8_t Adc_GetPrioChannel( uint32_t u32FifoData )
{
  return (uint8_t) (0x0000001Fu & u32FifoData);
} // Adc_GetPrioChannel

/**
 ******************************************************************************
 ** \brief Get Valid Data Flag from Priority FIFO data
 **
 ** This function returns the Channel data from input data from FIFO
 **
 ** \pre Adc_ReadPrioFifo() must be called before
 **
 ** \param [in]  u32FifoData      Priority FIFO data
 **
 ** \retval  AdcFifoDataValid     Recent FIFO data valid
 ** \retval  AdcFifoDataInvalid   Recent FIFO data invalid
 ******************************************************************************/
en_adc_fifo_data_valid_t Adc_GetPrioDataValid( uint32_t u32FifoData )
{
  if (0u == (0x00001000u & u32FifoData))
  {
    return AdcFifoDataValid;
  }
  
  return AdcFifoDataInvalid;
} // Adc_GetPrioDataValid

/**
 ******************************************************************************
 ** \brief Get Priority Conversion Start Cause from FIFO data
 **
 ** This function returns the Priority Conversion Start Cause from FIFO Data
 **
 ** \pre Adc_ReadPrioFifo() must be called before
 **
 ** \param [in]  u32FifoData         Priority FIFO data
 **
 ** \retval  AdcFifoSoftwareStart    Recent FIFO data cuased by Software Start
 ** \retval  AdcFifoTimerStart       Recent FIFO data causde by Timer Start
 ** \retval  AdcFifoExternalTrigger  Recent FIFO data causde by External Trigger
 ** \retval  AdcFifoErrorStart       Recent FIFO data causde by unknown factor
 ******************************************************************************/
en_adc_fifo_start_cause_t Adc_GetPrioDataCause( uint32_t u32FifoData )
{
  if (0x00000100u == (0x00000700u & u32FifoData))
  {
    return AdcFifoSoftwareStart;
  }

  if (0x00000200u == (0x00000700u & u32FifoData))
  {
    return AdcFifoTimerStart;
  }
  
  if (0x00000400u == (0x00000700u & u32FifoData))
  {
    return AdcFifoExternalTrigger;
  }
  
  return AdcFifoErrorStart;
} // Adc_GetPrioDataCause

/**
 ******************************************************************************
 ** \brief Read Raneg Comparison Result
 **
 ** This function reads out the Range Comparison Flag
 **
 ** \param [in]  pstcAdc             ADC instance
 **
 ** \retval  AdcRangeResultValid     Range result valid (occurred)
 ** \retval  AdcRangeResultInvalid   Range result invalid (not occurred)
 ** \retval  AdcRangeResultError     pstcAdc == NULL
 ******************************************************************************/
en_adc_range_result_t Adc_GetRangeResultValid( volatile stc_adcn_t* pstcAdc )
{
  // Check for NULL pointer
  if (pstcAdc  == NULL)
  {
    return AdcRangeResultError;
  }
  
  if (1u == pstcAdc->WCMRCIF_f.RCINT)
  {
    return AdcRangeResultValid;
  }
  
  return AdcRangeResultInvalid;
} // Adc_GetRangeResultValid

//@} // AdcGroup

#endif // #if (defined(PDL_PERIPHERAL_ADC_ACTIVE))

/******************************************************************************/
/* EOF (not truncated)                                                        */
/******************************************************************************/
