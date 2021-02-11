
#include <stdio.h>
#include <stdarg.h>

#include <systick.h>
#include "ethernet.h"
#include "sa_tcp_ip.h"
#include "rx_trace.h"

static int	_Enabled = 0;

//--- TrEnable ----------------------------------------
void TrEnable(int enable)
{
	_Enabled = enable;
}

//--- TrPrintf --------------------------------------------
void TrPrintf(int trace, const char *format, ...)
{
	va_list ap;
	int len;
	
	if (_Enabled && trace)
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
