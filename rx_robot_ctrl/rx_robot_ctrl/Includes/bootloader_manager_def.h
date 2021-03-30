#ifndef INCLUDES_BOOTLOADER_MANAGER_DEF_H_
#define INCLUDES_BOOTLOADER_MANAGER_DEF_H_

#include <stdint.h>
#include <stdbool.h>

/* Defines */

#define BOOTLOADER_DATA_FRAME_SIZE	64


/* ENUM definitons */

typedef enum {
	UNINITIALIZED,
	WAITING_FOR_DATA,
	WAITING_FOR_CONFIRM
} BootloaderStatus;

/* Data structures */
#pragma pack(1)

typedef struct {
	BootloaderStatus status;
	uint32_t progMemPos;
	uint32_t progMemBlocksUsed;
	uint32_t progSize;
} BootloaderStatus_t;

#pragma pack()

#endif /* INCLUDES_BOOTLOADER_MANAGER_DEF_H_ */
