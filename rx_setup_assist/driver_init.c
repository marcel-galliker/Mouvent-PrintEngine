/*
 * Code generated from Atmel Start.
 *
 * This file will be overwritten when reconfiguring your Atmel Start project.
 * Please copy examples or other code you want to keep to a separate file
 * to avoid losing it when reconfiguring.
 */

#include "driver_init.h"
#include <hal_init.h>
#include <hpl_pmc.h>
#include <peripheral_clk_config.h>
#include <utils.h>
#include <hpl_spi_base.h>
#include <hpl_tc.h>

#include <hpl_tc.h>

#include <hpl_tc.h>

#include <hpl_tc.h>

struct spi_m_sync_descriptor SPI_0;
struct timer_descriptor      TIMER_0;
struct timer_descriptor      TIMER_1;
struct timer_descriptor      TIMER_2;
struct timer_descriptor      TIMER_3;

struct i2c_m_sync_desc I2C_0;

struct mac_async_descriptor ETHERNET_MAC_0;

void EXTERNAL_IRQ_0_init(void)
{

	// Set pin direction to input
	gpio_set_pin_direction(Input1, GPIO_DIRECTION_IN);

	gpio_set_pin_pull_mode(Input1,
	                       // <y> Pull configuration
	                       // <id> pad_pull_config
	                       // <GPIO_PULL_OFF"> Off
	                       // <GPIO_PULL_UP"> Pull-up
	                       // <GPIO_PULL_DOWN"> Pull-down
	                       GPIO_PULL_OFF);

	gpio_set_pin_function(Input1, GPIO_PIN_FUNCTION_OFF);

	// Set pin direction to input
	gpio_set_pin_direction(Input2, GPIO_DIRECTION_IN);

	gpio_set_pin_pull_mode(Input2,
	                       // <y> Pull configuration
	                       // <id> pad_pull_config
	                       // <GPIO_PULL_OFF"> Off
	                       // <GPIO_PULL_UP"> Pull-up
	                       // <GPIO_PULL_DOWN"> Pull-down
	                       GPIO_PULL_OFF);

	gpio_set_pin_function(Input2, GPIO_PIN_FUNCTION_OFF);

	// Set pin direction to input
	gpio_set_pin_direction(PHY_Interrupt, GPIO_DIRECTION_IN);

	gpio_set_pin_pull_mode(PHY_Interrupt,
	                       // <y> Pull configuration
	                       // <id> pad_pull_config
	                       // <GPIO_PULL_OFF"> Off
	                       // <GPIO_PULL_UP"> Pull-up
	                       // <GPIO_PULL_DOWN"> Pull-down
	                       GPIO_PULL_OFF);

	gpio_set_pin_function(PHY_Interrupt, GPIO_PIN_FUNCTION_OFF);
}

void SPI_0_PORT_init(void)
{

	gpio_set_pin_function(SM_SDO, MUX_PD20B_SPI0_MISO);

	gpio_set_pin_function(SM_SDI, MUX_PD21B_SPI0_MOSI);

	gpio_set_pin_function(SM_CK, MUX_PD22B_SPI0_SPCK);
}

void SPI_0_CLOCK_init(void)
{
	_pmc_enable_periph_clock(ID_SPI0);
}

void SPI_0_init(void)
{
	SPI_0_CLOCK_init();
	spi_m_sync_set_func_ptr(&SPI_0, _spi_get_spi_m_sync());
	spi_m_sync_init(&SPI_0, SPI0);
	SPI_0_PORT_init();
}

void TIMER_0_PORT_init(void)
{

	gpio_set_pin_function(ENC_A, MUX_PA0B_TC0_TIOA0);

	gpio_set_pin_function(ENC_B, MUX_PA1B_TC0_TIOB0);

	gpio_set_pin_function(ENC_N, MUX_PA16B_TC0_TIOB1);
}
/**
 * \brief Timer initialization function
 *
 * Enables Timer peripheral, clocks and initializes Timer driver
 */
static void TIMER_0_init(void)
{
	_pmc_enable_periph_clock(ID_TC0_CHANNEL0);
	TIMER_0_PORT_init();
	timer_init(&TIMER_0, TC0, _tc_get_timer());
}

void TIMER_1_PORT_init(void)
{
}
/**
 * \brief Timer initialization function
 *
 * Enables Timer peripheral, clocks and initializes Timer driver
 */
static void TIMER_1_init(void)
{
	_pmc_enable_periph_clock(ID_TC1_CHANNEL0);
	TIMER_1_PORT_init();
	timer_init(&TIMER_1, TC1, _tc_get_timer());
}

void TIMER_2_PORT_init(void)
{
}
/**
 * \brief Timer initialization function
 *
 * Enables Timer peripheral, clocks and initializes Timer driver
 */
static void TIMER_2_init(void)
{
	_pmc_enable_periph_clock(ID_TC2_CHANNEL0);
	TIMER_2_PORT_init();
	timer_init(&TIMER_2, TC2, _tc_get_timer());
}

void TIMER_3_PORT_init(void)
{
}
/**
 * \brief Timer initialization function
 *
 * Enables Timer peripheral, clocks and initializes Timer driver
 */
static void TIMER_3_init(void)
{
	_pmc_enable_periph_clock(ID_TC3_CHANNEL0);
	TIMER_3_PORT_init();
	timer_init(&TIMER_3, TC3, _tc_get_timer());
}

void I2C_0_PORT_init(void)
{

	gpio_set_pin_function(SCL_M1, MUX_PA4A_TWIHS0_TWCK0);

	gpio_set_pin_function(SDA_M1, MUX_PA3A_TWIHS0_TWD0);
}

void I2C_0_CLOCK_init(void)
{
	_pmc_enable_periph_clock(ID_TWIHS0);
}

void I2C_0_init(void)
{
	I2C_0_CLOCK_init();

	i2c_m_sync_init(&I2C_0, TWIHS0);

	I2C_0_PORT_init();
}

void ETHERNET_MAC_0_PORT_init(void)
{

	gpio_set_pin_function(PHY_MDC, MUX_PD8A_GMAC_GMDC);

	gpio_set_pin_function(PHY_MDIO, MUX_PD9A_GMAC_GMDIO);

	gpio_set_pin_function(PHY_RXD0, MUX_PD5A_GMAC_GRX0);

	gpio_set_pin_function(PHY_RXD1, MUX_PD6A_GMAC_GRX1);

	gpio_set_pin_function(PHY_CRS_DV, MUX_PD4A_GMAC_GRXDV);

	gpio_set_pin_function(PHY_RXER, MUX_PD7A_GMAC_GRXER);

	gpio_set_pin_function(PHY_TXD0, MUX_PD2A_GMAC_GTX0);

	gpio_set_pin_function(PHY_TXD1, MUX_PD3A_GMAC_GTX1);

	gpio_set_pin_function(PHY_REFCLK, MUX_PD0A_GMAC_GTXCK);

	gpio_set_pin_function(PHY_TXEN, MUX_PD1A_GMAC_GTXEN);
}

void ETHERNET_MAC_0_CLOCK_init(void)
{
	_pmc_enable_periph_clock(ID_GMAC);
}

void ETHERNET_MAC_0_init(void)
{
	ETHERNET_MAC_0_CLOCK_init();
	mac_async_init(&ETHERNET_MAC_0, GMAC);
	ETHERNET_MAC_0_PORT_init();
}

void system_init(void)
{
	init_mcu();

	_pmc_enable_periph_clock(ID_PIOA);

	_pmc_enable_periph_clock(ID_PIOB);

	_pmc_enable_periph_clock(ID_PIOD);

	/* Disable Watchdog */
	hri_wdt_set_MR_WDDIS_bit(WDT);

	/* GPIO on PA11 */

	gpio_set_pin_level(SM_STBY_RESET,
	                   // <y> Initial level
	                   // <id> pad_initial_level
	                   // <false"> Low
	                   // <true"> High
	                   false);

	// Set pin direction to output
	gpio_set_pin_direction(SM_STBY_RESET, GPIO_DIRECTION_OUT);

	gpio_set_pin_function(SM_STBY_RESET, GPIO_PIN_FUNCTION_OFF);

	/* GPIO on PA12 */

	gpio_set_pin_level(LED,
	                   // <y> Initial level
	                   // <id> pad_initial_level
	                   // <false"> Low
	                   // <true"> High
	                   true);

	// Set pin direction to output
	gpio_set_pin_direction(LED, GPIO_DIRECTION_OUT);

	gpio_set_pin_function(LED, GPIO_PIN_FUNCTION_OFF);

	/* GPIO on PA15 */

	// Set pin direction to input
	gpio_set_pin_direction(SM_FLAG, GPIO_DIRECTION_IN);

	gpio_set_pin_pull_mode(SM_FLAG,
	                       // <y> Pull configuration
	                       // <id> pad_pull_config
	                       // <GPIO_PULL_OFF"> Off
	                       // <GPIO_PULL_UP"> Pull-up
	                       // <GPIO_PULL_DOWN"> Pull-down
	                       GPIO_PULL_OFF);

	gpio_set_pin_function(SM_FLAG, GPIO_PIN_FUNCTION_OFF);

	/* GPIO on PA21 */

	// Set pin direction to input
	gpio_set_pin_direction(DiscreteSignalOutput, GPIO_DIRECTION_IN);

	gpio_set_pin_pull_mode(DiscreteSignalOutput,
	                       // <y> Pull configuration
	                       // <id> pad_pull_config
	                       // <GPIO_PULL_OFF"> Off
	                       // <GPIO_PULL_UP"> Pull-up
	                       // <GPIO_PULL_DOWN"> Pull-down
	                       GPIO_PULL_OFF);

	gpio_set_pin_function(DiscreteSignalOutput, GPIO_PIN_FUNCTION_OFF);

	/* GPIO on PA24 */

	// Set pin direction to input
	gpio_set_pin_direction(SM_BUSY_SYNC, GPIO_DIRECTION_IN);

	gpio_set_pin_pull_mode(SM_BUSY_SYNC,
	                       // <y> Pull configuration
	                       // <id> pad_pull_config
	                       // <GPIO_PULL_OFF"> Off
	                       // <GPIO_PULL_UP"> Pull-up
	                       // <GPIO_PULL_DOWN"> Pull-down
	                       GPIO_PULL_OFF);

	gpio_set_pin_function(SM_BUSY_SYNC, GPIO_PIN_FUNCTION_OFF);

	/* GPIO on PA26 */

	gpio_set_pin_level(Watchdog,
	                   // <y> Initial level
	                   // <id> pad_initial_level
	                   // <false"> Low
	                   // <true"> High
	                   true);

	// Set pin direction to output
	gpio_set_pin_direction(Watchdog, GPIO_DIRECTION_OUT);

	gpio_set_pin_function(Watchdog, GPIO_PIN_FUNCTION_OFF);

	/* GPIO on PA28 */

	gpio_set_pin_level(PHY_RESET,
	                   // <y> Initial level
	                   // <id> pad_initial_level
	                   // <false"> Low
	                   // <true"> High
	                   false);

	// Set pin direction to output
	gpio_set_pin_direction(PHY_RESET, GPIO_DIRECTION_OUT);

	gpio_set_pin_function(PHY_RESET, GPIO_PIN_FUNCTION_OFF);

	/* GPIO on PB2 */

	gpio_set_pin_level(SM_CS,
	                   // <y> Initial level
	                   // <id> pad_initial_level
	                   // <false"> Low
	                   // <true"> High
	                   true);

	// Set pin direction to output
	gpio_set_pin_direction(SM_CS, GPIO_DIRECTION_OUT);

	gpio_set_pin_function(SM_CS, GPIO_PIN_FUNCTION_OFF);

	/* GPIO on PD18 */

	gpio_set_pin_level(SM_STEP_CLK,
	                   // <y> Initial level
	                   // <id> pad_initial_level
	                   // <false"> Low
	                   // <true"> High
	                   false);

	// Set pin direction to output
	gpio_set_pin_direction(SM_STEP_CLK, GPIO_DIRECTION_OUT);

	gpio_set_pin_function(SM_STEP_CLK, GPIO_PIN_FUNCTION_OFF);

	/* GPIO on PD19 */

	gpio_set_pin_level(SM_EXT_INPUT,
	                   // <y> Initial level
	                   // <id> pad_initial_level
	                   // <false"> Low
	                   // <true"> High
	                   true);

	// Set pin direction to output
	gpio_set_pin_direction(SM_EXT_INPUT, GPIO_DIRECTION_OUT);

	gpio_set_pin_function(SM_EXT_INPUT, GPIO_PIN_FUNCTION_OFF);

	/* GPIO on PD30 */

	gpio_set_pin_level(TriggerInput,
	                   // <y> Initial level
	                   // <id> pad_initial_level
	                   // <false"> Low
	                   // <true"> High
	                   false);

	// Set pin direction to output
	gpio_set_pin_direction(TriggerInput, GPIO_DIRECTION_OUT);

	gpio_set_pin_function(TriggerInput, GPIO_PIN_FUNCTION_OFF);

	EXTERNAL_IRQ_0_init();

	SPI_0_init();
	TIMER_0_init();
	TIMER_1_init();
	TIMER_2_init();
	TIMER_3_init();

	I2C_0_init();

	ETHERNET_MAC_0_init();

	ext_irq_init();
}
