#include "network.h"

#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "sys/types.h"

#include <ft900.h>
#include <gpio.h>
#include <motor.h>
#include <status.h>

#include "tinyprintf.h"

#include "lwip/sockets.h"
#include "lwip/ip4_addr.h"

#include "net.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "rx_robot_tcpip.h"

#include "bootloader.h"

#include "rx_trace.h"

/* Number of pending request on server's listening port. */
#define SERVER_LISTEN_BACKLOG_SIZE      5

/* Debug LwIP resource free in TIME-WAIT state. */
#define DEBUG_LWIP_RESOURCE_FREE        0

#define TASK_SERVER_STACK_SIZE			(500)           //Task Stack Size
#define TASK_SERVER_PRIORITY			(4)             //Task Priority
#define TASK_STATS_STACK_SIZE			(500)           //Task Stack Size
#define TASK_STATS_PRIORITY				(9)             //Task Priority

#define CHECK_INCOMMING_CONNECTION_INTERVAL	100

#define USE_DHCP            	0           // Disable DHCP
#define BROADCAST_ADDRESS       PP_HTONL(LWIP_MAKEU32(192, 168, 200, 255))
#define WILDCARD_IP_ADDRESS		PP_HTONL(LWIP_MAKEU32(192, 168, 200, 50))
#define IP_ADDRESS             	PP_HTONL(LWIP_MAKEU32(192, 168, 200, 50))
#define IP_ADDRESS_GATEWAY     	PP_HTONL(LWIP_MAKEU32(192, 168, 200, 1))
#define IP_SUBNET_MASK      	PP_HTONL(LWIP_MAKEU32(255, 255, 255, 0))

#define TASK_NOTIFY_NETIF_UP    0x01
#define TASK_NOTIFY_NETIF_DOWN  0x02
#define TASK_NOTIFY_LINK_UP     0x03
#define TASK_NOTIFY_LINK_DOWN   0x04

#define BROADCAST_INTERFACE_MAX_PKG_SIZE		256
#define BOOTLOADER_INTERFACE_MAX_PKG_SIZE 		512

#define RX_BOOT_MESSAGE_QUEUE_LENGTH	8
#define BOOTLOADER_MESSAGE_QUEUE_LENGTH	8

#define RX_BOOT_MESSAGE_QUEUE_SIZE		sizeof(void*)
#define BOOTLOADER_MESSAGE_QUEUE_SIZE	sizeof(void*)

static ip_addr_t _broadcastIpAddress = {BROADCAST_ADDRESS};

static ip_addr_t _wildcardIpAddress = 	{WILDCARD_IP_ADDRESS};
static ip_addr_t _ipAddress = 			{IP_ADDRESS};
static ip_addr_t _gatewayAddress = 		{IP_ADDRESS_GATEWAY};
static ip_addr_t _netMask = 			{IP_SUBNET_MASK};

static TaskHandle_t _networkManagerTask;
static TaskHandle_t _networkManagerConnectionTask;
#define NETWORK_UPDATE_INTERVAL	(10)

static bool _isInitialized = false;

static int32_t _communicationSocket = -1;
static int32_t _rxBootSocket = -1;

static struct sockaddr_in __attribute__ ((aligned (4))) _bootloaderServerAddress;
static struct sockaddr_in __attribute__ ((aligned (4))) _communicationServerAddress;
static struct sockaddr_in __attribute__ ((aligned (4))) _rxBootServerAddress;

static struct sockaddr_in __attribute__ ((aligned (4))) _bootloaderClientAddress;
static struct sockaddr_in __attribute__ ((aligned (4))) _communicationClientAddress;
static struct sockaddr_in __attribute__ ((aligned (4))) _rxBootClientAddress;

static QueueHandle_t _rxBootMessageQueue;
static QueueHandle_t _bootloaderMessageQueue;

static void network_task(void *pvParameters);
static void network_connection_task(void *pvParameters);
static void network_ethif_status_cb(int netif_up, int link_up, int packet_available);
static int  network_process_broadcast_interface(void);
static int  network_process_command_interface(void);

bool network_start(void) {
	sys_enable(sys_device_ethernet);

	memset(&_rxBootServerAddress, 0, sizeof(_rxBootServerAddress));
	memset(&_bootloaderServerAddress, 0, sizeof(_bootloaderServerAddress));
	memset(&_communicationServerAddress, 0, sizeof(_communicationServerAddress));
	memset(&_rxBootClientAddress, 0, sizeof(_rxBootClientAddress));
	memset(&_bootloaderClientAddress, 0, sizeof(_bootloaderClientAddress));
	memset(&_communicationClientAddress, 0, sizeof(_communicationClientAddress));

	if (xTaskCreate(network_task,
			"Network",
			TASK_SERVER_STACK_SIZE,
			NULL,
			TASK_SERVER_PRIORITY,
			&_networkManagerTask) != pdTRUE) {
	}

	_rxBootMessageQueue 	= xQueueCreate(RX_BOOT_MESSAGE_QUEUE_LENGTH, 	RX_BOOT_MESSAGE_QUEUE_SIZE);
	_bootloaderMessageQueue = xQueueCreate(BOOTLOADER_MESSAGE_QUEUE_LENGTH, BOOTLOADER_MESSAGE_QUEUE_SIZE);

	return true;
}

QueueHandle_t network_get_boot_message_queue(void)
{
	return _rxBootMessageQueue;
}

QueueHandle_t network_get_bootloader_message_queue(void)
{
	return _bootloaderMessageQueue;
}

void network_change_ip(ip_addr_t* newIpAddress)
{
	_ipAddress = *newIpAddress;
	struct netif* networkInterface = net_get_netif();
	netif_set_addr(networkInterface, &_ipAddress, &_netMask, &_gatewayAddress);
}

void network_broadcast(void *message, uint32_t size)
{
	static bool isBroadcastAddressInit;
	static struct sockaddr_in broadcastAddress;

	if(_rxBootSocket < 0)
		return;

	if(_isInitialized == false)
		return;

	if(isBroadcastAddressInit == false)
	{
		broadcastAddress.sin_family = AF_INET;
		broadcastAddress.sin_port = htons(PORT_UDP_BOOT_SVR);
		broadcastAddress.sin_addr.s_addr = ip4_addr_get_u32(&_broadcastIpAddress);

		isBroadcastAddressInit = true;
	}

	sendto(_rxBootSocket, message, size, MSG_DONTWAIT, (struct sockaddr *)&broadcastAddress, sizeof(broadcastAddress));
}

void network_send(void* message, uint32_t size)
{
	if(_isInitialized == false)
		return;

	if(_communicationSocket < 0)
		return;

	if(_communicationClientAddress.sin_addr.s_addr == 0)
		return;

	taskENTER_CRITICAL();
	sendto(_communicationSocket, message, size, MSG_DONTWAIT, (struct sockaddr *)&_communicationClientAddress, sizeof(_communicationClientAddress));
	taskEXIT_CRITICAL();
}

bool network_is_initialized(void)
{
	return _isInitialized;
}

static void network_task(void *pvParameters)
{
    uint32_t ulNotifiedValue;

    (void) pvParameters;

    /* Initialise Ethernet module and LwIP service. */
    net_init(_ipAddress, _gatewayAddress, _netMask, USE_DHCP, "Robot",
    		network_ethif_status_cb);

    /* Wait till network interface is up (IP address is valid in the case of DHCP) */
    while (xTaskNotifyWait( pdFALSE,    /* Don't clear bits on entry. */
            TASK_NOTIFY_NETIF_UP, /* Clear all bits on exit. */
            &ulNotifiedValue, /* Stores the notified value. */
            portMAX_DELAY ) == false)
        ;

    xTaskCreate(network_connection_task,
            "Connection",
            TASK_STATS_STACK_SIZE,
            NULL,
            TASK_STATS_PRIORITY,
            &_networkManagerConnectionTask);

    /* Wait till network interface is up (IP address is valid in the case of DHCP) */
    while (xTaskNotifyWait( pdTRUE,    /* Don't clear bits on entry. */
            TASK_NOTIFY_LINK_UP, /* Clear all bits on exit. */
            &ulNotifiedValue, /* Stores the notified value. */
            portMAX_DELAY ) == false)
        ;

    if (net_get_ip().addr == IPADDR_ANY)
    {
        while (1)
        {
            vTaskDelay(pdMS_TO_TICKS(100));
            if (net_get_ip().addr != IPADDR_ANY)
            {
                break;
            }
        }
    }

    while (1) {
    	_rxBootServerAddress.sin_family = AF_INET;
		_rxBootServerAddress.sin_port = htons(PORT_UDP_BOOT_CLNT);
		_rxBootServerAddress.sin_addr.s_addr = ip4_addr_get_u32(&_wildcardIpAddress);

		if ((_rxBootSocket = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
		{
			chip_reboot();
		}

		if (bind(_rxBootSocket, (struct sockaddr *) &_rxBootServerAddress, sizeof(_rxBootServerAddress)) < 0)
		{
			chip_reboot();
		}

        _bootloaderServerAddress.sin_family = AF_INET;
        _bootloaderServerAddress.sin_port = htons(PORT_UDP_BOOTLOADER);
        _bootloaderServerAddress.sin_addr.s_addr = ip4_addr_get_u32(&_wildcardIpAddress);

        _communicationServerAddress.sin_family = AF_INET;
        _communicationServerAddress.sin_port = htons(PORT_UDP_COMMUNICATION);
        _communicationServerAddress.sin_addr.s_addr = ip4_addr_get_u32(&_wildcardIpAddress);

		if((_communicationSocket = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
		{
			chip_reboot();
		}

		if(bind(_communicationSocket, (struct sockaddr *)&_communicationServerAddress, sizeof(_communicationServerAddress)) < 0)
		{
			chip_reboot();
		}

        _isInitialized = true;


    	TickType_t lastWakeTime;
    	TickType_t frequency = 10 / portTICK_PERIOD_MS;
        lastWakeTime = xTaskGetTickCount();
        while(1)
        {
        	int cnt=0;
        	gpio_main();
        	motor_main();
        	cnt += network_process_command_interface();
        	cnt += network_process_broadcast_interface();
        	bootloader_main();
        	if (cnt==0) vTaskDelayUntil(&lastWakeTime, frequency);
        }

        close(_rxBootSocket);
        close(_communicationSocket);
    }
}

static void network_connection_task(void *pvParameters)
{
    while (1)
    {
        if (net_is_link_up())
        {
            if (!ethernet_is_link_up())
            {
                net_set_link_down();
            }
        }
        else
        {
            if (ethernet_is_link_up())
            {
                net_set_link_up();
            }
        }
        for (int i = 0; i < 10; i++)
        {
            net_tick();
            vTaskDelay(10 * portTICK_PERIOD_MS);
        }
    }
}

static void network_ethif_status_cb(int netif_up, int link_up, int packet_available) {
    if (netif_up) {
        xTaskNotify(_networkManagerTask,TASK_NOTIFY_NETIF_UP,eSetBits);
    }
    else {
    }

    if (link_up) {
        xTaskNotify(_networkManagerTask,TASK_NOTIFY_LINK_UP,eSetBits);
    }
    else {
        xTaskNotify(_networkManagerTask,TASK_NOTIFY_LINK_DOWN,eSetBits);
    }
}

static int network_process_broadcast_interface(void)
{
	static uint8_t udpData[BROADCAST_INTERFACE_MAX_PKG_SIZE] = {0};
	static int32_t size_rx;

	socklen_t len = sizeof(_rxBootClientAddress);
	size_rx = recvfrom(_rxBootSocket, udpData, sizeof(udpData), MSG_DONTWAIT, (struct sockaddr *)&_rxBootClientAddress, &len);

	if((size_rx > 0) && (size_rx < BROADCAST_INTERFACE_MAX_PKG_SIZE))
	{
		void* message = pvPortMalloc(size_rx);
		memcpy(message, udpData, size_rx);

		// The pointer will be copied into the message q, that's why the reference of message is passed
		// After that call, message can go out of scope, it will either be freed inside the if if the message q is full
		// or it will be freed by the receiving end that handles the message.
		if(xQueueSend(_rxBootMessageQueue, &message, (TickType_t)0) != pdPASS)
			free(message);
		return 1;
	}
	return 0;
}

static int network_process_command_interface(void)
{
	static uint8_t udpData[sizeof(SBootloaderDataCmd)+10];
	SMsgHdr *phdr = (SMsgHdr*)udpData;
	int32_t msgLen;

	socklen_t len = sizeof(_communicationClientAddress);
	msgLen = recvfrom(_communicationSocket, udpData, sizeof(udpData), MSG_DONTWAIT, (struct sockaddr *)&_communicationClientAddress, &len);

	if(msgLen > 0)
	{
		if (phdr->msgLen==msgLen)
		{
			switch((phdr->msgId & COMMAND_TYPE_MASK))
			{
			case STATUS_COMMAND_MASK:		status_handle_message(udpData); break;
			case GPIO_COMMAND_MASK:			gpio_handle_message(udpData); 	break;
			case MOTOR_COMMAND_MASK: 		motor_handle_message(udpData);	break;
			case BOOTLOADER_COMMAND_MASK: 	bootloader_handle_command(udpData);	break;
			default:	break;
			}
			return 1;
		}
		else TrPrintf(TRUE, "Received invalid message, msgLen=%d, hdr.msgLen=%d, hdr.msgId=0x%08x", msgLen, phdr->msgLen, phdr->msgId);
	}
	return 0;
}
