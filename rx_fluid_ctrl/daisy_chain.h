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

void daisy_chain_set_tara(INT32 *tara, int cnt);
void daisy_chain_get_tara(INT32 *tara, int cnt);
void daisy_chain_get_weight(INT32 *weight, int cnt);

void daisy_chain_do_tara(int no);

int daisy_chain_is_active(void);

int daisy_chain_init(void);
int daisy_chain_end(void);
int daisy_chain_main(int ticks, int menu, int displayStatus);