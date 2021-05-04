/*
 * network_manager.h
 *
 *  Created on: 22 Feb 2021
 *      Author: Patrick
 */

#ifndef INCLUDES_network_H_
#define INCLUDES_network_H_

#include <stdbool.h>
#include <stdint.h>

#include "lwip/sockets.h"
#include "lwip/ip4_addr.h"

#include "FreeRTOS.h"
#include "queue.h"

bool network_start();
bool network_is_initialized(void);
void network_change_ip(ip_addr_t* newIpAddress);
void network_broadcast(void *message, uint32_t size);
void network_send(void* message, uint32_t size);

QueueHandle_t network_get_boot_message_queue(void);
QueueHandle_t network_get_bootloader_message_queue(void);

#endif /* INCLUDES_network_H_ */
