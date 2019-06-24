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
/** \file adc.h
 **
 ** Headerfile for ADC functions
 **  
 ** History:
 **   - 2013-02-26  1.0  MWi  First version.
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

#ifndef __ADC_H__
#define __ADC_H__

/******************************************************************************/
/* Include files                                                              */
/******************************************************************************/
#include "pdl.h"

#if (defined(PDL_PERIPHERAL_ADC_ACTIVE))

/* C binding of definitions if building with C++ compiler */
#ifdef __cplusplus
extern "C"
{
#endif
  
/**
 ******************************************************************************
 ** \defgroup AdcGroup 12-Bit Analog Digital Converter (ADC)
 **
 ** Provided functions of ADC module:
 ** - Adc_Init()
 ** - Adc_DeInit()
 ** - Adc_EnableWaitReady()
 ** - Adc_Enable()
 ** - Adc_Ready()
 ** - Adc_ScanSwTrigger()
 ** - Adc_ForceStop()
 ** - Adc_ScanStatus()
 ** - Adc_PrioStatus()
 ** - Adc_Prio2Status()
 ** - Adc_ScanFifoStatus()
 ** - Adc_ScanFifoClear()
 ** - Adc_PrioFifoStatus()
 ** - Adc_PrioFifoClear()
 ** - Adc_ReadScanFifo()
 ** - Adc_GetScanChannel()
 ** - Adc_GetScanDataValid()
 ** - Adc_GetScanDataCause()
 ** - Adc_ReadPrioFifo()
 ** - Adc_GetPrioChannel()
 ** - Adc_GetPrioDataValid()
 ** - Adc_GetPrioDataCause()
 ** - Adc_GetRangeResultValid()
 **
 ** Adc_Init() initializes an ADC instance according the configuration
 ** of the structure type #stc_adc_config_t. Adc_DeInit() de-initializes an
 ** ADC insstance.
 **
 ** For using the ADC instnace Adc_Enable() can be used. In this case the
 ** user has to poll the function Adc_Ready(). The advantage is that during
 ** polling the application can do other tasks. If this is not needed the
 ** function Adc_EnableWaitReady() can be used. Note that depending on the
 ** used device the time-out definition #PDL_ADC_READY_WAIT_COUNT may be need
 ** to be adjusted.
 **
 ** Adc_ScanSwStart() and Adc_PrioSwStop() can be used for software trigger 
 ** start for scan and priority conversion.
 **
 ** Adc_ForceStop() stops both ADC scan and priority conversion forcely.
 **
 ** Adc_ScanStatus(), Adc_PrioStatus(), Adc_Prio2Status() return the status
 ** of the scanning process (ongoing or ready).
 **
 ** Adc_ScanFifioStatus() and Adc_PrioFifioStatus() return the current status
 ** of the scan conversion and priority conversion result FIFO by the type
 ** #en_adc_fifo_status_t.
 **
 ** Adc_ScanFifioClear() and Adc_PrioFifioClear() clear the FIFOs.
 **
 ** Adc_ReadScanFifo() and Adc_ReadPrioFifo() read out the FIFOs.
 **
 ** Adc_GetScanChannel() and Adc_GetPrioChannel() extract the channel number
 ** from the raw FIFO data.
 **
 ** Adc_GetScanDataValid() and Adc_GetPrioDataValid() check, if FIFO data is
 ** valid.
 **
 ** Adc_GetScanDataCause() and Adc_GetPrioDataCause() return the conversion
 ** factor from the FIFO data.
 **
 ** Adc_GetRangeResultValid() returns the range comparison factor of the type
 ** of #en_adc_range_result_t.
 **
 **
 ** \note The callback arguments for the scan and priority result point to
 ** the according FIFO registers of the recent instance. The application must
 ** take care of emptying these FIFOs. Either the FIFO depth of the
 ** configuration can be used for counting or the functions
 ** Adc_ScanFifioStatus() or Adc_PrioFifioStatus() can be used.
 **
 ******************************************************************************/
//@{
  
/******************************************************************************
 * Global definitions
 ******************************************************************************/
#define stc_adcn_t   FM4_ADC_TypeDef
#define ADC0       (*((volatile stc_adcn_t *) FM4_ADC0_BASE))
#define ADC1       (*((volatile stc_adcn_t *) FM4_ADC1_BASE))
#define ADC2       (*((volatile stc_adcn_t *) FM4_ADC2_BASE))
  
#define PDL_ADC_READY_WAIT_COUNT 1000000u        ///< Time-out counter value for ADC ready bit polling
  
#define ADC_INSTANCE_COUNT                                \
        (uint8_t)(PDL_PERIPHERAL_ENABLE_ADC0 == PDL_ON) + \
        (uint8_t)(PDL_PERIPHERAL_ENABLE_ADC1 == PDL_ON) + \
        (uint8_t)(PDL_PERIPHERAL_ENABLE_ADC2 == PDL_ON)
  
/******************************************************************************
 * Global type definitions
 ******************************************************************************/
typedef void (*func_ptr_adc_parg32_t)(uint32_t* pu32Argument);

/**
 ******************************************************************************
 ** \brief Scan Conversion Mode
 ******************************************************************************/
typedef enum en_adc_scan_mode
{
  ScanSingleConversion  = 0,        ///< Single mode of scan conversion
  ScanRepeatConversion  = 1         ///< Repeat mode of scan conversion
} en_adc_scan_mode_t;

/**
 ******************************************************************************
 ** \brief Priority Conversion Stage Count
 ******************************************************************************/
typedef enum en_adc_sample_time_n
{
  Value1        = 0,    ///< Set value * 1
  Value4        = 1,    ///< Set value * 4
  Value8        = 2,    ///< Set value * 8
  Value16       = 3,    ///< Set value * 16
  Value32       = 4,    ///< Set value * 32
  Value64       = 5,    ///< Set value * 64
  Value128      = 6,    ///< Set value * 128
  Value256      = 7,    ///< Set value * 256
} en_adc_sample_time_n_t;

/**
 ******************************************************************************
 ** \brief FIFO Status (Scan, Priority Conversion)
 ******************************************************************************/
typedef enum en_adc_fifo_status
{
  AdcFifoEmpty     = 0,    ///< Scan FIFO empfty
  AdcFifoFilled    = 1,    ///< Scan FIFO (partly) filled
  AdcFifoFull      = 2,    ///< Scan FIFO full
  AdcFifoOverrun   = 3,    ///< Scan FIFO overrun
  AdcFifoError     = 4     ///< Unknown Fifo Error (no bit set) or pstcAdc == NULL
} en_adc_fifo_status_t;

/**
 ******************************************************************************
 ** \brief FIFO valid data result (Scan, Priority Conversion)
 ******************************************************************************/
typedef enum en_adc_fifo_data_valid
{
  AdcFifoDataInvalid  = 0,    ///< FIFO data invalid
  AdcFifoDataValid    = 1    ///< FIFO data valid
} en_adc_fifo_data_valid_t;

/**
 ******************************************************************************
 ** \brief FIFO data start cause
 ******************************************************************************/
typedef enum en_adc_fifo_start_cause
{
  AdcFifoSoftwareStart   = 0,    ///< FIFO data software start cause (Scan & Prio2 Conversion)
  AdcFifoTimerStart      = 1,    ///< FIFO data timer start cause (Scan & Prio2 Conversion)
  AdcFifoExternalTrigger = 2,    ///< FIFO data external trigger cause (Prio1 Conversion)
  AdcFifoErrorStart      = 3     ///< FIFO data caused by unknown factor (Scan & Prio2 Conversion)
} en_adc_fifo_start_cause_t;

/**
 ******************************************************************************
 ** \brief Range Comparison Factor
 ******************************************************************************/
typedef enum en_adc_range_result
{
  AdcRangeResultValid    = 0,   ///< Range result valid (occurred)
  AdcRangeResultInvalid  = 1,   ///< Range result invalid (not occurred)
  AdcRangeResultError    = 2    ///< pstcAdc == NULL
} en_adc_range_result_t;

/**
 ******************************************************************************
 ** \brief ADC scan/priority conversion timer trigger selection
 ** \note Do not change the numbering.
 ******************************************************************************/
typedef enum en_adc_timer_select
{
  AdcNoTimer =  0,   ///< No selected trigger
  AdcMft     =  1,   ///< Trigger by Multifunction Timer
  AdcBt0     =  2,   ///< Trigger with Base Timer 0
  AdcBt1     =  3,   ///< Trigger with Base Timer 1
  AdcBt2     =  4,   ///< Trigger with Base Timer 2
  AdcBt3     =  5,   ///< Trigger with Base Timer 3
  AdcBt4     =  6,   ///< Trigger with Base Timer 4
  AdcBt5     =  7,   ///< Trigger with Base Timer 5
  AdcBt6     =  8,   ///< Trigger with Base Timer 6
  AdcBt7     =  9,   ///< Trigger with Base Timer 7
  AdcBt8     = 10,   ///< Trigger with Base Timer 8
  AdcBt9     = 11,   ///< Trigger with Base Timer 9
  AdcBt10    = 12,   ///< Trigger with Base Timer 10
  AdcBt11    = 13,   ///< Trigger with Base Timer 11
  AdcBt12    = 14,   ///< Trigger with Base Timer 12
  AdcBt13    = 15,   ///< Trigger with Base Timer 13  
} en_adc_timer_select_t;

/**
 ******************************************************************************
 ** \brief ADC Channel list with same sort order as the ER32 registers
 ******************************************************************************/
typedef struct stc_ad_channel_list
{
  union unAdcChannels
  {
    uint32_t u32AD_CHn;
    struct stcChannelBits
    {
      uint32_t AD_CH_0  : 1 ;
      uint32_t AD_CH_1  : 1 ;
      uint32_t AD_CH_2  : 1 ;
      uint32_t AD_CH_3  : 1 ;
      uint32_t AD_CH_4  : 1 ;
      uint32_t AD_CH_5  : 1 ;
      uint32_t AD_CH_6  : 1 ;
      uint32_t AD_CH_7  : 1 ;
      uint32_t AD_CH_8  : 1 ;
      uint32_t AD_CH_9  : 1 ;
      uint32_t AD_CH_10 : 1 ;
      uint32_t AD_CH_11 : 1 ;
      uint32_t AD_CH_12 : 1 ;
      uint32_t AD_CH_13 : 1 ;
      uint32_t AD_CH_14 : 1 ;
      uint32_t AD_CH_15 : 1 ;
      uint32_t AD_CH_16 : 1 ;
      uint32_t AD_CH_17 : 1 ;
      uint32_t AD_CH_18 : 1 ;
      uint32_t AD_CH_19 : 1 ;
      uint32_t AD_CH_20 : 1 ;
      uint32_t AD_CH_21 : 1 ;
      uint32_t AD_CH_22 : 1 ;
      uint32_t AD_CH_23 : 1 ;
      uint32_t AD_CH_24 : 1 ;
      uint32_t AD_CH_25 : 1 ;
      uint32_t AD_CH_26 : 1 ;
      uint32_t AD_CH_27 : 1 ;
      uint32_t AD_CH_28 : 1 ;
      uint32_t AD_CH_29 : 1 ;
      uint32_t AD_CH_30 : 1 ;
      uint32_t AD_CH_31 : 1 ;
    };
  };
} stc_ad_channel_list_t ;

/**
 ******************************************************************************
 ** \brief ADC configuration.
 ******************************************************************************/
typedef struct stc_adc_config
{
  stc_ad_channel_list_t         u32CannelSelect;                ///< Selects channels for ADC (1 = active)
  boolean_t                     bLsbAlignment;                  ///< TURE: Result is LSB aligned
  en_adc_scan_mode_t            enScanMode;                     ///< Conversion Mode. See #en_adc_scan_mode_t for details
  stc_ad_channel_list_t         u32SamplingTimeSelect;          ///< Selects channels for Sampling time 0 or 1 setting
  en_adc_sample_time_n_t        enSamplingTimeN0;               ///< Sampling Time N0 Time Setting Bits, see #en_adc_sample_time_n_t for details
  uint8_t                       u8SamplingTime0;                ///< Sampling Time 0 0...15
  en_adc_sample_time_n_t        enSamplingTimeN1;               ///< Sampling Time N1 Time Setting Bits, see #en_adc_sample_time_n_t for details
  uint8_t                       u8SamplingTime1;                ///< Sampling Time 1 0...15
  uint8_t                       u8SamplingMultiplier;           ///< Multiplier of N, see Peripheral Manual for Details!
  uint8_t                       u8EnableTime;                   ///< Enable Time
  boolean_t                     bScanTimerStartEnable;          ///< TRUE: Triggers Scan Conversion by Timer
  en_adc_timer_select_t         enScanConversionTimerTrigger;   ///< See #en_adc_timer_select_t for details
  uint8_t                       u8ScanFifoDepth;                ///< Depth of the Scan Conversion FIFO 0...15
  boolean_t                     bPrioExtTrigEnable;             ///< TRUE: The external trigger analog inputs are selected with an external input.
                                                                ///< FALSE: The external trigger analog inputs are selected with P1A[2:0].
  boolean_t                     bPrioExtTrigStartEnable;        ///< TRUE: Triggers Prioritys Conversion on falling edge of external signal
  boolean_t                     bPrioTimerStartEnable;          ///< TRUE: Triggers Priority Conversion by Timer
  en_adc_timer_select_t         enPrioConversionTimerTrigger;   ///< See #en_adc_timer_select_t for details
  uint8_t                       u8PrioFifoDepth;                 ///< Depth of the Priority Conversion FIFO 0...3
  uint8_t                       u8PrioLevel1AnalogChSel;        ///< Priority Level 1 Analog Channel Selector Ch. 0...7
  uint8_t                       u8PrioLevel2AnalogChSel;        ///< Priority Level 2 Analog Channel Selector Ch. 0...31
  boolean_t                     bComparisonEnable;              ///< TRUE: Enables Comparison Mode
  uint16_t                      u16ComparisonValue;             ///< ADC Comparison Value (CMPD)

  boolean_t                     bCompareAllChannels;            ///< TRUE: Compare all selected Channels, FALSE: Compare CCH-Channel
  uint8_t                       u8ComapreChannel;               ///< CCH-Channel to be compared, if selected Ch. 0...31
  boolean_t                     bRangeComparisonEnable;         ///< TRUE: Range Comparison is enabled
  uint16_t                      u16UpperLimitRangeValue;        ///< Upper Limit Value for Range Comparison
  uint16_t                      u16LowerLimitRangeValue;        ///< Lower limit Value for Range Comparison
  uint8_t                       u8RangeCountValue;              ///< Range Count Value 1...7
  boolean_t                     bOutOfRange;                    ///< TRUE: Value out of Range, FALSE: Value within Range
  boolean_t                     bRangeCompareAllChannels;       ///< TRUE: Range compare all selected Channels, FALSE: Compare WCCH-Channel
  uint8_t                       u8RangeComapreChannel;          ///< WCCH-Channel to be Range compared, if selected Ch. 0...31  
#if (PDL_INTERRUPT_ENABLE_ADC0 == PDL_ON) || \
    (PDL_INTERRUPT_ENABLE_ADC1 == PDL_ON) || \
    (PDL_INTERRUPT_ENABLE_ADC2 == PDL_ON)
  boolean_t                     bCompIrqEqualGreater;           ///< TRUE: Generate Interrupt, if CMPD most significant 10 bis >= current ADC value
                                                                ///< FALSE: Generate Interrupt, if CMPD most significant 10 bis < current ADC value
  boolean_t                     bScanConversionIrqEnable;       ///< TRUE: Enable Scan Conversion Interrupt
  func_ptr_adc_parg32_t         pfnPrioConversion;              ///< Priority conversion callback
  boolean_t                     bPrioConversionIrqEnable;       ///< TRUE: Enable Priority Conversion Interrupt
  boolean_t                     bConversionCompIrqEnable;       ///< TRUE: Enable Conversion Comparison Interrupt
  boolean_t                     bFifoOverrunIrqEnable;          ///< TRUE: Enable FIFO Overrun Interrupt
  boolean_t                     bRangeComparisonIrqEnable;      ///< TRUE: Enable Range Comparison Interrupt
  func_ptr_adc_parg32_t         pfnScanConversion;              ///< Scanconversion callback
  func_ptr_t                    pfnErrorCallbackAdc;            ///< Error during conversion
  func_ptr_t                    pfnPrioErrorCallbackAdc;        ///< Priority FIFO overrun error callback pointer
  func_ptr_t                    pfnComparisonCallback;          ///< Comparison callback pointer
  func_ptr_t                    pfnRangeCallback;               ///< Range condition interrupt callback pointer
#endif // #if (PDL_INTERRUPT_ENABLE_ADC0 == PDL_ON) || (PDL_INTERRUPT_ENABLE_ADC1 == PDL_ON) || (PDL_INTERRUPT_ENABLE_ADC2 == PDL_ON)
} stc_adc_config_t;


/// Enumeration to define an index for each enabled ADC instance
typedef enum en_adc_instance_index
{
  #if (PDL_PERIPHERAL_ENABLE_ADC0 == PDL_ON)
    AdcInstanceIndexAdc0,
  #endif
  #if (PDL_PERIPHERAL_ENABLE_ADC1 == PDL_ON)
    AdcInstanceIndexAdc1,
  #endif
  #if (PDL_PERIPHERAL_ENABLE_ADC2 == PDL_ON)
    AdcInstanceIndexAdc2,
  #endif
    AdcInstanceIndexMax
} en_adc_instance_index_t;

/// ADC instance internal data, storing internal information for each enabled ADC instance.
typedef struct stc_adc_intern_data
{
  func_ptr_adc_parg32_t   pfnScanConversion;            ///< Scan conversion callback
  func_ptr_adc_parg32_t   pfnPrioConversion;            ///< Priority conversion callback
  func_ptr_t              pfnErrorCallbackAdc;          ///< Error during conversion
  func_ptr_t              pfnPrioErrorCallbackAdc;      ///< Priority FIFO overrun error callback pointer
  func_ptr_t              pfnComparisonCallback;        ///< Comparison callback pointer
  func_ptr_t              pfnRangeCallback;             ///< Range condition interrupt callback pointer
} stc_adc_intern_data_t;

/// ADC instance data type
typedef struct stc_adc_instance_data
{
  volatile stc_adcn_t*  pstcInstance;  ///< pointer to registers of an instance
  stc_adc_intern_data_t stcInternData; ///< module internal data of instance
} stc_adc_instance_data_t;

/******************************************************************************/
/* Global variable declarations ('extern', definition in C source)            */
/******************************************************************************/

/// Look-up table for all enabled ADC instances and their internal data
extern stc_adc_instance_data_t m_astcAdcInstanceDataLut[ADC_INSTANCE_COUNT];

/******************************************************************************/
/* Global function prototypes ('extern', definition in C source)              */
/******************************************************************************/
extern void                      AdcIrqHandler( volatile stc_adcn_t*   pstcAdc,
                                                stc_adc_intern_data_t* pstcAdcInternData );
extern en_result_t               Adc_Init( volatile stc_adcn_t* pstcAdc,
                                           stc_adc_config_t*    pstcConfig );
extern en_result_t               Adc_DeInit( volatile stc_adcn_t* pstcAdc );
extern en_result_t               Adc_EnableWaitReady( volatile stc_adcn_t* pstcAdc );
extern en_result_t               Adc_Enable( volatile stc_adcn_t* pstcAdc );
extern en_result_t               Adc_Ready( volatile stc_adcn_t* pstcAdc );
extern en_result_t               Adc_ScanSwTrigger( volatile stc_adcn_t* pstcAdc );
extern en_result_t               Adc_PrioSwTrigger( volatile stc_adcn_t* pstcAdc );
extern en_result_t               Adc_ForceStop( volatile stc_adcn_t* pstcAdc );
extern en_result_t               Adc_ScanStatus( volatile stc_adcn_t* pstcAdc );
extern en_result_t               Adc_PrioStatus( volatile stc_adcn_t* pstcAdc );
extern en_result_t               Adc_Prio2Status( volatile stc_adcn_t* pstcAdc );
extern en_adc_fifo_status_t      Adc_ScanFifoStatus( volatile stc_adcn_t* pstcAdc );
extern en_result_t               Adc_ScanFifoClear( volatile stc_adcn_t* pstcAdc );
extern en_adc_fifo_status_t      Adc_PrioFifoStatus( volatile stc_adcn_t* pstcAdc );
extern en_result_t               Adc_PrioFifoClear( volatile stc_adcn_t* pstcAdc );
extern uint32_t                  Adc_ReadScanFifo( volatile stc_adcn_t* pstcAdc );
extern uint8_t                   Adc_GetScanChannel( uint32_t u32FifoData );
extern en_adc_fifo_data_valid_t  Adc_GetScanDataValid( uint32_t u32FifoData );
extern en_adc_fifo_start_cause_t Adc_GetScanDataCause( uint32_t u32FifoData );
extern uint32_t                  Adc_ReadPrioFifo( volatile stc_adcn_t* pstcAdc );
extern uint8_t                   Adc_GetPrioChannel( uint32_t u32FifoData );
extern en_adc_fifo_data_valid_t  Adc_GetPrioDataValid( uint32_t u32FifoData );
extern en_adc_fifo_start_cause_t Adc_GetPrioDataCause( uint32_t u32FifoData );
extern en_adc_range_result_t     Adc_GetRangeResultValid( volatile stc_adcn_t* pstcAdc );

//@}
#ifdef __cplusplus
}
#endif

#endif // #if (defined(PDL_PERIPHERAL_ADC_ACTIVE))

#endif /* __ADC_H__ */
/******************************************************************************/
/* EOF (not truncated)                                                        */
/******************************************************************************/
