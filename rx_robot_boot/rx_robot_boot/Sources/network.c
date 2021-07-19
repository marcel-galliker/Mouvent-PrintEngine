
//--- includes ethernet -----------------------------------
#include "net.h"

//--- includes application ----------------------------
#include "rx_types.h"
#include "rx_robot_tcpip.h"
#include "rx_boot.h"
#include "robot_flash.h"
#include "bootloader.h"
#include "network.h"


//--- defines ---------------------------------------------------

#define USE_DHCP            	0           // Disable DHCP
#define IP_ADDR(a,b,c,d)		PP_HTONL(LWIP_MAKEU32((a), (b), (c), (d)))

#define BROADCAST_ADDRESS       IP_ADDR(192, 168, 200, 255)
#define WILDCARD_IP_ADDRESS		IP_ADDR(192, 168, 200, 50)

// static ip_addr_t _wildcardIpAddress = 	{WILDCARD_IP_ADDRESS};
static ip_addr_t _ipAddress 	= 	{IP_ADDR(192,168,200,50)};
static ip_addr_t _ipAddress_new = 	{IP_ADDR(0,0,0,0)};
static ip_addr_t _ctrl_addr 	= 	{IP_ADDR(0,0,0,0)};
static UINT16	 _ctrl_port		= 0;
struct udp_pcb *_rxBootPcb;
struct udp_pcb *_rxCtrlPcb;


//--- prototypes -------------------------------------------
static void _net_is_up(void);
static void _handle_boot_msg(void *arg, struct udp_pcb *pcb, struct pbuf *p, const ip_addr_t *addr, u16_t port);
static void _handle_ctrl_msg(void *arg, struct udp_pcb *pcb, struct pbuf *p, const ip_addr_t *addr, u16_t port);

//--- network_init --------------------------------------------
bool network_init(void)
{
	sys_enable(sys_device_ethernet);

	ip_addr_t gatewayAddress = 	{IP_ADDR(192, 168, 200, 1)};
	ip_addr_t netMask = 		{IP_ADDR(255, 255, 255, 0)};
	flash_read_ipAddr(&_ipAddress);
	net_init(_ipAddress, gatewayAddress, netMask, USE_DHCP, "Robot", NULL);
	return REPLY_OK;
}

//--- network_tick ---------------------------------------------
void network_tick(int tick)
{
	static bool _ethernet_is_up=false;
	static bool _link_is_up=false;
	net_tick();
	bool up=ethernet_is_link_up();
	if (up && !_ethernet_is_up)
	{
		 net_set_link_up();
	}
	if (!up && _ethernet_is_up)
	{
		printf("link went down\n");
	}
	_ethernet_is_up = up;
	up = net_is_link_up();
	if (up && !_link_is_up)
	{
		_net_is_up();
	}
	if (!up && _link_is_up)
	{
		//--- changing IP-Address ---------------------------------------------
		struct netif* my_netif=net_get_netif();

		udp_netif_ip_addr_changed(&_ipAddress, &_ipAddress_new);
		memcpy(&_ipAddress, &_ipAddress_new, sizeof(_ipAddress));

		memcpy(&my_netif->ip_addr, &_ipAddress_new, sizeof(my_netif->ip_addr));
		udp_netif_ip_addr_changed(&_ipAddress, &_ipAddress_new);

		net_set_link_up();
	}
	_link_is_up = up;
}

//--- _net_is_up ---------------------------------
static void _net_is_up(void)
{
	if (_rxBootPcb==NULL)
	{
		_rxBootPcb = udp_new();
		udp_bind(_rxBootPcb, IP_ADDR_ANY, PORT_UDP_BOOT_CLNT);
		udp_recv(_rxBootPcb, _handle_boot_msg, NULL);
	}

	if (_rxCtrlPcb==NULL) _rxCtrlPcb = udp_new();
	udp_bind(_rxCtrlPcb, &_ipAddress, PORT_UDP_CTRL);
	udp_recv(_rxCtrlPcb, _handle_ctrl_msg, NULL);
}

//--- _handle_boot_msg -------------------------------------
static void _handle_boot_msg(void *arg, struct udp_pcb *pcb, struct pbuf *p, const ip_addr_t *addr, u16_t port)
{
	LWIP_UNUSED_ARG(arg);
	if(p)
	{
		rx_boot_handle_msg(p->payload);
		pbuf_free(p);
	}
}

//--- network_send_boot_msg ----------------------------------------
void network_send_boot_msg(void *message, uint32_t size)
{
	struct pbuf *p = pbuf_alloc(PBUF_RAW, size, PBUF_RAM);
	if (p)
	{
		ip_addr_t addr={BROADCAST_ADDRESS};
		memcpy(p->payload, message, size);
		err_t err=udp_sendto(_rxBootPcb, p, &addr, PORT_UDP_BOOT_SVR);
		if (err)
			printf("err=%d\n", err);
		pbuf_free(p);
	}
}

//--- _handle_ctrl_msg -------------------------------------
static void _handle_ctrl_msg(void *arg, struct udp_pcb *pcb, struct pbuf *p, const ip_addr_t *addr, u16_t port)
{
	LWIP_UNUSED_ARG(arg);
	if(p)
	{
		memcpy(&_ctrl_addr, addr, sizeof(_ctrl_addr));
		_ctrl_port = port;
		bootloader_handle_msg(p->payload);
		pbuf_free(p);
	}
}

//--- network_send_ctrl_msg ---------------------------------------------
void network_send_ctrl_msg(void* message, uint32_t size)
{
	struct pbuf *p = pbuf_alloc(PBUF_RAW, size, PBUF_RAM);
	if (p)
	{
		memcpy(p->payload, message, size);
		err_t err=udp_sendto(_rxCtrlPcb, p, &_ctrl_addr, _ctrl_port);
		if (err) printf("err=%d\n", err);
		pbuf_free(p);
	}
}

//--- network_end ---------------------------------------------
void network_end(void)
{

}

//--- network_change_ip ----------------------------------------
void network_change_ip(ip_addr_t* newIpAddress)
{
	if (newIpAddress->addr != _ipAddress.addr)
	{
		flash_write_ipAddr(newIpAddress);

		memcpy(&_ipAddress_new, newIpAddress, sizeof(_ipAddress_new));
		net_set_link_down();
		// wait net_is_link_up()==FALSE
	}
}
