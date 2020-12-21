
#include <stdio.h>
#include <stdarg.h>

#include "ethernet.h"
#include "sa_tcp_ip.h"
#include "systick.h"
#include "rx_trace.h"

void TrPrintf(int trace, const char *format, ...)
{
	va_list ap;
	int len;
	
	if (trace)
	{
		SSetupAssist_TraceMsg msg;
		msg.hdr.msgId  = CMD_TRACE_GET;
		int now=sys_now();
		
		len = sprintf(msg.message, "%d.%03d: ", now/1000, now%1000);	
		va_start(ap, format);
		len += vsnprintf(msg.message+len, sizeof(msg.message)-len-1, format, ap);
		va_end(ap);
		msg.hdr.msgLen = sizeof(msg.hdr)+len+1;
		ethernet_send(&msg, msg.hdr.msgLen);
	}
} // end TrPrintf
