#pragma once

void motor_init(void);
bool motor_handle_msg(void* message);
void motor_tick(int tick);
void motor_flash_start(void);
void motor_flash_stop(void);
