#include "rx_boot.h"
#include "rx_boot_def.h"

#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "sys/types.h"

#include <ft900.h>

#include "FreeRTOS.h"
#include "task.h"

#include "net.h"

#include "robot_flash.h"
#include "network_manager.h"

/* Defines */

// Broadcast settings
#define BROADCAST_INTERVAL	1000

// Robot device information
#define DEVICE_ROBOT			0x08
#define DEVICE_NAME				"Robot"

// UDP boot commands
#define CMD_BOOT_INFO_REQ		0x11000001
#define REP_BOOT_INFO			0x12000001

#define CMD_BOOT_ADDR_SET		0x11000002
#define CMD_BOOT_FLASH_ON		0x11000003
#define CMD_BOOT_PING			0x11000004
#define REP_BOOT_PING			0x12000004

// MAC serial number mask
#define MAC_NO_MASK				0x0000ffff00000000


/* Static variables */


static NetworkItem_t 	_item;

// Status flags
static bool _isAddressConfirmed = false;

static bool _isInitialized = false;

static TickType_t _BroadcastTime;


/* Prototypes */
static void rx_boot_send_info(uint32_t id);

bool rx_boot_start(void)
{
	// Set Device name and get mac address
	memcpy(_item.deviceTypeStr, DEVICE_NAME, sizeof(_item.deviceTypeStr));
	_item.deviceType = DEVICE_ROBOT;
	memcpy(&_item.macAddr, net_get_mac(), NETIF_MAX_HWADDR_LEN);

	// Get serial number from MAC
	sprintf((char *)_item.serialNo, "%d", flash_read_serialNo());

	// Get ip address
	ip_addr_t ip = net_get_ip();
	ipaddr_ntoa_r(&ip, (char *)_item.ipAddr, sizeof(_item.ipAddr));

	_BroadcastTime = xTaskGetTickCount();

	_isInitialized = true;
	return true;
}

int rx_boot_main(TickType_t tick)
{
	if (_isInitialized && network_manager_is_initialized() && !_isAddressConfirmed && tick>_BroadcastTime)
	{
		rx_boot_send_info(REP_BOOT_INFO);
		_BroadcastTime = tick+BROADCAST_INTERVAL;
		return 1;
	}
	return 0;
}

static void rx_boot_send_info(uint32_t id)
{
	BootInfoMsg_t message;
	// Broadcast current network status
	message.id = id;
	memcpy(&message.item, &_item, sizeof(message.item));
	
	network_manager_broadcast(&message, sizeof(message));
}

void rx_boot_handle_command(void* msg)
{
	BootAddrSetCmd_t* commandData = NULL;
	uint32_t *command = (uint32_t *)msg;

	switch(*command)
	{
	case CMD_BOOT_INFO_REQ:
		_isAddressConfirmed = false;
		rx_boot_send_info(REP_BOOT_INFO);
		break;

	case CMD_BOOT_PING:
		rx_boot_send_info(REP_BOOT_PING);
		break;

	case CMD_BOOT_ADDR_SET:
		commandData = (BootAddrSetCmd_t*)msg;

		if(commandData->macAddr == _item.macAddr)
		{
			// Convert incoming IP from ASCII to ip_addr struct
			ip_addr_t newIp;
			ipaddr_aton((const char*)commandData->ipAddr, &newIp);

			// Change IP address
			network_manager_change_ip(&newIp);

			// Update NetworkItem with new IP
			ip_addr_t ip = net_get_ip();
			ipaddr_ntoa_r(&ip, (char *)_item.ipAddr, sizeof(_item.ipAddr));
			_isAddressConfirmed = true;
		}
		break;

	case CMD_BOOT_FLASH_ON:
		break;

	default:
		break;
	}
}

