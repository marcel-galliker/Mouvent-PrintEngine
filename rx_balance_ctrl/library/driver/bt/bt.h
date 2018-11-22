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
/** \file bt.h
 **
 ** Headerfile for BT functions
 **  
 ** History:
 **   - 2013-05-10  0.1  Edison Zhang  First version.
 **
 *****************************************************************************/
#ifndef __BT_H__
#define __BT_H__

/******************************************************************************/
/* Include files                                                              */
/******************************************************************************/
#include "pdl.h"

#if (defined(PDL_PERIPHERAL_BT_ACTIVE))

/**
 ******************************************************************************
 ** \defgroup BtGroup Base Timer (BT)
 **
 ** Provided BT module functions:
 ** 
 ** - Bt_ConfigIOMode()
 ** - Bt_SelTimerMode() 
 ** - Bt_Pwm_Init()
 ** - Bt_Pwm_EnableCount()
 ** - Bt_Pwm_DisableCount()
 ** - Bt_Pwm_EnableSwTrig()
 ** - Bt_Pwm_EnableInt()
 ** - Bt_Pwm_DisableInt()
 ** - Bt_Pwm_GetIntFlag()
 ** - Bt_Pwm_ClrIntFlag()
 ** - Bt_Pwm_WriteCycleVal()
 ** - Bt_Pwm_WriteDutyVal()
 ** - Bt_Pwm_ReadCurCnt()
 ** - Bt_Ppg_Init()
 ** - Bt_Ppg_EnableCount()
 ** - Bt_Ppg_DisableCount()
 ** - Bt_Ppg_EnableSwTrig()
 ** - Bt_Ppg_EnableInt()
 ** - Bt_Ppg_DisableInt()
 ** - Bt_Ppg_GetIntFlag()
 ** - Bt_Ppg_ClrIntFlag()
 ** - Bt_Ppg_WriteLowWidthVal()
 ** - Bt_Ppg_WriteHighWidthVal()
 ** - Bt_Ppg_ReadCurCnt()
 ** - Bt_Rt_Init()
 ** - Bt_Rt_EnableCount()
 ** - Bt_Rt_DisableCount()
 ** - Bt_Rt_EnableSwTrig()
 ** - Bt_Rt_EnableInt()
 ** - Bt_Rt_DisableInt()
 ** - Bt_Rt_GetIntFlag()
 ** - Bt_Rt_ClrIntFlag()
 ** - Bt_Rt_WriteCycleVal()
 ** - Bt_Rt_ReadCurCnt()
 ** - Bt_Pwc_Init()
 ** - Bt_Pwc_EnableCount()
 ** - Bt_Pwc_DisableCount()
 ** - Bt_Pwc_EnableInt()
 ** - Bt_Pwc_DisableInt()
 ** - Bt_Pwc_GetIntFlag()
 ** - Bt_Pwc_ClrIntFlag()
 ** - Bt_Pwc_GetErrorFlag()
 ** - Bt_Pwc_Get16BitMeasureData()
 ** - Bt_Pwc_Get32BitMeasureData()
 **
 ** \brief How to use PWM timer function of BT module
 **
 ** Bt_ConfigIOMode() must be used to configure BT I/O mode to I/O mode 0 first.
 **
 ** Bt_SelTimerMode() must be used to configure BT mode to PWM function then.
 **
 ** Bt_Pwm_Init() is used to initialize PWM timer source clock, output polarity,
 ** operation mode and so on. Following operation mode can set:
 ** - Continuous mode
 ** - One-shot mode
 **
 ** A PWM interrupt can be enabled by the function Bt_Pwm_EnableInt().
 ** This function can set callback function for each channel too.
 **
 ** With Bt_Pwm_WriteCycleVal() the PWM cycle counter is set to the value
 ** given in the parameter Bt_Pwm_WriteCycleVal#u16Cycle.
 **
 ** With Bt_Pwm_WriteDutyVal() the PWM duty counter is set to the value
 ** given in the parameter Bt_Pwm_WriteDutyVal#u16Duty.
 **
 ** Notes that PWM can be only set to 16-bit mode, so above two parameters 
 ** should be 16-bit. 
 **
 ** Bt_Pwm_EnableCount() is used to enabe PWM counter.
 **
 ** After above setting is done, calling Bt_Pwm_EnableSwTrig() will start
 ** PWM timer.
 **
 ** With Bt_Pwm_ReadCurCnt() the current PWM count can be read when PWM timer
 ** is running.
 **
 ** With interrupt mode, when the interrupt occurs, the interrupt flag will
 ** be cleared and run into user interrupt callback function.
 **
 ** With polling mode, user can use Bt_Pwm_GetIntFlag() to check if the 
 ** interrupt occurs, and clear the interrupt flag by Bt_Pwm_ClrIntFlag().
 ** 
 ** When stopping the PWM timer, use Bt_Pwm_DisableCount() to disable PWM
 ** counter and Bt_Pwm_DisableInt() to disable PWM interrupt.
 **
 ** \brief How to use PPG timer function of BT module
 **
 ** Bt_ConfigIOMode() must be used to configure BT I/O mode to I/O mode 0 first.
 **
 ** Bt_SelTimerMode() must be used to configure BT mode to PPG function then.
 **
 ** Bt_Ppg_Init() is used to initialize PPG timer source clock, output polarity,
 ** operation mode and so on. Following operation mode can set:
 ** - Continuous mode
 ** - One-shot mode
 **
 ** A PPG interrupt can be enabled by the function Bt_Ppg_EnableInt().
 ** This function can set callback function for each channel too.
 **
 ** With Bt_Ppg_WriteLowWidthVal() the PPG low width is set to the value
 ** given in the parameter Bt_Ppg_WriteLowWidthVal#u16Cycle.
 **
 ** With Bt_Ppg_WriteHighWidthVal() the PPG high width is set to the value
 ** given in the parameter Bt_Ppg_WriteHighWidthVal#u16Cycle.
 **
 ** Notes that PPG can be only set to 16-bit mode, so above two parameters 
 ** should be 16-bit. 
 **
 ** Bt_Ppg_EnableCount() is used to enabe PPG counter.
 **
 ** After above setting is done, calling Bt_Ppg_EnableSwTrig() will start
 ** PPG timer.
 **
 ** With Bt_Ppg_ReadCurCnt() the current PPG count can be read when PPG timer
 ** is running.
 **
 ** With interrupt mode, when the interrupt occurs, the interrupt flag will
 ** be cleared and run into user interrupt callback function.
 **
 ** With polling mode, user can use Bt_Ppg_GetIntFlag() to check if the 
 ** interrupt occurs, and clear the interrupt flag by Bt_Ppg_ClrIntFlag().
 ** 
 ** When stopping the PPG timer, use Bt_Ppg_DisableCount() to disable PPG
 ** counter and Bt_Ppg_DisableInt() to disable PPG interrupt.
 **
 ** ** \brief How to use Reload Timer (RT) function of BT module
 **
 ** Bt_ConfigIOMode() must be used to configure BT I/O mode to I/O mode 0 first.
 **
 ** Bt_SelTimerMode() must be used to configure BT mode to Reload Timer 
 ** function then.
 **
 ** Bt_Rt_Init() is used to initialize RT source clock, output polarity,
 ** operation mode and so on. Following operation mode can set:
 ** - Reload mode
 ** - One-shot mode
 **
 ** A RT interrupt can be enabled by the function Bt_Rt_EnableInt().
 ** This function can set callback function for each channel too.
 **
 ** RT can be set to 16-bit mode or 32-bit mode. In 16-bit mode, 
 ** Bt_Rt_WriteCycleVal() set the RT counter cycle of according channel. 
 ** In the 32-bit mode, Bt_Rt_WriteCycleVal() with even channel register 
 ** pointer as paremater set the low 16-bit cycle value and 
 ** Bt_Rt_WriteCycleVal() with odd channel register pointer as paremater 
 ** set the high 16-bit cycle value.
 **
 ** Bt_Rt_EnableCount() is used to enabe RT counter.
 **
 ** After above setting is done, calling Bt_Rt_EnableSwTrig() will start
 ** Reload Timer.
 **
 ** With Bt_Rt_ReadCurCnt() the current RT count can be read when Reload Timer
 ** is running.
 **
 ** With interrupt mode, when the interrupt occurs, the interrupt flag will
 ** be cleared and run into user interrupt callback function.
 **
 ** With polling mode, user can use Bt_Rt_GetIntFlag() to check if the 
 ** interrupt occurs, and clear the interrupt flag by Rt_Ppg_ClrIntFlag().
 ** 
 ** When stopping the PPG timer, use Bt_Rt_DisableCount() to disable RT
 ** counter and Bt_Rt_DisableInt() to disable RT interrupt.
 **
 ** ** \brief How to use PWC timer function of BT module
 **
 ** Bt_ConfigIOMode() must be used to configure BT I/O mode to I/O mode 0 first.
 **
 ** Bt_SelTimerMode() must be used to configure BT mode to PWC function then.
 **
 ** Bt_Pwc_Init() is used to initialize PWC timer source clock, measurement mode
 ** and so on. 
 ** Following measurement mode can set:
 ** - High pulse width measurement
 ** - Cycle measurement with rising edges
 ** - Cycle measurement with falling edges 
 ** - Interval measurement between all edges
 ** - Low pulse width measurement 
 ** Following operation mode can be set:
 ** - Continuous mode
 ** - One-shot mode
 **
 ** A PWC interrupt can be enabled by the function Bt_Pwc_EnableInt().
 ** This function can set callback function for each channel too.
 **
 ** Bt_Pwc_EnableCount() is used to enabe PWC counter.
 **
 ** After above setting is done, when the valid edge (1st) is detected, the 
 ** measurement starts, and the valid edge (2nd) is detected, the measurement
 ** ends, the interrupt request flag is set at the same time.
 **
 ** PWC timer can be set to 16-bit mode and 32-bit mode. In the 16-bit mode,
 ** with Bt_Pwc_Get16BitMeasureData() the measured value can be read after
 ** measurement completes, in the 32-bit mode, with Bt_Pwc_Get32BitMeasureData()
 ** the measured value can be read.
 **
 ** With interrupt mode, when the interrupt occurs, the interrupt flag will
 ** be cleared and the code runs into user interrupt callback function.
 **
 ** With polling mode, user can use Bt_Pwc_GetIntFlag() to check if the 
 ** completion interrupt occurs, and clear the interrupt flag by 
 ** Bt_Pwc_ClrIntFlag(). Bt_Pwc_GetErrorFlag() is used to get the error flag 
 ** of PWC measurement.
 ** 
 ** When stopping the PWC timer, use Bt_Pwc_DisableCount() to disable PWC
 ** counter and Bt_Pwc_DisableInt() to disable PWC interrupt.
 **
 ******************************************************************************/

//@{

/******************************************************************************
 * Global definitions
 ******************************************************************************/   
#define stc_btn_t  FM4_BT_PWM_TypeDef

#define BT0       (*((volatile stc_btn_t *) FM4_BT0_PWM_BASE))
#define BT1       (*((volatile stc_btn_t *) FM4_BT1_PWM_BASE))
#define BT2       (*((volatile stc_btn_t *) FM4_BT2_PWM_BASE))
#define BT3       (*((volatile stc_btn_t *) FM4_BT3_PWM_BASE))
#define BT4       (*((volatile stc_btn_t *) FM4_BT4_PWM_BASE))
#define BT5       (*((volatile stc_btn_t *) FM4_BT5_PWM_BASE))
#define BT6       (*((volatile stc_btn_t *) FM4_BT6_PWM_BASE))
#define BT7       (*((volatile stc_btn_t *) FM4_BT7_PWM_BASE))
#define BT8       (*((volatile stc_btn_t *) FM4_BT8_PWM_BASE))
#define BT9       (*((volatile stc_btn_t *) FM4_BT9_PWM_BASE))
#define BT10      (*((volatile stc_btn_t *) FM4_BT10_PWM_BASE))
#define BT11      (*((volatile stc_btn_t *) FM4_BT11_PWM_BASE))
#define BT12      (*((volatile stc_btn_t *) FM4_BT12_PWM_BASE))
#define BT13      (*((volatile stc_btn_t *) FM4_BT13_PWM_BASE))
#define BT14      (*((volatile stc_btn_t *) FM4_BT14_PWM_BASE))
#define BT15      (*((volatile stc_btn_t *) FM4_BT15_PWM_BASE))
   
/******************************************************************************
 * Global type definitions
 ******************************************************************************/
/**
 ******************************************************************************
 ** \brief Enumeration to define an index for each BT instance
 ******************************************************************************/   
typedef enum en_bt_instance_index
{
    BtInstanceIndexBt0 = 0,  ///< Instance index of BT0
    BtInstanceIndexBt1,      ///< Instance index of BT1
    BtInstanceIndexBt2,      ///< Instance index of BT2 
    BtInstanceIndexBt3,      ///< Instance index of BT3
    BtInstanceIndexBt4,      ///< Instance index of BT4
    BtInstanceIndexBt5,      ///< Instance index of BT5
    BtInstanceIndexBt6,      ///< Instance index of BT6
    BtInstanceIndexBt7,      ///< Instance index of BT7
    BtInstanceIndexBt8,      ///< Instance index of BT8
    BtInstanceIndexBt9,      ///< Instance index of BT9
    BtInstanceIndexBt10,     ///< Instance index of BT10
    BtInstanceIndexBt11,     ///< Instance index of BT11
    BtInstanceIndexBt12,     ///< Instance index of BT12
    BtInstanceIndexBt13,     ///< Instance index of BT13
    BtInstanceIndexBt14,     ///< Instance index of BT14
    BtInstanceIndexBt15,     ///< Instance index of BT15
    BtInstanceIndexMax       ///< Maximum instance index
} en_bt_instance_index_t;
  
/**
 ******************************************************************************
 ** \brief Enumeration to define BT IO mode
 ******************************************************************************/  
typedef enum en_bt_io_mode
{
    BtIoMode0 = 0,           ///< BT IO mode 0
    BtIoMode1,               ///< BT IO mode 1
    BtIoMode2,               ///< BT IO mode 2  
    BtIoMode3,               ///< BT IO mode 3
    BtIoMode4,               ///< BT IO mode 4
    BtIoMode5,               ///< BT IO mode 5
    BtIoMode6,               ///< BT IO mode 6
    BtIoMode7,               ///< BT IO mode 7
    BtIoMode8,               ///< BT IO mode 8
    
}en_bt_io_mode_t;

/**
 ******************************************************************************
 ** \brief Enumeration to define timer mode of BT
 ******************************************************************************/ 
typedef enum en_bt_timer_mode
{
    BtResetMode = 0,        ///< Reset mode
    BtPwmMode,              ///< PWM timer mode
    BtPpgMode,              ///< PPG timer mode
    BtRtMode,               ///< Reload timer mode
    BtPwcMode,              ///< PWC timer mode
    
}en_bt_timer_mode_t;

/**
 ******************************************************************************
 ** \brief Enumeration to define clock prescaler of PWM timer
 ******************************************************************************/ 
typedef enum en_pwm_clock_pres
{
    PwmPresNone = 0,        ///< PWM prescaler clock: no division
    PwmPres1Div4,           ///< PWM prescaler clock: 1/4
    PwmPres1Div16,          ///< PWM prescaler clock: 1/16
    PwmPres1Div128,         ///< PWM prescaler clock: 1/128
    PwmPres1Div256,         ///< PWM prescaler clock: 1/256
    PwmPres1ExtClkRising,   ///< Use external clock and count at rising edge
    PwmPres1ExtClkFalling,  ///< Use external clock and count at falling edge
    PwmPres1ExtClkBoth,     ///< Use external clock and count at both edge
    PwmPres1Div512,         ///< PWM prescaler clock: 1/512 
    PwmPres1Div1024,        ///< PWM prescaler clock: 1/1024 
    PwmPres1Div2048,        ///< PWM prescaler clock: 1/2048 
    
}en_pwm_clock_pres_t;

/**
 ******************************************************************************
 ** \brief Enumeration to define the configuration of PWM restart option 
 ******************************************************************************/ 
typedef enum en_pwm_restart_enable
{
    PwmRestartDisable = 0,  ///< Disable PWM restart
    PwmRestartEnable,       ///< Enable PWM restart
    
}en_pwm_restart_enable_t;

/**
 ******************************************************************************
 ** \brief Enumeration to define the configuration of PWM output mask
 ******************************************************************************/ 
typedef enum en_pwm_output_mask
{
    PwmOutputNormal = 0,   ///< Output normal PWM wave
    PwmOutputMask,         ///< Mask PWM wave 
    
}en_pwm_output_mask_t;

/**
 ******************************************************************************
 ** \brief Enumeration to define external trigger of PWM 
 ******************************************************************************/ 
typedef enum en_pwm_ext_trig
{
    PwmExtTrigDisable = 0, ///< Disable external trigger
    PwmExtTrigRising,      ///< Enable external trigger with rising edge
    PwmExtTrigFalling,     ///< Enable external trigger with falling edge
    PwmExtTrigBoth,        ///< Enable external trigger with both edge
    
}en_pwm_ext_trig_t;

/**
 ******************************************************************************
 ** \brief Enumeration to define PWM initial polarity
 ******************************************************************************/ 
typedef enum en_pwm_output_polarity
{
    PwmPolarityLow = 0,   ///< Initial polarity: low
    PwmPolarityHigh,      ///< Initial polarity: high
    
}en_pwm_output_polarity_t;

/**
 ******************************************************************************
 ** \brief Enumeration to define PWM timer mode
 ******************************************************************************/
typedef enum en_pwm_mode
{
    PwmContinuous = 0,   ///< Continuous mode
    PwmOneshot,          ///< One-shot mode 
   
}en_pwm_mode_t;

/**
 ******************************************************************************
 ** \brief Structure of PWM configuration
 ******************************************************************************/
typedef struct stc_bt_pwm_config
{ 
    en_pwm_clock_pres_t         enPres;                ///< Clock prescaler
    en_pwm_restart_enable_t     enRestartEn;           ///< Restart enable setting
    en_pwm_output_mask_t        enOutputMask;          ///< PWM output mask setting 
    en_pwm_ext_trig_t           enExtTrig;             ///< PWM external trigger setting 
    en_pwm_output_polarity_t    enOutputPolarity;      ///< PWM output polarity setting 
    en_pwm_mode_t               enMode;                ///< Continuous mode or one-shot mode
    
}stc_bt_pwm_config_t;

/**
 ******************************************************************************
 ** \brief Enumeration to define PWM interrupt type
 ******************************************************************************/
typedef enum en_bt_pwm_int
{
    PwmTrigInt = 0,       ///< Trigger interrupt
    PwmDutyMatchInt,      ///< Duty match interrupt
    PwmUnderflowInt,      ///< Underflow interrupt
    
}en_bt_pwm_int_t;

/**
 ******************************************************************************
 ** \brief structure to select the PWM interrupt
 ******************************************************************************/
typedef struct stc_pwm_int_sel
{
    boolean_t bPwmTrigInt;        ///< Trigger interrupt selection
    boolean_t bPwmDutyMatchInt;   ///< Duty match interrupt selection
    boolean_t bPwmUnderflowInt;   ///< Underflow interrupt selection
    
}stc_pwm_int_sel_t;

/**
 ******************************************************************************
 ** \brief structure to set the PWM interrupt callback function
 ******************************************************************************/
typedef struct stc_pwm_int_cb
{
    func_ptr_t pfnPwmTrigIntCallback;      ///< Pointer to trigger interrupt callback function
    func_ptr_t pfnPwmDutyMatchIntCallback; ///< Pointer to duty match interrupt callback function
    func_ptr_t pfnPwmUnderflowIntCallback; ///< Pointer to underflow interrupt callback function
  
}stc_pwm_int_cb_t;

/**
 ******************************************************************************
 ** \brief Enumeration to define clock prescaler of PPG timer
 ******************************************************************************/ 
typedef enum en_ppg_clock_pres
{
    PpgPresNone = 0,               ///< PPG prescaler clock: no division
    PpgPres1Div4,                  ///< PPG prescaler clock: 1/4
    PpgPres1Div16,                 ///< PPG prescaler clock: 1/16 
    PpgPres1Div128,                ///< PPG prescaler clock: 1/128 
    PpgPres1Div256,                ///< PPG prescaler clock: 1/256 
    PpgPres1ExtClkRising,          ///< Use external clock and count at rising edge
    PpgPres1ExtClkFalling,         ///< Use external clock and count at falling edge
    PpgPres1ExtClkBoth,            ///< Use external clock and count at both edge
    PpgPres1Div512,                ///< PPG prescaler clock: 1/512 
    PpgPres1Div1024,               ///< PPG prescaler clock: 1/1024 
    PpgPres1Div2048,               ///< PPG prescaler clock: 1/2048 
    
}en_ppg_clock_pres_t;

/**
 ******************************************************************************
 ** \brief Enumeration to define the configuration of PPG restart option 
 ******************************************************************************/ 
typedef enum en_ppg_restart_enable
{
    PpgRestartDisable = 0,         ///< Disable PPG restart
    PpgRestartEnable,              ///< Enable PPG restart
    
}en_ppg_restart_enable_t;

/**
 ******************************************************************************
 ** \brief Enumeration to define the configuration of PPG output mask
 ******************************************************************************/ 
typedef enum en_ppg_output_mask
{
    PpgOutputNormal = 0,           ///< Output normal PPG wave
    PpgOutputMask,                 ///< Mask PPG wave
    
}en_ppg_output_mask_t;

/**
 ******************************************************************************
 ** \brief Enumeration to define external trigger of PPG 
 ******************************************************************************/ 
typedef enum en_ppg_ext_trig
{
    PpgExtTrigDisable = 0,         ///< Disable external trigger
    PpgExtTrigRising,              ///< Enable external trigger with rising edge 
    PpgExtTrigFalling,             ///< Enable external trigger with falling edge 
    PpgExtTrigBoth,                ///< Enable external trigger with both edge 
    
}en_ppg_ext_trig_t;

/**
 ******************************************************************************
 ** \brief Enumeration to define PWM initial polarity
 ******************************************************************************/ 
typedef enum en_ppg_output_polarity
{
    PpgPolarityLow = 0,           ///< Initial polarity: low
    PpgPolarityHigh,              ///< Initial polarity: high
    
}en_ppg_output_polarity_t;

/**
 ******************************************************************************
 ** \brief Enumeration to define PWM timer mode
 ******************************************************************************/
typedef enum en_ppg_mode
{
    PpgContinuous = 0,             ///< Continuous mode
    PpgOneshot,                    ///< One-shot mode   
    
}en_ppg_mode_t;

/**
 ******************************************************************************
 ** \brief Structure of PPG configuration
 ******************************************************************************/
typedef struct stc_bt_ppg_config
{ 
    en_ppg_clock_pres_t         enPres;             ///< Clock prescaler
    en_ppg_restart_enable_t     enRestartEn;        ///< Restart enable setting
    en_ppg_output_mask_t        enOutputMask;       ///< PPG output mask setting 
    en_ppg_ext_trig_t           enExtTrig;          ///< PPG external trigger setting  
    en_ppg_output_polarity_t    enOutputPolarity;   ///< PPG output polarity setting  
    en_ppg_mode_t               enMode;             ///< Continuous mode or one-shot mode
    
}stc_bt_ppg_config_t;

/**
 ******************************************************************************
 ** \brief Enumeration to define PPG interrupt type
 ******************************************************************************/
typedef enum en_bt_ppg_int
{
    PpgTrigInt = 0,            ///< Trigger interrupt
    PpgUnderflowInt,           ///< Underflow interrupt
    
}en_bt_ppg_int_t;

/**
 ******************************************************************************
 ** \brief structure to select the PPG interrupt
 ******************************************************************************/
typedef struct stc_ppg_int_sel
{
    boolean_t bPpgTrigInt;       ///< PPG trigger interrupt selection
    boolean_t bPpgUnderflowInt;  ///< PPG underflow interrupt selection
    
}stc_ppg_int_sel_t;

/**
 ******************************************************************************
 ** \brief structure to set the PPG interrupt callback function
 ******************************************************************************/
typedef struct stc_ppg_int_cb
{
    func_ptr_t pfnPpgTrigIntCallback;      ///< Pointer to PPG trigger interrupt callback function
    func_ptr_t pfnPpgUnderflowIntCallback; ///< Pointer to PPG underflow interrupt callback function
  
}stc_ppg_int_cb_t;

/**
 ******************************************************************************
 ** \brief Enumeration to define clock prescaler of Reload timer
 ******************************************************************************/ 
typedef enum en_rt_clock_pres
{
    RtPresNone = 0,             ///< Reload timer prescaler clock: no division
    RtPres1Div4,                ///< Reload timer prescaler clock: 1/4
    RtPres1Div16,               ///< Reload timer prescaler clock: 1/16
    RtPres1Div128,              ///< Reload timer prescaler clock: 1/128
    RtPres1Div256,              ///< Reload timer prescaler clock: 1/256 
    RtPres1ExtClkRising,        ///< Use external clock and count at rising edge
    RtPres1ExtClkFalling,       ///< Use external clock and count at falling edge
    RtPres1ExtClkBoth,          ///< Use external clock and count at both edge
    RtPres1Div512,              ///< Reload timer prescaler clock: 1/512 
    RtPres1Div1024,             ///< Reload timer prescaler clock: 1/1024
    RtPres1Div2048,             ///< Reload timer prescaler clock: 1/2048
    
}en_rt_clock_pres_t;

/**
 ******************************************************************************
 ** \brief Enumeration to define external trigger of Reload timer
 ******************************************************************************/ 
typedef enum en_rt_ext_trigger
{
    RtExtTiggerDisable = 0,     ///< Disable external trigger  
    RtExtTiggerRisingEdge,      ///< Enable external trigger with rising edge
    RtExtTiggerFallingEdge,     ///< Enable external trigger with falling edge
    RtExtTiggerBothEdge,        ///< Enable external trigger with both edge
    RtExtTiggerLowLevel,        ///< Enable external trigger with low level
    RtExtTiggerHighLevel,       ///< Enable external trigger with high level
    
}en_rt_ext_trigger_t;

/**
 ******************************************************************************
 ** \brief Enumeration to define Reload timer output initial polarity
 ******************************************************************************/
typedef enum en_rt_output_polarity
{
    RtPolarityLow = 0,        ///< Initial polarity: low
    RtPolarityHigh,           ///< Initial polarity: high
    
}en_rt_output_polarity_t;

/**
 ******************************************************************************
 ** \brief Enumeration to define Reload timer mode
 ******************************************************************************/
typedef enum en_rt_mode
{
    RtReload = 0,            ///< Reload mode
    RtOneshot,               ///< One-shot mode
    
}en_rt_mode_t;

/**
 ******************************************************************************
 ** \brief Enumeration to define Reload timer size
 ******************************************************************************/
typedef enum en_rt_timer_size
{
    RtSize16Bit = 0,         ///< 16-bit mode
    RtSize32Bit,             ///< 32-bit mode
    
}en_rt_timer_size_t;

/**
 ******************************************************************************
 ** \brief Structure of Reload timer configuration
 ******************************************************************************/
typedef struct stc_bt_rt_config
{ 
    en_rt_clock_pres_t          enPres;                ///< Clock prescaler
    en_rt_timer_size_t          enSize;                ///< 16-bit or 32-bit
    en_rt_ext_trigger_t         enExtTrig;             ///< External trigger setting
    en_rt_output_polarity_t     enOutputPolarity;      ///< Output polarity setting
    en_rt_mode_t                enMode;                ///< Reload mode or one-shot mode
    
}stc_bt_rt_config_t;

/**
 ******************************************************************************
 ** \brief Enumeration to define Reload timer interrupt type
 ******************************************************************************/
typedef enum en_bt_rt_int
{
    RtTrigInt = 0,           ///< Trigger interrupt
    RtUnderflowInt,          ///< Underflow interrupt 
    
}en_bt_rt_int_t;

/**
 ******************************************************************************
 ** \brief structure to select the Reload timer interrupt
 ******************************************************************************/
typedef struct stc_rt_int_sel
{
    boolean_t bRtTrigInt;        ///< Trigger interrupt selection
    boolean_t bRtUnderflowInt;   ///< Underflow interrupt selection
    
}stc_rt_int_sel_t;

/**
 ******************************************************************************
 ** \brief structure to set the Reload timer interrupt callback function
 ******************************************************************************/
typedef struct stc_rt_int_cb
{
    func_ptr_t pfnRtTrigIntCallback;      ///< Pointer to trigger interrupt callback function
    func_ptr_t pfnRtUnderflowIntCallback; ///< Pointer to underflow interrupt callback function
  
}stc_rt_int_cb_t;

/**
 ******************************************************************************
 ** \brief Enumeration to define clock prescaler of PWC timer
 ******************************************************************************/ 
typedef enum en_pwc_clock_pres
{
    PwcPresNone = 0,        ///< PWC timer prescaler clock: no division
    PwcPres1Div4,           ///< PWC timer prescaler clock: 1/4
    PwcPres1Div16,          ///< PWC timer prescaler clock: 1/16
    PwcPres1Div128,         ///< PWC timer prescaler clock: 1/128
    PwcPres1Div256,         ///< PWC timer prescaler clock: 1/256 
    PwcPres1Div512 = 8,     ///< PWC timer prescaler clock: 1/512
    PwcPres1Div1024,        ///< PWC timer prescaler clock: 1/1024
    PwcPres1Div2048,        ///< PWC timer prescaler clock: 1/2048
    
}en_pwc_clock_pres_t;

/**
 ******************************************************************************
 ** \brief Enumeration to define measure mode of PWC timer
 ******************************************************************************/ 
typedef enum en_pwc_measure_edge
{
    PwcMeasureRisingToFalling = 0,  ///< Measure between rising edge with falling edge
    PwcMeasureRisingToRising,       ///< Measure between rising edge with rising edge
    PwcMeasureFallingToFalling,     ///< Measure between falling edge with falling edge
    PwcMeasureEitherToEither,       ///< Measure between either edge with either edge
    PwcMeasureFallingToRising,      ///< Measure between falling edge with falling edge
    
}en_pwc_measure_edge_t;

/**
 ******************************************************************************
 ** \brief Enumeration to define PWC timer mode
 ******************************************************************************/
typedef enum en_pwc_mode
{
    PwcContinuous = 0,          ///< Continuous mode
    PwcOneshot,                 ///< One-shot mode
    
}en_pwc_mode_t;

/**
 ******************************************************************************
 ** \brief Enumeration to define Reload timer size
 ******************************************************************************/
typedef enum en_pwc_timer_size
{
    PwcSize16Bit = 0,           ///< 16-bit mode
    PwcSize32Bit,               ///< 32-bit mode
    
}en_pwc_timer_size_t;

/**
 ******************************************************************************
 ** \brief Structure of PWC timer configuration
 ******************************************************************************/
typedef struct stc_bt_pwc_config
{ 
    en_pwc_clock_pres_t          enPres;         ///< Clock prescaler 
    en_pwc_timer_size_t          enSize;         ///< 16-bit or 32-bit  
    en_pwc_measure_edge_t        enMeasureEdge;  ///< Measure mode setting
    en_pwc_mode_t                enMode;         ///< Continuous mode or one-shot mode
    
}stc_bt_pwc_config_t;

/**
 ******************************************************************************
 ** \brief Enumeration to define PWC timer interrupt type
 ******************************************************************************/
typedef enum en_bt_pwc_int
{
    PwcMeasureCompleteInt = 0,       ///< Measure complete interrupt 
    PwcMeasureOverflowInt,           ///< Measure overflow interrupt  
    
}en_bt_pwc_int_t;

/**
 ******************************************************************************
 ** \brief structure to select the PWC timer interrupt
 ******************************************************************************/
typedef struct stc_pwc_int_sel
{
    boolean_t bPwcMeasCmpInt;        ///< Trigger interrupt selection
    boolean_t bPwcMeasOverflowInt;   ///< Underflow interrupt selection
    
}stc_pwc_int_sel_t;

/**
 ******************************************************************************
 ** \brief structure to set the PWC timer interrupt callback function
 ******************************************************************************/
typedef struct stc_pwc_int_cb
{
    func_ptr_t pfnPwcMeasCmpCallback;       ///< Poiter to PWC measure completion callback function
    func_ptr_t pfnPwcMeasOverflowCallback;  ///< Poiter to PWC measure overflow callback function
  
}stc_pwc_int_cb_t;

/**
 ******************************************************************************
 ** \brief BT internal data
 ******************************************************************************/
typedef struct stc_bt_intern_data
{
    func_ptr_t  pfnBit0IntCallback;  ///< Pointer to callback function the interrupt reflected by bit0 of STC 
    func_ptr_t  pfnBit1IntCallback;  ///< Pointer to callback function the interrupt reflected by bit1 of STC
    func_ptr_t  pfnBit2IntCallback;  ///< Pointer to callback function the interrupt reflected by bit2 of STC
} stc_bt_intern_data_t ;

/**
 ******************************************************************************
 ** \brief BT instance data
 ******************************************************************************/
typedef struct stc_bt_instance_data
{
    volatile stc_btn_t* pstcInstance;   ///< pointer to registers of an instance
    stc_bt_intern_data_t stcInternData; ///< module internal data of instance
} stc_bt_instance_data_t;

/******************************************************************************/
/* Global variable declarations ('extern', definition in C source)            */
/******************************************************************************/
/// Look-up table for all enabled BT instances and their internal data
extern stc_bt_instance_data_t m_astcBtInstanceDataLut[];

/******************************************************************************/
/* Global function prototypes (definition in C source)                        */
/******************************************************************************/
/* C binding of definitions if building with C++ compiler */
#ifdef __cplusplus
extern "C"
{
#endif
/* 1. Configure BT IO mode  */  
en_result_t Bt_ConfigIOMode(volatile stc_btn_t* pstcBt, en_bt_io_mode_t enIoMode);

/* 2. Select BT timer Mode */
en_result_t Bt_SelTimerMode(volatile stc_btn_t* pstcBt, en_bt_timer_mode_t enTimerMode);

/* 3. Timer configuration */
/* 3.1 PWM timer */
/* Init */
en_result_t Bt_Pwm_Init(volatile stc_btn_t* pstcBt, stc_bt_pwm_config_t* pstcPwmConfig);
/* Func/Int Enable/Disable */
en_result_t Bt_Pwm_EnableCount(volatile stc_btn_t* pstcBt);
en_result_t Bt_Pwm_DisableCount(volatile stc_btn_t* pstcBt);
en_result_t Bt_Pwm_EnableSwTrig(volatile stc_btn_t* pstcBt);
#if (PDL_INTERRUPT_ENABLE_BT0 == PDL_ON) ||  (PDL_INTERRUPT_ENABLE_BT1 == PDL_ON) || \
    (PDL_INTERRUPT_ENABLE_BT2 == PDL_ON) ||  (PDL_INTERRUPT_ENABLE_BT3 == PDL_ON) || \
    (PDL_INTERRUPT_ENABLE_BT4 == PDL_ON) ||  (PDL_INTERRUPT_ENABLE_BT5 == PDL_ON) || \
    (PDL_INTERRUPT_ENABLE_BT6 == PDL_ON) ||  (PDL_INTERRUPT_ENABLE_BT7 == PDL_ON) || \
    (PDL_INTERRUPT_ENABLE_BT8 == PDL_ON) ||  (PDL_INTERRUPT_ENABLE_BT9 == PDL_ON) || \
    (PDL_INTERRUPT_ENABLE_BT10 == PDL_ON) ||  (PDL_INTERRUPT_ENABLE_BT11 == PDL_ON) || \
    (PDL_INTERRUPT_ENABLE_BT12 == PDL_ON) ||  (PDL_INTERRUPT_ENABLE_BT13 == PDL_ON) || \
    (PDL_INTERRUPT_ENABLE_BT14 == PDL_ON) ||  (PDL_INTERRUPT_ENABLE_BT15 == PDL_ON) 
en_result_t Bt_Pwm_EnableInt(volatile stc_btn_t* pstcBt, stc_pwm_int_sel_t* pstcIntSel, 
                             stc_pwm_int_cb_t*  pstcIntCallback);
en_result_t Bt_Pwm_DisableInt(volatile stc_btn_t* pstcBt, stc_pwm_int_sel_t* pstcIntSel);
#endif
/* Interrupt flag read/clear */
en_int_flag_t Bt_Pwm_GetIntFlag(volatile stc_btn_t* pstcBt, en_bt_pwm_int_t enIntType);
en_result_t Bt_Pwm_ClrIntFlag(volatile stc_btn_t* pstcBt, en_bt_pwm_int_t enIntType);
/* Write/read count */
en_result_t Bt_Pwm_WriteCycleVal(volatile stc_btn_t* pstcBt, uint16_t u16Cycle);
en_result_t Bt_Pwm_WriteDutyVal(volatile stc_btn_t* pstcBt, uint16_t u16Duty);
uint16_t Bt_Pwm_ReadCurCnt(volatile stc_btn_t* pstcBt);

/* 3.2 PPG timer */
/* Init */
en_result_t Bt_Ppg_Init(volatile stc_btn_t* pstcBt, stc_bt_ppg_config_t* pstcPpgConfig);
/* Func/Int Enable/Disable */
en_result_t Bt_Ppg_EnableCount(volatile stc_btn_t* pstcBt);
en_result_t Bt_Ppg_DisableCount(volatile stc_btn_t* pstcBt);
en_result_t Bt_Ppg_EnableSwTrig(volatile stc_btn_t* pstcBt);
#if (PDL_INTERRUPT_ENABLE_BT0 == PDL_ON) ||  (PDL_INTERRUPT_ENABLE_BT1 == PDL_ON) || \
    (PDL_INTERRUPT_ENABLE_BT2 == PDL_ON) ||  (PDL_INTERRUPT_ENABLE_BT3 == PDL_ON) || \
    (PDL_INTERRUPT_ENABLE_BT4 == PDL_ON) ||  (PDL_INTERRUPT_ENABLE_BT5 == PDL_ON) || \
    (PDL_INTERRUPT_ENABLE_BT6 == PDL_ON) ||  (PDL_INTERRUPT_ENABLE_BT7 == PDL_ON) || \
    (PDL_INTERRUPT_ENABLE_BT8 == PDL_ON) ||  (PDL_INTERRUPT_ENABLE_BT9 == PDL_ON) || \
    (PDL_INTERRUPT_ENABLE_BT10 == PDL_ON) ||  (PDL_INTERRUPT_ENABLE_BT11 == PDL_ON) || \
    (PDL_INTERRUPT_ENABLE_BT12 == PDL_ON) ||  (PDL_INTERRUPT_ENABLE_BT13 == PDL_ON) || \
    (PDL_INTERRUPT_ENABLE_BT14 == PDL_ON) ||  (PDL_INTERRUPT_ENABLE_BT15 == PDL_ON)
en_result_t Bt_Ppg_EnableInt(volatile stc_btn_t* pstcBt, stc_ppg_int_sel_t* pstcIntSel, 
                             stc_ppg_int_cb_t*  pstcIntCallback);
en_result_t Bt_Ppg_DisableInt(volatile stc_btn_t* pstcBt, stc_ppg_int_sel_t* pstcIntSel);
#endif
/* Interrupt flag read/clear */
en_int_flag_t Bt_Ppg_GetIntFlag(volatile stc_btn_t* pstcBt, en_bt_ppg_int_t enIntType);
en_result_t Bt_Ppg_ClrIntFlag(volatile stc_btn_t* pstcBt, en_bt_ppg_int_t enIntType);
/* Write/read count */
en_result_t Bt_Ppg_WriteLowWidthVal(volatile stc_btn_t* pstcBt, uint16_t u16Val);
en_result_t Bt_Ppg_WriteHighWidthVal(volatile stc_btn_t* pstcBt, uint16_t u16Val);
uint16_t Bt_Ppg_ReadCurCnt(volatile stc_btn_t* pstcBt);

/* 3.3 RT timer */
/* Init */
en_result_t Bt_Rt_Init(volatile stc_btn_t* pstcBt, stc_bt_rt_config_t* pstcRtConfig);
/* Func/Int Enable/Disable */
en_result_t Bt_Rt_EnableCount(volatile stc_btn_t* pstcBt);
en_result_t Bt_Rt_DisableCount(volatile stc_btn_t* pstcBt);
en_result_t Bt_Rt_EnableSwTrig(volatile stc_btn_t* pstcBt);
#if (PDL_INTERRUPT_ENABLE_BT0 == PDL_ON) ||  (PDL_INTERRUPT_ENABLE_BT1 == PDL_ON) || \
    (PDL_INTERRUPT_ENABLE_BT2 == PDL_ON) ||  (PDL_INTERRUPT_ENABLE_BT3 == PDL_ON) || \
    (PDL_INTERRUPT_ENABLE_BT4 == PDL_ON) ||  (PDL_INTERRUPT_ENABLE_BT5 == PDL_ON) || \
    (PDL_INTERRUPT_ENABLE_BT6 == PDL_ON) ||  (PDL_INTERRUPT_ENABLE_BT7 == PDL_ON) || \
    (PDL_INTERRUPT_ENABLE_BT8 == PDL_ON) ||  (PDL_INTERRUPT_ENABLE_BT9 == PDL_ON) || \
    (PDL_INTERRUPT_ENABLE_BT10 == PDL_ON) ||  (PDL_INTERRUPT_ENABLE_BT11 == PDL_ON) || \
    (PDL_INTERRUPT_ENABLE_BT12 == PDL_ON) ||  (PDL_INTERRUPT_ENABLE_BT13 == PDL_ON) || \
    (PDL_INTERRUPT_ENABLE_BT14 == PDL_ON) ||  (PDL_INTERRUPT_ENABLE_BT15 == PDL_ON)
en_result_t Bt_Rt_EnableInt(volatile stc_btn_t* pstcBt, stc_rt_int_sel_t* pstcIntSel, 
                            stc_rt_int_cb_t*  pstcIntCallback);
en_result_t Bt_Rt_DisableInt(volatile stc_btn_t* pstcBt, stc_rt_int_sel_t* pstcIntSel);
#endif
/* Interrupt flag read/clear */
en_int_flag_t Bt_Rt_GetIntFlag(volatile stc_btn_t* pstcBt, en_bt_rt_int_t enIntType);
en_result_t Bt_Rt_ClrIntFlag(volatile stc_btn_t* pstcBt, en_bt_rt_int_t enIntType);
/* Write/read count */
en_result_t Bt_Rt_WriteCycleVal(volatile stc_btn_t* pstcBt, uint16_t u16Val);
uint16_t Bt_Rt_ReadCurCnt(volatile stc_btn_t* pstcBt);

/* 3.4 PWC timer */
/* Init */
en_result_t Bt_Pwc_Init(volatile stc_btn_t* pstcBt, stc_bt_pwc_config_t* pstcPwcConfig);
/* Func/Int Enable/Disable */
en_result_t Bt_Pwc_EnableCount(volatile stc_btn_t* pstcBt);
en_result_t Bt_Pwc_DisableCount(volatile stc_btn_t* pstcBt);
#if (PDL_INTERRUPT_ENABLE_BT0 == PDL_ON) ||  (PDL_INTERRUPT_ENABLE_BT1 == PDL_ON) || \
    (PDL_INTERRUPT_ENABLE_BT2 == PDL_ON) ||  (PDL_INTERRUPT_ENABLE_BT3 == PDL_ON) || \
    (PDL_INTERRUPT_ENABLE_BT4 == PDL_ON) ||  (PDL_INTERRUPT_ENABLE_BT5 == PDL_ON) || \
    (PDL_INTERRUPT_ENABLE_BT6 == PDL_ON) ||  (PDL_INTERRUPT_ENABLE_BT7 == PDL_ON) || \
    (PDL_INTERRUPT_ENABLE_BT8 == PDL_ON) ||  (PDL_INTERRUPT_ENABLE_BT9 == PDL_ON) || \
    (PDL_INTERRUPT_ENABLE_BT10 == PDL_ON) ||  (PDL_INTERRUPT_ENABLE_BT11 == PDL_ON) || \
    (PDL_INTERRUPT_ENABLE_BT12 == PDL_ON) ||  (PDL_INTERRUPT_ENABLE_BT13 == PDL_ON) || \
    (PDL_INTERRUPT_ENABLE_BT14 == PDL_ON) ||  (PDL_INTERRUPT_ENABLE_BT15 == PDL_ON)
en_result_t Bt_Pwc_EnableInt(volatile stc_btn_t* pstcBt, stc_pwc_int_sel_t* pstcIntSel, 
                             stc_pwc_int_cb_t*  pstcIntCallback);
en_result_t Bt_Pwc_DisableInt(volatile stc_btn_t* pstcBt, stc_pwc_int_sel_t* pstcIntSel);
#endif
/* Interrupt flag read/clear */
en_int_flag_t Bt_Pwc_GetIntFlag(volatile stc_btn_t* pstcBt, en_bt_pwc_int_t enIntType);
en_result_t Bt_Pwc_ClrIntFlag(volatile stc_btn_t* pstcBt, en_bt_pwc_int_t enIntType);
en_stat_flag_t Bt_Pwc_GetErrorFlag(volatile stc_btn_t* pstcBt);
/* Write/read count */
uint16_t Bt_Pwc_Get16BitMeasureData(volatile stc_btn_t* pstcBt);
uint32_t Bt_Pwc_Get32BitMeasureData(volatile stc_btn_t* pstcBt);

/* 4. IRQ handler */
void Bt_IrqHandler( volatile stc_btn_t* pstcBt,
                    stc_bt_intern_data_t* pstcBtInternData) ;
/* 5. Start/stop BT simultaneously  */
void Bt_SetSimultaneousStart(uint16_t u16Value);

//@} // BtGroup
#ifdef __cplusplus
}
#endif

#endif // #if (defined(PDL_PERIPHERAL_BT_ACTIVE))

#endif // #if  __BT_H__ 
/******************************************************************************/
/* EOF (not truncated)                                                        */
/******************************************************************************/
