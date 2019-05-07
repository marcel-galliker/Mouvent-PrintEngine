/******************************************************************************/
/** \file uart.c
 **
 ** uart functions to communicate between nios and conditioner board
 **
 **	Copyright 2016 by radex AG, Switzerland. All rights reserved.
 **	Written by Stefan Weber
 **
 ******************************************************************************/

#include "rx_mem.h"
#include "uart.h"

#define UART_CNT	4

// buffer type
typedef struct
{
	SFifo	rx_fifo;
	SFifo	tx_fifo;
	int 	base;
    int		rxCnt;
    int		txCnt;
} SUart;

// SUart	_Uart[UART_CNT];
SUart	*_Uart;//[UART_CNT];

// Local Prototype
void handle_uart_interrupt(void* context);
static void _uart_enable_tx_interrupt(int base);
static void _uart_disable_tx_interrupt(int base);

/**
 ******************************************************************************
 ** \brief  init uart function
 **
 ******************************************************************************/
void uart_init(void)
{
	int i;

	_Uart	= (SUart*)mem_alloc(UART_CNT*sizeof(SUart));

	//Set base
	_Uart[0].base=UART_0_BASE;
	_Uart[1].base=UART_1_BASE;
	_Uart[2].base=UART_2_BASE;
	_Uart[3].base=UART_3_BASE;

	for (i=0; i<UART_CNT; i++)
	{
		_Uart[i].rxCnt=0;
		_Uart[i].txCnt=0;
		fifo_init(&_Uart[i].rx_fifo);
		fifo_init(&_Uart[i].tx_fifo);
	}

	// Register ISR
    alt_irq_register(UART_0_IRQ, (void*)0, handle_uart_interrupt);	//priority 1
    alt_irq_register(UART_1_IRQ, (void*)1, handle_uart_interrupt);	//priority 2
    alt_irq_register(UART_2_IRQ, (void*)2, handle_uart_interrupt);	//priority 3
    alt_irq_register(UART_3_IRQ, (void*)3, handle_uart_interrupt);	//priority 4

    for (i=0; i<UART_CNT; i++)
    {
    	uart_set_baudrate(i, divisor_115200_baud);
    	_uart_enable_tx_interrupt(_Uart[i].base);
    }
}
/**
 ******************************************************************************
 ** \brief  change uart baudrate function
 **
 ** \input int base and int new_divisior
 **
 ******************************************************************************/
// Baudrate calc
// baudrate=(clock_freq/(divisor+1))
// clock_freq=50 MHz => avalon bus freq.
// baud 1 = 9600
// baud 2 = 115200
// divisor=clock_freq/baudrate -1
// divisor1=50MHz/9600 -1 = 5207.33 => 5207
// divisor2=50MHz/115200 -1 = 433.028 => 433 => verwende 434 !

void uart_set_baudrate(int uartNo, int divider)
{
	int base = _Uart[uartNo].base;

	if(IORD_ALTERA_AVALON_UART_DIVISOR(base)!=divider)
	{
		IOWR_ALTERA_AVALON_UART_CONTROL(base, 0);	// disable interrupts
		IOWR_ALTERA_AVALON_UART_DIVISOR(base, divider);
		fifo_init(&_Uart[uartNo].tx_fifo);
	    fifo_init(&_Uart[uartNo].rx_fifo);
//    	_uart_enable_tx_interrupt(_Uart[uartNo].base);
	    _uart_disable_tx_interrupt(_Uart[uartNo].base);
	}
}

//--- uart_read --------------------------------
int  uart_read(int no, UCHAR *data)
{
	return fifo_get(&_Uart[no].rx_fifo, data);
}

//--- uart_write -------------------------------
int uart_write  (int no, UCHAR *data, int count)
{
	if (fifo_put(&_Uart[no].tx_fifo, data, count))
	{
		_uart_enable_tx_interrupt(_Uart[no].base);
		return TRUE;
	}
	return FALSE;
}

//--- uart_write_byte -------------------------------
int uart_write_byte  (int no, UCHAR data)
{
	if (fifo_put(&_Uart[no].tx_fifo, &data, 1))
	{
		_uart_enable_tx_interrupt(_Uart[no].base);
		return TRUE;
	}
	return FALSE;
}

//--- uart_wait_sent -----------------------
void uart_wait_sent	 (int no)
{
	while (_Uart[no].tx_fifo.wrIdx != _Uart[no].tx_fifo.rdIdx);
}

/**
 ******************************************************************************
 ** \brief enable uart tx interrupt function
 **
 ******************************************************************************/
static void _uart_enable_tx_interrupt(int base)
{
	IOWR_ALTERA_AVALON_UART_CONTROL(base, ALTERA_AVALON_UART_CONTROL_RRDY_MSK | ALTERA_AVALON_UART_CONTROL_TRDY_MSK);
}

/**
 ******************************************************************************
 ** \brief enable uart rx interrupt function
 **
 ******************************************************************************/
static void _uart_disable_tx_interrupt(int base)
{
	IOWR_ALTERA_AVALON_UART_CONTROL(base, ALTERA_AVALON_UART_CONTROL_RRDY_MSK);
}

/**
 ******************************************************************************
 ** \brief uart 0 interrupt handler (rx and tx)
 **
 ******************************************************************************/
void handle_uart_interrupt(void* context)
{
	int		uartNo = (int)context;
	SUart  *puart = &_Uart[uartNo];
	int		base  = puart->base;
	alt_u32 status;
	UCHAR 	data;

    status 	= IORD_ALTERA_AVALON_UART_STATUS(base);

    /*
    if(status & ALTERA_AVALON_UART_STATUS_PE_MSK) arm_ptr->stat.cond[uartNo].error.parity_error++;
    if(status & ALTERA_AVALON_UART_STATUS_FE_MSK) arm_ptr->stat.cond[uartNo].error.framing_error++;
    if(status & ALTERA_AVALON_UART_STATUS_BRK_MSK)arm_ptr->stat.cond[uartNo].error.break_detected++;
    if(status & ALTERA_AVALON_UART_STATUS_ROE_MSK)arm_ptr->stat.cond[uartNo].error.receive_overrun++;
    if(status & ALTERA_AVALON_UART_STATUS_TOE_MSK)arm_ptr->stat.cond[uartNo].error.transmit_overrun++;
    */
    // RX Ready Interrupt
    if(status & ALTERA_AVALON_UART_STATUS_RRDY_MSK)
    {
		data=IORD_ALTERA_AVALON_UART_RXDATA(base);
    	if(!((status &ALTERA_AVALON_UART_STATUS_PE_MSK)||(status&ALTERA_AVALON_UART_STATUS_FE_MSK)))	//only valid data if there is no framing or parity error
    	{
    		fifo_put(&puart->rx_fifo, &data, 1);
    	}
    	// Clear the RRDY bit in the status register (read-modify-write)
        status 	&= ~ALTERA_AVALON_UART_STATUS_RRDY_MSK;
        IOWR_ALTERA_AVALON_UART_STATUS(base, status);
    }
    // TX Ready Interrupt
    else if(status & ALTERA_AVALON_UART_STATUS_TRDY_MSK)
    {
    	if (fifo_get(&puart->tx_fifo, &data))
    	{
    		IOWR_ALTERA_AVALON_UART_TXDATA(base, data);
    	}
    	else
    	{
    		_uart_disable_tx_interrupt(base);
    	}
    	// Clear the TRDY bit in the status register (read-modify-write)
        status 	&= ~ALTERA_AVALON_UART_STATUS_TRDY_MSK;
        IOWR_ALTERA_AVALON_UART_STATUS(base, status);
    }
}

