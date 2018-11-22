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
/** \file qprc.h
 **
 ** Headerfile for QPRC functions
 **
 ** History:
 **   - 2013-03-21  0.1  Edison  First version.
 **   - 2013-07-16  0.2  Edison  Add comments about how to use QPRC with APIs 
 **
 *****************************************************************************/
#ifndef __QPRC_H__
#define __QPRC_H__

/******************************************************************************/
/* Include files                                                              */
/******************************************************************************/
#include "pdl.h"

#if (defined(PDL_PERIPHERAL_QPRC_ACTIVE))

/**
 ******************************************************************************
 ** \defgroup QprcGroup Quadrature Position/Revolution Counter (QPRC)
 **
 ** Provided functions of QPRC module:
 ** 
 ** - Qprc_Init()
 ** - Qprc_StopPcCount() 
 ** - Qprc_RestartPcCount()
 ** - Qprc_SetPcCount()
 ** - Qprc_GetPcCount()
 ** - Qprc_SetRcCount()
 ** - Qprc_GetRcCount()
 ** - Qprc_SetPcCompareValue()
 ** - Qprc_GetPcCompareValue()
 ** - Qprc_SetPcRcCompareValue()
 ** - Qprc_GetPcRcCompareValue()
 ** - Qprc_ConfigPcMode()
 ** - Qprc_ConfigRcMode()
 ** - Qprc_EnableInt()
 ** - Qprc_DisableInt()
 ** - Qprc_GetIntFlag()
 ** - Qprc_ClrIntFlag()
 ** - Qprc_GetPcOfUfDir()
 ** - Qprc_GetPcDir()
 ** - Qprc_IrqHandler()
 **
 ** \brief How to use Position Count (PC) and Revolution Counter (RC)
 **
 ** Qprc_Init() must be used for configuration of a QPRC channel with a 
 ** structure of the type #stc_qprc_config_t.
 **
 ** A QPRC interrupt can be enabled by the function Qprc_EnableInt().
 ** This function can set callback function for each channel too.
 **
 ** With Qprc_SetPcCompareValue() the PC cmpare value is set to the value
 ** given in the parameter Qprc_SetPcCompareValue#u16PcValue. And PC cmpare 
 ** value is read by Qprc_GetPcCompareValue().
 **
 ** With Qprc_SetPcRcCompareValue() the PC and RC cmpare value is set to 
 ** the value given in the parameter Qprc_SetPcRcCompareValue#u16PcRcValue. 
 ** And PC and RC cmpare value is read by Qprc_GetPcRcCompareValue(). Whether
 ** PC and RC cmpare value compares to PC count and RC count depends on the 
 ** setting in paramter of the Qprc_Init().
 ** 
 ** The initial PC count value can be set by Qprc_SetPcCount(). And current
 ** PC count can be read by Qprc_GetPcCount()
 **
 ** The initial RC count value can be set by Qprc_SetRcCount(). And current
 ** RC count can be read by Qprc_GetRcCount()
 **
 ** The maximum PC count value can be set by Qprc_SetPcMaxValue(). And 
 ** the maximum PC count value can be read by Qprc_GetPcMaxValue(). If PC
 ** count exceeds this value, a PC overflow interrupt flag will be set.
 **
 ** After above setting, Qprc_ConfigPcMode() can start PC with following mode:
 ** - Up-down count mode(PC mode 1)
 ** - Phase different count mode (PC mode 2)
 ** - Directional count mode (PC mode 3)
 **
 ** Qprc_ConfigRcMode() can start RC with following mode:
 ** - ZIN trigger mode(RC mode 1)
 ** - PC underflow or overflow detection trigger mode (RC mode 2)
 ** - ZIN or PC underflow or overflow detection trigger mode(RC mode 3)
 **
 ** With interrupt mode, when the interrupt occurs, the interrupt flag will
 ** be cleared and run into user interrupt callback function.
 **
 ** With polling mode, user can use Mft_Qprc_GetIntFlag() to check if the 
 ** interrupt occurs, and clear the interrupt flag by Mft_Qprc_ClrIntFlag().
 **
 ** Qprc_GetPcOfUfDir() can get the PC direction after a PC overflow or  
 ** underflow occurs.
 **
 ** Qprc_GetPcDir() can get the current PC direction when PC is counting.
 **
 ** Qprc_StopPcCount() can stop PC when PC is counting. And Qprc_RestartPcCount()
 ** will restart PC when PC is in stop status.
 ** 
 ** When stopping the QPRC, disable PC by using Qprc_ConfigPcMode() to set PC 
 ** to PC mode0 and disable RC by using Qprc_ConfigRcMode() to set RC to RC 
 ** mode0. Use Mft_QPRC_DisableInt() to disable QPRC interrupt.
 **
 ******************************************************************************/

//@{

/******************************************************************************
 * Global definitions
 ******************************************************************************/
#define stc_qprcn_t     FM4_QPRC_TypeDef
#define stc_qprc_nfn_t  FM4_QPRC_NF_TypeDef

#define QPRC0       (*((volatile stc_qprcn_t *) FM4_QPRC0_BASE))
#define QPRC1       (*((volatile stc_qprcn_t *) FM4_QPRC1_BASE))
#define QPRC2       (*((volatile stc_qprcn_t *) FM4_QPRC2_BASE))
#define QPRC3       (*((volatile stc_qprcn_t *) FM4_QPRC3_BASE))

#define QPRCNF0       (*((volatile stc_qprc_nfn_t *) FM4_QPRC0_NF_BASE))
#define QPRCNF1       (*((volatile stc_qprc_nfn_t *) FM4_QPRC1_NF_BASE))
#define QPRCNF2       (*((volatile stc_qprc_nfn_t *) FM4_QPRC2_NF_BASE))
#define QPRCNF3       (*((volatile stc_qprc_nfn_t *) FM4_QPRC3_NF_BASE))

#define QPRC_PC_OVERFLOW_INT       0
#define QPRC_PC_UNDERFLOW_INT      1
#define QPRC_PC_ZERO_INDEX_INT     2

/**
 ******************************************************************************
 ** \brief Enumeration to define an index for each QPRC instance
 ******************************************************************************/
typedef enum en_qprc_instance_index
{
    QprcInstanceIndexQprc0 = 0,  ///< Instance index of QPRC0
    QprcInstanceIndexQprc1,      ///< Instance index of QPRC1
    QprcInstanceIndexQprc2,      ///< Instance index of QPRC2
    QprcInstanceIndexQprc3,      ///< Instance index of QPRC3

} en_qprc_instance_index_t;

/**
 ******************************************************************************
 ** \brief Enumeration to define position counter mode
 ******************************************************************************/
typedef enum en_qprc_pcmode
{
    QprcPcMode0 = 0,  ///< PC_Mode0: Disable position counter
    QprcPcMode1 = 1,  ///< PC_Mode1: Increments with AIN active edge and decrements with BIN active edge
    QprcPcMode2 = 2,  ///< PC_Mode2: Phase difference count mode: Counts up if AIN is leading BIN, down if BIN leading.
    QprcPcMode3 = 3   ///< PC_Mode3: Directional count mode: Counts up/down with BIN active edge and AIN level
} en_qprc_pcmode_t;

/**
 ******************************************************************************
 ** \brief Enumeration to define revolution counter mode
 ******************************************************************************/
typedef enum en_qprc_rcmode
{
    QprcRcMode0 = 0,  ///< RC_Mode0: Disable revolution counter
    QprcRcMode1 = 1,  ///< RC_Mode1: Up/down count of revolution counter with ZIN active edge
    QprcRcMode2 = 2,  ///< RC_Mode2: Up/down count of revolution counter on over- oder underflow in position count match
    QprcRcMode3 = 3   ///< RC_Mode3: Up/down count of revolution counter on over- oder underflow in position count match and ZIN active edge
} en_qprc_rcmode_t;

/**
 ******************************************************************************
 ** \brief Enumeration to define ZIN valid edge
 ******************************************************************************/
typedef enum en_qprc_zinedge
{
    QprcZinDisable     = 0,  ///< Disables edge and level detection
    QprcZinFallingEdge = 1,  ///< ZIN active at falling edge
    QprcZinRisingEdge  = 2,  ///< ZIN active at rising edge
    QprcZinBothEdges   = 3,  ///< ZIN active at falling or rising edge
    QprcZinLowLevel    = 4,  ///< ZIN active at low level detected
    QprcZinHighLevel   = 5   ///< ZIN active at high level detected
} en_qprc_zinedge_t;

/**
 ******************************************************************************
 ** \brief Enumeration to define BIN valid edge
 ******************************************************************************/
typedef enum en_qprc_binedge
{
    QprcBinDisable     = 0,  ///< Disables edge detection
    QprcBinFallingEdge = 1,  ///< BIN active at falling edge
    QprcBinRisingEdge  = 2,  ///< BIN active at rising edge
    QprcBinBothEdges   = 3   ///< BIN active at falling or rising edge
} en_qprc_binedge_t;

/**
 ******************************************************************************
 ** \brief Enumeration to define AIN valid edge
 ******************************************************************************/
typedef enum en_qprc_ainedge
{
    QprcAinDisable     = 0,  ///< Disables edge detection
    QprcAinFallingEdge = 1,  ///< AIN active at falling edge
    QprcAinRisingEdge  = 2,  ///< AIN active at rising edge
    QprcAinBothEdges   = 3   ///< AIN active at falling or rising edge
} en_qprc_ainedge_t;

/**
 ******************************************************************************
 ** \brief Enumeration to define reset mask times of position counter
 ******************************************************************************/
typedef enum en_qprc_pcresetmask
{
    QprcResetMaskDisable  = 0,  ///< No reset mask
    QprcResetMask2Times   = 1,  ///< The position counter reset or a revolution counter count-up or -down events are ignored until the position counter changes twice
    QprcResetMask4Times   = 2,  ///< The position counter reset or a revolution counter count-up or -down events are ignored until the position counter changes four times
    QprcResetMask8Times   = 3   ///< The position counter reset or a revolution counter count-up or -down events are ignored until the position counter changes eight times
} en_qprc_pcresetmask_t;

/**
 ******************************************************************************
 ** \brief Enumeration to define compare object of QPRCR register
 ******************************************************************************/
typedef enum en_qprc_compmode
{
    QprcComapreWithPosition   = 0,  ///< Compares the value of the QPRC Position and Revolution Counter Compare Register (QPRCR) with that of the position counter.
    QprcComapreWithRevolution = 1   ///< Compares the value of the QPRC Position and Revolution Counter Compare Register (QPRCR) with that of the revolution counter.
} en_qprc_compmode_t;

/**
 ******************************************************************************
 ** \brief Enumeration to define QPRC interrupt type
 ******************************************************************************/
typedef enum en_qprc_int
{
    QprcPcOfUfZeroInt = 0,    ///< Overflow, undowflow, zero match interrupt of position counter
    QprcPcMatchInt,           ///< PC match interrupt of position counter
    QprcPcRcMatchInt,         ///< PC and RC match interrupt
    QprcPcMatchRcMatchInt,    ///< PC match and RC match interrupt
    QprcPcCountInvertInt,     ///< PC invert interrupt
    QprcRcOutrangeInt,        ///< RC outrange interrupt

}en_qprc_int_t;

/**
 ******************************************************************************
 ** \brief Enumeration to define QPRC interrupt selection
 ******************************************************************************/
typedef struct stc_qprc_int_sel
{
    boolean_t    bQprcPcOfUfZeroInt;      ///< Overflow, undowflow, zero match interrupt of position counter, 1: selected, 0: not selected
    boolean_t    bQprcPcMatchInt;         ///< PC match interrupt of position counter, 1: selected, 0: not selected
    boolean_t    bQprcPcRcMatchInt;       ///< PC and RC match interrupt, 1: selected, 0: not selected
    boolean_t    bQprcPcMatchRcMatchInt;  ///< PC match and RC match interrupt, 1: selected, 0: not selected
    boolean_t    bQprcPcCountInvertInt;   ///< PC invert interrupt, 1: selected, 0: not selected
    boolean_t    bQprcRcOutrangeInt;      ///< RC outrange interrupt, 1: selected, 0: not selected

}stc_qprc_int_sel_t;

/**
 ******************************************************************************
 ** \brief Enumeration to define QPRC interrupt callback function
 ******************************************************************************/
typedef struct stc_qprc_int_cb
{
    func_ptr_arg1_t   pfnPcOfUfZeroIntCallback;      ///< Overflow, undowflow, zero match interrupt callback function of position counter
    func_ptr_t        pfnPcMatchIntCallback;         ///< PC match interrupt callback function of position counter
    func_ptr_t        pfnPcRcMatchIntCallback;       ///< PC and RC match interrupt callback function
    func_ptr_t        pfnPcMatchRcMatchIntCallback;  ///< PC match and RC match interrupt callback function
    func_ptr_t        pfnPcCountInvertIntCallback;   ///< PC invert interrupt callback function
    func_ptr_t        pfnRcOutrangeIntCallback;      ///< RC outrange interrupt callback function

}stc_qprc_int_cb_t;

/**
 ******************************************************************************
 ** \brief Enumeration to define QPRC filter width
 ******************************************************************************/
typedef enum en_qprc_filter_width
{
    QprcNoFilter = 0,           ///< No filter
    QprcFilterWidth4Pclk,       ///< QPRC filter width: 4 PCLK
    QprcFilterWidth8Pclk,       ///< QPRC filter width: 8 PCLK
    QprcFilterWidth16Pclk,      ///< QPRC filter width: 16 PCLK
    QprcFilterWidth32Pclk,      ///< QPRC filter width: 32 PCLK
    QprcFilterWidth64Pclk,      ///< QPRC filter width: 64 PCLK
    QprcFilterWidth128Pclk,     ///< QPRC filter width: 128 PCLK
    QprcFilterWidth256Pclk,     ///< QPRC filter width: 256 PCLK

}en_qprc_filter_width_t;

/**
 ******************************************************************************
 ** \brief Structure to configuration QPRC filter
 ******************************************************************************/
typedef struct stc_qprc_filter
{
    boolean_t               bInputMask;     ///< Input mask setting
    boolean_t               bInputInvert;   ///< Input invert setting
    en_qprc_filter_width_t  enWidth;        ///< QPRC filter width setting

}stc_qprc_filter_t;

/**
 ******************************************************************************
 ** \brief Structure to configuration QPRC
 ******************************************************************************/
typedef struct stc_qprc_config
{
    boolean_t             bSwapAinBin;                    ///< TRUE: Swap AIN and BIN inputs
    en_qprc_compmode_t    enComapreMode;                  ///< Description see #en_qprc_compmode_t
    en_qprc_zinedge_t     enZinEdge;                      ///< Detection mode of the ZIN pin
    en_qprc_binedge_t     enBinEdge;                      ///< Detection mode of the BIN pin
    en_qprc_ainedge_t     enAinEdge;                      ///< Detection mode of the AIN pin
    en_qprc_pcresetmask_t enPcResetMask;                  ///< Position counter reset mask
    boolean_t             b8KValue;                       ///< TRUE: Outrange mode from 0 to 0x7FFF, FALSE: Outrange mode from 0 to 0xFFFF:
    stc_qprc_filter_t     stcAinFilter;                   ///< AIN noise filter configuration
    stc_qprc_filter_t     stcBinFilter;                   ///< BIN noise filter configuration
    stc_qprc_filter_t     stcCinFilter;                   ///< CIN noise filter configuration

} stc_qprc_config_t;

/**
 ******************************************************************************
 ** \brief Structure of QPRC internal data
 ******************************************************************************/
typedef struct stc_qprc_intern_data
{
    func_ptr_arg1_t  pfnPcUfOfZeroCallback;      ///< Pointer to overflow, undowflow, zero match interrupt callback function of position counter
    func_ptr_t       pfnPcMatchCallback;         ///< Pointer to PC match interrupt callback function of position counter
    func_ptr_t       pfnPcRcMatchCallback;       ///< Pointer to PC and RC match interrupt callback function
    func_ptr_t       pfnPcMatchRcMatchCallback;  ///< Pointer to PC match and RC match interrupt
    func_ptr_t       pfnPcCountInvertCallback;   ///< Pointer to PC invert interrupt
    func_ptr_t       pfnRcOutrangeCallback;      ///< Pointer to RC outrange interrupt callback function

} stc_qprc_intern_data_t;

/**
 ******************************************************************************
 ** \brief Structure of QPRC instance data
 ******************************************************************************/
typedef struct stc_qprc_instance_data
{
    volatile stc_qprcn_t*  pstcInstance;       ///< pointer to registers of an instance
    volatile stc_qprc_nfn_t* pstcInstanceNf;   ///< pointer to registers of a QPRC-NF instance
    stc_qprc_intern_data_t stcInternData;      ///< module internal data of instance
} stc_qprc_instance_data_t;

/******************************************************************************/
/* Global variable definitions ('extern')                                     */
/******************************************************************************/

/// Look-up table for all enabled MFS instances and their internal data
extern stc_qprc_instance_data_t m_astcQprcInstanceDataLut[];

/******************************************************************************/
/* Global function prototypes (definition in C source)                        */
/******************************************************************************/
/* C binding of definitions if building with C++ compiler */
#ifdef __cplusplus
extern "C"
{
#endif

/* QPRC init */
en_result_t Qprc_Init( volatile stc_qprcn_t* pstcQprc, stc_qprc_config_t*  pstcConfig );
/* Stop/Restart Pc Count */
en_result_t Qprc_StopPcCount(volatile stc_qprcn_t *pstcQprc);
en_result_t Qprc_RestartPcCount(volatile stc_qprcn_t *pstcQprc);
/* Count write/read */
en_result_t Qprc_SetPcCount ( volatile stc_qprcn_t* pstcQprc, uint16_t u16PcValue ) ;
uint16_t Qprc_GetPcCount ( volatile stc_qprcn_t* pstcQprc );
en_result_t Qprc_SetRcCount ( volatile stc_qprcn_t* pstcQprc, uint16_t u16RcValue );
uint16_t Qprc_GetRcCount ( volatile stc_qprcn_t* pstcQprc );
en_result_t Qprc_SetPcMaxValue( volatile stc_qprcn_t* pstcQprc, uint16_t u16PcMaxValue ) ;
uint16_t Qprc_GetPcMaxValue(volatile stc_qprcn_t *pstcQprc);
en_result_t Qprc_SetPcCompareValue( volatile stc_qprcn_t* pstcQprc, uint16_t u16PcValue ) ;
uint16_t Qprc_GetPcCompareValue( volatile stc_qprcn_t *pstcQprc);
en_result_t Qprc_SetPcRcCompareValue( volatile stc_qprcn_t* pstcQprc, uint16_t u16PcRcValue ) ;
uint16_t Qprc_GetPcRcCompareValue(volatile stc_qprcn_t *pstcQprc);
/* Mode configuration */
en_result_t Qprc_ConfigPcMode( volatile stc_qprcn_t* pstcQprc, en_qprc_pcmode_t enMode );
en_result_t Qprc_ConfigRcMode( volatile stc_qprcn_t* pstcQprc, en_qprc_rcmode_t enMode );
/* Interrupt configuration */
#if (PDL_INTERRUPT_ENABLE_QPRC0 == PDL_ON) || \
    (PDL_INTERRUPT_ENABLE_QPRC1 == PDL_ON) || \
    (PDL_INTERRUPT_ENABLE_QPRC2 == PDL_ON) || \
    (PDL_INTERRUPT_ENABLE_QPRC3 == PDL_ON)
en_result_t Qprc_EnableInt( volatile stc_qprcn_t* pstcQprc, stc_qprc_int_sel_t* pstcIntSel,
                            stc_qprc_int_cb_t*  pstcIntCallback );
en_result_t Qprc_DisableInt( volatile stc_qprcn_t* pstcQprc, stc_qprc_int_sel_t* pstcIntSel );
#endif
en_int_flag_t Qprc_GetIntFlag( volatile stc_qprcn_t* pstcQprc, en_qprc_int_t enIntType );
en_result_t Qprc_ClrIntFlag( volatile stc_qprcn_t *pstcQprc, en_qprc_int_t enIntType );
/* status */
en_stat_flag_t Qprc_GetPcOfUfDir( volatile stc_qprcn_t* pstcQprc );
en_stat_flag_t Qprc_GetPcDir( volatile stc_qprcn_t* pstcQprc );
/* IRQ handler */
void Qprc_IrqHandler ( volatile stc_qprcn_t* pstcQprc,
                       stc_qprc_intern_data_t* pstcQprcInternData );
#ifdef __cplusplus
}
#endif

//@} // QprcGroup

#endif // #if (defined(PDL_PERIPHERAL_QPRC_ACTIVE))

#endif /* __QPRC_H__ */
/******************************************************************************/
/* EOF (not truncated)                                                        */
/******************************************************************************/
