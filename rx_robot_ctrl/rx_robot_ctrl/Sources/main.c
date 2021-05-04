#include <gpio.h>
#include <motor.h>
#include <network.h>
#include <status.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "sys/types.h"

#include "ft900.h"

#include "tinyprintf.h"

#include "FreeRTOS.h"
#include "task.h"

#include "motor.h"
#include "robot_flash.h"
#include "rx_boot.h"
#include "rx_trace.h"
#include "bootloader.h"

// ignore debug statements
#define DEBUG_PRINTF(...)


/** tfp_printf putc
 * 	Empty shell for possble debug output.
 *
 *  @param p Parameters
 *  @param c The character to write */
void myputc(void* p, char c) {

}

int main(void)
{
    sys_reset_all();
    interrupt_disable_globally();

    TrEnable(true);

    init_printf(NULL, myputc);

    flash_init();
    network_start();
    gpio_start();
    motor_start();
    rx_boot_start();
    status_start();
    bootloader_start();

    vTaskStartScheduler();

    for (;;)
        ;
}
