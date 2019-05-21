// ****************************************************************************
//
//	dpch_server.c
//
//	based on Open Source
//
// ****************************************************************************
//
//	Copyright 2013 by Radex AG, Switzerland. All rights reserved.
//	Written by Marcel Galliker
//
// ****************************************************************************



#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <arpa/inet.h>

#include "rx_error.h"
#include "rx_trace.h"
#include "tcp_ip.h"
#include "dhcp_server.h"
#include "dhcp_server_def.h"
#include "dhcp_ip_addr.h"

#define CONFIG_BUFFER_SIZE	1024

struct server_config gobal_config = {0};

struct dhcp_packet_handler gobal_packet_handler = 
{
	.do_discover	= &do_discover,
	.do_inform		= &do_inform,
	.do_request		= &do_request,
	.do_release		= &do_release,
	.do_decline		= &do_decline,
};


//--- dhcp_start -----------------------
int dhcp_start(void)
{
	TrPrintfL(DHCP_TRACE>1, "DHCP: dhcp_start");
	strcpy(gobal_config.server, RX_CTRL_MAIN);
	
	dhcp_addr_init(PATH_USER "gui.xml");
	
	gobal_config.port  = 67;
	gobal_config.lease = 86400;  
	gobal_config.renew = 68400;
	
	TrPrintfL(DHCP_TRACE, "DHCP: -------DUMP GOBAL_CONFIG----------");
	TrPrintfL(DHCP_TRACE, "DHCP: gobal_config.server=%s", gobal_config.server);
	TrPrintfL(DHCP_TRACE, "DHCP: gobal_config.port=%d", gobal_config.port);
	TrPrintfL(DHCP_TRACE, "DHCP: gobal_config.lease=%d", gobal_config.lease);
	TrPrintfL(DHCP_TRACE, "DHCP: gobal_config.renew=%d", gobal_config.renew);
	TrPrintfL(DHCP_TRACE, "DHCP: -----------------END--------------");
	
	int dhcp_socket = 0;
	int so_reuseaddr = 1;
	struct sockaddr_in server_address;

	if((dhcp_socket = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
	{
		Error(ERR_ABORT, 0, "***Cannot open the socket! %s(%d)***", strerror(errno), errno);
		goto ERROR;
	}

	setsockopt(dhcp_socket, SOL_SOCKET, SO_REUSEADDR, &so_reuseaddr, sizeof(so_reuseaddr));
	
	memset(&server_address, 0, sizeof(server_address));
	server_address.sin_family = AF_INET;
	server_address.sin_port = htons(gobal_config.port);
	server_address.sin_addr.s_addr = htonl(INADDR_ANY);

	if(bind(dhcp_socket, (struct sockaddr*)&server_address, sizeof(server_address)) < 0)
	{
		Error(ERR_ABORT, 0, "***Cannot bind the socket with the address! %s(%d)***", strerror(errno), errno);
		goto ERROR;
	}
	
	socklen_t addr_len = sizeof(struct sockaddr_in);
	while(1)
	{
		struct raw_msg *msg = (struct raw_msg*)malloc(sizeof(struct raw_msg));
		if(NULL == msg)
		{
			Error(ERR_ABORT, 0, "***Allocate memory failed! %s(%d)***", strerror(errno), errno);
			continue;
		}
		memset(msg, 0, sizeof(struct raw_msg));
		msg->socket_fd = dhcp_socket;
		msg->length = recvfrom(dhcp_socket, msg->buff, DHCP_MAX_MTU, 0, (struct sockaddr*)&msg->address, &addr_len);
		TrPrintfL(DHCP_TRACE, "DHCP: %d bytes received", msg->length);
		if(msg->length < 0)
		{
			Error(WARN, 0, "***Receive data error! %s(%d)***", strerror(errno), errno);
			free(msg);
			continue;
		}
		pthread_t thread_id;
		pthread_create(&thread_id, NULL, &handle_msg, (void *)msg);
	}
ERROR:
	if(0 != dhcp_socket)
	{
		close(dhcp_socket);
	}
	Error(WARN, 0, "***error!*** dhcp_marshall==>");
	return -1;
}

void *handle_msg(void *arg)
{
	TrPrintfL(DHCP_TRACE>1, "DHCP: ==>handle_msg, arg=%d", arg);
	struct raw_msg *msg = (struct raw_msg*)arg;
	struct dhcp_packet *request = dhcp_marshall(msg->buff, 0, msg->length);
	
	if(NULL != request)
	{
		struct dhcp_packet *response = dispatch(request);
		
		if(NULL != response)
		{
			int broadcast_socket = 0;
    		int so_broadcast = 1;
			int so_reuseaddr = 1;
    		struct sockaddr_in server_address;

    		if((broadcast_socket = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    		{
        		Error(ERR_ABORT, 0, "***Cannot open the socket! %s(%d)***", strerror(errno), errno);
    			goto ERROR;		
    		}

    		setsockopt(broadcast_socket, SOL_SOCKET, SO_BROADCAST, &so_broadcast, sizeof(so_broadcast));
			setsockopt(broadcast_socket, SOL_SOCKET, SO_REUSEADDR, &so_reuseaddr, sizeof(so_reuseaddr));	
		
    		memset(&server_address, 0, sizeof(server_address));
    		server_address.sin_family = AF_INET;
    		server_address.sin_port = htons(gobal_config.port);
    		server_address.sin_addr.s_addr = inet_addr(gobal_config.server);

    		if(bind(broadcast_socket, (struct sockaddr*)&server_address, sizeof(server_address)) < 0)
    		{
        		Error(ERR_ABORT, 0, "***Cannot bind the socket with the address! %s(%d)***", strerror(errno), errno);
        		goto ERROR;
    		}

			char buffer[DHCP_MAX_MTU];
			int length = dhcp_serialize(response, buffer, DHCP_MAX_MTU);
			
			struct sockaddr_in broadcast = {0};
			broadcast.sin_family = AF_INET;
			broadcast.sin_port = htons(BOOTP_REPLAY_PORT);
			broadcast.sin_addr.s_addr = htonl(INADDR_BROADCAST);

			int send_length = sendto(broadcast_socket, buffer, length, 0, (struct sockaddr*)&broadcast, sizeof(broadcast));
			if(send_length < 0)
			{
				Error(WARN, 0, "***Send data error! %s(%d)***", strerror(errno), errno);
			}
			else
			{
				TrPrintfL(DHCP_TRACE, "DHCP: Total %d bytes send!", send_length);
			}
ERROR:
			if(0 != broadcast_socket)
			{
				close(broadcast_socket);
			}
			dhcp_free_packet(response);
		}
		else
		{
			Error(WARN, 0, "Response packet is NULL.");
		}
		
		dhcp_free_packet(request);
	}
	else
	{
		Error(WARN, 0, "Can not dhcp_marshall request packet from raw bytes.");
	}
	free(msg);
	
	TrPrintfL(DHCP_TRACE>1, "DHCP: handle_msg==>");
	return NULL;
}

struct dhcp_packet *dispatch(struct dhcp_packet *request)
{	
	TrPrintfL(DHCP_TRACE>1, "DHCP: dispatch");
	if(NULL == request)
	{
		Error(ERR_CONT, 0, "***Request packet is NULL***");
		goto ERROR;
	}

	if(BOOT_REQUEST != request->op)
	{
		Error(WARN, 0, "***Packet is not send from dhcp client, ignor!***");
		goto ERROR;
	}

	//get the dhcp packet type
	char type = '\0';
	struct dhcp_option *option = request->options;
	while(NULL != option)
	{
		if(DHO_DHCP_MESSAGE_TYPE == option->code)
		{
			type = *option->value;
			break;
		}
		
		option = option->next;
	}

	if('\0' == type)
	{
		Error(ERR_CONT, 0, "***No dhcp message type found in the packet!***");
		goto ERROR;
	}
	TrPrintfL(DHCP_TRACE, "DHCP: packet type=%d", type);
	struct dhcp_packet *response = NULL;
	switch(type)
	{
		case DHCP_DISCOVER:	response = gobal_packet_handler.do_discover(request);	break;
		case DHCP_RELEASE:	response = gobal_packet_handler.do_release(request);	break;
		case DHCP_INFORM:	response = gobal_packet_handler.do_inform(request);		break;
		case DHCP_REQUEST:	response = gobal_packet_handler.do_request(request);	break;
		case DHCP_DECLINE:	response = gobal_packet_handler.do_decline(request);	break;
		default:																	break;
	}
	
	TrPrintfL(DHCP_TRACE>1, "DHCP: dispatch==>");
	return response;
ERROR:
	TrPrintfL(DHCP_TRACE>1, "DHCP: ***Error***, dispatch==>");
	return NULL;
}

struct dhcp_packet *do_discover(struct dhcp_packet *request)
{
	TrPrintfL(DHCP_TRACE>1, "DHCP: do_discover");
	struct network_config config = {0};
	memcpy(config.hardware_address, request->chaddr, 16);
	
	if(dhcp_addr_get_cfg(&config) < 0)
	{
		Error(WARN, 0, "Cannot assign IP address! do_discover==>");
		return NULL;
	}

	struct dhcp_packet *response = (struct dhcp_packet*)malloc(sizeof(struct dhcp_packet));
	if(NULL == response)
	{
		Error(ERR_ABORT, 0, "***Allocate memory failed! %s(%d)*** do_discover==>", strerror(errno), errno);
		return NULL;
	}
	memset(response, 0, sizeof(struct dhcp_packet));

	response->op = BOOT_REPLY;
	response->htype = request->htype;
	response->hlen = request->hlen;
	response->hops = 1;
	response->xid	 = request->xid;
	response->yiaddr = config.ip_address;
	response->flags  = request->flags;
	memcpy(response->chaddr, request->chaddr, 16);
	
	//options
	//message type
	struct dhcp_option *packet_type = (struct dhcp_option*)malloc(sizeof(struct dhcp_option));
	if(NULL == packet_type)
	{
		dhcp_free_packet(response);
		Error(ERR_ABORT, 0, "***Allocate memory failed! %s(%d)*** do_discover==>", strerror(errno), errno);
		return NULL;
	}
	memset(packet_type, 0, sizeof(struct dhcp_option));
	packet_type->code = DHO_DHCP_MESSAGE_TYPE;
	packet_type->value = (char *)malloc(1);
	if(NULL == packet_type->value)
	{
		dhcp_free_packet(response);
		Error(ERR_ABORT, 0, "***Allocate memory failed! %s(%d)*** do_discover==>", strerror(errno), errno);
		return NULL;
	}
	*packet_type->value = DHCP_OFFER;
	packet_type->length = 1;
	response->options = packet_type;

	//server identifier
	struct dhcp_option *server_identifier = (struct dhcp_option*)malloc(sizeof(struct dhcp_option));
	if(NULL == server_identifier)
	{
		dhcp_free_packet(response);
		Error(ERR_ABORT, 0, "***Allocate memory failed! %s(%d)*** do_discover==>", strerror(errno), errno);
		return NULL;
	}
    memset(server_identifier, 0, sizeof(struct dhcp_option));
    server_identifier->code = DHO_DHCP_SERVER_IDENTIFIER;
    server_identifier->value = (char *)malloc(4);
	if(NULL == server_identifier->value)
	{
		dhcp_free_packet(response);
		Error(ERR_ABORT, 0, "***Allocate memory failed! %s(%d)*** do_discover==>", strerror(errno), errno);
		return NULL;
	}
	UINT32 addr = sok_addr_32(gobal_config.server);
    server_identifier->length = 4;
	memcpy(server_identifier->value, &addr, server_identifier->length);
    packet_type->next = server_identifier;

	//lease time
	struct dhcp_option *lease_time = (struct dhcp_option*)malloc(sizeof(struct dhcp_option));
	if(NULL == lease_time)
	{
		dhcp_free_packet(response);
		Error(ERR_ABORT, 0, "***Allocate memory failed! %s(%d)*** do_discover==>", strerror(errno), errno);
		return NULL;
	}
    memset(lease_time, 0, sizeof(struct dhcp_option));
    lease_time->code = DHO_DHCP_LEASE_TIME;
    lease_time->value = (char *)malloc(4);
	if(NULL == lease_time->value)
	{
		dhcp_free_packet(response);
		Error(ERR_ABORT, 0, "***Allocate memory failed! %s(%d)*** do_discover==>", strerror(errno), errno);
		return NULL;
	}
    memcpy(lease_time->value, &gobal_config.lease, 4);
    lease_time->length = 4;
    server_identifier->next = lease_time;

	//renew time
	struct dhcp_option *renew_time = (struct dhcp_option*)malloc(sizeof(struct dhcp_option));
	if(NULL == renew_time)
	{
		dhcp_free_packet(response);
		Error(ERR_ABORT, 0, "***Allocate memory failed! %s(%d)*** do_discover==>", strerror(errno), errno);
		return NULL;
	}
    memset(renew_time, 0, sizeof(struct dhcp_option));
    renew_time->code = DHO_DHCP_RENEWAL_TIME;
    renew_time->value = (char *)malloc(4);
	if(NULL == renew_time->value)
	{
		dhcp_free_packet(response);
		Error(ERR_ABORT, 0, "***Allocate memory failed! %s(%d)*** do_discover==>", strerror(errno), errno);
		return NULL;
	}
    memcpy(renew_time->value, &gobal_config.renew, 4);
    renew_time->length = 4;
    lease_time->next = renew_time;

	//router/gateway
	struct dhcp_option *router = (struct dhcp_option*)malloc(sizeof(struct dhcp_option));
	if(NULL == router)
	{
		dhcp_free_packet(response);
		Error(ERR_ABORT, 0, "***Allocate memory failed! %s(%d)*** do_discover==>", strerror(errno), errno);
		return NULL;
	}
    memset(router, 0, sizeof(struct dhcp_option));
    router->code = DHO_ROUTERS;
    router->value = (char *)malloc(4);
	if(NULL == router->value)
	{
		dhcp_free_packet(response);
		Error(ERR_ABORT, 0, "***Allocate memory failed! %s(%d)*** do_discover==>", strerror(errno), errno);
		return NULL;
	}
    router->length = 4;
    memcpy(router->value, &config.router, router->length);
    renew_time->next = router;

	//netmask
	struct dhcp_option *subnet_mask = (struct dhcp_option*)malloc(sizeof(struct dhcp_option));
	if(NULL == subnet_mask)
	{
		dhcp_free_packet(response);
		Error(ERR_ABORT, 0, "***Allocate memory failed! %s(%d)*** do_discover==>", strerror(errno), errno);
		return NULL;
	}
    memset(subnet_mask, 0, sizeof(struct dhcp_option));
    subnet_mask->code = DHO_SUBNET;
    subnet_mask->value = (char *)malloc(4);
	if(NULL == subnet_mask->value)
	{
		dhcp_free_packet(response);
		Error(ERR_ABORT, 0, "***Allocate memory failed! %s(%d)*** do_discover==>", strerror(errno), errno);
		return NULL;
	}
    subnet_mask->length = 4;
    memcpy(subnet_mask->value, &config.netmask, subnet_mask->length);
    router->next = subnet_mask;

	//dns
	struct dhcp_option *dns_server = (struct dhcp_option*)malloc(sizeof(struct dhcp_option));
	if(NULL == dns_server)
	{
		dhcp_free_packet(response);
		Error(ERR_ABORT, 0, "***Allocate memory failed! %s(%d)*** do_discover==>", strerror(errno), errno);
		return NULL;
	}
    memset(dns_server, 0, sizeof(struct dhcp_option));
    dns_server->code = DHO_DOMAIN_NAME_SERVERS;
    dns_server->value = (char *)malloc(8);
	if(NULL == dns_server->value)
	{
		dhcp_free_packet(response);
		Error(ERR_ABORT, 0, "***Allocate memory failed! %s(%d)*** do_discover==>", strerror(errno), errno);
		return NULL;
	}
    memcpy(dns_server->value, &config.dns1, 4);
	memcpy(dns_server->value + 4, &config.dns2, 4);
    dns_server->length = 8;
    subnet_mask->next = dns_server;
	
	TrPrintfL(DHCP_TRACE>1, "DHCP: do_discover==>");
	return response;
}

struct dhcp_packet *do_request(struct dhcp_packet *request)
{
	TrPrintfL(DHCP_TRACE>1, "DHCP: do_request");
	struct network_config config = {0};
	memcpy(config.hardware_address, request->chaddr, 16);
	
	if(dhcp_addr_get_cfg(&config) < 0)
	{
		Error(WARN, 0, "Cannot assign IP address! do_request==>");
		return NULL;
	}
	
	char type = DHCP_ACK;
	UINT32 requested_address=0;
	
	if(requested_address!=0)
	{
		TrPrintfL(DHCP_TRACE>1, "DHCP: request->ciaddr is not 0, copy it to request_address");
		requested_address = request->ciaddr;
	}
	else
	{
		TrPrintfL(DHCP_TRACE>1, "DHCP: request->ciaddr is 0, get request_address from dhcp option");
		struct dhcp_option *option = request->options;
		while(NULL != option)
		{
			if(DHO_DHCP_REQUESTED_ADDRESS == option->code && option->length >= 4)
			{
				memcpy(&requested_address, option->value, 4);
				break;
			}
			
			option = option->next;
		}
	}
	
	if(config.ip_address != requested_address)
	{
		char str1[32], str2[32];
		Error(WARN, 0, "request_address >>%s<< is not the same as IP assigned >>%s<<, change packet type to NAK", sok_addr_str(requested_address, str1), sok_addr_str(config.ip_address, str2));
		type = DHCP_NAK;
	}
	
	struct dhcp_packet *response = (struct dhcp_packet*)malloc(sizeof(struct dhcp_packet));
	memset(response, 0, sizeof(struct dhcp_packet));

	response->op		= BOOT_REPLY;
	response->htype		= request->htype;
	response->hlen		= request->hlen;
	response->hops		= 1;
	response->xid		= request->xid;
	response->yiaddr	= requested_address;
	response->flags		= request->flags;
	memcpy(response->chaddr, request->chaddr, 16);
	
	if(DHCP_ACK == type)
	{
		//options
		//message type
		struct dhcp_option *packet_type = (struct dhcp_option*)malloc(sizeof(struct dhcp_option));
		if(NULL == packet_type)
		{
			dhcp_free_packet(response);
			Error(ERR_ABORT, 0, "***Allocate memory failed! %s(%d)*** do_discover==>", strerror(errno), errno);
			return NULL;
		}
		memset(packet_type, 0, sizeof(struct dhcp_option));
		packet_type->code = DHO_DHCP_MESSAGE_TYPE;
		packet_type->value = (char *)malloc(1);
		if(NULL == packet_type->value)
		{
			dhcp_free_packet(response);
			Error(ERR_ABORT, 0, "***Allocate memory failed! %s(%d)*** do_discover==>", strerror(errno), errno);
			return NULL;
		}
		*packet_type->value = type;
		packet_type->length = 1;
		response->options = packet_type;

		//server identifier
		struct dhcp_option *server_identifier = (struct dhcp_option*)malloc(sizeof(struct dhcp_option));
		if(NULL == server_identifier)
		{
			dhcp_free_packet(response);
			Error(ERR_ABORT, 0, "***Allocate memory failed! %s(%d)*** do_discover==>", strerror(errno), errno);
			return NULL;
		}
		memset(server_identifier, 0, sizeof(struct dhcp_option));
		server_identifier->code = DHO_DHCP_SERVER_IDENTIFIER;
		server_identifier->value = (char *)malloc(4);
		if(NULL == server_identifier->value)
		{
			dhcp_free_packet(response);
			Error(ERR_ABORT, 0, "***Allocate memory failed! %s(%d)*** do_discover==>", strerror(errno), errno);
			return NULL;
		}
		UINT32 addr = sok_addr_32(gobal_config.server);
		server_identifier->length = 4;
		memcpy(server_identifier->value, &addr, server_identifier->length);
		packet_type->next = server_identifier;

		//lease time
		struct dhcp_option *lease_time = (struct dhcp_option*)malloc(sizeof(struct dhcp_option));
		if(NULL == lease_time)
		{
			dhcp_free_packet(response);
			Error(ERR_ABORT, 0, "***Allocate memory failed! %s(%d)*** do_discover==>", strerror(errno), errno);
			return NULL;
		}
		memset(lease_time, 0, sizeof(struct dhcp_option));
		lease_time->code = DHO_DHCP_LEASE_TIME;
		lease_time->value = (char *)malloc(4);
		if(NULL == lease_time->value)
		{
			dhcp_free_packet(response);
			Error(ERR_ABORT, 0, "***Allocate memory failed! %s(%d)*** do_discover==>", strerror(errno), errno);
			return NULL;
		}
		memcpy(lease_time->value, &gobal_config.lease, 4);
		lease_time->length = 4;
		server_identifier->next = lease_time;

		//renew time
		struct dhcp_option *renew_time = (struct dhcp_option*)malloc(sizeof(struct dhcp_option));
		if(NULL == renew_time)
		{
			dhcp_free_packet(response);
			Error(ERR_ABORT, 0, "***Allocate memory failed! %s(%d)*** do_discover==>", strerror(errno), errno);
			return NULL;
		}
		memset(renew_time, 0, sizeof(struct dhcp_option));
		renew_time->code = DHO_DHCP_RENEWAL_TIME;
		renew_time->value = (char *)malloc(4);
		if(NULL == renew_time->value)
		{
			dhcp_free_packet(response);
			Error(ERR_ABORT, 0, "***Allocate memory failed! %s(%d)*** do_discover==>", strerror(errno), errno);
			return NULL;
		}
		memcpy(renew_time->value, &gobal_config.renew, 4);
		renew_time->length = 4;
		lease_time->next = renew_time;

		//router/gateway
		struct dhcp_option *router = (struct dhcp_option*)malloc(sizeof(struct dhcp_option));
		if(NULL == router)
		{
			dhcp_free_packet(response);
			Error(ERR_ABORT, 0, "***Allocate memory failed! %s(%d)*** do_discover==>", strerror(errno), errno);
			return NULL;
		}
		memset(router, 0, sizeof(struct dhcp_option));
		router->code = DHO_ROUTERS;
		router->value = (char *)malloc(4);
		if(NULL == router->value)
		{
			dhcp_free_packet(response);
			Error(ERR_ABORT, 0, "***Allocate memory failed! %s(%d)*** do_discover==>", strerror(errno), errno);
			return NULL;
		}
		memcpy(router->value, &config.router, 4);
		router->length = 4;
		renew_time->next = router;

		//netmask
		struct dhcp_option *subnet_mask = (struct dhcp_option*)malloc(sizeof(struct dhcp_option));
		if(NULL == subnet_mask)
		{
			dhcp_free_packet(response);
			Error(ERR_ABORT, 0, "***Allocate memory failed! %s(%d)*** do_discover==>", strerror(errno), errno);
			return NULL;
		}
		memset(subnet_mask, 0, sizeof(struct dhcp_option));
		subnet_mask->code = DHO_SUBNET;
		subnet_mask->value = (char *)malloc(4);
		if(NULL == subnet_mask->value)
		{
			dhcp_free_packet(response);
			Error(ERR_ABORT, 0, "***Allocate memory failed! %s(%d)*** do_discover==>", strerror(errno), errno);
			return NULL;
		}
		memcpy(subnet_mask->value, &config.netmask, 4);
		subnet_mask->length = 4;
		router->next = subnet_mask;

		//dns
		struct dhcp_option *dns_server = (struct dhcp_option*)malloc(sizeof(struct dhcp_option));
		if(NULL == dns_server)
		{
			dhcp_free_packet(response);
			Error(ERR_ABORT, 0, "***Allocate memory failed! %s(%d)*** do_discover==>", strerror(errno), errno);
			return NULL;
		}
		memset(dns_server, 0, sizeof(struct dhcp_option));
		dns_server->code = DHO_DOMAIN_NAME_SERVERS;
		dns_server->value = (char *)malloc(8);
		if(NULL == dns_server->value)
		{
			dhcp_free_packet(response);
			Error(ERR_ABORT, 0, "***Allocate memory failed! %s(%d)*** do_discover==>", strerror(errno), errno);
			return NULL;
		}
		memcpy(dns_server->value, &config.dns1, 4);
		memcpy(dns_server->value + 4, &config.dns2, 4);
		dns_server->length = 8;
		subnet_mask->next = dns_server;
	}
	
	TrPrintfL(DHCP_TRACE>1, "DHCP: do_request==>");
	return response;
}

struct dhcp_packet *do_release(struct dhcp_packet *request)
{
	TrPrintfL(DHCP_TRACE>1, "DHCP: do_release");
	return NULL;
	TrPrintfL(DHCP_TRACE>1, "DHCP: do_release==>");
}

struct dhcp_packet *do_inform(struct dhcp_packet *request)
{
	TrPrintfL(DHCP_TRACE>1, "DHCP: do_inform");
    struct network_config config = {0};
	memcpy(config.hardware_address, request->chaddr, 16);
	
	if(dhcp_addr_get_cfg(&config) < 0)
	{
		Error(WARN, 0, "Cannot assign IP address! do_inform==>");
		return NULL;
	}

	struct dhcp_packet *response = (struct dhcp_packet*)malloc(sizeof(struct dhcp_packet));
	memset(response, 0, sizeof(struct dhcp_packet));

	response->op	= BOOT_REPLY;
	response->htype = request->htype;
	response->hlen	= request->hlen;
	response->hops	= 1;
	response->xid	= request->xid;
	response->yiaddr= config.ip_address;
	response->flags	= request->flags;
	memcpy(response->chaddr, request->chaddr, 16);
	
	//options
	//message type
	struct dhcp_option *packet_type = (struct dhcp_option*)malloc(sizeof(struct dhcp_option));
	if(NULL == packet_type)
	{
		dhcp_free_packet(response);
		Error(ERR_ABORT, 0, "***Allocate memory failed! %s(%d)*** do_discover==>", strerror(errno), errno);
		return NULL;
	}
	memset(packet_type, 0, sizeof(struct dhcp_option));
	packet_type->code = DHO_DHCP_MESSAGE_TYPE;
	packet_type->value = (char *)malloc(1);
	if(NULL == packet_type->value)
	{
		dhcp_free_packet(response);
		Error(ERR_ABORT, 0, "***Allocate memory failed! %s(%d)*** do_discover==>", strerror(errno), errno);
		return NULL;
	}
	*packet_type->value = DHCP_ACK;
	packet_type->length = 1;
	response->options = packet_type;

	//server identifier
	struct dhcp_option *server_identifier = (struct dhcp_option*)malloc(sizeof(struct dhcp_option));
	if(NULL == server_identifier)
	{
		dhcp_free_packet(response);
		Error(ERR_ABORT, 0, "***Allocate memory failed! %s(%d)*** do_discover==>", strerror(errno), errno);
		return NULL;
	}
    memset(server_identifier, 0, sizeof(struct dhcp_option));
    server_identifier->code = DHO_DHCP_SERVER_IDENTIFIER;
    server_identifier->value = (char *)malloc(4);
	if(NULL == server_identifier->value)
	{
		dhcp_free_packet(response);
		Error(ERR_ABORT, 0, "***Allocate memory failed! %s(%d)*** do_discover==>", strerror(errno), errno);
		return NULL;
	}
	UINT32 addr = sok_addr_32(gobal_config.server);
    server_identifier->length = 4;
	memcpy(server_identifier->value, &addr, server_identifier->length);
    packet_type->next = server_identifier;

	//lease time
	struct dhcp_option *lease_time = (struct dhcp_option*)malloc(sizeof(struct dhcp_option));
	if(NULL == lease_time)
	{
		dhcp_free_packet(response);
		Error(ERR_ABORT, 0, "***Allocate memory failed! %s(%d)*** do_discover==>", strerror(errno), errno);
		return NULL;
	}
    memset(lease_time, 0, sizeof(struct dhcp_option));
    lease_time->code = DHO_DHCP_LEASE_TIME;
    lease_time->value = (char *)malloc(4);
	if(NULL == lease_time->value)
	{
		dhcp_free_packet(response);
		Error(ERR_ABORT, 0, "***Allocate memory failed! %s(%d)*** do_discover==>", strerror(errno), errno);
		return NULL;
	}
    memcpy(lease_time->value, &gobal_config.lease, 4);
    lease_time->length = 4;
    server_identifier->next = lease_time;

	//renew time
	struct dhcp_option *renew_time = (struct dhcp_option*)malloc(sizeof(struct dhcp_option));
	if(NULL == renew_time)
	{
		dhcp_free_packet(response);
		Error(ERR_ABORT, 0, "***Allocate memory failed! %s(%d)*** do_discover==>", strerror(errno), errno);
		return NULL;
	}
    memset(renew_time, 0, sizeof(struct dhcp_option));
    renew_time->code = DHO_DHCP_RENEWAL_TIME;
    renew_time->value = (char *)malloc(4);
	if(NULL == renew_time->value)
	{
		dhcp_free_packet(response);
		Error(ERR_ABORT, 0, "***Allocate memory failed! %s(%d)*** do_discover==>", strerror(errno), errno);
		return NULL;
	}
    memcpy(renew_time->value, &gobal_config.renew, 4);
    renew_time->length = 4;
    lease_time->next = renew_time;

	//router/gateway
	struct dhcp_option *router = (struct dhcp_option*)malloc(sizeof(struct dhcp_option));
	if(NULL == router)
	{
		dhcp_free_packet(response);
		Error(ERR_ABORT, 0, "***Allocate memory failed! %s(%d)*** do_discover==>", strerror(errno), errno);
		return NULL;
	}
    memset(router, 0, sizeof(struct dhcp_option));
    router->code = DHO_ROUTERS;
    router->value = (char *)malloc(4);
	if(NULL == router->value)
	{
		dhcp_free_packet(response);
		Error(ERR_ABORT, 0, "***Allocate memory failed! %s(%d)*** do_discover==>", strerror(errno), errno);
		return NULL;
	}
    memcpy(router->value, &config.router, 4);
    router->length = 4;
    renew_time->next = router;

	//netmask
	struct dhcp_option *subnet_mask = (struct dhcp_option*)malloc(sizeof(struct dhcp_option));
	if(NULL == subnet_mask)
	{
		dhcp_free_packet(response);
		Error(ERR_ABORT, 0, "***Allocate memory failed! %s(%d)*** do_discover==>", strerror(errno), errno);
		return NULL;
	}
    memset(subnet_mask, 0, sizeof(struct dhcp_option));
    subnet_mask->code = DHO_SUBNET;
    subnet_mask->value = (char *)malloc(4);
	if(NULL == subnet_mask->value)
	{
		dhcp_free_packet(response);
		Error(ERR_ABORT, 0, "***Allocate memory failed! %s(%d)*** do_discover==>", strerror(errno), errno);
		return NULL;
	}
    memcpy(subnet_mask->value, &config.netmask, 4);
    subnet_mask->length = 4;
    router->next = subnet_mask;

	//dns
	struct dhcp_option *dns_server = (struct dhcp_option*)malloc(sizeof(struct dhcp_option));
	if(NULL == dns_server)
	{
		dhcp_free_packet(response);
		Error(ERR_ABORT, 0, "***Allocate memory failed! %s(%d)*** do_discover==>", strerror(errno), errno);
		return NULL;
	}
    memset(dns_server, 0, sizeof(struct dhcp_option));
    dns_server->code = DHO_DOMAIN_NAME_SERVERS;
    dns_server->value = (char *)malloc(8);
	if(NULL == dns_server->value)
	{
		dhcp_free_packet(response);
		Error(ERR_ABORT, 0, "***Allocate memory failed! %s(%d)*** do_discover==>", strerror(errno), errno);
		return NULL;
	}
    memcpy(dns_server->value, &config.dns1, 4);
	memcpy(dns_server->value + 4, &config.dns2, 4);
    dns_server->length = 8;
    subnet_mask->next = dns_server;
	
	TrPrintfL(DHCP_TRACE>1, "DHCP: do_inform==>");
	return response;
}

struct dhcp_packet *do_decline(struct dhcp_packet *request)
{
	TrPrintfL(DHCP_TRACE>1, "DHCP: do_decline");
    return NULL;
	TrPrintfL(DHCP_TRACE>1, "DHCP: do_decline==>");
}


int main()
{
	err_init(0,64);
	dhcp_start();
}

