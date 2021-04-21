/*
 * network_manager.h
 *
 *  Created on: 22 Feb 2021
 *      Author: Patrick
 */

#ifndef INCLUDES_NETWORK_MANAGER_H_
#define INCLUDES_NETWORK_MANAGER_H_

#include <stdbool.h>
#include <stdint.h>

#include "lwip/sockets.h"
#include "lwip/ip4_addr.h"

#include "FreeRTOS.h"

bool network_manager_start();
bool network_manager_is_initialized(void);
void network_manager_change_ip(ip_addr_t* newIpAddress);
void network_manager_broadcast(void *message, uint32_t size);
void network_manager_send(void* message, uint32_t size);

#endif /* INCLUDES_NETWORK_MANAGER_H_ */
