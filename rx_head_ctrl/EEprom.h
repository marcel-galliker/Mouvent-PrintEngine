// ****************************************************************************
//
//	DIGITAL PRINTING - EEprom.h
//
//	
//
// ****************************************************************************
//
//	Copyright 2013 by Radex AG, Switzerland. All rights reserved.
//	Written by Marcel Galliker
//
// ****************************************************************************

#pragma once
#include "rx_common.h"
#include "rx_def.h"
#include "tcp_ip.h"


void eeprom_init_data(int headNo, BYTE *eeprom, SHeadEEpromInfo *pInfo);

void eeprom_display(void);
int  eeprom_main(int ticks, int menu);
void eeprom_read(void);

void eeprom_set_clusterNo(INT32 clusterNo);
void eeprom_set_flowResistance(int headNo, int value);
void eeprom_reset_droplets_printed(int headNo);
void eeprom_add_droplets_printed(int headNo, UINT32 droplets, int time);
void eeprom_set_rob_pos(int headNo, int angle, int dist);
void eeprom_set_density(SDensityMsg *pmsg);
void eeprom_display_user(void);
void eeprom_test(int head);