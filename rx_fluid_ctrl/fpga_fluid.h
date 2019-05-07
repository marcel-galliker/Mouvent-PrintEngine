// ********************************************************void ********************
//
//	DIGITAL PRINTING - fpga_fluid.h
//
//	Communication between ARM processor and FPGA
//
// ****************************************************************************
//
//	Copyright 2015 by Radex AG, Switzerland. All rights reserved.
//	Written by Marcel Galliker
//
// ****************************************************************************

#pragma once

#include "rx_sok.h"
#include "tcp_ip.h"


//--- global variables ----------------------

//--- functions -----------------------------
void  fpga_init(void);
int   fpga_is_init(void);
void  fpga_end(void);

void  fpga_cfg(void);

void  fpga_main(int ticks, int menu);


int   fpga_load(char *path_rbf);
int	  fpga_qsys_id(void);
int	  fpga_qsys_timestamp(void);
