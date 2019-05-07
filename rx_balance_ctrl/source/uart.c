/******************************************************************************/
/** \file uart.c
 ** 
 ** uart functions to communicate between fpga and conditioner board
 **
 **	Copyright 2016 by radex AG, Switzerland. All rights reserved.
 **	Written by Stefan Weber
 **
 ******************************************************************************/

/******************************************************************************/
/* Include files                                                              */
/******************************************************************************/

#include <string.h>
#include "IOMux.h" 
#include "fifo.h"
#include "comm.h"
#include "main.h"
#include "balance_def.h"
#include "uart.h"

SFifo			_RxFifo_prev;
SFifo			_TxFifo_prev;

SFifo			_RxFifo_next;
SFifo			_TxFifo_next;

static int _uart_prev_error_overrun=0;
static int _uart_prev_error_parity =0;
static int _uart_prev_error_framing=0;
static int _uart_prev_fifo_error=0;

static int _uart_prev_sent=0;
static int _uart_prev_sent_msg=0;
static int _uart_prev_received=0;
static int _uart_prev_fifo_received=0;

static int _uart_next_error_overrun=0;
static int _uart_next_error_parity =0;
static int _uart_next_error_framing=0;
static int _uart_next_fifo_error=0;

static int _uart_next_sent=0;
static int _uart_next_sent_msg=0;
static int _uart_next_received=0;
static int _uart_next_fifo_received=0;
	
//--- prototypes --------------------------------------------------------------
static void InitMfs0(UINT32 baudrate);
static void InitMfs2(UINT32 baudrate);

/**
 ******************************************************************************
 ** \brief  init uart function
 ******************************************************************************/
void uart_init(UINT32 baudrate)
{
	fifo_init(&_RxFifo_prev);	
	fifo_init(&_TxFifo_prev);	
	comm_init(0);
	InitMfs0(baudrate);
	
	fifo_init(&_RxFifo_next);
	fifo_init(&_TxFifo_next);	
	comm_init(1);
	InitMfs2(baudrate);
}

/**
 ******************************************************************************
 **  \brief       Initialization of MFS0 in UART-Mode
 ******************************************************************************/
static void InitMfs0(UINT32 baudrate)
{
	// Initialize MFS to UART asynchronous mode
	FM4_MFS0->SMR =  0x01u;                     // serial data output enable, operation mode 0
	FM4_MFS0->SCR =  0x80u;                     // MFS reset
	FM4_MFS0->SSR =  0x00u;                     // 0x80 = clear receive error flag
	FM4_MFS0->ESCR = 0x00u;          
	FM4_MFS0->SCR_f.RIE=0x01u;									// enable received interrupt 
	
	//rt/tx FIFO each = 64 Byte
	//Max. FBYTE Count = 64
	//Count of Data to be Stored
	FM4_MFS0->FCR1_f.FSEL=0x00u;								// FIFO2 Rx, FIFO1 Tx 
	FM4_MFS0->FCR1_f.FLSTE=0x0u;								// disable Data lost detection
	
	FM4_MFS0->FCR0_f.FCL1=0x01u; 								//reset fifo1
	//FM4_MFS0->FCR0_f.FCL2=0x01u; 							//reset fifo2 (not used, as we do not use a rx fifo)
	
	FM4_MFS0->FCR1_f.FDRQ=0x0u;									// no irq if tx fifo is empty (change later if you put data in tx fifo)
	FM4_MFS0->FCR1_f.FTIE=0x1u; 								// enable transmit fifo interrupt
//	FM4_MFS0->FCR1_f.FTIE=0x0u; 								// enable transmit fifo interrupt
	
	FM4_MFS0->FBYTE1=0x00u; 										// set fifo 1 tx value 
	//FM4_MFS0->FBYTE2=0x01u; 									//fifo 1 rx interrupt after 1	byte received

	FM4_MFS0->FCR0_f.FE1=0x01;									//enable fifo1
	//FM4_MFS0->FCR0_f.FE2=0x01;								//enable fifo2 (not used, as we do not use a rx fifo)

	FM4_MFS0->BGR = 2000000ul / baudrate - 1;                  // 115.2kbaud @ 2 MHz
	//FM4_MFS0->BGR = 20000000ul / 19200ul - 1;                  // 19.2kbaud @ 20 MHz
	//FM4_MFS0->BGR = 20000000ul / 9600ul - 1;                  // 9.6kbaud @ 20 MHz
	FM4_MFS0->SCR |= 0x03u;                                     // RX, TX enable  
	
	// MFS0 transmit interrupt settings 
	NVIC_ClearPendingIRQ(MFS0_TX_IRQn);
	NVIC_EnableIRQ(MFS0_TX_IRQn);
	NVIC_SetPriority(MFS0_TX_IRQn, 14u);
	
	// MFS0 receive interrupt settings
	NVIC_ClearPendingIRQ(MFS0_RX_IRQn);
	NVIC_EnableIRQ(MFS0_RX_IRQn);
	NVIC_SetPriority(MFS0_RX_IRQn, 8u);	
}

/**
 ******************************************************************************
 **  \brief       Initialization of MFS2 in UART-Mode
 ******************************************************************************/
static void InitMfs2(UINT32 baudrate)
{
	// Initialize MFS to UART asynchronous mode
	FM4_MFS2->SMR =  0x01u;                     // serial data output enable, operation mode 0
	FM4_MFS2->SCR =  0x80u;                     // MFS reset
	FM4_MFS2->SSR =  0x00u;                     // 0x80 = clear receive error flag
	FM4_MFS2->ESCR = 0x00u;          
	FM4_MFS2->SCR_f.RIE=0x01u;									// enable received interrupt 
	
	//rt/tx FIFO each = 64 Byte
	//Max. FBYTE Count = 64
	//Count of Data to be Stored
	FM4_MFS2->FCR1_f.FSEL=0x00u;								// FIFO2 Rx, FIFO1 Tx 
	FM4_MFS2->FCR1_f.FLSTE=0x0u;								// disable Data lost detection
	
	FM4_MFS2->FCR0_f.FCL1=0x01u; 								//reset fifo1
	//FM4_MFS2->FCR0_f.FCL2=0x01u; 							//reset fifo2 (not used, as we do not use a rx fifo)
	
	FM4_MFS2->FCR1_f.FDRQ=0x0u;									// no irq if tx fifo is empty (change later if you put data in tx fifo)
	FM4_MFS2->FCR1_f.FTIE=0x1u; 								// enable transmit fifo interrupt
	
	FM4_MFS2->FBYTE1=0x00u; 										// set fifo 1 tx value 
	//FM4_MFS2->FBYTE2=0x01u; 									//fifo 1 rx interrupt after 1	byte received

	FM4_MFS2->FCR0_f.FE1=0x01;									//enable fifo1
	//FM4_MFS2->FCR0_f.FE2=0x01;								//enable fifo2 (not used, as we do not use a rx fifo)

	FM4_MFS2->BGR = 2000000ul / baudrate - 1;                  // 115.2kbaud @ 2 MHz
	//FM4_MFS2->BGR = 20000000ul / 19200ul - 1;                  // 19.2kbaud @ 20 MHz
	//FM4_MFS2->BGR = 20000000ul / 9600ul - 1;                  // 9.6kbaud @ 20 MHz
	FM4_MFS2->SCR |= 0x03u;                                     // RX, TX enable  
	
	// MFS2 transmit interrupt settings 
	NVIC_ClearPendingIRQ(MFS2_TX_IRQn);
	NVIC_EnableIRQ(MFS2_TX_IRQn);
	NVIC_SetPriority(MFS2_TX_IRQn, 15u);
	
	// MFS2 receive interrupt settings
	NVIC_ClearPendingIRQ(MFS2_RX_IRQn);
	NVIC_EnableIRQ(MFS2_RX_IRQn);
	NVIC_SetPriority(MFS2_RX_IRQn, 9u);	
}

/**
 ******************************************************************************
 ** \brief  MFS0 RX IRQ handler (occurs if there is new data)
 ******************************************************************************/
void MFS0_RX_IRQHandler(void)
{	
	if(FM4_MFS0->SSR_f.RDRF)		// Received Data Register contains data
	{
		UCHAR data=FM4_MFS0->RDR;
		_uart_prev_received++;
		if (fifo_put(&_RxFifo_prev, &data, 1))
		{
			_uart_prev_fifo_received++;
		}
		else
		{
			_uart_prev_fifo_error++;
		}
	}
	else
	{
		if(FM4_MFS0->SSR_f.PE)	//Parity Error
		{
				_uart_prev_error_parity++;
		}
		if(FM4_MFS0->SSR_f.FRE)	//Framing error
		{
				_uart_prev_error_framing++;
		}	
		if(FM4_MFS0->SSR_f.ORE)	//overrun error
		{
			_uart_prev_error_overrun++;
		}	
		FM4_MFS0->SSR_f.REC=1;	// clear error flac
	}
	NVIC_ClearPendingIRQ(MFS0_RX_IRQn);
}

/**
 ******************************************************************************
 ** \brief  MFS0 TX IRQ handler (occurs if the tx reg is empty)
 ******************************************************************************/
void MFS0_TX_IRQHandler(void)
{
	UCHAR data;
	if (fifo_get(&_TxFifo_prev, &data))
	{
		FM4_MFS0->TDR = data;
		_uart_prev_sent++;
	}
	else	
	{
		FM4_MFS0->FCR1_f.FTIE=0x0u; 	 	// disable transmit interrupt
		_uart_prev_sent_msg++;
	}
	NVIC_ClearPendingIRQ(MFS0_TX_IRQn);
}


/**
 ******************************************************************************
 ** \brief  read from the receive prev fifo if it is not empty
 ** \return "data is valid"
 ******************************************************************************/
int uart_read_prev(UCHAR *data)
{
	return fifo_get(&_RxFifo_prev, data);
}


/**
 ******************************************************************************
 ** \brief  write data to fifo if he is not full, so check before! enable tx interrupt
 **
 ** \input	char data to write in fifo, int count
 ******************************************************************************/
void uart_write_prev(UCHAR *data, int count)
{
	if (fifo_put(&_TxFifo_prev, data, count))
	{
		FM4_MFS0->FCR1_f.FTIE=0x1u; 	// enable transmit interrupt
	}
}


/**
 ******************************************************************************
 ** \brief  MFS2 RX IRQ handler (occurs if there is new data)
 ******************************************************************************/
void MFS2_RX_IRQHandler(void)
{	
	if(FM4_MFS2->SSR_f.RDRF)		// Received Data Register contains data
	{
		UCHAR data=FM4_MFS2->RDR;
		_uart_next_received++;
		if (fifo_put(&_RxFifo_next, &data, 1))
		{
			_uart_next_fifo_received++;
		}
		else
		{
			_uart_next_fifo_error++;
		}	
	}	
	else
	{
		if(FM4_MFS2->SSR_f.PE)	//Parity Error
		{
				_uart_next_error_parity++;
		}
		if(FM4_MFS2->SSR_f.FRE)	//Framing error
		{
				_uart_next_error_framing++;
		}	
		if(FM4_MFS2->SSR_f.ORE)	//overrun error
		{
			_uart_next_error_overrun++;
		}	
		FM4_MFS2->SSR_f.REC=1;	// clear error flac
	}
	NVIC_ClearPendingIRQ(MFS2_RX_IRQn);
}

/**
 ******************************************************************************
 ** \brief  MFS2 TX IRQ handler (occurs if the tx reg is empty)
 ******************************************************************************/
void MFS2_TX_IRQHandler(void)
{
	UCHAR data;
	if (fifo_get(&_TxFifo_next, &data))
	{
		FM4_MFS2->TDR = data;
		_uart_next_sent++;
	}
	else	
	{
		FM4_MFS2->FCR1_f.FTIE=0x0u; 	 	// disable transmit interrupt
		fifo_init(&_TxFifo_next);
		_uart_next_sent_msg++;
	}
	NVIC_ClearPendingIRQ(MFS2_TX_IRQn);
}

/**
 ******************************************************************************
 ** \brief  read from the receive next fifo if it is not empty
 ** \return "data is valid"
 ******************************************************************************/
int uart_read_next(UCHAR *data)
{
	return fifo_get(&_RxFifo_next, data);
}

/**
 ******************************************************************************
 ** \brief  write data to fifo if he is not full, so check before! enable tx interrupt
 **
 ** \input	char data to write in fifo, int count
 ******************************************************************************/
void uart_write_next(UCHAR *data, int count)
{
	if (fifo_put(&_TxFifo_next, data, count))
	{
		FM4_MFS2->FCR1_f.FTIE=0x1u; 	// enable transmit interrupt
	}
}
