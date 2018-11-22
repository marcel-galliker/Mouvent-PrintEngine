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
/** \file dac.c
 **
 ** A detailed description is available at 
 ** @link DacGroup 12-Bit Digital Analog Converter description @endlink
 **
 ** History:
 **   - 2013-04-05  1.0  MWi  First version.
 **
 ******************************************************************************/

/******************************************************************************/
/* Include files                                                              */
/******************************************************************************/
#include "dac.h"

#if (defined(PDL_PERIPHERAL_DAC_ACTIVE))

/**
 ******************************************************************************
 ** \addtogroup DacGroup
 ******************************************************************************/
//@{

/******************************************************************************/
/* Local pre-processor symbols/macros ('#define')                             */
/******************************************************************************/

/******************************************************************************/
/* Global variable definitions (declared in header file with 'extern')        */
/******************************************************************************/

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
 ** \brief DAC Initialization
 **
 ** \param [in]  pstcDac           Pointer to DAC instance
 ** \param [in]  pstcConfig        Pointer to DAC configuration strucutre
 **
 ** \retval Ok                     DAC Instance sucessfully initialized
 ** \retval ErrorInvalidParameter  pstcDac == NULL or pstcConfig == NULL
 ******************************************************************************/
en_result_t Dac_Init( stc_dacn_t*       pstcDac,
                      stc_dac_config_t* pstcConfig
                    )
{
    // Check for NULL pointers
    if ((pstcDac    == NULL) ||
        (pstcConfig == NULL)
       )
    {
      return ErrorInvalidParameter;
    }      
    
    pstcDac->DACR0_f.DAC10 = (TRUE == pstcConfig->bDac12Bit0)        ? 0u : 1u ;
    pstcDac->DACR0_f.DAC10 = (TRUE == pstcConfig->bDac10RightAlign0) ? 1u : 0u ;
    pstcDac->DACR1_f.DAC10 = (TRUE == pstcConfig->bDac12Bit1)        ? 0u : 1u ;
    pstcDac->DACR1_f.DAC10 = (TRUE == pstcConfig->bDac10RightAlign1) ? 1u : 0u ;
    
    return Ok;
} // Dac_Init

/**
 ******************************************************************************
 ** \brief DAC De-Initialization
 **
 ** \param [in]  pstcDac           Pointer to DAC instance
 **
 ** \retval Ok                     DAC Instance sucessfully de-initialized
 ** \retval ErrorInvalidParameter  pstcDac == NULL or pstcConfig == NULL
 ******************************************************************************/
en_result_t Dac_DeInit( stc_dacn_t* pstcDac )
{
    // Check for NULL pointers
    if (pstcDac == NULL)
    {
      return ErrorInvalidParameter;
    }      
    
    pstcDac->DACR0 = 0u;
    pstcDac->DADR0 = 0u;
    pstcDac->DACR1 = 0u;
    pstcDac->DADR1 = 0u;
    
    return Ok;
} // Dac_DeInit

/**
 ******************************************************************************
 ** \brief Set DAC0 12-bit value
 **
 ** \param [in]  pstcDac        Pointer to DAC instance
 ** \param [in]  u16DacValue    DAC0 value to be output
 **
 ** \retval Ok                  Value written.
 ******************************************************************************/
en_result_t Dac_SetValue0( stc_dacn_t* pstcDac,
                           uint16_t    u16DacValue
                         )
{
  pstcDac->DADR0_f.DA = u16DacValue;
  
  return Ok;
} // Dac_SetValue0


/**
 ******************************************************************************
 ** \brief Set DAC1 12-bit value
 **
 ** \param [in]  pstcDac        Pointer to DAC instance
 ** \param [in]  u16DacValue    DAC1 value to be output
 **
 ** \retval Ok                  Value written.
 ******************************************************************************/
en_result_t Dac_SetValue1( stc_dacn_t* pstcDac,
                           uint16_t    u16DacValue
                         )
{
  pstcDac->DADR1_f.DA = u16DacValue;
  
  return Ok;
} // Dac_SetValue1

/**
 ******************************************************************************
 ** \brief Enable DAC0
 **
 ** \param [in]  pstcDac        Pointer to DAC instance
 **
 ** \retval Ok                  DAC0 enabled
 ******************************************************************************/
en_result_t Dac_Enable0( stc_dacn_t* pstcDac )
{
  pstcDac->DACR0_f.DAE = 1u;
  
  return Ok;
} // Dac_Enable0

/**
 ******************************************************************************
 ** \brief Enable DAC1
 **
 ** \param [in]  pstcDac        Pointer to DAC instance
 **
 ** \retval Ok                  DAC1 enabled
 ******************************************************************************/
en_result_t Dac_Enable1( stc_dacn_t* pstcDac )
{
  pstcDac->DACR1_f.DAE = 1u;
  
  return Ok;
} // Dac_Enable1


/**
 ******************************************************************************
 ** \brief Disable DAC0
 **
 ** \param [in]  pstcDac        Pointer to DAC instance
 **
 ** \retval Ok                  DAC0 disabled
 ******************************************************************************/
en_result_t Dac_Disable0( stc_dacn_t* pstcDac )
{
  pstcDac->DACR0_f.DAE = 0u;
  
  return Ok;
} // Dac_Disable0

/**
 ******************************************************************************
 ** \brief Disable DAC1
 **
 ** \param [in]  pstcDac        Pointer to DAC instance
 **
 ** \retval Ok                  DAC1 disabled
 ******************************************************************************/
en_result_t Dac_Disable1( stc_dacn_t* pstcDac )
{
  pstcDac->DACR1_f.DAE = 0u;
  
  return Ok;
} // Dac_Disable1


//@} // DacGroup

#endif // #if (defined(PDL_PERIPHERAL_DAC_ACTIVE))

/******************************************************************************/
/* EOF (not truncated)                                                        */
/******************************************************************************/
