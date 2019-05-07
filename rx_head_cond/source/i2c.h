/******************************************************************************/
/** \file i2c.h
 ** 
 ** i2c function for preasure sensors on conditioner board
 **
 **	Copyright 2016 by radex AG, Switzerland. All rights reserved.
 **	Written by Stefan Weber
 **
 ******************************************************************************/
 
#ifndef __i2c_h__
#define __i2c_h__

/*****************************************************************************/
/* Include files                                                             */
/*****************************************************************************/


/*****************************************************************************/
/* Function prototype                                                        */
/*****************************************************************************/
int		I2cStartRead		(volatile stc_mfsn_t* pstcI2C, uint8_t addr);
int		I2cReceiveByteSeq	(volatile stc_mfsn_t* pstcI2C, uint8_t* pdata, uint8_t u8LastByte);
void 	I2cStopRead			(volatile stc_mfsn_t* pstcI2C);
void 	I2cStopWrite		(volatile stc_mfsn_t* pstcI2C);
// int		I2cReceiveByte(volatile stc_mfsn_t* pstcI2C);

// void 	I2cWait(void);

#endif /* __i2c_h__ */
