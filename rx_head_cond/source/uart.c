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
#include "cond_def_head.h"
#include "fifo.h"
#include "comm.h"
#include "pump_ctrl.h"
#include "uart.h"

static SFifo	_RxFifo;
static SFifo	_TxFifo;

static int _uart_sent=0;
static int _uart_msg_sent=0;
static int _uart_msg_received=0;

static int _uart_error_parity = 0;
static int _uart_error_framing= 0;
static int _uart_error_overrun= 0;	
	
//--- prototypes --------------------------------------------------------------
static void InitMfs0(UINT32 baudrate);

void RxMessage_Handler(void);	// implemented in MAIN.C

/**
 ******************************************************************************
 ** \brief  init uart function
 ******************************************************************************/
void uart_init(UINT32 baudrate)
{
	fifo_init(&_RxFifo);	
	fifo_init(&_TxFifo);	
	InitMfs0(baudrate);
}

/**
 ******************************************************************************
 **  \brief       Initialization of MFS0 in UART-Mode
 ******************************************************************************/
static void InitMfs0(UINT32 baudrate)
{
	// Initialize ports for MFS0
	FM4_GPIO->PFR2   = 0x06u;                   // P21>SIN0_0, P22>SOT0_0
	FM4_GPIO->EPFR07 &= 0xFFFFFF0Ful;
	FM4_GPIO->EPFR07 |= 0x00000040ul;
	FM4_GPIO->ADE &= ~(1ul << 17ul);            // switch off AN17 for SIN0_0
	FM4_GPIO->ADE &= ~(1ul << 16ul);            // switch off AN16 for SOT0_0
  
	// Initialize MFS to UART asynchronous mode
	FM4_MFS0->SMR =  0x01u;                     // serial data output enable, operation mode 0
	FM4_MFS0->SCR =  0x80u;                     // MFS reset
	FM4_MFS0->SSR =  0x00u;                     // 0x80 = clear receive error flag
	FM4_MFS0->ESCR = 0x00u;          
	FM4_MFS0->SCR_f.RIE=0x01u;									// enable received interrupt 
	
	//rx/tx FIFO each = 64 Byte
	//Max. FBYTE Count = 64
	//Count of Data to be Stored
	FM4_MFS0->FCR1_f.FSEL=0x00u;								// FIFO2 Rx, FIFO1 Tx 
	FM4_MFS0->FCR1_f.FLSTE=0x0u;								// disable Data lost detection
	
	FM4_MFS0->FCR0_f.FCL1=0x01u; 								//reset fifo1
	//FM4_MFS0->FCR0_f.FCL2=0x01u; 							    //reset fifo2 (not used, as we do not use a rx fifo)
	
	FM4_MFS0->FCR1_f.FDRQ=0x0u;									// no irq if tx fifo is empty (change later if you put data in tx fifo)
	FM4_MFS0->FCR1_f.FTIE=0x1u; 								// enable transmit fifo interrupt
	
	FM4_MFS0->FBYTE1=0x00u; 									// set fifo 1 tx value 
	//FM4_MFS0->FBYTE2=0x01u; 									//fifo 1 rx interrupt after 1	byte received

	FM4_MFS0->FCR0_f.FE1=0x01;									//enable fifo1
	//FM4_MFS0->FCR0_f.FE2=0x01;								//enable fifo2 (not used, as we do not use a rx fifo)

	FM4_MFS0->BGR = 20000000ul / baudrate - 1;                  // 115.2kbaud @ 20 MHz
	//FM4_MFS0->BGR = 20000000ul / 19200ul - 1;                 // 19.2kbaud @ 20 MHz
	//FM4_MFS0->BGR = 20000000ul / 9600ul - 1;                  // 9.6kbaud @ 20 MHz
	FM4_MFS0->SCR |= 0x03u;                                     // RX, TX enable  
	
	// MFS0 transmit interrupt settings 
	NVIC_ClearPendingIRQ(MFS0_TX_IRQn);
	NVIC_EnableIRQ(MFS0_TX_IRQn);
	NVIC_SetPriority(MFS0_TX_IRQn, 14u);
	
	// MFS0 receive interrupt settings
	NVIC_ClearPendingIRQ(MFS0_RX_IRQn);
	NVIC_EnableIRQ(MFS0_RX_IRQn);
	NVIC_SetPriority(MFS0_RX_IRQn, 14u);	
}

/**
 ******************************************************************************
 ** \brief  MFS0 RX IRQ handler (occurs if there is new data)
 ******************************************************************************/
void MFS0_RX_IRQHandler(void)
{	
	if (FM4_MFS0->SSR_f.RDRF)
	{
		UCHAR data=FM4_MFS0->RDR;
		if (comm_received(0, data))
		{
			int length;
			if (comm_get_data(0, (UCHAR*)&RX_Config, sizeof(RX_Config), &length))
			{
				_uart_msg_received++;
				RxMessage_Handler();
			}
		}
	}
	else
	{
		if(FM4_MFS0->SSR_f.PE)  _uart_error_parity++;
		if(FM4_MFS0->SSR_f.FRE)	_uart_error_framing++;
		if(FM4_MFS0->SSR_f.ORE)	_uart_error_overrun++;
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
	if (fifo_get(&_TxFifo, &data))
	{
		FM4_MFS0->TDR = data;
		_uart_sent++;
	}
	else	
	{
		FM4_MFS0->FCR1_f.FTIE=0x0u; 	 	// disable transmit interrupt
		_uart_msg_sent++;
	}
	NVIC_ClearPendingIRQ(MFS0_TX_IRQn);
}

/**
 ******************************************************************************
 ** \brief  write data to fifo if he is not full, so check before! enable tx interrupt
 **
 ** \input	RingFifo_t * pointer to ringFifo, char data to write in fifo
 ******************************************************************************/
void uart_write(UCHAR *data, int count)
{
	if (fifo_put(&_TxFifo, data, count))
	{
		FM4_MFS0->FCR1_f.FTIE=0x1u; 	// enable transmit interrupt
	}
}

/**
 ******************************************************************************
 ** \brief  read from the receive fifo if it is not empty
 ** \return "data is valid"
 ******************************************************************************/
int uart_read(UCHAR *data)
{
	return fifo_get(&_RxFifo, data);
}

