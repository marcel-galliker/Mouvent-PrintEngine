#ifndef INCLUDES_BOOTLOADER_MANAGER_H_
#define INCLUDES_BOOTLOADER_MANAGER_H_

#include "bootloader_manager_def.h"

#include <stdint.h>
#include <stdbool.h>

bool bootloader_manager_start(void);
bool bootloader_manager_is_initalized(void);
BootloaderStatus_t* bootloader_manager_get_status(void);

#endif /* INCLUDES_BOOTLOADER_MANAGER_H_ */
