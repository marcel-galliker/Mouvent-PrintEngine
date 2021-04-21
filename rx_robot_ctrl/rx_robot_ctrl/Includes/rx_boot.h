#ifndef INCLUDES_RX_BOOT_H_
#define INCLUDES_RX_BOOT_H_

#include "FreeRTOS.h"

bool rx_boot_start(void);
void rx_boot_handle_command(void* msg);
int  rx_boot_main(TickType_t tick);

#endif /* INCLUDES_RX_BOOT_H_ */
