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
/** \file csv.h
 **
 ** Headerfile for CSV functions
 **  
 **
 ** History:
 **   - 2013-10-11  1.0  Edison Zhang   First version.
 **
 ******************************************************************************/
 
#ifndef _CSV_H_
#define _CSV_H_

/******************************************************************************/
/* Include files                                                              */
/******************************************************************************/
#include "pdl.h"

#if (defined(PDL_PERIPHERAL_CSV_ACTIVE))

/**
 ******************************************************************************
 ** \defgroup CsvGroup Clock Supervisor Functions (CSV)
 **
 ** Provided functions of CSV module:
 ** 
 ** - Csv_EnableMainCsv()
 ** - Csv_DisableMainCsv()
 ** - Csv_EnableSubCsv()
 ** - Csv_DisableSubCsv()
 ** - Csv_GetCsvFailCause()  
 ** - Csv_EnableFcs() 
 ** - Csv_DisableFcs() 
 ** - Csv_EnableFcsReset() 
 ** - Csv_DisableFcsReset() 
 ** - Csv_EnableFcsInt() 
 ** - Csv_DisableFcsInt() 
 ** - Csv_ClrFcsIntFlag() 
 ** - Csv_GetFcsIntFlag() 
 ** - Csv_SetFcsCrDiv()
 ** - Csv_SetFcsDetectRange()
 ** - Csv_GetFcsDetectCount()  
 **
 ** \brief What is CSV?
 ** The CSV module includes CSV and FCS function. CSV uses the high and low
 ** CR to monitor main and sub clock, if the abnormal state is detected, a 
 ** reset occurs. FCS uses high-speed CR to monitor the main clock, a range 
 ** can be set in advance. If the monitor cycle exeeds the range, either 
 ** interrupt or reset will occurs.
 **  
 ** \brief How to use CSV module with the APIs provided?
 ** First, Enable the CSV function with Csv_EnableMainCsv() or Csv_EnableSubCsv()
 ** When the abnormal status is detected, a CSV reset occurs, then read the 
 ** the CSV failure cause by Csv_GetCsvFailCause().
 ** Disable the CSV function with Csv_DisableMainCsv() or Csv_DisableSubCsv()
 **  
 ** \brief How to use FCS module with the APIs provided?
 ** First, set the CR dividor with Csv_SetFcsCrDiv() and set the expected range
 ** of main clok cycle with Csv_SetFcsDetectRange().
 ** Second, enable the FCS interrupt with Csv_EnableFcsInt() or enable FCS
 ** reset with Csv_EnableFcsReset(). 
 ** Then start FCS function with Csv_EnableFcs().
 ** When abnormal frequency is detected, an interrupt occurs when FCS interrupt
 ** is enabled, then read the interrupt flag by Csv_GetFcsIntFlag() and 
 ** clear the interrupt flag by Csv_ClrFcsIntFlag().
 ** When abnormal frequency is detected, a reset issues when FCS reset is
 ** enabled.
 ** When abnormal frequency is detected, current main clock cycle can be read
 ** by Csv_GetFcsDetectCount().
 ** Disable FCS by Csv_DisableFcs(), disable FCS reset by Csv_DisableFcsReset()
 ** and disable FCS interrupt by Csv_DisableFcsInt().
 **
 ******************************************************************************/
//@{
/******************************************************************************
 * Global type definitions
 ******************************************************************************/
/**
 ******************************************************************************
 ** \brief Enumeration to define an index for each BT instance
 ******************************************************************************/     
typedef enum en_fcs_cr_div
{
    FcsCrDiv256 = 5,        ///< 1/256 frequency of high-speed CR oscillation   
    FcsCrDiv512 = 6,        ///< 1/512 frequency of high-speed CR oscillation   
    FcsCrDiv1024 = 7,       ///< 1/1024 frequency of high-speed CR oscillation
    
}en_fcs_cr_div_t;

/**
 ******************************************************************************
 ** \brief Structure of CSV status
 ******************************************************************************/
typedef struct stc_csv_status
{
    boolean_t bCsvMainClockStatus;
    boolean_t bCsvSubClockStatus;
    
}stc_csv_status_t;

/**
 ******************************************************************************
 ** \brief FCS interrupt callback function type
 ******************************************************************************/
typedef void fn_fcs_int_callback(void);

/******************************************************************************/
/* Global function prototypes (definition in C source)                        */
/******************************************************************************/
/* C binding of definitions if building with C++ compiler */
#ifdef __cplusplus
extern "C" {
#endif
/* CSV */  
void Csv_EnableMainCsv(void);
void Csv_DisableMainCsv(void);
void Csv_EnableSubCsv(void);
void Csv_DisableSubCsv(void);
uint8_t Csv_GetCsvFailCause(stc_csv_status_t* pstcCsvStatus);
/* FCS */  
void Csv_EnableFcs(void);
void Csv_DisableFcs(void);
void Csv_EnableFcsReset(void);
void Csv_DisableFcsReset(void);
en_result_t Csv_EnableFcsInt(fn_fcs_int_callback* pfnIntCallback);
void Csv_DisableFcsInt(void);
void Csv_ClrFcsIntFlag(void);
uint8_t Csv_GetFcsIntFlag(void);
en_result_t Csv_SetFcsCrDiv(en_fcs_cr_div_t enDiv) ;
void Csv_SetFcsDetectRange(uint16_t u16LowerVal, uint16_t u16UpperVal);
uint16_t Csv_GetFcsDetectCount(void);
/* IRQ */
void Csv_IrqHandler(void);
#ifdef __cplusplus
}
#endif

//@}

#endif

#endif /* _CLOCK_FM3_H_ */
/*****************************************************************************/
/* END OF FILE */
