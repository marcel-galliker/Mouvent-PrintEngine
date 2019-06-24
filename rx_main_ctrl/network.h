// ****************************************************************************
//
//	Network.h
//
//	main controller tasks.
//
// ****************************************************************************
//
//	Copyright 2014 by Radex AG, Switzerland. All rights reserved.
//	Written by Marcel Galliker 
//
// ****************************************************************************

#pragma once

#include "rx_def.h"
#include "rx_sok.h"

void net_init(void);
void net_end(void);
void net_tick(void);


void net_save				(const char *filename);
void net_get_ip_addr		(SNetworkItem *item, char *ipAddr);
int  net_register			(SNetworkItem *item);
int	 net_booted				(UINT64 macAddr);
int	 net_is_booted			(UINT64 macAddr);
void net_register_by_device	(EDevice dev, int no);
int	 net_is_registered		(EDevice dev, int no);
void net_unregister			(SNetworkItem *item);
void net_disconnnected		(SNetworkItem *item);
int  net_port_listening		(EDevice dev, int no, int port);

//void net_connected			(EDevice dev, int no);§
//void net_device_disconnected(EDevice dev, int no);
int  net_get_ipaddr			(SNetworkItem *item, char *ipAddr, int size);
int  net_device_to_ipaddr	(EDevice dev, int no, char *ipAddr, int size);
void net_ipaddr_to_device	(const char *ipAddr, EDevice *pdev, int *pno);

UINT32 net_head_ctrl_addr	(int headNo);
UINT32 net_head_data_addr	(int headNo, int udpNo, int portCnt);

void net_send_config		(RX_SOCKET socket);
void net_set_config	  		(RX_SOCKET socket, SIfConfig *config);
void net_reset				(void);
void net_send_items			(RX_SOCKET socket, int reload);
int  net_set_item			(RX_SOCKET socket, SNetworkItem *item, int flash);
int  net_delete_item		(RX_SOCKET socket, SNetworkItem *item);
void net_send_item			(EDevice deviceType, int no);
