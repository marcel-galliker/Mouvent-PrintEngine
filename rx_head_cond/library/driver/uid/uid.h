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
/** \file uid.h
 **
 ** Headerfile for CLK functions
 **  
 ** History:
 **   - 2013-09-26  1.0  MWi  First version.
 **
 ******************************************************************************/

#ifndef __UID_H__
#define __UID_H__

/******************************************************************************/
/* Include files                                                              */
/******************************************************************************/
#include "pdl.h"

#if (defined(PDL_PERIPHERAL_UID_ACTIVE))

/* C binding of definitions if building with C++ compiler */
#ifdef __cplusplus
extern "C"
{
#endif
  
/**
 ******************************************************************************
 ** \defgroup UidGroup Unique ID Functions (UID)
 **
 ** Provided functions of UID module:
 ** - Uid_ReadUniqueId()
 ** - Uid_ReadUniqueId0()
 ** - Uid_ReadUniqueId1()
 ** - Uid_ReadUniqueId64()
 **
 ** Uid_ReadUniqueId() reads out the Unique ID to a pointered structure of the
 ** type #stc_unique_id_t. Uid_ReadUniqueId0() reads out the Unique ID 0
 ** register and aligns the result to the LSB. Uid_ReadUniqueId1() reads out the
 ** Unique ID 1 register. Both functions masks reverd bits to '0'.
 ** Uid_ReadUniqueId64() reads out the complet Unique ID registers aligns them
 ** to LSB and masks out reserved bits to '0'.
 **
 ******************************************************************************/
//@{

/******************************************************************************/
/* Global pre-processor symbols/macros ('#define')                            */
/******************************************************************************/

/******************************************************************************/
/* Global type definitions                                                    */
/******************************************************************************/

/**
 ******************************************************************************
 ** \brief PDL structure of Unique ID register set
 ******************************************************************************/  
 typedef struct stc_unique_id
 {
   uint32_t u32Uidr0;
   uint32_t u32Uidr1;
 } stc_unique_id_t;
 
/******************************************************************************/
/* Global variable definitions ('extern')                                     */
/******************************************************************************/ 
 
/******************************************************************************/
/* Global function prototypes (definition in C source)                        */
/******************************************************************************/

en_result_t Uid_ReadUniqueId(stc_unique_id_t *pstcUniqueId);
 
uint32_t Uid_ReadUniqueId0(void);
 
uint32_t Uid_ReadUniqueId1(void);

uint64_t Uid_ReadUniqueId64(void);
 
//@} // UidGroup

#ifdef __cplusplus
}
#endif

#endif // #if (defined(PDL_PERIPHERAL_UID_ACTIVE))

#endif /* __UID_H__ */

/******************************************************************************/
/* EOF (not truncated)                                                        */
/******************************************************************************/
