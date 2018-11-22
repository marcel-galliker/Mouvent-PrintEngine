#ifndef _DHCP_IP_ADDR_H
#define _DHCP_IP_ADDR_H

#include "rx_common.h"

int dhcp_addr_init(char *path);
int dhcp_addr_get_cfg(struct network_config *pcfg);

#endif
