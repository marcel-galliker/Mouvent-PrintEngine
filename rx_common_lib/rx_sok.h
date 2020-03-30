// ****************************************************************************
//
//	rx_sok.h
//
// ****************************************************************************
//
//	Copyright 2013 by Radex AG, Switzerland. All rights reserved.
//	Written by Marcel Galliker 
//
// ****************************************************************************

#pragma once


#ifdef __cplusplus
extern "C"{
#endif

#include "rx_common.h"

#ifdef linux
	#include <netinet/in.h>
#else
	#include <winsock.h>
#endif

//--- adaption for linux ------------------------------------------------------

#ifdef linux
	typedef int RX_SOCKET;
	#define INVALID_SOCKET	-1
	#define SOCKET_ERROR	-1
	#define TIMEVAL struct timeval
#else
	typedef SOCKET RX_SOCKET;
	#define MSG_NOSIGNAL	0	// for windows!
#endif

typedef HANDLE RX_RAW_SOCKET;

#define NO_SOCKET	(RX_SOCKET)0


// #define MAX_UDP_DATA_SIZE	1472
// #define MAX_TCP_DATA_SIZE	1460

#define MAX_UDP_DATA_SIZE	(9000-28)	// jumbo frames!

#pragma pack(1)
typedef struct
{
	char	hostname[64];
	INT32	dhcp;
	UINT8	addr[4];
	UINT8	mask[4];
} SIfConfig;
#pragma pack()
	
//--- prototypes ---------------------------------------------------------------

int		sok_get_ifconfig		(const char *ifname, SIfConfig *config);
int		sok_set_ifconfig		(const char *ifname, SIfConfig *config);
int		sok_get_ifcnt			(const char *grep);
	
void	sok_get_mac_address		(const char *deviceName, UINT64 *macAddr);
void	sok_get_ip_address_str	(const char *deviceName, char *addr, int size);
void	sok_get_ip_address_num	(const char *deviceName, UINT32 *addr);
void	sok_set_ip_address_num	(const char *deviceName, unsigned char addr[4]);
int		sok_set_ip_address_str	(const char *deviceName, const char *addr);
int		sok_sockaddr			(struct sockaddr_in *ipAddr, const char *addr, int port);
UINT32	sok_addr_32				(const char *addr);
char*   sok_addr_str			(UINT32 addr, char *str);
char*	sok_get_peer_name		(RX_SOCKET socket, char *namestr, char *ipstr, UINT32 *pport);
char*	sok_get_peer_name_raw	(RX_RAW_SOCKET socket, char *namestr, char *ipstr, UINT32 *pport);
char*	sok_get_socket_name		(RX_SOCKET socket, char *namestr, char *ipstr, UINT32 *pport);
char*	sok_get_socket_name_raw	(RX_RAW_SOCKET socket, char *namestr, char *ipstr, UINT32 *pport);
void	sok_get_server_name		(HANDLE hserver, char *namestr, char *ipstr, UINT32 *pport);
void	sok_get_srv_socket_name	(HANDLE hserver, char *namestr, char *ipstr, UINT32 *pport);
UINT32	sok_get_addr_32			(RX_SOCKET socket);
char*	sok_get_addr_str		(struct sockaddr *addr, char *str, int size);
char*	sok_mac_addr_str		(UINT64 macAddr, char *str, int size);

typedef int(*msg_handler)		(RX_SOCKET socket, void *msg, int len, struct sockaddr	*sender, void *par);
typedef int(*open_handler)		(RX_SOCKET socket, const char *peerName);
typedef int(*close_handler)		(RX_SOCKET socket, const char *peerName);

int sok_start_server	(HANDLE    *hserver, const char *addr, int port, int type, int maxConnections, msg_handler handle_msg, open_handler handle_open, close_handler handle_close);
int sok_stop_server		(HANDLE    *hserver);
int sok_open_client		(RX_SOCKET *psocket, const char *addr, int port, int type);
int sok_open_client_2	(RX_SOCKET *psocket, const char *addr, int port, int type, msg_handler, close_handler handle_close);
int sok_close			(RX_SOCKET *psocket);
RX_SOCKET sok_socket	(HANDLE    hserver);

int sok_open_raw		(RX_RAW_SOCKET *psocket, const char *if_name, const char *addr, int port);
int sok_send_raw_udp	(RX_RAW_SOCKET *psocket, UINT64 dstMacAddr, UINT32 dstIpAddr, UINT32 dstPort, void *data, int dataLen);
int sok_send_arp		(RX_RAW_SOCKET *psocket, UINT64 dstMacAddr, void *data, int dataLen);
int sok_close_raw		(RX_RAW_SOCKET *psocket);
int sok_ping			(const char *addr);

void sok_debug			(RX_SOCKET socket);
int  sok_receiver	    (HANDLE hserver, RX_SOCKET *psocket, msg_handler handle_msg, void *par);
int  sok_send			(RX_SOCKET *socket, void *msg);
int  sok_send_wide		(RX_SOCKET *socket, void *msg);
int  sok_send_2			(RX_SOCKET *socket, INT32 id, UINT32 dataLen, void *data);
// int  sok_send_3         (RX_SOCKET *socket, UINT32 dataLen, void *data);
int  sok_send_to_clients(HANDLE hserver, RX_SOCKET *socket, void *msg);	// return=clients count
int  sok_send_to_clients_2(HANDLE hserver,  RX_SOCKET *socket, INT32 id, UINT32 len, void *data);
int  sok_send_to_clients_telnet(HANDLE hserver, const void *data, UINT32 lrn);
int  sok_sendto			(HANDLE hserver, const void *buf, UINT32 len, int flags, const struct sockaddr *dest_addr, int addrlen);
int  sok_broadcast		(RX_SOCKET socket, const void *buf, UINT32 len, const char *ipAddr, int port);
	
int  sok_get_term_str	(HANDLE hserver, char *str, int size);

int sok_check_addr_str	(RX_SOCKET *psocket, char *addr, char *file, int line);
int sok_check_addr_32	(RX_SOCKET *psocket, UINT32 addr, char *file, int line);
	
HANDLE debug_mutex(HANDLE hserver);

#ifdef linux
	UINT32 sok_get_netspeed(const char *deviceName);
#else
	UINT32 sok_get_netspeed(RX_SOCKET socket);
#endif

#ifdef __cplusplus
}
#endif
