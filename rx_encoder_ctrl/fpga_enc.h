// ********************************************************void ********************
//
//	DIGITAL PRINTING - fpga_enc.h
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
// extern SFpga				Fpga;

//--- functions -----------------------------
void  fpga_init(void);
int   fpga_is_init(void);
void  fpga_end(void);

void  fpga_main(int ticks, int menu, int showCorrection, int showParam);

int   fpga_load(char *path_rbf);
void* fpga_map_page(UINT64 addr, UINT32 size, UINT32 size_expected);

void  fpga_enc_config(int encNo, SEncoderCfg  *pCfg, int outNo, int synth, int retsart);
void  fpga_enc_config_test(void);
void  fpga_enc_simu(int khz);
void  fpga_encoder_enable(int enable);
void  fpga_shift_delay(int strokes);
void  fpga_encoder_reset_reg(void);
int   fpga_pg_config(RX_SOCKET socket, SEncoderCfg *pcfg, int restart);
int   fpga_pg_config_fhnw(UINT32 posActual, UINT32 posFwd, UINT32 posBwd);

void  fpga_pg_init(int restart);
void  fpga_pg_stop(void);
void  fpga_pg_set_dist(int cnt, int dist);
void  fpga_set_printmark(int window, int ignore);

void  fpga_stop_printing();
void  fpga_abort_printing();
void  fpga_output(int no);
void  fpga_uv_on(void);
void  fpga_uv_off(void);


// void  fpga_reset_pos(void);
