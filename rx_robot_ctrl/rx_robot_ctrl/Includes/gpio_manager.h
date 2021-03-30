#ifndef INCLUDES_GPIO_MANAGER_H_
#define INCLUDES_GPIO_MANAGER_H_

#include "gpio_manager_def.h"

#include <stdint.h>
#include <stdbool.h>


bool gpio_manager_start(void);
bool gpio_manager_is_initalized(void);
void gpio_manager_set_output(uint8_t output, bool value);
bool gpio_manager_get_input(uint8_t input);
void gpio_manager_enable_motor(uint8_t motor);
void gpio_manager_disable_motor(uint8_t motor);
void gpio_manager_start_motor(uint8_t motor);
void gpio_manager_stop_motor(uint8_t motor);
GpioStatus_t* gpio_manager_get_status(void);

#endif /* INCLUDES_GPIO_MANAGER_H_ */
