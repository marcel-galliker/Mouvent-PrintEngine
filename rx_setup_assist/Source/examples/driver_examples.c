/*
 * Code generated from Atmel Start.
 *
 * This file will be overwritten when reconfiguring your Atmel Start project.
 * Please copy examples or other code you want to keep to a separate file
 * to avoid losing it when reconfiguring.
 */

#include "driver_examples.h"
#include "driver_init.h"
#include "utils.h"

static void button_on_PA17_pressed(void)
{
}
static void button_on_PA18_pressed(void)
{
}
static void button_on_PA31_pressed(void)
{
}

/**
 * Example of using EXTERNAL_IRQ_0
 */
void EXTERNAL_IRQ_0_example(void)
{
	ext_irq_register(PIO_PA17_IDX, button_on_PA17_pressed);
	ext_irq_register(PIO_PA18_IDX, button_on_PA18_pressed);
	ext_irq_register(PIO_PA31_IDX, button_on_PA31_pressed);
}

/**
 * Example of using SPI_0 to write "Hello World" using the IO abstraction.
 */
static uint8_t example_SPI_0[12] = "Hello World!";

void SPI_0_example(void)
{
	struct io_descriptor *io;
	spi_m_sync_get_io_descriptor(&SPI_0, &io);

	spi_m_sync_enable(&SPI_0);
	io_write(io, example_SPI_0, 12);
}

/**
 * Example of using TIMER_0.
 */
static struct timer_task TIMER_0_task1, TIMER_0_task2;

static void TIMER_0_task1_cb(const struct timer_task *const timer_task)
{
}

static void TIMER_0_task2_cb(const struct timer_task *const timer_task)
{
}

void TIMER_0_example(void)
{
	TIMER_0_task1.interval = 100;
	TIMER_0_task1.cb       = TIMER_0_task1_cb;
	TIMER_0_task1.mode     = TIMER_TASK_REPEAT;
	TIMER_0_task2.interval = 200;
	TIMER_0_task2.cb       = TIMER_0_task2_cb;
	TIMER_0_task2.mode     = TIMER_TASK_REPEAT;

	timer_add_task(&TIMER_0, &TIMER_0_task1);
	timer_add_task(&TIMER_0, &TIMER_0_task2);
	timer_start(&TIMER_0);
}

/**
 * Example of using TIMER_1.
 */
static struct timer_task TIMER_1_task1, TIMER_1_task2;

static void TIMER_1_task1_cb(const struct timer_task *const timer_task)
{
}

static void TIMER_1_task2_cb(const struct timer_task *const timer_task)
{
}

void TIMER_1_example(void)
{
	TIMER_1_task1.interval = 100;
	TIMER_1_task1.cb       = TIMER_1_task1_cb;
	TIMER_1_task1.mode     = TIMER_TASK_REPEAT;
	TIMER_1_task2.interval = 200;
	TIMER_1_task2.cb       = TIMER_1_task2_cb;
	TIMER_1_task2.mode     = TIMER_TASK_REPEAT;

	timer_add_task(&TIMER_1, &TIMER_1_task1);
	timer_add_task(&TIMER_1, &TIMER_1_task2);
	timer_start(&TIMER_1);
}

/**
 * Example of using TIMER_2.
 */
static struct timer_task TIMER_2_task1, TIMER_2_task2;

static void TIMER_2_task1_cb(const struct timer_task *const timer_task)
{
}

static void TIMER_2_task2_cb(const struct timer_task *const timer_task)
{
}

void TIMER_2_example(void)
{
	TIMER_2_task1.interval = 100;
	TIMER_2_task1.cb       = TIMER_2_task1_cb;
	TIMER_2_task1.mode     = TIMER_TASK_REPEAT;
	TIMER_2_task2.interval = 200;
	TIMER_2_task2.cb       = TIMER_2_task2_cb;
	TIMER_2_task2.mode     = TIMER_TASK_REPEAT;

	timer_add_task(&TIMER_2, &TIMER_2_task1);
	timer_add_task(&TIMER_2, &TIMER_2_task2);
	timer_start(&TIMER_2);
}

/**
 * Example of using TIMER_3.
 */
static struct timer_task TIMER_3_task1, TIMER_3_task2;

static void TIMER_3_task1_cb(const struct timer_task *const timer_task)
{
}

static void TIMER_3_task2_cb(const struct timer_task *const timer_task)
{
}

void TIMER_3_example(void)
{
	TIMER_3_task1.interval = 100;
	TIMER_3_task1.cb       = TIMER_3_task1_cb;
	TIMER_3_task1.mode     = TIMER_TASK_REPEAT;
	TIMER_3_task2.interval = 200;
	TIMER_3_task2.cb       = TIMER_3_task2_cb;
	TIMER_3_task2.mode     = TIMER_TASK_REPEAT;

	timer_add_task(&TIMER_3, &TIMER_3_task1);
	timer_add_task(&TIMER_3, &TIMER_3_task2);
	timer_start(&TIMER_3);
}

void I2C_0_example(void)
{
	struct io_descriptor *I2C_0_io;

	i2c_m_sync_get_io_descriptor(&I2C_0, &I2C_0_io);
	i2c_m_sync_enable(&I2C_0);
	i2c_m_sync_set_slaveaddr(&I2C_0, 0x12, I2C_M_SEVEN);
	io_write(I2C_0_io, (uint8_t *)"Hello World!", 12);
}

void ETHERNET_MAC_0_example(void)
{
	mac_async_enable(&ETHERNET_MAC_0);
	mac_async_write(&ETHERNET_MAC_0, (uint8_t *)"Hello World!", 12);
}
