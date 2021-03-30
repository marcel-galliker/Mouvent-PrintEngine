#ifndef INCLUDES_RX_BOOT_DEF_H_
#define INCLUDES_RX_BOOT_DEF_H_

#include <stdbool.h>
#include <stdint.h>

#define IP_ADDR_SIZE	32

#pragma pack(1)

typedef struct NetworkItem
{
	int8_t		deviceTypeStr[32];
	int8_t		serialNo[32];
	uint64_t	macAddr;
	uint8_t		deviceType;
	int8_t		deviceNo;
	int8_t		ipAddr[32];
	uint8_t		connected;
	uint8_t		platform;
	uint32_t	rfsPort;
	uint32_t	ports[8];
} NetworkItem_t;

typedef struct BootInfoMsg
{
	uint32_t		id;
	NetworkItem_t	item;
} BootInfoMsg_t;

typedef struct BootInfoReqCmd
{
	uint32_t	cmd;
} BootInfoReqCmd_t;

typedef struct BootAddrSetCmd
{
	uint32_t	id;
	uint64_t	macAddr;
	uint8_t		ipAddr[IP_ADDR_SIZE];
} BootAddrSetCmd_t;

#pragma pack(0)

#endif /* INCLUDES_RX_BOOT_DEF_H_ */
