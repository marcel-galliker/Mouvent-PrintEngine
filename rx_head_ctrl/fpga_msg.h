// ********************************************************void ********************
//
//	DIGITAL PRINTING - fpga_msg.h
//
//	Communication between ARM processor and FPGA
//
// ****************************************************************************
//
//	Copyright 2016 by Radex AG, Switzerland. All rights reserved.
//	Written by Marcel Galliker
//
// ****************************************************************************

#pragma once

#include "rx_sok.h"
#include "tcp_ip.h"


void  fpga_handle_msg(RX_SOCKET sender, void *msg);
