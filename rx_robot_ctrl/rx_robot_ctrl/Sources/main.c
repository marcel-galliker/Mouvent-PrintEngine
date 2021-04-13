#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "sys/types.h"

#include "ft900.h"

#include "tinyprintf.h"

#include "FreeRTOS.h"
#include "task.h"

#include "gpio_manager.h"
#include "network_manager.h"
#include "rx_boot.h"
#include "rx_trace.h"
#include "status_manager.h"
#include "bootloader_manager.h"
#include "motor_manager.h"

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

    network_manager_start();
    gpio_manager_start();
    motor_manager_start();
    rx_boot_start();
    status_manager_start();
    bootloader_manager_start();

    vTaskStartScheduler();

    for (;;)
        ;
}
