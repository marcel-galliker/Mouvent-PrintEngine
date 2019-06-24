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
/** \file dac.h
 **
 ** Headerfile for DAC functions
 **  
 ** History:
 **   - 2013-03-22  1.0  MWi  First version.
 **
 ******************************************************************************/

#ifndef __DAC_H__
#define __DAC_H__

/******************************************************************************/
/* Include files                                                              */
/******************************************************************************/
#include "pdl.h"

#if (defined(PDL_PERIPHERAL_DAC_ACTIVE))

/* C binding of definitions if building with C++ compiler */
#ifdef __cplusplus
extern "C"
{
#endif

/**
 ******************************************************************************
 ** \defgroup DacGroup 12-Bit Digital Analog Converter (DAC)
 **
 ** Provided functions of DAC module:
 ** - Dac_Init()
 ** - Dac_DeInit()
 ** - Dac_SetValue0()
 ** - Dac_SetValue1()
 ** - Dac_Enable0()
 ** - Dac_Enable1()
 ** - Dac_Disable0()
 ** - Dac_Disable1()
 **
 ** With Dac_Init() the DAC is configured (10/12-Bit mode, 10-Bit data
 ** alignment). Each channel can be en- and disabled by e.g. Dac_Enable0(),
 ** Dac_Disable0(). Output values can be set easily by Dac_SetValue0() and
 ** Dac_SetValue1().
 **
 ******************************************************************************/
//@{

/******************************************************************************/
/* Global pre-processor symbols/macros ('#define')                            */
/******************************************************************************/
 
/******************************************************************************
 * Global type definitions
 ******************************************************************************/
#define stc_dacn_t   FM4_DAC_TypeDef
#define DAC0  (*(( stc_dacn_t *) FM4_DAC_BASE))
  
/**
 ******************************************************************************
 ** \brief DAC configuration.
 ******************************************************************************/
typedef struct stc_dac_config
{
  boolean_t bDac12Bit0;            ///< Channel 0: TRUE: 12-Bit mode, FALSE: 10-Bit mode
  boolean_t bDac10RightAlign0;     ///< Channel 0: TRUE: 10-Bit data aligned to DA[9:0], FLASE: 10-Bit data aligned to DA[11:2]
  boolean_t bDac12Bit1;            ///< Channel 1: TRUE: 12-Bit mode, FALSE: 10-Bit mode
  boolean_t bDac10RightAlign1;     ///< Channel 1: TRUE: 10-Bit data aligned to DA[9:0], FLASE: 10-Bit data aligned to DA[11:2]
} stc_dac_config_t;

/******************************************************************************/
/* Global variable definitions ('extern')                                     */
/******************************************************************************/

/******************************************************************************/
/* Global function prototypes (definition in C source)                        */
/******************************************************************************/
extern en_result_t Dac_Init( stc_dacn_t*       pstcDac,
                             stc_dac_config_t* pstcConfig
                           );

extern en_result_t Dac_DeInit( stc_dacn_t* pstcDac );

extern en_result_t Dac_SetValue0( stc_dacn_t* pstcDac,
                                  uint16_t    u16DacValue
                                );

extern en_result_t Dac_SetValue1( stc_dacn_t* pstcDac,
                                  uint16_t    u16DacValue
                                );

extern en_result_t Dac_Enable0( stc_dacn_t* pstcDac );

extern en_result_t Dac_Enable1( stc_dacn_t* pstcDac );

extern en_result_t Dac_Disable0( stc_dacn_t* pstcDac );

extern en_result_t Dac_Disable1( stc_dacn_t* pstcDac );

//@} // DacGroup

#ifdef __cplusplus
}
#endif

#endif // #if (defined(PDL_PERIPHERAL_DAC_ACTIVE))

#endif /* __DAC_H__ */
/******************************************************************************/
/* EOF (not truncated)                                                        */
/******************************************************************************/

