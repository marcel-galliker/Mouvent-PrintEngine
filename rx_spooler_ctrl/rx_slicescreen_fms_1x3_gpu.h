// ****************************************************************************
//
//	rx_screen_fms_1x3.h
//
// ****************************************************************************
//
//	Copyright 2017 by Radex AG, Switzerland. All rights reserved.
//	Written by Gilbert Therville 
//
// ****************************************************************************

#pragma once
#ifdef __cplusplus
extern "C"{
#endif

#include "rx_def.h"
#include "rx_slicescreen.h"

// --- Prototypes --------------------------------------------
int gpu_is_board_present(void);
int gpu_init(void);
int gpu_blk_size(void);
int gpu_stream_cnt(void);
int gpu_screen_FMS_1x3g(SSLiceInfo *inplane, SSLiceInfo *outplane, void *epplaneScreenConfig, const char *dots, int threadNo);
int gpu_time(int no);

#ifdef __cplusplus
}
#endif