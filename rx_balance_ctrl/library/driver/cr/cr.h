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
/** \file cr.h
 **
 ** Headerfile for CR functions
 **  
 **
 ** History:
 **   - 2013-10-11  1.0  Edison Zhang   First version.
 **
 ******************************************************************************/

#ifndef __CR_H__
#define __CR_H__

/******************************************************************************/
/* Include files                                                              */
/******************************************************************************/
#include "pdl.h"

#if (defined(PDL_PERIPHERAL_CR_ACTIVE))

/* C binding of definitions if building with C++ compiler */
#ifdef __cplusplus
extern "C"
{
#endif

/**
 ******************************************************************************
 ** \defgroup CrGroup Cr Functions (CR)
 **
 ** Provided functions of CR module:
 ** 
 ** - Cr_SetFreqDiv()
 ** - Cr_SetTempTrimmingData()
 ** - Cr_GetTempTrimmingData()
 ** - Cr_SetFreqTrimmingData()
 ** - Cr_GetFreqTrimmingData() 
 **
 ** Cr_SetFreqDiv() is used to divde CR input signal, which outputs to base 
 ** timer from dividor output.
 ** Cr_SetTempTrimmingData() sets the CR temperature trimming data, and 
 ** Cr_GetTempTrimmingData() gets the CR temperature trimming data.
 ** Cr_SetFreqTrimmingData() sets the CR frequency trimming data, and 
 ** Cr_GetFreqTrimmingData() gets the CR frequency trimming data.  
 **
 ******************************************************************************/
//@{

/******************************************************************************
 * Global definitions
 ******************************************************************************/
  
/******************************************************************************
 * Global type definitions
 ******************************************************************************/
typedef enum en_cr_freq_div
{
    CrFreqDivBy4 = 0,  ///< CR output (to Base Timer) prescaler: 4
    CrFreqDivBy8,      ///< CR output (to Base Timer) prescaler: 8
    CrFreqDivBy16,     ///< CR output (to Base Timer) prescaler: 16
    CrFreqDivBy32,     ///< CR output (to Base Timer) prescaler: 32
    CrFreqDivBy64,     ///< CR output (to Base Timer) prescaler: 64
    CrFreqDivBy128,    ///< CR output (to Base Timer) prescaler: 128
    CrFreqDivBy256,    ///< CR output (to Base Timer) prescaler: 256
    CrFreqDivBy512,    ///< CR output (to Base Timer) prescaler: 512
  
}en_cr_freq_div_t;
   
/******************************************************************************/
/* Global variable definitions ('extern')                                     */
/******************************************************************************/
/******************************************************************************/
/* Global function prototypes (definition in C source)                        */
/******************************************************************************/
en_result_t Cr_SetFreqDiv(en_cr_freq_div_t enCrDiv);
en_result_t Cr_SetTempTrimmingData(uint8_t u8Data);
uint8_t Cr_GetTempTrimmingData(void);
en_result_t Cr_SetFreqTrimmingData(uint16_t u16Data);
uint16_t Cr_GetFreqTrimmingData(void);

//@} // CrGroup

#ifdef __cplusplus
}
#endif

#endif /* #if (defined(PDL_PERIPHERAL_CR_ACTIVE)) */

#endif /* __CR_H__ */
/******************************************************************************/
/* EOF (not truncated)                                                        */
/******************************************************************************/
