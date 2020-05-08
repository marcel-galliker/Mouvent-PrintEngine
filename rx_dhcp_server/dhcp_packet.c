#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<errno.h>

#include "rx_error.h"
#include "rx_trace.h"

#include "dhcp_server.h"
#include "dhcp_packet.h"

char DHCP_MAGIC_COOKIE[4] = {0x63, 0x82, 0x53, 0x63};

//Caller need to free the memory used for the DHCP packet 
struct dhcp_packet *dhcp_marshall(char buffer[], int offset, int length)
{
	TrPrintfL(DHCP_TRACE>1, "DHCP: ==>marshall, offset=%d, length=%d", offset, length);
	
	struct dhcp_packet *packet = NULL;
	//first check if the arguments is valid
	if(NULL == buffer)
	{
		Error(ERR_CONT, 0, "***BUFFER for marshall is NULL***");
		goto ERROR_POS;
	}
	
	if(length < BOOTP_ABSOLUTE_MIN_LEN)
	{
		Error(ERR_CONT, 0, "The length of dhcp packet is less than min size");
		goto ERROR_POS;
	}

	if(length > DHCP_MAX_MTU)
	{
		Error(ERR_CONT, 0, "The length of dhcp packet is more than max MTU");
		goto ERROR_POS;
	}

	packet = (struct dhcp_packet*)malloc(sizeof(struct dhcp_packet));
	if(NULL == packet)
	{
		Error(ERR_ABORT, 0, "***Allocate memory failed! %s(%d)***", strerror(errno), errno);
		goto ERROR_POS;
	}
	
	memset(packet, 0, sizeof(struct dhcp_packet));
	
	UCHAR* packet_begin = buffer + offset; 
	//parse static part of packet
	memcpy(&(packet->op),		packet_begin, 1);
	memcpy(&(packet->htype),	packet_begin + 1, 1);
	memcpy(&(packet->hlen),		packet_begin + 2, 1);
	memcpy(&(packet->hops),		packet_begin + offset + 3, 1);
	memcpy(&packet->xid,		packet_begin + 4, 4);
	memcpy(&packet->secs,		packet_begin + 8, 2);
	memcpy(&packet->flags,		packet_begin + 10, 2);
	memcpy(&packet->ciaddr,		packet_begin + 12, 4);
	memcpy(&packet->yiaddr,		packet_begin + 16, 4);
	memcpy(&packet->siaddr,		packet_begin + 20, 4);
	memcpy(&packet->giaddr,		packet_begin + 24, 4);
	memcpy(packet->chaddr,		packet_begin + 28, 16);
	memcpy(packet->sname,		packet_begin + 44, 64);
	memcpy(packet->file,		packet_begin + 108, 128);
	
	TrPrintfL(DHCP_TRACE, "DHCP: --------------DUMP DHCP PACKET-------------");
	TrPrintfL(DHCP_TRACE, "DHCP: packet->op=%d", packet->op);
	TrPrintfL(DHCP_TRACE, "DHCP: packet->htype=%d", packet->htype);
	TrPrintfL(DHCP_TRACE, "DHCP: packet->hlen=%d", packet->hlen);
	TrPrintfL(DHCP_TRACE, "DHCP: packet->hops=%d", packet->hops);
	TrPrintfL(DHCP_TRACE, "DHCP: packet->xid=0x%08x", packet->xid);
	TrPrintfL(DHCP_TRACE, "DHCP: packet->secs=0x%04x", packet->secs);
	TrPrintfL(DHCP_TRACE, "DHCP: packet->flags=0x%04x", packet->flags);
	TrPrintfL(DHCP_TRACE, "DHCP: packet->ciaddr=0x%08x", packet->ciaddr);
	TrPrintfL(DHCP_TRACE, "DHCP: packet->yiaddr=0x%08x", packet->yiaddr);
	TrPrintfL(DHCP_TRACE, "DHCP: packet->siaddr=0x%08x", packet->siaddr);
	TrPrintfL(DHCP_TRACE, "DHCP: packet->giaddr=0x%08x", packet->giaddr);
	TrPrintfL(DHCP_TRACE, "DHCP: packet->chaddr=%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x", packet->chaddr[0], packet->chaddr[1], packet->chaddr[2], 
	packet->chaddr[3], packet->chaddr[4], packet->chaddr[5], packet->chaddr[6], packet->chaddr[7], packet->chaddr[8], packet->chaddr[9], 
	packet->chaddr[10], packet->chaddr[11], packet->chaddr[12], packet->chaddr[13], packet->chaddr[14], packet->chaddr[15]);
	TrPrintfL(DHCP_TRACE, "DHCP: DHCP: packet->sname=%s", packet->sname);
	TrPrintfL(DHCP_TRACE, "packet->file=%s", packet->file);
	TrPrintfL(DHCP_TRACE, "DHCP: ---------------------------------------------");
	
	//check DHCP magic cookie
	char magic[4];
	memcpy(magic, packet_begin + 236, 4);
	if(0 != memcmp(DHCP_MAGIC_COOKIE, magic, 4))
	{
		Error(ERR_CONT, 0, "DHCP packet magic cookie is not matched!");
		goto ERROR_POS;
	}

	//parse options
	int options_offset = 240; //236 + 4
	packet->options = NULL;
	struct dhcp_option *prev = NULL;
	while(1)
	{ 
		if(options_offset > length - 1)
		{
			break;
		}
		
		//code
		char code;
		memcpy(&code, packet_begin + options_offset, 1);
		options_offset++;

		TrPrintfL(DHCP_TRACE, "DHCP: dhcp option code=%d", code);

		if(DHO_PAD == code)
		{
			continue;
		}
		
		if(DHO_END == code)
		{
			TrPrintfL(DHCP_TRACE>1, "DHCP: dhcp option end");
			break;
		}

		//length
		int len;
		char len_buff;
		memcpy(&len_buff, packet_begin + options_offset, 1);
		len = (int)len_buff;
		options_offset++;
		
		TrPrintfL(DHCP_TRACE, "DHCP: dhcp option length=%d", len);

		if(options_offset + len > length - 1)
		{
			Error(WARN, 0, "The options length is more than packet length, but no end mark.");
			break;
		}
		
		//value
		struct dhcp_option * option = (struct dhcp_option*)malloc(sizeof(struct dhcp_option));
		if(NULL == option)
		{
			Error(ERR_ABORT, 0, "***Allocate memory failed! %s(%d)***", strerror(errno), errno);
			goto ERROR_POS;
		}
		memset(option, 0, sizeof(struct dhcp_option));

		option->code = code;
		option->length = len_buff;
		option->value = (char*)malloc(len);
		if(NULL == option->value)
		{
			Error(ERR_ABORT, 0, "***Allocate memory failed! %s(%d)***", strerror(errno), errno);
			goto ERROR_POS;
		}
		memcpy(option->value, buffer + options_offset, len);
		option->next = NULL;	
		options_offset += len;
		
		//Add the option into the packet
		if(NULL == packet->options)
		{
			packet->options = option;
		}	
		if(NULL != prev)
		{
			prev->next = option;
		}
		
		prev = option;
	}

	if(options_offset < length - 1)
	{
		packet->padding = (char*)malloc(length - options_offset);
		if(NULL == packet->padding)
		{
			Error(ERR_ABORT, 0, "***Allocate memory failed! %s(%d)***", strerror(errno), errno);
		}
		else
		{
			memcpy(packet->padding, buffer + options_offset, length - options_offset - 1);
		}
	}
	else
	{
		packet->padding = NULL;
	}
	
	TrPrintfL(DHCP_TRACE>1, "DHCP: marshall==>");
	return packet;

ERROR_POS:
	if(NULL != packet)
	{
		dhcp_free_packet(packet);
	}
	Error(WARN, 0, "***error!*** marshall==>");
	return NULL;
}

//Use this function to free dhcp packet
void dhcp_free_packet(struct dhcp_packet *packet)
{
	TrPrintfL(DHCP_TRACE>1, "DHCP: ==>dhcp_free_packet, packet=%d", packet);
	if(NULL == packet)
	{
		TrPrintfL(DHCP_TRACE>1, "DHCP: packet pointer is NULL, dhcp_free_packet==>");
		return;
	}

	if(NULL != packet->padding)
	{
		free(packet->padding);
	}

	struct dhcp_option *option = packet->options;
	while(option != NULL)
	{
		if(option->value != NULL) free(option->value);
		struct dhcp_option *current = option; 
		option = current->next;
		
		free(current);
	}

	free(packet);
	
	TrPrintfL(DHCP_TRACE>1, "DHCP: dhcp_free_packet==>");
	return;
}

int dhcp_serialize(struct dhcp_packet *packet, char buffer[], int length)
{
	TrPrintfL(DHCP_TRACE>1, "DHCP: serialize==>, packet=%d", packet);
	if(NULL == packet)
	{
		TrPrintfL(DHCP_TRACE>1, "DHCP: packet pointer is NULL, ==>serialize");
		return 0;
	}

	//calculate the total size of the packet
	//static part
	int packet_len = BOOTP_ABSOLUTE_MIN_LEN;
	//magic cookie
	packet_len += sizeof(DHCP_MAGIC_COOKIE);
	//options
	struct dhcp_option *option = packet->options;
	while(NULL != option)
	{
		packet_len += 2;
		packet_len += (int)option->length;
		option = option->next;
	}
	//end option
	packet_len++;
	
	//calculate padding length
	int padding_len = 0;
	if(packet_len < BOOTP_ABSOLUTE_MIN_LEN + DHCP_VEND_SIZE)
	{
		padding_len = DHCP_VEND_SIZE + BOOTP_ABSOLUTE_MIN_LEN - packet_len;
		packet_len = DHCP_VEND_SIZE + BOOTP_ABSOLUTE_MIN_LEN;
	}
	
	if(packet_len > length)
	{
		Error(ERR_CONT, 0, "Buffer size is less than packet length, buffer size=%d, packet length=%d", length, packet_len);
		TrPrintfL(DHCP_TRACE>1, "DHCP: ==>serialize");
		return 0;
	}
	
	TrPrintfL(DHCP_TRACE, "DHCP: --------------DUMP DHCP PACKET-------------");
	TrPrintfL(DHCP_TRACE, "DHCP: packet->op=%d", packet->op);
	TrPrintfL(DHCP_TRACE, "DHCP: packet->htype=%d", packet->htype);
	TrPrintfL(DHCP_TRACE, "DHCP: packet->hlen=%d", packet->hlen);
	TrPrintfL(DHCP_TRACE, "DHCP: packet->hops=%d", packet->hops);
	TrPrintfL(DHCP_TRACE, "DHCP: packet->xid=0x%08x", packet->xid);
	TrPrintfL(DHCP_TRACE, "DHCP: packet->secs=0x%04x", packet->secs);
	TrPrintfL(DHCP_TRACE, "DHCP: packet->flags=0x%04x", packet->flags);
	TrPrintfL(DHCP_TRACE, "DHCP: packet->ciaddr=0x%08", packet->ciaddr);
	TrPrintfL(DHCP_TRACE, "DHCP: packet->yiaddr=0x%08", packet->yiaddr);
	TrPrintfL(DHCP_TRACE, "DHCP: packet->siaddr=0x%08", packet->siaddr);
	TrPrintfL(DHCP_TRACE, "DHCP: packet->giaddr=0x%08", packet->giaddr);
	TrPrintfL(DHCP_TRACE, "DHCP: packet->chaddr=%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x", packet->chaddr[0], packet->chaddr[1], packet->chaddr[2], 
	packet->chaddr[3], packet->chaddr[4], packet->chaddr[5], packet->chaddr[6], packet->chaddr[7], packet->chaddr[8], packet->chaddr[9], 
	packet->chaddr[10], packet->chaddr[11], packet->chaddr[12], packet->chaddr[13], packet->chaddr[14], packet->chaddr[15]);
	TrPrintfL(DHCP_TRACE, "DHCP: packet->sname=%s", packet->sname);
	TrPrintfL(DHCP_TRACE, "DHCP: packet->file=%s", packet->file);
	TrPrintfL(DHCP_TRACE, "DHCP: ---------------------------------------------");
	
	memcpy(buffer, &(packet->op), 1);
	memcpy(buffer + 1, &(packet->htype), 1);
	memcpy(buffer + 2, &(packet->hlen), 1);
	memcpy(buffer + 3, &(packet->hops), 1);
	memcpy(buffer + 4, &packet->xid, 4);
	memcpy(buffer + 8, &packet->secs, 2);
	memcpy(buffer + 10, &packet->flags, 2);
	memcpy(buffer + 12, &packet->ciaddr, 4);
	memcpy(buffer + 16, &packet->yiaddr, 4);
	memcpy(buffer + 20, &packet->siaddr, 4);
	memcpy(buffer + 24, &packet->giaddr, 4);
	memcpy(buffer + 28, packet->chaddr, 16);
	memcpy(buffer + 44, packet->sname, 64);
	memcpy(buffer + 108, packet->file, 128);

	memcpy(buffer + 236, DHCP_MAGIC_COOKIE, 4);
	
	int options_offset = 240;
	option = packet->options;
    while(NULL != option)
    {
		TrPrintfL(DHCP_TRACE, "DHCP: dhcp option code=%d, length=%d", option->code, option->length);
		memcpy(buffer + options_offset, &(option->code), 1);
		options_offset++;
		memcpy(buffer + options_offset, &(option->length), 1);
		options_offset++;

		int len = (int)option->length;
		memcpy(buffer + options_offset, option->value, len);
		options_offset += len;		

		option = option->next;
	}

	char dhcp_option_end = DHO_END;
	memcpy(buffer + options_offset, &dhcp_option_end, 1);
	options_offset++;	

	if(padding_len > 0)
	{
		memset(buffer + options_offset, 0, padding_len);  
	}

	TrPrintfL(DHCP_TRACE>1, "DHCP: total %d bytes writen, ==>serialize", packet_len);
	return packet_len; 
}
