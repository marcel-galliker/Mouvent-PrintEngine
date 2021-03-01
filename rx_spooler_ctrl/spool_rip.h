// ****************************************************************************
//
//	spool_rip.h
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

void sr_mnt_drive(const char *path);
void sr_mnt_path(const char *orgPath, char *mntPath);

void sr_reset();

void sr_set_filedef_start(RX_SOCKET socket, SMsgHdr *msg);
void sr_set_filedef_blk  (RX_SOCKET socket, SMsgHdr *msg);
void sr_set_filedef_end  (RX_SOCKET socket, SMsgHdr *msg);

void sr_set_layout_start(RX_SOCKET socket, char *dataPath);
void sr_set_layout_blk  (RX_SOCKET socket, SMsgHdr *msg);
void sr_set_layout_end  (RX_SOCKET socket, SMsgHdr *msg);

void sr_get_label_size	(int *width, int *length, UCHAR *bitsPerPixel);

void sr_data_record		(SPrintDataMsg *msg);

int sr_rip_label(BYTE *buffer[MAX_COLORS], SBmpInfo *pInfo, int offsetPx, int lengthPx, int blkNo, int blkCnt, const char* filepath);
void sr_rip_unused();
void sr_abort(void);
