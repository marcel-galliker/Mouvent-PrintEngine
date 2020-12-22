/*
 * ethernet.c
 *
 * Created: 05/05/2020 10:26:41
 *  Author: Patrick
 */ 

#include <type_defs.h>
#include <stdlib.h>
#include <string.h>

#include "ethernet.h"

#include "eth_ipstack_main.h"
#include "ethernet_phy.h"
#include "ethernet_phy_main.h"

#include "driver_init.h"

#include "lwip/debug.h"
#include "lwip/stats.h"
#include "lwip/tcp.h"
#include "lwip/timers.h"

#include "comm.h"
#include "sa_tcp_ip.h"
#include "communication.h"
#include "rx_trace.h"
#include "error.h"


#define MAC_ADDRESS (0x11)
#define TCP_TIMEOUT	(5)
#define TCP_MESSAGE_MAX_SIZE	(256)

typedef struct {
	UINT32 tx_message_count;
	UINT32 rx_message_count;
	UINT32 timeout;
} STcpStatus;

static int	_BufIdx;
static BYTE	_Buf[TCP_MESSAGE_MAX_SIZE];

static bool	recv_flag = false;
static bool	link_up   = false;

static struct tcp_pcb*	_client;
static STcpStatus*		_clientTcpStatus;


void mac_receive_cb(struct mac_async_descriptor *desc);
static err_t connection_accept(void *arg, struct tcp_pcb *pcb, err_t err);
static void connection_close(struct tcp_pcb *pcb, STcpStatus *tcpStatus);
static err_t command_recv(void *arg, struct tcp_pcb *pcb, struct pbuf *p, err_t err);
static err_t command_poll(void *arg, struct tcp_pcb *tpcb);


void ethernet_init(void){
	struct tcp_pcb*  _tcpConnection;
	int32_t ret;
	
	TrPrintf(true, "ethernet_init");	
		
	comm_init(COMM_NUMBER_TCP);

	mac_async_register_callback(&ETHERNET_MAC_0, MAC_ASYNC_RECEIVE_CB, (FUNC_PTR)mac_receive_cb);
	
	eth_ipstack_init();
	do {
		ret = ethernet_phy_get_link_status(&ETHERNET_PHY_0_desc, &link_up);
		if(ret == ERR_NONE && link_up)
			break;
	} while(true);
	
	TCPIP_STACK_INTERFACE_0_init((uint8_t *)MAC_ADDRESS);	
	
	netif_set_up(&TCPIP_STACK_INTERFACE_0_desc);
	
	mac_async_enable(&ETHERNET_MAC_0);
	
	_tcpConnection = tcp_new();
	tcp_bind(_tcpConnection, IP_ADDR_ANY, SETUP_ASSIST_PORT);
	
	_tcpConnection = tcp_listen(_tcpConnection);
	
	if(_tcpConnection != NULL)
	{
		tcp_accept(_tcpConnection, connection_accept);
	}
}

void ethernet_tick()
{
	if(recv_flag) {
		recv_flag = false;
		ethernetif_mac_input(&TCPIP_STACK_INTERFACE_0_desc);
	}
	
	sys_check_timeouts();
}

INT32 ethernet_send(void* buffer, UINT32 length)
{
	err_t write_error = ERR_OK;
	
	if(_client == NULL)
	{
		return REPLY_ERROR;
	}
	
	write_error = tcp_write(_client, buffer, length, TCP_WRITE_FLAG_COPY);
	
	if(write_error != ERR_OK)
	{
		connection_close(_client, _clientTcpStatus);
		return REPLY_ERROR;
	}
	
	tcp_output(_client);
	_clientTcpStatus->tx_message_count++;


	return REPLY_OK;
}

void mac_receive_cb(struct mac_async_descriptor *desc)
{
	recv_flag = true;
}

static err_t connection_accept(void *arg, struct tcp_pcb *pcb, err_t err)
{
	STcpStatus *tcpStatus;
	
	LWIP_UNUSED_ARG(arg);
	LWIP_UNUSED_ARG(err);
	
	tcp_setprio(pcb, TCP_PRIO_MIN);
	
	tcpStatus = (STcpStatus *)mem_malloc(sizeof(STcpStatus));

	if (tcpStatus == NULL) {
		return ERR_MEM;
	}
	
	memset(tcpStatus, 0, sizeof(STcpStatus));
	
	tcp_arg(pcb, tcpStatus);
	tcp_recv(pcb, command_recv);
	tcp_poll(pcb, command_poll, 2);
	
	_client = pcb;
	_clientTcpStatus = tcpStatus;
	_BufIdx = 0;
	
	TrPrintf(true, "main connected");

	return ERR_OK;
}

static void connection_close(struct tcp_pcb *pcb, STcpStatus *tcpStatus)
{
	tcp_arg(pcb, NULL);
	tcp_recv(pcb, NULL);
	tcp_poll(pcb, NULL, 0);
	
	mem_free(tcpStatus);
	
	tcp_close(pcb);
	_client = NULL;
	_clientTcpStatus = NULL;
}

static err_t command_recv(void *arg, struct tcp_pcb *pcb, struct pbuf *p, err_t err)
{
	UINT32		*pmsgLen = (UINT32*)_Buf;
	STcpStatus	*tcpStatus;
	
	tcpStatus = (STcpStatus *)arg;
	
	tcpStatus->rx_message_count++;
	tcpStatus->timeout = 0;
	
	if(p == NULL)
	{
		TrPrintf(true, "command_recv: p==NULL");
		connection_close(pcb, tcpStatus);
		return ERR_OK;
	}

	if(p->next != NULL)
	{
		TrPrintf(true, "command_recv: p->next!=NULL");
		connection_close(pcb, tcpStatus);
		return ERR_OK;
		// TODO: Handle if there is more than just one buffer full of incoming data
	}
	
//	TrPrintf(true, "command_recv: %d Bytes", p->len);
	
	for(UINT32 i = 0; i < p->len; i++)
	{
		if (_BufIdx>=TCP_MESSAGE_MAX_SIZE)
		{
			//--- ERROR ---------------------------
			connection_close(pcb, tcpStatus);
			return ERR_OK;
		}
		_Buf[_BufIdx++] = ((BYTE*)p->payload)[i];
		if ((_BufIdx>=4) && (_BufIdx==*pmsgLen))
		{
			handle_command_message(_Buf);
			_BufIdx=0;
		}
	}		

	pbuf_free(p);
	tcp_recved(pcb, p->len);

	return ERR_OK;
}

static err_t command_poll(void *arg, struct tcp_pcb *tpcb)
{
	STcpStatus	*tcpStatus;
	
	tcpStatus = (STcpStatus *)arg;
	
	if(tcpStatus->timeout++ > TCP_TIMEOUT)
	{
		
		connection_close(tpcb, tcpStatus);
	}
	
	return ERR_OK;
}

