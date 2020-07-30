// ****************************************************************************
//
//	DIGITAL PRINTING - conditioner.h
//
//
// ****************************************************************************
//
//	Copyright 2016 by Radex AG, Switzerland. All rights reserved.
//	Written by Stefan Weber
//
// ****************************************************************************

#pragma once

#include "rx_def.h"
#include "nios_def_head.h"
#include "fpga.h"

extern SFluidStateLight		RX_FluidStat[MAX_HEADS_BOARD];

int  cond_init(void);
int  cond_end(void);
void cond_shutdown(void);
void cond_main(int ticks, int menu);

int  cond_restart_loader(int condNo);
int  cond_restart_app(int condNo);

void cond_error_check(void);
void cond_error_reset(void);

void cond_resetPumpTime(int condNo);

int _cond_load(const char *exepath);
// void cond_display_status(int show, int status);
// void cond_handle_status(int headNo, SConditionerStat_mcu *status, int len);

void cond_ctrlMode		(int headNo, EnFluidCtrlMode ctrlMode);
void cond_ctrlMode2		(int headNo, EnFluidCtrlMode ctrlMode);
EnFluidCtrlMode cond_getCtrlMode(int headNo);
void cond_heater_test	(int temp);
void cond_toggle_meniscus_check(void);
void cond_set_clusterNo(INT32 no);
void cond_set_flowResistance(int headNo, int value);
void cond_set_disabledJets(int headNo, INT16 *jets);
void cond_set_densityValues(int headNo, INT16 *values);
void cond_set_voltage(int headNo, UINT8 voltage);
void cond_set_rob_pos(int headNo, int angle, int dist);
void cond_set_purge_par (int headNo, int delay, int time);
void cond_add_droplets_printed(int headNo, UINT64 droplets);
void cond_set_config	(int headNo, SConditionerCfg *cfg);
void cond_setInk		(int headNo, SInkDefinition *pink);
void cond_volume_printed(int headNo, int volume);
void cond_start_preslog (void);
ELogItemType cond_err_level		(void);

void cond_start_log(void);
void cond_offset_cal(int headNo);
void cond_offset_del(int headNo);
void cond_toggle_psensor_cali(int headNo);
void cond_toggle_psensor_cali_user(int headNo);
    