/******************************************************************************/
/** \file i2c.c
 ** 
 ** i2c function for pressure sensors on conditioner board
 **
 **	Copyright 2016 by radex AG, Switzerland. All rights reserved.
 **	Written by Stefan Weber
 **
 ******************************************************************************/

/******************************************************************************/
/* Include files                                                              */
/******************************************************************************/
#include "IOMux.h"
#include "i2c.h"

/*****************************************************************************/
/* Defines                                                                   */
/*****************************************************************************/
#define TIME_OUT         400	// error counter time-out
#define TIME_OUT_INT    3000    // Time out for INT flag polling
#define I2C_WRITE		0x00
#define I2C_READ		0x01

//--- prototypes ------------------------

static int  _i2c_start(volatile stc_mfsn_t* pstcI2C, uint8_t addr, int op);
static void _i2c_sleep(void);

/**
 ******************************************************************************
 **  \brief       Start of I2C Communication with 1st Data Byte
 **
 **  \param [in]  u8Data  Data Byte
 **  \param [in]  pstcI2C pointer to I2C_Device
 **
 **  \retval      0    0k
 **  \retval      -1   Time-out Error
 **  \retval      -2   Bus Error
 **  \retval      -3   Ack Error
 **  \retval      -5   Trx Error
 ******************************************************************************/
static int _i2c_start(volatile stc_mfsn_t* pstcI2C, uint8_t addr, int op)
{ 
    uint16_t u16ErrorCount = 0;
    uint16_t timeout;

    pstcI2C->ISBA = 0x00;
    pstcI2C->ISMK = 0x80;         	// ISMK_EN = 1

    pstcI2C->TDR  = addr<<1 | op;   // Send data to EEPROM
    pstcI2C->IBCR = 0x80;         	// MSS = 1: Master Mode

    do
    {
        for (timeout = TIME_OUT_INT; !(pstcI2C->IBCR & 0x01); ) // Wait for transmission complete via INT flag
        {                                  
            if (--timeout<=0) return -1;	// timeout
        }

        if (pstcI2C->IBCR & 0x02) 	 return -2; // BER == 1? ->  error    
        if (!(pstcI2C->IBCR & 0x40)) return -3; // ACT == 0? ->  error
        if (pstcI2C->IBCR_f.MSS==0)  return -3; // MSS == 0? ->  slave Mode
                
        if (pstcI2C->IBSR_f.RSA ==1)  // RSA == 1? ->  Reserved Adress
        {
            if(pstcI2C->IBSR_f.FBT==0)	// not First byte bit
            {
                if(pstcI2C->IBSR_f.TRX==0) //receiving data
                {
                    
                }
            }
            else // first byte received/transmitted
            {
                u16ErrorCount = 0; 
            }
        }
            
        // MSS is set ...
        else if (pstcI2C->IBSR & 0x40)  // RACK == 1? ->  busy or error
        {
            pstcI2C->IBCR = 0xE0;  // MMS = CSS = ACKE = 1: Try restart (if busy)
            u16ErrorCount++;

            if (u16ErrorCount > TIME_OUT)	
                return -1;
        }
        else
        {
            u16ErrorCount = 0; 
        }
    } while (u16ErrorCount);

    if (!(pstcI2C->IBSR & 0x10))   		// TRX == 0? ->  error
    {
        // return -5; // not used S.Weber
    }

	//--- special address --------------------------------------------
	if(pstcI2C->IBSR_f.RSA==1)
	{
		uint8_t dummy=dummy;
		return I2cReceiveByteSeq(pstcI2C, &dummy, FALSE);
	}

    return 0;
}


//--- I2cStartRead ---------------------------------
int I2cStartRead(volatile stc_mfsn_t* pstcI2C, uint8_t addr)
{
	return _i2c_start(pstcI2C, addr, I2C_READ);
}

//--- I2cStartWrite ---------------------------------
int I2cStartWrite(volatile stc_mfsn_t* pstcI2C, uint8_t addr)
{
	return _i2c_start(pstcI2C, addr, I2C_WRITE);
}

/**
 ******************************************************************************
 **  \brief    Stop I2C Write Communication
 **
 **  \param [in]	pstcI2C pointer to I2C_Device
 **
 **  \return   void
 ******************************************************************************/
void I2cStopWrite(volatile stc_mfsn_t* pstcI2C)
{
	pstcI2C->IBCR = 0x20;    // MMS = CSS = INT = 0, ACKE = 1
}

/**
 ******************************************************************************
 **  \brief    Stop I2C Read Communication
 **
 **  \param [in]	pstcI2C pointer to I2C_Device
 **
 **  \return   void
 ******************************************************************************/
void I2cStopRead(volatile stc_mfsn_t* pstcI2C)
{
	pstcI2C->ISMK_f.EN = 0; // disable i2c bus
	pstcI2C->IBCR = 0x01;   // INT = 1, MMS = ACKE = 0
}

/**
 ******************************************************************************
 **  \brief     Receive I2C Byte
 **
 **  \param [in]	pstcI2C pointer to I2C_Device
 **
 **  \return    Received Data (positive value)
 **  \return    -1 = Time-out Error
 **  \return    -2 = Bus Error
 **  \return    -3 = Ack Error
 **  \return    -4 = Rack Error
 **  \return    -5 = Trx Error
 ******************************************************************************/
/*
int I2cReceiveByte(volatile stc_mfsn_t* pstcI2C)
{ 
  uint8_t  u8Data;
  uint16_t u16ErrorCount = 1;
  uint16_t u16Timeout;
  
  do
  {
    if ((!(pstcI2C->IBSR & 0x80)))   // FBT = 1
    {
      u8Data = pstcI2C->RDR;
			if(pstcI2C->IBSR_f.RSA==1)
			{
				pstcI2C->IBCR_f.WSEL=1;
				pstcI2C->IBCR_f.ACKE=1;
				pstcI2C->IBCR_f.INT=0;
			}
      u16ErrorCount = 0;
    }
    else
    {
      u16ErrorCount++;
      if (u16ErrorCount > TIME_OUT) 	// too much retrails
      {
        return -1; 
      }
      
      pstcI2C->IBCR = 0xB0;   				// MMS = 1, WSEL = 1, ACKE = 1, Clear INT flag
      
      u16Timeout = TIME_OUT_INT;
      
      while((!(pstcI2C->IBCR & 0x01)) // Wait for transmission complete via INT flag
            && (u16Timeout))
      {                                  
        u16Timeout--;
      }
      
      if (0 == u16Timeout)
      {
        return -1;
      }
  
      if (pstcI2C->IBCR & 0x02)       // BER == 1? ->  error
      {
        return -2;
      }
      
      if (!(pstcI2C->IBCR & 0x40))    // ACT == 0? ->  error
      {
        return -3;
      }
      
      // MSS is set, no reserved address
      
      if (pstcI2C->IBSR & 0x40)       // RACK == 1? ->  error
      {
        return -4;
      }
      
      if (pstcI2C->IBSR & 0x10)       // TRX == 1? ->  error
      {
        return -5;
      }
    }
  }while(u16ErrorCount);
  
  return (int16_t)u8Data;
}
*/

/**
 ******************************************************************************
 **  \brief     Receive I2C Byte sequentially (with dummy TX data for generating
 **             clock)
 **
 ** \param [in]	pstcI2C pointer to I2C_Device
**  \param [in] u8LastByte   1 == indicates last byte to read w/o TX dummy data
 **
 **  \return    -1 = Time-out Error
 **  \return    -2 = Bus Error
 **  \return    -3 = Ack Error
 **  \return    -4 = Rack Error
 **  \return    -5 = Trx Error
 ******************************************************************************/

int	I2cReceiveByteSeq(volatile stc_mfsn_t* pstcI2C, uint8_t* pdata, uint8_t u8LastByte)
{ 
	int  timeout;
	*pdata = 0;
	if (u8LastByte) pstcI2C->IBCR = 0x80;   // MSS = 1, ACKE = 0, Clear INT flag
	else			pstcI2C->IBCR = 0xa0;  	// MSS = 1, ACKE = 1, Clear INT flag
		
	for(timeout=TIME_OUT_INT; timeout; timeout--)
	{
		if (pstcI2C->SSR & 0x04)
		{
			*pdata = pstcI2C->RDR;
			_i2c_sleep();
			return 0;
		}
		if (  pstcI2C->IBCR & 0x02)  return -2; // BER  == 1? ->  error
		if (!(pstcI2C->IBCR & 0x40)) return -3; // ACT  == 0? ->  error
//		if (  pstcI2C->IBSR & 0x40)  return -4; // RACK == 1? ->  error
		if (  pstcI2C->IBSR & 0x10)  return -5; // TRX  == 1? ->  error
	};
	return -1; // timeout
}

//--- _i2c_sleep --------------------------------
static void _i2c_sleep(void)
{
	volatile int i;
	for(i=0; i<40; i++); // 30=error!
}

/**
 ******************************************************************************
 **  \brief        Send I2C Byte
 **
 **  \param [in]   u8Data   Byte Data
 **
 **  \retval      0    0k
 **  \retval      -1   Time-out Error
 **  \retval      -2   Bus Error
 **  \retval      -3   Ack Error
 **  \retval      -4   Rack Error
 **  \retval      -5   Trx Error
 ******************************************************************************/
int I2cSendByte(uint8_t u8Data, volatile stc_mfsn_t* pstcI2C)
{
	uint16_t u16Timeout;

	pstcI2C->TDR = u8Data;  // Send data to EEPROM
	pstcI2C->IBCR = 0xB0;   // WSEL = 1, ACKE = 1, Clear INT flag

	u16Timeout = TIME_OUT_INT;

	while(!(pstcI2C->IBCR & 0x01)) // Wait for transmission complete via INT flag
	{
		if (!u16Timeout--) return -1;	// timeout
	}

	if (pstcI2C->IBCR & 0x02)     return -2; // BER == 1? ->  error
	if (!(pstcI2C->IBCR & 0x40))  return -3; // ACT == 0? ->  error

	// MSS is set, no reserved address

	if (pstcI2C->IBSR & 0x40)  	  return -4; // RACK == 1? ->  error
	if (!(pstcI2C->IBSR & 0x10))  return -5; // TRX == 0? ->  error

	return 0;
}
