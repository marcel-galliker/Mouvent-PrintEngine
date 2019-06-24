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
/** \file rtc.c
 **
 ** A detailed description is available at
 ** @link RtcGroup RTC Module description @endlink
 **
 ** History:
 **   - 2013-04-04  1.0  NT   First version.
 **   - 2013-11-22  1.1  EZ   Add the "bNoInit" parameter in the Rtc_Init()
 **                           function.
 **                           Add Rtc_GetRunStatus() function.
 **   - 2014-01-15  1.2  EZ   The RTC interrupt IRQ disable/enable replaces
 **                           gloable RTC disable/enable
 **
 **   - 2014-06-13  1.3  EZ   Get the ST bit from before using it in Rtc_Init()
 ******************************************************************************/

/******************************************************************************/
/* Include files                                                              */
/******************************************************************************/
#include "pdl.h"

#if (defined(PDL_PERIPHERAL_RTC_ACTIVE))

/**
 ******************************************************************************
 ** \addtogroup RtcGroup
 ******************************************************************************/
//@{

/******************************************************************************/
/* Local pre-processor symbols/macros ('#define')                             */
/******************************************************************************/
/* RTC time value */
#define RTC_YEAR_1900           (1900)
#define RTC_YEAR_2000           (2000)
#define RTC_SEC_MAX             (59u)
#define RTC_MIN_MAX             (59u)
#define RTC_HOUR_MAX            (23u)
#define RTC_DAY_MAX             (31u)
#define RTC_MONTH_MAX           (12u)
#define RTC_DAYOFWEEK_MAX       (6u)
#define RTC_YEAR_MAX            (99u)

/* Caluculate value */
#define RTC_4BIT                (4u)
#define RTC_8BIT                (8u)
#define RTC_16BIT               (16u)
#define RTC_DIGIT_10            (10u)
#define RTC_2BYTES              (2u)
#define RTC_4BYTES              (4u)

/* for WTCR20 */
#define RTC_WTCR20_PWRITE       (0x20u)
#define RTC_WTCR20_PREAD        (0x10u)
#define RTC_WTCR20_BWRITE       (0x08u)
#define RTC_WTCR20_BREAD        (0x04u)
#define RTC_WTCR20_CWRITE       (0x02u)
#define RTC_WTCR20_CREAD        (0x01u)

/* Mask value */
#define RTC_MASK_LOW4BIT        (0x0Fu)
#define RTC_MASK_HIGH4BIT       (0xF0u)

/* Error code for mktime */
#define RTC_ERR                 (-1)

/******************************************************************************/
/* Global variable definitions (declared in header file with 'extern')        */
/******************************************************************************/

/******************************************************************************/
/* Local type definitions ('typedef')                                         */
/******************************************************************************/

/******************************************************************************/
/* Local function prototypes ('static')                                       */
/******************************************************************************/
static void RtcToTm( stc_rtc_time_t* pstcRtcTime,
                            struct tm*      pstcTime);
static uint8_t RtcBinToBcd(uint8_t u8BinValue);
static uint8_t RtcBcdToBin(uint8_t u8BcdValue);
static void RtcInitIrq(void);
static void RtcDeInitIrq(void);
static en_result_t RtcWaitTransComplete(void);
static en_result_t RtcTransToVbat(uint8_t u8Control);

/******************************************************************************/
/* Local variable definitions ('static')                                      */
/******************************************************************************/
static stc_rtc_intern_data_t stcRtcInternData;
static uint8_t u8CRead;
static uint32_t u32NvicData;

/******************************************************************************/
/* Function implementation - global ('extern') and local ('static')           */
/******************************************************************************/

/**
 ******************************************************************************
 ** \brief Disable RTC NVIC and save original value
 ******************************************************************************/
void RtcDisableNvic(void)
{
    u32NvicData = NVIC->ISER[(uint32_t)((int32_t)RTC_IRQn) >> 5];
    NVIC->ICER[((uint32_t)(RTC_IRQn) >> 5)] = (1 << ((uint32_t)(RTC_IRQn) & 0x1F));
}

/**
 ******************************************************************************
 ** \brief Restore RTC NVIC
 ******************************************************************************/
void RtcRestoreNvic(void)
{
    NVIC->ISER[(uint32_t)((int32_t)RTC_IRQn) >> 5] = u32NvicData;
}


/**
 ******************************************************************************
 ** \brief Convert RTC time structure to time.h tm structure
 **
 ** \param [in]  pstcRtcTime    RTC Time structure
 ** \param [out] pstcTime       tm Time structure
 **
 ******************************************************************************/
static void RtcToTm( stc_rtc_time_t* pstcRtcTime,
                            struct tm*      pstcTime)
{
    pstcTime->tm_year  = (int)((RTC_YEAR_2000 + pstcRtcTime->u8Year) - RTC_YEAR_1900);
    pstcTime->tm_mon   = (int)(pstcRtcTime->u8Month - 1);
    pstcTime->tm_mday  = (int)(pstcRtcTime->u8Day);
    pstcTime->tm_sec   = (int)(pstcRtcTime->u8Second);
    pstcTime->tm_min   = (int)(pstcRtcTime->u8Minute);
    pstcTime->tm_hour  = (int)(pstcRtcTime->u8Hour);
    pstcTime->tm_isdst = 0;
} /* RtcToTm */

/**
 ******************************************************************************
 ** \brief Binary to BCD conversion
 **
 ** \param [in]   u8BinValue  Binary Value
 **
 ** \return  8-Bit BCD Value
 **
 ******************************************************************************/
static uint8_t RtcBinToBcd(uint8_t u8BinValue)
{
    uint8_t u8BcdValue;

    /* Ten's digit */
    u8BcdValue = (uint8_t)((u8BinValue / RTC_DIGIT_10) << RTC_4BIT);

    /* One's digit */
    u8BcdValue |= u8BinValue % RTC_DIGIT_10;

    return (u8BcdValue);
} /* RtcBinToBcd */

/**
 ******************************************************************************
 ** \brief BCD to Binary conversion
 **
 ** \param [in]   u8BcdValue  BCD Value
 **
 ** \return  8-Bit Bin Value
 **
 ******************************************************************************/
static uint8_t RtcBcdToBin(uint8_t u8BcdValue)
{
    uint8_t u8BinValue;

    /* Ten's digit */
    u8BinValue = ((RTC_MASK_HIGH4BIT & u8BcdValue) >> RTC_4BIT) * RTC_DIGIT_10;

    /* One's digit */
    u8BinValue += u8BcdValue & RTC_MASK_LOW4BIT;

    return (u8BinValue);
} /* RtcBcdToBin */

/**
 ******************************************************************************
 ** \brief Device dependent initialization of interrupts according CMSIS with
 **        level defined in pdl.h
 **
 ******************************************************************************/
static void RtcInitIrq(void)
{
#if (PDL_PERIPHERAL_ENABLE_RTC0 == PDL_ON)
    NVIC_ClearPendingIRQ(RTC_IRQn);
    NVIC_EnableIRQ(RTC_IRQn);
    NVIC_SetPriority(RTC_IRQn, PDL_IRQ_LEVEL_RTC0);
#endif
} /* RtcInitIrq */

/**
 ******************************************************************************
 ** \brief Device dependent de-initialization of interrupts according CMSIS with
 **        level defined in pdl.h
 **
 ******************************************************************************/
static void RtcDeInitIrq(void)
{
#if (PDL_PERIPHERAL_ENABLE_RTC0 == PDL_ON)
    NVIC_ClearPendingIRQ(RTC_IRQn);
    NVIC_DisableIRQ(RTC_IRQn);
    NVIC_SetPriority(RTC_IRQn, PDL_DEFAULT_INTERRUPT_LEVEL);
#endif
} /* RtcDeInitIrq */

/**
 ******************************************************************************
 ** \brief Wait to complete writing transmission
 **
 ** \retval  Ok             Success to complete transmission
 ** \retval  ErrorTimeout   Timeout to complete transmission
 **
 ******************************************************************************/
static en_result_t RtcWaitTransComplete(void)
{
    en_result_t enResult = Ok;
    uint32_t    u32WaitCompleteTimeout;

    u32WaitCompleteTimeout = RTC_TRANS_POLLING_TIMEOUT;
    while ((TRUE == FM4_RTC->WTCR10_f.TRANS)    /* Wait until transmission is completed */
    &&     (0 != u32WaitCompleteTimeout)        /* If transmission is not completed even if it passes for a long time... */
          )
    {
        PDL_WAIT_LOOP_HOOK();
        u32WaitCompleteTimeout--;
    }

    if (0 == u32WaitCompleteTimeout)
    {
        enResult = ErrorTimeout;
    }

    return (enResult);
} /* RtcWaitTransComplete */

/**
 ******************************************************************************
 ** \brief Transfer to/from VBAT domain and wait to complete transmission
 **
 ** \param [in] u8Control   Control to transmission
 **
 ** \retval  Ok             Success to complete transmission
 ** \retval  ErrorTimeout   Timeout to complete transmission
 **
 ******************************************************************************/
static en_result_t RtcTransToVbat(uint8_t u8Control)
{
    en_result_t enResult;

    /* Transmit to or from VBAT domain */
    FM4_RTC->WTCR20 = u8Control;
    /* Wait to complete transmission */
    enResult = RtcWaitTransComplete();

    return (enResult);
} /* RtcTransToVbat */

/**
 ******************************************************************************
 ** \brief RTC  interrupt service routine.
 **
 ******************************************************************************/
void RtcIrqHandler(void)
{
    stc_rtc_wtcr12_field_t stcWtcr12;

    /* Local variables to prevent multiple volatile I/O read-out */
    /*   warining in function call 'pfnReadCallback(...)' */
    uint8_t u8Second;
    uint8_t u8Minute;
    uint8_t u8Hour;
    uint8_t u8Day;
    uint8_t u8DayOfWeek;
    uint8_t u8Month;
    uint8_t u8Year;

    stcWtcr12 = FM4_RTC->WTCR12_f;

    /* Read Completion? */
    if (TRUE == stcWtcr12.INTCRI)
    {
        /* Clear INTCRI Flag */
        stcWtcr12.INTCRI = FALSE;

        if (NULL != stcRtcInternData.pfnReadCallback)
        {
            if (TRUE == u8CRead)
            {
                u8CRead = FALSE;
                /* Convert to Binary */
                u8Second    = RtcBcdToBin(FM4_RTC->WTSR);
                u8Minute    = RtcBcdToBin(FM4_RTC->WTMIR);
                u8Hour      = RtcBcdToBin(FM4_RTC->WTHR);
                u8Day       = RtcBcdToBin(FM4_RTC->WTDR);
                u8DayOfWeek = RtcBcdToBin(FM4_RTC->WTDW);
                u8Month     = RtcBcdToBin(FM4_RTC->WTMOR);
                u8Year      = RtcBcdToBin(FM4_RTC->WTYR);
                /* Callback */
                stcRtcInternData.pfnReadCallback(u8Second,
                                                 u8Minute,
                                                 u8Hour,
                                                 u8Day,
                                                 u8DayOfWeek,
                                                 u8Month,
                                                 u8Year);
            }
        }
    }

    /* Timer writing error Interrupt? */
    if (TRUE == stcWtcr12.INTERI)
    {
        /* Clear INTERI Flag */
        stcWtcr12.INTERI = FALSE;

        if (NULL != stcRtcInternData.pfnTimeWrtErrCallback)
        {
            /* Callback */
            stcRtcInternData.pfnTimeWrtErrCallback();
        }
    }

    /* Alarm Completion? */
    if (TRUE == stcWtcr12.INTALI)
    {
        /* Clear INTALI Flag */
        stcWtcr12.INTALI = FALSE;

        if (NULL != stcRtcInternData.pfnAlarmCallback)
        {
            /* Callback */
            stcRtcInternData.pfnAlarmCallback();
        }
    }

    /* Timer Interrupt? */
    if (TRUE == stcWtcr12.INTTMI)
    {
        /* Clear INTTMI Flag */
        stcWtcr12.INTTMI = FALSE;

        if (NULL != stcRtcInternData.pfnTimerCallback)
        {
            /* Callback */
            stcRtcInternData.pfnTimerCallback();
        }
    }

    /* 0.5-Second Interrupt? */
    if (TRUE == stcWtcr12.INTSSI)
    {
        /* Clear INTSSI Flag */
        stcWtcr12.INTSSI = FALSE;

        if (NULL != stcRtcInternData.pfnHalfSecondCallback)
        {
            /* Callback */
            stcRtcInternData.pfnHalfSecondCallback();
        }
    }

    /* One Second Interrupt? */
    if (TRUE == stcWtcr12.INTSI)
    {
        /* Clear INTSI Flag */
        stcWtcr12.INTSI = FALSE;

        if (NULL != stcRtcInternData.pfnOneSecondCallback)
        {
            /* Callback */
            stcRtcInternData.pfnOneSecondCallback();
        }
    }

    /* One Minute Interrupt? */
    if (TRUE == stcWtcr12.INTMI)
    {
        /* Clear INTMI Flag */
        stcWtcr12.INTMI = FALSE;

        if (NULL != stcRtcInternData.pfnOneMinuteCallback)
        {
            /* Callback */
            stcRtcInternData.pfnOneMinuteCallback();
        }
    }

    /* One Hour Interrupt? */
    if (TRUE == stcWtcr12.INTHI)
    {
        /* Clear INTHI Flag */
        stcWtcr12.INTHI = FALSE;

        if (NULL != stcRtcInternData.pfnOneHourCallback)
        {
            /* Callback */
            stcRtcInternData.pfnOneHourCallback();
        }
    }

    FM4_RTC->WTCR12_f = stcWtcr12;
} /* RtcIrqHandler */

/**
 ******************************************************************************
 ** \brief Initialize RTC
 **
 ** This function initializes the RTC module and sets up the internal
 ** data structures.
 ** If the sub clock (VBAT domain) was not enabled before, the sub clock enable
 ** and stablilization is done in this function.
 **
 ** \param [in] pstcConfig      Pointer to RTC configuration structure
 ** \param [in] pstcTime        Pointer to RTC Date and Time structure
 ** \param [in] pstcAlarm       Pointer to RTC Alarm (Data and Time) structure
 ** \param [in] bTouchNvic      TRUE = touch shared NVIC registers
 **                             FALSE = do not touch
 ** \param [in] bResetNoReInit  TRUE = don't initialize RTC if RTC is running
 **                             FALSE = Initialize RTC regardness of RTC status
 **
 ** \retval Ok                    Internal data has been setup
 ** \retval ErrorInvalidParameter If one of following conditions are met:
 **                               - pstcConfig == NULL
 **                               - pstcTime == NULL
 **                               - Invalid value of a pstcConfig's element
 **                               - Invalid value of a pstcTime's element
 **                               - Invalid value of a pstcAlarm's element
 **                               pstcAlarm can set NULL
 **
 ******************************************************************************/
en_result_t Rtc_Init(stc_rtc_config_t*       pstcConfig,
                     stc_rtc_time_t*         pstcTime,
                     stc_rtc_alarm_t*        pstcAlarm,
                     boolean_t               bTouchNvic,
                     boolean_t               bResetNoReInit
                     )
{
    en_result_t enResult;

    /*
     * If bDstbResetNoReInit is TRUE, don't do initialization when power on or
     * deep standby reset occurs and RTC is aready running. However the interrupt
     * callback and interrupt enable still need updating.
     */

    RtcTransToVbat(RTC_WTCR20_CREAD);

    if((TRUE == bResetNoReInit) && (TRUE == FM4_RTC->WTCR10_f.ST))
    {
        /* Check for Interrupt enables */
        if (0 != ((pstcConfig->u8AllInterrupts) & RTC_INTERRUPT_ENABLE_MASK))
        {
            /* No errors expected here (pointers checked previously) */
            (void)Rtc_EnableDisableInterrupts(pstcConfig);

            if (TRUE == bTouchNvic)
            {
                RtcInitIrq();
            }
        }
        /* Set-up callbacks */
        stcRtcInternData.pfnReadCallback       = pstcConfig->pfnReadCallback;
        stcRtcInternData.pfnTimeWrtErrCallback = pstcConfig->pfnTimeWrtErrCallback;
        stcRtcInternData.pfnAlarmCallback      = pstcConfig->pfnAlarmCallback;
        stcRtcInternData.pfnTimerCallback      = pstcConfig->pfnTimerCallback;
        stcRtcInternData.pfnHalfSecondCallback = pstcConfig->pfnHalfSecondCallback;
        stcRtcInternData.pfnOneSecondCallback  = pstcConfig->pfnOneSecondCallback;
        stcRtcInternData.pfnOneMinuteCallback  = pstcConfig->pfnOneMinuteCallback;
        stcRtcInternData.pfnOneHourCallback    = pstcConfig->pfnOneHourCallback;

        enResult = Ok;
    }
     /* Check for valid pointer */
    else if (NULL == pstcConfig)
    {
        enResult = ErrorInvalidParameter;
    }

    /* Wait to complete transmission */
    else if (Ok != RtcWaitTransComplete())
    {
        enResult = ErrorTimeout;
    }

    else
    {
        /* For initialization: Force all bits of WTCR10-13 (inclusive ST bit) */
        /*   to '0' (w/o previous check) */
        FM4_RTC->WTCR10 = 0;
        FM4_RTC->WTCR11 = 0;
        FM4_RTC->WTCR12 = 0;
        FM4_RTC->WTCR13 = 0;
        FM4_RTC->WTCR20 = 0;
        FM4_RTC->WTCR21 = 0;

        /* Check and Set Date and Time */
        enResult = Rtc_SetDateTime(pstcTime);
        if (Ok == enResult)
        {
            /* If Alarm is specified */
            if (NULL != pstcAlarm)
            {
                /* Check and Set Alarm Date and Time */
                enResult = Rtc_SetAlarmDateTime(pstcAlarm);
            }
        }

        if (Ok == enResult)
        {
            /* Check value for frequency correction value */
            if (RTC_MAX_FREQ_CORR_VALUE < (pstcConfig->u16FreqCorrValue))
            {
                enResult = ErrorInvalidParameter;
            }
        }

        if (Ok == enResult)
        {
            /* Set value for frequency correction */
            FM4_RTC->WTCAL0 = pstcConfig->u8FreqCorrValue0;
            FM4_RTC->WTCAL1 = pstcConfig->u8FreqCorrValue1;

            /* Set Frequency correction enable */
            if (TRUE == pstcConfig->bUseFreqCorr)
            {
                FM4_RTC->WTCALEN = 1;
            }
            else
            {
                FM4_RTC->WTCALEN = 0;
            }

            /* Set Divider ratio */
            switch (pstcConfig->u8DividerRatio)
            {
                case RtcDivRatio1:
                case RtcDivRatio2:
                case RtcDivRatio4:
                case RtcDivRatio8:
                case RtcDivRatio16:
                case RtcDivRatio32:
                case RtcDivRatio64:
                case RtcDivRatio128:
                case RtcDivRatio256:
                case RtcDivRatio512:
                case RtcDivRatio1024:
                case RtcDivRatio2048:
                case RtcDivRatio4096:
                case RtcDivRatio8192:
                case RtcDivRatio16384:
                case RtcDivRatio32768:
                    FM4_RTC->WTDIV = pstcConfig->u8DividerRatio;
                    break;
                default:
                    enResult = ErrorInvalidParameter;
                    break;
            }
        }

        if (Ok == enResult)
        {
            /* Set Dividor output enable */
            if (TRUE == pstcConfig->bUseDivider)
            {
                FM4_RTC->WTDIVEN_f.WTDIVEN = TRUE;
            }
            else
            {
                FM4_RTC->WTDIVEN_f.WTDIVEN = FALSE;
            }

            /* Set Frequency correction cycle setting */
            if (RTC_MAX_FREQ_CORR_CYCLE_SET_VALUE < pstcConfig->u8FreqCorrCycle)
            {
                enResult = ErrorInvalidParameter;
            }
        }

        if (Ok == enResult)
        {
            FM4_RTC->WTCALPRD = pstcConfig->u8FreqCorrCycle;

            /* Set RTCCO output selection */
            switch (pstcConfig->u8CoSignalDiv)
            {
                case RtcCoDiv1:
                    FM4_RTC->WTCOSEL_f.WTCOSEL = FALSE;
                    break;
                case RtcCoDiv2:
                    FM4_RTC->WTCOSEL_f.WTCOSEL = TRUE;
                    break;
                default:
                    enResult = ErrorInvalidParameter;
                    break;
            }
        }

        if (Ok == enResult)
        {
            /* Transmit to VBAT domain and wait to complete transmission */
            enResult = RtcTransToVbat(RTC_WTCR20_PWRITE);
        }

        if (Ok == enResult)
        {
            /* Setup Alarm Registers */
            enResult = Rtc_EnableDisableAlarmRegisters(&pstcConfig->stcAlarmRegisterEnable);
        }

        if (Ok == enResult)
        {
            /* When interruption enables, callback may call before initialization. */
            stcRtcInternData.pfnReadCallback       = NULL;
            stcRtcInternData.pfnTimeWrtErrCallback = NULL;
            stcRtcInternData.pfnAlarmCallback      = NULL;
            stcRtcInternData.pfnTimerCallback      = NULL;
            stcRtcInternData.pfnHalfSecondCallback = NULL;
            stcRtcInternData.pfnOneSecondCallback  = NULL;
            stcRtcInternData.pfnOneMinuteCallback  = NULL;
            stcRtcInternData.pfnOneHourCallback    = NULL;
            u8CRead = FALSE;

            /* Check for Interrupt enables */
            if (0 != ((pstcConfig->u8AllInterrupts) & RTC_INTERRUPT_ENABLE_MASK))
            {
                /* No errors expected here (pointers checked previously) */
                (void)Rtc_EnableDisableInterrupts(pstcConfig);

                if (TRUE == bTouchNvic)
                {
                    RtcInitIrq();
                }
            }

            /* Set-up callbacks */
            stcRtcInternData.pfnReadCallback       = pstcConfig->pfnReadCallback;
            stcRtcInternData.pfnTimeWrtErrCallback = pstcConfig->pfnTimeWrtErrCallback;
            stcRtcInternData.pfnAlarmCallback      = pstcConfig->pfnAlarmCallback;
            stcRtcInternData.pfnTimerCallback      = pstcConfig->pfnTimerCallback;
            stcRtcInternData.pfnHalfSecondCallback = pstcConfig->pfnHalfSecondCallback;
            stcRtcInternData.pfnOneSecondCallback  = pstcConfig->pfnOneSecondCallback;
            stcRtcInternData.pfnOneMinuteCallback  = pstcConfig->pfnOneMinuteCallback;
            stcRtcInternData.pfnOneHourCallback    = pstcConfig->pfnOneHourCallback;

            /* Start RTC */
            FM4_RTC->WTCR10_f.ST = TRUE;
            /* Transmit to VBAT domain and wait to complete transmission */
            enResult = RtcTransToVbat(RTC_WTCR20_CWRITE);
        }
    }

    return (enResult);
} /* Rtc_Init */

/**
 ******************************************************************************
 ** \brief Initialize RTC
 **
 ** This function reads the RTC run status
 **
 ** \retval TRUE   RTC is running
 ** \retval FALSE  RTC stops
 **
 ******************************************************************************/
boolean_t Rtc_GetRunStatus(void)
{
    return (boolean_t)FM4_RTC->WTCR10_f.ST;
}

/**
 ******************************************************************************
 ** \brief Check and Set RTC Date and RTC Time
 **
 ** This function builds two 32-bit words for minimum write access to the
 ** RTC's date and time registers.
 **
 ** \note During update interrupts are globally disabled!
 **
 ** \param [in]  pstcRtcTime    Pointer to RTC Date and Time structure
 **
 ** \retval  Ok                       Successful initialization
 ** \retval  ErrorInvalidParameter    If one of following conditions are met:
 **                                   - pstcRtcTime == NULL
 **                                   - Invalid value of a pstcRtcTime's element
 ** \retval  ErrorTimeout             Polling loop timed out
 **
 ******************************************************************************/
en_result_t Rtc_SetDateTime(stc_rtc_time_t* pstcRtcTime)
{
    en_result_t            enResult;
    uint32_t               u32BusyTimeout;
    stc_rtc_wtcr12_field_t stcWtcr12;
    stc_rtc_wtcr13_field_t stcWtcr13;

    /* Check for valid pointer */
    if ((NULL == pstcRtcTime)
    /* Check Second */
    ||  (RTC_SEC_MAX < pstcRtcTime->u8Second)
    /* Check Minute */
    ||  (RTC_MIN_MAX < pstcRtcTime->u8Minute)
    /* Check Hour */
    ||  (RTC_HOUR_MAX < pstcRtcTime->u8Hour)
    /* Check Day */
    || ((RTC_DAY_MAX < pstcRtcTime->u8Day) || (0 == pstcRtcTime->u8Day))
    /* Check Day of the week */
    ||  (RTC_DAYOFWEEK_MAX < pstcRtcTime->u8DayOfWeek)
    /* Check Month */
    || ((RTC_MONTH_MAX < pstcRtcTime->u8Month) || (0 == pstcRtcTime->u8Month))
    /* Check Year */
    || ((RTC_YEAR_MAX < pstcRtcTime->u8Year) || (0 == pstcRtcTime->u8Year)))
    {
        enResult = ErrorInvalidParameter;
    }

    else
    {
        /* Wait to complete transmission */
        enResult = RtcWaitTransComplete();
        if (Ok == enResult)
        {
            /* RTC running? */
            if (TRUE == FM4_RTC->WTCR10_f.ST)
            {
                /* Backup Interrupt Settings */
                stcWtcr13 = FM4_RTC->WTCR13_f;

                /* Wait to release busy */
                u32BusyTimeout = RTC_BUSY_POLLING_TIMEOUT;
                while (((TRUE == FM4_RTC->WTCR10_f.BUSY) || (TRUE == FM4_RTC->WTCR10_f.SCST)) &&
                        (0 != u32BusyTimeout)
                      )
                {
                    PDL_WAIT_LOOP_HOOK();
                    u32BusyTimeout--;
                }

                if (0 == u32BusyTimeout)
                {
                    enResult = ErrorTimeout;
                }

                if (Ok == enResult)
                {
                    /* Next instructions must be free of interruptions */
                    RtcDisableNvic();

                    /* Disable interrupt */
                    FM4_RTC->WTCR13 = 0;

                    /* Clear every second interrupt */
                    stcWtcr12 = FM4_RTC->WTCR12_f;
                    stcWtcr12.INTSI = FALSE;
                    FM4_RTC->WTCR12_f = stcWtcr12;

                    /* Enable every second interrupt */
                    FM4_RTC->WTCR13_f.INTSIE = TRUE;

                    /* Wait to occur interrupt */
                    do
                    {
                        stcWtcr12 = FM4_RTC->WTCR12_f;
                    } while (0 == stcWtcr12.INTSI);

                    /* Clear every second interrupt */
                    FM4_RTC->WTCR12_f.INTSI = 0;

                    /* Stop 1-second pulse detection */
                    FM4_RTC->WTCR10_f.SCST = TRUE;

                    /* Retrieve Interrupt Settings */
                    FM4_RTC->WTCR13_f = stcWtcr13;
                }
            }
            /* RTC not running ... */
            else
            {
                /* Next instructions must be free of interruptions */
                RtcDisableNvic();
            }

            if (Ok == enResult)
            {
                /* update RTC */
                FM4_RTC->WTSR  = RtcBinToBcd(pstcRtcTime->u8Second);
                FM4_RTC->WTMIR = RtcBinToBcd(pstcRtcTime->u8Minute);
                FM4_RTC->WTHR  = RtcBinToBcd(pstcRtcTime->u8Hour);
                FM4_RTC->WTDR  = RtcBinToBcd(pstcRtcTime->u8Day);
                FM4_RTC->WTDW  = pstcRtcTime->u8DayOfWeek;
                FM4_RTC->WTMOR = RtcBinToBcd(pstcRtcTime->u8Month);
                FM4_RTC->WTYR  = RtcBinToBcd(pstcRtcTime->u8Year);

                /* RTC running? */
                if (TRUE == FM4_RTC->WTCR10_f.ST)
                {
                    /* Enable 1-second pulse detection */
                    FM4_RTC->WTCR10_f.SCST = FALSE;
                }

                /* Transmit to VBAT domain and wait to complete transmission */
                enResult = RtcTransToVbat(RTC_WTCR20_CWRITE);

                /* Restore normal interupt handling */
                RtcRestoreNvic();
            }
        }
    }

    return (enResult);
} /* Rtc_SetDateTime */


/**
 ******************************************************************************
 ** \brief Check and Set Alarm Date and Alarm Time
 **
 ** \param [in]  pstcRtcAlarm   Pointer to RTC Alarm (Date and Time) structure
 **
 ** \retval  Ok                     Successful initialization
 ** \retval  ErrorInvalidParameter  If one of following conditions are met:
 **                                 - pstcRtcAlarm == NULL
 **                                 - Invalid value of a pstcRtcAlarm's element
 ** \retval  ErrorTimeout           Timeout to complete transmission
 **
 ******************************************************************************/
en_result_t Rtc_SetAlarmDateTime(stc_rtc_alarm_t* pstcRtcAlarm)
{
    en_result_t            enResult;
    stc_rtc_wtcr10_field_t stcWtcr10;
    stc_rtc_wtcr12_field_t stcWtcr12;
    stc_rtc_wtcr13_field_t stcWtcr13;

    /* Check for valid pointer */
    if ((NULL == pstcRtcAlarm)
    /* Check Alarm Day */
    || ((RTC_DAY_MAX < pstcRtcAlarm->u8AlarmDay) || (0 == pstcRtcAlarm->u8AlarmDay))
    /* Check Alarm Hour */
    ||  (RTC_HOUR_MAX < pstcRtcAlarm->u8AlarmHour)
    /* Check Alarm Minute */
    ||  (RTC_MIN_MAX < pstcRtcAlarm->u8AlarmMinute)
    /* Check Alarm Year */
    || ((RTC_YEAR_MAX < pstcRtcAlarm->u8AlarmYear) || (0 == pstcRtcAlarm->u8AlarmYear))
    /* Check Alarm Month */
    || ((RTC_MONTH_MAX < pstcRtcAlarm->u8AlarmMonth) || (0 == pstcRtcAlarm->u8AlarmMonth)))
    {
        enResult = ErrorInvalidParameter;
    }

    else
    {
        /* Wait to complete transmission */
        enResult = RtcWaitTransComplete();
        if (Ok == enResult)
        {
            /* Next instructions must be free of interruptions */
            RtcDisableNvic();

            /* RTC is running? */
            if (TRUE == FM4_RTC->WTCR10_f.ST)
            {
                stcWtcr10 = FM4_RTC->WTCR10_f;  /* to retrieve ST bit */
                stcWtcr13 = FM4_RTC->WTCR13_f;  /* Backup Interrupt settings */

                FM4_RTC->WTCR10_f.ST = FALSE;   /* RTC stops temporally */
                /* Transmit to VBAT domain and wait to complete transmission */
                enResult = RtcTransToVbat(RTC_WTCR20_CWRITE);
                if (Ok == enResult)
                {
                    /* Disable interrupt */
                    FM4_RTC->WTCR13 = 0;

                    /* Clear read interrupt */
                    stcWtcr12 = FM4_RTC->WTCR12_f;
                    stcWtcr12.INTCRI = FALSE;
                    FM4_RTC->WTCR12_f = stcWtcr12;

                    /* Enable read interrupt */
                    FM4_RTC->WTCR13_f.INTCRIE = TRUE;

                    /* Transfer from VBAT domain and wait to complete transmission */
                    if (Ok != RtcTransToVbat(RTC_WTCR20_CREAD))
                    {
                        FM4_RTC->WTCR13_f = stcWtcr13;  /* Retrieve Interrupt settings */
                        if (TRUE == stcWtcr10.ST)       /* Retrieve RTC running */
                        {
                            FM4_RTC->WTCR10_f.ST = TRUE;
                            /* Transfer from VBAT domain and wait to complete transmission */
                            (void)RtcTransToVbat(RTC_WTCR20_CWRITE);
                        }
                        enResult = ErrorTimeout;
                    }
                }

                if (Ok == enResult)
                {
                    /* Disable interrupt */
                    FM4_RTC->WTCR13 = 0;

                    /* Clear read interrupt and alarm interrupt */
                    stcWtcr12 = FM4_RTC->WTCR12_f;
                    stcWtcr12.INTCRI = FALSE;
                    stcWtcr12.INTSI = FALSE;
                    FM4_RTC->WTCR12_f = stcWtcr12;

                    /* update RTC Alarm registers */
                    FM4_RTC->ALDR  = RtcBinToBcd(pstcRtcAlarm->u8AlarmDay);
                    FM4_RTC->ALHR  = RtcBinToBcd(pstcRtcAlarm->u8AlarmHour);
                    FM4_RTC->ALMIR = RtcBinToBcd(pstcRtcAlarm->u8AlarmMinute);
                    FM4_RTC->ALYR  = RtcBinToBcd(pstcRtcAlarm->u8AlarmYear);
                    FM4_RTC->ALMOR = RtcBinToBcd(pstcRtcAlarm->u8AlarmMonth);

                    /* Enable every second interrupt */
                    FM4_RTC->WTCR13_f.INTSIE = TRUE;

                    /* Wait to occur every second interrupt */
                    do
                    {
                        stcWtcr12 = FM4_RTC->WTCR12_f;
                    } while (FALSE == stcWtcr12.INTSI);

                    /* Call the interruption handler (Clear every second interrupt) */
                    RtcIrqHandler();

                    /* Start RTC */
                    FM4_RTC->WTCR10_f.ST = TRUE;
                    /* Transmit to VBAT domain and wait to complete transmission */
                    enResult = RtcTransToVbat(RTC_WTCR20_CWRITE);

                    FM4_RTC->WTCR13_f = stcWtcr13;
                }
            }
            else
            {
                /* update RTC Alarm registers */
                FM4_RTC->ALDR  = RtcBinToBcd(pstcRtcAlarm->u8AlarmDay);
                FM4_RTC->ALHR  = RtcBinToBcd(pstcRtcAlarm->u8AlarmHour);
                FM4_RTC->ALMIR = RtcBinToBcd(pstcRtcAlarm->u8AlarmMinute);
                FM4_RTC->ALYR  = RtcBinToBcd(pstcRtcAlarm->u8AlarmYear);
                FM4_RTC->ALMOR = RtcBinToBcd(pstcRtcAlarm->u8AlarmMonth);

                /* Transmit to VBAT domain and wait to complete transmission */
                enResult = RtcTransToVbat(RTC_WTCR20_CWRITE);
            }
            RtcRestoreNvic();   /* Restore normal interupt handling */
        }
    }

    return (enResult);
} /* Rtc_SetAlarmDateTime */

/**
 ******************************************************************************
 ** \brief Enable and/or Disable RTC (and Timer) Interrupts
 **
 ** With this function the interrupts enabled and disabled by the RTC
 ** configuration can be changed.
 **
 ** \param [in]  pstcConfig       RTC configuration structure
 **
 ** \retval Ok                    Interrupt settings done
 ** \retval ErrorInvalidParameter pstcConfig == NULL
 **
 ******************************************************************************/
en_result_t Rtc_EnableDisableInterrupts(stc_rtc_config_t* pstcConfig)
{
    stc_rtc_wtcr13_field_t stcWtcr13 = { 0 }; /* At first, set all interrupts to disabling */
    en_result_t            enResult;

    enResult = ErrorInvalidParameter;

    /* Check for valid pointer */
    if (NULL != pstcConfig)
    {
        /* Enable counter value read completion interrupt */
        if (TRUE == pstcConfig->stcRtcInterruptEnable.ReadCompletionIrqEn)
        {
            stcWtcr13.INTCRIE = TRUE;
        }

        /* Enable timer rewrite error interrupt */
        if (TRUE == pstcConfig->stcRtcInterruptEnable.TimeRewriteErrorIrqEn)
        {
            stcWtcr13.INTERIE = TRUE;
        }

        /* Enable alarm coincidence interrupt */
        if (TRUE == pstcConfig->stcRtcInterruptEnable.AlarmIrqEn)
        {
            stcWtcr13.INTALIE = TRUE;
        }

        /* Enable timer underflow detection interrupt */
        if (TRUE == pstcConfig->stcRtcInterruptEnable.TimerIrqEn)
        {
            stcWtcr13.INTTMIE = TRUE;
        }

        /* Enable every hour interrupt */
        if (TRUE == pstcConfig->stcRtcInterruptEnable.OneHourIrqEn)
        {
            stcWtcr13.INTHIE = TRUE;
        }

        /* Enable every minute interrupt */
        if (TRUE == pstcConfig->stcRtcInterruptEnable.OneMinuteIrqEn)
        {
            stcWtcr13.INTMIE = TRUE;
        }

        /* Enable every second interrupt */
        if (TRUE == pstcConfig->stcRtcInterruptEnable.OneSecondIrqEn)
        {
            stcWtcr13.INTSIE = TRUE;
        }

        /* Enable every 0.5 second interrupt */
        if (TRUE == pstcConfig->stcRtcInterruptEnable.HalfSecondIrqEn)
        {
            stcWtcr13.INTSSIE = TRUE;
        }

        FM4_RTC->WTCR13_f = stcWtcr13;  /* Update WTCR13 */

        enResult = Ok;
    }

    return (enResult);
} /* Rtc_EnableDisableInterrupts */

/**
 ******************************************************************************
 ** \brief Enable and/or Disable RTC Alarm Register
 **
 ** With this function the Alarm registers can be en- or disabled according
 ** the configuration
 **
 ** \param [in]  pstcRtcAlarmEn   Pointer to settings of alarm enable
 **
 ** \retval Ok                    Alarm Register set
 ** \retval ErrorInvalidParameter pstcRtcAlarmEn == NULL
 ** \retval ErrorTimeout          Timeout to complete transmission
 **
 ******************************************************************************/
en_result_t Rtc_EnableDisableAlarmRegisters(stc_rtc_alarm_enable_t* pstcRtcAlarmEn)
{
    /* At first, set all alarms to disabling */
    stc_rtc_wtcr11_field_t stcWtcr11 = { 0 };
    en_result_t            enResult;

    enResult = ErrorInvalidParameter;

    /* Check for valid pointer */
    if (NULL != pstcRtcAlarmEn)
    {
        /* Enable year alarm */
        if (TRUE == pstcRtcAlarmEn->AlarmYearEnable)
        {
            stcWtcr11.YEN = TRUE;
        }

        /* Enable month alarm */
        if (TRUE == pstcRtcAlarmEn->AlarmMonthEnable)
        {
            stcWtcr11.MOEN = TRUE;
        }

        /* Enable day alarm */
        if (TRUE == pstcRtcAlarmEn->AlarmDayEnable)
        {
            stcWtcr11.DEN = TRUE;
        }

        /* Enable hour alarm */
        if (TRUE == pstcRtcAlarmEn->AlarmHourEnable)
        {
            stcWtcr11.HEN = TRUE;
        }

        /* Enable minute alarm */
        if (TRUE == pstcRtcAlarmEn->AlarmMinuteEnable)
        {
            stcWtcr11.MIEN = TRUE;
        }

        FM4_RTC->WTCR11_f = stcWtcr11; /* Update WTCR11 */

        /* Transmit to VBAT domain and wait to complete transmission */
        enResult = RtcTransToVbat(RTC_WTCR20_CWRITE);
    }

    return (enResult);
} /* Rtc_EnableDisableAlarmRegisters */

/**
 ******************************************************************************
 ** \brief Read recent Time of RTC in polling mode
 **
 ** This function requests a copy of the recent time to the RTC registers and
 ** waits until finish. The recent time is transfered to the configuration
 ** pointer structure.
 **
 ** \note In this function a possible enabled Read Completion Interrupt
 **       (INTCRI) is temporarily disabled.
 **
 ** \param [in,out]   pstcRtcTime      RTC Date and Time structure
 **
 ** \retval Ok                         Date and time sucessfully transfered
 ** \retval ErrorInvalidParameter      pstcRtcTime == NULL
 ** \retval ErrorTimeout               Polling loop timed out
 **
 ******************************************************************************/
en_result_t Rtc_ReadDateTimePolling(stc_rtc_time_t* pstcRtcTime)
{
    en_result_t enResult;
    uint8_t     u8Wtcr13;
    uint8_t     u8Wtcr12;

    enResult = ErrorInvalidParameter;

    /* Check for valid pointer */
    if (NULL != pstcRtcTime)
    {
        u8Wtcr13 = FM4_RTC->WTCR13;         /* Save Interrupt Enable */
        FM4_RTC->WTCR13_f.INTCRIE = FALSE;  /* Disable INTCRIE */

        /* Wait to complete recalling RTC setting from VBAT domain */
        enResult = RtcWaitTransComplete();
        if (Ok != enResult)
        {
            /* Restore Inerrupt Enable */
            FM4_RTC->WTCR13 = u8Wtcr13;
        }
        else
        {
            /* Transfer from VBAT domain and wait to complete transmission */
            enResult = RtcTransToVbat(RTC_WTCR20_CREAD);
            if (Ok != enResult)
            {
                /* Restore Inerrupt Enable */
                FM4_RTC->WTCR13 = u8Wtcr13;
            }
        }

        if (Ok == enResult)
        {
            /* Clear INTCRI flag */
            u8Wtcr12 = FM4_RTC->WTCR12;
            u8Wtcr12 &= ~(RTC_WTCR12_INTCRI_FLAG_POSITION);
            FM4_RTC->WTCR12 = u8Wtcr12;

            /* Store Date & Time in pointered structure */
            pstcRtcTime->u8Second    = RtcBcdToBin(FM4_RTC->WTSR);
            pstcRtcTime->u8Minute    = RtcBcdToBin(FM4_RTC->WTMIR);
            pstcRtcTime->u8Hour      = RtcBcdToBin(FM4_RTC->WTHR);
            pstcRtcTime->u8Day       = RtcBcdToBin(FM4_RTC->WTDR);
            pstcRtcTime->u8DayOfWeek = RtcBcdToBin(FM4_RTC->WTDW);
            pstcRtcTime->u8Month     = RtcBcdToBin(FM4_RTC->WTMOR);
            pstcRtcTime->u8Year      = RtcBcdToBin(FM4_RTC->WTYR);

            /* Restore Inerrupt Enable */
            FM4_RTC->WTCR13 = u8Wtcr13;
        }
    }

    return (enResult);
} /* Rtc_ReadDateTimePolling */

/**
 ******************************************************************************
 ** \brief Request for Read recent Time of RTC
 **
 ** This function requests a copy of the recent time to the RTC registers.
 ** In the RTC ISR the callback function stc_rtc_config_t#pfnReadCallback
 ** with all its 7 arguments for date and time is called.
 **
 ** \note This function needs the INTCRIE bit (Read Completion Interrupt
 **       Enable) set to '1' by Rtc_Init() or Rtc_EnableDisableInterrupts()
 **
 ** \retval Ok                    Date and time sucessfully transfered
 ** \retval ErrorNotReady         A previously request was not finished
 **
 ******************************************************************************/
en_result_t Rtc_RequestDateTime(void)
{
    en_result_t enResult;

    /* Check if a transmission via VBAT domain was started and did not finish yet ... */
    if (TRUE == FM4_RTC->WTCR10_f.TRANS)
    {
        enResult = ErrorNotReady;
    }
    else
    {
        /* Start request */
        u8CRead = TRUE;
        FM4_RTC->WTCR20 = RTC_WTCR20_CREAD;
        enResult = Ok;
    }

    return (enResult);
}

/**
 ******************************************************************************
 ** \brief Set Timer Status of RTC
 **
 ** This function sets the mode and Timer value of the RTC Timer.
 **
 ** \pre The RTC has to be initialized before to use this function properly.
 **      Also use the Timer Interrupt enable by Rtc_Init()
 **
 ** \param [in]  pstcConfig         Timer Configuration
 **
 ** \retval  Ok                     Timer sucessfully set
 ** \retval  ErrorInvalidParameter  If one of following conditions are met:
 **                                 - pstcConfig == NULL
 **                                 - Invalid value of a timer for setting
 ** \retval  ErrorTimeout           Timeout to complete transmission
 **
 ******************************************************************************/
en_result_t Rtc_TimerSet(stc_rtc_timer_config_t* pstcConfig)
{
    en_result_t            enResult;
    uint32_t               u32WaitPollingTimeout;
    stc_rtc_wtcr12_field_t stcWtcr12;
    stc_rtc_wtcr13_field_t stcWtcr13;
    stc_rtc_wtcr21_field_t stcWtcr21;

    /* Check for valid pointer and get pointer to internal data struct and check */
    if ((NULL == pstcConfig)
    ||  (0 == pstcConfig->u32TimerValue)
    ||  (RTC_MAX_TIMER_SET_VALUE < pstcConfig->u32TimerValue)
       )
    {
        enResult = ErrorInvalidParameter;
    }
    else
    {
        /* Wait to complete transmission */
        enResult = RtcWaitTransComplete();
        if (Ok == enResult)
        {
            stcWtcr21 = FM4_RTC->WTCR21_f;
            /* At first, the RTC timer stops */
            FM4_RTC->WTCR21_f.TMST = FALSE;
            /* Transmit to VBAT domain and wait to complete transmission */
            enResult = RtcTransToVbat(RTC_WTCR20_CWRITE);

            /* Set timer counter control */
            if (TRUE == pstcConfig->bTimerIntervalEnable)
            {
                /* Operates at a specific interval */
                FM4_RTC->WTCR21_f.TMEN = TRUE;
            }
            else
            {
                /* Operates after a specific time */
                FM4_RTC->WTCR21_f.TMEN = FALSE;
            }
            /* Transmit to VBAT domain and wait to complete transmission */
            enResult = RtcTransToVbat(RTC_WTCR20_CWRITE);
            if (Ok != enResult)
            {
                /* Retrieve WTCR21 */
                FM4_RTC->WTCR21_f = stcWtcr21;
                /* Transmit to VBAT domain and wait to complete transmission */
                (void)RtcTransToVbat(RTC_WTCR20_CWRITE);
            }
        }
        if (Ok == enResult)
        {
            /* Check the timer has stopped */
            u32WaitPollingTimeout = RTC_BUSY_POLLING_TIMEOUT;
            while((TRUE == FM4_RTC->WTCR21_f.TMRUN) && (0 != u32WaitPollingTimeout))
            {
                PDL_WAIT_LOOP_HOOK();
                u32WaitPollingTimeout--;
            }
            if (0 == u32WaitPollingTimeout)
            {
                enResult = ErrorTimeout;
            }
            else
            {
                /* Next instructions must be free of interruptions */
                RtcDisableNvic();
                stcWtcr13 = FM4_RTC->WTCR13_f;  /* Back up interrupt settings */
                if (TRUE == FM4_RTC->WTCR10_f.RUN)  /* RTC running ? */
                {
                    /* Disable interrupt */
                    FM4_RTC->WTCR13 = 0;

                    /* Clear every second interrupt */
                    stcWtcr12 = FM4_RTC->WTCR12_f;
                    stcWtcr12.INTSI = FALSE;
                    FM4_RTC->WTCR12_f = stcWtcr12;

                    /* Enable every second interrupt */
                    FM4_RTC->WTCR13_f.INTSIE = TRUE;

                    /* Wait to occur interrupt */
                    do
                    {
                        stcWtcr12 = FM4_RTC->WTCR12_f;
                    } while (0 == stcWtcr12.INTSI);

                    /* Clear every second interrupt */
                    FM4_RTC->WTCR12_f.INTSI = 0;
                }


                /* Set up new Value */
                FM4_RTC->WTTR0 = (uint8_t)(pstcConfig->u32TimerValue);
                FM4_RTC->WTTR1 = (uint8_t)((pstcConfig->u32TimerValue) >> RTC_8BIT);
                FM4_RTC->WTTR2 = (uint8_t)((pstcConfig->u32TimerValue) >> RTC_16BIT);
                /* Transmit to VBAT domain and wait to complete transmission */
                enResult = RtcTransToVbat(RTC_WTCR20_CWRITE);

                FM4_RTC->WTCR13_f = stcWtcr13;  /* Retrieve interrupt settings */
                RtcRestoreNvic();
            }
        }
    }

    return (enResult);
} /* Rtc_TimerSet */

/**
 ******************************************************************************
 ** \brief Start Timer Status of RTC
 **
 ** This function starts the Timer of the RTC Timer.
 **
 ** \pre The RTC has to be initialized before to use this function properly.
 **      Also use the Timer Interrupt enable by Rtc_Init()
 **
 ** \retval  Ok                     Timer sucessfully start
 ** \retval  ErrorTimeout           Timeout to complete transmission
 **
 ******************************************************************************/
en_result_t Rtc_TimerStart(void)
{
    en_result_t            enResult;
    stc_rtc_wtcr21_field_t stcWtcr21;

    /* Back up WTCR21 */
    stcWtcr21 = FM4_RTC->WTCR21_f;
    /* Start the timer */
    FM4_RTC->WTCR21_f.TMST = TRUE;
    /* Transmit to VBAT domain and wait to complete transmission */
    enResult = RtcTransToVbat(RTC_WTCR20_CWRITE);
    if (Ok != enResult)
    {
        /* Retrieve WTCR21 */
        FM4_RTC->WTCR21_f = stcWtcr21;
        /* Transmit to VBAT domain and wait to complete transmission */
        (void)RtcTransToVbat(RTC_WTCR20_CWRITE);
    }

    return (enResult);
} /* Rtc_TimerStart */

/**
 ******************************************************************************
 ** \brief Stops Timer Status of RTC
 **
 ** This function stops the Timer of the RTC Timer.
 **
 ** \pre The RTC has to be initialized before to use this function properly.
 **      Also use the Timer Interrupt enable by Rtc_Init()
 **
 ** \retval  Ok                     Timer sucessfully stop
 ** \retval  ErrorTimeout           Timeout to complete transmission
 **
 ******************************************************************************/
en_result_t Rtc_TimerStop(void)
{
    en_result_t            enResult;
    stc_rtc_wtcr21_field_t stcWtcr21;

    /* Back up WTCR21 */
    stcWtcr21 = FM4_RTC->WTCR21_f;
    /* Stop the timer */
    FM4_RTC->WTCR21_f.TMST = FALSE;
    /* Transmit to VBAT domain and wait to complete transmission */
    enResult = RtcTransToVbat(RTC_WTCR20_CWRITE);
    if (Ok != enResult)
    {
        /* Retrieve WTCR21 */
        FM4_RTC->WTCR21_f = stcWtcr21;
        /* Transmit to VBAT domain and wait to complete transmission */
        (void)RtcTransToVbat(RTC_WTCR20_CWRITE);
    }

    return (enResult);
} /* Rtc_TimerStop */

/**
 ******************************************************************************
 ** \brief Read Timer Status of RTC
 **
 ** This function provides the status of the TMRUN bit of the WTCR21 register.
 **
 ** \retval  RtcTimerNoOperation    Timer Counter is not operating
 ** \retval  RtcTimerInOperation    Timer Counter is operationg
 **
 ******************************************************************************/
en_rtc_timer_status_t Rtc_TimerStatusRead(void)
{
    stc_rtc_wtcr21_field_t stcWtcr21;
    en_rtc_timer_status_t  enStatus;

    stcWtcr21 = FM4_RTC->WTCR21_f;

    if (FALSE == stcWtcr21.TMRUN)
    {
        enStatus = RtcTimerNoOperation;
    }
    else
    {
        enStatus = RtcTimerInOperation;
    }

    return (enStatus);
} /* Rtc_TimerStatusRead */

/**
 ******************************************************************************
 ** \brief Read Transmission Status of RTC
 **
 ** This function provides the status of the TRANS bit of the WTCR10 register.
 **
 ** \retval  RtcTransNoOperation    Transmission is not operating
 ** \retval  RtcTransInOperation    Transmission is operationg
 **
 ******************************************************************************/
en_rtc_trans_status_t Rtc_TransStatusRead(void)
{
    en_rtc_trans_status_t  enStatus;

    if (FALSE == FM4_RTC->WTCR10_f.TRANS)
    {
        enStatus = RtcTransNoOperation;
    }
    else
    {
        enStatus = RtcTransInOperation;
    }

    return (enStatus);
} /* Rtc_TransStatusRead */

/**
 ******************************************************************************
 ** \brief De-Initialize RTC
 **
 ** This function stops and resets the RTC module and its interrupts.
 **
 ** \retval  Ok                     Internal data has been setup
 ** \retval  ErrorTimeout           Timeout to complete transmission
 **
 ******************************************************************************/
en_result_t Rtc_DeInit(void)
{
    en_result_t enResult;

    /* Wait to complete transmission */
    enResult = RtcWaitTransComplete();
    if (Ok == enResult)
    {
        RtcDeInitIrq();

        /* Set all registers of RTC instance to '0' */
        FM4_RTC->WTCR10   = 0;
        FM4_RTC->WTCR11   = 0;
        FM4_RTC->WTCR12   = 0;
        FM4_RTC->WTCR13   = 0;
        FM4_RTC->WTCR20   = 0;
        FM4_RTC->WTCR21   = 0;
        FM4_RTC->WTSR     = 0;
        FM4_RTC->WTMIR    = 0;
        FM4_RTC->WTHR     = 0;
        FM4_RTC->WTDR     = 0;
        FM4_RTC->WTDW     = 0;
        FM4_RTC->WTMOR    = 0;
        FM4_RTC->WTYR     = 0;
        FM4_RTC->ALMIR    = 0;
        FM4_RTC->ALHR     = 0;
        FM4_RTC->ALDR     = 0;
        FM4_RTC->ALMOR    = 0;
        FM4_RTC->ALYR     = 0;
        /* Transmit to VBAT domain and wait to complete transmission */
        enResult = RtcTransToVbat(RTC_WTCR20_CWRITE);
        if (Ok == enResult)
        {
            /* Set all registers of RTCCLK instance to '0' */
            FM4_RTC->WTCAL0   = 0;
            FM4_RTC->WTCAL1   = 0;
            FM4_RTC->WTCALEN  = 0;
            FM4_RTC->WTDIV    = 0;
            FM4_RTC->WTDIVEN  = 0;
            FM4_RTC->WTCALPRD = 0;
            FM4_RTC->WTCOSEL  = 0;
            /* Transmit to VBAT domain and wait to complete transmission */
            enResult = RtcTransToVbat(RTC_WTCR20_PWRITE);
        }
    }

    return (enResult);
} /* Rtc_DeInit */

/**
 ******************************************************************************
 ** \brief Get raw time
 **
 ** This function calculates the "raw" time ('UNIX time').
 **
 ** \param [in] pstcRtcTime     Pointer to RTC Time structure
 **
 ** \return     Calculated time or '-1' on error
 **
 ******************************************************************************/
time_t Rtc_GetRawTime(stc_rtc_time_t* pstcRtcTime)
{
    time_t uiTime;
    struct tm stcTime;

    /* Check for NULL pointer */
    if (NULL == pstcRtcTime)
    {
        uiTime = (time_t)RTC_ERR;
    }
    else
    {
        /* Convert RTC time structure to time.h tm structure */
        RtcToTm(pstcRtcTime, &stcTime);
        uiTime = mktime(&stcTime);
    }

    /* Return raw time (UNIX time) */
    return (uiTime);
} /* Rtc_GetRawTime */

/**
 ******************************************************************************
 ** \brief Set Day of the Week
 **
 ** This function calculates the day of the week from YY-MM-DD in the Time
 ** structure. It uses mktime of time.h library.
 **
 ** \param  [in,out] pstcRtcTime     RTC Time structure
 **
 ** \retval Ok                       Internal data has been setup
 ** \retval ErrorInvalidParameter    pstcRtcTime == NULL or mktime failed
 **
 ******************************************************************************/
en_result_t Rtc_SetDayOfWeek(stc_rtc_time_t* pstcRtcTime)
{
    en_result_t enResult;
    struct tm stcTime;

    enResult = ErrorInvalidParameter;

    /* Check for NULL pointer */
    if (NULL != pstcRtcTime)
    {
        enResult = Ok;
        /* Convert RTC time structure to time.h tm structure */
        RtcToTm(pstcRtcTime, &stcTime);

        /* Caluculated raw time (UNIX time) is error */
        if ((time_t)RTC_ERR == mktime(&stcTime))
        {
            enResult = ErrorInvalidParameter;
        }

        if (Ok == enResult)
        {
            /* Set caluculated the day of week */
            pstcRtcTime->u8DayOfWeek = (uint8_t)(stcTime.tm_wday);
        }
    }

    return (enResult);
} /* Rtc_SetDayOfWeek */

/**
 ******************************************************************************
 ** \brief Sets the RTC time structure from raw time
 **
 ** This function calculates from the RTC time structure "raw" time
 ** ('UNIX time').
 **
 ** \param [out] pstcRtcTime         RTC Time structure
 ** \param [in]  tRawTime            "Raw" time
 **
 ** \retval Ok                       Internal data has been setup
 ** \retval ErrorInvalidParameter    pstcRtcTime == NULL or localtime failed
 **
 ******************************************************************************/
en_result_t Rtc_SetTime(stc_rtc_time_t* pstcRtcTime,
                        time_t          tRawTime
                       )
{
    en_result_t enResult;
    struct tm* pstcTime;

    enResult = ErrorInvalidParameter;

    /* Check for NULL pointer */
    if (NULL != pstcRtcTime)
    {
        /* Get the pointer which converted to RTC time structure from raw time */
        pstcTime = localtime((const time_t*) &tRawTime);

        /* Un-success */
        if (NULL == pstcTime)
        {
            enResult = ErrorInvalidParameter;
        }
        else
        {
            pstcRtcTime->u8Year      = (uint8_t)((pstcTime->tm_year + RTC_YEAR_1900) - RTC_YEAR_2000);
            pstcRtcTime->u8Month     = (uint8_t)(pstcTime->tm_mon + 1);
            pstcRtcTime->u8Day       = (uint8_t)(pstcTime->tm_mday);
            pstcRtcTime->u8Second    = (uint8_t)(pstcTime->tm_sec);
            pstcRtcTime->u8Minute    = (uint8_t)(pstcTime->tm_min);
            pstcRtcTime->u8Hour      = (uint8_t)(pstcTime->tm_hour);
            pstcRtcTime->u8DayOfWeek = (uint8_t)(pstcTime->tm_wday);
            enResult = Ok;
        }
    }

    return (enResult);
} /* Rtc_SetTime */

/**
 ******************************************************************************
 ** \brief Write the data to Backup Register (byte access)
 **
 ** \param [in]  u8Data             Data for backup
 ** \param [in]  u8Area             Backup register area
 **                                 This parameter should be set RtcBkupRegAreaXX
 **                                 (XX is 00 to 1F for Unicorn)
 **
 ** \retval  Ok                     Internal data has been setup
 ** \retval  ErrorInvalidParameter  Backup register area is out of range
 ** \retval  ErrorTimeout           Timeout to complete transmission
 **
 ******************************************************************************/
en_result_t Rtc_WriteBkupReg8(uint8_t u8Data,
                              uint8_t u8Area
                             )
{
    volatile uint8_t* pu8BkupRegAddr;
    en_result_t       enResult;

    enResult = ErrorInvalidParameter;
    /* Check for valid pointer and register area */
    if (RtcBkupRegAreaMax > u8Area)
    {
        /* Wait to complete transmission */
        enResult = RtcWaitTransComplete();
        if (Ok == enResult)
        {
            pu8BkupRegAddr = (uint8_t *)(RTC_BKUP_REG_BASE + u8Area);

            *pu8BkupRegAddr = u8Data;

            /* Transmit to VBAT domain and wait to complete transmission */
            enResult = RtcTransToVbat(RTC_WTCR20_BWRITE);
        }
    }

    return (enResult);
} /* Rtc_WriteBkup8 */

/**
 ******************************************************************************
 ** \brief Write the data to Backup Register (half word access)
 **
 ** \param [in]  u16Data            Data for backup
 ** \param [in]  u8Area             Backup register area
 **                                 This parameter should be set RtcBkupRegAreaXX
 **                                 (XX is 00 to 1F for Unicorn)
 **
 ** \retval  Ok                     Internal data has been setup
 ** \retval  ErrorInvalidParameter  Backup register area is out of range
 **                                 or not 2byte alignment
 ** \retval  ErrorTimeout           Timeout to complete transmission
 **
 ******************************************************************************/
en_result_t Rtc_WriteBkupReg16(uint16_t u16Data,
                               uint8_t  u8Area
                              )
{
    volatile uint16_t* pu16BkupRegAddr;
    en_result_t        enResult;

    enResult = ErrorInvalidParameter;

    /* Check for valid pointer and register area */
    if ((RtcBkupRegAreaMax > u8Area)
    /* Check alignment */
    &&  (0 == (u8Area % RTC_2BYTES))
       )
    {
        /* Wait to complete transmission */
        enResult = RtcWaitTransComplete();
        if (Ok == enResult)
        {
            pu16BkupRegAddr = (uint16_t *)((uint8_t *)(RTC_BKUP_REG_BASE + u8Area));

            *pu16BkupRegAddr = u16Data;

            /* Transmit to VBAT domain and wait to complete transmission */
            enResult = RtcTransToVbat(RTC_WTCR20_BWRITE);
        }
    }

    return (enResult);
} /* Rtc_WriteBkup16 */

/**
 ******************************************************************************
 ** \brief Write the data to Backup Register (word access)
 **
 ** \param [in]  u32Data            Data for backup
 ** \param [in]  u8Area             Backup register area
 **                                 This parameter should be set RtcBkupRegAreaXX
 **                                 (XX is 00 to 1F for Unicorn)
 **
 ** \retval  Ok                     Internal data has been setup
 ** \retval  ErrorInvalidParameter  Backup register area is out of range
 **                                 or not 4byte alignment
 ** \retval  ErrorTimeout           Timeout to complete transmission
 **
 ******************************************************************************/
en_result_t Rtc_WriteBkupReg32(uint32_t u32Data,
                               uint8_t  u8Area
                              )
{
    volatile uint32_t* pu32BkupRegAddr;
    en_result_t        enResult;

    enResult = ErrorInvalidParameter;

    /* Check for valid pointer and register area */
    if ((RtcBkupRegAreaMax > u8Area)
    /* Check alignment */
    &&  (0 == (u8Area % RTC_4BYTES))
       )
    {
        /* Wait to complete transmission */
        enResult = RtcWaitTransComplete();
        if (Ok == enResult)
        {
            pu32BkupRegAddr = (uint32_t *)((uint8_t *)(RTC_BKUP_REG_BASE + u8Area));

            *pu32BkupRegAddr = u32Data;

            /* Transmit to VBAT domain and wait to complete transmission */
            enResult = RtcTransToVbat(RTC_WTCR20_BWRITE);
        }
    }

    return (enResult);
} /* Rtc_WriteBkup32 */

/**
 ******************************************************************************
 ** \brief Read the data from Backup Register (byte access)
 **
 ** \param [in]  u8Area             Backup register area
 **                                 This parameter should be set RtcBkupRegAreaXX
 **                                 (XX is 00 to 1F for Unicorn)
 **
 ** \retval  Backup register value (byte size)
 **          (If u8Area is invalid, this function returns 0.)
 **
 ******************************************************************************/
uint8_t Rtc_ReadBkupReg8(uint8_t u8Area)
{
    uint8_t u8RetVal = 0;
    volatile uint8_t* pu8BkupRegAddr;

    /* Check for valid pointer and register area */
    if (RtcBkupRegAreaMax > u8Area)
    {
        /* Wait to complete transmission */
        if (Ok == RtcWaitTransComplete())
        {
            /* Transmit from VBAT domain and wait to complete transmission */
            if (Ok != RtcTransToVbat(RTC_WTCR20_BREAD))
            {
                pu8BkupRegAddr = (uint8_t *)(RTC_BKUP_REG_BASE + u8Area);
                u8RetVal = *pu8BkupRegAddr;
            }
        }
    }

    return (u8RetVal);
} /* Rtc_WriteBkup8 */

/**
 ******************************************************************************
 ** \brief Read the data from Backup Register (half word access)
 **
 ** \param [in]  u8Area             Backup register area
 **                                 This parameter should be set RtcBkupRegAreaXX
 **                                 (XX is 00 to 1F for Unicorn)
 **
 ** \retval  Backup register value (half word size)
 **          (If u8Area is invalid, this function returns 0.)
 **
 ******************************************************************************/
uint16_t Rtc_ReadBkupReg16(uint8_t u8Area)
{
    uint16_t u16RetVal = 0;
    volatile uint16_t* pu16BkupRegAddr;

    /* Check for valid pointer and register area */
    if ((RtcBkupRegAreaMax > u8Area)
    /* Check alignment */
    &&  (0 == (u8Area % RTC_2BYTES))
       )
    {
        /* Wait to complete transmission */
        if (Ok == RtcWaitTransComplete())
        {
            /* Transmit from VBAT domain and wait to complete transmission */
            if (Ok != RtcTransToVbat(RTC_WTCR20_BREAD))
            {
                pu16BkupRegAddr = (uint16_t *)((uint8_t *)(RTC_BKUP_REG_BASE + u8Area));
                u16RetVal = *pu16BkupRegAddr;
            }
        }
    }

    return (u16RetVal);
} /* Rtc_WriteBkup16 */

/**
 ******************************************************************************
 ** \brief Read the data from Backup Register (word access)
 **
 ** \param [in]  u8Area             Backup register area
 **                                 This parameter should be set RtcBkupRegAreaXX
 **                                 (XX is 00 to 1F for Unicorn)
 **
 ** \retval  Backup register value (word size)
 **          (If u8Area is invalid, this function returns 0.)
 **
 ******************************************************************************/
uint32_t Rtc_ReadBkupReg32(uint8_t u8Area)
{
    uint32_t u32RetVal = 0;
    volatile uint32_t* pu32BkupRegAddr;

    /* Check for valid pointer and register area */
    if ((RtcBkupRegAreaMax > u8Area)
    /* Check alignment */
    &&  (0 == (u8Area % RTC_4BYTES))
       )
    {
        /* Wait to complete transmission */
        if (Ok == RtcWaitTransComplete())
        {
            /* Transmit from VBAT domain and wait to complete transmission */
            if (Ok != RtcTransToVbat(RTC_WTCR20_BREAD))
            {
                pu32BkupRegAddr = (uint32_t *)((uint8_t *)(RTC_BKUP_REG_BASE + u8Area));
                u32RetVal = *pu32BkupRegAddr;
            }
        }
    }

    return (u32RetVal);
} /* Rtc_WriteBkup32 */

//@} // RtcGroup

#endif /* #if (defined(PDL_PERIPHERAL_RTC_ACTIVE)) */
/******************************************************************************/
/* EOF (not truncated)                                                        */
/******************************************************************************/
