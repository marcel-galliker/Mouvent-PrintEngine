#include "gpio_manager.h"

#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "sys/types.h"

#include <ft900.h>

#include "rx_robot_def.h"
#include "rx_robot_tcpip.h"
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


// GPIO pins
#define MOTOR_SD_MODE	23

#define GPIO_SPIM_CLK	27
#define GPIO_SPIM_MISO	30
#define GPIO_SPIM_MOSI	29
#define GPIO_SPIM_SS0	28
#define GPIO_SPIM_SS1	33
#define GPIO_SPIM_SS2	34
#define GPIO_SPIM_SS3	35

#define WATCHDOG_IO		66

// I/O arrays
static uint8_t _outputs[] = {
		57, 	// OUTPUT 0
		61,		// OUTPUT 1
		62,		// OUTPUT 2
		65		// OUTPUT 3
};

static uint8_t _inputs[] = {
	53,	// INPUT_0
	54,	// INPUT_1
	56,	// INPUT_2
	52,	// INPUT_3
	55,	// INPUT_4
	11,	// INPUT_5
	10	// INPUT_6
};

static uint8_t _stepperMotors[] = {
 	50,	// MOTOR0_EN
 	48,	// MOTOR1_EN
	46,	// MOTOR2_EN
 	18	// MOTOR3_EN
};

static uint8_t _referenceOutputs[] = {
		31,	// MOTOR0_L_REF
		40,	// MOTOR1_L_REF
		25,	// MOTOR2_L_REF
		19,	// MOTOR3_L_REF
		26,	// MOTOR0_R_REF
		41,	// MOTOR1_R_REF
		24,	// MOTOR2_R_REF
		20,	// MOTOR3_R_REF
};

// I/O array sizes
static const uint8_t _outputCount = (sizeof(_outputs) / sizeof(_outputs[0]));
static const uint8_t _inputCount = (sizeof(_inputs) / sizeof(_inputs[0]));
static const uint8_t _motorCount = (sizeof(_stepperMotors) / sizeof(_stepperMotors[0]));
static const uint8_t _refCount = (sizeof(_referenceOutputs) / sizeof(_referenceOutputs[0]));

// Status Flags
static bool _isInitialized = false;

/* Prototypes */

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
static int  update_input(int no);
static void gpio_manager_set_outputs(uint8_t outputs);
static void _gpio_manager_reset_edgeCnt(uint8_t inputs);


bool gpio_manager_start(void)
{
	init_voltage_watchdog();
	init_motor_gpios();
	init_spi_gpios();
	init_inputs();
	init_outputs();

	_isInitialized = true;
	return true;
}

bool gpio_manager_is_initalized(void)
{
	return _isInitialized;
}

void gpio_manager_set_output(uint8_t output, bool value)
{
	if(output < OUTPUT_CNT) gpio_write(_outputs[output], value);
}

bool gpio_manager_get_input(uint8_t input)
{
	if(input < INPUT_CNT)
		return update_input(input);
	else return 0;
}

void gpio_manager_enable_motor(uint8_t motor)
{
	if(motor<MOTOR_CNT) gpio_write(_stepperMotors[motor], false);
}

void gpio_manager_disable_motor(uint8_t motor)
{
	if(motor<MOTOR_CNT) gpio_write(_stepperMotors[motor], true);
}

void gpio_manager_start_motor(uint8_t motor)
{
	if(motor<MOTOR_CNT)
	{
		gpio_write(_referenceOutputs[motor], false);
		gpio_write(_referenceOutputs[motor + _motorCount], false);
	}
}

void gpio_manager_stop_motor(uint8_t motor)
{
	if(motor < MOTOR_CNT)
	{
		gpio_write(_referenceOutputs[motor], true);
		gpio_write(_referenceOutputs[motor + _motorCount], true);
	}
}

void gpio_handle_message(void* message)
{
	SGpioSetCmd* cmd = (SGpioSetCmd*)message;

	if(_isInitialized)
	{
		switch(cmd->header.msgId)
		{
		case CMD_GPIO_SET:		gpio_manager_set_outputs(cmd->outputs);		break;
		case CMD_GPIO_RESET:	_gpio_manager_reset_edgeCnt(cmd->outputs);	break;

		default:
			break;
		}
	}
}

//--- gpio_main ----------------------------------------
void gpio_main(void)
{
	toggle_watchdog();
	update_outputs();
	update_inputs();
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

	gpio_function(_inputs[0], pad_gpio53);
	gpio_function(_inputs[1], pad_gpio54);
	gpio_function(_inputs[2], pad_gpio56);
	gpio_function(_inputs[3], pad_gpio52);
	gpio_function(_inputs[4], pad_gpio55);
	gpio_function(_inputs[5], pad_gpio11);
	gpio_function(_inputs[6], pad_gpio10);

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
		RX_RobotStatus.gpio.outputs = (RX_RobotStatus.gpio.outputs & ~(1 << count)) | (val << count);
	}
}

static void update_inputs(void)
{
	int count;
	for(count = 0; count < _inputCount; count++)
	{
		update_input(count);
	}
}

static int update_input(int no)
{
	uint8_t val;

	val = gpio_read(_inputs[no]);

#ifdef INVERTED_INPUTS
	val = !val;
#endif

	if(val && !(RX_RobotStatus.gpio.inputs & (1 << no)))
	{
		RX_RobotStatus.gpio.inputEdges[no]++;
	}

	if (val) RX_RobotStatus.gpio.inputs |=  (1 << no);
	else	 RX_RobotStatus.gpio.inputs &= ~(1 << no);
	return (val!=0);
}

static void gpio_manager_set_outputs(uint8_t outputs)
{
	for(int i = 0; i < _outputCount; i++)
	{
		uint8_t bit = (outputs >> i) & 1U;
		gpio_write(_outputs[i], bit);
	}
}

static void _gpio_manager_reset_edgeCnt(uint8_t inputs)
{
	int in;
	for(in = 0; in < _inputCount; in++)
	{
		if (inputs & (1<<in)) RX_RobotStatus.gpio.inputEdges[in]=0;
	}
}
