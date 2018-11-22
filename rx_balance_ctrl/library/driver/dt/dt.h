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
/** \file dt.h
 **
 ** Headerfile for DT functions
 **  
 **
 ** History:
 **   - 2013-04-01  1.0  NT   First version.
 ******************************************************************************/

#ifndef __DT_H__
#define __DT_H__

/******************************************************************************/
/* Include files                                                              */
/******************************************************************************/
#include "pdl.h"

#if (defined(PDL_PERIPHERAL_DT_ACTIVE))

/* C binding of definitions if building with C++ compiler */
#ifdef __cplusplus
extern "C"
{
#endif

/**
 ******************************************************************************
 ** \defgroup DtGroup Dual Timer (DT)
 **
 ** Provided functions of DT module:
 ** 
 ** - Dt_Init()
 ** - Dt_DeInit() 
 ** - Dt_EnableCount()
 ** - Dt_DisableCount()
 ** - Dt_EnableInt()
 ** - Dt_DisableInt()
 ** - Dt_GetIntFlag()
 ** - Dt_GetMaskIntFlag()
 ** - Dt_ClrIntFlag()
 ** - Dt_WriteLoadVal()
 ** - Dt_WriteBgLoadVal()
 ** - Dt_ReadCurCntVal()
 **
 ** \brief How to use DT module
 **
 ** Dt_Init() must be used for configuration of a Dual Timer (DT) channel
 ** a structure with the type #stc_dt_channel_config_t.
 **
 ** A DT Channel can be enabled by the function Dt_EnableCount().
 ** Depending on the used mode, it is started in:
 ** - Free-run mode
 ** - Periodic mode
 ** - One-shot mode
 **
 ** A DT interruption can be enabled by the function Dt_EnableInt().
 ** This function can set callback function for each channel too.
 ** Before enabling a DT channel by Dt_EnableCount(), it is recommended
 ** to call this function.
 **
 ** With Dt_WriteLoadVal() the recent DT counter is set to the value
 ** given in the parameter Dt_WriteLoadVal#u32LoadVal.
 ** This works in each of the three operation modes.
 **
 ** With Dt_WriteBgLoadVal() the background reload value can be set,
 ** which is then set to the load value after the DT counter reaches the
 ** next 0.
 **
 ** With Dt_ReadCurCntVal() the current DT counter can be read.
 **
 ** Before deinitialization of the DT by Dt_DeInit(), it is recommended
 ** to disable all channels via Dt_DisableCount() and Dt_DisableInt() before,
 ** to avoid a possible, unwanted interrupt.
 **
 ******************************************************************************/
//@{

/******************************************************************************
 * Global type definitions
 ******************************************************************************/
/**
 ******************************************************************************
 ** \brief Redefinition of DT register structure 
 ******************************************************************************/
typedef FM4_DTIM_TypeDef stc_dtn_t;

/**
 ******************************************************************************
 ** \brief Dt mode
 ** 
 ** To select between Free-run, Periodic, and One-Shot mode
 ******************************************************************************/
typedef enum en_dt_mode
{
    DtFreeRun      = 0,  ///< Free-running mode
    DtPeriodic     = 1,  ///< Periodic mode
    DtOneShot      = 2   ///< One-shot mode
} en_dt_mode_t;

/**
 ******************************************************************************
 ** \brief Dt Prescaler
 ** 
 ** To select clock divider
 ******************************************************************************/
typedef enum en_dt_prescaler
{
    DtPrescalerDiv1     = 0,  ///< Prescaler divisor 1
    DtPrescalerDiv16    = 1,  ///< Prescaler divisor 16
    DtPrescalerDiv256   = 2   ///< Prescaler divisor 256
} en_dt_prescaler_t;

/**
 ******************************************************************************
 ** \brief Dt Counter Size
 ** 
 ** To select the size of the counter
 ******************************************************************************/
typedef enum en_dt_countersize
{
    DtCounterSize16     = 0,  ///< 16 Bit counter size
    DtCounterSize32     = 1   ///< 32 Bit counter size
} en_dt_countersize_t;

/**
 ******************************************************************************
 ** \brief Dt channel number
 ******************************************************************************/
typedef enum en_dt_channel
{
    DtChannel0     = 0,  ///< channel 0
    DtChannel1     = 1,  ///< channel 1
    DtMaxChannels  = 2   ///< Number of channels
} en_dt_channel_t;

/**
 *****************************************************************************
 ** \brief Dt channel configuration
 ** 
 ** The DT configuration is done on a per channel basis
 *****************************************************************************/
typedef struct stc_dt_channel_config
{
  uint8_t u8Mode;           ///< Mode, see description of #en_dt_mode_t
  uint8_t u8PrescalerDiv;   ///< Prescaler, see description of #en_dt_prescaler_t
  uint8_t u8CounterSize;    ///< Counter size, see description of #en_dt_countersize_t
} stc_dt_channel_config_t;

/**
 ******************************************************************************
 ** \brief DtCallback function prototype.
 ******************************************************************************/
typedef void (*dt_cb_func_ptr_t)(void);

/******************************************************************************
 * Global definitions
 ******************************************************************************/
#define DT0     (*((volatile stc_dtn_t *) FM4_DTIM_BASE))

/* for TimerPre of TimerXControl */
#define DT_PRE_TIMER_DIV_1      (0x00)
#define DT_PRE_TIMER_DIV_16     (0x01)
#define DT_PRE_TIMER_DIV_256    (0x02)

/******************************************************************************/
/* Local type definitions ('typedef')                                         */
/******************************************************************************/

/// Enumeration to define an index for each enabled Dual timer instance
typedef enum en_dt_instance_index
{
#if (PDL_PERIPHERAL_ENABLE_DT0 == PDL_ON)
    DtInstanceIndexDt0,
#endif
    DtInstanceIndexMax
} en_dt_instance_index_t;

/// Datatype for holding internal data needed for DT
typedef struct stc_dt_intern_data
{
    /// Callback for interrupts of DT channel 0
    dt_cb_func_ptr_t  pfnIntCallbackIntern[DtMaxChannels] ;
} stc_dt_intern_data_t ;

/// DT instance data type
typedef struct stc_dt_instance_data
{
    volatile stc_dtn_t*  pstcInstance;  ///< pointer to registers of an instance
    stc_dt_intern_data_t stcInternData; ///< module internal data of instance
} stc_dt_instance_data_t;

/******************************************************************************/
/* Global variable definitions ('extern')                                     */
/******************************************************************************/

/******************************************************************************/
/* Global function prototypes (definition in C source)                        */
/******************************************************************************/
/* External function  */
extern void DtIrqHandler(uint8_t u8Ch);

/* Init/Deinit */
extern en_result_t Dt_Init(stc_dt_channel_config_t* pstcConfig,
                           uint8_t                  u8Ch
                          );
extern en_result_t Dt_DeInit(uint8_t u8Ch);

/* Function/Int Enable/Disable */
extern en_result_t Dt_EnableCount(uint8_t u8Ch);
extern en_result_t Dt_DisableCount(uint8_t u8Ch);

extern en_result_t Dt_EnableInt(dt_cb_func_ptr_t pfnIntCallback,
                                uint8_t          u8Ch
                               );
extern en_result_t Dt_DisableInt(uint8_t u8Ch);

/* Get/Clr Status/Int flag */
extern boolean_t Dt_GetIntFlag(uint8_t u8Ch);
extern boolean_t Dt_GetMaskIntFlag(uint8_t u8Ch);
extern en_result_t Dt_ClrIntFlag(uint8_t u8Ch);

/* Write/Read count value */
extern en_result_t Dt_WriteLoadVal(uint32_t u32LoadVal,
                                   uint8_t  u8Ch
                                  );
extern en_result_t Dt_WriteBgLoadVal(uint32_t u32BgLoadVal,
                                     uint8_t  u8Ch
                                    );

extern uint32_t Dt_ReadCurCntVal(uint8_t u8Ch);

//@} // DtGroup

#ifdef __cplusplus
}
#endif

#endif /* #if (defined(PDL_PERIPHERAL_DT_ACTIVE)) */

#endif /* __DT_H__ */
/******************************************************************************/
/* EOF (not truncated)                                                        */
/******************************************************************************/
