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
/** \file reset.h
 **
 ** Headerfile for RESET FACTOR functions
 **  
 ** History:
 **   - 2013-04-09  1.0  MWi  First version.
 **
 ******************************************************************************/

#ifndef __RESET_H__
#define __RESET_H__

/******************************************************************************/
/* Include files                                                              */
/******************************************************************************/
#include "pdl.h"

#if (defined(PDL_PERIPHERAL_RESET_ACTIVE))

/* C binding of definitions if building with C++ compiler */
#ifdef __cplusplus
extern "C"
{
#endif

/**
 ******************************************************************************
 ** \defgroup ResetGroup Reset Factor (RESET)
 **
 ** Provided functions of RESET module:
 ** - Reset_GetCause()
 ** - Reset_GetStoredCause()
 **
 ** Reset_GetCause() reads the Reset Casue, clears the HW register, and stores
 ** the result in a global variable. This is needed, if the SystemInit function
 ** uses this function for retrieving the reset cause (for RTC, etc.), but the
 ** application itself also needs the reset cause.
 ** Reset_GetStoredCause() reads out the global reset cause variable, without
 ** touching the reset cause register. Reset_GetCause() must be called before.
 ** 
 ** This driver does not need any configuration structure, but uses a result
 ** structure #stc_reset_result_t, which has to be provided by the caller.
 **
 ******************************************************************************/
//@{

/******************************************************************************/
/* Global pre-processor symbols/macros ('#define')                            */
/******************************************************************************/
 
/******************************************************************************
 * Global type definitions
 ******************************************************************************/
  
/**
 ******************************************************************************
 ** \brief Datatype for Reset Cause return structure
 ******************************************************************************/
typedef struct stc_reset_result
{
  boolean_t    bPowerOn;              ///< TRUE: Power on reset occurred
  boolean_t    bInitx;                ///< TRUE: INITX (external) reset occurred
  boolean_t    bLowVoltageDetection;  ///< TRUE: Low Voltage Detection reset ocucrred (only applicable for Type3 and 7, always FALSE otherwise)
  boolean_t    bSoftwareWatchdog;     ///< TRUE: Software Watchdog reset occurred
  boolean_t    bHardwareWatchdog;     ///< TRUE: Hardware Watchdog reset occurred
  boolean_t    bClockSupervisor;      ///< TRUE: Clock Supervisor reset occurred
  boolean_t    bAnomalousFrequency;   ///< TRUE: Anomalous Frequency reset occurred
  boolean_t    bSoftware;             ///< TRUE: Software reset occurred
} stc_reset_result_t ;

/******************************************************************************/
/* Global variable definitions ('extern')                                     */
/******************************************************************************/

/******************************************************************************/
/* Global function prototypes (definition in C source)                        */
/******************************************************************************/

extern en_result_t Reset_GetCause( stc_reset_result_t* pstcResult ) ;
extern en_result_t Reset_GetStoredCause( stc_reset_result_t* pstcResult );

//@} // ResetGroup

#ifdef __cplusplus
}
#endif

#endif // #if (defined(PDL_PERIPHERAL_RESET_ACTIVE))

#endif /* __RESET_H__ */
/******************************************************************************/
/* EOF (not truncated)                                                        */
/******************************************************************************/
