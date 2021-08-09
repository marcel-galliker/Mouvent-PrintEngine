#pragma once
#include "rx_def.h"
#include "rx_error.h"

void robi_lb702_init(void);
void robi_lb702_main(int ticks, int menu);
int robi_lb702_handle_ctrl_msg(RX_SOCKET socket, int msgId, void *pdata);
int robi_lb702_screw_correction(void);
int robi_lb702_screw_edgeCnt(void);