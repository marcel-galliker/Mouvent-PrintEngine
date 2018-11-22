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
/** \file lpm.h
 **
 ** Headerfile for LPM functions
 **  
 ** History:
 **   - 2013-10-23  1.0  MWi  First version.
 **
 ******************************************************************************/

#ifndef __LPM_H__
#define __LPM_H__

/******************************************************************************/
/* Include files                                                              */
/******************************************************************************/
#include "pdl.h"

#if (defined(PDL_PERIPHERAL_LPM_ACTIVE))

/* C binding of definitions if building with C++ compiler */
#ifdef __cplusplus
extern "C"
{
#endif
  
/**
 ******************************************************************************
 ** \defgroup LpmGroup Low Power Modes Functions (LPM)
 **
 ** \note This driver module requires the CLK and EXINT module to be activated.
 **       For Back-up Register usage only, no additional module has to be
 **       activated
 **
 ** Provided functions of LPM module:
 ** - Lpm_SetHsCrSleep()
 ** - Lpm_SetMainSleep()
 ** - Lpm_SetPllSleep()
 ** - Lpm_SetLsCrSleep()
 ** - Lpm_SetSubSleep()
 ** - Lpm_SetHsCrTimer()
 ** - Lpm_SetMainTimer()
 ** - Lpm_SetPllTimer()
 ** - Lpm_SetLsCrTimer()
 ** - Lpm_SetSubTimer()
 ** - Lpm_SetRtcMode()
 ** - Lpm_SetStopMode()
 ** - Lpm_SetDeepRtcMode()
 ** - Lpm_SetDeepStopMode()
 ** - Lpm_ReadBackupRegisters()
 ** - Lpm_WriteBackupRegisters()
 ** - Lpm_Read_u8DataBackupRegister()
 ** - Lpm_Write_u8DataBackupRegister()
 ** - Lpm_Read_u16DataBackupRegister()
 ** - Lpm_Write_u16DataBackupRegister()
 ** - Lpm_Read_u32DataBackupRegister()
 ** - Lpm_Write_u32DataBackupRegister()
 ** <br>
 **
 ** Lpm_WriteBackupRegisters() writes a structure of the type of
 ** #stc_backupreg_t to the back-up registers and Lpm_ReadBackupRegisters()
 ** reads them out to this structure.<br>
 ** Lpm_Read_u8DataBackupRegister() and Lpm_Write_u8DataBackupRegister()
 ** accesses a single byte of the back-up registers.<br>
 ** Lpm_Read_u16DataBackupRegister() and Lpm_Write_u16DataBackupRegister()
 ** accesses 16-Bit data of the back-up registers.<br>
 ** Lpm_Read_u32DataBackupRegister() and Lpm_Write_u32DataBackupRegister()
 ** accesses 32-Bit data of the back-up registers.
 **
 ** <br>
 ** The following functions are only available, if CLK and EXINT modules
 ** are activated:<br><br>
 ** <b>Sleep Mode Transition Functions:</b>
 ** <table border=0 cellspacing=0 cellpadding=0>
 ** <tr><td>Lpm_SetHsCrSleep()</td><td>&nbsp;&nbsp;High-Speec CR Clock</td></tr>
 ** <tr><td>Lpm_SetMainSleep()</td><td>&nbsp;&nbsp;Main Clock</td></tr>
 ** <tr><td>Lpm_SetPllSleep()</td><td>&nbsp;&nbsp;PLL Clock</td></tr>
 ** <tr><td>Lpm_SetLsCrSleep()</td><td>&nbsp;&nbsp;Low-Speed CR Clock</td></tr>
 ** <tr><td>Lpm_SetSubSleep()</td><td>&nbsp;&nbsp;Sub Clock</td></tr>
 ** </table><br>
 **
 ** <b>Timer Mode Transition Functions:</b>
 ** <table border=0 cellspacing=0 cellpadding=0>
 ** <tr><td>Lpm_SetHsCrTimer()</td><td>&nbsp;&nbsp;High-Speec CR Clock</td></tr>
 ** <tr><td>Lpm_SetMainTimer()</td><td>&nbsp;&nbsp;Main Clock</td></tr>
 ** <tr><td>Lpm_SetPllTimer()</td><td>&nbsp;&nbsp;PLL Clock</td></tr>
 ** <tr><td>Lpm_SetLsCrTimer()</td><td>&nbsp;&nbsp;Low-Speed CR Clock</td></tr>
 ** <tr><td>Lpm_SetSubTimer()</td><td>&nbsp;&nbsp;Sub Clock</td></tr>
 ** </table><br>
 **
 ** <b>Stop Mode Transition Functions:</b>
 ** <table border=0 cellspacing=0 cellpadding=0>
 ** <tr><td>Lpm_SetRtcMode()</td><td>&nbsp;&nbsp;RTC Stop Mode
 ** <tr><td>Lpm_SetStopMode()</td><td>&nbsp;&nbsp;Stop Mode
 ** </table><br>
 **
 ** <b>Deep Standby Mode Transition Functions:</b>
 ** <table border=0 cellspacing=0 cellpadding=0>
 ** <tr><td>Lpm_SetDeepRtcMode()</td><td>&nbsp;&nbsp;Deep RTC Stop Mode
 ** <tr><td>Lpm_SetDeepStopMode()</td><td>&nbsp;&nbsp;Deep Stop Mode
 ** </table><br>
 **
 **
 ******************************************************************************/
//@{

/******************************************************************************/
/* Global pre-processor symbols/macros ('#define')      
 ******************************************************************************/
   
// 8-Bit Backup Register address offset
#define PDL_u8BREG00    0u
#define PDL_u8BREG01    1u
#define PDL_u8BREG02    2u
#define PDL_u8BREG03    3u
#define PDL_u8BREG04    4u
#define PDL_u8BREG05    5u
#define PDL_u8BREG06    6u
#define PDL_u8BREG07    7u
#define PDL_u8BREG08    8u
#define PDL_u8BREG09    9u
#define PDL_u8BREG0A   10u
#define PDL_u8BREG0B   11u
#define PDL_u8BREG0C   12u
#define PDL_u8BREG0D   13u
#define PDL_u8BREG0E   14u
#define PDL_u8BREG0F   15u
#define PDL_u8BREG10   16u
#define PDL_u8BREG11   17u
#define PDL_u8BREG12   18u
#define PDL_u8BREG13   19u
#define PDL_u8BREG14   20u
#define PDL_u8BREG15   21u
#define PDL_u8BREG16   22u
#define PDL_u8BREG17   23u
#define PDL_u8BREG18   24u
#define PDL_u8BREG19   25u
#define PDL_u8BREG1A   26u
#define PDL_u8BREG1B   27u
#define PDL_u8BREG1C   28u
#define PDL_u8BREG1D   29u
#define PDL_u8BREG1E   30u
#define PDL_u8BREG1F   31u
   
// 16-Bit Backup Register address offset
#define PDL_u16BREG00    0u
#define PDL_u16BREG02    2u
#define PDL_u16BREG04    4u
#define PDL_u16BREG06    6u
#define PDL_u16BREG08    8u
#define PDL_u16BREG0A   10u
#define PDL_u16BREG0C   12u
#define PDL_u16BREG0E   14u
#define PDL_u16BREG10   16u
#define PDL_u16BREG12   18u
#define PDL_u16BREG14   20u
#define PDL_u16BREG16   22u
#define PDL_u16BREG18   24u
#define PDL_u16BREG1A   26u
#define PDL_u16BREG1C   28u
#define PDL_u16BREG1E   30u
   
// 32-Bit Backup Register address offset
#define PDL_u32BREG00    0u
#define PDL_u32BREG04    4u
#define PDL_u32BREG08    8u
#define PDL_u32BREG0C   12u
#define PDL_u32BREG10   16u
#define PDL_u32BREG14   20u
#define PDL_u32BREG18   24u
#define PDL_u32BREG1C   28u
   
/******************************************************************************
 * Global type definitions
 ******************************************************************************/
 typedef struct stc_backupreg
 {
   uint8_t u8BREG00;
   uint8_t u8BREG01;
   uint8_t u8BREG02;
   uint8_t u8BREG03;
   uint8_t u8BREG04;
   uint8_t u8BREG05;
   uint8_t u8BREG06;
   uint8_t u8BREG07;
   uint8_t u8BREG08;
   uint8_t u8BREG09;
   uint8_t u8BREG0A;
   uint8_t u8BREG0B;
   uint8_t u8BREG0C;
   uint8_t u8BREG0D;
   uint8_t u8BREG0E;
   uint8_t u8BREG0F;
   uint8_t u8BREG10;
   uint8_t u8BREG11;
   uint8_t u8BREG12;
   uint8_t u8BREG13;
   uint8_t u8BREG14;
   uint8_t u8BREG15;
   uint8_t u8BREG16;
   uint8_t u8BREG17;
   uint8_t u8BREG18;
   uint8_t u8BREG19;
   uint8_t u8BREG1A;
   uint8_t u8BREG1B;
   uint8_t u8BREG1C;
   uint8_t u8BREG1D;
   uint8_t u8BREG1E;
   uint8_t u8BREG1F;
 } stc_backupreg_t;
 
/******************************************************************************/
/* Global variable definitions ('extern')                                     */
/******************************************************************************/


/******************************************************************************/
/* Global function prototypes (definition in C source)                        */
/******************************************************************************/
 
 #if (defined(PDL_PERIPHERAL_CLK_ACTIVE)) && (defined(PDL_PERIPHERAL_EXINT_ACTIVE))
 
extern en_result_t Lpm_SetHsCrSleep(boolean_t bDisablePllClock,
                                    boolean_t bDisableSubClock) ;

extern en_result_t Lpm_SetMainSleep(boolean_t bDisablePllClock,
                                    boolean_t bDisableSubClock) ;
 
extern en_result_t Lpm_SetPllSleep(boolean_t bDisableSubClock) ;

extern en_result_t Lpm_SetLsCrSleep(boolean_t bDisablePllMainClock,
                                    boolean_t bDisableSubClock) ;

extern en_result_t Lpm_SetSubSleep(boolean_t bDisablePllMainClock) ;

extern en_result_t Lpm_SetHsCrTimer(boolean_t bDisablePllMainClock,
                                              boolean_t bDisableSubClock) ;

extern en_result_t Lpm_SetMainTimer(boolean_t bDisablePllClock,
                                              boolean_t bDisableSubClock) ;

extern en_result_t Lpm_SetPllTimer(boolean_t bDisableSubClock) ;

extern en_result_t Lpm_SetLsCrTimer(boolean_t bDisablePllMainClock,
                                    boolean_t bDisableSubClock) ;

extern en_result_t Lpm_SetSubTimer(boolean_t bDisablePllMainClock) ;

extern en_result_t Lpm_SetRtcMode(void) ;

extern en_result_t Lpm_SetStopMode(void) ;

extern en_result_t Lpm_SetDeepRtcMode(void) ;

extern en_result_t Lpm_SetDeepStopMode(void) ;

#endif //  #if (defined(PDL_PERIPHERAL_CLK_ACTIVE)) && (defined(PDL_PERIPHERAL_EXINT_ACTIVE))

extern en_result_t Lpm_ReadBackupRegisters(stc_backupreg_t* stcBackUpReg) ;

extern en_result_t Lpm_WriteBackupRegisters(stc_backupreg_t* stcBackUpReg) ;

extern en_result_t Lpm_Read_u8DataBackupRegister(uint8_t u8AddressOffset, uint8_t* u8Data) ;

extern en_result_t Lpm_Write_u8DataBackupRegister(uint8_t u8AddressOffset, uint8_t u8Data) ;

extern en_result_t Lpm_Read_u16DataBackupRegister(uint8_t u8AddressOffset, uint16_t* u16Data) ;

extern en_result_t Lpm_Write_u16DataBackupRegister(uint8_t u8AddressOffset, uint16_t u16Data) ;

extern en_result_t Lpm_Read_u32DataBackupRegister(uint8_t u8AddressOffset, uint32_t* u32Data) ;

extern en_result_t Lpm_Write_u32DataBackupRegister(uint8_t u8AddressOffset, uint32_t u32Data) ;

//@} // LpmGroup

#ifdef __cplusplus
}
#endif

#endif // #if (defined(PDL_PERIPHERAL_LPM_ACTIVE))

#endif /* __LPM_H__ */
/******************************************************************************/
/* EOF (not truncated)                                                        */
/******************************************************************************/
