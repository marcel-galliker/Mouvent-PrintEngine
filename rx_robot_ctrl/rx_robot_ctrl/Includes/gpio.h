#ifndef INCLUDES_GPIO_H_
#define INCLUDES_GPIO_H_

#include <stdint.h>
#include <stdbool.h>


bool gpio_start(void);
void gpio_handle_message(void* message);
void gpio_main(void);
void gpio_set_output(uint8_t output, bool value);
bool gpio_get_input(uint8_t input);
void gpio_enable_motor(uint8_t motor, int enable);
void gpio_start_motor(uint8_t motor);
void gpio_stop_motor(uint8_t motor);

#endif /* INCLUDES_GPIO_H_ */
