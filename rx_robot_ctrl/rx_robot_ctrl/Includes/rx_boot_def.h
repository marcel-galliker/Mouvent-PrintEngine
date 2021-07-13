#pragma once

#include "rx_types.h"

#define IP_ADDR_SIZE	32

#pragma pack(1)

typedef struct SNetworkItem
{
	char	deviceTypeStr[32];
	char	serialNo[32];
	UINT64	macAddr;
	UCHAR	deviceType;
	INT8	deviceNo;
	char	ipAddr[32];
	UINT8	connected;
	UINT8	platform;	// EPlatform
	UINT32	rfsPort;
	UINT32	ports[8];
} SNetworkItem;

typedef struct SBootInfoMsg
{
	UINT32			id;
	SNetworkItem	item;
} SBootInfoMsg;

typedef struct SBootInfoReqCmd
{
	UINT32	cmd;
} SBootInfoReqCmd;

typedef struct SBootAddrSetCmd
{
	UINT32			id;
	UINT64			macAddr;
	char			ipAddr[IP_ADDR_SIZE];
} SBootAddrSetCmd;

#pragma pack(0)
