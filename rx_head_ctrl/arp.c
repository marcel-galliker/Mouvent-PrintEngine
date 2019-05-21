// ****************************************************************************
//
//	DIGITAL PRINTING - arp.c
//
//	Communication between ARM processor and FPGA
//
// ****************************************************************************
//
//	Copyright 2015 by radex AG, Switzerland. All rights reserved.
//	Written by Marcel Galliker
//
// ****************************************************************************


long	ARP_RequestCtr[2]={0,0};
long	ARP_ReplyCtr[2]={0,0};

#ifdef linux

#include "rx_common.h"
#include "rx_threads.h"

#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/time.h>

#include <asm/types.h>

#include <math.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

#include <linux/if_packet.h>
#include <linux/if_ether.h>
#include <linux/if_arp.h>

#include "fpga.h"

#define BUF_SIZE 42
#define DEVICE "eth0"
#define ETH_P_NULL 0x0
#define ETH_MAC_LEN ETH_ALEN
#define	ETH_ARP 0x0806

#define ARP_TRACE		FALSE

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

static FILE *_TraceFile=NULL;

//--- prototypes ---------------------------
static void _arp_handle_msg(int udpNo, SArpMsg *msg);
static void _arp_trace_msg (int udpNo, SArpMsg *pmsg, char *title);

//--- arp_init -------------------------------------------
void arp_init()
{
	_TraceFile = fopen("/tmp/arp_trace.txt", "w");
}

//--- arp ----------------------------------------------------
void arp_main(int ticks, int menu)
{
	int udpNo;
	SArpMsg msg;

	if (menu)
	{
		for (udpNo=0; udpNo<MAX_ETH_PORT; udpNo++)
		{
			if (fpga_get_arp_buf(udpNo, (BYTE*)&msg, sizeof(SArpMsg))==REPLY_OK)
			{
				if (htons(msg.eth.h_proto) ==  ETH_ARP)
					_arp_handle_msg(udpNo, &msg);
			}
		}
	}
}

//--- _arp_trace_msg ------------------------------------------------------
static void _arp_trace_msg(int udpNo, SArpMsg *pmsg, char *title)
{
//	if (!ARP_TRACE) return;
	if (!_TraceFile) return;

	fprintf(_TraceFile, "\n--- UDP[%d]: %s -----------------------------\n", udpNo, title);
	{
		extern SFpga	Fpga;
		fprintf(_TraceFile, "\t udp alive: %03d   %03d,    time=%d\n", Fpga.stat->udp_alive[0], Fpga.stat->udp_alive[1], rx_get_ticks());
	}
	fprintf(_TraceFile, "\t- Ethernet Header---------------------------\n");
	fprintf(_TraceFile, "\tDestination: %02X:%02X:%02X:%02X:%02X:%02X\n", pmsg->eth.h_dest[0], pmsg->eth.h_dest[1], pmsg->eth.h_dest[2], pmsg->eth.h_dest[3], pmsg->eth.h_dest[4], pmsg->eth.h_dest[5]);
	fprintf(_TraceFile, "\tSource:      %02X:%02X:%02X:%02X:%02X:%02X\n", pmsg->eth.h_source[0], pmsg->eth.h_source[1], pmsg->eth.h_source[2], pmsg->eth.h_source[3], pmsg->eth.h_source[4], pmsg->eth.h_source[5]);
	fprintf(_TraceFile, "\tType:        0x%02x\n", htons(pmsg->eth.h_proto));

	fprintf(_TraceFile, "\t- ARP Header ---------------------------------\n");
	fprintf(_TraceFile, "\tHardware type: 0x%x\n", htons(pmsg->arp.hw_type));
	fprintf(_TraceFile, "\tProtocol type: 0x%x\n", htons(pmsg->arp.proto_type));
	fprintf(_TraceFile, "\tHardware size: %d\n",	pmsg->arp.hw_size);
	fprintf(_TraceFile, "\tProtocol size: %d \n",	pmsg->arp.proto_size);
	fprintf(_TraceFile, "\tOpcode: %d \n",			htons(pmsg->arp.opcode));
	fprintf(_TraceFile, "\tSender MAC address: %02X:%02X:%02X:%02X:%02X:%02X\n",
		pmsg->arp.sender_mac[0],
		pmsg->arp.sender_mac[1],
		pmsg->arp.sender_mac[2],
		pmsg->arp.sender_mac[3],
		pmsg->arp.sender_mac[4],
		pmsg->arp.sender_mac[5]
		);
	fprintf(_TraceFile, "\tSender IP  address: %d.%d.%d.%d\n",
		pmsg->arp.sender_ip[0],
		pmsg->arp.sender_ip[1],
		pmsg->arp.sender_ip[2],
		pmsg->arp.sender_ip[3]
		);
	fprintf(_TraceFile, "\tTarget MAC address: %02X:%02X:%02X:%02X:%02X:%02X\n",
		pmsg->arp.target_mac[0],
		pmsg->arp.target_mac[1],
		pmsg->arp.target_mac[2],
		pmsg->arp.target_mac[3],
		pmsg->arp.target_mac[4],
		pmsg->arp.target_mac[5]
		);
	fprintf(_TraceFile, "\tTarget IP  address: %d.%d.%d.%d\n",
		pmsg->arp.target_ip[0],
		pmsg->arp.target_ip[1],
		pmsg->arp.target_ip[2],
		pmsg->arp.target_ip[3]
		);
	
}

//--- _arp_handle_msg -------------------------------------------------------
static void _arp_handle_msg(int udpNo, SArpMsg *pmsg)
{
	SArpMsg	reply;
	UINT32 addr, addr_msg;

	if(htons(pmsg->arp.hw_type) != 0x0001)
		return;

	ARP_RequestCtr[udpNo]++;
	_arp_trace_msg(udpNo, pmsg, "ARP REQUEST");
	fpga_get_ip_addr(udpNo, &addr);
	addr_msg = *((UINT32*)pmsg->arp.target_ip); 
	if (addr_msg != addr)
	{
		UCHAR *a = (UCHAR*)&addr;

//		if (ARP_TRACE)
		if (_TraceFile)
			fprintf(_TraceFile, "Requested IP-Address >>%d.%d.%d.%d<< does not match >>%d.%d.%d.%d<<\n", pmsg->arp.target_ip[0], pmsg->arp.target_ip[1], pmsg->arp.target_ip[2], pmsg->arp.target_ip[3], a[0], a[1], a[2], a[3]);
		return;
	}
	//--- reply.ethernet ---
	memset(&reply, 0, sizeof(reply));
	memcpy(reply.eth.h_dest, pmsg->eth.h_source, sizeof(reply.eth.h_dest));
	fpga_get_mac_addr(udpNo, reply.eth.h_source);
	reply.eth.h_proto = htons(ETH_ARP);

	//---  reply.arp --------------------------
	memcpy(&reply.arp, &pmsg->arp, sizeof(reply.arp));
	reply.arp.opcode = htons(ARP_REPLY);
	memcpy(reply.arp.target_mac, pmsg->arp.sender_mac,	sizeof(reply.arp.target_mac));
	memcpy(reply.arp.target_ip,  pmsg->arp.sender_ip,	sizeof(reply.arp.target_ip));
	fpga_get_mac_addr(udpNo, reply.arp.sender_mac);
	memcpy(reply.arp.sender_ip,  pmsg->arp.target_ip,	sizeof(reply.arp.sender_ip));

	//--- send the reply --------------------------------------------
	_arp_trace_msg(udpNo, &reply, "ARP REPLY");

	if (fpga_set_arp_buf(udpNo, (BYTE*)&reply, sizeof(SArpMsg))==REPLY_OK)
		ARP_ReplyCtr[udpNo]++;
}

#endif
