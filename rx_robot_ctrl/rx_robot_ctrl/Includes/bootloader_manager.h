#pragma once


#include <stdint.h>
#include <stdbool.h>

bool bootloader_manager_start(void);
void bootloader_manager_main(void);
void bootloader_manager_handle_command(void* msg);
