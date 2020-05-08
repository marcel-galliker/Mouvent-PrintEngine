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
/** \file reset.c
 **
 ** A detailed description is available at 
 ** @link ResetGroup Reset Cause description @endlink
 **
 ** History:
 **   - 2013-04-09  1.0  MWi  First version.
 **
 ******************************************************************************/

/******************************************************************************/
/* Include files                                                              */
/******************************************************************************/
#include "reset.h"
#include <string.h>

#if (defined(PDL_PERIPHERAL_RESET_ACTIVE))

/**
 ******************************************************************************
 ** \addtogroup ResetGroup
 ******************************************************************************/
//@{

/******************************************************************************/
/* Local pre-processor symbols/macros ('#define')                             */
/******************************************************************************/

/******************************************************************************/
/* Global variable definitions (declared in header file with 'extern')        */
/******************************************************************************/
stc_reset_result_t stcStoredResetCause; ///< Global reset cause register
  

/******************************************************************************/
/* Local type definitions ('typedef')                                         */
/******************************************************************************/

/******************************************************************************/
/* Local function prototypes ('static')                                       */
/******************************************************************************/

/******************************************************************************/
/* Local variable definitions ('static')                                      */
/******************************************************************************/

/******************************************************************************/
/* Function implementation - global ('extern') and local ('static')           */
/******************************************************************************/

/**
 ******************************************************************************
 ** \brief Get current Reset Cause Register contents
 **
 ** This function reads the Reset Cause Register and stores the cause bits
 ** in the result structure pointer.
 **
 ** \attention Calling this function clears all bits in the Reset Cause Register
 **            RST_STR! Reset_GetCause() should only be called after Start-up
 **            code!
 **
 ** \param [out]  pstcResult     Reset Cause result structure
 **
 ** \retval Ok                   Function done sucessfully
 **
 ******************************************************************************/
en_result_t Reset_GetCause(stc_reset_result_t* pstcResult)
{
  stc_crg_rst_str_field_t stcReadResetCause;
  
  stcReadResetCause = FM4_CRG->RST_STR_f;

  // Check Power-on bit
  if (1 == stcReadResetCause.PONR)
  {
    pstcResult->bPowerOn = TRUE;
  }
  else
  {
    pstcResult->bPowerOn = FALSE;
  }
  
  // Check INITX/External Reset bit
  if (1 == stcReadResetCause.INITX)
  {
    pstcResult->bInitx = TRUE;
  }
  else
  {
    pstcResult->bInitx = FALSE;
  }
  
  // Check Low Voltage Detection bit
  if (1 == stcReadResetCause.LVDH)
  {
    pstcResult->bLowVoltageDetection = TRUE;
  }
  else
  {
    pstcResult->bLowVoltageDetection = FALSE;
  }
  
  // Check Software Watchdog bit
  if (1 == stcReadResetCause.SWDT)
  {
    pstcResult->bSoftwareWatchdog = TRUE;
  }
  else
  {
    pstcResult->bSoftwareWatchdog = FALSE;
  }
  
  // Check Hardware Watchdog bit
  if (1 == stcReadResetCause.HWDT)
  {
    pstcResult->bHardwareWatchdog = TRUE;
  }
  else
  {
    pstcResult->bHardwareWatchdog = FALSE;
  }
  
  // Check Clock Supervisor bit
  if (1 == stcReadResetCause.CSVR)
  {
    pstcResult->bClockSupervisor = TRUE;
  }
  else
  {
    pstcResult->bClockSupervisor = FALSE;
  }
  
  // Check Anomalous Frequency bit
  if (1 == stcReadResetCause.FCSR)
  {
    pstcResult->bAnomalousFrequency = TRUE;
  }
  else
  {
    pstcResult->bAnomalousFrequency = FALSE;
  }
  
  // Check Software Reset bit
  if (1 == stcReadResetCause.SRST)
  {
    pstcResult->bSoftware = TRUE;
  }
  else
  {
    pstcResult->bSoftware = FALSE;
  }

  // Copy reset cause structure argument contents to global reset cause structure
  memcpy(&stcStoredResetCause, pstcResult, sizeof(stcStoredResetCause));
  
  return Ok;
} // Reset_GetCause

/**
 ******************************************************************************
 ** \brief Get stored Reset Cause
 **
 ** This function reads the gloabal Reset Cause Variable
 **
 ** \pre Reset_GetCause() must be called before.
 **
 ** \param [out]  pstcResult     Reset Cause result structure
 **
 ** \retval Ok                   Function done sucessfully
 **
 ******************************************************************************/
en_result_t Reset_GetStoredCause( stc_reset_result_t* pstcResult )
{
  // Copy global reset cause structure contents to reset cause structure argument 
  memcpy(pstcResult, &stcStoredResetCause, sizeof(stcStoredResetCause));
  
  return Ok;
}

//@} // ResetGroup

#endif // #if (defined(PDL_PERIPHERAL_RESET_ACTIVE))

/******************************************************************************/
/* EOF (not truncated)                                                        */
/******************************************************************************/
