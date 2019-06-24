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
/** \file rtc.h
 **
 ** Headerfile for RTC functions
 **  
 **
 ** History:
 **   - 2013-04-04  1.0  NT   First version.
 **   - 2013-11-22  1.1  EZ   Correct the definition value of 
 **                           "RTC_INTERRUPT_ENABLE_MASK" from 0x0F to 0xFF
 **   - 2014-01-15  1.2  EZ   Add Rtc_GetRunStatus() function.
 **
 ******************************************************************************/

#ifndef __RTC_H__
#define __RTC_H__

/******************************************************************************/
/* Include files                                                              */
/******************************************************************************/
#include "pdl.h"
#include "time.h"

#if (defined(PDL_PERIPHERAL_RTC_ACTIVE))

/* C binding of definitions if building with C++ compiler */
#ifdef __cplusplus
extern "C"
{
#endif

/**
 ******************************************************************************
 ** \defgroup RtcGroup Real Time Clock (RTC)
 **
 ** Provided functions of RTC module:
 ** 
 ** - Rtc_Init()
 ** - Rtc_SetDateTime()
 ** - Rtc_SetAlarmDateTime()
 ** - Rtc_EnableDisableInterrupts()
 ** - Rtc_EnableDisableAlarmRegisters()
 ** - Rtc_ReadDateTimePolling()
 ** - Rtc_RequestDateTime()
 ** - Rtc_TimerSet()
 ** - Rtc_TimerStart()
 ** - Rtc_TimerStop()
 ** - Rtc_TimerStatusRead()
 ** - Rtc_TransStatusRead()
 ** - Rtc_DeInit()
 ** - Rtc_GetRawTime()
 ** - Rtc_SetDayOfWeek()
 ** - Rtc_SetTime()
 ** - Rtc_WriteBkupReg8()
 ** - Rtc_WriteBkupReg16()
 ** - Rtc_WriteBkupReg32()
 ** - Rtc_ReadBkupReg8()
 ** - Rtc_ReadBkupReg16()
 ** - Rtc_ReadBkupReg32()
 **
 ** Rtc_Init() initializes the RTC with the given time and date given in the
 ** RTC configuration. It also initialized the NVIC, if specidfied.
 ** Rtc_SetDateTime() allows to set a new date and time.
 ** Rtc_SetAlarmDateTime() allows to set a new alarm date and time
 ** Rtc_EnableDisableInterrupts() allows to change the interrupt configurations
 ** on runtime without touching the RTC work.
 ** With Rtc_EnableDisableAlarmRegisters() the comparison to the recent time
 ** can be adjusted on runtime.
 ** Rtc_ReadClockStatus() reads out the status of the RTC.
 ** Rtc_TimerSet() sets the timer value for its interval or one-shot counting.
 ** Rtc_TimerStart() starts and Rtc_TimerStop() stops the Timer.
 ** With Rtc_ReadDateTimePolling() the recent date and time can be stored in
 ** the pointered configuration structure while the Read Completion Interrupt
 ** is temporarily disabled.
 ** Rtc_TimerStatusRead() reads out the Timer status.
 ** Rtc_TransStatusRead() reads out the transmission status.
 ** And finally Rtc_DeInit() allows to de-initialize all RTC functions and
 ** possible interrupts. Also here to touch the NVIC registers can be set.
 **
 ** Rtc_GetRawTime() is a helper function, which calculates the 'raw' time
 ** (UNIX time) from the RTC time structure #stc_rtc_time_t.
 ** Rtc_SetDayOfWeek() is a helper function, which sets the day of the week
 ** calculated from the date and time given in #stc_rtc_time_t.
 ** Rtc_SetTime() is a helper function which calculates the RTC time structure
 ** from the 'raw' time.
 **
 ** Rtc_WriteBkupReg8(), Rtc_WriteBkupReg16() and Rtc_WriteBkupReg32() write
 ** a single byte, 16-bit or 32-bit word to a given backup register area.
 ** Rtc_ReadBkupReg8(), Rtc_ReadBkupReg16() and Rtc_ReadBkupReg32() read
 ** a single byte, 16-bit or 32-bit word from a given backup register area.
 **
 ******************************************************************************/
//@{

/**
 ******************************************************************************
 ** \brief Rtccallback function prototypes.
 ******************************************************************************/

/******************************************************************************
 * Global definitions
 ******************************************************************************/

#define RTC0 (*((volatile stc_rtcn_t *)FM4_RTC_BASE))

#define RTC_INTERRUPT_ENABLE_MASK   (0xFFu)     ///< Number of Irq bit fields, see #stc_rtc_interrupts_t
#define RTC_POLLING_TIMEOUT         (100000u)   ///< Timeout for WTCR2.CREAD bit polling
#define RTC_BUSY_POLLING_TIMEOUT    (200000u)   ///< Timeout for BUSY bit polling
#define RTC_TRANS_POLLING_TIMEOUT   (500000u)   ///< Timeout for TRANS bit polling

/**
 ******************************************************************************
 ** \brief Year calculation macro for adjusting RTC year format
 ******************************************************************************/
#define Rtc_Year(a) ((a) - 2000)

/**
 ******************************************************************************
 ** \brief Year calculation macro for adjusting RTC year format
 ******************************************************************************/

/**
 ******************************************************************************
 ** \brief Bit Positions of Enable ALARM Flasgs in WTCR11
 ******************************************************************************/  
#define RTC_WTCR11_MIEN_FLAG_POSITION (0x01u)
#define RTC_WTCR11_HEN_FLAG_POSITION  (0x02u)
#define RTC_WTCR11_DEN_FLAG_POSITION  (0x04u)
#define RTC_WTCR11_MOEN_FLAG_POSITION (0x08u)
#define RTC_WTCR11_YEN_FLAG_POSITION  (0x10u)
#define RTC_WTCR11_ALL_FLAG_POSITION  (0x1Fu)

/**
 ******************************************************************************
 ** \brief Bit Positions of Interrupt Flasgs in WTCR12
 ******************************************************************************/  
#define RTC_WTCR12_INTSSI_FLAG_POSITION (0x01u)
#define RTC_WTCR12_INTSI_FLAG_POSITION  (0x02u)
#define RTC_WTCR12_INTMI_FLAG_POSITION  (0x04u)
#define RTC_WTCR12_INTHI_FLAG_POSITION  (0x08u)
#define RTC_WTCR12_INTTMI_FLAG_POSITION (0x10u)
#define RTC_WTCR12_INTALI_FLAG_POSITION (0x20u)
#define RTC_WTCR12_INTERI_FLAG_POSITION (0x40u)
#define RTC_WTCR12_INTCRI_FLAG_POSITION (0x80u)

/**
 ******************************************************************************
 ** \brief Maxmimum Frequency Correction Value
 ******************************************************************************/  
#define RTC_MAX_FREQ_CORR_VALUE (0x3FFu)

/**
 ******************************************************************************
 ** \brief Maxmimum Frequency Correction Cycle Setting Value
 ******************************************************************************/  
#define RTC_MAX_FREQ_CORR_CYCLE_SET_VALUE (0x3Fu)

/**
 ******************************************************************************
 ** \brief Maxmimum Timer Setting Value
 ******************************************************************************/  
#define RTC_MAX_TIMER_SET_VALUE (0x2A2FFu)

/**
 ******************************************************************************
 ** \brief Backup register area
 ******************************************************************************/
#define RTC_BKUP_REG_BASE       (FM4_PERIPH_BASE + 0x3B200UL)  /* Backup registers */

/******************************************************************************
 * Global type definitions
 ******************************************************************************/
typedef FM4_RTC_TypeDef stc_rtcn_t;

/** function pointer type to void/uint8_t... function */
typedef void  (*func_ptr_rtc_arglist_t)(uint8_t,  /* Second */
                                        uint8_t,  /* Minute */
                                        uint8_t,  /* Hour */
                                        uint8_t,  /* Day */
                                        uint8_t,  /* Day of Week */
                                        uint8_t,  /* Month */
                                        uint8_t); /* Year */

/**
 ******************************************************************************
 ** \brief Month name definitions (not used in driver - to be used by
 **        user applciation)
 ******************************************************************************/
typedef enum en_rtc_month
{
    Rtc_January   = 1,
    Rtc_Febuary   = 2,
    Rtc_March     = 3,
    Rtc_April     = 4,
    Rtc_May       = 5,
    Rtc_June      = 6,
    Rtc_July      = 7,
    Rtc_August    = 8,
    Rtc_September = 9,
    Rtc_October   = 10,
    Rtc_November  = 11,
    Rtc_December  = 12
} en_rtc_month_t;

/**
 ******************************************************************************
 ** \brief Day of week name definitions (not used in driver - to be used by
 **        user applciation)
 ******************************************************************************/
typedef enum en_rtc_day_of_week
{
    Rtc_Sunday    = 0,
    Rtc_Monday    = 1,
    Rtc_Tuesday   = 2,
    Rtc_Wednesday = 3,
    Rtc_Thursday  = 4,
    Rtc_Friday    = 5,
    Rtc_Saturday  = 6
} en_rtc_day_of_week_t;

/**
 ******************************************************************************
 ** \brief Rtc Timer Status
 ** 
 ** Read-out Status of RTC Timer Status (WTCR21.TMRUN Register Bit)
 ******************************************************************************/
typedef enum en_rtc_timer_status
{
    RtcTimerNoOperation     = 0,    ///< Timer Counter is not operating
    RtcTimerInOperation     = 1,    ///< Timer Counter is operationg
    RtcTimerInvalidInstance = 2     ///< Error: RTC instance pointer == NULL
} en_rtc_timer_status_t ;

/**
 ******************************************************************************
 ** \brief RTC Transmission Status
 ** 
 ** Read-out Status of RTC Transmission Status (WTCR10.TRANS Register Bit)
 ******************************************************************************/
typedef enum en_rtc_trans_status
{
    RtcTransNoOperation     = 0,    ///< Transmission is not operating
    RtcTransInOperation     = 1,    ///< Transmission is operationg
    RtcTransInvalidInstance = 2     ///< Error: RTC instance pointer == NULL
} en_rtc_trans_status_t ;

/**
 ******************************************************************************
 ** \brief Rtc Divider Ratio Setting
 ** 
 ** Divider Ration Settings for WTC. The enumarted values do not correspond to
 ** the bit patterns of WTDIV!
 ******************************************************************************/
typedef enum en_rtc_divider_ratio
{
    RtcDivRatio1     = 0x00,    ///< RIN clock is not divided
    RtcDivRatio2     = 0x01,    ///< RIN clock is divided by 2
    RtcDivRatio4     = 0x02,    ///< RIN clock is divided by 4
    RtcDivRatio8     = 0x03,    ///< RIN clock is divided by 8
    RtcDivRatio16    = 0x04,    ///< RIN clock is divided by 16
    RtcDivRatio32    = 0x05,    ///< RIN clock is divided by 32
    RtcDivRatio64    = 0x06,    ///< RIN clock is divided by 64
    RtcDivRatio128   = 0x07,    ///< RIN clock is divided by 128
    RtcDivRatio256   = 0x08,    ///< RIN clock is divided by 256
    RtcDivRatio512   = 0x09,    ///< RIN clock is divided by 512
    RtcDivRatio1024  = 0x0A,    ///< RIN clock is divided by 1024
    RtcDivRatio2048  = 0x0B,    ///< RIN clock is divided by 2048
    RtcDivRatio4096  = 0x0C,    ///< RIN clock is divided by 4096
    RtcDivRatio8192  = 0x0D,    ///< RIN clock is divided by 8192
    RtcDivRatio16384 = 0x0E,    ///< RIN clock is divided by 16384
    RtcDivRatio32768 = 0x0F     ///< RIN clock is divided by 32768
} en_rtc_divider_ratio_t ;

/**
 ******************************************************************************
 ** \brief Rtc CO Division
 ******************************************************************************/
typedef enum en_rtc_co_div
{
    RtcCoDiv1 = 0,  ///< CO signal of RTC count part is output
    RtcCoDiv2 = 1   ///< CO signal of RTC count divided by 2 is output
} en_rtc_co_div_t ;

/**
 ******************************************************************************
 ** \brief Backup register access size
 ******************************************************************************/
typedef enum en_rtc_bkup_reg_size
{
    RtcBkupRegSizeByte     = 0, ///< byte size (8bit)
    RtcBkupRegSizeHalfWord = 1, ///< half word size (16bit)
    RtcBkupRegSizeWord     = 2  ///< word size (32bit)
} en_rtc_bkup_reg_size_t ;

/**
 ******************************************************************************
 ** \brief Backup register area
 ******************************************************************************/
typedef enum en_rtc_bkup_reg_area
{
    RtcBkupRegArea00 = 0x00,
    RtcBkupRegArea01,
    RtcBkupRegArea02,
    RtcBkupRegArea03,
    RtcBkupRegArea04,
    RtcBkupRegArea05,
    RtcBkupRegArea06,
    RtcBkupRegArea07,
    RtcBkupRegArea08,
    RtcBkupRegArea09,
    RtcBkupRegArea0A,
    RtcBkupRegArea0B,
    RtcBkupRegArea0C,
    RtcBkupRegArea0D,
    RtcBkupRegArea0E,
    RtcBkupRegArea0F,
    RtcBkupRegArea10,
    RtcBkupRegArea11,
    RtcBkupRegArea12,
    RtcBkupRegArea13,
    RtcBkupRegArea14,
    RtcBkupRegArea15,
    RtcBkupRegArea16,
    RtcBkupRegArea17,
    RtcBkupRegArea18,
    RtcBkupRegArea19,
    RtcBkupRegArea1A,
    RtcBkupRegArea1B,
    RtcBkupRegArea1C,
    RtcBkupRegArea1D,
    RtcBkupRegArea1E,
    RtcBkupRegArea1F,
#if (PDL_DEVICE_TYPE != PDL_TYPE0)
    RtcBkupRegArea20,
    RtcBkupRegArea21,
    RtcBkupRegArea22,
    RtcBkupRegArea23,
    RtcBkupRegArea24,
    RtcBkupRegArea25,
    RtcBkupRegArea26,
    RtcBkupRegArea27,
    RtcBkupRegArea28,
    RtcBkupRegArea29,
    RtcBkupRegArea2A,
    RtcBkupRegArea2B,
    RtcBkupRegArea2C,
    RtcBkupRegArea2D,
    RtcBkupRegArea2E,
    RtcBkupRegArea2F,
    RtcBkupRegArea30,
    RtcBkupRegArea31,
    RtcBkupRegArea32,
    RtcBkupRegArea33,
    RtcBkupRegArea34,
    RtcBkupRegArea35,
    RtcBkupRegArea36,
    RtcBkupRegArea37,
    RtcBkupRegArea38,
    RtcBkupRegArea39,
    RtcBkupRegArea3A,
    RtcBkupRegArea3B,
    RtcBkupRegArea3C,
    RtcBkupRegArea3D,
    RtcBkupRegArea3E,
    RtcBkupRegArea3F,
    RtcBkupRegArea40,
    RtcBkupRegArea41,
    RtcBkupRegArea42,
    RtcBkupRegArea43,
    RtcBkupRegArea44,
    RtcBkupRegArea45,
    RtcBkupRegArea46,
    RtcBkupRegArea47,
    RtcBkupRegArea48,
    RtcBkupRegArea49,
    RtcBkupRegArea4A,
    RtcBkupRegArea4B,
    RtcBkupRegArea4C,
    RtcBkupRegArea4D,
    RtcBkupRegArea4E,
    RtcBkupRegArea4F,
    RtcBkupRegArea50,
    RtcBkupRegArea51,
    RtcBkupRegArea52,
    RtcBkupRegArea53,
    RtcBkupRegArea54,
    RtcBkupRegArea55,
    RtcBkupRegArea56,
    RtcBkupRegArea57,
    RtcBkupRegArea58,
    RtcBkupRegArea59,
    RtcBkupRegArea5A,
    RtcBkupRegArea5B,
    RtcBkupRegArea5C,
    RtcBkupRegArea5D,
    RtcBkupRegArea5E,
    RtcBkupRegArea5F,
    RtcBkupRegArea60,
    RtcBkupRegArea61,
    RtcBkupRegArea62,
    RtcBkupRegArea63,
    RtcBkupRegArea64,
    RtcBkupRegArea65,
    RtcBkupRegArea66,
    RtcBkupRegArea67,
    RtcBkupRegArea68,
    RtcBkupRegArea69,
    RtcBkupRegArea6A,
    RtcBkupRegArea6B,
    RtcBkupRegArea6C,
    RtcBkupRegArea6D,
    RtcBkupRegArea6E,
    RtcBkupRegArea6F,
    RtcBkupRegArea70,
    RtcBkupRegArea71,
    RtcBkupRegArea72,
    RtcBkupRegArea73,
    RtcBkupRegArea74,
    RtcBkupRegArea75,
    RtcBkupRegArea76,
    RtcBkupRegArea77,
    RtcBkupRegArea78,
    RtcBkupRegArea79,
    RtcBkupRegArea7A,
    RtcBkupRegArea7B,
    RtcBkupRegArea7C,
    RtcBkupRegArea7D,
    RtcBkupRegArea7E,
    RtcBkupRegArea7F,
#endif
    RtcBkupRegAreaMax
} en_rtc_bkup_reg_area_t;

/**
 ******************************************************************************
 ** \brief Interrupt configuration structure
 ** 
 ** The RTC interrupt configuration structure
 ******************************************************************************/
typedef struct stc_rtc_interrupts
{
    uint8_t  HalfSecondIrqEn       : 1; ///< 1: 0.5-Second interrupt enabled
                                        ///< 0: 0.5-Second interrupt disabled
    uint8_t  OneSecondIrqEn        : 1; ///< 1: 1-Second interrupt enabled
                                        ///< 0: 1-Second interrupt disabled
    uint8_t  OneMinuteIrqEn        : 1; ///< 1: 1-Minute interrupt enabled
                                        ///< 0: 1-Minute interrupt disabled
    uint8_t  OneHourIrqEn          : 1; ///< 1: 1-Hour interrupt enabled
                                        ///< 0: 1-Hour interrupt disabled
    uint8_t  TimerIrqEn            : 1; ///< 1: Timer interrupt enabled
                                        ///< 0: Timer interrupt disabled  
    uint8_t  AlarmIrqEn            : 1; ///< 1: Alarm interrupt enabled
                                        ///< 0: Alarm interrupt disabled
    uint8_t  TimeRewriteErrorIrqEn : 1; ///< 1: Time Rewrite Error interrupt enabled
                                        ///< 0: Time Rewrite Error interrupt disabled
    uint8_t  ReadCompletionIrqEn   : 1; ///< 1: RTC Read Completion interrupt enabled
                                        ///< 0: RTC Read Completion interrupt disabled
} stc_rtc_interrupts_t;

/**
 ******************************************************************************
 ** \brief Alarm Register En- and Disable
 ** 
 ** The RTC Alarm register configuration structure
 ******************************************************************************/
typedef struct stc_rtc_alarm_enable
{
    uint8_t AlarmYearEnable        : 1; ///< 1: RTC Alarm Year Register enabled
                                        ///< 0: RTC Alarm Year Register disabled
    uint8_t AlarmMonthEnable       : 1; ///< 1: RTC Alarm Month Register enabled
                                        ///< 0: RTC Alarm Month Register disabled
    uint8_t AlarmDayEnable         : 1; ///< 1: RTC Alarm Day Register enabled
                                        ///< 0: RTC Alarm Day Register disabled
    uint8_t AlarmHourEnable        : 1; ///< 1: RTC Alarm Hour Register enabled
                                        ///< 0: RTC Alarm Hour Register disabled
    uint8_t AlarmMinuteEnable      : 1; ///< 1: RTC Alarm Minute Register enabled
                                        ///< 0: RTC Alarm Minute Register disabled
    uint8_t Reserved               : 3;
} stc_rtc_alarm_enable_t;

/**
 ******************************************************************************
 ** \brief Real Time Clock Date and Time structure
 ******************************************************************************/
typedef struct stc_rtc_time
{
    uint8_t  u8Second;      ///< Second (Format 0...59)
    uint8_t  u8Minute;      ///< Minutes (Format 0...59)
    uint8_t  u8Hour;        ///< Hour (Format 0...23)
    uint8_t  u8Day;	        ///< Day (Format 1...31)
    uint8_t  u8DayOfWeek;   ///< Day of the week (Format 0...6)
    uint8_t  u8Month;       ///< Month (Foramt 1...12)
    uint8_t  u8Year;        ///< Year (Format 1...99) + 2000
} stc_rtc_time_t;

/**
 ******************************************************************************
 ** \brief Real Time Clock Alarm structure
 ******************************************************************************/
typedef struct stc_rtc_alarm
{
    uint8_t  u8AlarmMinute; ///< Alarm minutes (Format 1...59)
    uint8_t  u8AlarmHour;   ///< Alarm hour (Format 0...23)  
    uint8_t  u8AlarmDay;    ///< Alarm day (Format 1...31) 
    uint8_t  u8AlarmMonth;  ///< Alarm month (Foramt 1...12)
    uint8_t  u8AlarmYear;   ///< Alarm year (Format 1...99) + 2000  
} stc_rtc_alarm_t;

/**
 ******************************************************************************
 ** \brief Real Time Clock configuration
 ** 
 ** The RTC configuration settings
 ******************************************************************************/
typedef struct stc_rtc_config
{
    boolean_t                bUseFreqCorr;          ///< TRUE:  Use Frequency correction value
                                                    ///< FALSE: Do not use Frequency correction value
    boolean_t                bUseDivider;           ///< TRUE:  Enable Divider for Divider Ratio
                                                    ///< FALSE: Disable Divider for Divider Ratio
    union
    {
        uint16_t             u16FreqCorrValue;      ///< Frequency correction value (10 bit)
        struct
        {
            uint8_t          u8FreqCorrValue0;      ///< Frequency correction value (Lower 8bit for WTCAL0)
            uint8_t          u8FreqCorrValue1;      ///< Frequency correction value (Upper 2bit for WTCAL1)
        };
    };
    uint8_t                  u8DividerRatio;        ///< Divider Ratio Setting, see #en_rtc_divider_ratio_t for details
    uint8_t                  u8FreqCorrCycle;       ///< Frequency Correction Cycle Value
    uint8_t                  u8CoSignalDiv;         ///< Divider of CO signal, see #en_rtc_co_div_t for details
    union
    {
        stc_rtc_interrupts_t stcRtcInterruptEnable; ///< See #stc_rtc_interrupts_t for details
        uint8_t              u8AllInterrupts;       ///< Only used by driver
    };
    stc_rtc_alarm_enable_t   stcAlarmRegisterEnable;///< see #stc_rtc_alarm_enable_t for details
    func_ptr_rtc_arglist_t   pfnReadCallback;       ///< Callback function pointer for read completion Interrupt
    func_ptr_t               pfnTimeWrtErrCallback; ///< Callback function pointer for Timer writing error Interrupt
    func_ptr_t               pfnAlarmCallback;      ///< Callback function pointer for Alarm Interrupt
    func_ptr_t               pfnTimerCallback;      ///< Callback function pointer for Timer Interrupt
    func_ptr_t               pfnHalfSecondCallback; ///< Callback function pointer for 0.5-Second Interrupt
    func_ptr_t               pfnOneSecondCallback;  ///< Callback function pointer for One Second Interrupt
    func_ptr_t               pfnOneMinuteCallback;  ///< Callback function pointer for One Minute Interrupt
    func_ptr_t               pfnOneHourCallback;    ///< Callback function pointer for One Hour Interrupt
} stc_rtc_config_t;

/**
 ******************************************************************************
 ** \brief Real Time Clock Timer configuration
 ** 
 ** The RTC Timer configuration settings
 ******************************************************************************/
typedef struct stc_rtc_timer_config
{
    union {
        uint32_t    u32TimerValue;          ///< 18-Bit value for RTC Timer
        struct {
            uint8_t u8TimerValue0;          ///< RTC Timer value bit7-0 for WTTR0
            uint8_t u8TimerValue1;          ///< RTC Timer value bit15-8 for WTTR1
            uint8_t u8TimerValue2;          ///< RTC Timer value bit17,16 for WTTR2
            uint8_t Reserved;
        };
    };
    boolean_t       bTimerIntervalEnable;   ///< TRUE: Timer is set in count interval mode
                                            ///< FALSE: Timer is set in count elapse mode ('one shot')
} stc_rtc_timer_config_t ;

/******************************************************************************/
/* Local type definitions ('typedef')                                         */
/******************************************************************************/
/// Datatype for holding internal data needed for RTC
typedef struct stc_rtc_intern_data
{
    func_ptr_rtc_arglist_t pfnReadCallback;         ///< Callback function pointer for read completion Interrupt
    func_ptr_t             pfnTimeWrtErrCallback;   ///< Callback function pointer Timer writing error Interrupt
    func_ptr_t             pfnAlarmCallback;        ///< Callback function pointer Alarm Interrupt
    func_ptr_t             pfnTimerCallback;        ///< Callback function pointer Timer Interrupt
    func_ptr_t             pfnHalfSecondCallback;   ///< Callback function pointer 0.5-Second Interrupt
    func_ptr_t             pfnOneSecondCallback;    ///< Callback function pointer One Second Interrupt
    func_ptr_t             pfnOneMinuteCallback;    ///< Callback function pointer One Minute Interrupt
    func_ptr_t             pfnOneHourCallback;      ///< Callback function pointer One Hour Interrupt

    func_ptr_t             pfnCallbackRtc0;         ///< Callback for interrupts of WC
    uint32_t               u32RawTime;              ///< Unit Time (seconds since 1970-01-01)
    struct tm*             pstcCalendar;            ///< Pointer to global calendar structure
} stc_rtc_intern_data_t ;

/******************************************************************************/
/* Global variable definitions ('extern')                                     */
/******************************************************************************/

/******************************************************************************/
/* Global function prototypes (definition in C source)                        */
/******************************************************************************/
extern void RtcIrqHandler(void);

extern en_result_t Rtc_Init(stc_rtc_config_t*       pstcConfig,
                            stc_rtc_time_t*         pstcTime,
                            stc_rtc_alarm_t*        pstcAlarm,
                            boolean_t               bTouchNvic,
                            boolean_t               bResetNoReInit
                            );

extern boolean_t Rtc_GetRunStatus(void);

extern en_result_t Rtc_SetDateTime(stc_rtc_time_t* pstcRtcTime);

extern en_result_t Rtc_SetAlarmDateTime(stc_rtc_alarm_t* pstcRtcAlarm);

extern en_result_t Rtc_EnableDisableInterrupts(stc_rtc_config_t* pstcConfig);

extern en_result_t Rtc_EnableDisableAlarmRegisters(stc_rtc_alarm_enable_t* pstcRtcAlarmEn);

extern en_result_t Rtc_ReadDateTimePolling(stc_rtc_time_t* pstcRtcTime);

extern en_result_t Rtc_RequestDateTime(void);

extern en_result_t Rtc_TimerSet(stc_rtc_timer_config_t* pstcConfig);

extern en_result_t Rtc_TimerStart(void);

extern en_result_t Rtc_TimerStop(void);

extern en_rtc_timer_status_t Rtc_TimerStatusRead(void);

extern en_rtc_trans_status_t Rtc_TransStatusRead(void);

extern en_result_t Rtc_DeInit(void);

extern time_t Rtc_GetRawTime(stc_rtc_time_t* pstcRtcTime);

extern en_result_t Rtc_SetDayOfWeek(stc_rtc_time_t* pstcRtcTime);

extern en_result_t Rtc_SetTime(stc_rtc_time_t* pstcRtcTime,
                               time_t          tRawTime
                              );

/* Access to back up register */
extern en_result_t Rtc_WriteBkupReg8(uint8_t u8Data,
                                     uint8_t u8Area
                                    );
extern en_result_t Rtc_WriteBkupReg16(uint16_t u16Data,
                                      uint8_t  u8Area
                                     );
extern en_result_t Rtc_WriteBkupReg32(uint32_t u32Data,
                                      uint8_t  u8Area
                                     );

extern uint8_t  Rtc_ReadBkupReg8 (uint8_t u8Area);
extern uint16_t Rtc_ReadBkupReg16(uint8_t u8Area);
extern uint32_t Rtc_ReadBkupReg32(uint8_t u8Area);

//@} // RtcGroup

#ifdef __cplusplus
}
#endif

#endif /* #if (defined(PDL_PERIPHERAL_RTC_ACTIVE)) */

#endif /* __RTC_H__ */
/******************************************************************************/
/* EOF (not truncated)                                                        */
/******************************************************************************/
