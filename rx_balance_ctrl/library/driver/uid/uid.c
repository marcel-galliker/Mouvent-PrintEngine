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
/** \file uid.c
 **
 ** A detailed description is available at 
 ** @link UidGroup Unique ID Module description @endlink
 **
 ** History:
 **   - 2013-09-26  1.0  MWi  First version.
 **
 ******************************************************************************/

/******************************************************************************/
/* Include files                                                              */
/******************************************************************************/
#include "uid.h"

#if (defined(PDL_PERIPHERAL_UID_ACTIVE))

/**
 ******************************************************************************
 ** \addtogroup UidGroup
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
 ** \brief Read Unique ID registers as is
 **
 ** This function reads out UIDR0 and UIDR1 as is without any shift to
 ** a pointered structure of the type stc_unique_id_t.
 ** Reserved bits are masked to '0'.
 **
 ** \param  [out] pstcUniqueId     Pointer to the Unique ID strucutre
 **
 ** \retval Ok                     Unique ID sucessfully written
 ** \retval ErrorInvalidParameter  pstcUniqueId == NULL
 ******************************************************************************/
en_result_t Uid_ReadUniqueId(stc_unique_id_t *pstcUniqueId)
{
  if (NULL == pstcUniqueId)
  {
    return ErrorInvalidParameter;
  }
  
  pstcUniqueId->u32Uidr0 = (0xFFFFFFF0u & (FM4_UNIQUE_ID->UIDR0));
  pstcUniqueId->u32Uidr1 = (0x00001FFFu & (FM4_UNIQUE_ID->UIDR1));
  
  return Ok;
}
   
/**
 ******************************************************************************
 ** \brief Read Unique ID registers 0 and shifts it by 4 (LSB aligned)
 **
 ** This function reads out UIDR0 and aligns the value to the LSB by 
 ** right-shifting by 4.
 **
 ** \return uint32_t                  Unique ID 0 >> 4
 ******************************************************************************/
uint32_t Uid_ReadUniqueId0(void)
{
  return ((FM4_UNIQUE_ID->UIDR0) >> 4u);
}

/**
 ******************************************************************************
 ** \brief Read Unique ID registers 1 and masks the upper 19 bits to '0'
 **
 ** This function reads out UIDR1 and masks the upper 19 bits to '0'
 **
 ** \return uint32_t                  Unique ID 1 & 0x00001FFF
 ******************************************************************************/
uint32_t Uid_ReadUniqueId1(void)
{
  return ((FM4_UNIQUE_ID->UIDR1) & 0x00001FFFu);
}

/**
 ******************************************************************************
 ** \brief Read Unique ID registers 0 and 1 and merge it LSB aligned to a 64 bit
 ** value
 **
 ** \return uint64_t                  Complete unique ID LSB aligned
 ******************************************************************************/
uint64_t Uid_ReadUniqueId64(void)
{
  uint64_t u64UniqueIdMsw;
  
  // Left shift by 60 bit in two steps
  u64UniqueIdMsw = (((FM4_UNIQUE_ID->UIDR1) & 0x00001FFFul) << 16ul);
  u64UniqueIdMsw <<= 12ul;
  
  u64UniqueIdMsw |= ((FM4_UNIQUE_ID->UIDR0) >> 4ul);
  return u64UniqueIdMsw;
}

//@} // UidGroup

#endif // #if (defined(PDL_PERIPHERAL_UID_ACTIVE))

/******************************************************************************/
/* EOF (not truncated)                                                        */
/******************************************************************************/
