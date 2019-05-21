// ****************************************************************************
//
//	DIGITAL PRINTING - tw8.h
//
//	head-control-client
//
// ****************************************************************************
//
//	Copyright 2018 by Radex AG, Switzerland. All rights reserved.
//	Written by Marcel Galliker
//
// ****************************************************************************

#pragma once

#include "rx_common.h"

int tw8_init(void);
int tw8_end(void);
int tw8_main(int ticks, int menu);
int tw8_menu_print(void);
int tw8_menu_cmd(char *str);

int		tw8_write(int chip, UINT32 addr, BYTE len, void *data);
int		tw8_read (int chip, UINT32 addr, BYTE len);

int		tw8_config(int chip, int speed_mmin, EPrinterType printerType);

int		tw8_present(void);
void	tw8_reset_error(void);


