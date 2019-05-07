// ****************************************************************************
//
//	spool_arp.c
//
// ****************************************************************************
//
//	Copyright 2016 by Radex AG, Switzerland. All rights reserved.
//	Written by Marcel Galliker 
//
// ****************************************************************************

//--- includes --------------------------------------------------
#include <sys/socket.h>
#include <sys/ioctl.h>

#include <linux/if_packet.h>
#include <linux/if_ether.h>
#include <linux/if_arp.h>

#include "rx_sok.h"
#include "spool_arp.h"


//--- structures --------------------------
#define	ETH_ARP 0x0806

struct __attribute__((packed)) arp_header
{
	unsigned short	hw_type;
	unsigned short	proto_type;
	unsigned char	hw_size;
	unsigned char	proto_size;
	unsigned short	opcode;
		#define ARP_REQUEST		0x0001
		#define ARP_REPLY		0x0002
	unsigned char	sender_mac[6];
	unsigned char	sender_ip[4];
	unsigned char	target_mac[6];
	unsigned char	target_ip[4];
};

typedef struct __attribute__((packed)) 
{
	struct ethhdr		eth;
	struct arp_header	arp;
} SArpMsg;


typedef struct
{
	char	name[10];
	UINT32	ip;
	UINT64  mac;
} SNetPortItem;

static SNetPortItem _NetPort[16];
static int			_Socket;

//--- arp_init ----------------------------
void arp_init(void)
{
	int _ifindex;
	struct ifreq ifr;
	int i, ret;
	UCHAR *ip, *mac;

	memset(_NetPort, 0, sizeof(_NetPort));

	/*open socket*/
	_Socket = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
	if (_Socket == -1) {
		perror("socket():");
		return;
	}
	
	for (i=0; i<SIZEOF(_NetPort); i++)
	{
		ifr.ifr_ifindex = i;
		if (ioctl(_Socket, SIOCGIFNAME, &ifr)==0)
		{
			strcpy(_NetPort[i].name, ifr.ifr_name);
			if (ioctl(_Socket, SIOCGIFADDR,   &ifr)==0) memcpy(&_NetPort[i].ip,  &ifr.ifr_addr.sa_data[2], sizeof(_NetPort[i].ip));
			if (ioctl(_Socket, SIOCGIFHWADDR, &ifr)==0)	memcpy(&_NetPort[i].mac, &ifr.ifr_hwaddr.sa_data,  sizeof(_NetPort[i].mac));
		}
	}

	/*
	for (i=0; i<SIZEOF(_NetPort); i++)
	{
		if (*_NetPort[i].name)
		{
			ip  = (UCHAR*)&_NetPort[i].ip;
			mac = (UCHAR*)&_NetPort[i].mac;
			printf("%d: >>%s<<\t%d.%d.%d.%d\t%02x:%02x:%02x:%02x:%02x:%02x\n", i, _NetPort[i].name, ip[0], ip[1], ip[2], ip[3], mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
		}
	}
	*/
}

//--- arp_request ------------------------------------------------
int arp_request(RX_SOCKET socket, UINT32 dst_ip)
{
	int i;
	UINT32 src_ip;
	SArpMsg msg;

	src_ip = sok_get_addr_32(socket);
	for (i=0; i<SIZEOF(_NetPort); i++)
	{
		if (_NetPort[i].ip == src_ip)
		{
			//--- msg.ethernet ---
			memset(&msg, 0, sizeof(msg));
			memcpy(msg.eth.h_source, &_NetPort[i].mac, sizeof(msg.eth.h_source));
		//	memcpy(msg.eth.h_dest,	 &dst_mac, sizeof(msg.eth.h_dest));
			memset(msg.eth.h_dest,	 0xff,	sizeof(msg.eth.h_dest));	// broadcast
			msg.eth.h_proto = htons(ETH_ARP);

			//---  msg.arp --------------------------
			msg.arp.hw_type		= htons(1);
			msg.arp.proto_type  = 0x0008;	// IPv4
			msg.arp.hw_size		= 6;
			msg.arp.proto_size	= 4;
			msg.arp.opcode		= htons(ARP_REQUEST);
			memcpy(&msg.arp.sender_mac, &_NetPort[i].mac, sizeof(msg.arp.sender_mac));
			memcpy(&msg.arp.sender_ip,  &src_ip,  sizeof(msg.arp.sender_ip));
			memset(&msg.arp.target_mac, 0x00,     sizeof(msg.arp.target_mac));
			memcpy(&msg.arp.target_ip,  &dst_ip,  sizeof(msg.arp.target_ip));

			//--- send the message ----------------------------------
			struct sockaddr_ll socket_address;
			int sent;
			//--- prepare sockaddr_ll ---
			memset(&socket_address, 0, sizeof(socket_address));
			socket_address.sll_family	= PF_PACKET;
			socket_address.sll_protocol = htons(ETH_P_IP);
			socket_address.sll_ifindex	= i;
			socket_address.sll_hatype	= ARPHRD_ETHER;
			socket_address.sll_pkttype	= PACKET_OTHERHOST;
			socket_address.sll_halen	= ETH_ALEN;

			memcpy(socket_address.sll_addr, msg.eth.h_dest, sizeof(msg.eth.h_dest));
			sent = sendto(_Socket, &msg, sizeof(msg), 0, (struct sockaddr*)&socket_address, sizeof(socket_address));
			if (sent == -1)
			{
				perror("sendto():");
			}
			return;
		}
	}
}

