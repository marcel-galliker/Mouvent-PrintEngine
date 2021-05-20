
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "sys/types.h"

#include <ft900.h>
#include "net.h"

#include "rx_timer.h"
#include "network.h"
#include "rx_boot.h"
#include "rx_boot_def.h"

#include "robot_flash.h"

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

static SNetworkItem 	_item;

static bool _isAddressConfirmed = false;

static UINT32 _BroadcastTime;


/* Prototypes */
static void _init(void);
static void _send_info(uint32_t id);

void _init(void)
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

	_BroadcastTime = rx_get_ticks();
}

int rx_boot_main(uint32_t tick)
{
	if (!_isAddressConfirmed && tick>_BroadcastTime)
	{
		if (_item.macAddr==0)
			_init();
		_send_info(REP_BOOT_INFO);
		_BroadcastTime = tick+BROADCAST_INTERVAL;
		return 1;
	}
	return 0;
}

static void _send_info(uint32_t id)
{
	SBootInfoMsg message;

	message.id = id;
	memcpy(&message.item, &_item, sizeof(message.item));

	network_send_boot_msg(&message, sizeof(message));
}

//--- rx_boot_handle_msg -----------------------
void rx_boot_handle_msg(void* msg)
{
	SBootAddrSetCmd* commandData = NULL;
	uint32_t *command = (uint32_t *)msg;

	switch(*command)
	{
	case CMD_BOOT_INFO_REQ:
		_isAddressConfirmed = false;
		_send_info(REP_BOOT_INFO);
		break;

	case CMD_BOOT_PING:
		_send_info(REP_BOOT_PING);
		break;

	case CMD_BOOT_ADDR_SET:
		commandData = (SBootAddrSetCmd*)msg;

		if(commandData->macAddr == _item.macAddr)
		{
			// Convert incoming IP from ASCII to ip_addr struct
			ip_addr_t newIp;
			ipaddr_aton((const char*)commandData->ipAddr, &newIp);

			// Change IP address
			network_change_ip(&newIp);

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

