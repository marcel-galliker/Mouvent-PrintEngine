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
#include "fpga_def_stepper.h"


//--- global variables ----------------------
extern SStepperFpga	Fpga;

//--- functions -----------------------------
void  fpga_init(void);
int   fpga_is_init(void);
void  fpga_end(void);

int   fpga_load(char *path_rbf);

void  fpga_main(int ticks, int menu);
int	  fpga_input(int no);
int	  fpga_pwm_speed(int no);
void  fpga_stepper_error_reset();
