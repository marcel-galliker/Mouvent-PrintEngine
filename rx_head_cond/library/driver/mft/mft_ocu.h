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
/** \file mft_ocu.h
 **
 ** Headerfile for MFT's Output Compare Unit functions
 **  
 ** History:
 **   - 2013-03-21  0.1  Edison  First version.
 **   - 2013-07-16  0.2  Edison  Add comments about how to use OCU with APIs 
 **   - 2014-01-15  0.3  Edison  Correct typo error
 **
 *****************************************************************************/

#ifndef __MFT_OCU_H__
#define __MFT_OCU_H__
/******************************************************************************/
/* Include files                                                              */
/******************************************************************************/
#include "pdl.h"

#if (defined(PDL_PERIPHERAL_MFT_OCU_ACTIVE))
  
/**
 ******************************************************************************
 ** \defgroup OcuGroup Output Compare Unit (OCU)
 **
 ** Provided functions of OCU module:
 ** 
 ** - Mft_Ocu_Init()
 ** - Mft_Ocu_WriteOcse() 
 ** - Mft_Ocu_EnableOperation()
 ** - Mft_Ocu_DisableOperation()
 ** - Mft_Ocu_EnableInt()
 ** - Mft_Ocu_DisableInt()
 ** - Mft_Ocu_WriteOccp()
 ** - Mft_Ocu_ReadOccp()
 ** - Mft_Ocu_GetIntFlag()
 ** - Mft_Ocu_ClrIntFlag()
 ** - Mft_Ocu_IrqHandler()
 **
 ** \brief How to use OCU module
 ** 
 ** Before using OCU, a FRT used to connect with applying OCU must be initialed.
 ** For how to configure FRT, see the description in the mft_frt.h   
 **
 ** Mft_Ocu_Init() must be used for configuration of a Output Comapre Unit (OCU) 
 ** channel with a structure of the type #stc_mft_ocu_config_t. A FRT should be
 ** connected with applying OCU with this function.
 ** 
 ** Mft_Ocu_WriteOcse() is used to set the compare function/mode. For the 
 ** meaning of each bit in OCSE register, please refer to MFT chapter of 
 ** FM4 peripheral manual.
 ** 
 ** With Mft_Ocu_WriteOccp() the OCU compare value is set to the value
 ** given in the parameter Mft_Ocu_WriteOccp#u16Cycle. Whether the compare
 ** value is modified directly depends on buffer function. 
 **
 ** A OCU interrupt can be enabled by the function Mft_Ocu_EnableInt().
 ** This function can set callback function for each channel too.
 **
 ** After above setting, calling Mft_Ocu_EnableOperation() will start OCU.
 **
 ** With interrupt mode, the interrupt occurs when FRT count match OCU compare
 ** value, the interrupt flag will be cleared and run into user 
 ** interrupt callback function.
 **
 ** With polling mode, user can use Mft_Ocu_GetIntFlag() to check if the 
 ** interrupt occurs, and clear the interrupt flag by Mft_Ocu_ClrIntFlag().
 ** 
 ** When stopping the OCU, use Mft_Ocu_DisableOperation() to disable OCU and 
 ** Mft_Ocu_DisableInt() to disable OCU interrupt.
 **
 ******************************************************************************/  
 //@{
/******************************************************************************
 * Global definitions
 ******************************************************************************/
#define stc_mftn_ocu_t FM4_MFT_OCU_TypeDef

#define MFT0_OCU       (*((volatile stc_mftn_ocu_t *) FM4_MFT0_OCU_BASE))
#define MFT1_OCU       (*((volatile stc_mftn_ocu_t *) FM4_MFT1_OCU_BASE))
#define MFT2_OCU       (*((volatile stc_mftn_ocu_t *) FM4_MFT1_OCU_BASE))

#define  MFT_OCU_CH0             0
#define  MFT_OCU_CH1             1
#define  MFT_OCU_CH2             2
#define  MFT_OCU_CH3             3  
#define  MFT_OCU_CH4             4
#define  MFT_OCU_CH5             5
#define  MFT_OCU_MAXCH           6

#define  MFT_OCU_CH10            0
#define  MFT_OCU_CH32            1
#define  MFT_OCU_CH54            2

/**
 ******************************************************************************
 ** \brief Enumeration to define an index for each OCU instance
 ******************************************************************************/   
typedef enum en_ocu_instance_index
{
    OcuInstanceIndexOcu0 = 0,  ///< Instance index of OCU0
    OcuInstanceIndexOcu1,      ///< Instance index of OCU1
    OcuInstanceIndexOcu2,      ///< Instance index of OCU2

} en_ocu_instance_index_t;  
  
/**
 ******************************************************************************
 ** \brief select the FRT to be connected to OCU
 ******************************************************************************/
typedef enum en_mft_ocu_frt
{
    Frt0ToOcu,       ///< connect FRT0 to OCU
    Frt1ToOcu,       ///< connect FRT1 to OCU
    Frt2ToOcu,       ///< connect FRT2 to OCU
    OcuFrtToExt,     ///< connect FRT of an external MFT
}en_mft_ocu_frt_t;

/**
 ******************************************************************************
 ** \brief output level of the RT pin state
 ******************************************************************************/
typedef enum en_ocu_rt_out_state
{
    RtLowLevel = 0,         ///< output low level to RT pin
    RtHighLevel             ///< output high level to RT pin
}en_ocu_rt_out_state_t;

/**
 ******************************************************************************
 ** \brief buffer register function of OCCP
 ******************************************************************************/
typedef enum en_ocu_occp_buf
{
    OccpBufDisable = 0,         ///< disable the buffer function
    OccpBufTrsfByFrtZero,       ///< buffer transfer when counter value is 0x0000
    OccpBufTrsfByFrtPeak,       ///< buffer transfer when counter value is TCCP
    OccpBufTrsfByFrtZeroPeak,   ///< buffer transfer when the value is both 0 and TCCP
}en_ocu_occp_buf_t;


/**
 ******************************************************************************
 ** \brief buffer register function of OCSE
 ******************************************************************************/
typedef enum en_ocu_ocse_buf
{
    OcseBufDisable = 0,         ///< disable the buffer function
    OcseBufTrsfByFrtZero,       ///< buffer transfer when counter value is 0x0000
    OcseBufTrsfByFrtPeak,       ///< buffer transfer when counter value is TCCP
    OcseBufTrsfByFrtZeroPeak,   ///< buffer transfer when the value is both 0 and TCCP

}en_ocu_ocse_buf_t;

/**
 ******************************************************************************
 ** \brief OCU configure
 ******************************************************************************/
typedef struct stc_mft_ocu_config
{
    en_mft_ocu_frt_t    enFrtConnect;  ///< select the FRT to be connected to OCU
    boolean_t           bFm4;          ///< select FM4 mode or FM3-compatible mode
    boolean_t           bCmod;         ///< FM3 compatible mode
    uint8_t             u8Mod;         ///< select OCU ch0~ch5 in FM3 compatible
    en_ocu_occp_buf_t   enOccpBufMode; ///< buffer register function of OCCP
    en_ocu_ocse_buf_t   enOcseBufMode; ///< buffer register function of OCSE
    en_ocu_rt_out_state_t enStatePin;  ///<RT output level state
}stc_mft_ocu_config_t;

/**
 ******************************************************************************
 ** \brief RT output status
 ******************************************************************************/
typedef enum en_rt_status
{
    RtOutputHold = 0,              ///< RT output hold
    RtOutputHigh,                  ///< RT output high 
    RtOutputLow,                   ///< RT output low  
    RtOutputReverse,               ///< RT output reverse  
  
}en_rt_even_status_t, en_rt_odd_status_t;

/**
 ******************************************************************************
 ** \brief The condition for IOP flag set
 ******************************************************************************/
typedef enum en_iop_flag_set_condition
{
    IopFlagHold = 0,           ///< IOP flag hold
    IopFlagSet,                ///< IOP flag set
  
}en_iop_flag_even_t, en_iop_flag_odd_t;

/**
 ******************************************************************************
 ** \brief The compare configuration of even OCU channel
 ******************************************************************************/
typedef struct stc_even_compare_config
{
    en_rt_even_status_t enFrtZeroEvenMatchEvenChRtStatus;    ///< Even channel's RT output status when even channel match occurs at the condition of FRT count=0x0000      
    en_rt_even_status_t enFrtZeroEvenNotMatchEvenChRtStatus; ///< Even channel's RT output status when even channel match doesn't occur at the condition of FRT count=0x0000    
    
    en_rt_even_status_t enFrtUpCntEvenMatchEvenChRtStatus;   ///< Even channel's RT output status when even channel match occurs at the condition of FRT is counting up     
    
    en_rt_even_status_t enFrtPeakEvenMatchEvenChRtStatus;  ///< Even channel's RT output status when even channel match occurs at the condition of FRT count=Peak
    en_rt_even_status_t enFrtPeakEvenNotMatchEvenChStatus; ///< Even channel's RT output status when even channel match doesn't occur at the condition of FRT count=Peak
    
    en_rt_even_status_t enFrtDownCntEvenMatchEvenChRtStatus;    ///< Even channel's RT output status when even channel match occurs at the condition of FRT is counting down 
    
    en_iop_flag_even_t enIopFlagWhenFrtZeroEvenMatch;    ///< Even channel OCU's IOP flag status when even channel match occurs at the condition of FRT count=0x0000
    en_iop_flag_even_t enIopFlagWhenFrtUpCntEvenMatch;   ///< Even channel OCU's IOP flag status when even channel match occurs at the condition of FRT is counting up
    en_iop_flag_even_t enIopFlagWhenFrtPeakEvenMatch;    ///< Even channel OCU's IOP flag status when even channel match occurs at the condition of FRT count=Peak
    en_iop_flag_even_t enIopFlagWhenFrtDownCntEvenMatch; ///< Even channel OCU's IOP flag status when even channel match occurs at the condition of FRT is counting down
    
}stc_even_compare_config_t;

/**
 ******************************************************************************
 ** \brief The compare configuration of odd OCU channel
 ******************************************************************************/
typedef struct stc_odd_compare_config
{
    en_rt_odd_status_t enFrtZeroOddMatchEvenMatchOddChRtStatus;       ///< Odd channel's RT output status when even channel and odd channel match occurs at the condition of FRT count=0x0000   
    en_rt_odd_status_t enFrtZeroOddMatchEvenNotMatchOddChRtStatus;    ///< Odd channel's RT output status when even channel not match and odd channel match occurs at the condition of FRT count=0x0000 
    en_rt_odd_status_t enFrtZeroOddNotMatchEvenMatchOddChRtStatus;    ///< Odd channel's RT output status when even channel match and odd channel not match occurs at the condition of FRT count=0x0000 
    en_rt_odd_status_t enFrtZeroOddNotMatchEvenNotMatchOddChRtStatus; ///< Odd channel's RT output status when even channel not match and odd channel not match occurs at the condition of FRT count=0x0000
    
    en_rt_odd_status_t enFrtUpCntOddMatchEvenMatchOddChRtStatus;      ///< Odd channel's RT output status when even channel and odd channel match occurs at the condition of FRT is counting up 
    en_rt_odd_status_t enFrtUpCntOddMatchEvenNotMatchOddChRtStatus;   ///< Odd channel's RT output status when even channel not match and odd channel match occurs at the condition of FRT is counting up
    en_rt_odd_status_t enFrtUpCntOddNotMatchEvenMatchOddChRtStatus;   ///< Odd channel's RT output status when even channel match and odd channel not match occurs at the condition of FRT is counting up
    
    en_rt_odd_status_t enFrtPeakOddMatchEvenMatchOddChRtStatus;      ///< Odd channel's RT output status when even channel and odd channel match occurs at the condition of FRT count=Peak
    en_rt_odd_status_t enFrtPeakOddMatchEvenNotMatchOddChRtStatus;   ///< Odd channel's RT output status when even channel not match and odd channel match occurs at the condition of FRT count=Peak
    en_rt_odd_status_t enFrtPeakOddNotMatchEvenMatchOddChRtStatus;   ///< Odd channel's RT output status when even channel match and odd channel not match occurs at the condition of FRT count=Peak
    en_rt_odd_status_t enFrtPeakOddNotMatchEvenNotMatchOddChRtStatus;///< Odd channel's RT output status when even channel not match and odd channel not match occurs at the condition of FRT count=Peak
    
    en_rt_odd_status_t enFrtDownOddMatchEvenMatchOddChRtStatus;      ///< Odd channel's RT output status when even channel and odd channel match occurs at the condition of FRT is counting down
    en_rt_odd_status_t enFrtDownOddMatchEvenNotMatchOddChRtStatus;   ///< Odd channel's RT output status when even channel not match and odd channel match occurs at the condition of FRT is counting down
    en_rt_odd_status_t enFrtDownOddNotMatchEvenMatchOddChRtStatus;   ///< Odd channel's RT output status when even channel match and odd channel not match occurs at the condition of FRT is coutning down
    
    en_iop_flag_odd_t enIopFlagWhenFrtZeroOddMatch;    ///< Odd channel OCU's IOP flag status when Odd channel match occurs at the condition of FRT count=0x0000
    en_iop_flag_odd_t enIopFlagWhenFrtUpCntOddMatch;   ///< Odd channel OCU's IOP flag status when Odd channel match occurs at the condition of FRT is counting up
    en_iop_flag_odd_t enIopFlagWhenFrtPeakOddMatch;    ///< Odd channel OCU's IOP flag status when Odd channel match occurs at the condition of FRT count=Peak
    en_iop_flag_odd_t enIopFlagWhenFrtDownCntOddMatch; ///< Odd channel OCU's IOP flag status when Odd channel match occurs at the condition of FRT is counting down
  
}stc_odd_compare_config_t;

/**
 ******************************************************************************
 ** \brief Structure of OCU internal data
 ******************************************************************************/
typedef struct stc_mft_ocu_intern_data
{
    func_ptr_t  pfnOcu0Callback;      ///< Callback function pointer of OCU0 interrupt 
    func_ptr_t  pfnOcu1Callback;      ///< Callback function pointer of OCU1 interrupt 
    func_ptr_t  pfnOcu2Callback;      ///< Callback function pointer of OCU2 interrupt 
    func_ptr_t  pfnOcu3Callback;      ///< Callback function pointer of OCU3 interrupt 
    func_ptr_t  pfnOcu4Callback;      ///< Callback function pointer of OCU4 interrupt 
    func_ptr_t  pfnOcu5Callback;      ///< Callback function pointer of OCU5 interrupt 
}stc_mft_ocu_intern_data_t;

/**
 ******************************************************************************
 ** \brief OCU instance data type
 ******************************************************************************/
typedef struct stc_mft_ocu_instance_data
{
  volatile stc_mftn_ocu_t*  pstcInstance;  ///< pointer to registers of an instance
  stc_mft_ocu_intern_data_t stcInternData; ///< module internal data of instance
} stc_mft_ocu_instance_data_t;

/* C binding of definitions if building with C++ compiler */
#ifdef __cplusplus
extern "C"
{
#endif
/******************************************************************************/
/* Global variable declarations ('extern', definition in C source)            */
/******************************************************************************/
/// Look-up table for all enabled OCU  instances and their internal data
extern stc_mft_ocu_instance_data_t m_astcMftOcuInstanceDataLut[];

/******************************************************************************/
/* Global function prototypes ('extern', definition in C source)              */
/******************************************************************************/
/* Init */
en_result_t Mft_Ocu_Init( volatile stc_mftn_ocu_t*pstcMft, 
                          uint8_t u8Ch, stc_mft_ocu_config_t*  pstcOcuConfig);

/* Compare congifuration */
en_result_t Mft_Ocu_SetEvenChCompareMode(volatile stc_mftn_ocu_t*pstcMft,
                                         uint8_t EvenCh,
                                         stc_even_compare_config_t* pstcConfig);
en_result_t Mft_Ocu_SetOddChCompareMode (volatile stc_mftn_ocu_t*pstcMft,
                                         uint8_t OddCh,
                                         stc_odd_compare_config_t* pstcConfig);
                          
/* Func/Int enable/disable */
en_result_t Mft_Ocu_EnableOperation(volatile stc_mftn_ocu_t*pstcMft, uint8_t u8Ch);
en_result_t Mft_Ocu_DisableOperation(volatile stc_mftn_ocu_t*pstcMft, uint8_t u8Ch);
#if (PDL_INTERRUPT_ENABLE_MFT0_OCU == PDL_ON) || \
    (PDL_INTERRUPT_ENABLE_MFT1_OCU == PDL_ON) || \
    (PDL_INTERRUPT_ENABLE_MFT2_OCU == PDL_ON)
en_result_t Mft_Ocu_EnableInt(volatile stc_mftn_ocu_t*pstcMft,
                              uint8_t u8Ch,func_ptr_t  pfnCallback);
en_result_t Mft_Ocu_DisableInt(volatile stc_mftn_ocu_t*pstcMft,uint8_t u8Ch);
#endif

/* Count write/read */
en_result_t Mft_Ocu_WriteOccp(volatile stc_mftn_ocu_t*pstcMft, 
                              uint8_t u8Ch, uint16_t u16Occp);
uint16_t Mft_Ocu_ReadOccp(volatile stc_mftn_ocu_t*pstcMft, uint8_t u8Ch);

/* Status read/clear */
en_int_flag_t Mft_Ocu_GetIntFlag(volatile stc_mftn_ocu_t*pstcMft, uint8_t u8Ch);
en_result_t Mft_Ocu_ClrIntFlag(volatile stc_mftn_ocu_t*pstcMft, uint8_t u8Ch);

/* Get RT pin level */
en_ocu_rt_out_state_t Mft_Ocu_GetRtPinLevel(volatile stc_mftn_ocu_t*pstcMft, uint8_t u8Ch);

/* IRQ handler */
void Mft_Ocu_IrqHandler( volatile stc_mftn_ocu_t*pstcMft,
                         stc_mft_ocu_intern_data_t* pstcMftOcuInternData) ;


#ifdef __cplusplus
}
#endif

//@}

#endif // #if (defined(PDL_PERIPHERAL_ADC_ACTIVE))

#endif
