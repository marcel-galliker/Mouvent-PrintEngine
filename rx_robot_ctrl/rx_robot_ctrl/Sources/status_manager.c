#include "status_manager.h"
#include "status_manager_def.h"

#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#include <ft900.h>

#include "tinyprintf.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "communication_def.h"
#include "network_manager.h"
#include "gpio_manager.h"
#include "bootloader_manager.h"
#include "motor_manager.h"


/* Defines */

// Task settings
#define TASK_STATUS_STACK_SIZE		(500)
#define TASK_STATUS_PRIORITY		(9)

// Status settings
#define STATUS_INTERVAL	10

/* Static variables */

// Task handles
static TaskHandle_t _statusManagerTask;

// Status variable
static RobotStatusMessage_t __attribute__ ((aligned (4))) _robotStatus;

// Status Flags
static bool _isInitialized = false;

static QueueHandle_t _statusMessageQueue;

/* Prototypes */

static void status_manager_task(void *pvParameters);

bool status_manager_start(void) {
	_robotStatus.header.messageId = CMD_STATUS_GET;
	_robotStatus.header.messageLength = sizeof(_robotStatus);

	_statusMessageQueue = network_manager_get_status_message_queue();

	if(_statusMessageQueue == NULL)
		chip_reboot();

	xTaskCreate(status_manager_task,
			"Status",
			TASK_STATUS_STACK_SIZE,
			NULL,
			TASK_STATUS_PRIORITY,
			&_statusManagerTask);

	return true;
}

bool status_manager_is_initalized(void) {
	return _isInitialized;
}

void status_manager_handle_message(void* message)
{
	RobotMessageHeader_t* header = (RobotMessageHeader_t*)message;

	if(_isInitialized == false)
		return;

	switch(header->messageId)
	{
	case CMD_STATUS_GET:
		status_manager_send_status();
		break;

	default:
		break;
	}
}

void status_manager_send_status(void)
{
	GpioStatus_t* gpioStatus = gpio_manager_get_status();
	BootloaderStatus_t* bootloaderStatus = bootloader_manager_get_status();
	MotorStatus_t* motorStatus = motor_manager_get_status();

	// This function can be called from multiple task contexts
	// because of this, it needs to be wrapped in a crtical section to prevent data changes while sending
	taskENTER_CRITICAL();

	memcpy(&_robotStatus.gpio, gpioStatus, sizeof(_robotStatus.gpio));
	memcpy(&_robotStatus.bootloader, bootloaderStatus, sizeof(_robotStatus.bootloader));
	memcpy(&_robotStatus.motors, motorStatus, sizeof(_robotStatus.motors));

	network_manager_send(&_robotStatus, sizeof(_robotStatus));

	taskEXIT_CRITICAL();
}

static void status_manager_task(void *pvParameters)
{
	static TickType_t lastWakeTime;
	static const TickType_t frequency = STATUS_INTERVAL / portTICK_PERIOD_MS;

	while(network_manager_is_initialized() == false){
		vTaskDelay(100 / portTICK_PERIOD_MS);
	}

	_isInitialized = true;

	lastWakeTime = xTaskGetTickCount();

	while(true)
	{
		if(_statusMessageQueue != NULL)
		{
			void* message = NULL;
			if(xQueueReceive(_statusMessageQueue, &message, 0) == pdPASS)
			{
				status_manager_handle_message(message);
				// Free the memory that was allocated by the network manager to hold the command
				vPortFree(message);
			}
		}

		vTaskDelayUntil(&lastWakeTime, frequency);
	}
}

