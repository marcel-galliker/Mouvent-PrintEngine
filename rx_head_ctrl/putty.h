// ****************************************************************************
//
//	DIGITAL PRINTING - putty.h
//
//
// ****************************************************************************
//
//	Copyright 2018 by Mouvent AG, Switzerland. All rights reserved.
//	Written by Marcel Galliker
//
// ****************************************************************************

#pragma once

void putty_init(void);

char *putty_input(char *str, int size);
void putty_handle_menu(char *str);	

void putty_display_fpga_status(void);
void putty_display_fpga_error(void);
void putty_display_nios_status(int nios, int status, int eeprom);
void putty_display_cond_status(int show, int status);