/******************************************************************************/
/** \file uart.h
 **
 ** uart functions to communicate between nios and conditioner board
 **
 **	Copyright 2016 by radex AG, Switzerland. All rights reserved.
 **	Written by Stefan Weber
 **
 ******************************************************************************/

#ifndef UART_H_
#define UART_H_

#include "system.h"
#include <stdio.h>
#include "alt_types.h"
#include "sys/alt_irq.h"
#include "priv/alt_legacy_irq.h"
#include "altera_avalon_timer.h"
#include "altera_avalon_timer_regs.h"
#include "altera_avalon_pio_regs.h"
#include <unistd.h>
#include "sys/alt_stdio.h"
#include "system.h"
#include "io.h"
#include "sys/alt_alarm.h"
#include "timer.h"
#include "altera_avalon_uart.h"
#include "altera_avalon_uart_fd.h"
#include "altera_avalon_uart_regs.h"
#include "nios_def_head.h"
#include "altera_avalon_jtag_uart_regs.h"

#include "fifo.h"
//source http://stackoverflow.com/questions/6822548/correct-way-of-implementing-a-uart-receive-buffer-in-a-small-arm-microcontroller
#define divisor_9600_baud 5207
#define divisor_19200_baud 2603
#define divisor_115200_baud 434


// Buffer read / write macros

void uart_init();

void uart_set_baudrate(int uartNo, int new_divider);

int  uart_write      (int no, UCHAR *data, int count);
int  uart_write_byte (int no, UCHAR data);
int  uart_read       (int no, UCHAR *data);
void uart_wait_sent	 (int no);

extern SNiosMemory *arm_ptr;

#endif /* UART_H_ */
