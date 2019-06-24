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
/** \file lpm.c
 **
 ** A detailed description is available at 
 ** @link LpmGroup Low Power Modes Module description @endlink
 **
 ** History:
 **   - 2013-10-23  1.0  MWi  First version.
 **
 ******************************************************************************/

/******************************************************************************/
/* Include files                                                              */
/******************************************************************************/
#include "lpm.h"

#if (defined(PDL_PERIPHERAL_LPM_ACTIVE))

/**
 ******************************************************************************
 ** \addtogroup LpmGroup
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
 ** \brief Read Backup Registers
 **
 ** \param [out]  stcBackUpReg     double pointer to user backup register
 **                                structure
 **
 ** \retval Ok                     Successfully read
 ******************************************************************************/ 
static en_result_t ReadBackupRegisters(stc_backupreg_t* stcBackUpReg)
{
  uint8_t  u8Counter;

  // Read Command to VBAT domain
  FM4_RTC->WTCR20_f.BREAD = 1;
  
  // Poll for read completed
  while(1 == FM4_RTC->WTCR20_f.BREAD)
  {}
  
  for(u8Counter = 0u; u8Counter < PDL_BACK_UP_REGISTERS; u8Counter += 4u)
  {
    *(uint32_t*)((uint32_t)(&stcBackUpReg->u8BREG00) + (uint32_t)u8Counter)
      = *(uint32_t*)((uint32_t)(&FM4_RTC->BREG00_03) + u8Counter);
  }
  
  return Ok;
} // ReadBackupRegisters

/**
 ******************************************************************************
 ** \brief Write Backup Registers
 **
 ** \param [out]  stcBackUpReg     structure of backup registers
 **
 ** \retval Ok                     Successfully written
 ******************************************************************************/ 
static en_result_t WriteBackupRegisters(stc_backupreg_t* stcBackUpReg)
{
  uint8_t u8Counter;
  
  for(u8Counter = 0u; u8Counter < PDL_BACK_UP_REGISTERS; u8Counter += 4u)
  {
    *(uint32_t*)((uint32_t)(&FM4_RTC->BREG00_03) + u8Counter)
      = *(uint32_t*)((uint32_t)(&stcBackUpReg->u8BREG00) + u8Counter);
  }
  
  // Write Command to VBAT domain
  FM4_RTC->WTCR20_f.BWRITE = 1;
  
  // Poll for read completed
  while(1 == FM4_RTC->WTCR20_f.BWRITE)
  {}
  
  return Ok;
} // WriteBackupRegisters

// The following code is only compiled, if CLK and EXINT modules are activated
#if (defined(PDL_PERIPHERAL_CLK_ACTIVE)) && (defined(PDL_PERIPHERAL_EXINT_ACTIVE))

/**
 ******************************************************************************
 ** \brief Enter HS-CR Sleep Mode
 **
 ** \param [in]  bDisablePllMainClock  TRUE: Disable PLL Clock and Main Clock
 ** \param [in]  bDisableSubClock      TRUE: Disable Sub Clock
 **
 ** \retval Ok                         Returned from HS-CR Sleep Mode
 ******************************************************************************/ 
en_result_t Lpm_SetHsCrSleep(boolean_t bDisablePllMainClock,
                             boolean_t bDisableSubClock)
{
  stc_clk_main_config_t stcMainClockConfig;
  
  Clk_MainGetParameters(&stcMainClockConfig);
  
  Clk_SwitchToHsCrClock();
  
  if (TRUE == bDisablePllMainClock)
  {
    Clk_DisablePllClock();
    Clk_DisableMainClock();
  }
  
  if (TRUE == bDisableSubClock)
  {
    Clk_DisableSubClock();
  }
  
  stcMainClockConfig.enMode = ClkSleep;
  Clk_SetMode(&stcMainClockConfig);
  
  return Ok;
} // Lpm_SetHsCrSleep

/**
 ******************************************************************************
 ** \brief Enter Main Sleep Mode
 **
 ** \param [in]  bDisablePllClock    TRUE: Disable PLL Clock
 ** \param [in]  bDisableSubClock    TRUE: Disable Sub Clock
 **
 ** \retval Ok                       Returned from Main Sleep Mode
 ******************************************************************************/ 
en_result_t Lpm_SetMainSleep(boolean_t bDisablePllClock,
                             boolean_t bDisableSubClock)
{
  stc_clk_main_config_t stcMainClockConfig;
  
  Clk_MainGetParameters(&stcMainClockConfig);
  
  FM4_CRG->SCM_CTL_f.MOSCE = 1u;        // enable Main Clock
  Clk_SwitchToMainClock();
  
  if (TRUE == bDisablePllClock)
  {
    Clk_DisablePllClock();
  }
  
  if (TRUE == bDisableSubClock)
  {
    Clk_DisableSubClock();
  }
  
  stcMainClockConfig.enMode = ClkSleep;
  Clk_SetMode(&stcMainClockConfig);
  
  return Ok;
} // Lpm_SetMainSleep

/**
 ******************************************************************************
 ** \brief Enter PLL Sleep Mode
 **
 ** \pre The system has to be in stabilized PLL mode before calling this
 **      function
 ** 
 ** \param [in]  bDisableSubClock    TRUE: Disable Sub Clock
 **
 ** \retval Ok                       Returned from PLL Sleep Mode
 ** \retval ErrorNotReady            PLL clock not available
 ******************************************************************************/ 
en_result_t Lpm_SetPllSleep(boolean_t bDisableSubClock)
{
  stc_clk_main_config_t stcMainClockConfig;

  if((1u    != FM4_CRG->SCM_STR_f.PLRDY) ||     // PLL ready?
     (0x40u != (FM4_CRG->SCM_STR & 0xE0u))      // Main PLL clock?
    )
  {
    return ErrorNotReady;       // System not in PLL mode
  }
  
  Clk_MainGetParameters(&stcMainClockConfig);
  
  if (TRUE == bDisableSubClock)
  {
    Clk_DisableSubClock();
  }
  
  stcMainClockConfig.enMode = ClkSleep;
  Clk_SetMode(&stcMainClockConfig);
  
  return Ok;
} // Lpm_SetPllSleep

/**
 ******************************************************************************
 ** \brief Enter LS-CR Sleep Mode
 **
 ** \param [in]  bDisablePllMainClock  TRUE: Disable PLL Clock and Main Clock
 ** \param [in]  bDisableSubClock      TRUE: Disable Sub Clock
 **
 ** \retval Ok                         Returned from LS-CR Sleep Mode
 ******************************************************************************/ 
en_result_t Lpm_SetLsCrSleep(boolean_t bDisablePllMainClock,
                             boolean_t bDisableSubClock)
{
  stc_clk_main_config_t stcMainClockConfig;
  
  Clk_MainGetParameters(&stcMainClockConfig);
  
  Clk_SwitchToLsCrClock();
  
  if (TRUE == bDisablePllMainClock)
  {
    Clk_DisablePllClock();
    Clk_DisableMainClock();
  }
  
  if (TRUE == bDisableSubClock)
  {
    Clk_DisableSubClock();
  }
  
  stcMainClockConfig.enMode = ClkSleep;
  Clk_SetMode(&stcMainClockConfig);
  
  return Ok;
} // Lpm_SetLsCrSleep

/**
 ******************************************************************************
 ** \brief Enter Sub Sleep Mode
 **
 ** \param [in]  bDisablePllMainClock  TRUE: Disable PLL Clock and Main Clock
 **
 ** \retval Ok                         Returned from LS-CR Sleep Mode
 ** \retval ErrorNotReady              Sub clock not available
 ******************************************************************************/ 
en_result_t Lpm_SetSubSleep(boolean_t bDisablePllMainClock)
{
  stc_clk_main_config_t stcMainClockConfig;
  
  if(1u != FM4_CRG->SCM_STR_f.MORDY)
  {
    return ErrorNotReady;       // No sub clock available
  }
  
  Clk_MainGetParameters(&stcMainClockConfig);
  
  Clk_SwitchToSubClock();
  
  if (TRUE == bDisablePllMainClock)
  {
    Clk_DisablePllClock();
    Clk_DisableMainClock();
  }
  
  stcMainClockConfig.enMode = ClkSleep;
  Clk_SetMode(&stcMainClockConfig);
  
  return Ok;
} // Lpm_SetLsCrSleep

/**
 ******************************************************************************
 ** \brief Enter PLL Timer Mode
 **
 ** \pre The system has to be in stabilized PLL mode before calling this
 **      function
 ** 
 ** \param [in]  bDisableSubClock    TRUE: Disable Sub Clock
 **
 ** \retval Ok                       Returned from PLL Timer Mode
 ** \retval ErrorNotReady            PLL clock not available
 ******************************************************************************/ 
en_result_t Lpm_SetPllTimer(boolean_t bDisableSubClock)
{
  stc_clk_main_config_t stcMainClockConfig;

  if((1u    != FM4_CRG->SCM_STR_f.PLRDY) ||     // PLL ready?
     (0x40u != (FM4_CRG->SCM_STR & 0xE0u))      // Main PLL clock?
    )
  {
    return ErrorNotReady;       // System not in PLL mode
  }
  
  Clk_MainGetParameters(&stcMainClockConfig);
  
  if (TRUE == bDisableSubClock)
  {
    Clk_DisableSubClock();
  }
  
  stcMainClockConfig.enMode = ClkTimer;
  Clk_SetMode(&stcMainClockConfig);
  
  return Ok;
} // Lpm_SetPllTimer

/**
 ******************************************************************************
 ** \brief Enter HS-CR Timer Mode
 **
 ** \param [in]  bDisablePllMainClock  TRUE: Disable PLL Clock and Main Clock
 ** \param [in]  bDisableSubClock      TRUE: Disable Sub Clock
 **
 ** \retval Ok                         Returned from HS-CR Timer Mode
 ******************************************************************************/ 
en_result_t Lpm_SetHsCrTimer(boolean_t bDisablePllMainClock,
                             boolean_t bDisableSubClock)
{
  stc_clk_main_config_t stcMainClockConfig;
  
  Clk_MainGetParameters(&stcMainClockConfig);
  
  Clk_SwitchToHsCrClock();
  
  if (TRUE == bDisablePllMainClock)
  {
    Clk_DisablePllClock();
    Clk_DisableMainClock();
  }
  
  if (TRUE == bDisableSubClock)
  {
    Clk_DisableSubClock();
  }
  
  stcMainClockConfig.enMode = ClkTimer;
  Clk_SetMode(&stcMainClockConfig);
  
  return Ok;
} // Lpm_SetHsCrTimer

/**
 ******************************************************************************
 ** \brief Enter Main Timer Mode
 **
 ** \param [in]  bDisablePllClock    TRUE: Disable PLL Clock
 ** \param [in]  bDisableSubClock    TRUE: Disable Sub Clock
 **
 ** \retval Ok                       Returned from Main Timer Mode
 ******************************************************************************/ 
en_result_t Lpm_SetMainTimer(boolean_t bDisablePllClock,
                             boolean_t bDisableSubClock)
{
  stc_clk_main_config_t stcMainClockConfig;
  
  Clk_MainGetParameters(&stcMainClockConfig);
  
  FM4_CRG->SCM_CTL_f.MOSCE = 1u;        // enable Main Clock
  Clk_SwitchToMainClock();
  
  if (TRUE == bDisablePllClock)
  {
    Clk_DisablePllClock();
  }
  
  if (TRUE == bDisableSubClock)
  {
    Clk_DisableSubClock();
  }
  
  stcMainClockConfig.enMode = ClkTimer;
  Clk_SetMode(&stcMainClockConfig);
  
  return Ok;
} // Lpm_SetMainTimer

/**
 ******************************************************************************
 ** \brief Enter LS-CR Timer Mode
 **
 ** \param [in]  bDisablePllMainClock  TRUE: Disable PLL Clock and Main Clock
 ** \param [in]  bDisableSubClock      TRUE: Disable Sub Clock
 **
 ** \retval Ok                         Returned from LS-CR Timer Mode
 ******************************************************************************/ 
en_result_t Lpm_SetLsCrTimer(boolean_t bDisablePllMainClock,
                             boolean_t bDisableSubClock)
{
  stc_clk_main_config_t stcMainClockConfig;
  
  Clk_MainGetParameters(&stcMainClockConfig);
  
  Clk_SwitchToLsCrClock();
  
  if (TRUE == bDisablePllMainClock)
  {
    Clk_DisablePllClock();
    Clk_DisableMainClock();
  }
  
  if (TRUE == bDisableSubClock)
  {
    Clk_DisableSubClock();
  }
  
  stcMainClockConfig.enMode = ClkTimer;
  Clk_SetMode(&stcMainClockConfig);
  
  return Ok;
} // Lpm_SetLsCrTimer

/**
 ******************************************************************************
 ** \brief Enter Sub Timer Mode
 **
 ** \param [in]  bDisablePllMainClock  TRUE: Disable PLL Clock and Main Clock
 **
 ** \retval Ok                         Returned from LS-CR Timer Mode
 ** \retval ErrorNotReady              Sub clock not available
 ******************************************************************************/ 
en_result_t Lpm_SetSubTimer(boolean_t bDisablePllMainClock)
{
  stc_clk_main_config_t stcMainClockConfig;
  
  if(1u != FM4_CRG->SCM_STR_f.MORDY)
  {
    return ErrorNotReady;       // No sub clock available
  }
  
  Clk_MainGetParameters(&stcMainClockConfig);
  
  Clk_SwitchToSubClock();
  
  if (TRUE == bDisablePllMainClock)
  {
    Clk_DisablePllClock();
    Clk_DisableMainClock();
  }
  
  stcMainClockConfig.enMode = ClkTimer;
  Clk_SetMode(&stcMainClockConfig);
  
  return Ok;
} // Lpm_SetSubTimer

/**
 ******************************************************************************
 ** \brief Enter RTC Mode
 **
 ** \pre This function requires a proper initialized and running RTC
 **
 ** \retval Ok                         Returned from RTC Mode
 ******************************************************************************/ 
en_result_t Lpm_SetRtcMode(void)
{
  stc_clk_main_config_t stcMainClockConfig;

  stcMainClockConfig.enMode = ClkRtc;
  Clk_SetMode(&stcMainClockConfig);
  
  return Ok;
} // Lpm_SetRtcMode

/**
 ******************************************************************************
 ** \brief Enter Stop Mode
 **
 ** \retval Ok                         Returned from Stop Mode
 ******************************************************************************/ 
en_result_t Lpm_SetStopMode(void)
{
  stc_clk_main_config_t stcMainClockConfig;

  stcMainClockConfig.enMode = ClkStop;
  Clk_SetMode(&stcMainClockConfig);
  
  return Ok;
} // Lpm_SetStopMode

/**
 ******************************************************************************
 ** \brief Enter Deep RTC Mode
 **
 ** \pre This function requires a proper initialized and running RTC
 **
 ** \retval Ok                         Returned from Deep RTC Mode
 ******************************************************************************/ 
en_result_t Lpm_SetDeepRtcMode(void)
{
  stc_clk_main_config_t stcMainClockConfig;

  stcMainClockConfig.enMode = ClkDeepRtc;
  Clk_SetMode(&stcMainClockConfig);
  
  return Ok;
} // Lpm_SetDeepRtcMode

/**
 ******************************************************************************
 ** \brief Enter Deep Stop Mode
 **
 ** \retval Ok                         Returned from Deep Stop Mode
 ******************************************************************************/ 
en_result_t Lpm_SetDeepStopMode(void)
{
  stc_clk_main_config_t stcMainClockConfig;

  stcMainClockConfig.enMode = ClkDeepStop;
  Clk_SetMode(&stcMainClockConfig);
  
  return Ok;
} // Lpm_SetDeepStopMode

#endif // #if (defined(PDL_PERIPHERAL_CLK_ACTIVE)) && (defined(PDL_PERIPHERAL_EXINT_ACTIVE))

// The following code is always compiled if LPM module (this) is activated
/**
 ******************************************************************************
 ** \brief Read Backup Registers
 **
 ** \param [out]  stcBackUpReg     Pointer to a data structure of backup
 **                                registers to be read
 **
 ** \retval Ok                     Successfully read
 ******************************************************************************/ 
en_result_t Lpm_ReadBackupRegisters(stc_backupreg_t* stcBackUpReg)
{
  ReadBackupRegisters(stcBackUpReg);
    
  return Ok;
} // Lpm_ReadBackupRegisters

/**
 ******************************************************************************
 ** \brief Write Backup Registers
 **
 ** \param [in]   stcBackUpReg     structure of backup registers
 **
 ** \retval Ok                     Successfully written
 ******************************************************************************/ 
en_result_t Lpm_WriteBackupRegisters(stc_backupreg_t* stcBackUpReg)
{
  WriteBackupRegisters(stcBackUpReg);
    
  return Ok;
} // Lpm_WriteBackupRegisters

/**
 ******************************************************************************
 ** \brief Read Backup Register (Byte)
 **
 ** \param [in]   u8AddressOffset  Byte address offset to Back-up register
 ** \param [out]  u8Data           Pointer to Byte data to be read
 **
 ** \retval Ok                     Successfully written
 ** \retval ErrorInvalidParameter  Address offset out of range
 ******************************************************************************/ 
en_result_t Lpm_Read_u8DataBackupRegister(uint8_t u8AddressOffset, uint8_t* u8Data)
{
  stc_backupreg_t stcBackUpReg;
  
  if (u8AddressOffset >= PDL_BACK_UP_REGISTERS)
  {
    return ErrorInvalidParameter;
  }
  
  ReadBackupRegisters(&stcBackUpReg);
  
  *u8Data =  *(uint8_t*)((uint32_t)(&stcBackUpReg.u8BREG00) + (uint32_t)u8AddressOffset);
  
  return Ok;
} // Lpm_Read_u8DataBackupRegister

/**
 ******************************************************************************
 ** \brief Write Backup Registers (Byte)
 **
 ** \param [in]   u8AddressOffset  Byte address offset to Back-up register
 ** \param [in]   u8Data           Byte data to be written
 **
 ** \retval Ok                     Successfully written
 ** \retval ErrorInvalidParameter  Address offset out of range
 ******************************************************************************/ 
en_result_t Lpm_Write_u8DataBackupRegister(uint8_t u8AddressOffset, uint8_t u8Data)
{
  stc_backupreg_t stcBackUpReg;
  
  if (u8AddressOffset >= PDL_BACK_UP_REGISTERS)
  {
    return ErrorInvalidParameter;
  }
  
  ReadBackupRegisters(&stcBackUpReg);
  *(uint8_t*)((uint32_t)(&stcBackUpReg.u8BREG00) + (uint32_t)u8AddressOffset) = u8Data;
  WriteBackupRegisters(&stcBackUpReg);
  
  return Ok;
} // Lpm_Write_u8DataBackupRegister

/**
 ******************************************************************************
 ** \brief Read Backup Register (16-Bit)
 **
 ** \param [in]   u8AddressOffset  Byte address offset to Back-up register
 ** \param [out]  u16Data          Pointer to 16-Bit data to be read
 **
 ** \retval Ok                     Successfully written
 ** \retval ErrorInvalidParameter  Address offset out of range (highest priority)
 ** \retval ErrorAddressAlignment  Address not aligned to 16-bit
 ******************************************************************************/ 
en_result_t Lpm_Read_u16DataBackupRegister (uint8_t u8AddressOffset, uint16_t* u16Data)
{
  stc_backupreg_t stcBackUpReg;
  
  if (u8AddressOffset >= PDL_BACK_UP_REGISTERS)
  {
    return ErrorInvalidParameter;
  }
  
  if (0 != (u8AddressOffset % 2))
  {
    return ErrorAddressAlignment;
  }
  
  ReadBackupRegisters(&stcBackUpReg);
  
  *u16Data =  *(uint16_t*)((uint32_t)(&stcBackUpReg.u8BREG00) + (uint32_t)u8AddressOffset);
  
  return Ok;
} // Lpm_Read_u16DataBackupRegister

/**
 ******************************************************************************
 ** \brief Write Backup Registers (16-Bit)
 **
 ** \param [in]   u8AddressOffset  Byte address offset to Back-up register
 ** \param [in]   u16Data          16-Bit data to be written
 **
 ** \retval Ok                     Successfully written
 ** \retval ErrorInvalidParameter  Address offset out of range (highest priority)
 ** \retval ErrorAddressAlignment  Address not aligned to 16-bit
 ******************************************************************************/ 
en_result_t Lpm_Write_u16DataBackupRegister(uint8_t u8AddressOffset, uint16_t u16Data)
{
  stc_backupreg_t stcBackUpReg;
  
  if (u8AddressOffset >= PDL_BACK_UP_REGISTERS)
  {
    return ErrorInvalidParameter;
  }
  
  if (0 != (u8AddressOffset % 2))
  {
    return ErrorAddressAlignment;
  }
  
  ReadBackupRegisters(&stcBackUpReg);
  *(uint16_t*)((uint32_t)(&stcBackUpReg.u8BREG00) + (uint32_t)u8AddressOffset) = u16Data;
  WriteBackupRegisters(&stcBackUpReg);
  
  return Ok;
} // Lpm_Write_u16DataBackupRegister

/**
 ******************************************************************************
 ** \brief Read Backup Register (32-Bit)
 **
 ** \param [in]   u8AddressOffset  Byte address offset to Back-up register
 ** \param [out]  u32Data          Pointer to 32-Bit data to be read
 **
 ** \retval Ok                     Successfully written
 ** \retval ErrorInvalidParameter  Address offset out of range (highest priority)
 ** \retval ErrorAddressAlignment  Address not aligned to 32-bit
 ******************************************************************************/ 
en_result_t Lpm_Read_u32DataBackupRegister (uint8_t u8AddressOffset, uint32_t* u32Data)
{
  stc_backupreg_t stcBackUpReg;
  
  if (u8AddressOffset >= PDL_BACK_UP_REGISTERS)
  {
    return ErrorInvalidParameter;
  }
  
  if (0 != (u8AddressOffset % 4))
  {
    return ErrorAddressAlignment;
  }
  
  ReadBackupRegisters(&stcBackUpReg);
  
  *u32Data =  *(uint32_t*)((uint32_t)(&stcBackUpReg.u8BREG00) + (uint32_t)u8AddressOffset);
  
  return Ok;
} // Lpm_Read_u32DataBackupRegister

/**
 ******************************************************************************
 ** \brief Write Backup Registers (32-Bit)
 **
 ** \param [in]   u8AddressOffset  Byte address offset to Back-up register
 ** \param [in]   u32Data          Pointer to 32-Bit data
 **
 ** \retval Ok                     Successfully written
 ** \retval ErrorInvalidParameter  Address offset out of range (highest priority)
 ** \retval ErrorAddressAlignment  Address not aligned to 32-bit
 ******************************************************************************/ 
en_result_t Lpm_Write_u32DataBackupRegister(uint8_t u8AddressOffset, uint32_t u32Data)
{
  stc_backupreg_t stcBackUpReg;
  
  if (u8AddressOffset >= PDL_BACK_UP_REGISTERS)
  {
    return ErrorInvalidParameter;
  }
  
  if (0 != (u8AddressOffset % 4))
  {
    return ErrorAddressAlignment;
  }
  
  ReadBackupRegisters(&stcBackUpReg);
  *(uint32_t*)((uint32_t)(&stcBackUpReg.u8BREG00) + (uint32_t)u8AddressOffset) = u32Data;
  WriteBackupRegisters(&stcBackUpReg);
  
  return Ok;
} // Lpm_Write_u32DataBackupRegister

//@} // LpmGroup

#endif // #if (defined(PDL_PERIPHERAL_ENABLE_LPM))

/******************************************************************************/
/* EOF (not truncated)                                                        */
/******************************************************************************/
