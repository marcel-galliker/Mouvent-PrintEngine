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
/** \file dt.c
 **
 ** A detailed description is available at 
 ** @link DtGroup DT Module description @endlink
 **
 ** History:
 **   - 2013-04-01  1.0  NT   First version.
 **
 ******************************************************************************/

/******************************************************************************/
/* Include files                                                              */
/******************************************************************************/
#include "pdl.h"

#if (defined(PDL_PERIPHERAL_DT_ACTIVE))

/**
 ******************************************************************************
 ** \addtogroup DtGroup
 ******************************************************************************/
//@{

/******************************************************************************/
/* Local pre-processor symbols/macros ('#define')                             */
/******************************************************************************/

/******************************************************************************/
/* Global variable definitions (declared in header file with 'extern')        */
/******************************************************************************/

/// Look-up table for all enabled DT instances and their internal data
static stc_dt_instance_data_t m_astcDtInstanceDataLut[DtInstanceIndexMax] =
{
#if (PDL_PERIPHERAL_ENABLE_DT0 == PDL_ON)
    {
        &DT0,   /* pstcInstance */
        {NULL}  /* stcInternData (not initialized yet) */
    }
#endif
};

/******************************************************************************/
/* Local type definitions ('typedef')                                         */
/******************************************************************************/


/******************************************************************************/
/* Local function prototypes ('static')                                       */
/******************************************************************************/
static stc_dt_intern_data_t* DtGetInternDataPtr(volatile stc_dtn_t** ppstcDt,
                                                uint8_t              u8Ch);
static void Dt_InitIrq(void);
static void Dt_DeInitIrq(void);

/******************************************************************************/
/* Local variable definitions ('static')                                      */
/******************************************************************************/

/*****************************************************************************/
/* Function implementation - global ('extern') and local ('static')          */
/*****************************************************************************/

/**
 ******************************************************************************
 ** \brief Check pointer to Dual Timer instance and enable channel
 **
 ** \param [in,out] ppstcDt  Pointer of pointer to Dual Timer instance register area
 ** \param [in]     u8Ch     Channel number
 **
 ** \retval Pointer to internal data or NULL if instance is not enabled.
 **         (or channel is invalid)
 **
 ******************************************************************************/
static stc_dt_intern_data_t* DtGetInternDataPtr(volatile stc_dtn_t** ppstcDt,
                                                uint8_t              u8Ch)
{
    stc_dt_intern_data_t* pstcDtInternData = NULL;
    uint32_t              u32Instance;

    /* check for channel */
    if ((NULL != ppstcDt)
    &&  (NULL != *ppstcDt)
    &&  (DtMaxChannels > u8Ch)
       )
    {
        /* Get ptr to internal data struct ... */
        for (u32Instance = 0; u32Instance < (uint32_t)DtInstanceIndexMax; u32Instance++)
        {
            if (*ppstcDt == m_astcDtInstanceDataLut[u32Instance].pstcInstance)
            {
                /* Set actual address of register list of current channel */
                *ppstcDt = &((*ppstcDt)[u8Ch]);
                pstcDtInternData = &m_astcDtInstanceDataLut[u32Instance].stcInternData;
                break;
            }
        }
    }

    return (pstcDtInternData);
} /* DtGetInternDataPtr */

/**
 ******************************************************************************
 ** \brief Device dependent initialization of interrupts according CMSIS with
 **        level defined in l3.h
 **
 ******************************************************************************/
static void Dt_InitIrq(void)
{
#if (PDL_PERIPHERAL_ENABLE_DT0 == PDL_ON)
    NVIC_ClearPendingIRQ(DT1_2_IRQn);
    NVIC_EnableIRQ(DT1_2_IRQn);
    NVIC_SetPriority(DT1_2_IRQn, PDL_IRQ_LEVEL_DT0);
#endif
} /* Dt_InitIrq */

/**
 ******************************************************************************
 ** \brief Device dependent de-initialization of interrupts according CMSIS 
 **
 ******************************************************************************/
static void Dt_DeInitIrq(void)
{
#if (PDL_PERIPHERAL_ENABLE_DT0 == PDL_ON)
    NVIC_ClearPendingIRQ(DT1_2_IRQn);
    NVIC_DisableIRQ(DT1_2_IRQn);
    NVIC_SetPriority(DT1_2_IRQn, PDL_DEFAULT_INTERRUPT_LEVEL);
#endif
} /* Dt_DeInitIrq */

/**
 *****************************************************************************
 ** \brief ISR callback for DT (channel 0 and 1)
 **
 ** This callbacks are called by the global DT ISR whenever an DT triggers an
 ** interrupt (on channel 0 and/or 1). It calls the callback functions that
 ** has been given during Dt initialization (see Dt_Init() and
 ** #stc_dt_channel_config_t) for each channel individually. If the pointer
 ** to a callback function is NULL, no call is performed.
 **
 ** The active interrupt request flags are cleared by the ISR
 **
 ** \param [in]  u8Ch             Channel number
 **
 *****************************************************************************/
void DtIrqHandler(uint8_t u8Ch)
{
    volatile stc_dtn_t*   pstcDt;
    stc_dt_intern_data_t* pstcDtInternData;

    pstcDt = &DT0;
    /* Get actual address of register list of current channel */
    pstcDt = (volatile stc_dtn_t*)(&((pstcDt)[u8Ch]));
    pstcDtInternData = &m_astcDtInstanceDataLut[0].stcInternData;

    if (TRUE == pstcDt->TIMERXRIS_f.TIMERXRIS)  /* Timer 0 interrupt? */
    {
        pstcDt->TIMERXINTCLR = 0;               /* Clear interrupt */

        /* Check for callback function pointer */
        if (NULL != pstcDtInternData->pfnIntCallbackIntern[u8Ch])
        {
            pstcDtInternData->pfnIntCallbackIntern[u8Ch]() ;
        }
    }
} /* DtIrqHandler */

/**
 *****************************************************************************
 ** \brief Initialize DT
 **
 ** This function initializes the specified channel of Dual Timer.
 **
 ** \param [in]  pstcConfig       Dual timer configuration
 ** \param [in]  u8Ch             Channel number
 **
 ** \retval Ok                    Process successfully done.
 ** \retval ErrorInvalidParameter If one of following conditions are met:
 **             - pstcConfig == NULL
 **             - u8Ch >= DtMaxChannels
 **             - pstcDtInternData == NULL (invalid or disabled DT unit
 **                                         (PDL_PERIPHERAL_ENABLE_DT0))
 **             - one or more enumerated values in pstcDt out of enumaration
 **
 *****************************************************************************/
en_result_t Dt_Init(stc_dt_channel_config_t* pstcConfig,
                    uint8_t                  u8Ch
                   )
{
    en_result_t                    enResult;
    /* Pointer to Dual Timer instance register area */
    volatile stc_dtn_t*            pstcDt;
    /* Pointer to internal data */
    stc_dt_intern_data_t*          pstcDtInternData;
    stc_dtim_timerXcontrol_field_t stcTIMERXCTRL = { 0 }; /* Preset to zero */

    pstcDt = &DT0;
    /*-------- Initialize internal data -------*/
    /* Get pointer to internal data structure and check channel... */
    pstcDtInternData = DtGetInternDataPtr(&pstcDt, u8Ch);
    /* ... and check for NULL */
    if ((NULL == pstcDtInternData)
    ||  (NULL == pstcConfig)
       )
    {
        enResult = ErrorInvalidParameter;
    }
    else
    {
        enResult = Ok;
        /*-------- Configure the DT timer -------*/
        pstcDt->TIMERXCONTROL = 0;              /* Disable everything */

        /* Set Timer Mode */
        switch (pstcConfig->u8Mode)
        {
            /* Free run mode */
            case DtFreeRun:
                stcTIMERXCTRL.TIMERMODE = FALSE;
                stcTIMERXCTRL.ONESHOT   = FALSE;
                break;
            /* Periodic mode */
            case DtPeriodic:
                stcTIMERXCTRL.TIMERMODE = TRUE;
                stcTIMERXCTRL.ONESHOT   = FALSE;
                break;
            /* One shot mode */
            case DtOneShot:
                stcTIMERXCTRL.TIMERMODE = FALSE;
                stcTIMERXCTRL.ONESHOT   = TRUE;
                break;
            default:
                enResult = ErrorInvalidParameter;
                break;
        }

        /* Set Prescaler */
        switch (pstcConfig->u8PrescalerDiv)
        {
            /* Clock/1 */
            case DtPrescalerDiv1:
                stcTIMERXCTRL.TIMERPRE = DT_PRE_TIMER_DIV_1;
                break;
            /* Clock/16 */
            case DtPrescalerDiv16:
                stcTIMERXCTRL.TIMERPRE = DT_PRE_TIMER_DIV_16;
                break;
            /* Clock/256 */
            case DtPrescalerDiv256:
                stcTIMERXCTRL.TIMERPRE = DT_PRE_TIMER_DIV_256;
                break;
            default:
                enResult = ErrorInvalidParameter;
                break;
        }

        /* Set Counter Size */
        switch (pstcConfig->u8CounterSize)
        {
            /* 16bit */
            case DtCounterSize16:
                stcTIMERXCTRL.TIMERSIZE = FALSE;
                break;
            /* 32bit */
            case DtCounterSize32:
                stcTIMERXCTRL.TIMERSIZE = TRUE;
                break;
            default:
                enResult = ErrorInvalidParameter;
                break;
        }
    }

    if (Ok == enResult)
    {
        /* Set control register */
        pstcDt->TIMERXCONTROL_f = stcTIMERXCTRL;

        /* Initialize (clear) callback */
        pstcDtInternData->pfnIntCallbackIntern[u8Ch] = NULL;
    }

    return (enResult);
} /* Dt_Init */

/**
 *****************************************************************************
 ** \brief De-Initialize DT
 **
 ** This function de-initializes the specified channel of Dual Timer.
 ** Dt-DeInit() accesses the DT hardware register. They are reset.
 **
 ** \param [in]  u8Ch             Channel number
 **
 ** \retval Ok                    Process successfully done.
 ** \retval ErrorInvalidParameter If one of following conditions are met:
 **             - pstcDt == NULL
 **             - u8Ch >= DtMaxChannels
 **             - pstcDtInternData == NULL (invalid or disabled DT unit
 **                                         (PDL_PERIPHERAL_ENABLE_DT0)) 
 **
 *****************************************************************************/
en_result_t Dt_DeInit(uint8_t u8Ch)
{
    en_result_t           enResult;
    /* Pointer to Dual Timer instance register area */
    volatile stc_dtn_t*   pstcDt;
    /* Pointer to internal data */
    stc_dt_intern_data_t* pstcDtInternData;

    pstcDt = &DT0;
    /*-------- Initialize internal data -------*/
    /* Get pointer to internal data structure and check channel... */
    pstcDtInternData = DtGetInternDataPtr(&pstcDt, u8Ch);
    /* ... and check for NULL */
    if (NULL == pstcDtInternData)
    {
        enResult = ErrorInvalidParameter;
    }
    else
    {
        /* De-Init DT Interrupt */
        Dt_DeInitIrq();

        /* Clear all registers */
        pstcDt->TIMERXCONTROL = 0;

        pstcDt->TIMERXLOAD    = 0;
        pstcDt->TIMERXINTCLR  = 1;
        pstcDt->TIMERXBGLOAD  = 0;

        /* Clear callback */
        pstcDtInternData->pfnIntCallbackIntern[u8Ch] = NULL;
        enResult = Ok;
    }

    return (enResult);
} /* Dt_DeInit */

/**
 *****************************************************************************
 ** \brief Enable Timer Counter
 **
 ** This function enables the timer counter.
 **
 ** \param [in]  u8Ch             Channel number
 **
 ** \retval Ok                    Process successfully done.
 ** \retval ErrorInvalidParameter If one of following conditions are met:
 **             - u8Ch >= DtMaxChannels
 **             - pstcDtInternData == NULL (invalid or disabled DT unit
 **                                         (PDL_PERIPHERAL_ENABLE_DT0)) 
 **
 *****************************************************************************/
en_result_t Dt_EnableCount(uint8_t u8Ch)
{
    en_result_t           enResult;
    /* Pointer to Dual Timer instance register area */
    volatile stc_dtn_t*   pstcDt;
    /* Pointer to internal data */
    stc_dt_intern_data_t* pstcDtInternData;

    enResult = ErrorInvalidParameter;

    pstcDt = &DT0;
    /* Get pointer to internal data structure and check channel... */
    pstcDtInternData = DtGetInternDataPtr(&pstcDt, u8Ch);
    /* ... and check for NULL */
    if (NULL != pstcDtInternData)
    {
        /* Enable timer counter */
        pstcDt->TIMERXCONTROL_f.TIMEREN = TRUE;
        enResult = Ok;
    }

    return (enResult);
} /* Dt_EnableCount */

/**
 *****************************************************************************
 ** \brief Disable Timer Counter
 **
 ** This function disables the timer counter.
 **
 ** \param [in]  u8Ch             Channel number
 **
 ** \retval Ok                    Process successfully done.
 ** \retval ErrorInvalidParameter If one of following conditions are met:
 **             - u8Ch >= DtMaxChannels
 **             - pstcDtInternData == NULL (invalid or disabled DT unit
 **                                         (PDL_PERIPHERAL_ENABLE_DT0)) 
 **
 *****************************************************************************/
en_result_t Dt_DisableCount(uint8_t u8Ch)
{
    en_result_t           enResult;
    /* Pointer to Dual Timer instance register area */
    volatile stc_dtn_t*   pstcDt;
    /* Pointer to internal data */
    stc_dt_intern_data_t* pstcDtInternData;

    enResult = ErrorInvalidParameter;

    pstcDt = &DT0;
    /* Get pointer to internal data structure and check channel... */
    pstcDtInternData = DtGetInternDataPtr(&pstcDt, u8Ch);
    /* ... and check for NULL */
    if (NULL != pstcDtInternData)
    {
        /* Disable timer counter */
        pstcDt->TIMERXCONTROL_f.TIMEREN = FALSE;
        enResult = Ok;
    }

    return (enResult);
} /* Dt_DisableCount */

/**
 *****************************************************************************
 ** \brief Enable Interrupt
 **
 ** This function enables the interruption.
 **
 ** \param [in]  pfnIntCallback   Callback function when interruption is occured.
 **                               (Can be set NULL)
 ** \param [in]  u8Ch             Channel number
 **
 ** \retval Ok                    Process successfully done.
 ** \retval ErrorInvalidParameter If one of following conditions are met:
 **             - u8Ch >= DtMaxChannels
 **             - pstcDtInternData == NULL (invalid or disabled DT unit
 **                                         (PDL_PERIPHERAL_ENABLE_DT0)) 
 **
 *****************************************************************************/
en_result_t Dt_EnableInt(dt_cb_func_ptr_t pfnIntCallback,
                         uint8_t          u8Ch
                        )
{
    en_result_t           enResult;
    /* Pointer to Dual Timer instance register area */
    volatile stc_dtn_t*   pstcDt;
    /* Pointer to internal data */
    stc_dt_intern_data_t* pstcDtInternData;

    enResult = ErrorInvalidParameter;

    pstcDt = &DT0;
    /* Get pointer to internal data structure and check channel... */
    pstcDtInternData = DtGetInternDataPtr(&pstcDt, u8Ch);
    /* ... and check for NULL */
    if (NULL != pstcDtInternData)
    {
        /* Set callback function */
        pstcDtInternData->pfnIntCallbackIntern[u8Ch] = pfnIntCallback;
        /* Enable interrupt */
        pstcDt->TIMERXCONTROL_f.INTENABLE = TRUE;
        /* Enable IRQ */
        Dt_InitIrq();
        enResult = Ok;
    }

    return (enResult);
} /* Dt_EnableInt */

/**
 *****************************************************************************
 ** \brief Disable Interrupt
 **
 ** This function disables the interruption.
 **
 ** \param [in]  u8Ch             Channel number
 **
 ** \retval Ok                    Process successfully done.
 ** \retval ErrorInvalidParameter If one of following conditions are met:
 **             - u8Ch >= DtMaxChannels
 **             - pstcDtInternData == NULL (invalid or disabled DT unit
 **                                         (PDL_PERIPHERAL_ENABLE_DT0)) 
 **
 *****************************************************************************/
en_result_t Dt_DisableInt(uint8_t u8Ch)
{
    en_result_t           enResult;
    /* Pointer to Dual Timer instance register area */
    volatile stc_dtn_t*   pstcDt;
    /* Pointer to internal data */
    stc_dt_intern_data_t* pstcDtInternData;

    enResult = ErrorInvalidParameter;

    pstcDt = &DT0;
    /* Get pointer to internal data structure and check channel... */
    pstcDtInternData = DtGetInternDataPtr(&pstcDt, u8Ch);
    /* ... and check for NULL */
    if (NULL != pstcDtInternData)
    {
        /* Disable IRQ */
        Dt_DeInitIrq();
        /* Disable interrupt */
        pstcDt->TIMERXCONTROL_f.INTENABLE = FALSE;
        /* Clear callback function */
        pstcDtInternData->pfnIntCallbackIntern[u8Ch] = NULL;
        enResult = Ok;
    }

    return (enResult);
} /* Dt_DisableInt */

/**
 ******************************************************************************
 ** \brief Get interrupt status
 ** The Function can return the interrupt status (TimerXRIS)
 **
 ** \param [in]     u8Ch          Channel number
 **
 ** \retval boolean_t:the interrupt status
 **
 ******************************************************************************/
boolean_t Dt_GetIntFlag(uint8_t u8Ch)
{
    /* Pointer to Dual Timer instance register area */
    volatile stc_dtn_t*   pstcDt;
    boolean_t             bRetVal = FALSE;
    /* Pointer to internal data */
    stc_dt_intern_data_t* pstcDtInternData;

    pstcDt = &DT0;
    /* Get pointer to internal data structure and check channel... */
    pstcDtInternData = DtGetInternDataPtr(&pstcDt, u8Ch);
    /* ... and check for NULL */
    if (NULL != pstcDtInternData)
    {
        /* Check the interrupt status */
        if (TRUE == pstcDt->TIMERXRIS_f.TIMERXRIS)
        {
            bRetVal = TRUE;
        }
    }

    return (bRetVal);
} /* Dt_GetIntFlag */

/**
 ******************************************************************************
 ** \brief Get mask interrupt status
 ** The Function can return the mask interrupt status (TimerXMIS)
 **
 ** \param [in]     u8Ch          Channel number
 **
 ** \retval boolean_t:the mask interrupt status
 **
 ******************************************************************************/
boolean_t Dt_GetMaskIntFlag(uint8_t u8Ch)
{
    /* Pointer to Dual Timer instance register area */
    volatile stc_dtn_t*   pstcDt;
    boolean_t             bRetVal = FALSE;
    /* Pointer to internal data */
    stc_dt_intern_data_t* pstcDtInternData;

    pstcDt = &DT0;
    /* Get pointer to internal data structure and check channel... */
    pstcDtInternData = DtGetInternDataPtr(&pstcDt, u8Ch);
    /* ... and check for NULL */
    if (NULL != pstcDtInternData)
    {
        /* Check the mask interrupt status */
        if (TRUE == pstcDt->TIMERXMIS_f.TIMERXMIS)
        {
            bRetVal = TRUE;
        }
    }

    return (bRetVal);
} /* Dt_GetMaskIntFlag */

/**
 ******************************************************************************
 ** \brief Clear interrupt status
 ** The Function clears the interrupt status
 **
 ** \param [in]  u8Ch           Channel number
 **
 ** \retval Ok                        Process successfully done.
 ** \retval ErrorInvalidParameter     If one of following conditions are met:
 **             - u8Ch >= DtMaxChannels
 **             - pstcMfsInternData == NULL (invalid or disabled MFS unit
 **                                          (PDL_PERIPHERAL_ENABLE_MFS)) 
 **
 ******************************************************************************/
en_result_t Dt_ClrIntFlag(uint8_t u8Ch)
{
    en_result_t           enResult;
    /* Pointer to Dual Timer instance register area */
    volatile stc_dtn_t*   pstcDt;
    /* Pointer to internal data */
    stc_dt_intern_data_t* pstcDtInternData;

    enResult = ErrorInvalidParameter;

    pstcDt = &DT0;
    /* Get pointer to internal data structure and check channel... */
    pstcDtInternData = DtGetInternDataPtr(&pstcDt, u8Ch);
    /* ... and check for NULL */
    if (NULL != pstcDtInternData)
    {
        /* Clear the interrupt status */
        pstcDt->TIMERXINTCLR = 1;
        enResult = Ok;
    }

    return (enResult);
} /* Dt_ClrIntFlag */

/**
 ******************************************************************************
 ** \brief Write load value
 ** The Function writes the load value to load register
 **
 ** \param [in]  u32LoadVal     Load value to set to load register
 ** \param [in]  u8Ch           Channel number
 **
 ** \retval Ok                        Process successfully done.
 ** \retval ErrorInvalidParameter     If one of following conditions are met:
 **             - u8Ch >= DtMaxChannels
 **             - pstcMfsInternData == NULL (invalid or disabled MFS unit
 **                                          (PDL_PERIPHERAL_ENABLE_MFS)) 
 **
 ******************************************************************************/
en_result_t Dt_WriteLoadVal(uint32_t u32LoadVal,
                            uint8_t  u8Ch
                           )
{
    en_result_t           enResult;
    /* Pointer to Dual Timer instance register area */
    volatile stc_dtn_t*   pstcDt;
    /* Pointer to internal data */
    stc_dt_intern_data_t* pstcDtInternData;

    enResult = ErrorInvalidParameter;

    pstcDt = &DT0;
    /* Get pointer to internal data structure and check channel... */
    pstcDtInternData = DtGetInternDataPtr(&pstcDt, u8Ch);
    /* ... and check for NULL */
    if (NULL != pstcDtInternData)
    {
        /* 16bit mode */
        if (FALSE == pstcDt->TIMERXCONTROL_f.TIMERSIZE)
        {
            u32LoadVal &= 0x0000FFFF;
        }
        /* Write load value to register */
        pstcDt->TIMERXLOAD = u32LoadVal;
        enResult = Ok;
    }

    return (enResult);
} /* Dt_WriteLoadVal */

/**
 ******************************************************************************
 ** \brief Write back-ground load value
 ** The Function writes the load value to back-ground load register
 **
 ** \param [in]  u32BgLoadVal   Load value to set to back-ground load register
 ** \param [in]  u8Ch           Channel number
 **
 ** \retval Ok                        Process successfully done.
 ** \retval ErrorInvalidParameter     If one of following conditions are met:
 **             - u8Ch >= DtMaxChannels
 **             - pstcMfsInternData == NULL (invalid or disabled MFS unit
 **                                          (PDL_PERIPHERAL_ENABLE_MFS)) 
 **
 ******************************************************************************/
en_result_t Dt_WriteBgLoadVal(uint32_t u32BgLoadVal,
                              uint8_t  u8Ch
                             )
{
    en_result_t           enResult;
    /* Pointer to Dual Timer instance register area */
    volatile stc_dtn_t*   pstcDt;
    /* Pointer to internal data */
    stc_dt_intern_data_t* pstcDtInternData;

    enResult = ErrorInvalidParameter;

    pstcDt = &DT0;
    /* Get pointer to internal data structure and check channel... */
    pstcDtInternData = DtGetInternDataPtr(&pstcDt, u8Ch);
    /* ... and check for NULL */
    if (NULL != pstcDtInternData)
    {
        /* 16bit mode */
        if (FALSE == pstcDt->TIMERXCONTROL_f.TIMERSIZE)
        {
            u32BgLoadVal &= 0x0000FFFF;
        }
        /* Write back-ground load value to register */
        pstcDt->TIMERXBGLOAD = u32BgLoadVal;
        enResult = Ok;
    }

    return (enResult);
} /* Dt_WriteBgLoadVal */

/**
 ******************************************************************************
 ** \brief Read current count value
 ** The Function reads the value from value register
 **
 ** \param [in]  u8Ch           Channel number
 **
 ** \retval uint32_t:current counter value
 **
 ******************************************************************************/
uint32_t Dt_ReadCurCntVal(uint8_t u8Ch)
{
    /* Pointer to Dual Timer instance register area */
    volatile stc_dtn_t*   pstcDt;
    uint32_t              u32DtValue = 0;
    /* Pointer to internal data */
    stc_dt_intern_data_t* pstcDtInternData;

    pstcDt = &DT0;
    /* Get pointer to internal data structure and check channel... */
    pstcDtInternData = DtGetInternDataPtr(&pstcDt, u8Ch);
    /* ... and check for NULL */
    if (NULL != pstcDtInternData)
    {
        /* Read current count value */
        u32DtValue = pstcDt->TIMERXVALUE;
        /* 16bit mode */
        if (FALSE == pstcDt->TIMERXCONTROL_f.TIMERSIZE)
        {
            u32DtValue &= 0x0000FFFF;
        }
    }

    return (u32DtValue);
} /* Dt_ReadCurCntVal */

//@} // DtGroup

#endif /* #if (defined(PDL_PERIPHERAL_DT_ACTIVE)) */

/******************************************************************************/
/* EOF (not truncated)                                                        */
/******************************************************************************/

