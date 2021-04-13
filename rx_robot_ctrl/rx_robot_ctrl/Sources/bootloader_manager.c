#include "bootloader_manager.h"
#include "bootloader_manager_def.h"
#include "rx_robot_tcpip.h"

#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#include <ft900.h>

#include "tinyprintf.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "network_manager.h"
#include "status_manager.h"

/* Defines */

// Task settings
#define TASK_BOOTLOADER_STACK_SIZE		(500)
#define TASK_BOOTLOADER_PRIORITY		(8)

// Flash size
#define FLASH_WRITE_POS 		(0)
#define FLASH_SECTOR_SIZE		(4096)
#define FLASH_SECTOR_COUNT		(64)
#define PROG_BUFFER_SIZE		(FLASH_SECTOR_SIZE)

/* Static variables */

// Task handles
static TaskHandle_t _bootloaderManagerTask;

// Bootloader message queue
static QueueHandle_t _bootloaderMessageQueue;

// Status Flags
static bool _isInitialized = false;

static uint8_t progMemBlock[PROG_BUFFER_SIZE] = {0};

/* Prototypes */

static void bootloader_manager_task(void *pvParameters);
static void bootloader_manager_handle_command(void* msg);
static void bootloader_manager_reset(void);
static void bootloader_manager_init(BootloaderStartCommand_t* command);
static void bootloader_manger_handle_data(BootloaderDataCommand_t* command);
static void bootloader_manager_confirm(void);
static void bootloader_manager_recover(void);


bool bootloader_manager_start(void) {
	// TODO: Check if network is initalized

	_bootloaderMessageQueue = network_manager_get_bootloader_message_queue();

	if(_bootloaderMessageQueue == NULL)
		chip_reboot();

	bootloader_manager_reset();

	xTaskCreate(bootloader_manager_task,
			"Bootloader",
			TASK_BOOTLOADER_STACK_SIZE,
			NULL,
			TASK_BOOTLOADER_PRIORITY,
			&_bootloaderManagerTask);

	_isInitialized = true;

	return true;
}

bool bootloader_manager_is_initalized(void) {
	return _isInitialized;
}

static void bootloader_manager_task(void *pvParameters)
{
	while(true)
	{
		if(_bootloaderMessageQueue != NULL)
		{
			void* message = NULL;
			if(xQueueReceive(_bootloaderMessageQueue, &message, portMAX_DELAY) == pdPASS)
			{
				bootloader_manager_handle_command(message);
				vPortFree(message);
				status_manager_send_status();
			}
		}
	}
}

static void bootloader_manager_handle_command(void* msg)
{
	SMsgHdr* header = (SMsgHdr*)msg;

	switch(header->msgId)
	{
	case CMD_BOOTLOADER_START:
		bootloader_manager_init((BootloaderStartCommand_t*)msg);
		break;

	case CMD_BOOTLOADER_DATA:
		bootloader_manger_handle_data((BootloaderDataCommand_t*)msg);
		break;

	case CMD_BOOTLOADER_CONFIRM:
		bootloader_manager_confirm();
		break;

	default:
		break;
	}
}

static void bootloader_manager_reset(void)
{
	memset(&RX_RobotStatus.bootloader, 0, sizeof(RX_RobotStatus.bootloader));
}

static void bootloader_manager_init(BootloaderStartCommand_t* command)
{
	if(RX_RobotStatus.bootloader.status != UNINITIALIZED)
		bootloader_manager_recover();

	bootloader_manager_reset();

	if(command->size > (FLASH_SECTOR_SIZE * FLASH_SECTOR_COUNT))
		return;

	taskENTER_CRITICAL();
	flash_chip_erase();
	taskEXIT_CRITICAL();

	RX_RobotStatus.bootloader.progSize = command->size;
	RX_RobotStatus.bootloader.status = WAITING_FOR_DATA;
}

static void bootloader_manger_handle_data(BootloaderDataCommand_t* command)
{
	uint32_t lengthIfWritten = ((RX_RobotStatus.bootloader.progMemBlocksUsed * PROG_BUFFER_SIZE) + command->length + RX_RobotStatus.bootloader.progMemPos);

	if(RX_RobotStatus.bootloader.status == UNINITIALIZED)
		return;

	if(RX_RobotStatus.bootloader.status != WAITING_FOR_DATA)
	{
		bootloader_manager_reset();
		bootloader_manager_recover();
		return;
	}

	if(lengthIfWritten > RX_RobotStatus.bootloader.progSize)
	{
		bootloader_manager_reset();
		bootloader_manager_recover();
		return;
	}

	if((RX_RobotStatus.bootloader.progMemPos + command->length) > PROG_BUFFER_SIZE)
	{
		bootloader_manager_reset();
		bootloader_manager_recover();
		return;
	}

	memcpy(&progMemBlock[RX_RobotStatus.bootloader.progMemPos], command->data, command->length);
	RX_RobotStatus.bootloader.progMemPos += command->length;

	if((PROG_BUFFER_SIZE == RX_RobotStatus.bootloader.progMemPos) ||
	   (lengthIfWritten == RX_RobotStatus.bootloader.progSize))
	{
		uint32_t flashPosition = RX_RobotStatus.bootloader.progMemBlocksUsed * FLASH_SECTOR_SIZE;

		taskENTER_CRITICAL();
		memcpy_dat2flash(flashPosition, progMemBlock, PROG_BUFFER_SIZE);
		taskEXIT_CRITICAL();

		RX_RobotStatus.bootloader.progMemBlocksUsed++;
		RX_RobotStatus.bootloader.progMemPos = 0;
	}

	if(lengthIfWritten == RX_RobotStatus.bootloader.progSize)
		RX_RobotStatus.bootloader.status = WAITING_FOR_CONFIRM;
}

static void bootloader_manager_confirm(void)
{
	if(RX_RobotStatus.bootloader.status == UNINITIALIZED)
		return;

	if(RX_RobotStatus.bootloader.status != WAITING_FOR_CONFIRM)
	{
		bootloader_manager_reset();
		bootloader_manager_recover();
		return;
	}

	chip_reboot();
}

static void bootloader_manager_recover(void)
{
	taskENTER_CRITICAL();
	flash_revert();
	taskEXIT_CRITICAL();
}
