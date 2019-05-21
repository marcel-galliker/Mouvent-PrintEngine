// ****************************************************************************
//
//	DIGITAL PRINTING - udp_test.h
//
// ****************************************************************************
//
//	Copyright 2013 by Radex AG, Switzerland. All rights reserved.
//	Written by Marcel Galliker
//
// ****************************************************************************

#pragma once

void udp_test_init(void);
void udp_test_send_block(int headCnt, int blkCnt);
void udp_test_print(char *fname);
void udp_test_print_tif(char *fname);
int  udp_test_send(void *data, int dataLen);
int  udp_test_sent_blocks(void);
int  udp_test_sent_alive(int no);

const char *udp_test_fname(void);