#pragma once
#include "rx_def.h"
#include "rx_error.h"

void robi_init(void);
void robi_main(int ticks, int menu);
void robi_menu(int help);
void robi_handle_menu(char *str);
void robi_display_status(void);
int robi_handle_ctrl_msg(RX_SOCKET socket, int msgId, void *pdata);
//int robi_connected(void);