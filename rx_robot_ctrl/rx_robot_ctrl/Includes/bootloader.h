#pragma once


#include <stdint.h>
#include <stdbool.h>

bool bootloader_start(void);
void bootloader_main(void);
void bootloader_handle_command(void* msg);
