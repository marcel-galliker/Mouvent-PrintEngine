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
/** \file mft_wfg.h
 **
 ** Headerfile for MFT's Waveform Generator functions
 **  
 ** History:
 **   - 2013-03-21  0.1  Edison  First version.
 **   - 2013-07-16  0.2  Edison  Add comments about how to use WFG with APIs
 **
 *****************************************************************************/

#ifndef __MFT_WFG_H__
#define __MFT_WFG_H__

#include "pdl.h"

#if (defined(PDL_PERIPHERAL_MFT_WFG_ACTIVE))

/* C binding of definitions if building with C++ compiler */
#ifdef __cplusplus
extern "C"
{
#endif
  
/**
 ******************************************************************************
 ** \defgroup WfgGroup Waveform Generator (WFG)
 **
 ** Provided functions of WFG module:
 ** 
 ** - Mft_Wfg_ConfigMode()
 ** - Mft_Wfg_ConfigCtrlBits() 
 ** - Mft_Wfg_InitTimerClock()
 ** - Mft_Wfg_EnableTimerInt() 
 ** - Mft_Wfg_DisableTimerInt()
 ** - Mft_Wfg_WriteTimerCountCycle()
 ** - Mft_Wfg_SetTimerCountCycle()  
 ** - Mft_Wfg_GetTimerIntFlag()
 ** - Mft_Wfg_ClrTimerIntFlag(); 
 ** - Mft_Wfg_SetTimerCycle()
 ** - Mft_Wfg_GetTimerCurCycle()
 ** - Mft_Wfg_ConfigNzcl()
 ** - Mft_Wfg_EnableDtifInt()
 ** - Mft_Wfg_DisableDtifInt() 
 ** - Mft_Wfg_SwTiggerDtif()
 ** - Mft_Wfg_GetDigitalFilterIntFlag() 
 ** - Mft_Wfg_ClrDigitalFilterIntFlag() 
 ** - Mft_Wfg_GetAnalogFilterIntFlag()
 ** - Mft_Wfg_ClrAnalogFilterIntFlag()
 ** - Mft_Wfg_IrqHandler() 
 **
 ** \brief the operation mode of WFG module 
 ** 
 ** The WFG can be configured to following mode:
 ** - Through mode
 ** - RT-PPG mode
 ** - Timer-PPG mode
 ** - RT-dead timer mode
 ** - PPG-dead timer mode 
 **
 ** \brief how to operate WFG with Through mode (one of usages).
 **   
 ** Before using WFG, the FRT used to connect with applying OCU must be 
 ** initialed first and OCU must be initialed then. 
 ** For how to configure FRT, see the description in the mft_frt.h
 ** For how to configure OCU, see the description in the mft_ocu.h
 **  
 ** Set the control bits with Mft_Wfg_ConfigCtrlBits() with a structure of 
 ** the type #stc_wfg_ctrl_bits 
 ** - enGtenBits = b'00
 ** - enPselBits = b'00
 ** - enPgenBits = b'00  
 ** - enDmodBit = b'0
 **
 ** Configure WFG to Through mode with Mft_Wfg_ConfigMode() 
 **  
 ** Enable OCU operation and RT signal will output to RTO pin directly.
 ** Changing the value of control bits will influence RTO output. 
 **
 ** \brief how to operate WFG with RT-PPG mode (one of usages). 
 **  
 ** Before using WFG, the FRT used to connect with applying OCU must be 
 ** initialized first and OCU must be initialed then, PPG should be initialized
 ** at the following.  
 ** For how to configure FRT, see the description in the mft_frt.h
 ** For how to configure OCU, see the description in the mft_ocu.h  
 ** For how to configure PPG, see the description in the ppg.h
 **
 ** Set the control bits with Mft_Wfg_ConfigCtrlBits() with a structure of 
 ** the type #stc_wfg_ctrl_bits 
 ** - enGtenBits = b'00 
 ** - enPselBits = b'00 (input PPG ch.0 to WFG0)
 ** - enPgenBits = b'11  
 ** - enDmodBit = b'0  
 **
 ** Configure WFG to RT-PPG mode with Mft_Wfg_ConfigMode()   
 **  
 ** Start PPG0 and enable OCU operation. 
 **  
 ** In this case RTO0 will outputs the logic AND signal of RT0 signal and PPG0,
 ** RTO1 will outputs the logic AND signal of RT1 signal and PPG0.
 ** Changing the value of control bits will influence RTO output.  
 ** 
 ** \brief how to operate WFG with Timer-PPG mode (one of usages). 
 ** 
 ** Before using WFG, the FRT used to connect with applying OCU must be 
 ** initialized first and OCU must be initialed then, PPG and WFG timer 
 ** should be initialized at the following.  
 ** For how to configure FRT, see the description in the mft_frt.h
 ** For how to configure OCU, see the description in the mft_ocu.h  
 ** For how to configure PPG, see the description in the ppg.h 
 ** For how to initialize WFG timer, see the description at the following.
 ** 
 ** Set the control bits with Mft_Wfg_ConfigCtrlBits() with a structure of 
 ** the type #stc_wfg_ctrl_bits 
 ** - enGtenBits = b'00 
 ** - enPselBits = b'00 (input PPG ch.0 to WFG10)
 ** - enPgenBits = b'11  
 ** - enDmodBit = b'0    
 **  
 ** Configure WFG to Timer-PPG mode with Mft_Wfg_ConfigMode()   
 **  
 ** Start PPG0, enable OCU operation and start WFG timer.   
 **
 ** In this case RTO0 will outputs the logic AND signal of WFG timer flag 0
 ** and PPG0, RTO1 will outputs the logic AND signal of WFG timer flag 1 
 ** and PPG0. 
 ** Changing the value of control bits will influence RTO output.  
 **
 ** \brief how to operate WFG with RT-dead timer mode (one of usages). 
 ** 
 ** Before using WFG, the FRT used to connect with applying OCU must be 
 ** initialized first and OCU must be initialed then, WFG timer 
 ** should be initialized at the following.  
 ** For how to configure FRT, see the description in the mft_frt.h
 ** For how to configure OCU, see the description in the mft_ocu.h  
 ** For how to initialize WFG timer, see the description at the following.
 ** 
 ** Configure WFG to RT-dead timer mode with Mft_Wfg_ConfigMode()  
 **  
 ** Set the control bits with Mft_Wfg_ConfigCtrlBits() with a structure of 
 ** the type #stc_wfg_ctrl_bits 
 ** - enGtenBits = b'00 
 ** - enPselBits = b'00 
 ** - enPgenBits = b'00  
 ** - enDmodBit = b'0    
 **  
 ** Enable OCU operation and start WFG timer.   
 **
 ** In this case, RT(1) will trigger the WFG timer to start, WFG will output
 ** the generated non-overlap singal.  
 ** Changing the value of control bits will influence RTO output.  
 **  
 ** \brief how to operate WFG with PPG-dead timer mode (one of usages). 
 ** 
 ** Before using WFG, WFG timer should be initialized at the following.  
 ** For how to initialize WFG timer, see the description at the following.
 ** 
 ** Configure WFG to PPG-dead timer mode with Mft_Wfg_ConfigMode()  
 **  
 ** Set the control bits with Mft_Wfg_ConfigCtrlBits() with a structure of 
 ** the type #stc_wfg_ctrl_bits 
 ** - enGtenBits = b'00 
 ** - enPselBits = b'00 (input PPG0 to WFG10)
 ** - enPgenBits = b'00  
 ** - enDmodBit = b'0    
 **  
 ** Start WFG timer.   
 **
 ** In this case, PPG0 will trigger the WFG timer to start, WFG will output
 ** the generated non-overlap singal.  
 ** Changing the value of control bits will influence RTO output.   
 **
 ** \brief How to use the WFG timer
 **
 ** Mft_Wfg_InitTimerClock() must be used for configuration of a 
 ** Free-Run timer (FRT) channel with a structure of the type 
 ** #en_wfg_timer_clock_t.
 **
 ** A WFG timer interrupt can be enabled by the function 
 ** Mft_Wfg_EnableTimerInt(). This function can set callback function 
 ** for each channel too.
 **
 ** With Mft_Wfg_WriteTimerCountCycle() the WFG timer cycle is set to the value
 ** given in the parameter Mft_Wfg_WriteTimerCountCycle#u16CycleA and
 ** Mft_Wfg_WriteTimerCountCycle#u16CycleB.
 **
 ** After above setting, calling Mft_Wfg_SetTimerCycle() will start WFG timer.
 **
 ** With Mft_Wfg_GetTimerCurCycle() the current WFG timer count can be 
 ** read when WFG timer is running.
 **
 ** With interrupt mode, when the interrupt occurs, the interrupt flag will
 ** be cleared and run into user interrupt callback function.
 **
 ** With polling mode, user can use Mft_Wfg_GetTimerIntFlag() to check if the 
 ** interrupt occurs, and clear the interrupt flag by Mft_Wfg_ClrTimerIntFlag().
 ** 
 ** When stopping the WFG, use Mft_Wfg_DisableTimerInt() to disable WFG timer 
 ** and Mft_Wfg_DisableTimerInt() to disable WFG timer interrupt. 
 ** 
 ** \brief How to use the NZCL (used for motor emergency shutdown) 
 **
 ** Mft_Wfg_ConfigNzcl() must be used for configuration of the WFG NZCL
 ** with a structure of the type #stc_wfg_nzcl_config_t. 
 ** 
 ** Both analog noise filter and digital noise filter can be used to filter the
 ** signal input from DTTIX pin. WFG NTCL interrupt with filter user mode can 
 ** be enabled by the function Mft_Wfg_EnableDtifInt(). 
 ** This function can set callback function for each channel too.
 ** With this funcion, the operation of WFG NZCL is also enabled. 
 **
 ** With interrupt mode, when the interrupt occurs, the interrupt flag will
 ** be cleared and run into user interrupt callback function.
 **
 ** With polling mode, user can use Mft_Wfg_GetDigitalFilterIntFlag() to check 
 ** if the interrupt of digital noise filter occurs, and clear the interrupt 
 ** flag by Mft_Wfg_ClrDigitalFilterIntFlag(). user can use 
 ** Mft_Wfg_GetAnalogFilterIntFlag() to check if the interrupt of digital 
 ** noise filter occurs, and clear the interrupt flag by 
 ** Mft_Wfg_ClrAnalogFilterIntFlag(). 
 ** 
 ** Mft_Wfg_SwTiggerDtif() is used to trigger the DTIF interrupt by software,
 ** regardless of DTTIX pin input status. 
 ** 
 ** When stopping the WFG NZCL, use Mft_Wfg_DisableDtifInt() to disable WFG NZCL
 ** operation as well as WFG NZCL interrupt. 
 **  
 ******************************************************************************/    
//@{
/******************************************************************************
 * Global definitions
 ******************************************************************************/
#define stc_mftn_wfg_t FM4_MFT_WFG_TypeDef

#define MFT0_WFG       (*((volatile stc_mftn_wfg_t *) FM4_MFT0_WFG_BASE))
#define MFT1_WFG       (*((volatile stc_mftn_wfg_t *) FM4_MFT1_WFG_BASE))
#define MFT2_WFG       (*((volatile stc_mftn_wfg_t *) FM4_MFT2_WFG_BASE))

#define  MFT_WFG_CH0             0
#define  MFT_WFG_CH1             1
#define  MFT_WFG_CH2             2
#define  MFT_WFG_CH3             3  
#define  MFT_WFG_CH4             4
#define  MFT_WFG_CH5             5

#define  MFT_WFG_CH10            0
#define  MFT_WFG_CH32            1
#define  MFT_WFG_CH54            2
#define  MFT_WFG_MAXCH           3

/**
 ******************************************************************************
 ** \brief Enumeration to define an index for each WFG instance
 ******************************************************************************/   
typedef enum en_wfg_instance_index
{
    WfgInstanceIndexWfg0 = 0,  ///< Instance index of WFG0
    WfgInstanceIndexWfg1,      ///< Instance index of WFG1
    WfgInstanceIndexWfg2,      ///< Instance index of WFG2

} en_wfg_instance_index_t;  
  
/******************************************************************************
 * \brief WFG operation mode
 ******************************************************************************/
typedef enum en_mft_wfg_mode
{
    WfgThroughMode = 0,           ///< through mode
    WfgRtPpgMode = 1,             ///< RT-PPG mode
    WfgTimerPpgMode = 2,          ///< Timer-PPG mode
    WfgRtDeadTimerMode = 4,       ///< RT-dead timer mode
    WfgRtDeadTimerFilterMode = 5, ///< RT-dead timer filter mode
    WfgPpgDeadTimerFilterMode = 6,///< PPG-dead timer filter mode
    WfgPpgDeadTimerMode = 7,      ///< PPG-dead timer mode
    
}en_mft_wfg_mode_t;

/******************************************************************************
 * \brief Enumeration of GTEN bits setting
 ******************************************************************************/
typedef enum en_gten_bits
{
    GtenBits00B = 0,     ///< GTEN == b'00
    GtenBits01B,         ///< GTEN == b'01
    GtenBits10B,         ///< GTEN == b'10 
    GtenBits11B,         ///< GTEN == b'11
  
}en_gten_bits_t;

/******************************************************************************
 * \brief Enumeration of PSEL bits setting
 ******************************************************************************/
typedef enum en_psel_bits
{
    PselBits00B = 0,    ///< PSEL == b'00
    PselBits01B,        ///< PSEL == b'01
    PselBits10B,        ///< PSEL == b'10
    PselBits11B,        ///< PSEL == b'11
  
}en_psel_bits_t;

/******************************************************************************
 * \brief Enumeration of PGEN bits setting
 ******************************************************************************/
typedef enum en_pgen_bits
{
    PgenBits00B = 0,   ///< PGEN == b'00
    PgenBits01B,       ///< PGEN == b'01 
    PgenBits10B,       ///< PGEN == b'10
    PgenBits11B,       ///< PGEN == b'11
    
}en_pgen_bits_t;

/******************************************************************************
 * \brief Enumeration of DMOD bit setting
 ******************************************************************************/
typedef enum en_dmod_bit
{
    DmodBit0B = 0, ///< output RTO1 and RTO0 signals without changing the level
    DmodBit1B,     ///< output both RTO1 and RTO0 signals reversed
    
}en_dmod_bit_t;

/******************************************************************************
 * \brief WFG control configure
 ******************************************************************************/
typedef struct stc_wfg_ctrl_bits
{
    en_gten_bits_t   enGtenBits; ///< select the output condition
    en_psel_bits_t   enPselBits; ///< select the PPG timer unit
    en_pgen_bits_t   enPgenBits; ///< how to reflect the CH_PPG signal
    en_dmod_bit_t    enDmodBit;  ///< polarity for RTO0 and RTO1 signal output
    
}stc_wfg_ctrl_bits;

/******************************************************************************
 * \brief Enumeration to set count clock prescaler
 ******************************************************************************/
typedef enum en_wfg_timer_clock
{
    WfgPlckDiv1 = 0,  ///< WFG timer clock prescaler: None
    WfgPlckDiv2,      ///< WFG timer clock prescaler: 1/2
    WfgPlckDiv4,      ///< WFG timer clock prescaler: 1/4
    WfgPlckDiv8,      ///< WFG timer clock prescaler: 1/8
    WfgPlckDiv16,     ///< WFG timer clock prescaler: 1/16
    WfgPlckDiv32,     ///< WFG timer clock prescaler: 1/32
    WfgPlckDiv64,     ///< WFG timer clock prescaler: 1/64
    WfgPlckDiv128,    ///< WFG timer clock prescaler: 1/128
  
}en_wfg_timer_clock_t;

/******************************************************************************
 * \brief WFG internal data
 ******************************************************************************/
typedef struct stc_mft_wfg_intern_data
{
    func_ptr_t  pfnWfg10TimerCallback;      ///< Callback function pointer of WFG10 timer interrupt callback  
    func_ptr_t  pfnWfg32TimerCallback;      ///< Callback function pointer of WFG32 timer interrupt callback  
    func_ptr_t  pfnWfg54TimerCallback;      ///< Callback function pointer of WFG54 timer interrupt callback  
    func_ptr_t  pfnWfgAnalogFilterCallback; ///< Callback function pointer of analog filter interrupt callback 
    func_ptr_t  pfnWfgDigtalFilterCallback; ///< Callback function pointer of analog filter interrupt callback 
}stc_mft_wfg_intern_data_t;

/******************************************************************************
 * brief noise canceling width for a digital noise-canceler
 ******************************************************************************/
typedef enum en_nzcl_filter_width
{
    NzlcNoFilter,      ///< no noise-canceling
    NzlcWidth4Cycle,   ///< 4 PCLK cycles
    NzlcWidth8Cycle,   ///< 8 PCLK cycles
    NzlcWidth16Cycle,  ///< 16 PCLK cycles
    NzlcWidth32Cycle,  ///< 32 PCLK cycles
    NzlcWidth64Cycle,  ///< 64 PCLK cycles
    NzlcWidth128Cycle, ///< 128 PCLK cycles
    
}en_nzcl_filter_width_t;

/******************************************************************************
 * brief NZCL configure
 ******************************************************************************/
typedef struct stc_wfg_nzcl_config
{
    boolean_t               bEnDigitalFilter;      ///< enable digital filter
    en_nzcl_filter_width_t  enDigitalFilterWidth;  ///< digital filter width
    boolean_t               bEnAnalogFilter;       ///< enable analog filter
    boolean_t               bSwitchToGpio;           ///< switch to GPIO
}stc_wfg_nzcl_config_t;

/******************************************************************************
 * \brief Structure of DTIF interrupt selection
 ******************************************************************************/
typedef struct stc_dtif_int_sel
{
    boolean_t  bDtifDigitalFilterInt;  ///< Digital filter interrupt selection
    boolean_t  bDtifAnalogFilterInt;   ///< Analog filter interrupt selection
  
}stc_dtif_int_sel_t;

/******************************************************************************
 * \brief Structure of DTIF callback function array
 ******************************************************************************/
typedef struct stc_dtif_int_cb
{
    func_ptr_t  pfnWfgAnalogFilterCallback;  ///< Pointer to analog filter interrupt callback function
    func_ptr_t  pfnWfgDigtalFilterCallback;  ///< Pointer to digital filter interrupt callback function
  
}stc_dtif_int_cb_t;

/******************************************************************************
 * brief structure of WFG instance data
 ******************************************************************************/
typedef struct stc_mft_wfg_instance_data
{
  volatile stc_mftn_wfg_t*  pstcInstance;  ///< pointer to registers of an instance
  stc_mft_wfg_intern_data_t stcInternData; ///< module internal data of instance
} stc_mft_wfg_instance_data_t;

/******************************************************************************/
/* Global variable declarations ('extern', definition in C source)            */
/******************************************************************************/
/// Look-up table for all enabled WFG  instances and their internal data
extern stc_mft_wfg_instance_data_t m_astcMftWfgInstanceDataLut[];

/******************************************************************************/
/* Global function prototypes ('extern', definition in C source)              */
/******************************************************************************/
/* 1. WFG function configuration */
en_result_t Mft_Wfg_ConfigMode(volatile stc_mftn_wfg_t*pstcMft,
		               uint8_t u8CoupleCh, en_mft_wfg_mode_t enMode);
en_result_t Mft_Wfg_ConfigCtrlBits(volatile stc_mftn_wfg_t*pstcMft,uint8_t u8CoupleCh,
		                   stc_wfg_ctrl_bits* pstcCtrlBits);

/* 2. WFG timer configuration */
en_result_t Mft_Wfg_InitTimerClock(volatile stc_mftn_wfg_t*pstcMft,uint8_t u8CoupleCh,
		                           en_wfg_timer_clock_t enClk);
#if (PDL_INTERRUPT_ENABLE_MFT0_WFG == PDL_ON) || \
    (PDL_INTERRUPT_ENABLE_MFT1_WFG == PDL_ON) || \
    (PDL_INTERRUPT_ENABLE_MFT2_WFG == PDL_ON) 
en_result_t Mft_Wfg_EnableTimerInt(volatile stc_mftn_wfg_t*pstcMft,
		                   uint8_t u8CoupleCh, func_ptr_t pfnCallback);
en_result_t Mft_Wfg_DisableTimerInt(volatile stc_mftn_wfg_t*pstcMft, uint8_t u8CoupleCh);
#endif
en_result_t Mft_Wfg_StartTimer(volatile stc_mftn_wfg_t*pstcMft, uint8_t u8CoupleCh);
en_result_t Mft_Wfg_StopTimer(volatile stc_mftn_wfg_t*pstcMft, uint8_t u8CoupleCh);
en_int_flag_t Mft_Wfg_GetTimerIntFlag(volatile stc_mftn_wfg_t*pstcMft, uint8_t u8CoupleCh);
en_result_t Mft_Wfg_ClrTimerIntFlag(volatile stc_mftn_wfg_t*pstcMft, uint8_t u8CoupleCh);
en_result_t Mft_Wfg_WriteTimerCountCycle(volatile stc_mftn_wfg_t*pstcMft,
                                         uint8_t u8CoupleCh, 
                                         uint16_t u16CycleA, 
                                         uint16_t u16CycleB);
en_result_t Mft_Wfg_SetTimerCycle( volatile stc_mftn_wfg_t*pstcMft,
                                      uint8_t u8CoupleCh, uint16_t u16Count);
uint16_t Mft_Wfg_GetTimerCurCycle(volatile stc_mftn_wfg_t*pstcMft, uint8_t u8CoupleCh);

/* 3. WFG NZCL configuration */
en_result_t Mft_Wfg_ConfigNzcl(volatile stc_mftn_wfg_t*pstcMft,
                               stc_wfg_nzcl_config_t* pstcNzclConfig);
#if (PDL_INTERRUPT_ENABLE_MFT0_WFG == PDL_ON) || \
    (PDL_INTERRUPT_ENABLE_MFT1_WFG == PDL_ON) || \
    (PDL_INTERRUPT_ENABLE_MFT2_WFG == PDL_ON) 
en_result_t Mft_Wfg_EnableDtifInt(volatile stc_mftn_wfg_t*pstcMft, stc_dtif_int_sel_t* pstcIntSel, 
                                   stc_dtif_int_cb_t* pstcCallback);
en_result_t Mft_Wfg_DisableDtifInt(volatile stc_mftn_wfg_t*pstcMft, stc_dtif_int_sel_t* stcIntSel);
#endif
en_result_t Mft_Wfg_SwTiggerDtif(volatile stc_mftn_wfg_t*pstcMft);
en_int_flag_t Mft_Wfg_GetDigitalFilterIntFlag(volatile stc_mftn_wfg_t*pstcMft);
en_result_t Mft_Wfg_ClrDigitalFilterIntFlag(volatile stc_mftn_wfg_t*pstcMft);
en_int_flag_t Mft_Wfg_GetAnalogFilterIntFlag(volatile stc_mftn_wfg_t*pstcMft);
en_result_t Mft_Wfg_ClrAnalogFilterIntFlag(volatile stc_mftn_wfg_t*pstcMft);

/* 4. IRQ handler */
void Mft_Wfg_IrqHandler(volatile  stc_mftn_wfg_t*pstcMft,
                        stc_mft_wfg_intern_data_t* pstcMftWfgInternData) ;

#ifdef __cplusplus
}
#endif

//@}

#endif // #if (defined(PDL_PERIPHERAL_ADC_ACTIVE))
#endif
