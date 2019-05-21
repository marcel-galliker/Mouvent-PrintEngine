// UDP_Test.cpp : Defines the entry point for the console application.
//

#ifdef WIN32
	#include "stdafx.h"
	#include <conio.h>
#else
	#include <stdio.h>
	#include <sys/socket.h>
	#include <sys/ioctl.h>
	#include <sys/time.h>

	#include <linux/if_packet.h>
	#include <linux/if_ether.h>
	#include <linux/if_arp.h>
#endif
#include "rx_common.h"
#include "rx_error.h"
#include "rx_sok.h"
#include "rx_threads.h"
#include "rx_trace.h"
#include "tcp_ip.h"

//--- COMPILER OPTIONS --------------------------------------------------

// #define RAW_SOCKETS 


//---------------------------------------------------------------------
#define HEAD_CNT		4
#define UDP_BLOCK_SIZE	3	// 0..3

char *ADDR_UDP[4] =
{
	"192.168.201.11",	
	"192.168.202.11",
	"192.168.203.11",
	"192.168.204.11"
};

#ifdef RAW_SOCKETS
	RX_RAW_SOCKET	dataSocket[4];
#else
	RX_SOCKET	dataSocket[4];
#endif


//--- udp_test_send_block ---------------------------------
int udp_test_send_block(int blkCnt, int ifCnt) // return = KB/Sec
{	
	int dataLen=RX_UdpBlockSize[UDP_BLOCK_SIZE];
	int head, blk, len, cnt, sent;
	int time;
	UINT32 dataBlkCnt;
	double speed;

	dataBlkCnt = 1024*1024*1024 / dataLen;
	dataBlkCnt = dataBlkCnt & ~127;  // align to 4*32-Bit BlockUsed-Flags
	dataBlkCnt /= 4;

	time = rx_get_ticks();

	SUDPDataBlockMsg msg;
	cnt=0;
	len = dataLen+4;
	memset(msg.blkData, 0x55, dataLen);

	/*
	for (blk=0; blk=100; blk++)
	{
		for (head=0; head<HEAD_CNT; head++)
		{		
			//--- alive ---
			msg.blkNo = 0xffffffff;
			sent = send(dataSocket[head], (char*)&msg, len, 0);			
		}
		rx_sleep(1000);
	}
	*/

	for (head=0; head<HEAD_CNT; head++)
	{
		for (blk=0; blk<blkCnt; blk++)
		{
			msg.blkNo = dataBlkCnt*head+blk;
			#ifdef RAW_SOCKETS
				UINT32 dstIpAddr;
				UINT64 dstMacAddr;
				macAddr_udp(0, 2, &dstMacAddr);
				dstIpAddr = sok_addr_32(ADDR_UDP[0]);
				sent = sok_send_raw_udp(dataSocket[0], dstMacAddr, dstIpAddr, PORT_UDP_DATA, &msg, len);
			#else
				sent = send(dataSocket[blk%ifCnt], (char*)&msg, len, 0);			
			#endif
			if (sent==len) cnt++;
		}
	}

	time = rx_get_ticks()-time;
	if (time>0) speed = cnt*dataLen*1000.0/time;
	else speed = 0;

	printf("Datablocks Sent: cnt=%d size=%d, time=%d ms, speed=%d KB/sec, speed=%d MB/sec\n\n", cnt, dataLen, time, (int)(speed/1024), (int)(speed/1024/1024));

	return (int)(speed/1024);
}


#ifdef linux
#define	ETH_ARP 0x0806

static UCHAR _src_mac[6];	/*our MAC address*/


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
	UINT32			sender_ip;
	unsigned char	target_mac[6];
	UINT32			target_ip;
};

typedef struct __attribute__((packed)) 
{
	struct ethhdr		eth;
	struct arp_header	arp;
} SArpMsg;
#endif

//--- main ---------------------------------------------------------------------
int main(int argc,  char* argv[])
{
	int error=0;
	int blkCnt, ifCnt, i, pos;
	int udpNo=0;
	int size;
	int speed;

	blkCnt=1;
	ifCnt =1;
	for (i=1; i<argc; i++)
	{
		if (pos=str_start(argv[i], "cnt="))
		{
			blkCnt = atoi(&argv[i][pos]);
		}
		if (pos=str_start(argv[i], "if="))
		{
			ifCnt = atoi(&argv[i][pos]);
		}
	}

	#ifdef linux
	if (FALSE)
	{
		char *senderIp = "192.168.201.201";
		char *targetIp = "192.168.201.102";
		char *device   = "p2p1"; 
		RX_RAW_SOCKET socket;
		SArpMsg msg;
		UINT64 mac;

		sok_open_raw(&socket, device, senderIp, 7777);
		sok_get_mac_address(device, &mac);

		memset(&msg, 0, sizeof(msg));
		memset(msg.eth.h_dest, 0xff, sizeof(msg.eth.h_dest));	// broadcast
		memcpy(msg.eth.h_source, &mac, sizeof(msg.eth.h_source));
		msg.eth.h_proto = htons(ETH_ARP);

		//---  msg.arp --------------------------
		msg.arp.hw_type = htons(1);
		msg.arp.opcode  = htons(ARP_REPLY);
		msg.arp.hw_size = 6;
		msg.arp.proto_size = 4;
		msg.arp.opcode = ARP_REQUEST;
		memset(msg.arp.target_mac, 0xff, sizeof(msg.arp.target_mac));	// broadcast
		msg.arp.target_ip = sok_addr_32(targetIp);
		memcpy(msg.arp.sender_mac, &mac,	sizeof(msg.arp.sender_mac));
		msg.arp.sender_ip = sok_addr_32(senderIp);

		sok_send_arp(socket, (UINT64)msg.eth.h_dest, &msg, sizeof(msg));

		sok_close_raw(&socket);
	}
	#endif

	err_init(0, 0);

	/*
	//--- ping the network -------------------
	if (TRUE)
	{
		sok_ping("192.168.200.101");
		for (i=0; i<ifCnt; i++)
		{
			sok_ping(ADDR_UDP[i]);
		}
		printf("Restart rx_head_ctrl now\n");
		printf("<ENTER> to procede.");
		getchar();
	}
	*/
			
	#ifdef linux
	{
		//	-> TESTED: Changing the process priority does not improve network performance!

		/*
		char cmd[100];

		int pid = getpid();
		sprintf(cmd, "renice -p %d -20", getpid());
		printf("enter command >>%s<<\n", cmd);
		printf("<ENTER> to procede.");
		getchar();
		*/
	}
	#else
		SetPriorityClass(GetCurrentProcess(), REALTIME_PRIORITY_CLASS);
		SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL);
	#endif

	printf("UDP-Test send %d blocks, blockSize=%d, interfaces=%d\n", blkCnt, RX_UdpBlockSize[UDP_BLOCK_SIZE], ifCnt);

	for (i=0; i<ifCnt; i++)
	{
		#ifdef RAW_SOCKETS
			printf("Connect udp[0]: %s err=%d\n", ADDR_UDP_0, error|=sok_open_raw(&dataSocket[0], "p2p1", "192.168.200.201", PORT_UDP_DATA));
		#else
			printf("Connect udp[%d]: %s err=%d\n", i, ADDR_UDP[i], error|=sok_open_client(&dataSocket[i], ADDR_UDP[i], PORT_UDP_DATA,  SOCK_DGRAM));
		#endif
	}
	rx_sleep(1000);	
		
	if (error) blkCnt=0;
	else speed = udp_test_send_block(blkCnt, ifCnt);
	
	rx_sleep(1000);	
	for (i=0; i<SIZEOF(dataSocket); i++)
	{
		#ifdef RAW_SOCKETS
			sok_close_raw(&dataSocket[i]);
		#else
			sok_close(&dataSocket[i]);
		#endif
	}

	size = blkCnt*HEAD_CNT*RX_UdpBlockSize[UDP_BLOCK_SIZE];
	printf("Data: %d MB\n", size/1024/1024);
	printf("Speed: %.1f MB/sec\n", speed/1024.0);

	printf("END --- PRESS ANY KEY\n");
//	_getch();
	return 0;
}