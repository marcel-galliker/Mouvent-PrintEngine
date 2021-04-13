#pragma once

#include <stdint.h>
#include <stdbool.h>

#include "rx_robot_tcpip.h"

/* Defines */

#define BOOTLOADER_DATA_FRAME_SIZE	64


/* ENUM definitons */

typedef enum {
	UNINITIALIZED_,
	WAITING_FOR_DATA_,
	WAITING_FOR_CONFIRM_
} BootloaderStatus;

/* Data structures */
#pragma pack(1)

typedef struct {
	BootloaderStatus status;
	uint32_t progMemPos;
	uint32_t progMemBlocksUsed;
	uint32_t progSize;
} BootloaderStatus_t;

typedef struct BootloaderStartCommand
{
	SMsgHdr header;
	uint32_t size;
	uint32_t checksum; // Not used yet
} BootloaderStartCommand_t;

typedef struct BootloaderDataCommand
{
	SMsgHdr header;
	uint8_t data[BOOTLOADER_DATA_FRAME_SIZE];
	uint32_t length;
} BootloaderDataCommand_t;
#pragma pack()
