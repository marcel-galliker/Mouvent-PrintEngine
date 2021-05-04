#include "rx_boot.h"
#include "rx_boot_def.h"

#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "sys/types.h"

#include <ft900.h>

#include "tinyprintf.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "net.h"

#include "network.h"

/* Defines */

// Broadcast settings
#define BROADCAST_INTERVAL	1000

// Robot device information
#define DEVICE_ROBOT			0x08
#define DEVICE_STRING			"Robot"

// UDP boot commands
#define CMD_BOOT_INFO_REQ		0x11000001
#define REP_BOOT_INFO			0x12000001

#define CMD_BOOT_ADDR_SET		0x11000002
#define CMD_BOOT_FLASH_ON		0x11000003
#define CMD_BOOT_PING			0x11000004
#define REP_BOOT_PING			0x12000004

// MAC serial number mask
#define MAC_NO_MASK				0x0000ffff00000000

// Task settings
#define TASK_BOOT_STACK_SIZE		(500)
#define TASK_BOOT_PRIORITY			(6)

#define TASK_BROADCAST_STACK_SIZE	(500)
#define TASK_BROADCAST_PRIORITY		(6)


/* Static variables */

static QueueHandle_t _rxBootMessageQueue;

// Task handles
static TaskHandle_t _rxBootTask;
static TaskHandle_t _rxBootBroadcastTask;

// Broadcast status and device name
static NetworkItem_t 	_item;
static const uint8_t 	_deviceName[] = DEVICE_STRING;

// Status flags
static bool _isAddressConfirmed = false;

/* Prototypes */
static void rx_boot_task(void *pvParameters);
static void rx_boot_broadcast_task(void *pvParameters);
static void rx_boot_send_info(uint32_t id);
static void rx_boot_handle_command(void* msg);


bool rx_boot_start(void) {
	// TODO: Check if network is initalized

	_rxBootMessageQueue = network_get_boot_message_queue();

	if(_rxBootMessageQueue == NULL)
		chip_reboot();

	xTaskCreate(rx_boot_task,
			"Boot",
			TASK_BOOT_STACK_SIZE,
			NULL,
			TASK_BOOT_PRIORITY,
			&_rxBootTask);

	xTaskCreate(rx_boot_broadcast_task,
			"Broadcast",
			TASK_BROADCAST_STACK_SIZE,
			NULL,
			TASK_BROADCAST_PRIORITY,
			&_rxBootBroadcastTask);

	return true;
}

static void rx_boot_task(void *pvParameters) {
	while(true)
	{
		if(_rxBootMessageQueue != NULL)
		{
			void* message = NULL;
			if(xQueueReceive(_rxBootMessageQueue, &message, portMAX_DELAY) == pdPASS)
			{
				rx_boot_handle_command(message);
				vPortFree(message);
			}
		}
	}
}

static void rx_boot_broadcast_task(void *pvParameters) {
	static TickType_t lastWakeTime;
	static const TickType_t frequency = BROADCAST_INTERVAL / portTICK_PERIOD_MS;

	// TODO: Find a better way to implement this (maybe task signals)
	while(network_is_initialized() == false){
		vTaskDelay(100 / portTICK_PERIOD_MS);
	}

	// Set Device name and get mac address
	memset(&_item, 0, sizeof(_item));
	memcpy(_item.deviceTypeStr, _deviceName, sizeof(_deviceName));
	_item.deviceType = DEVICE_ROBOT;
	memcpy(&_item.macAddr, net_get_mac(), NETIF_MAX_HWADDR_LEN);

	// Get serial number from MAC
	sprintf((char *)_item.serialNo, "%d", (int)(__bswap64(_item.macAddr & MAC_NO_MASK) >> 16));

	// Get ip address
	ip_addr_t ip = net_get_ip();
	ipaddr_ntoa_r(&ip, (char *)_item.ipAddr, sizeof(_item.ipAddr));

	lastWakeTime = xTaskGetTickCount();

	while(true)
	{
		if(!_isAddressConfirmed)
		{
			rx_boot_send_info(REP_BOOT_INFO);
		}

		vTaskDelayUntil(&lastWakeTime, frequency);
	}
}

static void rx_boot_send_info(uint32_t id)
{
	static BootInfoMsg_t message;

	// Broadcast current network status
	message.id = id;
	memcpy(&message.item, &_item, sizeof(message.item));
	network_broadcast(&message, sizeof(message));
}

static void rx_boot_handle_command(void* msg)
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

