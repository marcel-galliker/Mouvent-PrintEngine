#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "sys/types.h"

#include <ft900.h>
#include <gpio.h>
#include <network.h>
#include <ctrl.h>

#include "rx_robot_def.h"
#include "rx_robot_tcpip.h"

/* Defines */


// Set to 1 if inputs are inverted
#define INVERTED_INPUTS	1


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
static const uint8_t _outputs[OUTPUT_CNT] = {
		57, 	// OUTPUT 0
		61,		// OUTPUT 1
		62,		// OUTPUT 2
		65		// OUTPUT 3
};

static const uint8_t _inputs[INPUT_CNT] = {
	53,	// INPUT_0
	54,	// INPUT_1
	56,	// INPUT_2
	52,	// INPUT_3
	55,	// INPUT_4
	11,	// INPUT_5
	10	// INPUT_6
};

static const uint8_t _motorDisable[MOTOR_CNT] = {
 	50,	// MOTOR0_EN
 	48,	// MOTOR1_EN
	46,	// MOTOR2_EN
 	18	// MOTOR3_EN
};

static const uint8_t _motorStopLeft[MOTOR_CNT] = {
		31,	// MOTOR0_L_REF
		40,	// MOTOR1_L_REF
		25,	// MOTOR2_L_REF
		19,	// MOTOR3_L_REF
};

static const uint8_t _motorStopRight[MOTOR_CNT] = {
		26,	// MOTOR0_R_REF
		41,	// MOTOR1_R_REF
		24,	// MOTOR2_R_REF
		20,	// MOTOR3_R_REF
};

/* Prototypes */

// Initialization functions
static void _init_voltage_watchdog(void);
static void _init_motor_gpios(void);
static void _init_spi_gpios(void);
static void _init_inputs(void);
static void _init_outputs(void);

// Update functions
static void _toggle_watchdog(void);
static void _update_outputs(void);
static void _update_inputs(void);
static int  _update_input(int no);
static void _gpio_set_outputs(uint8_t outputs, int value);
static void _gpio_reset_edgeCnt(uint8_t inputs);


bool gpio_init(void)
{
	_init_voltage_watchdog();
	_init_motor_gpios();
	_init_spi_gpios();
	_init_inputs();
	_init_outputs();

	return true;
}

void gpio_set_output(uint8_t output, bool value)
{
	if(output < OUTPUT_CNT) gpio_write(_outputs[output], value);
}

bool gpio_get_input(uint8_t input)
{
	if(input < INPUT_CNT)
		return _update_input(input);
	else return 0;
}

void gpio_enable_motor(uint8_t motor, int enable)
{
	if(motor<MOTOR_CNT) gpio_write(_motorDisable[motor], !enable);
}


void gpio_start_motor(uint8_t motor)
{
	if(motor<MOTOR_CNT)
	{
		gpio_write(_motorStopLeft[motor], false);
		gpio_write(_motorStopRight[motor], false);
	}
}

void gpio_stop_motor(uint8_t motor)
{
	if(motor < MOTOR_CNT)
	{
		gpio_write(_motorStopLeft[motor], true);
		gpio_write(_motorStopRight[motor], true);
	}
}

bool gpio_handle_msg(void* message)
{
	SGpioSetCmd* cmd = (SGpioSetCmd*)message;

	switch(cmd->header.msgId)
	{
	case CMD_GPIO_OUT_SETLOW:	_gpio_set_outputs(cmd->outputs, 0);	return TRUE;
	case CMD_GPIO_OUT_SETHIGH:	_gpio_set_outputs(cmd->outputs, 1);	return TRUE;
	case CMD_GPIO_IN_RESET:		_gpio_reset_edgeCnt(cmd->outputs);	return TRUE;

	default:
		break;
	}
	return FALSE;
}

//--- gpio_tick ----------------------------------------
void gpio_tick(int tick)
{
	_toggle_watchdog();
	_update_outputs();
	_update_inputs();
}

static void _init_voltage_watchdog(void)
{
	// Set voltage watchdog pin as output
	gpio_function(WATCHDOG_IO, pad_gpio66);
	gpio_dir(WATCHDOG_IO, pad_dir_output);

	// Disable pull up/down on voltage watchdog output
	gpio_pull(WATCHDOG_IO, pad_pull_none);

	// Set output low
	gpio_write(WATCHDOG_IO, false);
}

static void _init_motor_output(int out, int put_pull, int value)
{
	gpio_function(out, pad_func_0);
	gpio_dir(out, pad_dir_output);
	gpio_pull(out, put_pull);
	gpio_write(out, value);
}

static void _init_motor_gpios(void)
{
	for(int motor = 0; motor < MOTOR_CNT; motor++)
	{
		_init_motor_output(_motorDisable[motor], pad_pull_pullup, TRUE);
		_init_motor_output(_motorStopLeft[motor], pad_pull_pulldown, FALSE);
		_init_motor_output(_motorStopRight[motor], pad_pull_pulldown, FALSE);
	}

	// Enable SPI mode / Disable step/dir mode
	gpio_dir(MOTOR_SD_MODE, pad_dir_output);
	gpio_pull(MOTOR_SD_MODE, pad_pull_none);
	gpio_write(MOTOR_SD_MODE, false);
}

static void _init_spi_gpios(void)
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

static void _init_inputs(void)
{
	uint8_t count;

	gpio_function(_inputs[0], pad_gpio53);
	gpio_function(_inputs[1], pad_gpio54);
	gpio_function(_inputs[2], pad_gpio56);
	gpio_function(_inputs[3], pad_gpio52);
	gpio_function(_inputs[4], pad_gpio55);
	gpio_function(_inputs[5], pad_gpio11);
	gpio_function(_inputs[6], pad_gpio10);

	for(count = 0; count < INPUT_CNT; count++)
	{
		gpio_dir(_inputs[count], pad_dir_input);
		gpio_pull(_inputs[count], pad_pull_none);
	}
}

static void _init_outputs(void)
{
	static uint8_t count;

	for(count = 0; count < OUTPUT_CNT; count++)
	{
		gpio_dir(_outputs[count], pad_dir_output);
		gpio_pull(_outputs[count], pad_pull_none);
		gpio_write(_outputs[count], false);
	}
}

static void _toggle_watchdog(void)
{
	gpio_write(WATCHDOG_IO, !gpio_read(WATCHDOG_IO));
}

static void _update_outputs(void)
{
	static uint8_t count;
	static int8_t val;

	for(count = 0; count < OUTPUT_CNT; count++)
	{
		val = gpio_read(_outputs[count]);
		RX_RobotStatus.gpio.outputs = (RX_RobotStatus.gpio.outputs & ~(1 << count)) | (val << count);
	}
}

static void _update_inputs(void)
{
	int count;
	for(count = 0; count < INPUT_CNT; count++)
	{
		_update_input(count);
	}
}

static int _update_input(int no)
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

static void _gpio_set_outputs(uint8_t outputs, int value)
{
	for(int out = 0; out < OUTPUT_CNT; out++)
	{
		if (outputs & (1<<out)) gpio_write(_outputs[out], value);
	}
}

static void _gpio_reset_edgeCnt(uint8_t inputs)
{
	for(int in = 0; in < INPUT_CNT; in++)
	{
		if (inputs & (1<<in)) RX_RobotStatus.gpio.inputEdges[in]=0;
	}
}
