// ********************************************************void ********************
//
//	DIGITAL PRINTING - fpga.h
//
//	Communication between ARM processor and FPGA
//
// ****************************************************************************
//
//	Copyright 2013 by Radex AG, Switzerland. All rights reserved.
//	Written by Marcel Galliker
//
// ****************************************************************************

#pragma once

#include "rx_sok.h"
#include "tcp_ip.h"

//--- global variables ----------------------
extern SFpgaHeadBoardCfg	FpgaCfg;


//--- functions -----------------------------
void  fpga_init(char *rbfFileName);
int   fpga_is_init(void);
int	  fpga_is_ready(void);
void  fpga_master_disable();
void  fpga_end(void);
void  fpga_overheated(void);

void  fpga_main(int ticks, int menu);

void   fpga_signal_message(void);

int   fpga_nios_reset(int reset);
void  fpga_get_mac_addr(int udpNo, BYTE *addr);
void  fpga_get_ip_addr (int udpNo, UINT32 *addr);
int   fpga_udp_block_size(void);
UINT32 fpga_get_block_used(int headNo, UINT32 blkNo, UINT32 blkCnt, UINT32 *buffer);
UINT32 fpga_get_blockOutIdx(int headNo);
UINT32 fpga_get_aliveCnt(int udpNo);

int  fpga_set_config(RX_SOCKET socket);
void fpga_enc_config(int synth);
void fpga_enc_enable(int enable);
int  fpga_abort(void);
int  fpga_image	(SFpgaImageCmd *msg);

void fpga_manual_pg(void);
void fpga_print_done(void);

void fpga_display_error(void);
void fpga_display_NiosStatus(void);
void fpga_display_used_flags(void);

void fpga_trace_registers(char *fname, int error);

int	fpga_enc_position(int no);
int fpga_temp(void);
int fpga_qsys_id(void);
int fpga_qsys_timestamp(void);
int fpga_fp_clock_mhz(void);

int fpga_get_nios_shutdown(void);