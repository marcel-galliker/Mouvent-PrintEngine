#pragma once

#include "rx_robot_tcpip.h"
#include "net.h"


//--- DEFINES -------------------------------------------
#define FLASH_SECTOR_SIZE	4096
#define FLASH_SECTOR_CNT	64
#define FLASH_SIZE			(FLASH_SECTOR_CNT*FLASH_SECTOR_SIZE)

#define FLASH_SECTOR_APP	25
#define FLASH_SECTOR_USER	62

//--- methods -----------------------------------------

void 	flash_init(void);

int		flash_serialNo_Valid(void);
UINT16 	flash_read_serialNo(void);
void 	flash_write_serialNo(UINT16 serialNo);
void 	flash_read_ipAddr(ip_addr_t *pipAddr);
void	flash_write_ipAddr(ip_addr_t *pipAddr);

