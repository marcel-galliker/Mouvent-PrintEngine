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
/** \file crc.h
 **
 ** Headerfile for CRC functions
 **  
 **
 ** History:
 **   - 2013-04-02  1.0  NT   First version.
 **
 ******************************************************************************/

#ifndef __CRC_H__
#define __CRC_H__

/******************************************************************************/
/* Include files                                                              */
/******************************************************************************/
#include "pdl.h"

#if (defined(PDL_PERIPHERAL_CRC_ACTIVE))

/* C binding of definitions if building with C++ compiler */
#ifdef __cplusplus
extern "C"
{
#endif

/**
 ******************************************************************************
 ** \defgroup CrcGroup Crc Functions (CRC)
 **
 ** Provided functions of CRC module:
 ** 
 ** - Crc_Init()
 ** - Crc_DeInit()
 ** - Crc_Push8()
 ** - Crc_Push16()
 ** - Crc_Push32()
 ** - Crc_ReadResult()
 **
 ** Initialize the CRC with Crc_Init(). After this there are to ways to
 ** calculate the check result:
 ** - Providing data by CPU via Crc_Push8(), Crc_Push16(), or Crc_Push32()
 ** - Using DMA
 **
 ** The result can be read by calling Crc_ReadResult().
 **
 ** Be careful with the endian.
 ******************************************************************************/
//@{

/******************************************************************************
 * Global definitions
 ******************************************************************************/

/******************************************************************************
 * Global type definitions
 ******************************************************************************/
  
/**
 ******************************************************************************
 ** \brief Crc mode
 ** 
 ** To select between CRC16 and CRC32
 ******************************************************************************/
typedef enum en_crc_mode
{
    Crc16  = 0,  ///< CCITT CRC16 standard
    Crc32  = 1   ///< IEEE-802.3 CRC32 Ethernet standard
} en_crc_mode_t;  

/**
 *****************************************************************************
 ** \brief Crc configuration
 *****************************************************************************/
typedef struct stc_crc_config
{
    en_crc_mode_t enMode;               ///< See description of #en_crc_mode_t
    boolean_t     bUseDma;              ///< TRUE: DMA usage, needs DMA driver
    boolean_t     bFinalXor;            ///< TRUE: CRC result as XOR value
    boolean_t     bResultLsbFirst;      ///< CRC result bit order: TRUE: LSB first, FALSE MSB first
    boolean_t     bResultLittleEndian;  ///< CRC result byte order: TRUE: Litte endian, FLASE: big endian
    boolean_t     bDataLsbFirst;        ///< CRC feed data bit order: TRUE: LSB first, FALSE MSB first
    boolean_t     bDataLittleEndian;    ///< CRC feed data byte order: TRUE: Litte endian, FLASE: big endian
    uint32_t      u32CrcInitValue;      ///< Initial value, upper 16 bit ignored using CRC16
} stc_crc_config_t;

/******************************************************************************/
/* Local type definitions ('typedef')                                         */
/******************************************************************************/

/******************************************************************************/
/* Global variable definitions ('extern')                                     */
/******************************************************************************/

/******************************************************************************/
/* Global function prototypes (definition in C source)                        */
/******************************************************************************/

/* Init/DeInit */
extern en_result_t Crc_Init(stc_crc_config_t* pstcConfig);
extern void Crc_DeInit(void);

/* Calucuration */
extern void Crc_Push8 (uint8_t   u8DataToPush);
extern void Crc_Push16(uint16_t u16DataToPush);
extern void Crc_Push32(uint32_t u32DataToPush);

/* Get result */
extern uint32_t Crc_ReadResult(void);


//@} // CrcGroup

#ifdef __cplusplus
}
#endif

#endif /* #if (defined(PDL_PERIPHERAL_CRC_ACTIVE)) */

#endif /* __CRC_H__ */
/******************************************************************************/
/* EOF (not truncated)                                                        */
/******************************************************************************/
