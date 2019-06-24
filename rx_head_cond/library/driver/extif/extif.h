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
/** \file extif.h
 **
 ** Headerfile for EXTIF functions
 **  
 ** History:
 **   - 2013-04-16  1.0  MWi  First version.
 **
 ******************************************************************************/

#ifndef __EXTIF_H__
#define __EXTIF_H__

/******************************************************************************/
/* Include files                                                              */
/******************************************************************************/
#include "pdl.h"

#if (defined(PDL_PERIPHERAL_EXTIF_ACTIVE))

/* C binding of definitions if building with C++ compiler */
#ifdef __cplusplus
extern "C"
{
#endif
  
/**
 ******************************************************************************
 ** \defgroup ExtifGroup External Bus Interface Functions (EXTIF)
 **
 ** Provided functions of EXTIF module:
 ** - Extif_InitArea()
 ** - Extif_ReadErrorStatus()
 ** - Extif_ReadErrorAddress()
 ** - Extif_ClearErrorStatus()
 ** - Extif_CheckSdcmdReady()
 ** - Extif_SetSdramCommand()
 **
 ** Extif_InitArea() initializes an area according the user configuration.
 ** Extif_ReadErrorStatus() reads out the error status of the Error Status
 ** Register (EST). Extif_ReadErrorAddress() reads out the address were the
 ** error occurred. Extif_ClearErrorStatus() clears the error status.
 ** Extif_CheckSdcmdReady() checks, if it is allowed to set a SDRAM command.
 ** Extif_SetSdramCommand() set the SDRAM command (calls
 ** Extif_CheckSdcmdReady() ).
 **
 ******************************************************************************/
//@{
  
/******************************************************************************/
/* Global pre-processor symbols/macros ('#define')                            */
/******************************************************************************/ 
#define EXTIF          (*((stc_extif_arrays_t *) FM4_EXBUS_BASE))
   
/******************************************************************************
 * NAND Flash access macros (intern)
 ******************************************************************************/
#define EXITIF_NAND_ALE_OFFSET   0x00003000ul  ///< Address offset for MNALE assert until write
#define EXITIF_NAND_ADDR_OFFSET  0x00002000ul  ///< Address offset for address cycle (+ MNALE)
#define EXITIF_NAND_CMD_OFFSET   0x00001000ul  ///< Address offset for command cycle (+ MNCLE)
#define EXITIF_NAND_DATA_OFFSET  0x00000000ul  ///< Address offset for data read/write
  
/******************************************************************************
 * NAND Flash access macros (user)
 ******************************************************************************/
#define Extif_Nand_SetCommand(base, cmd)  {*(volatile unsigned char*)((base) + EXITIF_NAND_CMD_OFFSET)  = (unsigned char)(cmd);}  ///< Sets a NAND command cycle
#define Extif_Nand_SetAddress(base, addr) {*(volatile unsigned char*)((base) + EXITIF_NAND_ADDR_OFFSET) = (unsigned char)(addr);} ///< Sets a NAND address cycle
#define Extif_Nand_ReadData(base)         (*(volatile unsigned char*)((base) + EXITIF_NAND_DATA_OFFSET))                          ///< Reads NAND data
#define Extif_Nand_WriteData(base, data)  {*(volatile unsigned char*)((base) + EXITIF_NAND_DATA_OFFSET) = (unsigned char)(data);} ///< Writes NAND data
#define Extif_Nand_ClearAle(base)         {*(volatile unsigned char*)((base) + EXITIF_NAND_ALE_OFFSET)  = (unsigned char)0;}      ///< De-asserts MNALE signal
   
/******************************************************************************
 * Global type definitions
 ******************************************************************************/

/**
 ******************************************************************************
 ** \brief Definition of MODE, TIM, AREA, ATIM register arrays
 ** 
 ** To select the data bus width
 ******************************************************************************/
typedef struct stc_extif_arrays
{
  union {
    uint32_t                au32MODE[8];
    stc_exbus_mode_field_t  astcMODE[8];
  };  
  union {
    uint32_t                au32TIM[8];
    stc_exbus_tim_field_t   astcTIM[8];
  };   
  union {
    uint32_t                au32AREA[8];
    stc_exbus_area_field_t  astcAREA[8];
  };   
  union {
    uint32_t                au32ATIM[8];
    stc_exbus_atim_field_t  astcATIM[8];
  };
} stc_extif_arrays_t;

/**
 ******************************************************************************
 ** \brief Extif data width
 ** 
 ** To select the data bus width
 ******************************************************************************/
typedef enum en_extif_width
{
  Extif8Bit   = 0,  ///< 8 Bit mode
  Extif16Bit  = 1   ///< 16 Bit  mode
} en_extif_width_t;

/**
 ******************************************************************************
 ** \brief Extif cycle defintion
 ** 
 ** To select the bus mode of the interface.
 **
 ** \note Do not change enumeration number. The numbers are taken for
 **       calculating the corresponding bitfield!
 ******************************************************************************/
typedef enum en_extif_cycle
{ 
  Extif0Cycle   = 0,  ///<  0 cycles
  Extif1Cycle   = 1,  ///<  1 cycle
  Extif2Cycle   = 2,  ///<  2 cycles
  Extif3Cycle   = 3,  ///<  3 cycles
  Extif4Cycle   = 4,  ///<  4 cycles
  Extif5Cycle   = 5,  ///<  5 cycles
  Extif6Cycle   = 6,  ///<  6 cycles
  Extif7Cycle   = 7,  ///<  7 cycles
  Extif8Cycle   = 8,  ///<  8 cycles
  Extif9Cycle   = 9,  ///<  9 cycles
  Extif10Cycle  = 10, ///< 10 cycles
  Extif11Cycle  = 11, ///< 11 cycles
  Extif12Cycle  = 12, ///< 12 cycles
  Extif13Cycle  = 13, ///< 13 cycles
  Extif14Cycle  = 14, ///< 14 cycles
  Extif15Cycle  = 15, ///< 15 cycles
  Extif16Cycle  = 16, ///< 16 cycles
  ExtifDisabled = 17  ///< Setting disabled
} en_extif_cycle_t;

/**
 ******************************************************************************
 ** \brief Extif mask setup (area size)
 ** 
 ** To select the Mask Setup value for the address area per chip select.
 **
 ** \note Do not change enumeration number. The numbers are taken for
 **       calculating the corresponding bitfield!
 ******************************************************************************/
typedef enum en_extif_mask
{
  Extif1MB   = 0,  ///< Area 1 MByte 
  Extif2MB   = 1,  ///< Area 2 MByte 
  Extif4MB   = 2,  ///< Area 4 MByte 
  Extif8MB   = 3,  ///< Area 8 MByte 
  Extif16MB  = 4,  ///< Area 16 MByte 
  Extif32MB  = 5,  ///< Area 32 MByte 
  Extif64MB  = 6,  ///< Area 64 MByte 
  Extif128MB = 7   ///< Area 128 MByte 
} en_extif_mask_t;

/**
 ******************************************************************************
 ** \brief Extif CAS address select
 ** 
 ** Select the address for the Column Address Select
 **
 ** \note Do not change enumeration number. The numbers are taken for
 **       calculating the corresponding bitfield!
 ******************************************************************************/
typedef enum en_extif_cas
{
  ExtifCas16Bit = 0,  ///< MAD[9:0] = Internal address [10:1], 16-Bit width
  ExtifCas32Bit = 1   ///< MAD[9:0] = Internal address [11:2], 32-Bit width  
} en_extif_cas_t;

/**
 ******************************************************************************
 ** \brief Extif RAS address select
 ** 
 ** Select the address for the Row Address Select
 **
 ** \note Do not change enumeration number. The numbers are taken for
 **       calculating the corresponding bitfield!
 ******************************************************************************/
typedef enum en_extif_ras
{
  ExtifRas_19_6  = 0,   ///< MAD[13:0] = Internal address [19:6]
  ExtifRas_20_7  = 1,   ///< MAD[13:0] = Internal address [20:7]
  ExtifRas_21_8  = 2,   ///< MAD[13:0] = Internal address [21:8]
  ExtifRas_22_9  = 3,   ///< MAD[13:0] = Internal address [22:9]
  ExtifRas_23_10 = 4,   ///< MAD[13:0] = Internal address [23:10]
  ExtifRas_24_11 = 5,   ///< MAD[13:0] = Internal address [24:11]
  ExtifRas_25_12 = 6,   ///< MAD[13:0] = Internal address [25:12]
} en_extif_ras_t;

/**
 ******************************************************************************
 ** \brief Extif BAS address select
 ** 
 ** Select the address for the Bank Address Select
 **
 ** \note Do not change enumeration number. The numbers are taken for
 **       calculating the corresponding bitfield!
 ******************************************************************************/
typedef enum en_extif_bas
{
  ExtifBas_20_19 = 0,   ///< MAD[13:0] = Internal address [20:19]
  ExtifBas_21_20 = 1,   ///< MAD[13:0] = Internal address [21:20]
  ExtifBas_22_21 = 2,   ///< MAD[13:0] = Internal address [22:21]
  ExtifBas_23_22 = 3,   ///< MAD[13:0] = Internal address [23:22]
  ExtifBas_24_23 = 4,   ///< MAD[13:0] = Internal address [24:23]
  ExtifBas_25_24 = 5,   ///< MAD[13:0] = Internal address [25:24]
  ExtifBas_26_25 = 6,   ///< MAD[13:0] = Internal address [26:25]
} en_extif_bas_t;

/**
 *****************************************************************************
 ** \brief Extif area configuration
 ** 
 ** The WDG configuration is done on a per area (Chip select) basis
 *****************************************************************************/
typedef struct stc_extif_area_config
{
  en_extif_width_t  enWidth;                   ///< 8, 16 bit data bus width. See description of #en_extif_width_t
  boolean_t         bReadByteMask;             ///< TRUE: Read Byte Mask enable
  boolean_t         bWriteEnableOff;           ///< TRUE: Write enable disabled
  boolean_t         bNandFlash;                ///< TRUE: NAND Flash bus enable, FLASE: NOR Flash/SRAM bus enable
  boolean_t         bPageAccess;               ///< TRUE: NOR Flash memory page access mode enabled
  boolean_t         bRdyOn;                    ///< TRUE: RDY mode enabled
  boolean_t         bStopDataOutAtFirstIdle;   ///< TRUE: Stop to write data output at first idle cycle, FALSE: Extends to write data output to the last idle cycle
  boolean_t         bMultiplexMode;            ///< TRUE: Multiplex mode
  boolean_t         bAleInvert;                ///< TRUE: Invert ALE signal (negative polarity)
  boolean_t         bAddrOnDataLinesOff;       ///< TRUE: Do not output address to data lines (Hi-Z during ALC cycle period)
  boolean_t         bMpxcsOff;                 ///< TRUE: Do not assert MCSX in ALC cycle period
  boolean_t         bMoexWidthAsFradc;         ///< TRUE: MOEX width is set with FRADC, FALSE: MOEX width is set with RACC-RADC
  en_extif_cycle_t  enReadAccessCycle;         ///< Read Access Cycle timing
  en_extif_cycle_t  enReadAddressSetupCycle;   ///< Read Address Setup Cycle timing
  en_extif_cycle_t  enFirstReadAddressCycle;   ///< First Read Address Cycle timing
  en_extif_cycle_t  enReadIdleCycle;           ///< Read Idle Cycle timing
  en_extif_cycle_t  enWriteAccessCycle;        ///< Write Access Cycle timing
  en_extif_cycle_t  enWriteAddressSetupCycle;  ///< Write Address Setup Cycle timing
  en_extif_cycle_t  enWriteEnableCycle;        ///< Write Enable Cycle timing
  en_extif_cycle_t  enWriteIdleCycle;          ///< Write Idle Cycle timing
  uint8_t           u8AreaAddress;             ///< Address bits [27:20]
  en_extif_mask_t   enAreaMask;                ///< See description of #en_extif_mask_t
  en_extif_cycle_t  enAddressLatchCycle;       ///< Address Latch Cycles
  en_extif_cycle_t  enAddressLatchSetupCycle;  ///< Address Latch Enable Setup Cycles
  en_extif_cycle_t  enAddressLatchWidthCycle;  ///< Address Latch Enable Width Cycles
  boolean_t         bSdramEnable;              ///< TRUE: Enables SDRAM functionality (only possible for area 8)
  boolean_t         bSdramPowerDownMode;       ///< TRUE: Enables SDRAM Power Down Mode (only possible for area 8)
  boolean_t         bSdramRefreshOff;          ///< TRUE: Disables Refresh Function (only possible for area 8)
  en_extif_cas_t    enCasel;                   ///< Column Address Select, see #en_extif_cas_t for details (only possible for area 8)
  en_extif_ras_t    enRasel;                   ///< Row Address Select, see #en_extif_ras_t for details (only possible for area 8)
  en_extif_bas_t    enBasel;                   ///< Bank Address Select, see #en_extif_bas_t for details (only possible for area 8)
  uint16_t          u16RefreshCount;           ///< Refresh Count in Cycles (only possible for area 8)
  uint8_t           u8RefreshNumber;           ///< Number of Refreshs (only possible for area 8)
  boolean_t         bRefreshEnable;            ///< TRUE: Enables Refresh (only possible for area 8)
  uint16_t          u16PowerDownCount;         ///< Power Down Count in Cycles (only possible for area 8)
  en_extif_cycle_t  enSdramCasLatencyCycle;    ///< Latency of CAS in Cycles (only possible for area 8)
  en_extif_cycle_t  enSdramRasCycle;           ///< RAS Cycles (only possible for area 8)
  en_extif_cycle_t  enSdramRasPrechargeCycle;  ///< RAS Precharge Cycles (only possible for area 8)
  en_extif_cycle_t  enSdramRasCasDelayCycle;   ///< RAS CAS Delay Cycles (only possible for area 8)
  en_extif_cycle_t  enSdramRasActiveCycle;     ///< RAS Active Cycles (only possible for area 8)
  en_extif_cycle_t  enSdramRefreshCycle;       ///< Refresh Cycles (only possible for area 8)
  en_extif_cycle_t  enSdramPrechargeCycle;     ///< Data-in to Precharge Lead Time in Cycles (only possible for area 8)
  boolean_t         bSdramErrorInterruptEnable; ///< TRUE: Enables SDRAM error interrupt (only possible for area 8)
  boolean_t         bSramFlashErrorInterruptEnable; ///< TRUE: Enables SRAM/Flash Error Interrupt (only possible for area 8)
  func_ptr_t        pfnSdramErrorCallback;     ///< Pointer to SDRAM Error Callback Function (only possible for area 8)
  func_ptr_t        pfnSramFlashErrorCallback; ///< Pointer to SRAM/Flash Error Callback Function (only possible for area 8)
  boolean_t         bMclkoutEnable;            ///< TRUE: Enables MCLKOUT pin
  uint8_t           u8MclkDivision;            ///< Division ratio for MCLK (1 ... 16 div)
  boolean_t         bPrecedReadContinuousWrite; ///< TRUE: Enables preceding read and continuous write request
} stc_extif_area_config_t;
  
/// EXTIF Error Interrupt Callback Pointers
typedef struct stc_extif_intern_data
{
  boolean_t         bSdramErrorInterruptEnable; ///< TRUE: Enables SDRAM error interrupt
  boolean_t         bSramFlashErrorInterruptEnable; ///< TRUE: Enables SRAM/Flash Error Interrupt
  func_ptr_t        pfnSdramErrorCallback;       ///< Pointer to SDRAM Error Callback Function
  func_ptr_t        pfnSramFlashErrorCallback;   ///< Pointer to SRAM/Flash Error Callback Function
} stc_extif_intern_data_t;

/******************************************************************************/
/* Global function prototypes ('extern', definition in C source)              */
/******************************************************************************/

extern void        ExtifIrqHandler( void );

extern en_result_t Extif_InitArea( uint8_t                  u8Area,
                                   stc_extif_area_config_t* pstcConfig );

extern en_result_t Extif_ReadErrorStatus( void );

extern uint32_t    Extif_ReadErrorAddress( void );

extern en_result_t Extif_ClearErrorStatus( void );

extern en_result_t Extif_CheckSdcmdReady( void );

extern en_result_t Extif_SetSdramCommand( uint16_t  u16Address,
                                          boolean_t bMsdwex,
                                          boolean_t bMcasx,
                                          boolean_t bMrasx,
                                          boolean_t bMcsx8,
                                          boolean_t bMadcke
                                       );

//@} // ExtifGroup

#ifdef __cplusplus
}
#endif

#endif // #if (defined(PDL_PERIPHERAL_EXTIF_ACTIVE))

#endif /* __EXTIF_H__ */
/******************************************************************************/
/* EOF (not truncated)                                                        */
/******************************************************************************/
