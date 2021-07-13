// ****************************************************************************
//
//	ctrl_svr.h
//
//	main controller tasks.
//
// ****************************************************************************
//
//	Copyright 2013 by Radex AG, Switzerland. All rights reserved.
//	Written by Marcel Galliker 
//
// ****************************************************************************

#pragma once

#include "rx_sok.h"

int  ctrl_start(void);
int  ctrl_end(void);

int  ctrl_get_device(RX_SOCKET socket, EDevice *device, int *no);
int  ctrl_is_connected(EDevice device, int no);

int  ctrl_ping_head(int no);
int	 ctrl_head_error_reset(void);

void ctrl_send_head_fluidCtrlMode(int headNo, EnFluidCtrlMode ctrlMode, int sendToFluid, int fromGui);
void ctrl_send_all_heads_fluidCtrlMode(int fluidNo, EnFluidCtrlMode ctrlMode);	// send ctrlMode to all heads connected to this fluid module
int  ctrl_send_purge_par(int fluidNo, int time, int position_check, int delay_time_ms);	// return=total time
int  ctrl_check_all_heads_in_fluidCtrlMode(int fluidNo, EnFluidCtrlMode ctrlMode);	// send ctrlMode to all heads connected to this fluid module
int  ctrl_singleHead(void);

void ctrl_update_hostname(void);
int  ctrl_set_config(void);
void ctrl_send_head_cfg(void);
void ctrl_get_density_values(RX_SOCKET socket, SValue *pmsg);
void ctrl_set_density_values(SDensityValuesMsg *pmsg);
void ctrl_set_disalbled_jets(SDisabledJetsMsg *pmsg);

int  ctrl_head_cfg_done(int headNo, UINT32 resetCnt);
void ctrl_head_cal_done(int fluidNo);
UINT32 ctrl_headResetCnt(void);
void ctrl_set_max_speed(void);
int	 ctrl_abort_printing(void);
void ctrl_send_firepulses(char *dots, int screenOnPrinter);
int  ctrl_print_page(SPageId *id);
int  ctrl_simu_encoder(int khz);

void ctrl_reply_stat(RX_SOCKET socket);
void ctrl_tick(void);

void ctrl_head_alive(int headNo);

void ctrl_empty_PurgeBuffer(int fluidNo);

void ctrl_set_rob_pos(int headNo, INT32 angle, INT32 stitch);
