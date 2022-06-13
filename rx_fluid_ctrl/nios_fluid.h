// ****************************************************************************
//
//	DIGITAL PRINTING - nios.h
//
//
// ****************************************************************************
//
//	Copyright 2013 by Radex AG, Switzerland. All rights reserved.
//	Written by Marcel Galliker
//
// ****************************************************************************

#pragma once

#include "rx_def.h"
//#include "rx_fluid_ctrl.h"
#include "nios_def_fluid.h"

extern char *mode(EnFluidCtrlMode mode);
extern SNiosFluidCfg	*_Cfg;

int nios_init(void);
int nios_end(void);
void nios_load(const char *exepath);

void nios_main(int ticks, int menu);

void nios_set_cfg			(SFluidBoardCfg *pcfg);
void nios_set_is_cfg		(SInkSupplyCfg *pcfg);
void nios_set_ctrlmode		(int isNo, EnFluidCtrlMode mode);
void nios_set_ctc_operation (int isNo,  int cmd, int step, int par);
// void nios_set_pressure		(int isNo, int pressure);
void nios_set_purge_par		(int isNo, int last_pos, int time, int act_pos);
void nios_set_head_state	(int isNo, SHeadStateLight *pstate);
void nios_set_temp			(int isNo, int temp);
void nios_set_purge_pressure(int isNo, int pressure);
void nios_start_log			(void);
void nios_start_temp_log	(void);
void nios_error_reset		(void);
void nios_set_degasser		(int *degas);

void nios_test_stop			(void);
void nios_test_air_valve	(int isNo, int value);
void nios_test_bleed_line	(int isNo);
void nios_test_bleed_valve	(int isNo, int value);
void nios_test_ink_pump		(int isNo, int pressure);
void nios_test_vacuum		(int pressure);
void nios_test_air_pressure	(int pressure);
void nios_test_flush		(int power);

int  nios_is_heater_connected(void);
