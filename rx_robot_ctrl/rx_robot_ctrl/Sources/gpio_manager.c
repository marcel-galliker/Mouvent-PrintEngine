#include "gpio_manager.h"
#include "gpio_manager_def.h"

#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "sys/types.h"

#include <ft900.h>

#include "tinyprintf.h"

#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>

#include "communication_def.h"
#include "network_manager.h"
#include "status_manager.h"

/* Defines */

// GPIO update interval
#define GPIO_UPDATE_INTERVAL	10

// Set to 1 if inputs are inverted
#define INVERTED_INPUTS	1

// Task settings
#define TASK_GPIO_STACK_SIZE		(500)
#define TASK_GPIO_PRIORITY			(9)

/* Static variables */

// Task handles
static TaskHandle_t _gpioManagerTask;

// I/O arrays
static uint8_t _outputs[] = {
		OUTPUT_0,
		OUTPUT_1,
		OUTPUT_2,
		OUTPUT_3
};

static uint8_t _inputs[] = {
		INPUT_0,
		INPUT_1,
		INPUT_2,
		INPUT_3,
		INPUT_4,
		INPUT_5,
		INPUT_6,
};

static uint8_t _stepperMotors[] = {
		MOTOR0_EN,
		MOTOR1_EN,
		MOTOR2_EN,
		MOTOR3_EN
};

static uint8_t _referenceOutputs[] = {
		MOTOR0_L_REF,
		MOTOR1_L_REF,
		MOTOR2_L_REF,
		MOTOR3_L_REF,
		MOTOR0_R_REF,
		MOTOR1_R_REF,
		MOTOR2_R_REF,
		MOTOR3_R_REF
};

// I/O array sizes
static const uint8_t _outputCount = (sizeof(_outputs) / sizeof(_outputs[0]));
static const uint8_t _inputCount = (sizeof(_inputs) / sizeof(_inputs[0]));
static const uint8_t _motorCount = (sizeof(_stepperMotors) / sizeof(_stepperMotors[0]));
static const uint8_t _refCount = (sizeof(_referenceOutputs) / sizeof(_referenceOutputs[0]));

// Status variable
static GpioStatus_t _gpioStatus;

// Status Flags
static bool _isInitialized = false;

static QueueHandle_t _gpioMessageQueue;

/* Prototypes */

// Tasks
static void gpio_manager_task(void *pvParameters);

// Initialization functions
static void init_voltage_watchdog(void);
static void init_motor_gpios(void);
static void init_spi_gpios(void);
static void init_inputs(void);
static void init_outputs(void);

// Update functions
static void toggle_watchdog(void);
static void update_outputs(void);
static void update_inputs(void);
static void gpio_manager_set_outputs(uint8_t outputs);
static void gpio_manager_handle_message(void* message);


bool gpio_manager_start(void) {
	memset(&_gpioStatus, 0, sizeof(_gpioStatus));

	_gpioMessageQueue = network_manager_get_gpio_message_queue();

	if(_gpioMessageQueue == NULL)
		chip_reboot();

	init_voltage_watchdog();
	init_motor_gpios();
	init_spi_gpios();
	init_inputs();
	init_outputs();

	xTaskCreate(gpio_manager_task,
				"GPIO",
				TASK_GPIO_STACK_SIZE,
				NULL,
				TASK_GPIO_PRIORITY,
				&_gpioManagerTask);

	return true;
}

bool gpio_manager_is_initalized(void)
{
	return _isInitialized;
}

void gpio_manager_set_output(uint8_t output, bool value)
{
	if(output >= NUMBER_OF_OUTPUTS)
		return;

	gpio_write(_outputs[output], value);
}

bool gpio_manager_get_input(uint8_t input)
{
	if(input > NUMBER_OF_INPUTS)
		return 0;

	return gpio_read(_inputs[input]);
}

void gpio_manager_enable_motor(uint8_t motor)
{
	if(motor > MOTOR_COUNT)
		return;

	gpio_write(_stepperMotors[motor], false);
}

void gpio_manager_disable_motor(uint8_t motor)
{
	if(motor > MOTOR_COUNT)
		return;

	gpio_write(_stepperMotors[motor], true);
}

void gpio_manager_start_motor(uint8_t motor)
{
	if(motor > MOTOR_COUNT)
		return;

	gpio_write(_referenceOutputs[motor], false);
	gpio_write(_referenceOutputs[motor + _motorCount], false);
}

void gpio_manager_stop_motor(uint8_t motor)
{
	if(motor > MOTOR_COUNT)
		return;

	gpio_write(_referenceOutputs[motor], true);
	gpio_write(_referenceOutputs[motor + _motorCount], true);
}

GpioStatus_t* gpio_manager_get_status(void)
{
	return &_gpioStatus;
}

static void gpio_manager_handle_message(void* message)
{
	RobotMessageHeader_t* header = (RobotMessageHeader_t*)message;
	GpioSetCommand_t* gpioSetCommand = NULL;

	if(_isInitialized == false)
		return;

	switch(header->messageId)
	{
	case CMD_GPIO_SET:
		gpioSetCommand = (GpioSetCommand_t*)header;
		gpio_manager_set_outputs(gpioSetCommand->outputs);
		break;

	default:
		break;
	}
}

static void gpio_manager_task(void *pvParameters) {
	static TickType_t lastWakeTime;
	static const TickType_t frequency = GPIO_UPDATE_INTERVAL / portTICK_PERIOD_MS;

	while(network_manager_is_initialized() == false){
		vTaskDelay(100 / portTICK_PERIOD_MS);
	}

	while(true)
	{
		toggle_watchdog();
		update_outputs();
		update_inputs();

		if(_gpioMessageQueue != NULL)
			{
				void* message = NULL;
				if(xQueueReceive(_gpioMessageQueue, &message, 0) == pdPASS)
				{
					gpio_manager_handle_message(message);
					// Free the memory that was allocated by the network manager to hold the command
					vPortFree(message);

					status_manager_send_status();
				}
			}

		vTaskDelayUntil(&lastWakeTime, frequency);
	}
}

static void init_voltage_watchdog(void)
{
	// Set voltage watchdog pin as output
	gpio_function(WATCHDOG_IO, pad_gpio66);
	gpio_dir(WATCHDOG_IO, pad_dir_output);

	// Disable pull up/down on voltage watchdog output
	gpio_pull(WATCHDOG_IO, pad_pull_none);

	// Set output low
	gpio_write(WATCHDOG_IO, false);
}

static void init_motor_gpios(void)
{
	uint8_t count = 0;

	for(count = 0; count < _motorCount; count++)
	{
		// Enable motor enable outputs
		gpio_function(_stepperMotors[count], pad_func_0);
		gpio_dir(_stepperMotors[count], pad_dir_output);
		gpio_pull(_stepperMotors[count], pad_pull_pullup);
		gpio_write(_stepperMotors[count], true);
	}

	for(count = 0; count < _refCount; count++)
	{
		// Enable reference outputs to controle motor
		gpio_function(_referenceOutputs[count], pad_func_0);
		gpio_dir(_referenceOutputs[count], pad_dir_output);
		gpio_pull(_referenceOutputs[count], pad_pull_pulldown);
		gpio_write(_referenceOutputs[count], false);
	}

	// Enable SPI mode / Disable step/dir mode
	gpio_dir(MOTOR_SD_MODE, pad_dir_output);
	gpio_pull(MOTOR_SD_MODE, pad_pull_none);
	gpio_write(MOTOR_SD_MODE, false);
}

static void init_spi_gpios(void)
{
	// Enable SPI device and setup I/Os
	sys_enable(sys_device_spi_master);
	gpio_function(GPIO_SPIM_CLK, pad_spim_sck);
	gpio_function(GPIO_SPIM_MOSI, pad_spim_mosi);
	gpio_function(GPIO_SPIM_MISO, pad_spim_miso);
	gpio_function(GPIO_SPIM_SS0, pad_spim_ss0);
	gpio_function(GPIO_SPIM_SS1, pad_spim_ss1);
	gpio_function(GPIO_SPIM_SS2, pad_spim_ss2);
	gpio_function(GPIO_SPIM_SS3, pad_spim_ss3);

	// Disable SPI chip select
	gpio_write(GPIO_SPIM_SS0, true);
	gpio_write(GPIO_SPIM_SS1, true);
	gpio_write(GPIO_SPIM_SS2, true);
	gpio_write(GPIO_SPIM_SS3, true);
}

static void init_inputs(void)
{
	uint8_t count;

	memset(_gpioStatus.inputEdges, 0, sizeof(_gpioStatus.inputEdges));

	gpio_function(INPUT_0, pad_gpio53);
	gpio_function(INPUT_1, pad_gpio54);
	gpio_function(INPUT_2, pad_gpio56);
	gpio_function(INPUT_3, pad_gpio52);
	gpio_function(INPUT_4, pad_gpio55);
	gpio_function(INPUT_5, pad_gpio11);
	gpio_function(INPUT_6, pad_gpio10);

	for(count = 0; count < _inputCount; count++)
	{
		gpio_dir(_inputs[count], pad_dir_input);
		gpio_pull(_inputs[count], pad_pull_none);
	}
}

static void init_outputs(void)
{
	static uint8_t count;

	for(count = 0; count < _outputCount; count++)
	{
		gpio_dir(_outputs[count], pad_dir_output);
		gpio_pull(_outputs[count], pad_pull_none);
		gpio_write(_outputs[count], false);
	}
}

static void toggle_watchdog(void)
{
	static int8_t watchdog_state;

	watchdog_state = gpio_read(WATCHDOG_IO);
	gpio_write(WATCHDOG_IO, !watchdog_state);
}

static void update_outputs(void)
{
	static uint8_t count;
	static int8_t val;

	for(count = 0; count < _outputCount; count++)
	{
		val = gpio_read(_outputs[count]);
		_gpioStatus.outputs = (_gpioStatus.outputs & ~(1 << count)) | (val << count);
	}
}

static void update_inputs(void)
{
	static uint8_t count;
	static uint8_t val;

	for(count = 0; count < _inputCount; count++)
	{
		val = gpio_read(_inputs[count]);

#ifdef INVERTED_INPUTS
		val = !val;
#endif

		if(val != ((_gpioStatus.inputs >> count) & 1))
		{
			_gpioStatus.inputEdges[count] += 1;
			_gpioStatus.inputs ^= 1 << count;
		}
	}
}

static void gpio_manager_set_outputs(uint8_t outputs)
{
	for(int i = 0; i < _outputCount; i++)
	{
		uint8_t bit = (outputs >> i) & 1U;
		gpio_write(_outputs[i], bit);
	}
}
