#pragma once

#include <stdint.h>
#include <stdbool.h>

#include "../../includes/rx_robot_def.h"

bool  motor_start(void);
SMotorStatus*  motor_get_status(void);
void motor_handle_message(void* message);
void motor_main(void);
