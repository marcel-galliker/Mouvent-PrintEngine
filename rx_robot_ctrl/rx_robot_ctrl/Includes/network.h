#pragma once

#include "net.h"

bool network_init(void);
void network_tick(int tick);
void network_end(void);

void network_change_ip(ip_addr_t* newIpAddress);
void network_send_boot_msg(void *message, uint32_t size);
void network_send_ctrl_msg(void* message, uint32_t size);
