// ****************************************************************************
//
//	ctrl_client.h
//
// ****************************************************************************
//
//	Copyright 2014 by Radex AG, Switzerland. All rights reserved.
//	Written by Marcel Galliker 
//
// ****************************************************************************

#pragma once

// #define CLEAR_BLOCK_USED	TRUE
#define CLEAR_BLOCK_USED	FALSE

int ctrl_start(const char *ipAddrMain);
int ctrl_end(void);

void ctrl_send(void *msg);
void ctrl_pause_printing(void);
void ctrl_start_printing(void);
void ctrl_send_load_progress(SPageId *id, const char *colorSN, int progress);