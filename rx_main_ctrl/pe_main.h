// ****************************************************************************
//
//	pe_main.h		
//
// ****************************************************************************
//
//	Copyright 2018 by Mouvent AG, Switzerland. All rights reserved.
//	Written by Marcel Galliker 
//
// ****************************************************************************

#pragma once

#include "rx_sok.h"

int pem_init(void);
int pem_end(void);
int pem_set_config(void);
int pem_send_printer_status(void);

int	pem_print_file_evt	(RX_SOCKET socket, int headNo, SPrintFileMsg	*msg);
