// ****************************************************************************
//
//	DIGITAL PRINTING - daisy_chain.h
//
//	Communication between Fluid and UART connected boards
//
// ****************************************************************************
//
//	Copyright 2018 by Mouvent AG, Switzerland. All rights reserved.
//	Written by Patrick Walther
//
// ****************************************************************************
#pragma once

//--- includes ----------------------------------------------------------------
#include "rx_def.h"
#include "daisy_chain_reg.h"
#include "daisy_chain_def.h"

int daisy_chain_init(void);
int daisy_chain_end(void);
int daisy_chain_main(int ticks, int menu, int displayStatus);