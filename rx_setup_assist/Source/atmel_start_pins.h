/*
 * Code generated from Atmel Start.
 *
 * This file will be overwritten when reconfiguring your Atmel Start project.
 * Please copy examples or other code you want to keep to a separate file
 * to avoid losing it when reconfiguring.
 */
#ifndef ATMEL_START_PINS_H_INCLUDED
#define ATMEL_START_PINS_H_INCLUDED

#include <hal_gpio.h>

// SAME70 has 4 pin functions

#define GPIO_PIN_FUNCTION_A 0
#define GPIO_PIN_FUNCTION_B 1
#define GPIO_PIN_FUNCTION_C 2
#define GPIO_PIN_FUNCTION_D 3

#define ENC_A GPIO(GPIO_PORTA, 0)
#define ENC_B GPIO(GPIO_PORTA, 1)
#define SDA_M1 GPIO(GPIO_PORTA, 3)
#define SCL_M1 GPIO(GPIO_PORTA, 4)
#define SM_STBY_RESET GPIO(GPIO_PORTA, 11)
#define LED GPIO(GPIO_PORTA, 12)
#define SM_FLAG GPIO(GPIO_PORTA, 15)
#define ENC_N GPIO(GPIO_PORTA, 16)
#define Input1 GPIO(GPIO_PORTA, 17)
#define Input2 GPIO(GPIO_PORTA, 18)
#define DiscreteSignalOutput GPIO(GPIO_PORTA, 21)
#define SM_BUSY_SYNC GPIO(GPIO_PORTA, 24)
#define Watchdog GPIO(GPIO_PORTA, 26)
#define PHY_RESET GPIO(GPIO_PORTA, 28)
#define PHY_Interrupt GPIO(GPIO_PORTA, 31)
#define SM_CS GPIO(GPIO_PORTB, 2)
#define PHY_REFCLK GPIO(GPIO_PORTD, 0)
#define PHY_TXEN GPIO(GPIO_PORTD, 1)
#define PHY_TXD0 GPIO(GPIO_PORTD, 2)
#define PHY_TXD1 GPIO(GPIO_PORTD, 3)
#define PHY_CRS_DV GPIO(GPIO_PORTD, 4)
#define PHY_RXD0 GPIO(GPIO_PORTD, 5)
#define PHY_RXD1 GPIO(GPIO_PORTD, 6)
#define PHY_RXER GPIO(GPIO_PORTD, 7)
#define PHY_MDC GPIO(GPIO_PORTD, 8)
#define PHY_MDIO GPIO(GPIO_PORTD, 9)
#define SM_STEP_CLK GPIO(GPIO_PORTD, 18)
#define SM_EXT_INPUT GPIO(GPIO_PORTD, 19)
#define SM_SDO GPIO(GPIO_PORTD, 20)
#define SM_SDI GPIO(GPIO_PORTD, 21)
#define SM_CK GPIO(GPIO_PORTD, 22)
#define TriggerInput GPIO(GPIO_PORTD, 30)

#endif // ATMEL_START_PINS_H_INCLUDED
